#pragma once

namespace duckdb {

class DatabaseInstance;

struct TeradataExecuteFunction {
	static void Register(DatabaseInstance &db);
};

} // namespace duckdb