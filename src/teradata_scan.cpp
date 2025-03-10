#include "duckdb.hpp"
#include "duckdb/main/extension_util.hpp"

#include "teradata_common.hpp"
#include "teradata_scan.hpp"
#include "teradata_type.hpp"

namespace duckdb {

//------------------------------------------------------------------------------
// UTIL
//------------------------------------------------------------------------------

class TeradataRequestConnection {
public:
	enum class State : uint8_t { NO_SESSION = 0, STANDBY, IN_REQUEST };
	TeradataRequestConnection() = default;

	void Connect();

	void Prepare(const string &query, vector<string> &names, vector<LogicalType> &types);

	void BeginRequest(const string &query);
	bool Fetch(DataChunk &chunk);
	void EndRequest();

	void Disconnect();

	~TeradataRequestConnection() {
		Disconnect();
	}

private:
	void FetchAndExpectParcel(idx_t parcel);

	DBCAREA dbc = {};
	Int32 session_id = 0;
	State state = State::NO_SESSION;
	char cnta[4] = {};
};

void TeradataRequestConnection::Connect() {
	// TODO: Check state
	state = State::NO_SESSION;

	dbc.change_opts = 'Y';
	dbc.resp_mode = 'R'; // 'Record' mode
	dbc.keep_resp = 'N'; // Only allow one sequential pass through the response buffer, then discard it
	dbc.use_presence_bits = 'N';
	dbc.wait_across_crash = 'N';
	dbc.tell_about_crash = 'Y';
	dbc.loc_mode = 'Y'; // 'Local' mode (?);
	dbc.var_len_req = 'N';
	dbc.var_len_fetch = 'N';
	dbc.save_resp_buf = 'N';
	dbc.two_resp_bufs = 'N';
	dbc.ret_time = 'N';
	dbc.parcel_mode = 'Y';   // 'Parcel' mode (?);
	dbc.wait_for_resp = 'Y'; // suspend the application until the response is received
	dbc.req_proc_opt = 'E';  // 'Execute' mode, dont prepare.

	// Set the total length
	dbc.total_len = sizeof(DBCAREA);

	Int32 result = EM_OK;
	DBCHINI(&result, cnta, &dbc); // Try to initialize the DBCAREA
	if (result != EM_OK) {
		// Failed to initialize the DBCAREA we cannot connect
		throw InternalException("Failed to initialize DBCAREA");
	}

	// Set the function to CONNECT
	dbc.func = DBFCON;

	char logon[] = "127.0.0.1/dbc,dbc";
	dbc.logon_ptr = logon;
	dbc.logon_len = static_cast<Int32>(strlen(logon));
	dbc.connect_type = 'R'; // 'Remote' connection

	// Try to connect
	DBCHCL(&result, cnta, &dbc);
	if (result != EM_OK) {
		// TODO: Terminate connection on error
		const string error_msg(dbc.msg_text, dbc.msg_len);
		throw IOException("Failed to connect to Teradata database: %s", error_msg);
	}

	// If the request was successful, perform a fetch to get the response
	dbc.func = DBFFET;

	char buf[512] = {};
	dbc.fet_data_ptr = buf;
	dbc.fet_max_data_len = sizeof(buf);

	// Set the request id
	dbc.i_req_id = dbc.o_req_id;
	dbc.i_sess_id = dbc.o_sess_id;

	// Now call the fetch command
	DBCHCL(&result, cnta, &dbc);
	// Check the return code
	if (result != EM_OK) {
		// TODO: Terminate the request properly
		const string error_msg(dbc.msg_text, dbc.msg_len);
		throw IOException("Connection failed: %s", error_msg);
	}

	// Now end the request
	dbc.func = DBFERQ;
	DBCHCL(&result, cnta, &dbc);
	if (result != EM_OK) {
		// TODO: Resubmit the request
		throw IOException("Failed to end request");
	}

	// At this point, we should save the session id we got from the connection
	session_id = dbc.o_sess_id;
	state = State::STANDBY;
}

void TeradataRequestConnection::BeginRequest(const string &query) {
	if (state != State::STANDBY) {
		throw IOException("Not connected to teradata!");
	}

	dbc.func = DBFIRQ;      // initiate request
	dbc.change_opts = 'Y';  // change options to indicate that we want to change the options (to indicator mode)
	dbc.resp_mode = 'I';    // indicator record mode
	dbc.req_proc_opt = 'E'; // execute mode
	dbc.i_sess_id = session_id;

	// We can't pass const char* to the dbc.req_ptr, so we need to copy the string
	vector<char> sql(query.begin(), query.end());

	dbc.req_ptr = sql.data();
	dbc.req_len = static_cast<Int32>(sql.size());

	int32_t result = EM_OK;
	DBCHCL(&result, cnta, &dbc);
	if (result != EM_OK) {
		throw IOException("Failed to execute SQL: %s", string(dbc.msg_text, dbc.msg_len));
	}

	// Fetch once to check if the request was successful
	char buf[512] = {0};
	dbc.func = DBFFET;
	dbc.i_req_id = dbc.o_req_id;
	dbc.fet_data_ptr = buf;
	dbc.fet_max_data_len = sizeof(buf);

	DBCHCL(&result, cnta, &dbc);
	if (result != EM_OK) {
		throw IOException("Failed to fetch result: %s", string(dbc.msg_text, dbc.msg_len));
	}

	// Now what
	if (dbc.fet_parcel_flavor != PclSUCCESS) {
		// TODO: Check error parcel instead of just failing
		throw IOException("Failed to execute SQL: %s");
	}

	state = State::IN_REQUEST;
}

bool TeradataRequestConnection::Fetch(DataChunk &chunk) {
	if (state != State::IN_REQUEST) {
		throw InternalException("Not connected to teradata!");
	}

	char buf[512] = {};
	dbc.func = DBFFET;
	dbc.fet_data_ptr = buf;
	dbc.fet_max_data_len = sizeof(buf);

	Int32 result = EM_OK;

	const idx_t validity_bytes = (chunk.ColumnCount() + 7) / 8;

	// This is how much capacity we have in the chunk
	idx_t row_idx = 0;

	while (row_idx < STANDARD_VECTOR_SIZE) {

		DBCHCL(&result, cnta, &dbc);
		if (result != EM_OK) {
			throw IOException("Failed to fetch result: %s", string(dbc.msg_text, dbc.msg_len));
		}

		switch (dbc.fet_parcel_flavor) {
		case PclDATAINFO: {
			printf("Got data info");
			break;
		}
		case PclENDREQUEST: {
			// No more rows remaining
			return false;
		}
		case PclRECORD: {
			printf("Got record!");
			// Within the Record parcel, each line of a table is separated from the next by hex 0D (decimal 13).
			// Data conversion rules in p.1144
			// p. 1144, 1143, 1145
			auto record_ptr = buf + validity_bytes;

			for (idx_t col_idx = 0; col_idx < chunk.ColumnCount(); col_idx++) {
				const auto is_null = (buf[col_idx / 8] & (1 << (col_idx % 8))) != 0;

				auto &col_vec = chunk.data[col_idx];
				if (is_null) {
					FlatVector::SetNull(col_vec, row_idx, true);
				} else {
					const auto data_ptr = FlatVector::GetData<int32_t>(col_vec);
					memcpy(data_ptr + row_idx, record_ptr, sizeof(int32_t));
				}

				// TODO: Size of the data type
				record_ptr += 4;
			}

			// Move to the next row
			row_idx++;
			chunk.SetCardinality(row_idx);
			break;
		}
		case PclENDSTATEMENT: {
			// TODO: Handle multiple statement (throw?)
			// printf("End statement");
			break;
		}
		default: {
			throw IOException("Unknown parcel flavor: %d", dbc.fet_parcel_flavor);
		}
		}
	}

	return true;
}

void TeradataRequestConnection::FetchAndExpectParcel(idx_t parcel) {
	Int32 result = EM_OK;
	DBCHCL(&result, cnta, &dbc);
	if (result != EM_OK) {
		throw IOException("Failed to fetch result: %s", string(dbc.msg_text, dbc.msg_len));
	}
	if (dbc.fet_parcel_flavor != parcel) {
		throw IOException("Expected parcel flavor %d, got %d", parcel, dbc.fet_parcel_flavor);
	}
}

void TeradataRequestConnection::Prepare(const string &query, vector<string> &names, vector<LogicalType> &types) {
	if (state != State::STANDBY) {
		throw IOException("Not connected to teradata!");
	}

	dbc.func = DBFIRQ;      // initiate request
	dbc.change_opts = 'Y';  // change options to indicate that we want to change the options (to indicator mode)
	dbc.resp_mode = 'I';    // indicator record mode
	dbc.req_proc_opt = 'P'; // prepare mode
	dbc.i_sess_id = session_id;

	// We can't pass const char* to the dbc.req_ptr, so we need to copy the string
	vector<char> sql(query.begin(), query.end());

	dbc.req_ptr = sql.data();
	dbc.req_len = static_cast<Int32>(sql.size());

	int32_t result = EM_OK;
	DBCHCL(&result, cnta, &dbc);
	if (result != EM_OK) {
		throw IOException("Failed to execute SQL: %s", string(dbc.msg_text, dbc.msg_len));
	}

	// Fetch once to check if the request was successful
	char buf[512] = {0};
	dbc.func = DBFFET;
	dbc.i_req_id = dbc.o_req_id;
	dbc.fet_data_ptr = buf;
	dbc.fet_max_data_len = sizeof(buf);

	// Now call the fetch command
	FetchAndExpectParcel(PclSUCCESS);

	// Now get the response
	FetchAndExpectParcel(PclPREPINFO);

	// Parse the prepinfo
	auto beg_ptr = buf;
	// auto end_ptr = buf + dbc.fet_ret_data_len;
	CliPrepInfoType info = {};
	memcpy(&info, beg_ptr, sizeof(CliPrepInfoType));
	beg_ptr += sizeof(CliPrepInfoType);

	// Loop for the columns (ignore summaries for now)
	for (PclInt16 col_idx = 0; col_idx < info.ColumnCount; col_idx++) {
		CliPrepColInfoType col_info = {};
		memcpy(&col_info, beg_ptr, sizeof(CliPrepColInfoType));
		beg_ptr += sizeof(CliPrepColInfoType);
		/*
		//auto ttype = TeradataColumnType::Get(col_info.DataType);
		if (ttype.type.id() == LogicalTypeId::INVALID) {
		    throw NotImplementedException("Unsupported Teradata Type: '%s'", ttype.name);
		}

		types.emplace_back(std::move(ttype.type));

		// Now also read the column_name length
		PclInt16 col_name_len = 0;
		memcpy(&col_name_len, beg_ptr, sizeof(PclInt16));
		beg_ptr += sizeof(PclInt16);

		// Copy the column name
		names.emplace_back(beg_ptr, col_name_len);

		// Skip that many bytes ahead
		beg_ptr += col_name_len;

		// Now read the column format length
		PclInt16 col_format_len = 0;
		memcpy(&col_format_len, beg_ptr, sizeof(PclInt16));
		beg_ptr += sizeof(PclInt16);

		// Skip that many bytes ahead
		beg_ptr += col_format_len;

		// Now read the column title length
		PclInt16 col_title_len = 0;
		memcpy(&col_title_len, beg_ptr, sizeof(PclInt16));
		beg_ptr += sizeof(PclInt16);

		// Skip that many bytes ahead
		beg_ptr += col_title_len;
		*/
	}

	FetchAndExpectParcel(PclENDSTATEMENT);

	FetchAndExpectParcel(PclENDREQUEST);

	// Now end the request
	dbc.func = DBFERQ;
	dbc.i_req_id = dbc.o_req_id;
	DBCHCL(&result, cnta, &dbc);
	if (result != EM_OK) {
		throw IOException("Failed to end request: %s", string(dbc.msg_text, dbc.msg_len));
	}
}

void TeradataRequestConnection::EndRequest() {
	if (state != State::IN_REQUEST) {
		throw InternalException("Not in request!");
	}

	// Now end the request
	dbc.func = DBFERQ;
	dbc.i_req_id = dbc.o_req_id;

	Int32 result = EM_OK;

	DBCHCL(&result, cnta, &dbc);
	if (result != EM_OK) {
		// TODO: Resubmit the request
		throw IOException("Failed to end request");
	}

	state = State::STANDBY;
}

void TeradataRequestConnection::Disconnect() {
	if (state == State::NO_SESSION) {
		return;
	}

	dbc.func = DBFDSC;
	dbc.i_sess_id = session_id;
	Int32 result = EM_OK;
	DBCHCL(&result, cnta, &dbc);
	if (result != EM_OK) {
		// Can this even happen?
		throw IOException("Failed to disconnect from Teradata database: %s", string(dbc.msg_text, dbc.msg_len));
	}
}

//------------------------------------------------------------------------------
// Bind
//------------------------------------------------------------------------------
struct TeradataScanData final : public TableFunctionData {
	unique_ptr<TeradataRequestConnection> conn;
};

static unique_ptr<FunctionData> Bind(ClientContext &context, TableFunctionBindInput &input,
                                     vector<LogicalType> &return_types, vector<string> &names) {
	auto result = make_uniq<TeradataScanData>();

	result->conn = make_uniq<TeradataRequestConnection>();
	result->conn->Connect();
	result->conn->Prepare("SELECT 2, 42", names, return_types);

	// return_types.push_back(LogicalType::INTEGER);
	// names.push_back("value");

	return std::move(result);
}

//------------------------------------------------------------------------------
// Init
//------------------------------------------------------------------------------
struct TeraDataScanState final : public GlobalTableFunctionState {
	bool done = false;
};

static unique_ptr<GlobalTableFunctionState> Init(ClientContext &context, TableFunctionInitInput &input) {
	auto result = make_uniq<TeraDataScanState>();
	auto &data = input.bind_data->Cast<TeradataScanData>();

	data.conn->BeginRequest("SELECT 2, 42");

	return std::move(result);
}

//------------------------------------------------------------------------------
// Execute
//------------------------------------------------------------------------------
static void Execute(ClientContext &context, TableFunctionInput &data, DataChunk &output) {
	auto &bdata = data.bind_data->Cast<TeradataScanData>();
	auto &state = data.global_state->Cast<TeraDataScanState>();

	if (state.done) {
		output.SetCardinality(0);
		return;
	}

	const auto keep_going = bdata.conn->Fetch(output);
	if (!keep_going) {
		bdata.conn->EndRequest();
		state.done = true;
	}
}

//------------------------------------------------------------------------------
// Register
//------------------------------------------------------------------------------
void TeradataScan::Register(DatabaseInstance &db) {

	TableFunction function;
	function.name = "teradata_scan";

	function.bind = Bind;
	function.init_global = Init;
	function.function = Execute;

	// function.named_parameters = {{"dsn", LogicalType::VARCHAR}};

	ExtensionUtil::RegisterFunction(db, function);
}

} // namespace duckdb