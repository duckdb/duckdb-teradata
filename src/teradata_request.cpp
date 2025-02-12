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

	// Setup buffer (1024 bytes) for fetching data
	buffer.resize(1024);
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

	if(result == EM_BUFOVERFLOW) {
		// Resize the buffer, and try again
		buffer.resize(dbc.fet_ret_data_len);
		dbc.fet_data_ptr = buffer.data();
		dbc.fet_max_data_len = buffer.size();
		DBCHCL(&result, cnta, &dbc);
	}

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
static TeradataType GetTeradataTypeFromParcel(const PclInt16 type) {
	// Nullable is 1 + the standard type
	// Param in is 500 + the standard type
	// Param inout is 501 + the standard type
	// Param out is 502 + the standard type
	switch (type) {

	case 400:
		return TeradataType(TeradataTypeId::BLOB, TeradataTypeVariant::STANDARD, 0, 0);
	case 448:
		return TeradataType(TeradataTypeId::VARCHAR, TeradataTypeVariant::STANDARD, 0, 0);
	case 449:
		return TeradataType(TeradataTypeId::VARCHAR, TeradataTypeVariant::NULLABLE, 0, 0);
	case 948:
		return TeradataType(TeradataTypeId::VARCHAR, TeradataTypeVariant::PARAM_IN, 0, 0);
	case 949:
		return TeradataType(TeradataTypeId::VARCHAR, TeradataTypeVariant::PARAM_INOUT, 0, 0);
	case 950:
		return TeradataType(TeradataTypeId::VARCHAR, TeradataTypeVariant::PARAM_OUT, 0, 0);

	case 452:
		return TeradataType(TeradataTypeId::CHAR, TeradataTypeVariant::STANDARD, 0, 0);
	case 453:
		return TeradataType(TeradataTypeId::CHAR, TeradataTypeVariant::NULLABLE, 0, 0);
	case 952:
		return TeradataType(TeradataTypeId::CHAR, TeradataTypeVariant::PARAM_IN, 0, 0);
	case 953:
		return TeradataType(TeradataTypeId::CHAR, TeradataTypeVariant::PARAM_INOUT, 0, 0);
	case 954:
		return TeradataType(TeradataTypeId::CHAR, TeradataTypeVariant::PARAM_OUT, 0, 0);

	case 472:
		return TeradataType(TeradataTypeId::LONGVARGRAPHIC, TeradataTypeVariant::STANDARD, 0, 0);
	case 473:
		return TeradataType(TeradataTypeId::LONGVARGRAPHIC, TeradataTypeVariant::NULLABLE, 0, 0);
	case 972:
		return TeradataType(TeradataTypeId::LONGVARGRAPHIC, TeradataTypeVariant::PARAM_IN, 0, 0);
	case 973:
		return TeradataType(TeradataTypeId::LONGVARGRAPHIC, TeradataTypeVariant::PARAM_INOUT, 0, 0);
	case 974:
		return TeradataType(TeradataTypeId::LONGVARGRAPHIC, TeradataTypeVariant::PARAM_OUT, 0, 0);

	case 480:
		return TeradataType(TeradataTypeId::FLOAT, TeradataTypeVariant::STANDARD, 0, 0);
	case 481:
		return TeradataType(TeradataTypeId::FLOAT, TeradataTypeVariant::NULLABLE, 0, 0);
	case 980:
		return TeradataType(TeradataTypeId::FLOAT, TeradataTypeVariant::PARAM_IN, 0, 0);
	case 981:
		return TeradataType(TeradataTypeId::FLOAT, TeradataTypeVariant::PARAM_INOUT, 0, 0);
	case 982:
		return TeradataType(TeradataTypeId::FLOAT, TeradataTypeVariant::PARAM_OUT, 0, 0);

	case 496:
		return TeradataType(TeradataTypeId::INTEGER, TeradataTypeVariant::STANDARD, 0, 0);
	case 497:
		return TeradataType(TeradataTypeId::INTEGER, TeradataTypeVariant::NULLABLE, 0, 0);
	case 996:
		return TeradataType(TeradataTypeId::INTEGER, TeradataTypeVariant::PARAM_IN, 0, 0);
	case 997:
		return TeradataType(TeradataTypeId::INTEGER, TeradataTypeVariant::PARAM_INOUT, 0, 0);
	case 998:
		return TeradataType(TeradataTypeId::INTEGER, TeradataTypeVariant::PARAM_OUT, 0, 0);

	case 500:
		return TeradataType(TeradataTypeId::SMALLINT, TeradataTypeVariant::STANDARD, 0, 0);
	case 501:
		return TeradataType(TeradataTypeId::SMALLINT, TeradataTypeVariant::NULLABLE, 0, 0);
	case 1000:
		return TeradataType(TeradataTypeId::SMALLINT, TeradataTypeVariant::PARAM_IN, 0, 0);
	case 1001:
		return TeradataType(TeradataTypeId::SMALLINT, TeradataTypeVariant::PARAM_INOUT, 0, 0);
	case 1002:
		return TeradataType(TeradataTypeId::SMALLINT, TeradataTypeVariant::PARAM_OUT, 0, 0);

	case 600:
		return TeradataType(TeradataTypeId::BIGINT, TeradataTypeVariant::STANDARD, 0, 0);
	case 601:
		return TeradataType(TeradataTypeId::BIGINT, TeradataTypeVariant::NULLABLE, 0, 0);
	case 1100:
		return TeradataType(TeradataTypeId::BIGINT, TeradataTypeVariant::PARAM_IN, 0, 0);
	case 1101:
		return TeradataType(TeradataTypeId::BIGINT, TeradataTypeVariant::PARAM_INOUT, 0, 0);
	case 1102:
		return TeradataType(TeradataTypeId::BIGINT, TeradataTypeVariant::PARAM_OUT, 0, 0);

	case 688:
		return TeradataType(TeradataTypeId::VARBYTE, TeradataTypeVariant::STANDARD, 0, 0);
	case 689:
		return TeradataType(TeradataTypeId::VARBYTE, TeradataTypeVariant::NULLABLE, 0, 0);
	case 1188:
		return TeradataType(TeradataTypeId::VARBYTE, TeradataTypeVariant::PARAM_IN, 0, 0);
	case 1189:
		return TeradataType(TeradataTypeId::VARBYTE, TeradataTypeVariant::PARAM_INOUT, 0, 0);
	case 1190:
		return TeradataType(TeradataTypeId::VARBYTE, TeradataTypeVariant::PARAM_OUT, 0, 0);

	case 692:
		return TeradataType(TeradataTypeId::BYTE, TeradataTypeVariant::STANDARD, 0, 0);
	case 693:
		return TeradataType(TeradataTypeId::BYTE, TeradataTypeVariant::NULLABLE, 0, 0);
	case 1192:
		return TeradataType(TeradataTypeId::BYTE, TeradataTypeVariant::PARAM_IN, 0, 0);
	case 1193:
		return TeradataType(TeradataTypeId::BYTE, TeradataTypeVariant::PARAM_INOUT, 0, 0);
	case 1194:
		return TeradataType(TeradataTypeId::BYTE, TeradataTypeVariant::PARAM_OUT, 0, 0);

	case 697:
		return TeradataType(TeradataTypeId::LONGVARBYTE, TeradataTypeVariant::STANDARD, 0, 0);
	case 698:
		return TeradataType(TeradataTypeId::LONGVARBYTE, TeradataTypeVariant::NULLABLE, 0, 0);
	case 1197:
		return TeradataType(TeradataTypeId::LONGVARBYTE, TeradataTypeVariant::PARAM_IN, 0, 0);
	case 1198:
		return TeradataType(TeradataTypeId::LONGVARBYTE, TeradataTypeVariant::PARAM_INOUT, 0, 0);
	case 1199:
		return TeradataType(TeradataTypeId::LONGVARBYTE, TeradataTypeVariant::PARAM_OUT, 0, 0);

	case 748:
		return TeradataType(TeradataTypeId::DATE_A, TeradataTypeVariant::STANDARD, 0, 0);
	case 749:
		return TeradataType(TeradataTypeId::DATE_A, TeradataTypeVariant::NULLABLE, 0, 0);
	case 1248:
		return TeradataType(TeradataTypeId::DATE_A, TeradataTypeVariant::PARAM_IN, 0, 0);
	case 1249:
		return TeradataType(TeradataTypeId::DATE_A, TeradataTypeVariant::PARAM_INOUT, 0, 0);
	case 1250:
		return TeradataType(TeradataTypeId::DATE_A, TeradataTypeVariant::PARAM_OUT, 0, 0);

	case 752:
		return TeradataType(TeradataTypeId::DATE_T, TeradataTypeVariant::STANDARD, 0, 0);
	case 753:
		return TeradataType(TeradataTypeId::DATE_T, TeradataTypeVariant::NULLABLE, 0, 0);
	case 1252:
		return TeradataType(TeradataTypeId::DATE_T, TeradataTypeVariant::PARAM_IN, 0, 0);
	case 1253:
		return TeradataType(TeradataTypeId::DATE_T, TeradataTypeVariant::PARAM_INOUT, 0, 0);
	case 1254:
		return TeradataType(TeradataTypeId::DATE_T, TeradataTypeVariant::PARAM_OUT, 0, 0);

	case 756:
		return TeradataType(TeradataTypeId::BYTEINT, TeradataTypeVariant::STANDARD, 0, 0);
	case 757:
		return TeradataType(TeradataTypeId::BYTEINT, TeradataTypeVariant::NULLABLE, 0, 0);
	case 1256:
		return TeradataType(TeradataTypeId::BYTEINT, TeradataTypeVariant::PARAM_IN, 0, 0);
	case 1257:
		return TeradataType(TeradataTypeId::BYTEINT, TeradataTypeVariant::PARAM_INOUT, 0, 0);
	case 1258:
		return TeradataType(TeradataTypeId::BYTEINT, TeradataTypeVariant::PARAM_OUT, 0, 0);
	default:
		// TODO: More types
		throw NotImplementedException("Unknown data type: %d", type);
	}
}

