#include "teradata_connection.hpp"
#include "teradata_common.hpp"
#include "teradata_request.hpp"

namespace duckdb {

void TeradataConnection::Reconnect() {

	Int32 result = EM_OK;

	const auto dbc_ptr = make_uniq<DBCAREA>();
	auto dbc = *dbc_ptr;         // Copy the DBCAREA to the member variable
	char cnta[4] = {0, 0, 0, 0}; // Initialize the control area to zero

	// Set the total length
	dbc.total_len = sizeof(DBCAREA);

	DBCHINI(&result, cnta, &dbc); // Try to initialize the DBCAREA
	if (result != EM_OK) {
		// Failed to initialize the DBCAREA we cannot connect
		throw InternalException("Failed to initialize DBCAREA: %s", string(dbc.msg_text, dbc.msg_len));
	}

	dbc.change_opts = 'Y';

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
	dbc.i_sess_id = dbc.o_sess_id;

	// Now call the fetch command
	DBCHCL(&result, cnta, &dbc);
	if (result != EM_OK) {
		// Failed to fetch
		throw IOException("Failed to fetch from Teradata: %s", string(dbc.msg_text, dbc.msg_len));
	}

	// End the connection request properly
	dbc.func = DBFERQ;
	DBCHCL(&result, cnta, &dbc);
	// Don't check error here - some versions may not require DBFERQ after connection

	session_id = dbc.o_sess_id;
	is_connected = true;
}

void TeradataConnection::Disconnect() {
	if (!is_connected) {
		return;
	}

	auto dbc_ptr = make_uniq<DBCAREA>();
	auto dbc = *dbc_ptr;         // Copy the DBCAREA to the member variable
	char cnta[4] = {0, 0, 0, 0}; // Initialize the control area to zero

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

void TeradataConnection::Execute(const string &sql, DataChunk &chunk, ArenaAllocator &arena,
                                 vector<unique_ptr<TeradataColumnWriter>> &writers) {
	TeradataRequestContext ctx(*this);
	ctx.Execute(sql, chunk, arena, writers);
}

unique_ptr<TeradataQueryResult> TeradataConnection::Query(const string &sql, bool materialize) {

	// TODO: Pool request contexts
	auto ctx = make_uniq<TeradataRequestContext>(*this);

	vector<TeradataType> types;
	ctx->Query(sql, types);

	if (materialize) {
		// Fetch all into a CDC and return a materialized result
		auto cdc = ctx->FetchAll(types);
		return make_uniq<MaterializedTeradataQueryResult>(std::move(types), std::move(cdc));
	} else {
		// Streaming result, pass on the context to the result so we can keep fetching it lazily
		return make_uniq<StreamingTeradataQueryResult>(std::move(types), std::move(ctx));
	}
}

void TeradataConnection::Prepare(const string &sql, vector<TeradataType> &types, vector<string> &names) {
	// TODO: Pool request contexts
	TeradataRequestContext td_ctx(*this);
	td_ctx.Prepare(sql, types, names);
}

} // namespace duckdb