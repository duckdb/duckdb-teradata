#include "teradata_execute.hpp"
#include "teradata_catalog.hpp"
#include "teradata_transaction.hpp"

#include "duckdb/function/function.hpp"
#include "duckdb/main/attached_database.hpp"
#include "duckdb/main/database_manager.hpp"
#include "duckdb/main/extension/extension_loader.hpp"

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
                                     vector<LogicalType> &return_types, vector<Identifier> &names) {
	return_types.emplace_back(LogicalType::BOOLEAN);
	names.emplace_back("Success");

	auto &td_catalog = TeradataCatalog::GetAttached(context, input.inputs[0].GetValue<string>(), "teradata_execute");
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
	transaction.GetConnection().Execute(data.query, context);
	data.finished = true;
}

//----------------------------------------------------------------------------------------------------------------------
// Register
//----------------------------------------------------------------------------------------------------------------------

void TeradataExecuteFunction::Register(ExtensionLoader &loader) {
	TableFunction func("teradata_execute", {LogicalType::VARCHAR, LogicalType::VARCHAR}, Execute, Bind);
	loader.RegisterFunction(func);
}

} // namespace duckdb