// TODO: not sure if this is always correct, read up more on formats
static bool TryParseTypeModsFromFormat(const char* ptr, PclInt16 len, int64_t &precision, int64_t &scale) {
	const auto end = ptr + len;
	if(ptr != end && *ptr == 'X') {
		ptr++;
		if(ptr != end && *ptr == '(') {
			ptr++;

			// parse the length/precision
			char* endptr;
			precision = strtol(ptr, &endptr, 10);
			if(endptr == ptr) {
				// no number found
				return false;
			}
			ptr = endptr;

			// skip the comma
			if(ptr != end && *ptr == ',') {
				ptr++;

				// parse the scale
				scale = strtol(ptr, &endptr, 10);
				if(endptr == ptr) {
					// no number found
					return false;
				}
			}
			if(ptr != end && *ptr == ')') {
				ptr++;
				return true;
			}

			return false;
		}
	}
	return false;
}

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

void TeradataPrepareRequest::GetColumns(vector<string> &names, vector<TeradataType> &types) {
	if (status != TeradataRequestStatus::OPEN) {
		throw IOException("Cannot get columns from a closed request");
	}

	// Fetch once to check if the request was successful

	dbc.func = DBFFET;
	dbc.i_req_id = dbc.o_req_id;
	dbc.fet_data_ptr = buffer.data();
	dbc.fet_max_data_len = buffer.size();

	// Now call the fetch command
	FetchAndExpectParcel(PclSUCCESS);

	// Now get the response
	FetchAndExpectParcel(PclPREPINFO);

	// Parse the prepinfo
	auto beg_ptr = buffer.data();
	// auto end_ptr = buf + dbc.fet_ret_data_len;
	CliPrepInfoType info = {};
	memcpy(&info, beg_ptr, sizeof(CliPrepInfoType));
	beg_ptr += sizeof(CliPrepInfoType);

	// Loop for the columns (ignore summaries for now)
	for (PclInt16 col_idx = 0; col_idx < info.ColumnCount; col_idx++) {

		CliPrepColInfoType col_info = {};
		memcpy(&col_info, beg_ptr, sizeof(CliPrepColInfoType));
		beg_ptr += sizeof(CliPrepColInfoType);

		auto td_type = GetTeradataTypeFromParcel(col_info.DataType);

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

		// Try to get the type mods
		int64_t precision = 0;
		int64_t scale = 0;

		if(TryParseTypeModsFromFormat(beg_ptr, col_format_len, precision, scale)) {
			td_type.SetPrecision(precision);
			td_type.SetScale(scale);
		}

		// Skip that many bytes ahead
		// TODO: Dont skip, we need this to be able to read the data (e.g. char width)
		beg_ptr += col_format_len;

		// Now read the column title length
		PclInt16 col_title_len = 0;
		memcpy(&col_title_len, beg_ptr, sizeof(PclInt16));
		beg_ptr += sizeof(PclInt16);

		// Skip that many bytes ahead
		beg_ptr += col_title_len;

		// Push the type
		types.push_back(td_type);
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
	dbc.func = DBFFET;
	dbc.i_req_id = dbc.o_req_id;
	dbc.fet_data_ptr = buffer.data();
	dbc.fet_max_data_len = buffer.size();

	FetchAndExpectParcel(PclSUCCESS);

	status = TeradataRequestStatus::OPEN;
}

void TeradataSqlRequest::FetchNextChunk(DataChunk &chunk, const vector<TeradataType> &td_types) {
	if (status != TeradataRequestStatus::OPEN) {
		throw InternalException("Cannot fetch from a closed request");
	}

	dbc.func = DBFFET;
	dbc.fet_data_ptr = buffer.data();
	dbc.fet_max_data_len = buffer.size();

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
			//
			// TODO: We need to parse this to get the field lengths and shit
			// p.413 in the manual
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

			// p. 19, DatabaseResponseMode
			auto record_ptr = buffer.data() + validity_bytes;

			for (idx_t col_idx = 0; col_idx < chunk.ColumnCount(); col_idx++) {
				const auto is_null = (buffer[col_idx / 8] & (1 << (col_idx % 8))) != 0;

				auto &col_vec = chunk.data[col_idx];
				auto &td_type = td_types[col_idx];

				// Switch on type
				switch (col_vec.GetType().id()) {
				case LogicalTypeId::TINYINT: {
					if(is_null) {
						FlatVector::SetNull(col_vec, row_idx, true);
					} else {
						const auto data_ptr = FlatVector::GetData<int8_t>(col_vec);
						memcpy(data_ptr + row_idx, record_ptr, sizeof(int8_t));
					}
					record_ptr += sizeof(int8_t);
				}
				case LogicalTypeId::UTINYINT: {
					if(is_null) {
						FlatVector::SetNull(col_vec, row_idx, true);
					} else {
						const auto data_ptr = FlatVector::GetData<uint8_t>(col_vec);
						memcpy(data_ptr + row_idx, record_ptr, sizeof(uint8_t));
					}
					record_ptr += sizeof(uint8_t);
				} break;
				case LogicalTypeId::SMALLINT: {
					if(is_null) {
						FlatVector::SetNull(col_vec, row_idx, true);
					} else {
						const auto data_ptr = FlatVector::GetData<int16_t>(col_vec);
						memcpy(data_ptr + row_idx, record_ptr, sizeof(int16_t));
					}
					record_ptr += sizeof(int16_t);
				} break;
				case LogicalTypeId::INTEGER: {
					if(is_null) {
						FlatVector::SetNull(col_vec, row_idx, true);
					} else {
						const auto data_ptr = FlatVector::GetData<int32_t>(col_vec);
						memcpy(data_ptr + row_idx, record_ptr, sizeof(int32_t));
					}
					record_ptr += sizeof(int32_t);
				} break;
				case LogicalTypeId::BIGINT: {
					if(is_null) {
						FlatVector::SetNull(col_vec, row_idx, true);
					} else {
						const auto data_ptr_bigint = FlatVector::GetData<int64_t>(col_vec);
						memcpy(data_ptr_bigint + row_idx, record_ptr, sizeof(int64_t));
					}
					record_ptr += sizeof(int64_t);
				} break;
				case LogicalTypeId::VARCHAR: {
					// TODO: we cant just memcpy, theres shift-out characters to deal with
					const auto data_ptr = FlatVector::GetData<string_t>(col_vec);
					if(td_type.GetId() == TeradataTypeId::CHAR) {
						const auto len = td_type.GetLength();
						if(is_null) {
							FlatVector::SetNull(col_vec, row_idx, true);
						} else {
							// Now do some shit
							*data_ptr = StringVector::AddString(col_vec, record_ptr, len);
						}
						record_ptr += len;
					} else {
						uint16_t len = 0;
						memcpy(&len, record_ptr, sizeof(uint16_t));
						record_ptr += sizeof(uint16_t);

						if(is_null) {
							FlatVector::SetNull(col_vec, row_idx, true);
						} else {
							*data_ptr = StringVector::AddString(col_vec, record_ptr, len);
							record_ptr += len;
						}
					}

					col_vec.Verify(1);
				} break;
				default:
					throw NotImplementedException("Unsupported Teradata Type: '%s'", col_vec.GetType().ToString());
				}
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