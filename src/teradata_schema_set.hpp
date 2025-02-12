#pragma once

#include "teradata_catalog_set.hpp"

namespace duckdb {

// Set of schemas in the catalog
class TeradataSchemaSet final : public TeradataCatalogSet {
public:
    explicit TeradataSchemaSet(Catalog &catalog) : TeradataCatalogSet(catalog) {
	}

protected:
	// Load all schemas in this catalog
	void LoadEntries(ClientContext &context) override;
};

}