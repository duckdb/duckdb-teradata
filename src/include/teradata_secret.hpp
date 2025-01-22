#pragma once

namespace duckdb {

class DatabaseInstance;

struct TeradataSecret {
	static constexpr auto TYPE = "teradata";

	static void Register(DatabaseInstance &db);
	static bool TryGet(ClientContext &context, const string &name, TeradataSecret &out);

	string host;
	string user;
	string database;
	string password;
};

} // namespace duckdb