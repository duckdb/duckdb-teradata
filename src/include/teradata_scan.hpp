#pragma once

namespace duckdb {

class DatabaseInstance;

struct TeradataScan {
	static void Register(DatabaseInstance &db);
};

}