#pragma once
#include "teradata_catalog.hpp"
#include "teradata_type.hpp"

namespace duckdb {

class DatabaseInstance;

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

private:
	optional_ptr<TeradataCatalog> catalog;
};

struct TeradataQueryFunction {
	static TableFunction GetFunction();
	static void Register(DatabaseInstance &db);
};

} // namespace duckdb