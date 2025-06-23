#include "teradata_request.hpp"
#include "teradata_type.hpp"
#include "teradata_connection.hpp"
#include "teradata_column_writer.hpp"

#include "util/binary_reader.hpp"

namespace duckdb {

TeradataRequestContext::TeradataRequestContext(const TeradataConnection &con) {
	Init(con);
}

void TeradataRequestContext::Init(const TeradataConnection &con) {
	// Initialize
	dbc.total_len = sizeof(DBCAREA);
	dbc.resp_buf_len = BUFFER_DEFAULT_SIZE;
	dbc.two_resp_bufs = 'Y';

	int32_t result = EM_OK;
	DBCHINI(&result, cnta, &dbc);
	if (result != EM_OK) {
		throw IOException("Failed to initialize DBCAREA: %s", string(dbc.msg_text, dbc.msg_len));
	}
	dbc.i_sess_id = con.GetSessionId();
	buffer.resize(BUFFER_DEFAULT_SIZE);
}

void TeradataRequestContext::Execute(const string &sql) {
	BeginRequest(sql, 'E');

	MatchParcel(PclENDSTATEMENT);

	EndRequest();
}

void TeradataRequestContext::Execute(const string &sql, DataChunk &chunk, ArenaAllocator &arena,
                                     vector<unique_ptr<TeradataColumnWriter>> &writers) {

	const auto row_count = chunk.size();
	const auto col_count = chunk.ColumnCount();

	D_ASSERT(row_count > 0);
	D_ASSERT(writers.size() == chunk.ColumnCount());

	// Allocate space for the row pointers and lengths
	const auto record_array = reinterpret_cast<char **>(arena.AllocateAligned(row_count * sizeof(char *)));
	const auto cursor_array = reinterpret_cast<char **>(arena.AllocateAligned(row_count * sizeof(char *)));
	const auto length_array = reinterpret_cast<int32_t *>(arena.AllocateAligned(row_count * sizeof(int32_t)));

	// How many bytes are needed for the validity (presence) bits?
	const int32_t validity_bytes = (static_cast<int32_t>(col_count) + 7) / 8;

	// Initialize the writers
	for (idx_t col_idx = 0; col_idx < col_count; col_idx++) {
		writers[col_idx]->Init(chunk.data[col_idx], row_count);
	}

	// Set the length of the rows to the length of the validity bits
	for (idx_t out_idx = 0; out_idx < row_count; out_idx++) {
		length_array[out_idx] = validity_bytes;
	}

	// First pass, column-wise compute the size of the rows
	for (idx_t col_idx = 0; col_idx < col_count; col_idx++) {
		writers[col_idx]->ComputeSizes(row_count, length_array);
	}

	// Allocate space for the rows
	for (idx_t out_idx = 0; out_idx < row_count; out_idx++) {

		const auto row_length = length_array[out_idx];
		const auto row_record = reinterpret_cast<char *>(arena.AllocateAligned(row_length));

		record_array[out_idx] = row_record;
		cursor_array[out_idx] = row_record + validity_bytes;

		// initialize the presence bits to 0
		memset(row_record, 0, validity_bytes);
	}

	// Second pass, set the presence bits
	for (idx_t col_idx = 0; col_idx < col_count; col_idx++) {
		writers[col_idx]->SetPresenceBits(row_count, col_idx, record_array);
	}

	// Now, third pass, actually write the rows
	for (idx_t col_idx = 0; col_idx < col_count; col_idx++) {
		writers[col_idx]->EncodeVector(row_count, cursor_array);
	}

	dbc.using_data_ptr_array = reinterpret_cast<char *>(record_array);
	dbc.using_data_len_array = length_array;
	dbc.using_data_count = row_count;

	dbc.use_presence_bits = 'Y';
	dbc.var_len_req = 'N';
	dbc.change_opts = 'Y';

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

	// DECIMAL
	case 484:
		return {TeradataTypeId::DECIMAL, TeradataColumnTypeVariant::STANDARD};
	case 485:
		return {TeradataTypeId::DECIMAL, TeradataColumnTypeVariant::NULLABLE};
	case 984:
		return {TeradataTypeId::DECIMAL, TeradataColumnTypeVariant::PARAM_IN};
	case 985:
		return {TeradataTypeId::DECIMAL, TeradataColumnTypeVariant::PARAM_INOUT};
	case 986:
		return {TeradataTypeId::DECIMAL, TeradataColumnTypeVariant::PARAM_OUT};

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

	// Just treat these as varchar with the max length
	case 697:
		return {TeradataTypeId::VARBYTE, TeradataColumnTypeVariant::STANDARD};
	case 698:
		return {TeradataTypeId::VARBYTE, TeradataColumnTypeVariant::NULLABLE};
	case 1197:
		return {TeradataTypeId::VARBYTE, TeradataColumnTypeVariant::PARAM_IN};
	case 1198:
		return {TeradataTypeId::VARBYTE, TeradataColumnTypeVariant::PARAM_INOUT};
	case 1199:
		return {TeradataTypeId::VARBYTE, TeradataColumnTypeVariant::PARAM_OUT};

	// DATE_A (ansidate)
	case 748:
		return {TeradataTypeId::DATE, TeradataColumnTypeVariant::STANDARD};
	case 749:
		return {TeradataTypeId::DATE, TeradataColumnTypeVariant::NULLABLE};
	case 1248:
		return {TeradataTypeId::DATE, TeradataColumnTypeVariant::PARAM_IN};
	case 1249:
		return {TeradataTypeId::DATE, TeradataColumnTypeVariant::PARAM_INOUT};
	case 1250:
		return {TeradataTypeId::DATE, TeradataColumnTypeVariant::PARAM_OUT};

	// DATE_T (integerdate)
	case 752:
		return {TeradataTypeId::DATE, TeradataColumnTypeVariant::STANDARD};
	case 753:
		return {TeradataTypeId::DATE, TeradataColumnTypeVariant::NULLABLE};
	case 1252:
		return {TeradataTypeId::DATE, TeradataColumnTypeVariant::PARAM_IN};
	case 1253:
		return {TeradataTypeId::DATE, TeradataColumnTypeVariant::PARAM_INOUT};
	case 1254:
		return {TeradataTypeId::DATE, TeradataColumnTypeVariant::PARAM_OUT};

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

void TeradataRequestContext::Prepare(const string &sql, vector<TeradataType> &types, vector<string> &names) {
	BeginRequest(sql, 'P');

	// Fetch and match the second parcel
	MatchParcel(PclPREPINFO);

	// Parse the columns
	BinaryReader reader(buffer.data(), dbc.fet_ret_data_len);

	const auto prep_info = reader.Read<CliPrepInfoType>();
	for (int16_t col_idx = 0; col_idx < prep_info.ColumnCount; col_idx++) {
		const auto col_info = reader.Read<CliPrepColInfoType>();

		auto col_type = GetTeradataTypeFromParcel(col_info.DataType);
		auto &td_type = col_type.type;

		// Try to get the type mods from the format
		if (td_type.HasLengthModifier()) {
			td_type.SetLength(col_info.DataLen);
		} else if (td_type.IsDecimal()) {
			// The docs seem wrong here.
			// - The first byte is the fractional digits
			// - The second byte is the integral digits
			// This is the opposite of what the docs say
			const uint8_t scale = col_info.DataLen & 0xFF;
			const uint8_t width = col_info.DataLen >> 8;

			td_type.SetWidth(width);
			td_type.SetScale(scale);
		}

		/*
		    "ColumnName specifies a column's name, consisting of length in bytes of the name followed by that name in
		    characters from the current session character set."

		    TODO: We need to do the charset conversion here if needed.
		 */

		const auto name_len = reader.Read<int16_t>();
		const auto name_str = reader.ReadBytes(name_len);

		names.emplace_back(name_str, name_len);

		// TODO: Do something with the format?
		const auto format_len = reader.Read<int16_t>();
		// const auto format_str = reader.ReadBytes(format_len);
		reader.Skip(format_len);

		// TODO: Do something with the title?
		const auto title_len = reader.Read<int16_t>();
		// const auto title_str = reader.ReadBytes(title_len);
		reader.Skip(title_len);

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
			// The docs seem wrong here.
			// - The first byte is the fractional digits
			// - The second byte is the integral digits
			// This is the opposite of what the docs say
			// const auto scale = reader.Read<uint8_t>();
			// const auto width = reader.Read<uint8_t>();

			const auto length = reader.Read<uint16_t>();
			const uint8_t scale = length & 0xFF;
			const uint8_t width = length >> 8;

			td_type.SetWidth(width);
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
		if (result == PclERROR) {
			// Parse the error message
			BinaryReader reader(buffer.data(), dbc.fet_ret_data_len);
			const auto stmt_no = reader.Read<uint16_t>();
			const auto info = reader.Read<uint16_t>();
			const auto code = reader.Read<uint16_t>();
			const auto msg_len = reader.Read<uint16_t>();
			const auto msg = reader.ReadBytes(msg_len);
			const auto msg_str = string(msg, msg_len);
			throw IOException("Expected parcel flavor %d, got error: stmt_no: %d, info: %d, code: %d, msg: '%s'",
			                  flavor, stmt_no, info, code, msg_str);
		}

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

		// Try to detect some common error codes
		switch (code) {
		case 2801:
		case 2802:
		case 2803:
			throw ConstraintException(string(msg, msg_len));
		default:
			throw IOException("Teradata request failed, stmt_no: %d, info: %d, code: %d, msg: '%s'", stmt_no, info,
			                  code, string(msg, msg_len));
		}
	}

	return dbc.fet_parcel_flavor;
}

bool TeradataRequestContext::Fetch(DataChunk &chunk, const vector<unique_ptr<TeradataColumnReader>> &readers) {
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

				// Call the reader to decode the column
				readers[col_idx]->Decode(reader, col_vec, row_idx, is_null);
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

	// Initialize readers
	vector<unique_ptr<TeradataColumnReader>> readers;
	for (const auto &td_type : types) {
		readers.push_back(TeradataColumnReader::Make(td_type));
	}

	// Fetch chunk by chunk and append to the CDC
	while (Fetch(chunk, readers)) {
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
	dbc.two_resp_bufs = 'Y'; // Enable double buffering for response buffers (to increase throughput?)
	dbc.resp_buf_len = BUFFER_DEFAULT_SIZE;

	// prepare mode (with params)	= 'S'
	// prepare mode (no params)		= 'P'
	// execute mode					= 'E'

	// Pass the SQL
	dbc.req_ptr = const_cast<char *>(sql.c_str());
	dbc.req_len = static_cast<int32_t>(sql.size());

	// Set max decimal return width
	dbc.max_decimal_returned = 38;

	// Set the return date format
	dbc.date_form = 'T';

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