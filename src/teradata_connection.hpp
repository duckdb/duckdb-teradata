#pragma once

#include "duckdb/common/string.hpp"
#include "duckdb/common/vector.hpp"
#include "duckdb/common/types/data_chunk.hpp"

#include "teradata_common.hpp"

namespace duckdb {

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

private:
	string logon_string;
	Int32 session_id = 0;
	bool is_connected = false;
	DBCAREA dbc;
	char cnta[4];
};

} // namespace duckdb