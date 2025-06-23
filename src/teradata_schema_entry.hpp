#pragma once

#include "duckdb/catalog/catalog_entry/schema_catalog_entry.hpp"
#include "duckdb/catalog/default/default_table_functions.hpp"

#include "teradata_table_set.hpp"
#include "teradata_index_set.hpp"

namespace duckdb {

// Teradata doesnt have the equivalent of a "Schema" in the traditional sense, therefore we
// treat "Databases" as schemas.
class TeradataSchemaEntry final : public SchemaCatalogEntry {
public:
	TeradataSchemaEntry(Catalog &catalog, CreateSchemaInfo &info);

	void Scan(ClientContext &context, CatalogType type, const std::function<void(CatalogEntry &)> &callback) override;
	void Scan(CatalogType type, const std::function<void(CatalogEntry &)> &callback) override;
	optional_ptr<CatalogEntry> CreateIndex(CatalogTransaction transaction, CreateIndexInfo &info,
	                                       TableCatalogEntry &table) override;
	optional_ptr<CatalogEntry> CreateFunction(CatalogTransaction transaction, CreateFunctionInfo &info) override;
	optional_ptr<CatalogEntry> CreateTable(CatalogTransaction transaction, BoundCreateTableInfo &info) override;
	optional_ptr<CatalogEntry> CreateView(CatalogTransaction transaction, CreateViewInfo &info) override;
	optional_ptr<CatalogEntry> CreateSequence(CatalogTransaction transaction, CreateSequenceInfo &info) override;
	optional_ptr<CatalogEntry> CreateTableFunction(CatalogTransaction transaction,
	                                               CreateTableFunctionInfo &info) override;
	optional_ptr<CatalogEntry> CreateCopyFunction(CatalogTransaction transaction,
	                                              CreateCopyFunctionInfo &info) override;
	optional_ptr<CatalogEntry> CreatePragmaFunction(CatalogTransaction transaction,
	                                                CreatePragmaFunctionInfo &info) override;
	optional_ptr<CatalogEntry> CreateCollation(CatalogTransaction transaction, CreateCollationInfo &info) override;
	optional_ptr<CatalogEntry> CreateType(CatalogTransaction transaction, CreateTypeInfo &info) override;
	optional_ptr<CatalogEntry> LookupEntry(CatalogTransaction transaction, const EntryLookupInfo &lookup_info) override;
	void DropEntry(ClientContext &context, DropInfo &info) override;
	void Alter(CatalogTransaction transaction, AlterInfo &info) override;

private:
	TeradataTableSet tables;
	TeradataIndexSet indexes;

	// Builtin default functions
	mutex default_function_lock;
	case_insensitive_map_t<unique_ptr<CatalogEntry>> default_function_map;

	optional_ptr<CatalogEntry> TryLoadBuiltInFunction(const string &entry_name);
	optional_ptr<CatalogEntry> LoadBuiltInFunction(DefaultTableMacro macro);
};

} // namespace duckdb