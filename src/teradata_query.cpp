#include "duckdb/main/database_manager.hpp"
#include "duckdb/parser/parsed_data/create_table_function_info.hpp"
#include "duckdb/main/attached_database.hpp"

#include "teradata_query.hpp"
#include "teradata_catalog.hpp"
#include "teradata_transaction.hpp"
#include "teradata_table_entry.hpp"

#include "duckdb/main/extension/extension_loader.hpp"

#include "teradata_filter.hpp"

#include <algorithm>

namespace duckdb {

//----------------------------------------------------------------------------------------------------------------------
// SQL helpers
//----------------------------------------------------------------------------------------------------------------------
string StripTrailingSemicolons(string sql) {
	const auto end = sql.find_last_not_of(" \t\n\r\f\v;");
	return end == string::npos ? string() : sql.substr(0, end + 1);
}

string BuildScanSQL(const string &sql, const string &schema_name, const string &table_name,
                    const string &where_clause) {
	if (sql.empty() && table_name.empty()) {
		throw InvalidInputException("Teradata query requires a valid SQL string");
	}
	const auto select = sql.empty() ? StringUtil::Format("SELECT * FROM %s.%s", schema_name, table_name) : sql;
	return where_clause.empty() ? select : select + " WHERE " + where_clause;
}

namespace {

//! A temporal type that Teradata transmits as CHAR(char_length)
struct CharTransmission {
	static constexpr int64_t ANY_WIDTH = -1;

