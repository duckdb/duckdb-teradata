#include "duckdb.hpp"
#include "duckdb/main/extension_util.hpp"

#include "teradata_common.hpp"
#include "teradata_scan.hpp"

namespace duckdb {

//------------------------------------------------------------------------------
// Bind
//------------------------------------------------------------------------------
struct TeradataScanData final : public TableFunctionData  {

};

static unique_ptr<FunctionData> Bind(ClientContext &context, TableFunctionBindInput &input,
	vector<LogicalType> &return_types, vector<string> &names) {
	auto result = make_uniq<TeradataScanData>();
	return std::move(result);
}

//------------------------------------------------------------------------------
// Init
//------------------------------------------------------------------------------
struct TeraDataScanState final : public GlobalTableFunctionState {

};

static unique_ptr<GlobalTableFunctionState> Init(ClientContext &context, TableFunctionInitInput &input) {
	auto result = make_uniq<TeraDataScanState>();
	return std::move(result);
}

//------------------------------------------------------------------------------
// Execute
//------------------------------------------------------------------------------
static void Execute(ClientContext &context, TableFunctionInput &data, DataChunk &output) {
	auto &bdata = data.bind_data->Cast<TeradataScanData>();
	auto &state = data.global_state->Cast<TeraDataScanState>();

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

	function.named_parameters = {{"dsn", LogicalType::VARCHAR}};
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