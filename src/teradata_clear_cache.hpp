#pragma once

namespace duckdb {

class ExtensionLoader;
class ClientContext;

struct TeradataClearCacheFunction {
	static void Clear(ClientContext &context);
	static void Register(ExtensionLoader &loader);
};

} // namespace duckdb
