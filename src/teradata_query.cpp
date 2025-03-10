#include "duckdb/main/database_manager.hpp"
#include "duckdb/parser/parsed_data/create_table_function_info.hpp"
#include "duckdb/main/attached_database.hpp"

#include "teradata_query.hpp"
#include "teradata_catalog.hpp"
#include "teradata_transaction.hpp"

#include "duckdb/main/extension_util.hpp"

namespace duckdb {

//----------------------------------------------------------------------------------------------------------------------
// Bind
//----------------------------------------------------------------------------------------------------------------------
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

	vector<TeradataType> td_types;
	// Send a "prepare" request to Teradata to get the column names and types
	{
		TeradataRequestContext td_ctx(con);
		td_ctx.Prepare(sql, td_types, names);

		// Convert to DuckDB types
		for (auto &td_type : td_types) {
			return_types.push_back(td_type.ToDuckDB());
		}
	}

	if (td_types.empty()) {
		throw BinderException("No fields returned by query \"%s\" - the query must be a SELECT statement that returns "
		                      "at least one column",
		                      sql);
	}

	auto result = make_uniq<TeradataBindData>();
	result->types = return_types;
	result->td_types = std::move(td_types);
	result->names = names;
	result->sql = sql;
	result->SetCatalog(td_catalog);

	return std::move(result);
}

//----------------------------------------------------------------------------------------------------------------------
// Init
//----------------------------------------------------------------------------------------------------------------------
struct TeradataQueryState final : GlobalTableFunctionState {
	unique_ptr<TeradataQueryResult> td_query;
};

static unique_ptr<GlobalTableFunctionState> TeradataQueryInit(ClientContext &context, TableFunctionInitInput &input) {
	auto &data = input.bind_data->Cast<TeradataBindData>();
	string sql = data.sql;

	// If we dont have a SQL string, just copy from the table
	if (sql.empty()) {
		if (data.table_name.empty()) {
			throw InvalidInputException("Teradata query requires a valid SQL string");
		}

		// If we get here, we have a table name, so we need to construct a SQL string
		sql = StringUtil::Format("SELECT * FROM %s.%s", data.schema_name, data.table_name);
	}

	auto result = make_uniq<TeradataQueryState>();

	auto &con = data.GetCatalog()->GetConnection();
	result->td_query = con.Query(sql, data.is_materialized);

	// Check that the types are still the same, in case we need to rebind
	auto &td_types = result->td_query->GetTypes();
	auto &duck_types = data.types;

	for (idx_t i = 0; i < td_types.size(); i++) {
		// TODO: Only look at TD id, but also check e.g. decimal precision
		if (duck_types[i] != td_types[i].ToDuckDB()) {
			throw InvalidInputException(
			    "Teradata query schema has changed since bind, please re-execute or re-prepare the query");
		}
	}

	return std::move(result);
}

//----------------------------------------------------------------------------------------------------------------------
// Execute
//----------------------------------------------------------------------------------------------------------------------
static void TeradataQueryExec(ClientContext &context, TableFunctionInput &data, DataChunk &output) {
	auto &state = data.global_state->Cast<TeradataQueryState>();

	// Scan the query result.
	state.td_query->Scan(output);
}

//----------------------------------------------------------------------------------------------------------------------
// Register
//----------------------------------------------------------------------------------------------------------------------
TableFunction TeradataQueryFunction::GetFunction() {
	TableFunction function;
	function.name = "teradata_query";
	function.arguments = {LogicalType::VARCHAR, LogicalType::VARCHAR};

	function.bind = TeradataQueryBind;
	function.init_global = TeradataQueryInit;
	function.function = TeradataQueryExec;
	return function;
}

void TeradataQueryFunction::Register(DatabaseInstance &db) {
	const auto function = TeradataQueryFunction::GetFunction();
	ExtensionUtil::RegisterFunction(db, function);
}

} // namespace duckdb