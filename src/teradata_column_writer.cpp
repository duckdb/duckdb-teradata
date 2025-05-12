#include "teradata_column_writer.hpp"

#include "duckdb/function/scalar/strftime_format.hpp"

#include <duckdb/common/types/time.hpp>

namespace duckdb {

void TeradataColumnWriter::SetPresenceBits(idx_t count, idx_t col_idx, idx_t col_count, char *records[]) const {
	const auto byte_idx = col_idx / 8;
	const auto bit_idx = (7 - (col_idx % 8));

	for (idx_t out_idx = 0; out_idx < count; out_idx++) {
		const auto row_idx = format.sel->get_index(out_idx);

		// Set the presence bit
		if (!format.validity.RowIsValid(row_idx)) {
			auto &validity_byte = records[out_idx][byte_idx];
			validity_byte |= 1 << bit_idx;
		}
	}
}

template <class IMPL>
class TeradataColumnWriterBase : public TeradataColumnWriter {
public:
	void ComputeSizes(idx_t count, int32_t lengths[]) override {
		for (idx_t out_idx = 0; out_idx < count; out_idx++) {
			// TODO: check that length doesnt overflow
			lengths[out_idx] += static_cast<IMPL *>(this)->GetSize(format, out_idx);
		}
	}

	void EncodeVector(idx_t count, char *records[]) override {
		for (idx_t out_idx = 0; out_idx < count; out_idx++) {
			static_cast<IMPL *>(this)->Encode(format, out_idx, records[out_idx]);
		}
	}

protected:
	// Needs to be implemented by the derived class
	virtual idx_t GetSize(UnifiedVectorFormat &format, idx_t out_idx) = 0;
	virtual void Encode(UnifiedVectorFormat &format, idx_t out_idx, char *&result) = 0;
};

//----------------------------------------------------------------------------------------------------------------------
// Types Writers
//----------------------------------------------------------------------------------------------------------------------

class TeradataVarcharWriter final : public TeradataColumnWriterBase<TeradataVarcharWriter> {
public:
	idx_t GetSize(UnifiedVectorFormat &format, idx_t out_idx) override {
		const auto row_idx = format.sel->get_index(out_idx);
		if (format.validity.RowIsValid(row_idx)) {
			const auto &str = UnifiedVectorFormat::GetData<string_t>(format)[row_idx];
			const auto len = str.GetSize();
			return sizeof(uint16_t) + len;
		}
		// We always write a length, even if null
		return sizeof(uint16_t);
	}

	// TODO: Deal with character encoding/conversion here
	void Encode(UnifiedVectorFormat &format, idx_t out_idx, char *&result) override {
		const auto row_idx = format.sel->get_index(out_idx);
		if (format.validity.RowIsValid(row_idx)) {
			const auto &str = UnifiedVectorFormat::GetData<string_t>(format)[row_idx];
			const auto len = str.GetSize();

			memcpy(result, &len, sizeof(uint16_t));
			result += sizeof(uint16_t);

			memcpy(result, str.GetDataUnsafe(), len);
			result += len;
		} else {
			constexpr auto zero_length = 0;
			memcpy(result, &zero_length, sizeof(uint16_t));
			result += sizeof(uint16_t);
		}
	}
};

class TeradataBlobWriter final : public TeradataColumnWriterBase<TeradataBlobWriter> {
public:
	idx_t GetSize(UnifiedVectorFormat &format, idx_t out_idx) override {
		const auto row_idx = format.sel->get_index(out_idx);
		if (format.validity.RowIsValid(row_idx)) {
			const auto &str = UnifiedVectorFormat::GetData<string_t>(format)[row_idx];
			const auto len = str.GetSize();
			return sizeof(uint16_t) + len;
		}
		// We always write a length, even if null
		return sizeof(uint16_t);
	}

