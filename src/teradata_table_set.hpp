#pragma once

#include "teradata_catalog_set.hpp"

namespace duckdb {

class Catalog;

class TeradataTableSet final : public TeradataInSchemaSet {
public:
	explicit TeradataTableSet(TeradataSchemaEntry &schema) : TeradataInSchemaSet(schema) {
	}
protected:
	void LoadEntries(ClientContext &context) override;
};

} // namespace duckdb