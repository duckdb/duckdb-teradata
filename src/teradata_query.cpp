#include "duckdb/main/database_manager.hpp"
#include "duckdb/parser/parsed_data/create_table_function_info.hpp"
#include "duckdb/main/attached_database.hpp"

#include "teradata_query.hpp"
#include "teradata_catalog.hpp"
#include "teradata_transaction.hpp"
#include "teradata_request.hpp"

#include "duckdb/main/extension_util.hpp"

namespace duckdb {

//----------------------------------------------------------------------------------------------------------------------
// Bind
//----------------------------------------------------------------------------------------------------------------------
struct TeradataQueryData final : TableFunctionData {
	vector<LogicalType> types;
	vector<string> names;
	vector<TeradataType> td_types;
	string sql;
	TeradataCatalog *catalog;
};

static unique_ptr<FunctionData> TeradataQueryBind(ClientContext &context, TableFunctionBindInput &input,
                                                  vector<LogicalType> &return_types, vector<string> &names) {

	if (input.inputs[0].IsNull() || input.inputs[1].IsNull()) {
		throw BinderException("Parameters to teradata_query cannot be NULL");
	}

	const auto db_name = input.inputs[0].GetValue<string>();

	// Look up the database to query
	auto &db_manager = DatabaseManager::Get(context);
	auto db = db_manager.GetDatabase(context, db_name);
	if (!db) {
		throw BinderException("Failed to find attached database \"%s\" referenced in teradata_query", db_name);
	}

	auto &catalog = db->GetCatalog();
	if (catalog.GetCatalogType() != "teradata") {
		throw BinderException("Attached database \"%s\" does not refer to a Teradata database", db_name);
	}

	auto &td_catalog = catalog.Cast<TeradataCatalog>();
	auto &transaction = Transaction::Get(context, catalog).Cast<TeradataTransaction>();

	// Strip any trailing semicolons
	auto sql = input.inputs[1].GetValue<string>();
	StringUtil::RTrim(sql);
	while (!sql.empty() && sql.back() == ';') {
		sql = sql.substr(0, sql.size() - 1);
		StringUtil::RTrim(sql);
	}

	const auto &con = transaction.GetConnection();
	const auto sid = con.GetSessionId();

	vector<TeradataType> td_types;
	// Send a "prepare" request to Teradata to get the column names and types
	{
		TeradataPrepareRequest request(sid, sql);
		request.GetColumns(names, td_types);

		// Convert to duckdb types
		for(auto &td_type : td_types) {
			return_types.push_back(td_type.GetDuckType());
		}
	}

	auto result = make_uniq<TeradataQueryData>();
	result->types = return_types;
	result->td_types = std::move(td_types);
	result->names = names;
	result->sql = sql;
	result->catalog = &td_catalog;

	return std::move(result);
}

//----------------------------------------------------------------------------------------------------------------------
// Init
//----------------------------------------------------------------------------------------------------------------------
struct TeradataQueryState final : GlobalTableFunctionState {
	TeradataSqlRequest request;

	TeradataQueryState(Int32 session_id, const string &sql) : request(session_id, sql) {
	}
};

static unique_ptr<GlobalTableFunctionState> TeradataQueryInit(ClientContext &context, TableFunctionInitInput &input) {
	auto &data = input.bind_data->Cast<TeradataQueryData>();

	// Create a new Teradata request, passing the session ID and SQL
	auto result = make_uniq<TeradataQueryState>(data.catalog->GetConnection().GetSessionId(), data.sql);

	// Check that the types are still the same, in case we need to rebind
	auto &req_types = result->request.GetTypes();
	for(idx_t i = 0; i < req_types.size(); i++) {
		// TODO: Only look at TD id, but also check e.g. decimal precision
		if(data.types[i] != req_types[i].GetDuckType()) {
			throw InvalidInputException("Teradata query schema has changed since bind, please re-execute or re-prepare the query");
		}
	}

	return std::move(result);
}

//----------------------------------------------------------------------------------------------------------------------
// Execute
//----------------------------------------------------------------------------------------------------------------------
static void TeradataQueryExec(ClientContext &context, TableFunctionInput &data, DataChunk &output) {
	auto &state = data.global_state->Cast<TeradataQueryState>();

	if (state.request.GetStatus() == TeradataRequestStatus::OPEN) {
		state.request.FetchNextChunk(output);
	} else {
		output.SetCardinality(0);
	}
}

//----------------------------------------------------------------------------------------------------------------------
// Register
//----------------------------------------------------------------------------------------------------------------------
void TeradataQueryFunction::Register(DatabaseInstance &db) {
	TableFunction function;
	function.name = "teradata_query";
	function.arguments = {LogicalType::VARCHAR, LogicalType::VARCHAR};

	function.bind = TeradataQueryBind;
	function.init_global = TeradataQueryInit;
	function.function = TeradataQueryExec;

	ExtensionUtil::RegisterFunction(db, function);
}

} // namespace duckdb