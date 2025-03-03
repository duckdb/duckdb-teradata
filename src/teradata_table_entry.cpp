#include "teradata_table_entry.hpp"

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
	// TODO;
	throw NotImplementedException("TeradataTableEntry::GetScanFunction");
}

TableStorageInfo TeradataTableEntry::GetStorageInfo(ClientContext &context) {
	// TODO;
	TableStorageInfo info;
	return info;
}

} // namespace duckdb