	void Encode(UnifiedVectorFormat &format, idx_t out_idx, char *&result) override {
		const auto row_idx = format.sel->get_index(out_idx);
		if (format.validity.RowIsValid(row_idx)) {
			const auto &str = UnifiedVectorFormat::GetData<string_t>(format)[row_idx];
			const auto len = str.GetSize();

			memcpy(result, &len, sizeof(uint16_t));
			result += sizeof(uint16_t);

			memcpy(result, str.GetDataUnsafe(), len);
			result += len;
		} else {
			constexpr auto zero_length = 0;
			memcpy(result, &zero_length, sizeof(uint16_t));
			result += sizeof(uint16_t);
		}
	}
};

template <class SRC, class DST = SRC>
class TeradataFixedSizeWriter final : public TeradataColumnWriterBase<TeradataFixedSizeWriter<SRC, DST>> {
public:
	idx_t GetSize(UnifiedVectorFormat &format, idx_t out_idx) override {
		return sizeof(DST);
	}

	void Encode(UnifiedVectorFormat &format, idx_t out_idx, char *&result) override {
		const auto row_idx = format.sel->get_index(out_idx);

		if (format.validity.RowIsValid(row_idx)) {
			const auto src = UnifiedVectorFormat::GetData<SRC>(format)[row_idx];

			// We have to memcpy here to avoid alignment issues
			const DST dst = static_cast<DST>(src);
			memcpy(result, &dst, sizeof(DST));
		}

		result += sizeof(DST);
	}
};

class TeradataDateWriter final : public TeradataColumnWriterBase<TeradataDateWriter> {
public:
	idx_t GetSize(UnifiedVectorFormat &format, idx_t out_idx) override {
		return sizeof(int32_t);
	}

	void Encode(UnifiedVectorFormat &format, idx_t out_idx, char *&result) override {
		const auto row_idx = format.sel->get_index(out_idx);

		if (format.validity.RowIsValid(row_idx)) {
			const auto duck_date = UnifiedVectorFormat::GetData<date_t>(format)[row_idx];

			int32_t year = 0;
			int32_t month = 0;
			int32_t day = 0;

			Date::Convert(duck_date, year, month, day);

			const auto td_date = (year - 1900) * 10000 + month * 100 + day;

			memcpy(result, &td_date, sizeof(int32_t));
		}

		result += sizeof(int32_t);
	}
};

class TeradataTimestampWriter final : public TeradataColumnWriterBase<TeradataTimestampWriter> {
public:

	StrfTimeFormat time_format;
	idx_t size;

	explicit TeradataTimestampWriter(LogicalTypeId id) {
		switch (id) {
		case LogicalTypeId::TIMESTAMP_SEC:
			StrfTimeFormat::ParseFormatSpecifier("%Y-%m-%d %H:%M:%S", time_format);
			size = 19;
			break;
		case LogicalTypeId::TIMESTAMP_MS:
			StrfTimeFormat::ParseFormatSpecifier("%Y-%m-%d %H:%M:%S.%g", time_format);
			size = 23;
			break;
		case LogicalTypeId::TIMESTAMP:
			StrfTimeFormat::ParseFormatSpecifier("%Y-%m-%d %H:%M:%S.%f", time_format);
			size = 26;
			break;
		case LogicalTypeId::TIMESTAMP_TZ:
			StrfTimeFormat::ParseFormatSpecifier("%Y-%m-%d %H:%M:%S.%f+00:00", time_format);
			size = 32;
			break;
		default:
			throw NotImplementedException("Unknown LogicalTypeId");
		}
	}

	idx_t GetSize(UnifiedVectorFormat &format, idx_t out_idx) override {
		return size;
	}

	// TODO: Deal with character encoding/conversion here
	void Encode(UnifiedVectorFormat &format, idx_t out_idx, char *&result) override {
		const auto row_idx = format.sel->get_index(out_idx);
		if (format.validity.RowIsValid(row_idx)) {
			const auto &ts = UnifiedVectorFormat::GetData<timestamp_t>(format)[row_idx];

			const auto date = Timestamp::GetDate(ts);
			const auto time = Timestamp::GetTime(ts);

			// Subtract one for the null terminator
			const auto len = time_format.GetLength(date, time, 0, nullptr);

			if (len != size) {
				throw InvalidInputException("Teradata timestamp: '%s' is not in the expected format", Timestamp::ToString(ts));
			}

			//auto result = make_unsafe_uniq_array_uninitialized<char>(len);
			time_format.FormatString(date, time, result);
		}

		result += size;
	}
};

class TeradataTimeWriter final : public TeradataColumnWriterBase<TeradataTimeWriter> {
public:
	static constexpr auto ZERO_PREC_TIME_SIZE = 8;
	idx_t GetSize(UnifiedVectorFormat &format, idx_t out_idx) override {
		return ZERO_PREC_TIME_SIZE;
	}

