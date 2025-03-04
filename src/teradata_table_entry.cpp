#include "teradata_table_entry.hpp"
#include "teradata_catalog.hpp"
#include "teradata_transaction.hpp"
#include "teradata_query.hpp"

#include "duckdb/function/table_function.hpp"
#include "duckdb/storage/table_storage_info.hpp"


namespace duckdb {

TeradataTableEntry::TeradataTableEntry(Catalog &catalog, SchemaCatalogEntry &schema, CreateTableInfo &info)
	: TableCatalogEntry(catalog, schema, info) {
}

unique_ptr<BaseStatistics> TeradataTableEntry::GetStatistics(ClientContext &context, column_t column_id) {
	return nullptr;
}

TableFunction TeradataTableEntry::GetScanFunction(ClientContext &context, unique_ptr<FunctionData> &bind_data) {
	auto &td_catalog = catalog.Cast<TeradataCatalog>();
	auto &transaction = Transaction::Get(context, catalog).Cast<TeradataTransaction>();

	auto result = make_uniq<TeradataBindData>();

	// Setup the bind data
	result->schema_name = schema.name;
	result->table_name = name;
	result->SetCatalog(td_catalog);

	for(idx_t col_idx = 0; col_idx < columns.LogicalColumnCount(); col_idx++) {
		auto &col = columns.GetColumnMutable(LogicalIndex(col_idx));
		result->names.push_back(col.GetName());
		result->types.push_back(col.GetType());
	}

	result->is_read_only = transaction.IsReadOnly();

	// Set the bind data
	bind_data = std::move(result);

	// Return the table function
	auto function = TeradataQueryFunction::GetFunction();
	return function;
}

TableStorageInfo TeradataTableEntry::GetStorageInfo(ClientContext &context) {
	// TODO;
	TableStorageInfo info;
	return info;
}

} // namespace duckdb