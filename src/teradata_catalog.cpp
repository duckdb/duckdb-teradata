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

TeradataCatalog::TeradataCatalog(AttachedDatabase &db, const string &logon_string, const string &database_to_load)
    : Catalog(db), schemas(*this, database_to_load), default_schema(database_to_load) {
	conn = make_uniq<TeradataConnection>(logon_string);
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

optional_ptr<SchemaCatalogEntry> TeradataCatalog::GetSchema(CatalogTransaction transaction, const string &schema_name,
                                                            OnEntryNotFound if_not_found,
                                                            QueryErrorContext error_context) {
	if (schema_name == DEFAULT_SCHEMA) {
		return GetSchema(transaction, default_schema, if_not_found, error_context);
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
// Table Management
//----------------------------------------------------------------------------------------------------------------------

unique_ptr<PhysicalOperator> TeradataCatalog::PlanCreateTableAs(ClientContext &context, LogicalCreateTable &op,
                                                                unique_ptr<PhysicalOperator> plan) {

	throw NotImplementedException("TeradataCatalog::PlanCreateTableAs");
}

unique_ptr<PhysicalOperator> TeradataCatalog::PlanDelete(ClientContext &context, LogicalDelete &op,
                                                         unique_ptr<PhysicalOperator> plan) {

	throw NotImplementedException("TeradataCatalog::PlanDelete");
}

unique_ptr<PhysicalOperator> TeradataCatalog::PlanUpdate(ClientContext &context, LogicalUpdate &op,
                                                         unique_ptr<PhysicalOperator> plan) {

	throw NotImplementedException("TeradataCatalog::PlanUpdate");
}

unique_ptr<LogicalOperator> TeradataCatalog::BindCreateIndex(Binder &binder, CreateStatement &stmt,
                                                             TableCatalogEntry &table,
                                                             unique_ptr<LogicalOperator> plan) {

	throw NotImplementedException("TeradataCatalog::BindCreateIndex");
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