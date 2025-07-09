#pragma once
#include "teradata_storage.hpp"
#include "teradata_connection.hpp"
#include "teradata_schema_set.hpp"

namespace duckdb {

class TeradataCatalog final : public Catalog {
public:
	explicit TeradataCatalog(AttachedDatabase &db, const string &logon_string, const string &databse_to_load, idx_t buffer_size);
	~TeradataCatalog() override;

public:
	// TODO: this should not be exposed like this. We should expose a pool instead?
	TeradataConnection &GetConnection() const;

public:
	void Initialize(bool load_builtin) override;

	optional_ptr<CatalogEntry> CreateSchema(CatalogTransaction transaction, CreateSchemaInfo &info) override;
	void ScanSchemas(ClientContext &context, std::function<void(SchemaCatalogEntry &)> callback) override;
	optional_ptr<SchemaCatalogEntry> LookupSchema(CatalogTransaction transaction, const EntryLookupInfo &schema_lookup,
	                                              OnEntryNotFound if_not_found) override;
	void DropSchema(ClientContext &context, DropInfo &info) override;

	PhysicalOperator &PlanInsert(ClientContext &context, PhysicalPlanGenerator &planner, LogicalInsert &op,
	                             optional_ptr<PhysicalOperator> plan) override;
	PhysicalOperator &PlanCreateTableAs(ClientContext &context, PhysicalPlanGenerator &planner, LogicalCreateTable &op,
	                                    PhysicalOperator &plan) override;
	PhysicalOperator &PlanDelete(ClientContext &context, PhysicalPlanGenerator &planner, LogicalDelete &op,
	                             PhysicalOperator &plan) override;
	PhysicalOperator &PlanUpdate(ClientContext &context, PhysicalPlanGenerator &planner, LogicalUpdate &op,
	                             PhysicalOperator &plan) override;

	unique_ptr<LogicalOperator> BindCreateIndex(Binder &binder, CreateStatement &stmt, TableCatalogEntry &table,
	                                            unique_ptr<LogicalOperator> plan) override;

	DatabaseSize GetDatabaseSize(ClientContext &context) override;
	bool InMemory() override;
	string GetCatalogType() override;
	string GetDBPath() override;

	static bool IsTeradataScan(const string &name) {
		return name == "teradata_query" || name == "teradata_scan";
	}

	void ClearCache();

private:
	unique_ptr<TeradataConnection> conn;
	string path;

	// The set of schemas in this database
	TeradataSchemaSet schemas;
	string default_schema;

	// Teradata response buffer size
	idx_t buffer_size;
};

} // namespace duckdb