#pragma once

#include "teradata_common.hpp"

#include <functional>
#include <mutex>

namespace duckdb {

//! A parcel the fake server sends back
struct FakeParcel {
	PclFlavor flavor;
	string data;
};

//! A column of a PREPINFO or DATAINFO parcel
struct FakeColumn {
	int16_t parcel_type;
	uint16_t length;
	string name;
};

//! Builders for the parcels the extension parses
struct Parcels {
	static FakeParcel PrepInfo(const vector<FakeColumn> &columns);
	static FakeParcel DataInfo(const vector<FakeColumn> &columns);
	//! A record: the null indicator bytes followed by the already-encoded fields
	static FakeParcel Record(const vector<bool> &nulls, const string &fields);
	static FakeParcel Failure(uint16_t code, const string &message);

	//! Field encodings, as in a record
	static string Int32(int32_t value);
	static string Varchar(const string &value);
	static string Char(const string &value);

	//! SUCCESS, the given parcels, ENDSTATEMENT, ENDREQUEST
	static vector<FakeParcel> Statement(vector<FakeParcel> body = {});
};

//! What the fake server does with a request
struct FakeResponse {
	vector<FakeParcel> parcels;
	//! Never send a response: fetches return EM_NODATA (non-blocking) forever
	bool never_answers = false;
};

//! A request the fake server received
struct FakeRequest {
	string sql;
	//! 'P' prepare, 'E' execute
	char mode;
	bool non_blocking;
	//! The parameter records of a parameterized request (one per row)
	vector<string> records;
};

//! An in-process stand-in for the Teradata CLIv2 library: scripted responses, and a log of every call
class FakeCLIv2 {
public:
	static FakeCLIv2 &Get();

	//! Route the extension's CLIv2 calls here, and forget all responses and logged calls
	void Install();

	//! Answer requests whose SQL contains `sql_fragment` (in mode 'P' or 'E') with `response`.
	//! Later registrations take precedence. Unmatched requests get an empty successful statement.
	void Respond(const string &sql_fragment, char mode, FakeResponse response);

	vector<string> Calls();
	vector<FakeRequest> Requests();

private:
	struct Rule {
		string sql_fragment;
		char mode;
		FakeResponse response;
	};

	static Int32 DBCHINI(Int32 *result, char *, struct DBCAREA *dbc);
	static Int32 DBCHCL(Int32 *result, char *, struct DBCAREA *dbc);
	Int32 Call(struct DBCAREA &dbc);
	Int32 InitiateRequest(struct DBCAREA &dbc);
	Int32 Fetch(struct DBCAREA &dbc);

	std::mutex lock;
	vector<Rule> rules;
	vector<string> calls;
	vector<FakeRequest> requests;

	// The request being answered
	FakeResponse current;
	idx_t parcel_idx = 0;
	Int32 next_id = 0;
};

} // namespace duckdb
