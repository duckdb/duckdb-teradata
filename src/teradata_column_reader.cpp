#include "teradata_column_reader.hpp"
#include "teradata_type.hpp"

#include "util/binary_reader.hpp"

namespace duckdb {

class TeradataVarcharReader final : public TeradataColumnReader {
public:
	void Decode(BinaryReader &reader, Vector &vec, idx_t row_idx, bool is_null) override {
		const auto length = reader.Read<uint16_t>();
		if (is_null) {
			FlatVector::SetNull(vec, row_idx, true);
		} else {
			const auto src_ptr = reader.ReadBytes(length);
			const auto dst_ptr = FlatVector::GetData<string_t>(vec);
			dst_ptr[row_idx] = StringVector::AddString(vec, src_ptr, length);
		}
	}

private:
	// TODO: Deal with character encoding/conversion here
	vector<char> encoding_buffer;
};

class TeradataCharReader final : public TeradataColumnReader {
public:
	explicit TeradataCharReader(int32_t max_size_p) : max_size(max_size_p) {
	}

	void Decode(BinaryReader &reader, Vector &vec, idx_t row_idx, bool is_null) override {
		if (is_null) {
			FlatVector::SetNull(vec, row_idx, true);
			reader.Skip(max_size);
		} else {
			const auto src_ptr = reader.ReadBytes(max_size);
			const auto dst_ptr = FlatVector::GetData<string_t>(vec);
			dst_ptr[row_idx] = StringVector::AddString(vec, src_ptr, max_size);
		}
	}

private:
	idx_t max_size;

	// TODO: Deal with character encoding/conversion here
	vector<char> encoding_buffer;
};

class TeradataDateReader final : public TeradataColumnReader {
public:
	void Decode(BinaryReader &reader, Vector &vec, idx_t row_idx, bool is_null) override {
		if (is_null) {
			FlatVector::SetNull(vec, row_idx, true);
		} else {
			// Teradata stores dates in the following format
			// (YEAR - 1900) * 10000 + (MONTH * 100) + DAY
			const auto td_date = reader.Read<int32_t>();

			const auto years = td_date / 10000 + 1900;
			const auto months = (td_date % 10000) / 100;
			const auto days = td_date % 100;

			FlatVector::GetData<date_t>(vec)[row_idx] = Date::FromDate(years, months, days);
		}
	}
};

class TeradataByteReader final : public TeradataColumnReader {
public:
	explicit TeradataByteReader(int32_t max_size_p) : max_size(max_size_p) {
	}

	void Decode(BinaryReader &reader, Vector &vec, idx_t row_idx, bool is_null) override {
		D_ASSERT(vec.GetType().id() == LogicalTypeId::BLOB);

		if (is_null) {
			FlatVector::SetNull(vec, row_idx, true);
			reader.Skip(max_size);
		} else {
			const auto src_ptr = reader.ReadBytes(max_size);
			const auto dst_ptr = FlatVector::GetData<string_t>(vec);
			dst_ptr[row_idx] = StringVector::AddStringOrBlob(vec, src_ptr, max_size);
		}
	}

private:
	idx_t max_size;
};

class TeradataVarbyteReader final : public TeradataColumnReader {
public:
	void Decode(BinaryReader &reader, Vector &vec, idx_t row_idx, bool is_null) override {
		D_ASSERT(vec.GetType().id() == LogicalTypeId::BLOB);

		const auto length = reader.Read<uint16_t>();
		if (is_null) {
			FlatVector::SetNull(vec, row_idx, true);
		} else {
			const auto src_ptr = reader.ReadBytes(length);
			const auto dst_ptr = FlatVector::GetData<string_t>(vec);
			dst_ptr[row_idx] = StringVector::AddStringOrBlob(vec, src_ptr, length);
		}
	}
};

template <class SRC, class DST = SRC>
class TeradataFixedSizeReader final : public TeradataColumnReader {
public:
	void Decode(BinaryReader &reader, Vector &vec, idx_t row_idx, bool is_null) override {
		if (is_null) {
			FlatVector::SetNull(vec, row_idx, true);
			reader.Skip(sizeof(SRC));
		} else {
			const auto src = reader.Read<SRC>();
			const auto dst = FlatVector::GetData<DST>(vec);
			dst[row_idx] = src;
		}
	}
};

class TeradataIntervalYearReader final : public TeradataColumnReader {
public:
	static constexpr auto BUFFER_SIZE = 6;

