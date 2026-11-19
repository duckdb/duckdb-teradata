#include "teradata_request.hpp"
#include "teradata_type.hpp"
#include "teradata_connection.hpp"
#include "teradata_column_writer.hpp"

#include "util/binary_reader.hpp"

#include "duckdb/main/client_context.hpp"

#include <chrono>
#include <thread>

namespace duckdb {

TeradataRequestContext::TeradataRequestContext(const TeradataConnection &con, optional_ptr<ClientContext> context_p)
    : context(context_p) {
	Init(con);
}

void TeradataRequestContext::Init(const TeradataConnection &con) {
	memset(&dbc, 0, sizeof(DBCAREA)); // Clear the DBCAREA structure
	memset(&cnta, 0, sizeof(cnta));   // Clear the control area

	// Initialize
	int32_t result = EM_OK;

	// Set the total length
	dbc.total_len = sizeof(DBCAREA);

	DBCHINI(&result, cnta, &dbc);
	if (result != EM_OK) {
		throw IOException("Failed to initialize DBCAREA: %s", string(dbc.msg_text, dbc.msg_len));
	}

	// Resize the parcel buffer to the default size
	buffer.resize(8 * 1024); // 8KB default parcel size

	// Set the session ID, and change options
	dbc.change_opts = 'Y';

	dbc.i_sess_id = con.GetSessionId();
	dbc.resp_buf_len = con.GetBufferSize();
	dbc.resp_mode = 'I';     // 'Record' mode
	dbc.keep_resp = 'N';     // Only allow one sequential pass through the response buffer, then discard it
	dbc.save_resp_buf = 'N'; // Do not save the response buffer
	dbc.two_resp_bufs = 'Y'; // Use two response buffers

	dbc.use_presence_bits = 'Y';

	// With a client context we poll for responses (EM_NODATA) so that the request can be interrupted
	dbc.wait_for_resp = context ? 'N' : 'Y';
	dbc.wait_across_crash = 'N';
	dbc.tell_about_crash = 'Y';

	dbc.ret_time = 'N'; // Do not return the time
	dbc.date_form = 'T';

	dbc.var_len_req = 'N';   // Required to pass parameter descriptor length, p.120
	dbc.var_len_fetch = 'N'; // Do not use variable length fetch

	dbc.max_decimal_returned = 38; // DuckDB default decimals are 38 digits
}

void TeradataRequestContext::Execute(const string &sql) {
	// The statement may call a stored procedure that returns result sets: accept them, and discard them
	dbc.dynamic_result_sets_allowed = 'Y';
	BeginRequest(sql, 'E');

	// Read up to the end of the request; failures are raised by FetchParcel
	uint16_t parcel;
	do {
		parcel = FetchParcel();
		if (parcel == PclERROR) {
			ThrowErrorParcel("Teradata request failed");
		}
	} while (parcel != PclENDREQUEST);
	is_open = false;
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

	dbc.change_opts = 'Y';
	BeginRequest(sql, 'E');

	MatchParcel(PclENDSTATEMENT);

	for (idx_t i = 1; i < chunk.size(); i++) {
		MatchParcel(PclSUCCESS);
		MatchParcel(PclENDSTATEMENT);
	}

	EndRequest();
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

		auto col_type = DecodeParcelType(col_info.DataType);
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

		auto col_type = DecodeParcelType(id_type);
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
			ThrowErrorParcel(StringUtil::Format("Expected parcel flavor %d, got error", flavor));
		}
		throw IOException("Expected parcel flavor %d, got %d", flavor, result);
	}
}

void TeradataRequestContext::ThrowErrorParcel(const string &context) const {
	BinaryReader reader(buffer.data(), dbc.fet_ret_data_len);
	const auto stmt_no = reader.Read<uint16_t>();
	const auto info = reader.Read<uint16_t>();
	const auto code = reader.Read<uint16_t>();
	const auto msg_len = reader.Read<uint16_t>();
	const auto msg = reader.ReadBytes(msg_len);
	throw IOException("%s: stmt_no: %d, info: %d, code: %d, msg: '%s'", context, stmt_no, info, code,
	                  string(msg, msg_len));
}

uint16_t TeradataRequestContext::FetchParcel() {
	dbc.func = DBFFET;
	dbc.fet_data_ptr = buffer.data();
	dbc.fet_max_data_len = static_cast<int32_t>(buffer.size());

	static constexpr int64_t MAX_POLL_INTERVAL_MS = 50;
	int64_t poll_interval_ms = 1;

	int32_t result = EM_OK;
	while (true) {
		DBCHCL(&result, cnta, &dbc);
		if (result == BUFOVFLOW) {
			buffer.resize(dbc.fet_ret_data_len);
			dbc.fet_data_ptr = buffer.data();
			dbc.fet_max_data_len = buffer.size();
			continue;
		}
		if (result == EM_NODATA) {
			// The response is not ready yet (non-blocking mode): check for interrupts, back off and poll again
			CheckInterrupt();
			std::this_thread::sleep_for(std::chrono::milliseconds(poll_interval_ms));
			poll_interval_ms = MinValue<int64_t>(poll_interval_ms * 2, MAX_POLL_INTERVAL_MS);
			continue;
		}
		break;
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

	CheckInterrupt();

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

void TeradataRequestContext::Abort() {
	if (!is_open) {
		return;
	}
	is_open = false;

	// Errors are ignored: we are already unwinding because of the interrupt
	int32_t result = EM_OK;
	dbc.func = DBFABT;
	DBCHCL(&result, cnta, &dbc);

	result = EM_OK;
	dbc.func = DBFERQ;
	DBCHCL(&result, cnta, &dbc);
}

void TeradataRequestContext::CheckInterrupt() {
	if (!context) {
		return;
	}
	try {
		context->BlockingInterruptCheck();
	} catch (...) {
		Abort();
		throw;
	}
}

TeradataRequestContext::~TeradataRequestContext() {
	if (is_open) {
		Close();
	}
}

} // namespace duckdb