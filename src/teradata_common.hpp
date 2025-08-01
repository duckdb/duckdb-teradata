#pragma once

#include "duckdb.hpp"

// Otherwise doesnt work on linux
#define NO_CLIV2_ERROR_T

// Teradata Includes
#include <coptypes.h>
#include <coperr.h>
#include <dbcarea.h>
#include <parcel.h>

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
	static void Load();
	static bool IsLoaded();
};

} // namespace duckdb