	// TODO: Deal with character encoding/conversion here
	void Encode(UnifiedVectorFormat &format, idx_t out_idx, char *&result) override {
		const auto row_idx = format.sel->get_index(out_idx);
		if (format.validity.RowIsValid(row_idx)) {
			const auto &ts = UnifiedVectorFormat::GetData<dtime_t>(format)[row_idx];
			const auto str = Time::ToString(ts);

			if (str.size() != ZERO_PREC_TIME_SIZE) {
				throw InvalidInputException("Teradata time: '%s' is not in the expected format", str);
			}

			memcpy(result, str.c_str(), str.size());
		}

		result += ZERO_PREC_TIME_SIZE;
	}
};

//----------------------------------------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------------------------------------

unique_ptr<TeradataColumnWriter> TeradataColumnWriter::Make(const LogicalType &type) {
	switch (type.id()) {
	case LogicalTypeId::TINYINT:
		return make_uniq_base<TeradataColumnWriter, TeradataFixedSizeWriter<int8_t>>();
	case LogicalTypeId::SMALLINT:
		return make_uniq_base<TeradataColumnWriter, TeradataFixedSizeWriter<int16_t>>();
	case LogicalTypeId::INTEGER:
		return make_uniq_base<TeradataColumnWriter, TeradataFixedSizeWriter<int32_t>>();
	case LogicalTypeId::BIGINT:
		return make_uniq_base<TeradataColumnWriter, TeradataFixedSizeWriter<int64_t>>();
	case LogicalTypeId::DOUBLE:
		return make_uniq_base<TeradataColumnWriter, TeradataFixedSizeWriter<double>>();
	case LogicalTypeId::VARCHAR:
		return make_uniq_base<TeradataColumnWriter, TeradataVarcharWriter>();
	case LogicalTypeId::BLOB:
		return make_uniq_base<TeradataColumnWriter, TeradataBlobWriter>();
	case LogicalTypeId::DECIMAL: {
		const auto width = DecimalType::GetWidth(type);
		if (width <= 2) {
			// In teradata, the small decimals are stored as 1 byte, compared to 2 bytes in duckdb
			return make_uniq_base<TeradataColumnWriter, TeradataFixedSizeWriter<int16_t, int8_t>>();
		}
		if (width <= 4) {
			return make_uniq_base<TeradataColumnWriter, TeradataFixedSizeWriter<int16_t>>();
		}
		if (width <= 9) {
			return make_uniq_base<TeradataColumnWriter, TeradataFixedSizeWriter<int32_t>>();
		}
		if (width <= 18) {
			return make_uniq_base<TeradataColumnWriter, TeradataFixedSizeWriter<int64_t>>();
		}
		if (width <= 38) {
			return make_uniq_base<TeradataColumnWriter, TeradataFixedSizeWriter<hugeint_t>>();
		}
		throw InvalidInputException("Invalid Teradata decimal width: %d", width);
	}
	case LogicalTypeId::DATE:
		return make_uniq_base<TeradataColumnWriter, TeradataDateWriter>();
	case LogicalTypeId::TIME:
		return make_uniq_base<TeradataColumnWriter, TeradataTimeWriter>();
	case LogicalTypeId::TIMESTAMP_SEC:
		return make_uniq_base<TeradataColumnWriter, TeradataTimestampWriter>(type.id());
	case LogicalTypeId::TIMESTAMP_MS:
		return make_uniq_base<TeradataColumnWriter, TeradataTimestampWriter>(type.id());
	case LogicalTypeId::TIMESTAMP:
		return make_uniq_base<TeradataColumnWriter, TeradataTimestampWriter>(type.id());
	case LogicalTypeId::TIMESTAMP_TZ:
		return make_uniq_base<TeradataColumnWriter, TeradataTimestampWriter>(type.id());
	default:
		throw NotImplementedException("TeradataColumnWriter::Make: type %s not supported", type.ToString());
	}
}

} // namespace duckdb