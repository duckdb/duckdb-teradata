#pragma once

#include "duckdb.hpp"

// Teradata Includes
#include <coptypes.h>
#include <coperr.h>
#include <dbcarea.h>
#include <parcel.h>

#define CONNECTED     0
#define NOT_CONNECTED 1
#define OK            0
#define STOP          1
#define FAILED       -1

#ifdef WIN32
__declspec(dllimport) char   COPCLIVersion[];
__declspec(dllimport) char   COPMTDPVersion[];
__declspec(dllimport) char   COPMOSIosVersion[];
__declspec(dllimport) char   COPMOSIDEPVersion[];
__declspec(dllimport) char   OSERRVersion[];
#else
extern char   COPCLIVersion[];
extern char   COPMTDPVersion[];
extern char   COPMOSIosVersion[];
extern char   COPMOSIDEPVersion[];
extern char   OSERRVersion[];
#endif

namespace duckdb {

struct TeradataTypeData {
	string type_name;
	string column_type;
	int64_t precision;
	int64_t scale;
};

enum class TeradataTypeAnnotation : uint8_t {
	STANDARD,
	CAST_TO_VARCHAR,
	NUMERIC_AS_DOUBLE,
	CTID,
	JSONB,
	FIXED_LENGTH_CHAR
};

struct TeradataType {
	idx_t oid = 0;
	TeradataTypeAnnotation info = TeradataTypeAnnotation::STANDARD;
	vector<TeradataType> children;
};

} // namespace duckdb