	idx_t char_size = 0;

	explicit TeradataIntervalYearReader(idx_t precision_p) : char_size(precision_p + 1) {
		D_ASSERT(precision_p < 5);
	}

	void Decode(BinaryReader &reader, Vector &vec, idx_t row_idx, bool is_null) override {
		if (is_null) {
			FlatVector::SetNull(vec, row_idx, true);
			reader.Skip(char_size);
		} else {

			char buffer[BUFFER_SIZE] = {0};
			reader.ReadInto(buffer, char_size);

			int32_t years = 0;

			// Scan into years
			sscanf(buffer, "%d", &years);

			interval_t interval = {};
			interval.months = years * Interval::MONTHS_PER_YEAR;

			const auto dst = FlatVector::GetData<interval_t>(vec);
			dst[row_idx] = interval;
		}
	}
};

class TeradataIntervalYearToMonthReader final : public TeradataColumnReader {
public:
	static constexpr auto BUFFER_SIZE = 6;

	idx_t char_size = 0;

	explicit TeradataIntervalYearToMonthReader(idx_t precision_p) : char_size(precision_p + 1) {
		D_ASSERT(precision_p < BUFFER_SIZE - 1);
	}
	void Decode(BinaryReader &reader, Vector &vec, idx_t row_idx, bool is_null) override {
		if (is_null) {
			FlatVector::SetNull(vec, row_idx, true);
			reader.Skip(char_size);
		} else {

			char buffer[BUFFER_SIZE] = {0};
			reader.ReadInto(buffer, char_size);

			int32_t years = 0;
			int32_t months = 0;

			// Scan into years and months
			sscanf(buffer, "%d-%d", &years, &months);

			interval_t interval = {};
			interval.months = years * Interval::MONTHS_PER_YEAR + months;

			const auto dst = FlatVector::GetData<interval_t>(vec);
			dst[row_idx] = interval;
		}
	}
};

class TeradataIntervalMonthReader final : public TeradataColumnReader {
public:
	static constexpr auto BUFFER_SIZE = 6;
	idx_t char_size = 0;

	explicit TeradataIntervalMonthReader(idx_t precision_p) : char_size(precision_p + 1) {
		D_ASSERT(precision_p < BUFFER_SIZE - 1);
	}

	void Decode(BinaryReader &reader, Vector &vec, idx_t row_idx, bool is_null) override {
		if (is_null) {
			FlatVector::SetNull(vec, row_idx, true);
			reader.Skip(char_size);
		} else {

			char buffer[BUFFER_SIZE] = {0};
			reader.ReadInto(buffer, char_size);

			int32_t months = 0;

			// Scan into months
			sscanf(buffer, "%d", &months);

			interval_t interval = {};
			interval.months = months;

			const auto dst = FlatVector::GetData<interval_t>(vec);
			dst[row_idx] = interval;
		}
	}
};

class TeradataIntervalDayReader final : public TeradataColumnReader {
public:
	idx_t char_size = 0;

	explicit TeradataIntervalDayReader(idx_t precision_p) : char_size(precision_p + 1) {
		D_ASSERT(precision_p < 5);
	}

	void Decode(BinaryReader &reader, Vector &vec, idx_t row_idx, bool is_null) override {
		if (is_null) {
			FlatVector::SetNull(vec, row_idx, true);
			reader.Skip(char_size);
		} else {

			char buffer[6] = {0};
			reader.ReadInto(buffer, char_size);

			int32_t days = 0;

			// Scan into days
			sscanf(buffer, "%d", &days);

			interval_t interval = {};
			interval.days = days;

			const auto dst = FlatVector::GetData<interval_t>(vec);
			dst[row_idx] = interval;
		}
	}
};

class TeradataIntervalDayToHourReader final : public TeradataColumnReader {
public:
	idx_t char_size = 0;

	explicit TeradataIntervalDayToHourReader(idx_t precision_p) : char_size(precision_p + 1) {
		D_ASSERT(precision_p < 5);
	}

