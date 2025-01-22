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

/*
enum class TeradataRequestState : uint8_t { READY, OPEN, CLOSED };


class TeradataRequest {
public:
    ~TeradataRequest() {
        // Close();
    }

    void PrepareSqlRequest(const string &query, vector<string> &names, vector<LogicalType> &types);

    void BeginSqlRequest(const string &query);
    bool FetchSqlRequest(DataChunk &chunk);
    void EndRequest();


    void Close() {
        if(state != TeradataRequestState::OPEN) {
            // We can only close an empty request
            return;
        }
        Int32 result = EM_OK;
        dbc.func = DBFERQ;
        DBCHCL(&result, cnta, &dbc);
        if(result != EM_OK) {
            throw IOException("Failed to close Teradata request");
        }
        state = TeradataRequestState::CLOSED;
    }
private:
    //TeradataRequestState state = TeradataRequestState::READY;
    //DBCAREA dbc = {};
    //char cnta[4] = {};
};
*/
