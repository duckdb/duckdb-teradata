#define DUCKDB_EXTENSION_MAIN

#include "teradata_extension.hpp"
#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension_util.hpp"
#include "duckdb/parser/parsed_data/create_scalar_function_info.hpp"

#include "teradata_scan.hpp"

#include <teradata_storage.hpp>

namespace duckdb {

inline void QuackScalarFun(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &name_vector = args.data[0];
	UnaryExecutor::Execute<string_t, string_t>(name_vector, result, args.size(), [&](string_t name) {
		return StringVector::AddString(result, "Quack " + name.GetString() + " 🐥");
	});
}

static void LoadInternal(DatabaseInstance &instance) {
	// Register a scalar function
	auto quack_scalar_function = ScalarFunction("quack", {LogicalType::VARCHAR}, LogicalType::VARCHAR, QuackScalarFun);
	ExtensionUtil::RegisterFunction(instance, quack_scalar_function);

	TeradataScan::Register(instance);

	instance.config.storage_extensions["teradata"] = make_uniq<TeradataStorageExtension>();
}

void TeradataExtension::Load(DuckDB &db) {
	LoadInternal(*db.instance);
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
