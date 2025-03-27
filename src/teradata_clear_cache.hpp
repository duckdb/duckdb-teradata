#pragma once

namespace duckdb {

class DatabaseInstance;
class ClientContext;

struct TeradataClearCacheFunction {
	static void Clear(ClientContext &context);
	static void Register(DatabaseInstance &db);
};

} // namespace duckdb