#include "teradata_catalog_set.hpp"
#include "teradata_schema_entry.hpp"

#include "duckdb/catalog/catalog.hpp"

namespace duckdb {
TeradataCatalogSet::TeradataCatalogSet(Catalog &catalog) : catalog(catalog) {

}

void TeradataCatalogSet::TryLoadEntries(ClientContext &context) {
	lock_guard<mutex> lock(load_lock);
	if (is_loaded) {
		return;
	}
	LoadEntries(context);
	is_loaded = true;
}

void TeradataCatalogSet::Scan(ClientContext &context, const std::function<void(CatalogEntry &)> &callback) {
	TryLoadEntries(context);
	// TODO: lock this too
	for (auto &entry : entries) {
		callback(*entry.second);
	}
}

TeradataInSchemaSet::TeradataInSchemaSet(TeradataSchemaEntry &schema)
	: TeradataCatalogSet(schema.ParentCatalog()), schema(schema) {
}

} // namespace duckdb