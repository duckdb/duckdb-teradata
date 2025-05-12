#include "duckdb/main/database_manager.hpp"
#include "duckdb/parser/parsed_data/create_table_function_info.hpp"
#include "duckdb/main/attached_database.hpp"

#include "teradata_query.hpp"
#include "teradata_catalog.hpp"
#include "teradata_transaction.hpp"
#include "teradata_table_entry.hpp"

#include "duckdb/main/extension_util.hpp"

#include <teradata_filter.hpp>

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
	auto &transaction = TeradataTransaction::Get(context, catalog);

	// Strip any trailing semicolons
	auto sql = input.inputs[1].GetValue<string>();
	StringUtil::RTrim(sql);
	while (!sql.empty() && sql.back() == ';') {
		sql = sql.substr(0, sql.size() - 1);
		StringUtil::RTrim(sql);
	}

	auto &con = transaction.GetConnection();

	vector<TeradataType> td_types;
	con.Prepare(sql, td_types, names);

	// Convert to DuckDB types
	for (auto &td_type : td_types) {
		return_types.push_back(td_type.ToDuckDB());
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
// Bind Info
//----------------------------------------------------------------------------------------------------------------------
static BindInfo TeradataQueryBindInfo(const optional_ptr<FunctionData> bind_data_p) {
	auto &bind_data = bind_data_p->Cast<TeradataBindData>();
	BindInfo info(ScanType::EXTERNAL);
	info.table = bind_data.GetTable().get();
	return info;
}

//----------------------------------------------------------------------------------------------------------------------
// Init
//----------------------------------------------------------------------------------------------------------------------
struct TeradataQueryState final : GlobalTableFunctionState {
	unique_ptr<TeradataQueryResult> td_query;
	DataChunk scan_chunk;
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

	// Also add simple filters if we got them
	if (input.filters) {
		const auto where_clause = TeradataFilter::Transform(input.column_ids, *input.filters, data.names);
		if (!where_clause.empty()) {
			sql += " WHERE " + where_clause;
		}
	}

	auto result = make_uniq<TeradataQueryState>();

	auto &con = data.GetCatalog()->GetConnection();
	result->td_query = con.Query(sql, data.is_materialized);

	// Check that the types are still the same, in case we need to rebind
	auto &td_types = result->td_query->GetTypes();

	for (idx_t i = 0; i < td_types.size(); i++) {
		// Compare the types of the query with the types we got during binding
		auto &expected = data.td_types[i];
		auto &actual = td_types[i];

		if (actual != expected) {
			if (expected.GetId() == TeradataTypeId::TIMESTAMP && actual.GetId() == TeradataTypeId::CHAR) {

				const auto is_ts_s = expected.GetWidth() == 6 && actual.GetLength() == 26;
				const auto is_ts_ms = expected.GetWidth() == 3 && actual.GetLength() == 23;
				const auto is_ts_us = expected.GetWidth() == 0 && actual.GetLength() == 19;

				if (is_ts_s || is_ts_ms || is_ts_us) {
					// Special case, this gets cast later
					continue;
				}
			}

			if (expected.GetId() == TeradataTypeId::TIMESTAMP_TZ && actual.GetId() == TeradataTypeId::CHAR &&
			    actual.GetLength() == 32) {
				// Special case, this gets cast later
				continue;
			}

			if (expected.GetId() == TeradataTypeId::TIME && actual.GetId() == TeradataTypeId::CHAR &&
			    actual.GetLength() == 15) {
				// Special case, this gets cast later
				continue;
			}

			if (expected.GetId() == TeradataTypeId::TIME_TZ && actual.GetId() == TeradataTypeId::CHAR &&
			    actual.GetLength() == 21) {
				// Special case, this gets cast later
				continue;
			}

			throw InvalidInputException("Teradata query schema has changed since it was last bound!\n"
			                            "Column: '%s' expected to be of type '%s' but received '%s'\n"
			                            "Please re-execute or re-prepare the query",
			                            data.names[i], expected.ToString(), actual.ToString());
		}
	}

	// Initialize the scan chunk
	result->td_query->InitScanChunk(result->scan_chunk);

	return std::move(result);
}

//----------------------------------------------------------------------------------------------------------------------
// Execute
//----------------------------------------------------------------------------------------------------------------------
static void TeradataQueryExec(ClientContext &context, TableFunctionInput &data, DataChunk &output) {
	auto &state = data.global_state->Cast<TeradataQueryState>();

	// Scan the query result.
	state.td_query->Scan(state.scan_chunk);

	// Get the scan count
	const auto count = state.scan_chunk.size();

	// Cast all vectors
	// For most types, this is a no-op, the target just references the source.
	// But there are some special cases, like TIMESTAMP that always gets transmitted as VARCHAR.
	for (idx_t col_idx = 0; col_idx < state.scan_chunk.ColumnCount(); col_idx++) {
		auto &source = state.scan_chunk.data[col_idx];
		auto &target = output.data[col_idx];

		VectorOperations::DefaultCast(source, target, count);
	}

	output.SetCardinality(count);
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
	function.get_bind_info = TeradataQueryBindInfo;
	function.projection_pushdown = false;
	function.filter_pushdown = true;

	return function;
}

void TeradataQueryFunction::Register(DatabaseInstance &db) {
	const auto function = TeradataQueryFunction::GetFunction();
	ExtensionUtil::RegisterFunction(db, function);
}

} // namespace duckdb