	void Decode(BinaryReader &reader, Vector &vec, idx_t row_idx, bool is_null) override {
		if (is_null) {
			FlatVector::SetNull(vec, row_idx, true);
			reader.Skip(char_size);
		} else {

			char buffer[6] = {0};
			reader.ReadInto(buffer, char_size);

			int32_t days = 0;
			int32_t hours = 0;

			// Scan into days and hours
			sscanf(buffer, "%d %d", &days, &hours);

			// Convert all to micros
			const auto micros = hours * Interval::MICROS_PER_HOUR + days * Interval::MICROS_PER_DAY;
			const auto interval = Interval::FromMicro(micros);

			const auto dst = FlatVector::GetData<interval_t>(vec);
			dst[row_idx] = interval;
		}
	}
};

class TeradataIntervalDayToMinuteReader final : public TeradataColumnReader {
public:
	idx_t char_size = 0;

	explicit TeradataIntervalDayToMinuteReader(idx_t precision_p) : char_size(precision_p + 1) {
		D_ASSERT(precision_p < 5);
	}

	void Decode(BinaryReader &reader, Vector &vec, idx_t row_idx, bool is_null) override {
		if (is_null) {
			FlatVector::SetNull(vec, row_idx, true);
			reader.Skip(char_size);
		} else {

			char buffer[6] = {0};
			reader.ReadInto(buffer, char_size);

			int32_t days = 0;
			int32_t hours = 0;
			int32_t minutes = 0;

			// Scan into days, hours and minutes
			sscanf(buffer, "%d %d:%d", &days, &hours, &minutes);

			const auto micros = hours * Interval::MICROS_PER_HOUR + days * Interval::MICROS_PER_DAY +
			                    minutes * Interval::MICROS_PER_MINUTE;
			const auto interval = Interval::FromMicro(micros);

			const auto dst = FlatVector::GetData<interval_t>(vec);
			dst[row_idx] = interval;
		}
	}
};

class TeradataIntervalDayToSecondReader final : public TeradataColumnReader {
public:
	idx_t char_size = 0;
	idx_t precision = 0;
	idx_t second_precision = 0;

	explicit TeradataIntervalDayToSecondReader(idx_t precision, idx_t second_precision)
	    : precision(precision), second_precision(second_precision) {
		D_ASSERT(precision < 5);
		D_ASSERT(second_precision < 7);
		if (second_precision != 0) {
			char_size = precision + second_precision + 11;
		} else {
			char_size = precision + 10;
		}
	}

	void Decode(BinaryReader &reader, Vector &vec, idx_t row_idx, bool is_null) override {
		if (is_null) {
			FlatVector::SetNull(vec, row_idx, true);
			reader.Skip(char_size);
		} else {
			char buffer[32] = {0};
			reader.ReadInto(buffer, char_size);

			int32_t days = 0;
			int32_t hours = 0;
			int32_t minutes = 0;
			int32_t seconds = 0;
			int32_t fractional_seconds = 0;

			if (second_precision != 0) {
				// Scan into days, hours, minutes, seconds and fractional seconds
				sscanf(buffer, "%d %d:%d:%d.%d", &days, &hours, &minutes, &seconds, &fractional_seconds);
			} else {
				// Scan into days, hours, minutes and seconds
				sscanf(buffer, "%d %d:%d:%d", &days, &hours, &minutes, &seconds);
			}

			auto micros = hours * Interval::MICROS_PER_HOUR + days * Interval::MICROS_PER_DAY +
			              minutes * Interval::MICROS_PER_MINUTE + seconds * Interval::MICROS_PER_SEC;

			// Also add fractional seconds if present
			if (second_precision != 0) {
				micros += UnsafeNumericCast<int64_t>(fractional_seconds) * Interval::MICROS_PER_SEC /
				          static_cast<int64_t>(pow(10, second_precision));
			}

			const auto interval = Interval::FromMicro(micros);
			const auto dst = FlatVector::GetData<interval_t>(vec);
			dst[row_idx] = interval;
		}
	}
};

class TeradataIntervalHourReader final : public TeradataColumnReader {
public:
	idx_t char_size = 0;

	explicit TeradataIntervalHourReader(idx_t precision) : char_size(precision + 1) {
		D_ASSERT(precision < 5);
	}

