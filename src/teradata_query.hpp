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

struct TeradataQueryFunction {
	static TableFunction GetFunction();
	static void Register(ExtensionLoader &loader);
};

} // namespace duckdb
