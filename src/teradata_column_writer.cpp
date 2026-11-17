#include "teradata_column_writer.hpp"

#include "duckdb/common/types/cast_helpers.hpp"
#include "duckdb/function/scalar/strftime_format.hpp"

#include <duckdb/common/types/time.hpp>

#include <algorithm>

namespace duckdb {

//----------------------------------------------------------------------------------------------------------------------
// Teradata Column Writer
//----------------------------------------------------------------------------------------------------------------------

void TeradataColumnWriter::SetPresenceBits(idx_t count, idx_t col_idx, char *records[]) const {
	const auto byte_idx = col_idx / 8;
	const auto bit_idx = (7 - (col_idx % 8));

	for (idx_t out_idx = 0; out_idx < count; out_idx++) {
		const auto row_idx = format.sel->get_index(out_idx);

		// Set the presence bit
		if (!format.validity.RowIsValid(row_idx)) {
			auto &validity_byte = records[out_idx][byte_idx];
			validity_byte |= static_cast<char>(1) << bit_idx;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
// Helper alias
//----------------------------------------------------------------------------------------------------------------------

template <class T>
using const_optional_ptr = optional_ptr<const T>;

//----------------------------------------------------------------------------------------------------------------------
// Typed Column Writer Helper Class
//----------------------------------------------------------------------------------------------------------------------

template <class T>
class TypedColumnWriter : public TeradataColumnWriter {
public:
	void ComputeSizes(idx_t count, int32_t lengths[]) override {
		auto data_ptr = UnifiedVectorFormat::GetData<T>(format);

		for (idx_t out_idx = 0; out_idx < count; out_idx++) {
			const auto row_idx = format.sel->get_index(out_idx);
			const auto is_valid = format.validity.RowIsValid(row_idx);

			if (is_valid) {
				lengths[out_idx] += Reserve(&data_ptr[row_idx]);
			} else {
				lengths[out_idx] += Reserve(nullptr);
			}
		}
	}

	void EncodeVector(idx_t count, char *records[]) override {
		auto data_ptr = UnifiedVectorFormat::GetData<T>(format);

		for (idx_t out_idx = 0; out_idx < count; out_idx++) {
			const auto row_idx = format.sel->get_index(out_idx);
			const auto is_valid = format.validity.RowIsValid(row_idx);

			if (is_valid) {
				Encode(&data_ptr[row_idx], records[out_idx]);
			} else {
				Encode(nullptr, records[out_idx]);
			}
		}
	}

	virtual int32_t Reserve(const_optional_ptr<T> value) = 0;
	virtual void Encode(const_optional_ptr<T> value, char *&result) = 0;
};

//----------------------------------------------------------------------------------------------------------------------
// Column Writers
//----------------------------------------------------------------------------------------------------------------------

class TeradataVarcharWriter final : public TypedColumnWriter<string_t> {
public:
	int32_t Reserve(const_optional_ptr<string_t> value) override {
		return sizeof(uint16_t) + (value ? value->GetSize() : 0);
	}

	// TODO: Deal with character encoding/conversion here
	void Encode(const_optional_ptr<string_t> value, char *&result) override {
		if (value) {
			const auto len = value->GetSize();
			memcpy(result, &len, sizeof(uint16_t));
			result += sizeof(uint16_t);

			memcpy(result, value->GetDataUnsafe(), len);
			result += len;
		} else {
			constexpr auto zero_length = 0;
			memcpy(result, &zero_length, sizeof(uint16_t));
			result += sizeof(uint16_t);
		}
	}
};

class TeradataBlobWriter final : public TypedColumnWriter<string_t> {
public:
	int32_t Reserve(const_optional_ptr<string_t> value) override {
		return sizeof(uint16_t) + (value ? value->GetSize() : 0);
	}

	void Encode(const_optional_ptr<string_t> value, char *&result) override {
		if (value) {
			const auto len = value->GetSize();
			memcpy(result, &len, sizeof(uint16_t));
			result += sizeof(uint16_t);

			memcpy(result, value->GetDataUnsafe(), len);
			result += len;
		} else {
			constexpr auto zero_length = 0;
			memcpy(result, &zero_length, sizeof(uint16_t));
			result += sizeof(uint16_t);
		}
	}
};

template <class SOURCE, class TARGET = SOURCE>
class TeradataFixedSizeWriter final : public TypedColumnWriter<SOURCE> {
public:
	int32_t Reserve(const_optional_ptr<SOURCE> value) override {
		return sizeof(TARGET);
	}

	void Encode(const_optional_ptr<SOURCE> value, char *&result) override {
		if (value) {
			const auto src = *value;
			const auto dst = static_cast<TARGET>(src);

			// We have to memcpy here to avoid alignment issues
			memcpy(result, &dst, sizeof(TARGET));
		}

		result += sizeof(TARGET);
	}
};

class TeradataDateWriter final : public TypedColumnWriter<date_t> {
public:
	int32_t Reserve(const_optional_ptr<date_t> value) override {
		return sizeof(int32_t);
	}

	void Encode(const_optional_ptr<date_t> value, char *&result) override {
		if (value) {

			int32_t year = 0;
			int32_t month = 0;
			int32_t day = 0;

			Date::Convert(*value, year, month, day);

			const auto td_date = (year - 1900) * 10000 + month * 100 + day;

			memcpy(result, &td_date, sizeof(int32_t));
		}

		result += sizeof(int32_t);
	}
};

class TeradataTimestampTZWriter final : public TypedColumnWriter<timestamp_t> {
public:
	static constexpr auto CHAR_SIZE = 32;

	TeradataTimestampTZWriter() {
		StrfTimeFormat::ParseFormatSpecifier("%Y-%m-%d %H:%M:%S.%f+00:00", time_format);
	}

	int32_t Reserve(const_optional_ptr<timestamp_t> value) override {
		return CHAR_SIZE;
	}

	void Encode(const_optional_ptr<timestamp_t> value, char *&result) override {
		if (value) {
			const auto date = Timestamp::GetDate(*value);
			const auto time = Timestamp::GetTime(*value);

			const auto len = time_format.GetLength(date, time, 0, nullptr);

			if (len != CHAR_SIZE) {
				throw InvalidInputException("Teradata timestamp: '%s' is not in the expected format",
				                            Timestamp::ToString(*value));
			}

			time_format.FormatString(date, time, result);
		}

		result += CHAR_SIZE;
	}

private:
	StrfTimeFormat time_format;
};

class TeradataTimestampUSWriter final : public TypedColumnWriter<timestamp_t> {
public:
	static constexpr auto CHAR_SIZE = 26;

	TeradataTimestampUSWriter() {
		StrfTimeFormat::ParseFormatSpecifier("%Y-%m-%d %H:%M:%S.%f", time_format);
	}

	int32_t Reserve(const_optional_ptr<timestamp_t> value) override {
		return CHAR_SIZE;
	}

	void Encode(const_optional_ptr<timestamp_t> value, char *&result) override {
		if (value) {
			const auto date = Timestamp::GetDate(*value);
			const auto time = Timestamp::GetTime(*value);

			const auto len = time_format.GetLength(date, time, 0, nullptr);

			if (len != CHAR_SIZE) {
				throw InvalidInputException("Teradata timestamp: '%s' is not in the expected format",
				                            Timestamp::ToString(*value));
			}

			time_format.FormatString(date, time, result);
		}

		result += CHAR_SIZE;
	}

private:
	StrfTimeFormat time_format;
};

class TeradataTimestampMSWriter final : public TypedColumnWriter<timestamp_t> {
public:
	static constexpr auto CHAR_SIZE = 23;

	TeradataTimestampMSWriter() {
		StrfTimeFormat::ParseFormatSpecifier("%Y-%m-%d %H:%M:%S.%g", time_format);
	}

	int32_t Reserve(const_optional_ptr<timestamp_t> value) override {
		return CHAR_SIZE;
	}

	void Encode(const_optional_ptr<timestamp_t> value, char *&result) override {
		if (value) {
			auto ts = *value;

			if (ts.IsFinite()) {
				ts = Timestamp::FromEpochMs(ts.value);
			}

			const auto date = Timestamp::GetDate(ts);
			const auto time = Timestamp::GetTime(ts);
			const auto len = time_format.GetLength(date, time, 0, nullptr);

			if (len != CHAR_SIZE) {
				throw InvalidInputException("Teradata timestamp: '%s' is not in the expected format",
				                            Timestamp::ToString(ts));
			}

			time_format.FormatString(date, time, result);
		}

		result += CHAR_SIZE;
	}

private:
	StrfTimeFormat time_format;
};

class TeradataTimestampSecWriter final : public TypedColumnWriter<timestamp_t> {
public:
	static constexpr auto CHAR_SIZE = 19;

	TeradataTimestampSecWriter() {
		StrfTimeFormat::ParseFormatSpecifier("%Y-%m-%d %H:%M:%S", time_format);
	}

	int32_t Reserve(const_optional_ptr<timestamp_t> value) override {
		return CHAR_SIZE;
	}

	void Encode(const_optional_ptr<timestamp_t> value, char *&result) override {
		if (value) {
			auto ts = *value;

			if (ts.IsFinite()) {
				ts = Timestamp::FromEpochSeconds(ts.value);
			}

			const auto date = Timestamp::GetDate(ts);
			const auto time = Timestamp::GetTime(ts);

			const auto len = time_format.GetLength(date, time, 0, nullptr);

			if (len != CHAR_SIZE) {
				throw InvalidInputException("Teradata timestamp: '%s' is not in the expected format",
				                            Timestamp::ToString(ts));
			}

			time_format.FormatString(date, time, result);
		}

		result += CHAR_SIZE;
	}

private:
	StrfTimeFormat time_format;
};

class TeradataTimeWriter final : public TypedColumnWriter<dtime_t> {
public:
	static constexpr auto CHAR_SIZE = 15;

	int32_t Reserve(const_optional_ptr<dtime_t> value) override {
		return CHAR_SIZE;
	}

	void Encode(const_optional_ptr<dtime_t> value, char *&result) override {
		if (value) {
			const auto &time = *value;

			int32_t time_units[4];
			Time::Convert(time, time_units[0], time_units[1], time_units[2], time_units[3]);

			char micro_buffer[6];
			TimeToStringCast::FormatMicros(time_units[3], micro_buffer);
			TimeToStringCast::Format(result, CHAR_SIZE, time_units, micro_buffer);
		}
		result += CHAR_SIZE;
	}
};

class TeradataTimeTZWriter final : public TypedColumnWriter<dtime_tz_t> {
public:
	static constexpr auto CHAR_SIZE = 21;

	int32_t Reserve(const_optional_ptr<dtime_tz_t> value) override {
		return CHAR_SIZE;
	}

	void Encode(const_optional_ptr<dtime_tz_t> value, char *&result) override {
		if (value) {
			const auto time = value->time();

			int32_t time_units[4];
			Time::Convert(time, time_units[0], time_units[1], time_units[2], time_units[3]);

			char micro_buffer[6];
			TimeToStringCast::FormatMicros(time_units[3], micro_buffer);
			TimeToStringCast::Format(result, 15, time_units, micro_buffer);

			memcpy(result + 15, "+00:00", 6);
		}

		result += CHAR_SIZE;
	}
};

//----------------------------------------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------------------------------------

namespace {

using WriterFactory = unique_ptr<TeradataColumnWriter> (*)();

template <class WRITER>
unique_ptr<TeradataColumnWriter> MakeWriter() {
	return make_uniq_base<TeradataColumnWriter, WRITER>();
}

struct WriterEntry {
	LogicalTypeId id;
	WriterFactory factory;
};

const WriterEntry WRITERS[] = {
    {LogicalTypeId::TINYINT, MakeWriter<TeradataFixedSizeWriter<int8_t>>},
    {LogicalTypeId::SMALLINT, MakeWriter<TeradataFixedSizeWriter<int16_t>>},
    {LogicalTypeId::INTEGER, MakeWriter<TeradataFixedSizeWriter<int32_t>>},
    {LogicalTypeId::BIGINT, MakeWriter<TeradataFixedSizeWriter<int64_t>>},
    {LogicalTypeId::DOUBLE, MakeWriter<TeradataFixedSizeWriter<double>>},
    {LogicalTypeId::VARCHAR, MakeWriter<TeradataVarcharWriter>},
    {LogicalTypeId::BLOB, MakeWriter<TeradataBlobWriter>},
    {LogicalTypeId::DATE, MakeWriter<TeradataDateWriter>},
    {LogicalTypeId::TIME, MakeWriter<TeradataTimeWriter>},
    {LogicalTypeId::TIME_TZ, MakeWriter<TeradataTimeTZWriter>},
    {LogicalTypeId::TIMESTAMP_SEC, MakeWriter<TeradataTimestampSecWriter>},
    {LogicalTypeId::TIMESTAMP_MS, MakeWriter<TeradataTimestampMSWriter>},
    {LogicalTypeId::TIMESTAMP, MakeWriter<TeradataTimestampUSWriter>},
    {LogicalTypeId::TIMESTAMP_TZ, MakeWriter<TeradataTimestampTZWriter>},
};

struct DecimalWriterEntry {
	uint8_t max_width;
	WriterFactory factory;
};

// Decimals are stored in the smallest integer that fits their width.
// In Teradata, the small decimals are stored as 1 byte, compared to 2 bytes in DuckDB.
const DecimalWriterEntry DECIMAL_WRITERS[] = {
    {2, MakeWriter<TeradataFixedSizeWriter<int16_t, int8_t>>}, {4, MakeWriter<TeradataFixedSizeWriter<int16_t>>},
    {9, MakeWriter<TeradataFixedSizeWriter<int32_t>>},         {18, MakeWriter<TeradataFixedSizeWriter<int64_t>>},
    {38, MakeWriter<TeradataFixedSizeWriter<hugeint_t>>},
};

WriterFactory FindDecimalWriter(uint8_t width) {
	const auto entry = std::find_if(std::begin(DECIMAL_WRITERS), std::end(DECIMAL_WRITERS),
	                                [&](const DecimalWriterEntry &e) { return width <= e.max_width; });
	if (entry == std::end(DECIMAL_WRITERS)) {
		throw InvalidInputException("Invalid Teradata decimal width: %d", width);
	}
	return entry->factory;
}

WriterFactory FindWriter(const LogicalType &type) {
	if (type.id() == LogicalTypeId::DECIMAL) {
		return FindDecimalWriter(DecimalType::GetWidth(type));
	}
	const auto entry =
	    std::find_if(std::begin(WRITERS), std::end(WRITERS), [&](const WriterEntry &e) { return e.id == type.id(); });
	if (entry == std::end(WRITERS)) {
		throw NotImplementedException("TeradataColumnWriter::Make: type %s not supported", type.ToString());
	}
	return entry->factory;
}

} // namespace

unique_ptr<TeradataColumnWriter> TeradataColumnWriter::Make(const LogicalType &type) {
	return FindWriter(type)();
}

} // namespace duckdb