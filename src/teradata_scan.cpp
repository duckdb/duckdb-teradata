#include "duckdb.hpp"
#include "duckdb/main/extension_util.hpp"

#include "teradata_common.hpp"
#include "teradata_scan.hpp"

namespace duckdb {

//------------------------------------------------------------------------------
// UTIL
//------------------------------------------------------------------------------

static void Connect() {
	DBCAREA dbc;

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
	dbc.parcel_mode = 'Y'; // 'Parcel' mode (?);
	dbc.wait_for_resp = 'Y'; // suspend the application until the response is received
	dbc.req_proc_opt = 'E'; // 'Execute' mode, dont prepare.

	// set the total length
	dbc.total_len = sizeof(DBCAREA);


	char context_area[4] = {0};  // The context area is for internal use by CLIV2.
	Int32 result = EM_OK;

	DBCHINI(&result, context_area, &dbc); // Try to initialize the DBCAREA
	if(result != EM_OK) {
		// Failed to initialize the DBCAREA we cannot connect
		throw IOException("Failed to initialize DBCAREA");
	}

	// Set the function to CONNECT
	dbc.func = DBFCON;

	// Set the logon string
	char logon[] = "127.0.0.1/dbc,dbc";
	dbc.logon_ptr = logon;
	dbc.logon_len = static_cast<Int32>(strlen(logon));
	dbc.connect_type = 'R'; // 'Remote' connection

	DBCHCL(&result, context_area, &dbc); // Try to connect
	if(result != EM_OK) {
		// Failed to connect
		// TODO: Terminate the session properly, dont throw an exception

		// process the error message
		// if the message are pointer is specified, it is used, otherwise its in msg_text
		if(dbc.dbciMsgP != nullptr) {
			auto len = dbc.dbciMsgM;
			string str(dbc.dbciMsgP, len);
			throw IOException("Failed to connect to Teradata database: %s", str);
		}

		string str(dbc.msg_text, dbc.msg_len);
		throw IOException("Failed to connect to Teradata database: %s", str);
	}

	// If the request was successful, perform a fetch to get the response
	dbc.func = DBFFET;

	// Set the input request id to the output request id from the last request
	dbc.i_req_id = dbc.o_req_id;

	char buf[1024] = {0};
	dbc.fet_data_ptr = buf;
	dbc.fet_max_data_len = sizeof(buf);

	// Now call the fetch command
	DBCHCL(&result, context_area, &dbc);
	// Check the return code
	if(result != EM_OK) {
		string str(dbc.msg_text, dbc.msg_len);
		throw IOException("Connection failed: %s", str);
	}

	// Now end the request
	dbc.func = DBFERQ;
	dbc.i_req_id = dbc.o_req_id;
	DBCHCL(&result, context_area, &dbc);
	if(result != EM_OK) {
		// TODO: Resubmit the request
		throw IOException("Failed to end request");
	}

	// At this point, we should save the session id we got from the connection
	const auto session_id = dbc.o_sess_id;

	printf("Connected to Teradata with session id: %d\n", session_id);



	// Try to make a SQL request
	dbc.func = DBFIRQ; // initiate request
	dbc.change_opts = 'Y'; // change options to indicate that we want to change the options (to indicator mode)
	dbc.resp_mode = 'I'; // indicator record mode
	dbc.i_sess_id = session_id;

	char sql[] = "SELECT NULL, 42";
	dbc.req_ptr = sql;
	dbc.req_len = static_cast<Int32>(strlen(sql));

	DBCHCL(&result, context_area, &dbc);
	if(result != EM_OK) {
		auto str = string(dbc.msg_text, dbc.msg_len);
		throw IOException("Failed to execute SQL: %s", str);
	}
	// this will set the o_req_id

	/*
		Record mode (Y) may return the following parcels
		- Failure (9) - The request failed
		- Error (49) - An error occurred
		- StatementError (192) - An error occurred in the statement
		- Success (8) - The request was successful
		- StatementStatus (205)
		- Record (10)
		- With (33)
		- EndWith (35)
		- EndStatement (11)
		- EndRequest (12)
	 */

	// Now fetch the result
	dbc.func = DBFFET;
	dbc.i_req_id = dbc.o_req_id;
	dbc.fet_data_ptr = buf;
	dbc.fet_max_data_len = sizeof(buf);

	memset(buf, 0, sizeof(buf));

	printf("Begin sql request");
	bool do_fetch = false;
	do {

		DBCHCL(&result, context_area, &dbc);
		if(result != EM_OK) {
			auto str = string(dbc.msg_text, dbc.msg_len);
			throw IOException("Failed to fetch result: %s", str);
		}

		switch (dbc.fet_parcel_flavor) {
			case PclSUCCESS: {
				printf("Success!");
				do_fetch = true;
				break;
			}
			case PclDATAINFO: {
				printf("Got data info");
				break;
			}
			case PclENDREQUEST: {
				do_fetch = false;
				break;
			}
			case PclRECORD: {
				printf("Got record!");
				// Within the Record parcel, each line of a table is separated from the next by hex 0D (decimal 13).
				// Data conversion rules in p.1144
				// p. 1144, 1143, 1145
				auto n_columns = 2;
				auto n_validity_bytes = (n_columns + 7) / 8;

				auto data = buf + n_validity_bytes;

				int32_t val = 0;
				memcpy(&val, data, sizeof(int32_t));
				printf("Value: %d", val);

				break;
			}
			case PclENDSTATEMENT: {
				printf("End statement");
				break;
			}
			default: {
				printf("Unknown parcel flavor: %d", dbc.fet_parcel_flavor);
				do_fetch = false;
				break;
			}
		}

	} while(do_fetch);
	printf("Done with request");

	// TODO: End request properly
}

//------------------------------------------------------------------------------
// Bind
//------------------------------------------------------------------------------
struct TeradataScanData final : public TableFunctionData  {

};

static unique_ptr<FunctionData> Bind(ClientContext &context, TableFunctionBindInput &input,
	vector<LogicalType> &return_types, vector<string> &names) {
	auto result = make_uniq<TeradataScanData>();

	return_types.push_back(LogicalType::INTEGER);
	names.push_back("value");

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

	result->done = false;

	// Connect to teradata
	Connect();

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

	output.data[0].SetValue(0, Value::INTEGER(0));
	output.SetCardinality(1);
	state.done = true;
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


/*
static DBCAREA* Connect(const string &dsn) {
	DBCAREA dbc;

	auto config = ParseConnectionParameters(dsn);
	const char *host = config.host.empty() ? nullptr : config.host.c_str();
	const char *user = config.user.empty() ? nullptr : config.user.c_str();
	const char *passwd = config.passwd.empty() ? nullptr : config.passwd.c_str();
	//const char *db = config.database.empty() ? nullptr : config.database.c_str();

	const auto lLogon = config.host+"/"+config.user+","+config.passwd;
	const char *psLogon = lLogon.c_str();
	dbc.change_opts = 'Y';
	dbc.resp_mode = 'R';
	dbc.use_presence_bits = 'N';
	dbc.keep_resp = 'N';
	dbc.wait_across_crash = 'N';
	dbc.tell_about_crash = 'Y';
	dbc.loc_mode = 'Y';
	dbc.var_len_req = 'N';
	dbc.var_len_fetch = 'N';
	dbc.save_resp_buf = 'N';
	dbc.two_resp_bufs = 'N';
	dbc.ret_time = 'N';
	dbc.parcel_mode = 'Y';
	dbc.wait_for_resp = 'Y';
	dbc.req_proc_opt = 'E';

	char cnta[4];
	Int32 result = EM_OK;
	dbc.total_len = sizeof(struct DBCAREA);
	DBCHINI(&result, cnta, &dbc);
	if (result != EM_OK) {
		throw IOException("1:Failed to connect to Teradata database with parameters: %s", psLogon);
	}
	//
	dbc.logon_ptr = psLogon;
	dbc.logon_len = static_cast<Int32>(strlen(psLogon));
	dbc.func = DBFCON;

	DBCHCL(&result, cnta, &dbc);
	if (result != EM_OK) {
		throw IOException("2:Failed to connect to Teradata database with parameters : %s", psLogon);
	};
	throw NotImplementedException("Connected using \"%s\"", psLogon);
	// DBCAREA* lconnection = &dbc;
	return &dbc;
}
*/
}