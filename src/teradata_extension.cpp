#define DUCKDB_EXTENSION_MAIN

#include "teradata_extension.hpp"
#include "teradata_query.hpp"
#include "teradata_execute.hpp"
#include "teradata_storage.hpp"
#include "teradata_secret.hpp"
#include "teradata_clear_cache.hpp"
#include "teradata_common.hpp"

#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension_util.hpp"
#include "duckdb/parser/parsed_data/create_scalar_function_info.hpp"
#include "duckdb/main/connection_manager.hpp"
#include "duckdb/planner/extension_callback.hpp"

namespace duckdb {

//----------------------------------------------------------------------------------------------------------------------
// State
//----------------------------------------------------------------------------------------------------------------------

class TeradataExtensionState final : public ClientContextState {
public:
	bool CanRequestRebind() override {
		return true;
	}

	RebindQueryInfo OnPlanningError(ClientContext &context, SQLStatement &statement, ErrorData &error) override {
		if (error.Type() != ExceptionType::BINDER) {
			return RebindQueryInfo::DO_NOT_REBIND;
		}

		const auto &extra_info = error.ExtraInfo();
		const auto entry = extra_info.find("error_subtype");
		if (entry == extra_info.end()) {
			return RebindQueryInfo::DO_NOT_REBIND;
		}
		if (entry->second != "COLUMN_NOT_FOUND") {
			return RebindQueryInfo::DO_NOT_REBIND;
		}

		// Try to clear caches and rebind
		TeradataClearCacheFunction::Clear(context);
		return RebindQueryInfo::ATTEMPT_TO_REBIND;
	}
};

class TeradataExtensionCallback final : public ExtensionCallback {
	void OnConnectionOpened(ClientContext &context) override {
		context.registered_state->Insert("teradata_extension", make_shared_ptr<TeradataExtensionState>());
	}
};

//----------------------------------------------------------------------------------------------------------------------
// Extension Load
//----------------------------------------------------------------------------------------------------------------------

void TeradataExtension::Load(DuckDB &db) {
	auto &instance = *db.instance;

	// Register Teradata functions
	TeradataQueryFunction::Register(instance);
	TeradataExecuteFunction::Register(instance);
	TeradataClearCacheFunction::Register(instance);
	TeradataSecret::Register(instance);

	// Register storage
	instance.config.storage_extensions["teradata"] = make_uniq<TeradataStorageExtension>();

	// Register callbacks
	instance.config.extension_callbacks.push_back(make_uniq<TeradataExtensionCallback>());

	// Register states on existing connections
	for (const auto &con : ConnectionManager::Get(instance).GetConnectionList()) {
		con->registered_state->Insert("teradata_extension", make_shared_ptr<TeradataExtensionState>());
	}

	instance.config.AddExtensionOption(
		"teradata_use_primary_index",
		"Whether or not to use a primary index when creating Teradata tables",
		LogicalType::BOOLEAN, Value::BOOLEAN(true));

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
