#pragma once

namespace duckdb {

class ExtensionLoader;

struct TeradataSecret {
	static const char *TYPE;

	static void Register(ExtensionLoader &loader);
	static bool TryGet(ClientContext &context, const string &name, TeradataSecret &out);

	string host;
	string user;
	string database;
	string password;
};

} // namespace duckdb
