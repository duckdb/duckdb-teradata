#include "teradata_clear_cache.hpp"
#include "teradata_catalog.hpp"

#include "duckdb/parser/parsed_data/create_table_function_info.hpp"
#include "duckdb/main/database_manager.hpp"
#include "duckdb/main/attached_database.hpp"
#include "duckdb/main/extension/extension_loader.hpp"

namespace duckdb {

class ClearCacheFunctionData final : public TableFunctionData {
public:
	bool finished = false;
};

static unique_ptr<FunctionData> ClearCacheBind(ClientContext &context, TableFunctionBindInput &input,
                                               vector<LogicalType> &return_types, vector<string> &names) {

	auto result = make_uniq<ClearCacheFunctionData>();
	return_types.push_back(LogicalType::BOOLEAN);
	names.emplace_back("Success");
	return std::move(result);
}

void TeradataClearCacheFunction::Clear(ClientContext &context) {
	auto databases = DatabaseManager::Get(context).GetDatabases(context);
	for (auto &db_ref : databases) {
		auto &db = *db_ref.get();
		auto &catalog = db.GetCatalog();
		if (catalog.GetCatalogType() != "teradata") {
			continue;
		}
		catalog.Cast<TeradataCatalog>().ClearCache();
	}
}

static void ClearCacheFunction(ClientContext &context, TableFunctionInput &data_p, DataChunk &output) {
	auto &data = data_p.bind_data->CastNoConst<ClearCacheFunctionData>();
	if (data.finished) {
		return;
	}
	TeradataClearCacheFunction::Clear(context);
	data.finished = true;
}

void TeradataClearCacheFunction::Register(ExtensionLoader &loader) {
	TableFunction func("teradata_clear_cache", {}, ClearCacheFunction, ClearCacheBind);
	loader.RegisterFunction(func);
}

} // namespace duckdb