	void Decode(BinaryReader &reader, Vector &vec, idx_t row_idx, bool is_null) override {
		if (is_null) {
			FlatVector::SetNull(vec, row_idx, true);
			reader.Skip(char_size);
		} else {
			char buffer[6] = {0};
			reader.ReadInto(buffer, char_size);

			int32_t hours = 0;

			// Scan into hours
			sscanf(buffer, "%d", &hours);

			interval_t interval = {};
			interval.micros = hours * Interval::MICROS_PER_HOUR;

			const auto dst = FlatVector::GetData<interval_t>(vec);
			dst[row_idx] = interval;
		}
	}
};

class TeradataIntervalHourToMinuteReader final : public TeradataColumnReader {
public:
	idx_t char_size = 0;

	explicit TeradataIntervalHourToMinuteReader(idx_t precision) : char_size(precision + 1) {
		D_ASSERT(precision < 5);
	}

	void Decode(BinaryReader &reader, Vector &vec, idx_t row_idx, bool is_null) override {
		if (is_null) {
			FlatVector::SetNull(vec, row_idx, true);
			reader.Skip(char_size);
		} else {
			char buffer[6] = {0};
			reader.ReadInto(buffer, char_size);

			int32_t hours = 0;
			int32_t minutes = 0;

			// Scan into hours and minutes
			sscanf(buffer, "%d:%d", &hours, &minutes);

			interval_t interval = {};
			interval.micros = hours * Interval::MICROS_PER_HOUR + minutes * Interval::MICROS_PER_MINUTE;

			const auto dst = FlatVector::GetData<interval_t>(vec);
			dst[row_idx] = interval;
		}
	}
};

class TeradataIntervalHourToSecondReader final : public TeradataColumnReader {
public:
	idx_t char_size = 0;
	idx_t precision = 0;
	idx_t second_precision = 0;

	explicit TeradataIntervalHourToSecondReader(idx_t precision, idx_t second_precision)
	    : precision(precision), second_precision(second_precision) {
		D_ASSERT(precision < 5);
		D_ASSERT(second_precision < 7);
		if (second_precision != 0) {
			char_size = precision + second_precision + 8;
		} else {
			char_size = precision + 7;
		}
	}

	void Decode(BinaryReader &reader, Vector &vec, idx_t row_idx, bool is_null) override {
		if (is_null) {
			FlatVector::SetNull(vec, row_idx, true);
			reader.Skip(char_size);
		} else {
			char buffer[32] = {0};
			reader.ReadInto(buffer, char_size);

			int32_t hours = 0;
			int32_t minutes = 0;
			int32_t seconds = 0;
			int32_t fractional_seconds = 0;

			if (second_precision != 0) {
				// Scan into hours, minutes, seconds and fractional seconds
				sscanf(buffer, "%d:%d:%d.%d", &hours, &minutes, &seconds, &fractional_seconds);
			} else {
				// Scan into hours, minutes and seconds
				sscanf(buffer, "%d:%d:%d", &hours, &minutes, &seconds);
			}

			auto micros = hours * Interval::MICROS_PER_HOUR + minutes * Interval::MICROS_PER_MINUTE +
			              seconds * Interval::MICROS_PER_SEC;

			// Also add fractional seconds if present
			if (second_precision != 0) {
				micros += UnsafeNumericCast<int64_t>(fractional_seconds) * Interval::MICROS_PER_SEC /
				          static_cast<int64_t>(pow(10, second_precision));
			}

			const auto interval = Interval::FromMicro(micros);
			const auto dst = FlatVector::GetData<interval_t>(vec);
			dst[row_idx] = interval;
		}
	}
};

class TeradataIntervalMinuteReader final : public TeradataColumnReader {
public:
	idx_t char_size = 0;
	explicit TeradataIntervalMinuteReader(idx_t precision) : char_size(precision + 1) {
		D_ASSERT(precision < 5);
	}
	void Decode(BinaryReader &reader, Vector &vec, idx_t row_idx, bool is_null) override {
		if (is_null) {
			FlatVector::SetNull(vec, row_idx, true);
			reader.Skip(char_size);
		} else {
			char buffer[6] = {0};
			reader.ReadInto(buffer, char_size);

			int32_t minutes = 0;

			// Scan into minutes
			sscanf(buffer, "%d", &minutes);

			interval_t interval = {};
			interval.micros = minutes * Interval::MICROS_PER_MINUTE;

			const auto dst = FlatVector::GetData<interval_t>(vec);
			dst[row_idx] = interval;
		}
	}
};

class TeradataIntervalMinuteToSecondReader final : public TeradataColumnReader {
public:
	idx_t char_size = 0;
	idx_t precision = 0;
	idx_t second_precision = 0;

