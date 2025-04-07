#include "teradata_table_entry.hpp"
#include "teradata_catalog.hpp"
#include "teradata_transaction.hpp"
#include "teradata_query.hpp"

#include "duckdb/function/table_function.hpp"
#include "duckdb/storage/table_storage_info.hpp"

namespace duckdb {

TeradataTableEntry::TeradataTableEntry(Catalog &catalog, SchemaCatalogEntry &schema, CreateTableInfo &info)
    : TableCatalogEntry(catalog, schema, info) {

	for (auto &col : columns.Logical()) {
		// Use the default mapping from DuckDB to Teradata types if were creating a table from within duckdb
		teradata_types.push_back(TeradataType::FromDuckDB(col.GetType()));
	}
}

TeradataTableEntry::TeradataTableEntry(Catalog &catalog, SchemaCatalogEntry &schema, TeradataTableInfo &info)
    : TableCatalogEntry(catalog, schema, info), teradata_types(std::move(info.teradata_types)) {
	D_ASSERT(teradata_types.size() == columns.LogicalColumnCount());
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
	result->SetTable(*this);

	for (idx_t col_idx = 0; col_idx < columns.LogicalColumnCount(); col_idx++) {
		auto &col = columns.GetColumnMutable(LogicalIndex(col_idx));
		result->names.push_back(col.GetName());
		result->types.push_back(col.GetType());
	}

	result->is_read_only = transaction.IsReadOnly();
	result->is_materialized = false;
	result->td_types = teradata_types;

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