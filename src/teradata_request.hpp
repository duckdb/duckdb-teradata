#pragma once

#include "teradata_common.hpp"
#include "teradata_type.hpp"

#include "duckdb/common/types/column/column_data_collection.hpp"

namespace duckdb {

class TeradataConnection;

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
	PclFlavor FetchNextParcel();
	void MatchNextParcel(PclFlavor flavor);

protected:
	DBCAREA dbc;
	Int32 session_id;
	char cnta[4];
	TeradataRequestStatus status;
	vector<char> buffer;
};

// TODO: we should pass the connection to the request instead of the session_id
// Then the request can reconnect if needed
class TeradataPrepareRequest final : public TeradataRequest {
public:
	explicit TeradataPrepareRequest(Int32 session_id_p, const string &sql);
	void GetColumns(vector<string> &names, vector<TeradataType> &types);
};

class TeradataSqlRequest final : public TeradataRequest {
public:
	explicit TeradataSqlRequest(Int32 session_id_p, const string &sql);
	void FetchNextChunk(DataChunk &chunk);
	const vector<TeradataType> &GetTypes() const { return td_types; }
	static unique_ptr<ColumnDataCollection> Execute(const TeradataConnection &conn, const string &sql);

private:
	vector<TeradataType> td_types;
};

} // namespace duckdb