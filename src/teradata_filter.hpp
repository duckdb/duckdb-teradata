#pragma once

#include "duckdb.hpp"

namespace duckdb {

class TableFilterSet;

class TeradataFilter {
public:
	static string Transform(const vector<column_t> &column_ids, optional_ptr<TableFilterSet> filters,
	                        const vector<string> &names);
};

} // namespace duckdb
