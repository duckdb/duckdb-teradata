#include "teradata_catalog.hpp"
#include "teradata_connection.hpp"
#include "teradata_common.hpp"

namespace duckdb {

//----------------------------------------------------------------------------------------------------------------------
// Initialization
//----------------------------------------------------------------------------------------------------------------------

TeradataCatalog::TeradataCatalog(AttachedDatabase &db) : Catalog(db) {
	string logon = "127.0.0.1/dbc,dbc";
	conn = make_uniq<TeradataConnection>(logon);
}

TeradataCatalog::~TeradataCatalog() {
}

void TeradataCatalog::Initialize(bool load_builtin) {
}

//----------------------------------------------------------------------------------------------------------------------
// Schema Management
//----------------------------------------------------------------------------------------------------------------------

optional_ptr<CatalogEntry> TeradataCatalog::CreateSchema(CatalogTransaction transaction, CreateSchemaInfo &info) {
	throw NotImplementedException("TeradataCatalog::CreateSchema");
}

void TeradataCatalog::ScanSchemas(ClientContext &context, std::function<void(SchemaCatalogEntry &)> callback) {
	throw NotImplementedException("TeradataCatalog::ScanSchemas");
}

optional_ptr<SchemaCatalogEntry> TeradataCatalog::GetSchema(CatalogTransaction transaction, const string &schema_name,
                                                            OnEntryNotFound if_not_found,
                                                            QueryErrorContext error_context) {
	throw NotImplementedException("TeradataCatalog::GetSchema");
}

void TeradataCatalog::DropSchema(ClientContext &context, DropInfo &info) {
	throw NotImplementedException("TeradataCatalog::DropSchema");
}

//----------------------------------------------------------------------------------------------------------------------
// Table Management
//----------------------------------------------------------------------------------------------------------------------

unique_ptr<PhysicalOperator> TeradataCatalog::PlanInsert(ClientContext &context, LogicalInsert &op,
                                                         unique_ptr<PhysicalOperator> plan) {
	throw NotImplementedException("TeradataCatalog::PlanInsert");
}

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
	throw NotImplementedException("TeradataCatalog::InMemory");
}

string TeradataCatalog::GetCatalogType() {
	throw NotImplementedException("TeradataCatalog::GetCatalogType");
}

string TeradataCatalog::GetDBPath() {
	throw NotImplementedException("TeradataCatalog::GetDBPath");
}

} // namespace duckdb