	explicit TeradataIntervalMinuteToSecondReader(idx_t precision, idx_t second_precision)
	    : precision(precision), second_precision(second_precision) {
		D_ASSERT(precision < 5);
		D_ASSERT(second_precision < 7);
		if (second_precision != 0) {
			char_size = precision + second_precision + 4;
		} else {
			char_size = precision + 3;
		}
	}

	void Decode(BinaryReader &reader, Vector &vec, idx_t row_idx, bool is_null) override {
		if (is_null) {
			FlatVector::SetNull(vec, row_idx, true);
			reader.Skip(char_size);
		} else {
			char buffer[32] = {0};
			reader.ReadInto(buffer, char_size);

			int32_t minutes = 0;
			int32_t seconds = 0;
			int32_t fractional_seconds = 0;

			if (second_precision != 0) {
				// Scan into minutes, seconds and fractional seconds
				sscanf(buffer, "%d:%d.%d", &minutes, &seconds, &fractional_seconds);
			} else {
				// Scan into minutes and seconds
				sscanf(buffer, "%d:%d", &minutes, &seconds);
			}

			auto micros = minutes * Interval::MICROS_PER_MINUTE + seconds * Interval::MICROS_PER_SEC;

			// Also add fractional seconds if present
			if (second_precision != 0) {
				micros += UnsafeNumericCast<int64_t>(fractional_seconds) * Interval::MICROS_PER_SEC /
				          static_cast<int64_t>(pow(10, second_precision));
			}

			const auto interval = Interval::FromMicro(micros);
			const auto dst = FlatVector::GetData<interval_t>(vec);
			dst[row_idx] = interval;
		}
	}
};

class TeradataIntervalSecondReader final : public TeradataColumnReader {
public:
	idx_t char_size = 0;
	idx_t precision = 0;
	idx_t second_precision = 0;

	explicit TeradataIntervalSecondReader(idx_t precision, idx_t second_precision)
	    : precision(precision), second_precision(second_precision) {
		D_ASSERT(precision < 5);
		if (second_precision != 0) {
			char_size = precision + second_precision + 2;
		} else {
			char_size = precision + 1;
		}
	}

