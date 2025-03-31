#include "teradata_request.hpp"
#include "teradata_type.hpp"
#include "teradata_connection.hpp"

#include "util/binary_reader.hpp"

namespace duckdb {

TeradataRequestContext::TeradataRequestContext(const TeradataConnection &con) {
	Init(con);
}

void TeradataRequestContext::Init(const TeradataConnection &con) {
	// Initialize
	dbc.total_len = sizeof(DBCAREA);

	int32_t result = EM_OK;
	DBCHINI(&result, cnta, &dbc);
	if (result != EM_OK) {
		throw IOException("Failed to initialize DBCAREA: %s", string(dbc.msg_text, dbc.msg_len));
	}
	dbc.i_sess_id = con.GetSessionId();
	buffer.resize(1024);
}

void TeradataRequestContext::Execute(const string &sql) {
	BeginRequest(sql, 'E');

	MatchParcel(PclENDSTATEMENT);

	EndRequest();
}

void TeradataRequestContext::Execute(const string &sql, DataChunk &chunk, ArenaAllocator &arena) {

	const auto row_count = chunk.size();
	const auto col_count = chunk.ColumnCount();

	D_ASSERT(row_count > 0);

	// Setup unified vector formats for each column
	vector<UnifiedVectorFormat> formats;
	formats.resize(chunk.ColumnCount());

	for (idx_t col_idx = 0; col_idx < col_count; col_idx++) {
		auto &col_vec = chunk.data[col_idx];
		auto &format = formats[col_idx];
		col_vec.ToUnifiedFormat(row_count, format);
	}

	// Allocate space for the row pointers and lengths
	const auto records_ptr = reinterpret_cast<char **>(arena.AllocateAligned(row_count * sizeof(char *)));
	const auto lengths_ptr = reinterpret_cast<int32_t *>(arena.AllocateAligned(row_count * sizeof(int32_t)));

	// Compute the size of the rows
	for (idx_t out_idx = 0; out_idx < row_count; out_idx++) {
		int32_t row_length = 0;

		for (idx_t col_idx = 0; col_idx < col_count; col_idx++) {
			auto &format = formats[col_idx];

			const auto row_idx = format.sel->get_index(out_idx);
			if (!format.validity.RowIsValid(row_idx)) {
				// TODO: use presence bits, write null
				continue;
			}

			auto &col_type = chunk.data[col_idx].GetType();
			const auto is_varlen = col_type.InternalType() == PhysicalType::VARCHAR;

			if (!is_varlen) {
				// Fixed length
				const auto col_length = GetTypeIdSize(col_type.InternalType());
				const auto new_length = row_length + col_length;

				if (new_length > NumericLimits<int32_t>::Maximum()) {
					throw InvalidInputException("Row too long for Teradata");
				}

				row_length += UnsafeNumericCast<int32_t>(col_length);

			} else {
				// Variable length
				const auto str_length = UnifiedVectorFormat::GetData<string_t>(format)[row_idx].GetSize();
				if (str_length > TeradataType::MAX_TYPE_LENGTH) {
					throw InvalidInputException("String too long for Teradata: %d", str_length);
				}

				// Add the length of the string + the length of the length prefix
				const auto new_length = row_length + sizeof(uint16_t) + str_length;
				if (new_length > NumericLimits<int32_t>::Maximum()) {
					throw InvalidInputException("Row too long for Teradata");
				}

				row_length += UnsafeNumericCast<int32_t>(sizeof(uint16_t) + str_length);
			}
		}

		// Allocate space for the row
		const auto row_record = reinterpret_cast<char *>(arena.Allocate(row_length));

		// Set the row pointer & length
		records_ptr[out_idx] = row_record;
		lengths_ptr[out_idx] = row_length;
	}

	// Now actually write the rows
	for (idx_t out_idx = 0; out_idx < row_count; out_idx++) {

		auto row_record = records_ptr[out_idx];

		for (idx_t col_idx = 0; col_idx < col_count; col_idx++) {
			auto &format = formats[col_idx];

			const auto row_idx = format.sel->get_index(out_idx);
			if (!format.validity.RowIsValid(row_idx)) {
				// TODO: use presence bits, write null
				continue;
			}

			auto &col_type = chunk.data[col_idx].GetType();
			const auto is_varlen = col_type.InternalType() == PhysicalType::VARCHAR;

			if (!is_varlen) {
				// Fixed length
				const auto col_data = format.data;
				const auto col_size = GetTypeIdSize(col_type.InternalType());
				memcpy(row_record, col_data + col_size * row_idx, col_size);

				// Increment pointer
				row_record += col_size;

			} else {
				// Variable length
				const auto &string_data = UnifiedVectorFormat::GetData<string_t>(format)[row_idx];
				const auto string_size = string_data.GetSize();

				// Copy length
				memcpy(row_record, &string_size, sizeof(uint16_t));

				// Increment pointer
				row_record += sizeof(uint16_t);

				// Copy data
				memcpy(row_record, string_data.GetDataUnsafe(), string_size);

				// Increment pointer
				row_record += string_size;
			}
		}
	}

	dbc.using_data_ptr_array = reinterpret_cast<char *>(records_ptr);
	dbc.using_data_len_array = lengths_ptr;
	dbc.using_data_count = chunk.size();

	BeginRequest(sql, 'E');

	MatchParcel(PclENDSTATEMENT);

	for (idx_t i = 1; i < chunk.size(); i++) {
		MatchParcel(PclSUCCESS);
		MatchParcel(PclENDSTATEMENT);
	}

	EndRequest();
}

enum class TeradataColumnTypeVariant { STANDARD, NULLABLE, PARAM_IN, PARAM_INOUT, PARAM_OUT };

struct TeradataColumnType {
	TeradataType type;
	TeradataColumnTypeVariant variant;
};

static TeradataColumnType GetTeradataTypeFromParcel(const PclInt16 type) {
	// Nullable is 1 + the standard type
	// Param in is 500 + the standard type
	// Param inout is 501 + the standard type
	// Param out is 502 + the standard type
	switch (type) {
	case 400:
		return {TeradataTypeId::BLOB, TeradataColumnTypeVariant::STANDARD};
	case 448:
		return {TeradataTypeId::VARCHAR, TeradataColumnTypeVariant::STANDARD};
	case 449:
		return {TeradataTypeId::VARCHAR, TeradataColumnTypeVariant::NULLABLE};
	case 948:
		return {TeradataTypeId::VARCHAR, TeradataColumnTypeVariant::PARAM_IN};
	case 949:
		return {TeradataTypeId::VARCHAR, TeradataColumnTypeVariant::PARAM_INOUT};
	case 950:
		return {TeradataTypeId::VARCHAR, TeradataColumnTypeVariant::PARAM_OUT};

	case 452:
		return {TeradataTypeId::CHAR, TeradataColumnTypeVariant::STANDARD};
	case 453:
		return {TeradataTypeId::CHAR, TeradataColumnTypeVariant::NULLABLE};
	case 952:
		return {TeradataTypeId::CHAR, TeradataColumnTypeVariant::PARAM_IN};
	case 953:
		return {TeradataTypeId::CHAR, TeradataColumnTypeVariant::PARAM_INOUT};
	case 954:
		return {TeradataTypeId::CHAR, TeradataColumnTypeVariant::PARAM_OUT};

	// LONGVARCHAR
	case 456:
		return {TeradataTypeId::VARCHAR, TeradataColumnTypeVariant::STANDARD};
	case 457:
		return {TeradataTypeId::VARCHAR, TeradataColumnTypeVariant::NULLABLE};
	case 956:
		return {TeradataTypeId::VARCHAR, TeradataColumnTypeVariant::PARAM_IN};
	case 957:
		return {TeradataTypeId::VARCHAR, TeradataColumnTypeVariant::PARAM_INOUT};
	case 958:
		return {TeradataTypeId::VARCHAR, TeradataColumnTypeVariant::PARAM_OUT};

	case 472:
		return {TeradataTypeId::LONGVARGRAPHIC, TeradataColumnTypeVariant::STANDARD};
	case 473:
		return {TeradataTypeId::LONGVARGRAPHIC, TeradataColumnTypeVariant::NULLABLE};
	case 972:
		return {TeradataTypeId::LONGVARGRAPHIC, TeradataColumnTypeVariant::PARAM_IN};
	case 973:
		return {TeradataTypeId::LONGVARGRAPHIC, TeradataColumnTypeVariant::PARAM_INOUT};
	case 974:
		return {TeradataTypeId::LONGVARGRAPHIC, TeradataColumnTypeVariant::PARAM_OUT};

	case 480:
		return {TeradataTypeId::FLOAT, TeradataColumnTypeVariant::STANDARD};
	case 481:
		return {TeradataTypeId::FLOAT, TeradataColumnTypeVariant::NULLABLE};
	case 980:
		return {TeradataTypeId::FLOAT, TeradataColumnTypeVariant::PARAM_IN};
	case 981:
		return {TeradataTypeId::FLOAT, TeradataColumnTypeVariant::PARAM_INOUT};
	case 982:
		return {TeradataTypeId::FLOAT, TeradataColumnTypeVariant::PARAM_OUT};

	case 496:
		return {TeradataTypeId::INTEGER, TeradataColumnTypeVariant::STANDARD};
	case 497:
		return {TeradataTypeId::INTEGER, TeradataColumnTypeVariant::NULLABLE};
	case 996:
		return {TeradataTypeId::INTEGER, TeradataColumnTypeVariant::PARAM_IN};
	case 997:
		return {TeradataTypeId::INTEGER, TeradataColumnTypeVariant::PARAM_INOUT};
	case 998:
		return {TeradataTypeId::INTEGER, TeradataColumnTypeVariant::PARAM_OUT};

	case 500:
		return {TeradataTypeId::SMALLINT, TeradataColumnTypeVariant::STANDARD};
	case 501:
		return {TeradataTypeId::SMALLINT, TeradataColumnTypeVariant::NULLABLE};
	case 1000:
		return {TeradataTypeId::SMALLINT, TeradataColumnTypeVariant::PARAM_IN};
	case 1001:
		return {TeradataTypeId::SMALLINT, TeradataColumnTypeVariant::PARAM_INOUT};
	case 1002:
		return {TeradataTypeId::SMALLINT, TeradataColumnTypeVariant::PARAM_OUT};

	case 600:
		return {TeradataTypeId::BIGINT, TeradataColumnTypeVariant::STANDARD};
	case 601:
		return {TeradataTypeId::BIGINT, TeradataColumnTypeVariant::NULLABLE};
	case 1100:
		return {TeradataTypeId::BIGINT, TeradataColumnTypeVariant::PARAM_IN};
	case 1101:
		return {TeradataTypeId::BIGINT, TeradataColumnTypeVariant::PARAM_INOUT};
	case 1102:
		return {TeradataTypeId::BIGINT, TeradataColumnTypeVariant::PARAM_OUT};

	case 688:
		return {TeradataTypeId::VARBYTE, TeradataColumnTypeVariant::STANDARD};
	case 689:
		return {TeradataTypeId::VARBYTE, TeradataColumnTypeVariant::NULLABLE};
	case 1188:
		return {TeradataTypeId::VARBYTE, TeradataColumnTypeVariant::PARAM_IN};
	case 1189:
		return {TeradataTypeId::VARBYTE, TeradataColumnTypeVariant::PARAM_INOUT};
	case 1190:
		return {TeradataTypeId::VARBYTE, TeradataColumnTypeVariant::PARAM_OUT};

	case 692:
		return {TeradataTypeId::BYTE, TeradataColumnTypeVariant::STANDARD};
	case 693:
		return {TeradataTypeId::BYTE, TeradataColumnTypeVariant::NULLABLE};
	case 1192:
		return {TeradataTypeId::BYTE, TeradataColumnTypeVariant::PARAM_IN};
	case 1193:
		return {TeradataTypeId::BYTE, TeradataColumnTypeVariant::PARAM_INOUT};
	case 1194:
		return {TeradataTypeId::BYTE, TeradataColumnTypeVariant::PARAM_OUT};

	case 697:
		return {TeradataTypeId::LONGVARBYTE, TeradataColumnTypeVariant::STANDARD};
	case 698:
		return {TeradataTypeId::LONGVARBYTE, TeradataColumnTypeVariant::NULLABLE};
	case 1197:
		return {TeradataTypeId::LONGVARBYTE, TeradataColumnTypeVariant::PARAM_IN};
	case 1198:
		return {TeradataTypeId::LONGVARBYTE, TeradataColumnTypeVariant::PARAM_INOUT};
	case 1199:
		return {TeradataTypeId::LONGVARBYTE, TeradataColumnTypeVariant::PARAM_OUT};

	case 748:
		return {TeradataTypeId::DATE_A, TeradataColumnTypeVariant::STANDARD};
	case 749:
		return {TeradataTypeId::DATE_A, TeradataColumnTypeVariant::NULLABLE};
	case 1248:
		return {TeradataTypeId::DATE_A, TeradataColumnTypeVariant::PARAM_IN};
	case 1249:
		return {TeradataTypeId::DATE_A, TeradataColumnTypeVariant::PARAM_INOUT};
	case 1250:
		return {TeradataTypeId::DATE_A, TeradataColumnTypeVariant::PARAM_OUT};

	case 752:
		return {TeradataTypeId::DATE_T, TeradataColumnTypeVariant::STANDARD};
	case 753:
		return {TeradataTypeId::DATE_T, TeradataColumnTypeVariant::NULLABLE};
	case 1252:
		return {TeradataTypeId::DATE_T, TeradataColumnTypeVariant::PARAM_IN};
	case 1253:
		return {TeradataTypeId::DATE_T, TeradataColumnTypeVariant::PARAM_INOUT};
	case 1254:
		return {TeradataTypeId::DATE_T, TeradataColumnTypeVariant::PARAM_OUT};

	case 756:
		return {TeradataTypeId::BYTEINT, TeradataColumnTypeVariant::STANDARD};
	case 757:
		return {TeradataTypeId::BYTEINT, TeradataColumnTypeVariant::NULLABLE};
	case 1256:
		return {TeradataTypeId::BYTEINT, TeradataColumnTypeVariant::PARAM_IN};
	case 1257:
		return {TeradataTypeId::BYTEINT, TeradataColumnTypeVariant::PARAM_INOUT};
	case 1258:
		return {TeradataTypeId::BYTEINT, TeradataColumnTypeVariant::PARAM_OUT};
	default:
		// TODO: More types
		throw NotImplementedException("Unknown data type: %d", type);
	}
}

static bool TryParseTypeModsFromFormat(const char *ptr, int16_t len, int64_t &precision, int64_t &scale) {
	const auto end = ptr + len;
	if (ptr != end && *ptr == 'X') {
		ptr++;
		if (ptr != end && *ptr == '(') {
			ptr++;

			// parse the length/precision
			char *endptr;
			precision = strtol(ptr, &endptr, 10);
			if (endptr == ptr) {
				// no number found
				return false;
			}
			ptr = endptr;

			// skip the comma
			if (ptr != end && *ptr == ',') {
				ptr++;

				// parse the scale
				scale = strtol(ptr, &endptr, 10);
				if (endptr == ptr) {
					// no number found
					return false;
				}
			}
			if (ptr != end && *ptr == ')') {
				ptr++;
				return true;
			}

			return false;
		}
	}
	return false;
}

void TeradataRequestContext::Prepare(const string &sql, vector<TeradataType> &types, vector<string> &names) {
	BeginRequest(sql, 'S');

	// Fetch and match the second parcel
	MatchParcel(PclPREPINFO);

	// Parse the columns
	BinaryReader reader(buffer.data(), dbc.fet_ret_data_len);

	const auto prep_info = reader.Read<CliPrepInfoType>();
	for (int16_t col_idx = 0; col_idx < prep_info.ColumnCount; col_idx++) {
		const auto col_info = reader.Read<CliPrepColInfoType>();

		const auto name_len = reader.Read<int16_t>();
		const auto name_str = reader.ReadBytes(name_len);

		names.emplace_back(name_str, name_len);

		const auto format_len = reader.Read<int16_t>();
		const auto format_str = reader.ReadBytes(format_len);

		// TODO: Do something with the title?
		const auto title_len = reader.Read<int16_t>();
		// const auto title_str = reader.ReadBytes(title_len);
		reader.Skip(title_len);

		// Get the type from the parcel
		auto col_type = GetTeradataTypeFromParcel(col_info.DataType);
		auto &td_type = col_type.type;

		// Try to get the type mods from the format
		int64_t precision = 0;
		int64_t scale = 0;

		if (TryParseTypeModsFromFormat(format_str, format_len, precision, scale)) {
			td_type.SetPrecision(precision);
			td_type.SetScale(scale);
		}

		types.push_back(td_type);
	}

	// End statement
	MatchParcel(PclENDSTATEMENT);

	// End request
	MatchParcel(PclENDREQUEST);

	is_open = false;
}

void TeradataRequestContext::Query(const string &sql, vector<TeradataType> &types) {

	BeginRequest(sql, 'E');

	const auto parcel = FetchParcel();

	if (parcel == PclENDSTATEMENT) {
		// No data
		EndRequest();
		return;
	}

	// Otherwise, return records
	if (parcel != PclDATAINFO) {
		throw IOException("Expected PclDATAINFO, got %d", parcel);
	}

	// Now parse the data info and set types
	// TODO:
	BinaryReader reader(buffer.data(), dbc.fet_ret_data_len);

	// Read the number of columns
	const auto field_count = reader.Read<uint16_t>();
	for (uint16_t field_idx = 0; field_idx < field_count; field_idx++) {
		const auto id_type = reader.Read<uint16_t>();

		auto col_type = GetTeradataTypeFromParcel(id_type);
		auto &td_type = col_type.type;

		if (td_type.IsDecimal()) {
			const auto precision = reader.Read<uint16_t>();
			const auto scale = reader.Read<uint16_t>();
			td_type.SetPrecision(precision);
			td_type.SetScale(scale);
		} else {
			const auto length = reader.Read<uint16_t>();
			if (td_type.HasLengthModifier()) {
				td_type.SetLength(length);
			}
		}

		types.push_back(td_type);
	}
}

void TeradataRequestContext::MatchParcel(uint16_t flavor) {
	const auto result = FetchParcel();
	if (result != flavor) {
		throw IOException("Expected parcel flavor %d, got %d", flavor, result);
	}
}

uint16_t TeradataRequestContext::FetchParcel() {
	dbc.func = DBFFET;
	dbc.fet_data_ptr = buffer.data();
	dbc.fet_max_data_len = static_cast<int32_t>(buffer.size());

	int32_t result = EM_OK;
	DBCHCL(&result, cnta, &dbc);

	while (result == BUFOVFLOW) {
		buffer.resize(dbc.fet_ret_data_len);
		dbc.fet_data_ptr = buffer.data();
		dbc.fet_max_data_len = buffer.size();
		DBCHCL(&result, cnta, &dbc);
	}

	while (result == EM_NODATA) {
		// No data, try again
		// TODO: Yield the DuckDB task here, dont just busy loop
		DBCHCL(&result, cnta, &dbc);
	}

	if (result != EM_OK) {
		throw IOException("Failed to fetch result: %s", string(dbc.msg_text, dbc.msg_len));
	}

	if (dbc.fet_parcel_flavor == PclFAILURE) {
		BinaryReader reader(buffer.data(), dbc.fet_ret_data_len);
		const auto stmt_no = reader.Read<uint16_t>();
		const auto info = reader.Read<uint16_t>();
		const auto code = reader.Read<uint16_t>();
		const auto msg_len = reader.Read<uint16_t>();
		const auto msg = reader.ReadBytes(msg_len);

		throw IOException("Teradata request failed, stmt_no: %d, info: %d, code: %d, msg: '%s'", stmt_no, info, code,
		                  string(msg, msg_len));
	}

	return dbc.fet_parcel_flavor;
}

template <class T>
static void ReadField(BinaryReader &reader, Vector &col_vec, idx_t row_idx, bool is_null) {
	if (is_null) {
		FlatVector::SetNull(col_vec, row_idx, true);
		reader.Skip(sizeof(T));
	} else {
		FlatVector::GetData<T>(col_vec)[row_idx] = reader.Read<T>();
	}
}

// Specialize booleans to read any non-zero value as true
static void ReadBoolField(BinaryReader &reader, Vector &col_vec, idx_t row_idx, bool is_null) {
	if (is_null) {
		FlatVector::SetNull(col_vec, row_idx, true);
		reader.Skip(sizeof(bool));
	} else {
		FlatVector::GetData<bool>(col_vec)[row_idx] = reader.Read<int8_t>() != 0;
	}
}

static void ReadBlobField(BinaryReader &reader, Vector &col_vec, idx_t row_idx, bool is_null,
                          const TeradataType &td_type) {
	switch (td_type.GetId()) {
	case TeradataTypeId::VARBYTE: {
		const auto length = reader.Read<uint16_t>();
		if (is_null) {
			FlatVector::SetNull(col_vec, row_idx, true);
		} else {
			const auto data_ptr = FlatVector::GetData<string_t>(col_vec);
			const auto text_ptr = reader.ReadBytes(length);
			data_ptr[row_idx] = StringVector::AddStringOrBlob(col_vec, text_ptr, length);
		}
	} break;
	case TeradataTypeId::BYTE: {
		const auto max_length = td_type.GetLength();
		if (is_null) {
			FlatVector::SetNull(col_vec, row_idx, true);
			reader.Skip(max_length);
		} else {
			const auto data_ptr = FlatVector::GetData<string_t>(col_vec);
			const auto text_ptr = reader.ReadBytes(max_length);
			data_ptr[row_idx] = StringVector::AddStringOrBlob(col_vec, text_ptr, max_length);
		}
	} break;
	default:
		throw NotImplementedException("Unsupported Binary Type: '%s'", td_type.ToString());
	}
}

static void ReadVarcharField(BinaryReader &reader, Vector &col_vec, idx_t row_idx, bool is_null,
                             const TeradataType &td_type) {

	// The logic differs slightly depending on what type of varchar we are dealing with
	switch (td_type.GetId()) {
	case TeradataTypeId::VARCHAR: {
		const auto length = reader.Read<uint16_t>();
		if (is_null) {
			FlatVector::SetNull(col_vec, row_idx, true);
		} else {
			// TODO: This is not enough, we need to handle shift-out characters
			const auto data_ptr = FlatVector::GetData<string_t>(col_vec);
			const auto text_ptr = reader.ReadBytes(length);
			data_ptr[row_idx] = StringVector::AddString(col_vec, text_ptr, length);
		}
	} break;
	case TeradataTypeId::DATE_A:
	case TeradataTypeId::CHAR: {
		const auto max_length = td_type.GetLength();
		if (is_null) {
			FlatVector::SetNull(col_vec, row_idx, true);
			reader.Skip(max_length);
		} else {
			// For CHAR, the max length is the length of the field
			// TODO: This is not enough, we need to handle shift-out characters
			const auto data_ptr = FlatVector::GetData<string_t>(col_vec);
			const auto text_ptr = reader.ReadBytes(max_length);
			data_ptr[row_idx] = StringVector::AddString(col_vec, text_ptr, max_length);
		}
	} break;
	default:
		throw NotImplementedException("Unsupported String Type: '%s'", td_type.ToString());
	}
}

bool TeradataRequestContext::Fetch(DataChunk &chunk, const vector<TeradataType> &types) {
	if (!is_open) {
		chunk.SetCardinality(0);
		return false;
	}

	idx_t row_idx = 0;

	while (row_idx < chunk.GetCapacity()) {
		const auto parcel = FetchParcel();

		switch (parcel) {
		case PclRECORD: {
			// Offset null bytes
			const auto null_bytes = (chunk.ColumnCount() + 7) / 8;
			const auto data_ptr = buffer.data() + null_bytes;
			const auto data_len = dbc.fet_ret_data_len;

			// Parse the record
			BinaryReader reader(data_ptr, data_len);

			for (idx_t col_idx = 0; col_idx < chunk.ColumnCount(); col_idx++) {
				// The NullIndicators Field contains one bit for each item in the DataField,
				// stored in the minimum number of 8-bit bytes required to hold them,
				// with the unused bits in the rightmost byte set to zero.
				// Each bit is matched on a positional basis to an item in the Data Field.
				// That is, the ith bit in the NullIndicators Field corresponds to the ith item in the Data Field.
				const auto byte_idx = col_idx / 8;
				const auto bit_idx = (7 - (col_idx % 8));
				const bool is_null = (buffer[byte_idx] & (1 << bit_idx)) != 0;

				auto &col_vec = chunk.data[col_idx];

				// Convert Type
				switch (col_vec.GetType().id()) {
				case LogicalTypeId::BOOLEAN:
					ReadBoolField(reader, col_vec, row_idx, is_null);
					break;
				case LogicalTypeId::TINYINT:
					ReadField<int8_t>(reader, col_vec, row_idx, is_null);
					break;
				case LogicalTypeId::UTINYINT:
					ReadField<uint8_t>(reader, col_vec, row_idx, is_null);
					break;
				case LogicalTypeId::SMALLINT:
					ReadField<int16_t>(reader, col_vec, row_idx, is_null);
					break;
				case LogicalTypeId::INTEGER:
					ReadField<int32_t>(reader, col_vec, row_idx, is_null);
					break;
				case LogicalTypeId::BIGINT:
					ReadField<int64_t>(reader, col_vec, row_idx, is_null);
					break;
				case LogicalTypeId::DOUBLE:
					ReadField<double>(reader, col_vec, row_idx, is_null);
					break;
				case LogicalTypeId::BLOB:
					ReadBlobField(reader, col_vec, row_idx, is_null, types[col_idx]);
					break;
				case LogicalTypeId::VARCHAR:
					ReadVarcharField(reader, col_vec, row_idx, is_null, types[col_idx]);
					break;
				default:
					throw NotImplementedException("Unsupported Teradata Type: '%s'", col_vec.GetType().ToString());
				}
			}

			// Increment row id
			row_idx++;

		} break;
		case PclENDSTATEMENT: {
			EndRequest();
			chunk.SetCardinality(row_idx);
			return true;
		}
		default: {
			throw IOException("Unexpected parcel flavor %d", parcel);
		}
		}
	}
	chunk.SetCardinality(row_idx);
	return true;
}

unique_ptr<ColumnDataCollection> TeradataRequestContext::FetchAll(const vector<TeradataType> &types) {
	if (!is_open) {
		throw IOException("Teradata request is not open");
	}
	if (types.empty()) {
		throw IOException("Cannot fetch all data without types");
	}

	// Convert teradatat types to DuckDB types
	vector<LogicalType> duck_types;
	for (const auto &td_type : types) {
		duck_types.push_back(td_type.ToDuckDB());
	}

	// Create a CDC to hold our result
	auto result = make_uniq<ColumnDataCollection>(Allocator::DefaultAllocator(), duck_types);

	// Initialize CDC append and payload chunk
	ColumnDataAppendState append_state;
	result->InitializeAppend(append_state);

	DataChunk chunk;
	chunk.Initialize(Allocator::DefaultAllocator(), duck_types);

	// Fetch chunk by chunk and append to the CDC
	while (Fetch(chunk, types)) {
		result->Append(append_state, chunk);
		chunk.Reset();
	}

	return result;
}

void TeradataRequestContext::BeginRequest(const string &sql, char mode) {
	// Setup the request
	dbc.func = DBFIRQ;     // initiate request
	dbc.change_opts = 'Y'; // change options to indicate that we want to change the options (to indicator mode)
	dbc.resp_mode = 'I';   // indicator record mode
	dbc.req_proc_opt = mode;

	// prepare mode (with params)	= 'S'
	// prepare mode (no params)		= 'P'
	// execute mode					= 'E'

	// Pass the SQL
	dbc.req_ptr = const_cast<char *>(sql.c_str());
	dbc.req_len = static_cast<int32_t>(sql.size());

	// Initialize request
	int32_t result = EM_OK;
	DBCHCL(&result, cnta, &dbc);
	if (result != EM_OK) {
		throw IOException("Failed to execute SQL: %s", string(dbc.msg_text, dbc.msg_len));
	}

	is_open = true;
	dbc.i_req_id = dbc.o_req_id;

	// Fetch and match the first parcel
	MatchParcel(PclSUCCESS);
}

void TeradataRequestContext::EndRequest() {
	// End request
	MatchParcel(PclENDREQUEST);
	is_open = false;
}

void TeradataRequestContext::Close() {
	if (!is_open) {
		return;
	}

	int32_t result = EM_OK;

	dbc.func = DBFERQ;
	DBCHCL(&result, cnta, &dbc);
	if (result != EM_OK) {
		throw IOException("Failed to close Teradata request");
	}
	is_open = false;
}

TeradataRequestContext::~TeradataRequestContext() {
	if (is_open) {
		Close();
	}
}

} // namespace duckdb