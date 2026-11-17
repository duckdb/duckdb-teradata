#pragma once

#include "teradata_catalog_set.hpp"
#include "teradata_table_entry.hpp"

namespace duckdb {

struct BoundCreateTableInfo;

//! One row of the column catalog query: a column of a table in the schema
struct TeradataColumnRow {
	string table_name;
	string column_name;
	//! The two-letter type code from DBC.ColumnsV, e.g. "CV"
	string type_code;
	int32_t length;
};

//! Group column rows, ordered by table, into table definitions. Tables with an unsupported column type are skipped.
vector<unique_ptr<TeradataTableInfo>> BuildTableInfos(const Identifier &schema_name,
                                                      const vector<TeradataColumnRow> &rows);

//! The Teradata CREATE TABLE statement for a DuckDB table definition
string GetTeradataCreateTableSQL(const CreateTableInfo &info, bool use_primary_index);

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