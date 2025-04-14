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
	default:
		throw NotImplementedException("Teradata type reader for '%s' not implemented", type.ToString());
	}
}

} // namespace duckdb