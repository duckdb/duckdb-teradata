#pragma once

#include "teradata_catalog_set.hpp"

namespace duckdb {

struct BoundCreateTableInfo;

class Catalog;

class TeradataTableSet final : public TeradataInSchemaSet {
public:
	explicit TeradataTableSet(TeradataSchemaEntry &schema) : TeradataInSchemaSet(schema) {
	}

	optional_ptr<CatalogEntry> CreateTable(ClientContext &context, BoundCreateTableInfo &info);
protected:
	void LoadEntries(ClientContext &context) override;
};

} // namespace duckdb