#include "teradata_connection.hpp"
#include "teradata_common.hpp"
#include "teradata_request.hpp"

namespace duckdb {

void TeradataConnection::Reconnect() {

	Int32 result = EM_OK;

	dbc.change_opts = 'Y';
	dbc.resp_mode = 'R'; // 'Record' mode
	dbc.keep_resp = 'N'; // Only allow one sequential pass through the response buffer, then discard it
	dbc.use_presence_bits = 'N';
	dbc.wait_across_crash = 'N';
	dbc.tell_about_crash = 'Y';
	dbc.loc_mode = 'Y'; // 'Local' mode (?);
	dbc.var_len_req = 'Y'; // Required to pass parameter descriptor length, p.120
	dbc.var_len_fetch = 'N';
	dbc.save_resp_buf = 'N';
	dbc.two_resp_bufs = 'N';
	dbc.ret_time = 'N';
	dbc.parcel_mode = 'Y';   // 'Parcel' mode (?);
	dbc.wait_for_resp = 'Y'; // suspend the application until the response is received
	dbc.req_proc_opt = 'E';  // 'Execute' mode, dont prepare.

	// Set the total length
	dbc.total_len = sizeof(DBCAREA);

	DBCHINI(&result, cnta, &dbc); // Try to initialize the DBCAREA
	if (result != EM_OK) {
		// Failed to initialize the DBCAREA we cannot connect
		throw InternalException("Failed to initialize DBCAREA: %s", string(dbc.msg_text, dbc.msg_len));
	}

	vector<char> logon_buf(logon_string.begin(), logon_string.end());
	dbc.func = DBFCON;
	dbc.logon_ptr = logon_buf.data();
	dbc.logon_len = logon_buf.size();
	dbc.connect_type = 'R';

	// Try to connect
	DBCHCL(&result, cnta, &dbc);
	if (result != EM_OK) {
		// Failed to connect
		throw IOException("Failed to connect to Teradata: %s", string(dbc.msg_text, dbc.msg_len));
	}

	dbc.func = DBFFET;
	char buf[512] = {};
	dbc.fet_data_ptr = buf;
	dbc.fet_max_data_len = sizeof(buf);

	dbc.i_req_id = dbc.o_req_id;

	// Now call the fetch command
	DBCHCL(&result, cnta, &dbc);
	if (result != EM_OK) {
		// Failed to fetch
		throw IOException("Failed to fetch from Teradata: %s", string(dbc.msg_text, dbc.msg_len));
	}

	session_id = dbc.o_sess_id;
	is_connected = true;
}

void TeradataConnection::Disconnect() {
	if (!is_connected) {
		return;
	}

	Int32 result = EM_OK;

	dbc.func = DBFDSC;
	dbc.i_sess_id = session_id;
	DBCHCL(&result, cnta, &dbc);
	if (result != EM_OK) {
		// Can this even happen?
		throw IOException("Failed to disconnect from Teradata database: %s", string(dbc.msg_text, dbc.msg_len));
	}

	is_connected = false;
}

void TeradataConnection::Execute(const string &sql) {

	// TODO: Pool request contexts
	TeradataRequestContext ctx(*this);
	ctx.Execute(sql);
}

void TeradataConnection::Execute(const string &sql, DataChunk &chunk) {
	TeradataRequestContext ctx(*this);
	ctx.Execute(sql, chunk);
}


unique_ptr<TeradataQueryResult> TeradataConnection::Query(const string &sql, bool materialize) {

	// TODO: Pool request contexts
	auto ctx = make_uniq<TeradataRequestContext>(*this);

	vector<TeradataType> types;
	ctx->Query(sql, types);

	if(materialize) {
		// Fetch all into a CDC and return a materialized result
		auto cdc = ctx->FetchAll(types);
		return make_uniq<MaterializedTeradataQueryResult>(std::move(types), std::move(cdc));
	} else {
		// Streaming result, pass on the context to the result so we can keep fetching it lazily
		return make_uniq<StreamingTeradataQueryResult>(std::move(types), std::move(ctx));
	}
}




} // namespace duckdb