#pragma once

#include "duckdb.hpp"

namespace duckdb {

class ExtensionLoader;

struct TeradataSecret {
	static const char *TYPE;

	static void Register(ExtensionLoader &loader);
	static bool TryGet(ClientContext &context, const string &name, TeradataSecret &out);
	//! Validate CREATE SECRET options: exactly the known keys (case-insensitive), all of them required.
	//! Returns them with lowercase keys.
	static case_insensitive_map_t<string> ParseOptions(const case_insensitive_map_t<Value> &options);

	string host;
	string user;
	string database;
	string password;
};

} // namespace duckdb
