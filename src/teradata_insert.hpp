#pragma once

#include "duckdb/execution/physical_operator.hpp"
#include "duckdb/catalog/catalog_entry/table_catalog_entry.hpp"
#include "duckdb/catalog/catalog_entry/schema_catalog_entry.hpp"
#include "duckdb/planner/logical_operator.hpp"
#include "duckdb/planner/parsed_data/bound_create_table_info.hpp"
#include "duckdb/common/index_vector.hpp"

namespace duckdb {

//! A column an INSERT writes to
struct InsertColumn {
	string name;
	LogicalType type;
};

//! The table columns an INSERT writes, in the order they appear in its input.
//! `column_index_map` maps each table column to its input position (INVALID_INDEX if not inserted); empty means all.
vector<PhysicalIndex> GetInsertColumnOrder(const physical_index_vector_t<idx_t> &column_index_map,
                                           idx_t table_column_count);

//! A parameterized Teradata INSERT statement for the given columns
string BuildInsertSQL(const string &schema_name, const string &table_name, const vector<InsertColumn> &columns);

class TeradataInsert final : public PhysicalOperator {
public:
	//! INSERT INTO
	TeradataInsert(PhysicalPlan &plan, LogicalOperator &op, TableCatalogEntry &table,
	               physical_index_vector_t<idx_t> column_index_map);
	//! CREATE TABLE AS
	TeradataInsert(PhysicalPlan &plan, LogicalOperator &op, SchemaCatalogEntry &schema,
	               unique_ptr<BoundCreateTableInfo> info);

	//! The table to insert into
	optional_ptr<TableCatalogEntry> table;
	//! Table schema, in case of CREATE TABLE AS
	optional_ptr<SchemaCatalogEntry> schema;
	//! Create table info, in case of CREATE TABLE AS
	unique_ptr<BoundCreateTableInfo> info;
	//! column_index_map
	physical_index_vector_t<idx_t> column_index_map;

public:
	// Source interface
	SourceResultType GetDataInternal(ExecutionContext &context, DataChunk &chunk,
	                                 OperatorSourceInput &input) const override;

	bool IsSource() const override {
		return true;
	}

public:
	// Sink interface
	unique_ptr<GlobalSinkState> GetGlobalSinkState(ClientContext &context) const override;
	SinkResultType Sink(ExecutionContext &context, DataChunk &chunk, OperatorSinkInput &input) const override;
	SinkFinalizeType Finalize(Pipeline &pipeline, Event &event, ClientContext &context,
	                          OperatorSinkFinalizeInput &input) const override;

	bool IsSink() const override {
		return true;
	}

	bool ParallelSink() const override {
		return false;
	}

	string GetName() const override;
	InsertionOrderPreservingMap<string> ParamsToString() const override;
};

} // namespace duckdb
