#include "teradata_schema_entry.hpp"

#include <teradata_table_entry.hpp>

namespace duckdb {

TeradataSchemaEntry::TeradataSchemaEntry(Catalog &catalog, CreateSchemaInfo &info)
	: SchemaCatalogEntry(catalog, info), tables(*this) {
}

void TeradataSchemaEntry::Scan(CatalogType type, const std::function<void(CatalogEntry &)> &callback) {
	throw NotImplementedException("Scan without context not supported");
}

void TeradataSchemaEntry::Scan(ClientContext &context, CatalogType type, const std::function<void(CatalogEntry &)> &callback) {
	switch(type) {
		case CatalogType::TABLE_ENTRY:
			tables.Scan(context, [&](CatalogEntry &schema) { callback(schema.Cast<TeradataTableEntry>()); });
		break;
		default:
			throw InternalException("Type not supported for TeradataSchemaEntry::Scan");
	}
}

optional_ptr<CatalogEntry> TeradataSchemaEntry::CreateIndex(CatalogTransaction transaction, CreateIndexInfo &info, TableCatalogEntry &table) {
	throw NotImplementedException("TeradataSchemaEntry::CreateIndex");
}

optional_ptr<CatalogEntry> TeradataSchemaEntry::CreateFunction(CatalogTransaction transaction, CreateFunctionInfo &info) {
	throw NotImplementedException("TeradataSchemaEntry::CreateFunction");
}

optional_ptr<CatalogEntry> TeradataSchemaEntry::CreateTable(CatalogTransaction transaction, BoundCreateTableInfo &info) {
	throw NotImplementedException("TeradataSchemaEntry::CreateTable");
}

optional_ptr<CatalogEntry> TeradataSchemaEntry::CreateView(CatalogTransaction transaction, CreateViewInfo &info) {
	throw NotImplementedException("TeradataSchemaEntry::CreateView");
}

optional_ptr<CatalogEntry> TeradataSchemaEntry::CreateSequence(CatalogTransaction transaction, CreateSequenceInfo &info) {
	throw NotImplementedException("TeradataSchemaEntry::CreateSequence");
}

optional_ptr<CatalogEntry> TeradataSchemaEntry::CreateTableFunction(CatalogTransaction transaction, CreateTableFunctionInfo &info) {
	throw NotImplementedException("TeradataSchemaEntry::CreateTableFunction");
}

optional_ptr<CatalogEntry> TeradataSchemaEntry::CreateCopyFunction(CatalogTransaction transaction, CreateCopyFunctionInfo &info) {
	throw NotImplementedException("TeradataSchemaEntry::CreateCopyFunction");
}

optional_ptr<CatalogEntry> TeradataSchemaEntry::CreatePragmaFunction(CatalogTransaction transaction, CreatePragmaFunctionInfo &info) {
	throw NotImplementedException("TeradataSchemaEntry::CreatePragmaFunction");
}

optional_ptr<CatalogEntry> TeradataSchemaEntry::CreateCollation(CatalogTransaction transaction, CreateCollationInfo &info) {
	throw NotImplementedException("TeradataSchemaEntry::CreateCollation");
}

optional_ptr<CatalogEntry> TeradataSchemaEntry::CreateType(CatalogTransaction transaction, CreateTypeInfo &info) {
	throw NotImplementedException("TeradataSchemaEntry::CreateType");
}

optional_ptr<CatalogEntry> TeradataSchemaEntry::GetEntry(CatalogTransaction transaction, CatalogType type, const string &name) {
	throw NotImplementedException("TeradataSchemaEntry::GetEntry");
}

void TeradataSchemaEntry::DropEntry(ClientContext &context, DropInfo &info) {
	throw NotImplementedException("TeradataSchemaEntry::DropEntry");
}

void TeradataSchemaEntry::Alter(CatalogTransaction transaction, AlterInfo &info) {
	throw NotImplementedException("TeradataSchemaEntry::Alter");
}



}