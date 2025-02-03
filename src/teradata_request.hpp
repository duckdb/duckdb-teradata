#pragma once

#include "teradata_common.hpp"

namespace duckdb {

enum class TeradataRequestStatus : uint8_t { READY, OPEN, DONE, CLOSED };

class TeradataRequest {
public:
	~TeradataRequest() {
		Close();
	}
	void Close();
	TeradataRequestStatus GetStatus() const {
		return status;
	}

protected:
	explicit TeradataRequest(Int32 session_id_p);
	void FetchAndExpectParcel(PclFlavor expected);

protected:
	DBCAREA dbc;
	Int32 session_id;
	char cnta[4];
	TeradataRequestStatus status;
};

class TeradataPrepareRequest final : public TeradataRequest {
public:
	explicit TeradataPrepareRequest(Int32 session_id_p, const string &sql);
	void GetColumns(vector<string> &names, vector<LogicalType> &types);
};

class TeradataSqlRequest final : public TeradataRequest {
public:
	explicit TeradataSqlRequest(Int32 session_id_p, const string &sql);
	void FetchNextChunk(DataChunk &chunk);
};

} // namespace duckdb