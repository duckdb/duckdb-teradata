#pragma once

namespace duckdb {

class DatabaseInstance;

struct TeradataQueryFunction {
	static void Register(DatabaseInstance &db);
};

}