	TeradataTypeId id;
	int64_t width;
	int64_t char_length;
};

const CharTransmission CHAR_TRANSMISSIONS[] = {
    {TeradataTypeId::TIMESTAMP, 0, 19},
    {TeradataTypeId::TIMESTAMP, 3, 23},
    {TeradataTypeId::TIMESTAMP, 6, 26},
    {TeradataTypeId::TIMESTAMP_TZ, CharTransmission::ANY_WIDTH, 32},
    {TeradataTypeId::TIME, CharTransmission::ANY_WIDTH, 15},
    {TeradataTypeId::TIME_TZ, CharTransmission::ANY_WIDTH, 21},
};

} // namespace

bool IsTransmittedAs(const TeradataType &expected, const TeradataType &actual) {
	if (expected == actual) {
		return true;
	}
	if (actual.GetId() != TeradataTypeId::CHAR) {
		return false;
	}
	return std::any_of(std::begin(CHAR_TRANSMISSIONS), std::end(CHAR_TRANSMISSIONS), [&](const CharTransmission &t) {
		return t.id == expected.GetId() && (t.width == CharTransmission::ANY_WIDTH || t.width == expected.GetWidth()) &&
		       t.char_length == actual.GetLength();
	});
}

void CheckBoundTypes(const vector<string> &names, const vector<TeradataType> &expected,
                     const vector<TeradataType> &actual) {
	// A result without data (no DATAINFO parcel) carries no column types to compare
	if (!actual.empty() && actual.size() != expected.size()) {
		throw InvalidInputException("Teradata query schema has changed since it was last bound!\n"
		                            "Expected %d column(s) but received %d\n"
		                            "Please re-execute or re-prepare the query",
		                            expected.size(), actual.size());
	}
	for (idx_t i = 0; i < actual.size(); i++) {
		if (!IsTransmittedAs(expected[i], actual[i])) {
			throw InvalidInputException("Teradata query schema has changed since it was last bound!\n"
			                            "Column: '%s' expected to be of type '%s' but received '%s'\n"
			                            "Please re-execute or re-prepare the query",
			                            names[i], expected[i].ToString(), actual[i].ToString());
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
// Bind
//----------------------------------------------------------------------------------------------------------------------
static unique_ptr<FunctionData> TeradataQueryBind(ClientContext &context, TableFunctionBindInput &input,
                                                  vector<LogicalType> &return_types, vector<Identifier> &names) {

	if (input.inputs[0].IsNull() || input.inputs[1].IsNull()) {
		throw BinderException("Parameters to teradata_query cannot be NULL");
	}

	auto &td_catalog = TeradataCatalog::GetAttached(context, input.inputs[0].GetValue<string>(), "teradata_query");
	auto &transaction = TeradataTransaction::Get(context, td_catalog);
	const auto sql = StripTrailingSemicolons(input.inputs[1].GetValue<string>());

	auto &con = transaction.GetConnection();

	vector<TeradataType> td_types;
	vector<string> column_names;
	con.Prepare(sql, td_types, column_names, context);
	for (auto &column_name : column_names) {
		names.emplace_back(column_name);
	}

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
	result->names = std::move(column_names);
	result->sql = sql;
	result->SetCatalog(td_catalog);

	return std::move(result);
}

//----------------------------------------------------------------------------------------------------------------------
// Bind Info
//----------------------------------------------------------------------------------------------------------------------
static BindInfo TeradataQueryBindInfo(TableFunctionGetBindInfoInput &input) {
	return BindInfo(ScanType::EXTERNAL);
}

static optional_ptr<TableCatalogEntry> TeradataQueryGetTableEntry(optional_ptr<const FunctionData> bind_data_p) {
	if (!bind_data_p) {
		return nullptr;
	}
	auto &bind_data = bind_data_p->Cast<TeradataBindData>();
	return bind_data.GetTable().get();
}

//----------------------------------------------------------------------------------------------------------------------
// Init
//----------------------------------------------------------------------------------------------------------------------
struct TeradataQueryState final : GlobalTableFunctionState {
	unique_ptr<TeradataQueryResult> td_query;
	DataChunk scan_chunk;
	vector<column_t> column_ids;
};

static unique_ptr<GlobalTableFunctionState> TeradataQueryInit(ClientContext &context, TableFunctionInitInput &input) {
	auto &data = input.bind_data->Cast<TeradataBindData>();

	const auto where_clause =
	    input.filters ? TeradataFilter::Transform(input.column_ids, *input.filters, data.names) : string();
	const auto sql = BuildScanSQL(data.sql, data.schema_name, data.table_name, where_clause);

	auto result = make_uniq<TeradataQueryState>();
	result->column_ids = input.column_ids;

	auto &con = data.GetCatalog()->GetConnection();
	result->td_query = con.Query(sql, data.is_materialized, context);

	// Check that the types are still the same, in case we need to rebind
	CheckBoundTypes(data.names, data.td_types, result->td_query->GetTypes());

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
	for (idx_t output_idx = 0; output_idx < state.scan_chunk.ColumnCount(); output_idx++) {
		const auto col_idx = state.column_ids[output_idx];

		if (col_idx == COLUMN_IDENTIFIER_ROW_ID) {
			throw InvalidInputException("Teradata query does not support the row id column (rowid)");
		}

		auto &source = state.scan_chunk.data[col_idx]; // col ids
		auto &target = output.data[output_idx];

		VectorOperations::DefaultCast(source, target, count);
	}

	output.SetCardinality(count);
}

//----------------------------------------------------------------------------------------------------------------------
// Register
//----------------------------------------------------------------------------------------------------------------------
TableFunction TeradataQueryFunction::GetFunction() {
	TableFunction function("teradata_query", {LogicalType::VARCHAR, LogicalType::VARCHAR}, TeradataQueryExec,
	                       TeradataQueryBind, TeradataQueryInit);
	function.get_bind_info = TeradataQueryBindInfo;
	function.get_table_entry = TeradataQueryGetTableEntry;
	function.projection_pushdown = true;
	function.filter_pushdown = true;

	return function;
}

void TeradataQueryFunction::Register(ExtensionLoader &loader) {
	const auto function = TeradataQueryFunction::GetFunction();
	loader.RegisterFunction(function);
}

} // namespace duckdb
