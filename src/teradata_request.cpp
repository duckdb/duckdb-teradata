#include "teradata_request.hpp"
#include "teradata_type.hpp"

namespace duckdb {

//----------------------------------------------------------------------------------------------------------------------
// Teradata Request Base
//----------------------------------------------------------------------------------------------------------------------
TeradataRequest::TeradataRequest(Int32 session_id_p) : session_id(session_id_p), status(TeradataRequestStatus::READY) {

	// Set total length	of DBCAREA
	dbc.total_len = sizeof(DBCAREA);

	Int32 result;
	DBCHINI(&result, cnta, &dbc);
	if (result != 0) {
		throw IOException("Failed to initialize DBCAREA: %s", string(dbc.msg_text, dbc.msg_len));
	}

	// Setup session id
	dbc.i_sess_id = session_id;
}

void TeradataRequest::Close() {
	if (status == TeradataRequestStatus::READY || status == TeradataRequestStatus::CLOSED) {
		// We can only close an empty request
		return;
	}
	Int32 result = EM_OK;
	dbc.func = DBFERQ;
	DBCHCL(&result, cnta, &dbc);
	if (result != EM_OK) {
		throw IOException("Failed to close Teradata request");
	}
	status = TeradataRequestStatus::CLOSED;
}

void TeradataRequest::FetchAndExpectParcel(PclFlavor expected) {
	Int32 result = EM_OK;
	DBCHCL(&result, cnta, &dbc);
	if (result != EM_OK) {
		throw IOException("Failed to fetch result: %s", string(dbc.msg_text, dbc.msg_len));
	}
	if (dbc.fet_parcel_flavor != expected) {
		throw IOException("Expected parcel flavor %d, got %d", expected, dbc.fet_parcel_flavor);
	}
}

//----------------------------------------------------------------------------------------------------------------------
// Teradata Prepare Request
//----------------------------------------------------------------------------------------------------------------------

TeradataPrepareRequest::TeradataPrepareRequest(Int32 session_id_p, const string &sql) : TeradataRequest(session_id_p) {
	dbc.func = DBFIRQ;      // initiate request
	dbc.change_opts = 'Y';  // change options to indicate that we want to change the options (to indicator mode)
	dbc.resp_mode = 'I';    // indicator record mode
	dbc.req_proc_opt = 'P'; // prepare mode

	vector<char> sql_buf(sql.begin(), sql.end());
	dbc.req_ptr = sql_buf.data();
	dbc.req_len = sql_buf.size();

	int32_t result = EM_OK;
	DBCHCL(&result, cnta, &dbc);
	if (result != EM_OK) {
		throw IOException("Failed to execute SQL: %s", string(dbc.msg_text, dbc.msg_len));
	}

	dbc.i_req_id = dbc.o_req_id;
	status = TeradataRequestStatus::OPEN;
}

void TeradataPrepareRequest::GetColumns(vector<string> &names, vector<LogicalType> &types) {
	if (status != TeradataRequestStatus::OPEN) {
		throw IOException("Cannot get columns from a closed request");
	}

	// Fetch once to check if the request was successful
	char buf[512] = {0};
	dbc.func = DBFFET;
	dbc.i_req_id = dbc.o_req_id;
	dbc.fet_data_ptr = buf;
	dbc.fet_max_data_len = sizeof(buf);

	// Now call the fetch command
	FetchAndExpectParcel(PclSUCCESS);

	// Now get the response
	FetchAndExpectParcel(PclPREPINFO);

	// Parse the prepinfo
	auto beg_ptr = buf;
	// auto end_ptr = buf + dbc.fet_ret_data_len;
	CliPrepInfoType info = {};
	memcpy(&info, beg_ptr, sizeof(CliPrepInfoType));
	beg_ptr += sizeof(CliPrepInfoType);

	// Loop for the columns (ignore summaries for now)
	for (PclInt16 col_idx = 0; col_idx < info.ColumnCount; col_idx++) {
		CliPrepColInfoType col_info = {};
		memcpy(&col_info, beg_ptr, sizeof(CliPrepColInfoType));
		beg_ptr += sizeof(CliPrepColInfoType);

		auto ttype = TeradataColumnType::Get(col_info.DataType);
		if (ttype.type.id() == LogicalTypeId::INVALID) {
			throw NotImplementedException("Unsupported Teradata Type: '%s'", ttype.name);
		}

		types.emplace_back(std::move(ttype.type));

		// Now also read the column_name length
		PclInt16 col_name_len = 0;
		memcpy(&col_name_len, beg_ptr, sizeof(PclInt16));
		beg_ptr += sizeof(PclInt16);

		// Copy the column name
		names.emplace_back(beg_ptr, col_name_len);

		// Skip that many bytes ahead
		beg_ptr += col_name_len;

		// Now read the column format length
		PclInt16 col_format_len = 0;
		memcpy(&col_format_len, beg_ptr, sizeof(PclInt16));
		beg_ptr += sizeof(PclInt16);

		// Skip that many bytes ahead
		beg_ptr += col_format_len;

		// Now read the column title length
		PclInt16 col_title_len = 0;
		memcpy(&col_title_len, beg_ptr, sizeof(PclInt16));
		beg_ptr += sizeof(PclInt16);

		// Skip that many bytes ahead
		beg_ptr += col_title_len;
	}

	FetchAndExpectParcel(PclENDSTATEMENT);

	FetchAndExpectParcel(PclENDREQUEST);

	status = TeradataRequestStatus::DONE;
}

//----------------------------------------------------------------------------------------------------------------------
// Teradata SQL	Request
//----------------------------------------------------------------------------------------------------------------------

TeradataSqlRequest::TeradataSqlRequest(Int32 session_id_p, const string &sql) : TeradataRequest(session_id_p) {

	dbc.func = DBFIRQ;      // initiate request
	dbc.change_opts = 'Y';  // change options to indicate that we want to change the options (to indicator mode)
	dbc.resp_mode = 'I';    // indicator record mode
	dbc.req_proc_opt = 'E'; // execute mode
	dbc.i_sess_id = session_id;

	// We can't pass const char* to the dbc.req_ptr, so we need to copy the string
	vector<char> sql_buf(sql.begin(), sql.end());

	dbc.req_ptr = sql_buf.data();
	dbc.req_len = static_cast<Int32>(sql_buf.size());

	int32_t result = EM_OK;
	DBCHCL(&result, cnta, &dbc);
	if (result != EM_OK) {
		throw IOException("Failed to execute SQL: %s", string(dbc.msg_text, dbc.msg_len));
	}

	// Fetch once to check if the request was successful
	char buf[512] = {0};
	dbc.func = DBFFET;
	dbc.i_req_id = dbc.o_req_id;
	dbc.fet_data_ptr = buf;
	dbc.fet_max_data_len = sizeof(buf);

	FetchAndExpectParcel(PclSUCCESS);

	status = TeradataRequestStatus::OPEN;
}

void TeradataSqlRequest::FetchNextChunk(DataChunk &chunk) {
	if (status != TeradataRequestStatus::OPEN) {
		throw InternalException("Cannot fetch from a closed request");
	}

	char buf[512] = {};
	dbc.func = DBFFET;
	dbc.fet_data_ptr = buf;
	dbc.fet_max_data_len = sizeof(buf);

	Int32 result = EM_OK;

	const idx_t validity_bytes = (chunk.ColumnCount() + 7) / 8;

	// This is how much capacity we have in the chunk
	idx_t row_idx = 0;

	while (row_idx < STANDARD_VECTOR_SIZE) {

		DBCHCL(&result, cnta, &dbc);
		if (result != EM_OK) {
			throw IOException("Failed to fetch result: %s", string(dbc.msg_text, dbc.msg_len));
		}

		switch (dbc.fet_parcel_flavor) {
		case PclDATAINFO: {
			// printf("Got data info");
			break;
		}
		case PclENDREQUEST: {
			// No more rows remaining
			status = TeradataRequestStatus::DONE;
			return;
		}
		case PclRECORD: {
			// printf("Got record!");
			// Within the Record parcel, each line of a table is separated from the next by hex 0D (decimal 13).
			// Data conversion rules in p.1144
			// p. 1144, 1143, 1145
			auto record_ptr = buf + validity_bytes;

			for (idx_t col_idx = 0; col_idx < chunk.ColumnCount(); col_idx++) {
				const auto is_null = (buf[col_idx / 8] & (1 << (col_idx % 8))) != 0;

				auto &col_vec = chunk.data[col_idx];
				if (is_null) {
					FlatVector::SetNull(col_vec, row_idx, true);
				} else {
					const auto data_ptr = FlatVector::GetData<int32_t>(col_vec);
					memcpy(data_ptr + row_idx, record_ptr, sizeof(int32_t));
				}

				// TODO: Size of the data type
				record_ptr += 4;
			}

			// Move to the next row
			row_idx++;
			chunk.SetCardinality(row_idx);
			break;
		}
		case PclENDSTATEMENT: {
			// TODO: Handle multiple statement (throw?)
			// printf("End statement");
			break;
		}
		default: {
			status = TeradataRequestStatus::DONE;
			throw IOException("Unknown parcel flavor: %d", dbc.fet_parcel_flavor);
		}
		}
	}
}

} // namespace duckdb