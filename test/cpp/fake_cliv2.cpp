#include "fake_cliv2.hpp"

#include <algorithm>
#include <cstring>
#include <thread>

namespace duckdb {

namespace {

template <class T>
string Bytes(const T &value) {
	return string(reinterpret_cast<const char *>(&value), sizeof(T));
}

string LengthPrefixed(const string &value) {
	return Bytes(static_cast<int16_t>(value.size())) + value;
}

} // namespace

//----------------------------------------------------------------------------------------------------------------------
// Parcels
//----------------------------------------------------------------------------------------------------------------------
FakeParcel Parcels::PrepInfo(const vector<FakeColumn> &columns) {
	CliPrepInfoType info;
	memset(&info, 0, sizeof(info));
	info.ColumnCount = static_cast<PclInt16>(columns.size());
	string data = Bytes(info);
	for (auto &column : columns) {
		CliPrepColInfoType col;
		col.DataType = column.parcel_type;
		col.DataLen = column.length;
		// name, format, title
		data += Bytes(col) + LengthPrefixed(column.name) + LengthPrefixed("") + LengthPrefixed("");
	}
	return {PclPREPINFO, data};
}

FakeParcel Parcels::DataInfo(const vector<FakeColumn> &columns) {
	string data = Bytes(static_cast<uint16_t>(columns.size()));
	for (auto &column : columns) {
		data += Bytes(static_cast<uint16_t>(column.parcel_type)) + Bytes(column.length);
	}
	return {PclDATAINFO, data};
}

FakeParcel Parcels::Record(const vector<bool> &nulls, const string &fields) {
	string indicators((nulls.size() + 7) / 8, '\0');
	for (idx_t i = 0; i < nulls.size(); i++) {
		if (nulls[i]) {
			indicators[i / 8] = static_cast<char>(indicators[i / 8] | (1 << (7 - i % 8)));
		}
	}
	return {PclRECORD, indicators + fields};
}

FakeParcel Parcels::Failure(uint16_t code, const string &message) {
	const uint16_t stmt_no = 1;
	const uint16_t info = 0;
	return {PclFAILURE,
	        Bytes(stmt_no) + Bytes(info) + Bytes(code) + Bytes(static_cast<uint16_t>(message.size())) + message};
}

string Parcels::Int32(int32_t value) {
	return Bytes(value);
}

string Parcels::Varchar(const string &value) {
	return Bytes(static_cast<uint16_t>(value.size())) + value;
}

string Parcels::Char(const string &value) {
	return value;
}

vector<FakeParcel> Parcels::Statement(vector<FakeParcel> body) {
	vector<FakeParcel> result {{PclSUCCESS, ""}};
	result.insert(result.end(), body.begin(), body.end());
	result.push_back({PclENDSTATEMENT, ""});
	result.push_back({PclENDREQUEST, ""});
	return result;
}

//----------------------------------------------------------------------------------------------------------------------
// FakeCLIv2
//----------------------------------------------------------------------------------------------------------------------
FakeCLIv2 &FakeCLIv2::Get() {
	static FakeCLIv2 instance;
	return instance;
}

void FakeCLIv2::Install() {
	std::lock_guard<std::mutex> guard(lock);
	rules.clear();
	calls.clear();
	requests.clear();
	active.clear();
	TeradataCLIV2::SetImplementation(&FakeCLIv2::DBCHINI, &FakeCLIv2::DBCHCL);
}

void FakeCLIv2::Respond(const string &sql_fragment, char mode, FakeResponse response) {
	std::lock_guard<std::mutex> guard(lock);
	rules.push_back({sql_fragment, mode, std::move(response)});
}

vector<string> FakeCLIv2::Calls() {
	std::lock_guard<std::mutex> guard(lock);
	return calls;
}

vector<FakeRequest> FakeCLIv2::Requests() {
	std::lock_guard<std::mutex> guard(lock);
	return requests;
}

bool FakeCLIv2::WaitForRequest(const string &sql_fragment, std::chrono::milliseconds timeout) {
	const auto deadline = std::chrono::steady_clock::now() + timeout;
	while (std::chrono::steady_clock::now() < deadline) {
		const auto received = Requests();
		if (std::any_of(received.begin(), received.end(),
		                [&](const FakeRequest &r) { return r.sql.find(sql_fragment) != string::npos; })) {
			return true;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
	return false;
}

Int32 FakeCLIv2::StartRequest(FakeResponse response) {
	const auto id = ++next_id;
	active[id] = ActiveRequest {std::move(response), 0, std::chrono::steady_clock::now()};
	return id;
}

Int32 FakeCLIv2::DBCHINI(Int32 *result, char *, struct DBCAREA *) {
	*result = EM_OK;
	return EM_OK;
}

Int32 FakeCLIv2::DBCHCL(Int32 *result, char *, struct DBCAREA *dbc) {
	auto &fake = Get();
	std::lock_guard<std::mutex> guard(fake.lock);
	*result = fake.Call(*dbc);
	return *result;
}

Int32 FakeCLIv2::Call(struct DBCAREA &dbc) {
	switch (dbc.func) {
	case DBFCON:
		calls.emplace_back("CON");
		dbc.o_sess_id = 1;
		// The logon response
		dbc.o_req_id = StartRequest(FakeResponse {{{PclSUCCESS, ""}}});
		return EM_OK;
	case DBFIRQ:
		return InitiateRequest(dbc);
	case DBFFET:
		return Fetch(dbc);
	case DBFABT:
		calls.emplace_back("ABT");
		return EM_OK;
	case DBFERQ:
		calls.emplace_back("ERQ");
		active.erase(dbc.i_req_id);
		return EM_OK;
	case DBFDSC:
		calls.emplace_back("DSC");
		return EM_OK;
	default:
		return EM_OK;
	}
}

Int32 FakeCLIv2::InitiateRequest(struct DBCAREA &dbc) {
	FakeRequest request {string(dbc.req_ptr, dbc.req_len), dbc.req_proc_opt, dbc.wait_for_resp == 'N', {}};
	if (dbc.using_data_count > 0) {
		auto records = reinterpret_cast<char **>(dbc.using_data_ptr_array);
		for (UInt32 i = 0; i < dbc.using_data_count; i++) {
			request.records.emplace_back(records[i], dbc.using_data_len_array[i]);
		}
	}
	calls.push_back("IRQ " + request.sql);

	// Later rules take precedence
	const auto rule = std::find_if(rules.rbegin(), rules.rend(), [&](const Rule &r) {
		return r.mode == request.mode && request.sql.find(r.sql_fragment) != string::npos;
	});
	FakeResponse response;
	if (rule != rules.rend()) {
		response = rule->response;
	} else {
		// One statement per parameter row, as a parameterized request answers
		const auto statements = std::max<UInt32>(dbc.using_data_count, 1);
		for (UInt32 i = 0; i < statements; i++) {
			response.parcels.push_back({PclSUCCESS, ""});
			response.parcels.push_back({PclENDSTATEMENT, ""});
		}
		response.parcels.push_back({PclENDREQUEST, ""});
	}
	requests.push_back(std::move(request));
	dbc.o_req_id = StartRequest(std::move(response));
	return EM_OK;
}

Int32 FakeCLIv2::Fetch(struct DBCAREA &dbc) {
	const auto request = active.find(dbc.i_req_id);
	if (request == active.end()) {
		return Error(dbc, "fake CLIv2: fetch for an unknown request");
	}
	auto &state = request->second;
	if (std::chrono::steady_clock::now() - state.started < state.response.delay) {
		return EM_NODATA;
	}
	if (state.parcel_idx >= state.response.parcels.size()) {
		return Error(dbc, "fake CLIv2: no more parcels");
	}
	const auto &parcel = state.response.parcels[state.parcel_idx];
	dbc.fet_ret_data_len = static_cast<Int32>(parcel.data.size());
	if (parcel.data.size() > static_cast<idx_t>(dbc.fet_max_data_len)) {
		// The caller grows its buffer and fetches the same parcel again
		return BUFOVFLOW;
	}
	memcpy(dbc.fet_data_ptr, parcel.data.data(), parcel.data.size());
	dbc.fet_parcel_flavor = parcel.flavor;
	state.parcel_idx++;
	return EM_OK;
}

Int32 FakeCLIv2::Error(struct DBCAREA &dbc, const string &message) {
	const auto length = std::min<idx_t>(message.size(), sizeof(dbc.msg_text));
	memcpy(dbc.msg_text, message.c_str(), length);
	dbc.msg_len = static_cast<short>(length);
	return NOACTIVEREQUEST;
}

} // namespace duckdb
