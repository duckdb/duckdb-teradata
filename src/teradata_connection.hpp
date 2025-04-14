#pragma once

#include "teradata_common.hpp"
#include "teradata_result.hpp"

#include "duckdb/common/string.hpp"
#include "duckdb/common/vector.hpp"
#include "duckdb/common/types/data_chunk.hpp"

namespace duckdb {

class TeradataColumnWriter;

class TeradataConnection {
public:
	explicit TeradataConnection(const string &logon_string_p) {
		SetLogonString(logon_string_p);
		Reconnect();
	}

	~TeradataConnection() {
		Disconnect();
	}

	Int32 GetSessionId() const {
		return session_id;
	}

	void SetLogonString(const string &logon_string_p) {
		logon_string = logon_string_p;
	}

	const string &GetLogonString() const {
		return logon_string;
	}

	void Reconnect();
	void Disconnect();

	void Execute(const string &sql);

	// Execute a parameterized statement, once for each row in the chunk
	// TODO: Move arena and writers into a ExecuteState class
	void Execute(const string &sql, DataChunk &chunk, ArenaAllocator &arena,
	             vector<unique_ptr<TeradataColumnWriter>> &writers);

	// Execute a query with a result set, optionally materializing everything
	unique_ptr<TeradataQueryResult> Query(const string &sql, bool materialize);

	// Prepare a query
	void Prepare(const string &sql, vector<TeradataType> &types, vector<string> &names);

private:
	string logon_string;
	Int32 session_id = 0;
	bool is_connected = false;
	DBCAREA dbc;
	char cnta[4];
};

} // namespace duckdb