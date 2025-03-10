#pragma once

#include "teradata_type.hpp"
#include "duckdb/catalog/catalog_entry/table_catalog_entry.hpp"

namespace duckdb {

class TeradataTableEntry final : public TableCatalogEntry {
public:
	TeradataTableEntry(Catalog &catalog, SchemaCatalogEntry &schema, CreateTableInfo &info);

	// Get the statistics of a specific column
	unique_ptr<BaseStatistics> GetStatistics(ClientContext &context, column_t column_id) override;

	// Get the scan function of the table
	TableFunction GetScanFunction(ClientContext &context, unique_ptr<FunctionData> &bind_data) override;

	//! Returns the storage info of this table
	TableStorageInfo GetStorageInfo(ClientContext &context) override;
};

} // namespace duckdb