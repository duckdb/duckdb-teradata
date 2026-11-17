#pragma once

#include "duckdb/execution/physical_operator.hpp"
#include "duckdb/catalog/catalog_entry/table_catalog_entry.hpp"
#include "duckdb/catalog/catalog_entry/schema_catalog_entry.hpp"
#include "duckdb/planner/logical_operator.hpp"

namespace duckdb {

class TableFilterSet;

//! The WHERE clause for a DELETE/UPDATE from the filters pushed into its table scan
string TableFilterClause(const TableFilterSet &filters, const vector<string> &names,
                         const vector<ColumnIndex> &column_ids, const string &statement);

class TeradataDeleteUpdate final : public PhysicalOperator {
public:
	TeradataDeleteUpdate(PhysicalPlan &plan, LogicalOperator &op, TableCatalogEntry &table, string name, string query);

	//! The table to delete/update from
	TableCatalogEntry &table;
	//! The name of the operator
	string name;
	//! The generated statement
	string query;

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
