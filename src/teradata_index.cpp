#include "teradata_index.hpp"
#include "teradata_catalog.hpp"

#include "duckdb/catalog/catalog_entry/index_catalog_entry.hpp"
#include "duckdb/planner/operator/logical_extension_operator.hpp"
#include "duckdb/parser/statement/create_statement.hpp"

#include <duckdb/catalog/catalog_entry/table_catalog_entry.hpp>
#include <duckdb/parser/parsed_data/drop_info.hpp>

namespace duckdb {

//----------------------------------------------------------------------------------------------------------------------
// Physical Operator
//----------------------------------------------------------------------------------------------------------------------
PhysicalTeradataCreateIndex::PhysicalTeradataCreateIndex(PhysicalPlan &plan, unique_ptr<CreateIndexInfo> info,
                                                         TableCatalogEntry &table)
    : PhysicalOperator(plan, PhysicalOperatorType::EXTENSION, {LogicalType::BIGINT}, 1), info(std::move(info)),
      table(table) {
}

SourceResultType PhysicalTeradataCreateIndex::GetData(ExecutionContext &context, DataChunk &chunk,
                                                      OperatorSourceInput &input) const {
	auto &catalog = table.catalog;
	auto &schema = table.schema;
	auto transaction = catalog.GetCatalogTransaction(context.client);
	auto existing = schema.GetEntry(transaction, CatalogType::INDEX_ENTRY, info->index_name);
	if (existing) {
		switch (info->on_conflict) {
		case OnCreateConflict::IGNORE_ON_CONFLICT:
			return SourceResultType::FINISHED;
		case OnCreateConflict::ERROR_ON_CONFLICT:
			throw BinderException("Index with name \"%s\" already exists in schema \"%s\"", info->index_name,
			                      table.schema.name);
		case OnCreateConflict::REPLACE_ON_CONFLICT: {
			DropInfo drop_info;
			drop_info.type = CatalogType::INDEX_ENTRY;
			drop_info.schema = info->schema;
			drop_info.name = info->index_name;
			schema.DropEntry(context.client, drop_info);
			break;
		}
		default:
			throw InternalException("Unsupported on create conflict");
		}
	}
	schema.CreateIndex(transaction, *info, table);

	return SourceResultType::FINISHED;
}

//----------------------------------------------------------------------------------------------------------------------
// Logical Operator
//----------------------------------------------------------------------------------------------------------------------
class LogicalTeradataCreateIndex final : public LogicalExtensionOperator {
public:
	LogicalTeradataCreateIndex(unique_ptr<CreateIndexInfo> info_p, TableCatalogEntry &table)
	    : info(std::move(info_p)), table(table) {
	}

	unique_ptr<CreateIndexInfo> info;
	TableCatalogEntry &table;

	PhysicalOperator &CreatePlan(ClientContext &context, PhysicalPlanGenerator &planner) override {
		return planner.Make<PhysicalTeradataCreateIndex>(std::move(info), table);
	}

	void Serialize(Serializer &serializer) const override {
		throw NotImplementedException("Cannot serialize TeradataCreateIndex operator");
	}

	void ResolveTypes() override {
		types = {LogicalType::BIGINT};
	}
};

//----------------------------------------------------------------------------------------------------------------------
// Bind
//----------------------------------------------------------------------------------------------------------------------
unique_ptr<LogicalOperator> TeradataCatalog::BindCreateIndex(Binder &binder, CreateStatement &stmt,
                                                             TableCatalogEntry &table,
                                                             unique_ptr<LogicalOperator> plan) {
	return make_uniq<LogicalTeradataCreateIndex>(unique_ptr_cast<CreateInfo, CreateIndexInfo>(std::move(stmt.info)),
	                                             table);
}

} // namespace duckdb
