#include "teradata_common.hpp"

#include "duckdb/common/dl.hpp"

namespace duckdb {

typedef Int32 (*td_callback)(Int32 *, char *, struct DBCAREA *);
static td_callback DBCHINI_IMPL = nullptr;
static td_callback DBCHCL_IMPL = nullptr;

static mutex load_mutex;
static bool IsLoadedInternal() {
	return DBCHINI_IMPL != nullptr && DBCHCL_IMPL != nullptr;
}

/*
	https://docs.teradata.com/r/Enterprise_IntelliFlex_Lake_VMware/Teradata-Call-Level-Interface-Version-2-Reference-for-Workstation-Attached-Systems-20.00/CLI-Files-and-Setup/Finding-CLI-Files-on-the-System

	On all 64-bit UNIX platforms, the default installation directory for:
		/opt/teradata/client/<release_number>/lib64

	On Apple macOS platforms, the default installation directory is:
		/Library/Application Support/teradata/client/<release_number>/lib

	On 64-bit Windows, the default installation directory for 64-bit files is:
		%ProgramFiles%\Teradata\ Client\<release_number>\bin
 */
static const char* CLIV2_SEARCH_PATHS[] = {
#if defined(__APPLE__)
	"libcliv2.dylib",
	"/Library/Application Support/teradata/client/20.00/lib/libcliv2.dylib",
	"/Library/Application Support/teradata/client/17.20/lib/libcliv2.dylib",
	"/Library/Application Support/teradata/client/17.10/lib/libcliv2.dylib",
	"/Library/Application Support/teradata/client/17.00/lib/libcliv2.dylib",
	"/Library/Application Support/teradata/client/16.20/lib/libcliv2.dylib",
#endif
#if defined(_WIN32)
	"libcliv2.dll",
	"C:\\Program Files\\Teradata\\Client\\20.00\\bin\\libcliv2.dll",
	"C:\\Program Files\\Teradata\\Client\\17.20\\bin\\libcliv2.dll",
	"C:\\Program Files\\Teradata\\Client\\17.10\\bin\\libcliv2.dll",
	"C:\\Program Files\\Teradata\\Client\\17.00\\bin\\libcliv2.dll",
	"C:\\Program Files\\Teradata\\Client\\16.20\\bin\\libcliv2.dll",
#endif
#if defined(__linux__)
	"libcliv2.so",
	"opt/teradata/client/20.00/lib64",
	"opt/teradata/client/17.20/lib64",
	"opt/teradata/client/17.10/lib64",
	"opt/teradata/client/17.00/lib64",
	"opt/teradata/client/16.20/lib64",
#endif
};

static bool TryPath(const char* str, void* &handle, vector<string> &errors) {
	const auto new_handle = dlopen(str, RTLD_LAZY | RTLD_GLOBAL);
	if (new_handle) {
		handle = new_handle;
		return true;
	}
	errors.push_back( GetDLError());
	return false;
}


static void* TryLoadCLIV2(const string &override_path) {
	vector<string> errors;
	void* handle = nullptr;

	// Try the user-specified override path first
	if (TryPath(override_path.c_str(), handle, errors)) {
		return handle;
	}

	// Otherwise, check some default paths
	for (const auto path : CLIV2_SEARCH_PATHS) {
		if (TryPath(path, handle, errors)) {
			return handle;
		}
	}

	throw IOException("Failed to load Teradata CLIV2: %s", StringUtil::Join(errors, "\n"));
}

bool TeradataCLIV2::IsLoaded() {
	lock_guard<mutex> lock(load_mutex);
	return IsLoadedInternal();
}

void TeradataCLIV2::Load(ClientContext &context) {
	lock_guard<mutex> lock(load_mutex);
	if (IsLoadedInternal()) {
		return;
	}

	Value cliv2_lib_path_value;
	string cliv2_lib_path;
	if (context.TryGetCurrentSetting("teradata_cliv2_library_path", cliv2_lib_path_value)) {
		cliv2_lib_path = cliv2_lib_path_value.GetValue<string>();
	}

	const auto cliv2_handle = TryLoadCLIV2(cliv2_lib_path);

	// Load the DBCHINI and DBCHCL functions from the Teradata library
	DBCHINI_IMPL = reinterpret_cast<td_callback>(dlsym(cliv2_handle, "DBCHINI"));
	if (!DBCHINI_IMPL) {
		throw IOException("Failed to load DBCHINI from Teradata library: %s", dlerror());
	}

	DBCHCL_IMPL = reinterpret_cast<td_callback>(dlsym(cliv2_handle, "DBCHCL"));
	if (!DBCHCL_IMPL) {
		throw IOException("Failed to load DBCHCL from Teradata library: %s", dlerror());
	}
}

} // namespace duckdb


Int32 DBCHINI(Int32 *a, char *b, struct DBCAREA *c) {
	if (duckdb::DBCHINI_IMPL) {
		return duckdb::DBCHINI_IMPL(a, b, c); // Dummy call for the sake of example
	}
	throw duckdb::IOException("Teradata CLIV2 not loaded, cannot call DBCHINI!");
}

Int32 DBCHCL(Int32 *a, char *b, struct DBCAREA *c) {
	if (duckdb::DBCHCL_IMPL) {
		return duckdb::DBCHCL_IMPL(a, b, c); // Dummy call for the sake of example
	}
	throw duckdb::IOException("Teradata CLIV2 not loaded, cannot call DBCHCL!");
}