	void Decode(BinaryReader &reader, Vector &vec, idx_t row_idx, bool is_null) override {
		if (is_null) {
			FlatVector::SetNull(vec, row_idx, true);
			reader.Skip(char_size);
		} else {
			char buffer[32] = {0};
			reader.ReadInto(buffer, char_size);

			int32_t seconds = 0;
			int32_t fractional_seconds = 0;

			if (second_precision != 0) {
				// Scan into seconds and fractional seconds
				sscanf(buffer, "%d.%d", &seconds, &fractional_seconds);
			} else {
				// Scan into seconds
				sscanf(buffer, "%d", &seconds);
			}

			auto micros = seconds * Interval::MICROS_PER_SEC;

			// Also add fractional seconds if present
			if (second_precision != 0) {
				micros += UnsafeNumericCast<int64_t>(fractional_seconds) * Interval::MICROS_PER_SEC /
				          static_cast<int64_t>(pow(10, second_precision));
			}

			const auto interval = Interval::FromMicro(micros);
			const auto dst = FlatVector::GetData<interval_t>(vec);
			dst[row_idx] = interval;
		}
	}
};

//----------------------------------------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------------------------------------

unique_ptr<TeradataColumnReader> TeradataColumnReader::Make(const TeradataType &type) {
	switch (type.GetId()) {
	case TeradataTypeId::VARCHAR:
		return make_uniq<TeradataVarcharReader>();
	case TeradataTypeId::CHAR:
		return make_uniq<TeradataCharReader>(type.GetLength());
	case TeradataTypeId::BYTE:
		return make_uniq<TeradataByteReader>(type.GetLength());
	case TeradataTypeId::VARBYTE:
		return make_uniq<TeradataVarbyteReader>();
	case TeradataTypeId::DATE:
		return make_uniq<TeradataDateReader>();
	case TeradataTypeId::BYTEINT:
		return make_uniq<TeradataFixedSizeReader<int8_t>>();
	case TeradataTypeId::SMALLINT:
		return make_uniq<TeradataFixedSizeReader<int16_t>>();
	case TeradataTypeId::INTEGER:
		return make_uniq<TeradataFixedSizeReader<int32_t>>();
	case TeradataTypeId::BIGINT:
		return make_uniq<TeradataFixedSizeReader<int64_t>>();
	case TeradataTypeId::FLOAT:
		return make_uniq<TeradataFixedSizeReader<double>>();
	case TeradataTypeId::DECIMAL: {
		const auto width = type.GetWidth();
		if (width <= 2) {
			// In teradata, the small decimals are stored as 1 byte, compared to 2 bytes in duckdb
			return make_uniq<TeradataFixedSizeReader<int8_t, int16_t>>();
		}
		if (width <= 4) {
			return make_uniq<TeradataFixedSizeReader<int16_t>>();
		}
		if (width <= 9) {
			return make_uniq<TeradataFixedSizeReader<int32_t>>();
		}
		if (width <= 18) {
			return make_uniq<TeradataFixedSizeReader<int64_t>>();
		}
		if (width <= 38) {
			return make_uniq<TeradataFixedSizeReader<hugeint_t>>();
		}
		throw InvalidInputException("Invalid Teradata decimal width: %d", width);
	}
	case TeradataTypeId::INTERVAL_YEAR:
		return make_uniq<TeradataIntervalYearReader>(type.GetWidth());
	case TeradataTypeId::INTERVAL_YEAR_TO_MONTH:
		return make_uniq<TeradataIntervalYearToMonthReader>(type.GetWidth());
	case TeradataTypeId::INTERVAL_MONTH:
		return make_uniq<TeradataIntervalMonthReader>(type.GetWidth());
	case TeradataTypeId::INTERVAL_DAY:
		return make_uniq<TeradataIntervalDayReader>(type.GetWidth());
	case TeradataTypeId::INTERVAL_DAY_TO_HOUR:
		return make_uniq<TeradataIntervalDayToHourReader>(type.GetWidth());
	case TeradataTypeId::INTERVAL_DAY_TO_MINUTE:
		return make_uniq<TeradataIntervalDayToMinuteReader>(type.GetWidth());
	case TeradataTypeId::INTERVAL_DAY_TO_SECOND:
		return make_uniq<TeradataIntervalDayToSecondReader>(type.GetWidth(), type.GetScale());
	case TeradataTypeId::INTERVAL_HOUR:
		return make_uniq<TeradataIntervalHourReader>(type.GetWidth());
	case TeradataTypeId::INTERVAL_HOUR_TO_MINUTE:
		return make_uniq<TeradataIntervalHourToMinuteReader>(type.GetWidth());
	case TeradataTypeId::INTERVAL_HOUR_TO_SECOND:
		return make_uniq<TeradataIntervalHourToSecondReader>(type.GetWidth(), type.GetScale());
	case TeradataTypeId::INTERVAL_MINUTE:
		return make_uniq<TeradataIntervalMinuteReader>(type.GetWidth());
	case TeradataTypeId::INTERVAL_MINUTE_TO_SECOND:
		return make_uniq<TeradataIntervalMinuteToSecondReader>(type.GetWidth(), type.GetScale());
	case TeradataTypeId::INTERVAL_SECOND:
		return make_uniq<TeradataIntervalSecondReader>(type.GetWidth(), type.GetScale());
	default:
		throw NotImplementedException("Teradata type reader for '%s' not implemented", type.ToString());
	}
}

struct IntervalYearOp {
	static void Parse(BinaryReader &reader, interval_t &result) {
		int32_t years = 0;
		sscanf(reader.ReadBytes(4), "%d", &years);
	}
};

} // namespace duckdb