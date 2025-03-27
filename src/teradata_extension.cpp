#define DUCKDB_EXTENSION_MAIN

#include "teradata_extension.hpp"
#include "teradata_scan.hpp"
#include "teradata_query.hpp"
#include "teradata_execute.hpp"
#include "teradata_storage.hpp"

#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension_util.hpp"
#include "duckdb/parser/parsed_data/create_scalar_function_info.hpp"

namespace duckdb {

void TeradataExtension::Load(DuckDB &db) {
	auto &instance = *db.instance;

	// Register Teradata functions
	TeradataScan::Register(instance);
	TeradataQueryFunction::Register(instance);
	TeradataExecuteFunction::Register(instance);

	instance.config.storage_extensions["teradata"] = make_uniq<TeradataStorageExtension>();
}

std::string TeradataExtension::Name() {
	return "teradata";
}

std::string TeradataExtension::Version() const {
#ifdef EXT_VERSION_TERADATA
	return EXT_VERSION_TERADATA;
#else
	return "";
#endif
}

} // namespace duckdb

extern "C" {

DUCKDB_EXTENSION_API void teradata_init(duckdb::DatabaseInstance &db) {
	duckdb::DuckDB db_wrapper(db);
	db_wrapper.LoadExtension<duckdb::TeradataExtension>();
}

DUCKDB_EXTENSION_API const char *teradata_version() {
	return duckdb::DuckDB::LibraryVersion();
}
}

#ifndef DUCKDB_EXTENSION_MAIN
#error DUCKDB_EXTENSION_MAIN not defined
#endif
