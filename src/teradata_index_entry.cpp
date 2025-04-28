#include "teradata_index_entry.hpp"
#include "duckdb/catalog/catalog_entry/schema_catalog_entry.hpp"

namespace duckdb {

TeradataIndexEntry::TeradataIndexEntry(Catalog &catalog, SchemaCatalogEntry &schema, CreateIndexInfo &info,
                                       string table_name)
    : IndexCatalogEntry(catalog, schema, info), table_name(std::move(table_name)) {
}

string TeradataIndexEntry::GetSchemaName() const {
	return schema.name;
}

string TeradataIndexEntry::GetTableName() const {
	return table_name;
}

} // namespace duckdb