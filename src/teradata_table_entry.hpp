#pragma once

#include "teradata_type.hpp"
#include "duckdb/catalog/catalog_entry/table_catalog_entry.hpp"
#include "duckdb/parser/parsed_data/create_table_info.hpp"

namespace duckdb {

class TeradataTableInfo final : public CreateTableInfo {
public:
	// If we are populating a table entry from an existing teradata table, we also pass on the teradata types
	vector<TeradataType> teradata_types;
};

class TeradataTableEntry final : public TableCatalogEntry {
public:
	// Create a Teradata table entry from within DuckDB
	TeradataTableEntry(Catalog &catalog, SchemaCatalogEntry &schema, CreateTableInfo &info);

	// Create a Teradata table entry by providing data gathered from Teradata
	TeradataTableEntry(Catalog &catalog, SchemaCatalogEntry &schema, TeradataTableInfo &info);

	// Get the statistics of a specific column
	unique_ptr<BaseStatistics> GetStatistics(ClientContext &context, column_t column_id) override;

	// Get the scan function of the table
	TableFunction GetScanFunction(ClientContext &context, unique_ptr<FunctionData> &bind_data) override;

	//! Returns the storage info of this table
	TableStorageInfo GetStorageInfo(ClientContext &context) override;

public:
	// The raw teradata typess
	vector<TeradataType> teradata_types;
};

} // namespace duckdb