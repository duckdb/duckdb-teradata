#pragma once

#include "duckdb.hpp"

// Otherwise doesnt work on linux
#define NO_CLIV2_ERROR_T

// Teradata Includes
#include <coptypes.h>
#include <coperr.h>
#include <dbcarea.h>
#include <parcel.h>

// dbcarea.h defines these as macros, which clash with duckdb::TriggerEventType enumerators
#undef INSERT_EVENT
#undef DELETE_EVENT

#define CONNECTED     0
#define NOT_CONNECTED 1
#define OK            0
#define STOP          1
#define FAILED        -1

#ifdef WIN32
__declspec(dllimport) char COPCLIVersion[];
__declspec(dllimport) char COPMTDPVersion[];
__declspec(dllimport) char COPMOSIosVersion[];
__declspec(dllimport) char COPMOSIDEPVersion[];
__declspec(dllimport) char OSERRVersion[];
#else
extern char COPCLIVersion[];
extern char COPMTDPVersion[];
extern char COPMOSIosVersion[];
extern char COPMOSIDEPVersion[];
extern char OSERRVersion[];
#endif

namespace duckdb {

struct TeradataCLIV2 {
	typedef Int32 (*callback_t)(Int32 *, char *, struct DBCAREA *);

	static void Load(ClientContext &context);
	static bool IsLoaded();

	//! Route CLIv2 calls to the given functions instead of the Teradata library (for tests)
	static void SetImplementation(callback_t dbchini, callback_t dbchcl);
};

} // namespace duckdb
