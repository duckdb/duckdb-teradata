#include "teradata_execute.hpp"
#include "teradata_catalog.hpp"
#include "teradata_transaction.hpp"

#include "duckdb/function/function.hpp"
#include "duckdb/main/extension_util.hpp"

namespace duckdb {

//----------------------------------------------------------------------------------------------------------------------
// Bind
//----------------------------------------------------------------------------------------------------------------------
struct TeradataExecuteBindData final : public TableFunctionData {
	TeradataExecuteBindData(TeradataCatalog &td_catalog, string query_p)
	    : td_catalog(td_catalog), query(std::move(query_p)) {
	}

	bool finished = false;
	TeradataCatalog &td_catalog;
	string query;
};

static unique_ptr<FunctionData> Bind(ClientContext &context, TableFunctionBindInput &input,
                                     vector<LogicalType> &return_types, vector<string> &names) {
	return_types.emplace_back(LogicalType::BOOLEAN);
	names.emplace_back("Success");

	// look up the database to query
	auto db_name = input.inputs[0].GetValue<string>();
	auto &db_manager = DatabaseManager::Get(context);
	auto db = db_manager.GetDatabase(context, db_name);
	if (!db) {
		throw BinderException("Failed to find attached database \"%s\" referenced in teradata_execute", db_name);
	}
	auto &catalog = db->GetCatalog();
	if (catalog.GetCatalogType() != "teradata") {
		throw BinderException("Attached database \"%s\" does not refer to a Teradata database", db_name);
	}
	auto &td_catalog = catalog.Cast<TeradataCatalog>();
	return make_uniq<TeradataExecuteBindData>(td_catalog, input.inputs[1].GetValue<string>());
}

//----------------------------------------------------------------------------------------------------------------------
// Execute
//----------------------------------------------------------------------------------------------------------------------
static void Execute(ClientContext &context, TableFunctionInput &data_p, DataChunk &output) {
	auto &data = data_p.bind_data->CastNoConst<TeradataExecuteBindData>();
	if (data.finished) {
		return;
	}

	const auto &transaction = TeradataTransaction::Get(context, data.td_catalog);
	transaction.GetConnection().Execute(data.query);
	data.finished = true;
}

//----------------------------------------------------------------------------------------------------------------------
// Register
//----------------------------------------------------------------------------------------------------------------------

void TeradataExecuteFunction::Register(DatabaseInstance &db) {
	TableFunction func("teradata_execute", {LogicalType::VARCHAR, LogicalType::VARCHAR}, Execute, Bind);
	ExtensionUtil::RegisterFunction(db, func);
}

} // namespace duckdb