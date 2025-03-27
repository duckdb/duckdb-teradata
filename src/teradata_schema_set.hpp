#pragma once

#include "teradata_catalog_set.hpp"

namespace duckdb {

// Set of schemas in the catalog
class TeradataSchemaSet final : public TeradataCatalogSet {
public:
	explicit TeradataSchemaSet(Catalog &catalog, string schema_to_load);

protected:
	// Load all schemas in this catalog
	void LoadEntries(ClientContext &context) override;

	//! Schema to load - if empty loads all schemas (default behavior)
	string schema_to_load;
};

} // namespace duckdb