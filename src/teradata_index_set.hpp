#pragma once

#include "teradata_catalog_set.hpp"
#include "teradata_index_entry.hpp"

namespace duckdb {

class TeradataSchemaEntry;
class TableCatalogEntry;

class TeradataIndexSet final : public TeradataInSchemaSet {
public:
	explicit TeradataIndexSet(TeradataSchemaEntry &schema);

public:
	optional_ptr<CatalogEntry> CreateIndex(ClientContext &context, CreateIndexInfo &info, TableCatalogEntry &table);

protected:
	void LoadEntries(ClientContext &context) override;
};

} // namespace duckdb