#pragma once
#include "teradata_catalog.hpp"
#include "teradata_type.hpp"

namespace duckdb {

class DatabaseInstance;
class ExtensionLoader;
class TeradataTableEntry;

class TeradataBindData final : public TableFunctionData {
public:
	string schema_name;
	string table_name;
	string sql;

	vector<LogicalType> types;
	vector<string> names;
	vector<TeradataType> td_types;

	bool is_read_only = false;
	bool is_materialized = false;

	void SetCatalog(TeradataCatalog &catalog) {
		this->catalog = &catalog;
	}
	optional_ptr<TeradataCatalog> GetCatalog() const {
		return catalog;
	}
	void SetTable(TeradataTableEntry &table) {
		this->table = &table;
	}
	optional_ptr<TeradataTableEntry> GetTable() const {
		return table;
	}

private:
	optional_ptr<TeradataCatalog> catalog;
	optional_ptr<TeradataTableEntry> table;
};

//! The SQL text without trailing whitespace and semicolons
string StripTrailingSemicolons(string sql);

//! The SQL a scan sends: the bound query, or a SELECT from the bound table, with the pushed-down WHERE clause
string BuildScanSQL(const string &sql, const string &schema_name, const string &table_name, const string &where_clause);

//! True if a column bound as `expected` may arrive as `actual`: Teradata transmits some temporal types as CHAR
bool IsTransmittedAs(const TeradataType &expected, const TeradataType &actual);

//! Throws if a query result no longer has the types the query was bound with
void CheckBoundTypes(const vector<string> &names, const vector<TeradataType> &expected,
                     const vector<TeradataType> &actual);

struct TeradataQueryFunction {
	static TableFunction GetFunction();
	static void Register(ExtensionLoader &loader);
};

} // namespace duckdb
