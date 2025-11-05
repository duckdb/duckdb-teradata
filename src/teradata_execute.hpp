#pragma once

namespace duckdb {

class ExtensionLoader;

struct TeradataExecuteFunction {
	static void Register(ExtensionLoader &loader);
};

} // namespace duckdb
