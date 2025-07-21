#include "teradata_catalog.hpp"
#include "teradata_connection.hpp"
#include "teradata_common.hpp"

#include "teradata_schema_set.hpp"
#include "teradata_schema_entry.hpp"
#include "teradata_table_entry.hpp"

namespace duckdb {

//----------------------------------------------------------------------------------------------------------------------
// Initialization
//----------------------------------------------------------------------------------------------------------------------

TeradataCatalog::TeradataCatalog(AttachedDatabase &db, const string &logon_string, const string &database_to_load, idx_t buffer_size)
    : Catalog(db), schemas(*this, database_to_load), default_schema(database_to_load), buffer_size(buffer_size) {
	conn = make_uniq<TeradataConnection>(logon_string, buffer_size);
	path = logon_string;

	// No empty default schema
	if (default_schema.empty()) {
		throw InvalidInputException("No default schema provided for TeradataCatalog!");
	}
}

TeradataCatalog::~TeradataCatalog() {
}

void TeradataCatalog::Initialize(bool load_builtin) {
}

TeradataConnection &TeradataCatalog::GetConnection() const {
	return *conn;
}

//----------------------------------------------------------------------------------------------------------------------
// Schema Management
//----------------------------------------------------------------------------------------------------------------------

optional_ptr<CatalogEntry> TeradataCatalog::CreateSchema(CatalogTransaction transaction, CreateSchemaInfo &info) {
	throw NotImplementedException("TeradataCatalog::CreateSchema");
}

void TeradataCatalog::ScanSchemas(ClientContext &context, std::function<void(SchemaCatalogEntry &)> callback) {
	// Ok return a schema, with entries
	schemas.Scan(context, [&](CatalogEntry &schema) { callback(schema.Cast<TeradataSchemaEntry>()); });
}

optional_ptr<SchemaCatalogEntry> TeradataCatalog::LookupSchema(CatalogTransaction transaction,
                                                               const EntryLookupInfo &schema_lookup,
                                                               OnEntryNotFound if_not_found) {
	auto schema_name = schema_lookup.GetEntryName();
	if (schema_name == DEFAULT_SCHEMA) {
		schema_name = default_schema;
	}

	auto entry = schemas.GetEntry(transaction.GetContext(), schema_name);
	if (!entry && if_not_found != OnEntryNotFound::RETURN_NULL) {
		throw BinderException("Schema \"%s\" not found", schema_name);
	}
	return reinterpret_cast<SchemaCatalogEntry *>(entry.get());
}

void TeradataCatalog::DropSchema(ClientContext &context, DropInfo &info) {
	throw NotImplementedException("TeradataCatalog::DropSchema");
}

void TeradataCatalog::ClearCache() {
	schemas.ClearEntries();
}

//----------------------------------------------------------------------------------------------------------------------
// Metadata
//----------------------------------------------------------------------------------------------------------------------

DatabaseSize TeradataCatalog::GetDatabaseSize(ClientContext &context) {
	throw NotImplementedException("TeradataCatalog::GetDatabaseSize");
}

bool TeradataCatalog::InMemory() {
	return false;
}

string TeradataCatalog::GetCatalogType() {
	return "teradata";
}

string TeradataCatalog::GetDBPath() {
	return path;
}

} // namespace duckdb