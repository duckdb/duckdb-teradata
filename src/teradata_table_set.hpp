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

//! Groups column rows, arriving ordered by table, into table definitions as they come in, so that only the current
//! table's columns are held. Tables with an unsupported column type are skipped.
class TeradataTableInfoBuilder {
public:
	explicit TeradataTableInfoBuilder(Identifier schema_name);

	void Add(const TeradataColumnRow &row);
	//! The definitions of all supported tables added so far
	vector<unique_ptr<TeradataTableInfo>> Finish();

private:
	void StartTable(const string &table_name);
	void FinishTable();

	Identifier schema_name;
	vector<unique_ptr<TeradataTableInfo>> completed;
	string current_table;
	//! The table being built; null while skipping a table with an unsupported column
	unique_ptr<TeradataTableInfo> current;
	bool started = false;
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