#include "teradata_execute.hpp"
#include "teradata_catalog.hpp"
#include "teradata_transaction.hpp"

#include "duckdb/function/function.hpp"
#include "duckdb/main/attached_database.hpp"
#include "duckdb/main/database_manager.hpp"
#include "duckdb/main/extension/extension_loader.hpp"
#include "duckdb/transaction/meta_transaction.hpp"

namespace duckdb {

//----------------------------------------------------------------------------------------------------------------------
// Bind
//----------------------------------------------------------------------------------------------------------------------
struct TeradataExecuteBindData final : public TableFunctionData {
	TeradataExecuteBindData(TeradataCatalog &td_catalog, string query_p, bool use_transaction)
	    : td_catalog(td_catalog), query(std::move(query_p)), use_transaction(use_transaction) {
	}

	bool finished = false;
	TeradataCatalog &td_catalog;
	string query;
	//! Run inside the DuckDB transaction's Teradata transaction (BEGIN/END TRANSACTION), or on its own
	bool use_transaction;
};

static bool GetUseTransaction(const named_argument_map_t &named_parameters) {
	const auto entry = named_parameters.find(Identifier("transaction"));
	return entry == named_parameters.end() || entry->second.IsNull() || entry->second.GetValue<bool>();
}

static unique_ptr<FunctionData> Bind(ClientContext &context, TableFunctionBindInput &input,
                                     vector<LogicalType> &return_types, vector<Identifier> &names) {
	return_types.emplace_back(LogicalType::BOOLEAN);
	names.emplace_back("Success");

	auto &td_catalog = TeradataCatalog::Find(context, input.inputs[0].GetValue<string>(), "teradata_execute");
	return make_uniq<TeradataExecuteBindData>(td_catalog, input.inputs[1].GetValue<string>(),
	                                          GetUseTransaction(input.named_parameters));
}

//----------------------------------------------------------------------------------------------------------------------
// Execute
//----------------------------------------------------------------------------------------------------------------------
static void Execute(ClientContext &context, TableFunctionInput &data_p, DataChunk &output) {
	auto &data = data_p.bind_data->CastNoConst<TeradataExecuteBindData>();
	if (data.finished) {
		return;
	}

	if (data.use_transaction) {
		const auto &transaction = TeradataTransaction::Get(context, data.td_catalog);
		transaction.GetConnection().Execute(data.query, context);
	} else {
		// The session is shared: if this DuckDB transaction already opened a Teradata transaction, the statement
		// would run inside it
		if (MetaTransaction::Get(context).TryGetTransaction(data.td_catalog.GetAttached())) {
			throw InvalidInputException(
			    "teradata_execute with transaction := false cannot run after other statements on the same Teradata "
			    "database in this transaction. Run it as its own statement, outside BEGIN ... COMMIT.");
		}
		// Teradata commits the statement on its own (implicit transaction)
		data.td_catalog.GetConnection().Execute(data.query, context);
	}
	data.finished = true;
}

//----------------------------------------------------------------------------------------------------------------------
// Register
//----------------------------------------------------------------------------------------------------------------------

void TeradataExecuteFunction::Register(ExtensionLoader &loader) {
	TableFunction func("teradata_execute", {LogicalType::VARCHAR, LogicalType::VARCHAR}, Execute, Bind);
	func.GetSignature().AddKeywordOnly("transaction", LogicalType::BOOLEAN, Value::BOOLEAN(true));
	loader.RegisterFunction(func);
}

} // namespace duckdb
