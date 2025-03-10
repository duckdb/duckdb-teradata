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
	entries.emplace(result->name, std::move(entry));
	return result;
}

void TeradataCatalogSet::DropEntry(ClientContext &context, DropInfo &info) {
	string drop_query = "DROP ";
	drop_query += CatalogTypeToString(info.type) + " ";
	if (info.if_not_found == OnEntryNotFound::RETURN_NULL) {
		drop_query += " IF EXISTS ";
	}
	if (!info.schema.empty()) {
		drop_query += KeywordHelper::WriteQuoted(info.schema, '"') + ".";
	}
	drop_query += KeywordHelper::WriteQuoted(info.name, '"');
	if (info.cascade) {
		drop_query += "CASCADE";
	}

	// Execute the drop query
	const auto &transaction = TeradataTransaction::Get(context, catalog).Cast<TeradataTransaction>();
	auto &conn = transaction.GetConnection();

	conn.Execute(drop_query);

	// erase the entry from the catalog set
	lock_guard<mutex> l(entry_lock);
	entries.erase(info.name);
}

optional_ptr<CatalogEntry> TeradataCatalogSet::GetEntry(ClientContext &context, const string &name) {
	TryLoadEntries(context);
	{
		lock_guard<mutex> l(entry_lock);
		auto entry = entries.find(name);
		if (entry != entries.end()) {
			return entry->second.get();
		}
	}

	// entry not found
	// TODO: Try to reload the catalog!

	return nullptr;
}

TeradataInSchemaSet::TeradataInSchemaSet(TeradataSchemaEntry &schema)
    : TeradataCatalogSet(schema.ParentCatalog()), schema(schema) {
}

optional_ptr<CatalogEntry> TeradataInSchemaSet::CreateEntry(unique_ptr<CatalogEntry> entry) {
	entry->internal = schema.internal;
	return TeradataCatalogSet::CreateEntry(std::move(entry));
}

} // namespace duckdb