#pragma once

#include "duckdb/catalog/catalog_entry/index_catalog_entry.hpp"

namespace duckdb {

class TeradataIndexEntry final : public IndexCatalogEntry {
public:
	TeradataIndexEntry(Catalog &catalog, SchemaCatalogEntry &schema, CreateIndexInfo &info, string table_name);

public:
	Identifier GetSchemaName() const override;
	Identifier GetTableName() const override;

private:
	string table_name;
};

} // namespace duckdb