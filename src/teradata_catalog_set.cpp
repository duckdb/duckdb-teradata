#include "teradata_catalog_set.hpp"
#include "teradata_schema_entry.hpp"
#include "teradata_transaction.hpp"
#include "teradata_connection.hpp"

#include "duckdb/catalog/catalog.hpp"
#include "duckdb/parser/parsed_data/drop_info.hpp"

namespace duckdb {
TeradataCatalogSet::TeradataCatalogSet(Catalog &catalog) : catalog(catalog) {
}

void TeradataCatalogSet::TryLoadEntries(ClientContext &context) {
	lock_guard<mutex> lock(load_lock);
	if (is_loaded) {
		return;
	}
	LoadEntries(context);
	is_loaded = true;
}

void TeradataCatalogSet::Scan(ClientContext &context, const std::function<void(CatalogEntry &)> &callback) {
	TryLoadEntries(context);
	// TODO: lock this too
	for (auto &entry : entries) {
		callback(*entry.second);
	}
}

optional_ptr<CatalogEntry> TeradataCatalogSet::CreateEntry(unique_ptr<CatalogEntry> entry) {
	lock_guard<mutex> l(entry_lock);
	auto result = entry.get();
	if (result->name.empty()) {
		throw InternalException("TeradataCatalogSet::CreateEntry called with empty name");
	}

	entry_map.emplace(result->name, result->name);
	entries.emplace(result->name, std::move(entry));

	return result;
}

void TeradataCatalogSet::DropEntry(ClientContext &context, DropInfo &info) {
	const auto &qname = info.GetQualifiedName();
	const auto &entry_name = qname.Name().GetIdentifierName();
	const auto &schema_name = qname.Schema().GetIdentifierName();

	string drop_query = "DROP ";
	drop_query += CatalogTypeToString(info.type) + " ";
	if (info.if_not_found == OnEntryNotFound::RETURN_NULL) {

		// Teradata doesnt support IF EXISTS, so check if the entry exists first
		const auto entry = GetEntry(context, entry_name);
		if (!entry) {
			return;
		}
	}

	if (!schema_name.empty()) {
		drop_query += KeywordHelper::WriteQuoted(schema_name, '"') + ".";
	}
	drop_query += KeywordHelper::WriteQuoted(entry_name, '"');
	if (info.cascade) {
		drop_query += "CASCADE";
	}

	// Execute the drop query
	const auto &transaction = TeradataTransaction::Get(context, catalog);
	auto &conn = transaction.GetConnection();

	conn.Execute(drop_query, context);

	// Erase the entry, and its case-insensitive alias, under the name it is stored as
	lock_guard<mutex> l(entry_lock);
	const auto stored_name = FindStoredName(entry_name);
	if (stored_name) {
		entries.erase(*stored_name);
		entry_map.erase(*stored_name);
	}
}

optional<string> TeradataCatalogSet::FindStoredName(const string &name) const {
	if (entries.find(name) != entries.end()) {
		return name;
	}
	const auto alias = entry_map.find(name);
	if (alias == entry_map.end() || entries.find(alias->second) == entries.end()) {
		return {};
	}
	return alias->second;
}

optional_ptr<CatalogEntry> TeradataCatalogSet::GetEntry(ClientContext &context, const string &name) {
	TryLoadEntries(context);
	lock_guard<mutex> l(entry_lock);
	// TODO: Try to reload the catalog if the entry is not found
	const auto stored_name = FindStoredName(name);
	return stored_name ? entries.find(*stored_name)->second.get() : nullptr;
}

TeradataInSchemaSet::TeradataInSchemaSet(TeradataSchemaEntry &schema)
    : TeradataCatalogSet(schema.ParentCatalog()), schema(schema) {
}

optional_ptr<CatalogEntry> TeradataInSchemaSet::CreateEntry(unique_ptr<CatalogEntry> entry) {
	entry->internal = schema.internal;
	return TeradataCatalogSet::CreateEntry(std::move(entry));
}

void TeradataCatalogSet::ClearEntries() {
	entry_map.clear();
	entries.clear();
	is_loaded = false;
}

} // namespace duckdb