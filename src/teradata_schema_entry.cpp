#include "teradata_schema_entry.hpp"
#include "teradata_table_entry.hpp"

#include "duckdb/catalog/default/default_table_functions.hpp"
#include "duckdb/catalog/catalog_entry/table_macro_catalog_entry.hpp"
#include "duckdb/parser/parsed_data/drop_info.hpp"
#include "duckdb/planner/parsed_data/bound_create_table_info.hpp"

namespace duckdb {

TeradataSchemaEntry::TeradataSchemaEntry(Catalog &catalog, CreateSchemaInfo &info)
    : SchemaCatalogEntry(catalog, info), tables(*this), indexes(*this) {
}

void TeradataSchemaEntry::Scan(CatalogType type, const std::function<void(CatalogEntry &)> &callback) {
	throw NotImplementedException("Scan without context not supported");
}

void TeradataSchemaEntry::Scan(ClientContext &context, CatalogType type,
                               const std::function<void(CatalogEntry &)> &callback) {
	switch (type) {
	case CatalogType::TABLE_ENTRY:
		tables.Scan(context, [&](CatalogEntry &schema) { callback(schema.Cast<TeradataTableEntry>()); });
		break;
	case CatalogType::INDEX_ENTRY:
		indexes.Scan(context, [&](CatalogEntry &schema) { callback(schema.Cast<TeradataIndexEntry>()); });
	default:
		break; // throw InternalException("Type not supported for TeradataSchemaEntry::Scan");
	}
}

optional_ptr<CatalogEntry> TeradataSchemaEntry::CreateIndex(CatalogTransaction transaction, CreateIndexInfo &info,
                                                            TableCatalogEntry &table) {

	return indexes.CreateIndex(transaction.GetContext(), info, table);
}

optional_ptr<CatalogEntry> TeradataSchemaEntry::CreateFunction(CatalogTransaction transaction,
                                                               CreateFunctionInfo &info) {
	throw NotImplementedException("TeradataSchemaEntry::CreateFunction");
}

optional_ptr<CatalogEntry> TeradataSchemaEntry::CreateTable(CatalogTransaction transaction,
                                                            BoundCreateTableInfo &info) {
	auto &base_info = info.Base();
	auto table_name = base_info.table;
	if (base_info.on_conflict == OnCreateConflict::REPLACE_ON_CONFLICT) {
		throw NotImplementedException("TeradataSchemaEntry::CreateTable REPLACE ON CONFLICT");
	}
	return tables.CreateTable(transaction.GetContext(), info);
}

optional_ptr<CatalogEntry> TeradataSchemaEntry::CreateView(CatalogTransaction transaction, CreateViewInfo &info) {
	throw NotImplementedException("TeradataSchemaEntry::CreateView");
}

optional_ptr<CatalogEntry> TeradataSchemaEntry::CreateSequence(CatalogTransaction transaction,
                                                               CreateSequenceInfo &info) {
	throw NotImplementedException("TeradataSchemaEntry::CreateSequence");
}

optional_ptr<CatalogEntry> TeradataSchemaEntry::CreateTableFunction(CatalogTransaction transaction,
                                                                    CreateTableFunctionInfo &info) {
	throw NotImplementedException("TeradataSchemaEntry::CreateTableFunction");
}

optional_ptr<CatalogEntry> TeradataSchemaEntry::CreateCopyFunction(CatalogTransaction transaction,
                                                                   CreateCopyFunctionInfo &info) {
	throw NotImplementedException("TeradataSchemaEntry::CreateCopyFunction");
}

optional_ptr<CatalogEntry> TeradataSchemaEntry::CreatePragmaFunction(CatalogTransaction transaction,
                                                                     CreatePragmaFunctionInfo &info) {
	throw NotImplementedException("TeradataSchemaEntry::CreatePragmaFunction");
}

optional_ptr<CatalogEntry> TeradataSchemaEntry::CreateCollation(CatalogTransaction transaction,
                                                                CreateCollationInfo &info) {
	throw NotImplementedException("TeradataSchemaEntry::CreateCollation");
}

optional_ptr<CatalogEntry> TeradataSchemaEntry::CreateType(CatalogTransaction transaction, CreateTypeInfo &info) {
	throw NotImplementedException("TeradataSchemaEntry::CreateType");
}

optional_ptr<CatalogEntry> TeradataSchemaEntry::LookupEntry(CatalogTransaction transaction,
                                                            const EntryLookupInfo &lookup_info) {
	switch (lookup_info.GetCatalogType()) {
	case CatalogType::TABLE_ENTRY:
		return tables.GetEntry(transaction.GetContext(), lookup_info.GetEntryName());
	case CatalogType::TABLE_FUNCTION_ENTRY:
		return TryLoadBuiltInFunction(lookup_info.GetEntryName());
	default:
		return nullptr;
	}
}

void TeradataSchemaEntry::DropEntry(ClientContext &context, DropInfo &info) {
	info.schema = name;
	switch (info.type) {
	case CatalogType::TABLE_ENTRY:
		tables.DropEntry(context, info);
		break;
	default:
		throw NotImplementedException("TeradataSchemaEntry::DropEntry");
	}
}

void TeradataSchemaEntry::Alter(CatalogTransaction transaction, AlterInfo &info) {
	throw NotImplementedException("TeradataSchemaEntry::Alter");
}

//----------------------------------------------------------------------------------------------------------------------
// Built-in Functions
//----------------------------------------------------------------------------------------------------------------------

// clang-format off
static const DefaultTableMacro td_table_macros[] = {
	{DEFAULT_SCHEMA, "query", {"sql", nullptr}, {{nullptr, nullptr}}, "FROM teradata_query({CATALOG}, sql)"},
	{nullptr, nullptr, {nullptr}, {{nullptr, nullptr}}, nullptr}
};
// clang-format on

optional_ptr<CatalogEntry> TeradataSchemaEntry::LoadBuiltInFunction(DefaultTableMacro macro) {
	string macro_def = macro.macro;
	macro_def = StringUtil::Replace(macro_def, "{CATALOG}", KeywordHelper::WriteQuoted(catalog.GetName(), '\''));
	macro_def = StringUtil::Replace(macro_def, "{SCHEMA}", KeywordHelper::WriteQuoted(name, '\''));
	macro.macro = macro_def.c_str();

	auto info = DefaultTableFunctionGenerator::CreateTableMacroInfo(macro);
	auto table_macro =
	    make_uniq_base<CatalogEntry, TableMacroCatalogEntry>(catalog, *this, info->Cast<CreateMacroInfo>());
	auto result = table_macro.get();
	default_function_map.emplace(macro.name, std::move(table_macro));
	return result;
}

optional_ptr<CatalogEntry> TeradataSchemaEntry::TryLoadBuiltInFunction(const string &entry_name) {
	lock_guard<mutex> guard(default_function_lock);
	auto entry = default_function_map.find(entry_name);
	if (entry != default_function_map.end()) {
		return entry->second.get();
	}
	for (idx_t index = 0; td_table_macros[index].name != nullptr; index++) {
		if (StringUtil::CIEquals(td_table_macros[index].name, entry_name)) {
			return LoadBuiltInFunction(td_table_macros[index]);
		}
	}
	return nullptr;
}

} // namespace duckdb