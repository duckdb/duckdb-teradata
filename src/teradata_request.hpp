#pragma once

#include "teradata_common.hpp"
#include "teradata_type.hpp"

#include "duckdb/common/types/column/column_data_collection.hpp"

namespace duckdb {

class TeradataConnection;
class TeradataColumnReader;
class TeradataColumnWriter;

class TeradataRequestContext {
public:
	explicit TeradataRequestContext(const TeradataConnection &con);
	void Init(const TeradataConnection &con);

	// Execute a statement without returning any data
	void Execute(const string &sql);

	// Execute a paramterized statment, once for each row in the chunk
	void Execute(const string &sql, DataChunk &chunk, ArenaAllocator &arena,
	             vector<unique_ptr<TeradataColumnWriter>> &writers);

	// Prepare a statement, returning the types of the result set
	void Prepare(const string &sql, vector<TeradataType> &types, vector<string> &names);

	// Issue a SQL query and return the types of the result set
	void Query(const string &sql, vector<TeradataType> &types);

	// Fetch the next data chunk after calling Query. Returns true if there is more data to fetch
	bool Fetch(DataChunk &chunk, const vector<unique_ptr<TeradataColumnReader>> &readers);

	// Fetch all data after calling Query, into a ColumnDataCollection.
	unique_ptr<ColumnDataCollection> FetchAll(const vector<TeradataType> &types);
	~TeradataRequestContext();

private:
	static constexpr auto BUFFER_DEFAULT_SIZE = 4096;

	void BeginRequest(const string &sql, char mode);
	void EndRequest();
	void Close();
	void MatchParcel(uint16_t flavor);
	uint16_t FetchParcel();

	DBCAREA dbc = {};
	char cnta[4] = {};
	vector<char> buffer;
	bool is_open = false;
};

} // namespace duckdb