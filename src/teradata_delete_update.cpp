#include "teradata_delete_update.hpp"
#include "teradata_catalog.hpp"
#include "teradata_transaction.hpp"
#include "duckdb/catalog/catalog_entry/table_catalog_entry.hpp"
#include "duckdb/execution/operator/filter/physical_filter.hpp"
#include "duckdb/execution/operator/projection/physical_projection.hpp"
#include "duckdb/execution/operator/scan/physical_table_scan.hpp"
#include "duckdb/execution/physical_plan_generator.hpp"
#include "duckdb/planner/table_filter_set.hpp"

#include <algorithm>
#include "duckdb/planner/expression/bound_reference_expression.hpp"
#include "duckdb/planner/operator/logical_delete.hpp"
#include "duckdb/planner/operator/logical_update.hpp"

namespace duckdb {

//----------------------------------------------------------------------------------------------------------------------
// Delete Operator
//----------------------------------------------------------------------------------------------------------------------

TeradataDeleteUpdate::TeradataDeleteUpdate(PhysicalPlan &plan, LogicalOperator &op, TableCatalogEntry &table,
                                           string name, string query)
    : PhysicalOperator(plan, PhysicalOperatorType::EXTENSION, op.types, 1), table(table), name(std::move(name)),
      query(std::move(query)) {
}

class TeradataDeleteUpdateGlobalState final : public GlobalSinkState {
public:
	idx_t affected_rows = 0;
};

unique_ptr<GlobalSinkState> TeradataDeleteUpdate::GetGlobalSinkState(ClientContext &context) const {
	return make_uniq<TeradataDeleteUpdateGlobalState>();
}

SinkResultType TeradataDeleteUpdate::Sink(ExecutionContext &context, DataChunk &chunk, OperatorSinkInput &input) const {
	// This operator does not actually sink any data, it just generates a DELETE statement
	return SinkResultType::FINISHED;
}

SinkFinalizeType TeradataDeleteUpdate::Finalize(Pipeline &pipeline, Event &event, ClientContext &context,
                                                OperatorSinkFinalizeInput &input) const {
	auto &gstate = input.global_state.Cast<TeradataDeleteUpdateGlobalState>();
	auto &transaction = TeradataTransaction::Get(context, table.catalog);
	auto &connection = transaction.GetConnection();

	// TODO:
	// We cant get affected rows from the DELETE statement, we need to inspect the query log.
	// So for now, always return 0 affected rows.
	connection.Execute(query, context);

	gstate.affected_rows = 0; // result->AffectedRows();
	return SinkFinalizeType::READY;
}

SourceResultType TeradataDeleteUpdate::GetDataInternal(ExecutionContext &context, DataChunk &chunk,
                                                       OperatorSourceInput &input) const {
	auto &insert_gstate = sink_state->Cast<TeradataDeleteUpdateGlobalState>();
	chunk.SetCardinality(1);
	chunk.SetValue(0, 0, Value::BIGINT(insert_gstate.affected_rows));

	return SourceResultType::FINISHED;
}

string TeradataDeleteUpdate::GetName() const {
	return name;
}

InsertionOrderPreservingMap<string> TeradataDeleteUpdate::ParamsToString() const {
	InsertionOrderPreservingMap<string> result;
	result["Table Name"] = table.name.GetIdentifierName();
	return result;
}

//----------------------------------------------------------------------------------------------------------------------
// Plan
//----------------------------------------------------------------------------------------------------------------------

static string JoinConjuncts(const string &left, const string &right) {
	if (left.empty() || right.empty()) {
		return left.empty() ? right : left;
	}
	return left + " AND " + right;
}

// The column a pushed-down filter applies to; empty if the filter is not on a table column
static Identifier FilterColumnName(idx_t column_index, const vector<string> &names,
                                   const vector<ColumnIndex> &column_ids) {
	if (column_index >= names.size()) {
		return Identifier();
	}
	const auto col_id = column_ids[column_index].GetPrimaryIndex();
	if (col_id == COLUMN_IDENTIFIER_ROW_ID) {
		throw NotImplementedException("RowID column not supported in Teradata DELETE statements - "
		                              "use a primary key or unique index instead");
	}
	return Identifier(names[col_id]);
}

string TableFilterClause(const TableFilterSet &filters, const vector<string> &names,
                         const vector<ColumnIndex> &column_ids, const string &statement) {
	if (filters.HasMultiColumnFilters()) {
		throw NotImplementedException("Multi-column filters are not supported in Teradata %s statements", statement);
	}
	string result;
	for (const auto &entry : filters) {
		BoundReferenceExpression column(FilterColumnName(entry.GetIndex(), names, column_ids), LogicalTypeId::INVALID,
		                                0);
		result = JoinConjuncts(result, entry.Filter().ToExpression(column)->ToString());
	}
	return result;
}

static bool IsSimpleProjection(const Expression &expr) {
	switch (expr.GetExpressionType()) {
	case ExpressionType::BOUND_REF:
	case ExpressionType::BOUND_COLUMN_REF:
	case ExpressionType::VALUE_CONSTANT:
		return true;
	default:
		return false;
	}
}

static void CheckSimpleProjection(const PhysicalProjection &proj) {
	const auto all_simple = std::all_of(proj.select_list.begin(), proj.select_list.end(),
	                                    [](const unique_ptr<Expression> &expr) { return IsSimpleProjection(*expr); });
	if (!all_simple) {
		throw NotImplementedException("Unsupported expression type in projection - only simple deletes/updates "
		                              "are supported in the Teradata connector");
	}
}

// Attempt to reconstruct the WHERE clause from the physical operator tree
static string ExtractFilters(PhysicalOperator &child, const string &statement) {
	switch (child.type) {
	case PhysicalOperatorType::FILTER:
		return JoinConjuncts(ExtractFilters(child.children[0], statement),
		                     child.Cast<PhysicalFilter>().expression->ToString());
	case PhysicalOperatorType::PROJECTION:
		CheckSimpleProjection(child.Cast<PhysicalProjection>());
		return ExtractFilters(child.children[0], statement);
	case PhysicalOperatorType::TABLE_SCAN: {
		const auto &table_scan = child.Cast<PhysicalTableScan>();
		return table_scan.table_filters
		           ? TableFilterClause(*table_scan.table_filters, table_scan.names, table_scan.column_ids, statement)
				   : string();
	}
	default:
		throw NotImplementedException("Unsupported operator type %s in %s statement - only simple deletes "
		                              "(e.g. %s "
		                              "FROM tbl WHERE x=y) are supported in the Teradata connector",
		                              PhysicalOperatorToString(child.type), statement, statement);
	}
}

//----------------------------------------------------------------------------------------------------------------------
// Plan DELETE
//----------------------------------------------------------------------------------------------------------------------

// Attempt to reconstruct the DELETE statement from the logical delete operator and its child
static string ConstructDeleteStatement(LogicalDelete &op, PhysicalOperator &child) {
	string result = "DELETE FROM ";
	result += KeywordHelper::WriteQuoted(op.table.schema.name.GetIdentifierName(), '"');
	result += ".";
	result += KeywordHelper::WriteQuoted(op.table.name.GetIdentifierName(), '"');
	const auto filters = ExtractFilters(child, "DELETE");
	if (!filters.empty()) {
		result += " WHERE " + filters;
	}
	return result;
}

PhysicalOperator &TeradataCatalog::PlanDelete(ClientContext &context, PhysicalPlanGenerator &planner, LogicalDelete &op,
                                              PhysicalOperator &plan) {
	if (op.return_chunk) {
		throw BinderException("RETURNING clause not yet supported for deletion of a Teradata table");
	}

	auto &execute =
	    planner.Make<TeradataDeleteUpdate>(op, op.table, "TERADATA_DELETE", ConstructDeleteStatement(op, plan));
	execute.children.push_back(plan);
	return execute;
}

//----------------------------------------------------------------------------------------------------------------------
// Plan UPDATE
//----------------------------------------------------------------------------------------------------------------------

static string ConstructUpdateStatement(LogicalUpdate &op, PhysicalOperator &child) {
	// FIXME - all of this is pretty gnarly, we should provide a hook earlier on
	// in the planning process to convert this into a SQL statement
	string result = "UPDATE";
	result += KeywordHelper::WriteQuoted(op.table.schema.name.GetIdentifierName(), '"');
	result += ".";
	result += KeywordHelper::WriteQuoted(op.table.name.GetIdentifierName(), '"');
	result += " SET ";
	if (child.type != PhysicalOperatorType::PROJECTION) {
		throw NotImplementedException("Teradata Update not supported - Expected the "
		                              "child of an update to be a projection");
	}
	auto &proj = child.Cast<PhysicalProjection>();
	for (idx_t c = 0; c < op.columns.size(); c++) {
		if (c > 0) {
			result += ", ";
		}
		auto &col = op.table.GetColumn(op.table.GetColumns().PhysicalToLogical(op.columns[c]));
		result += KeywordHelper::WriteQuoted(col.GetName().GetIdentifierName(), '"');
		result += " = ";
		if (op.expressions[c]->GetExpressionType() == ExpressionType::VALUE_DEFAULT) {
			result += "DEFAULT";
			continue;
		}
		if (op.expressions[c]->GetExpressionType() != ExpressionType::BOUND_REF) {
			throw NotImplementedException("Teradata Update not supported - Expected a bound reference expression");
		}
		auto &ref = op.expressions[c]->Cast<BoundReferenceExpression>();
		result += proj.select_list[ref.Index()]->ToString();
	}
	result += " ";
	auto filters = ExtractFilters(child.children[0], "UPDATE");
	if (!filters.empty()) {
		result += " WHERE " + filters;
	}
	return result;
}

PhysicalOperator &TeradataCatalog::PlanUpdate(ClientContext &context, PhysicalPlanGenerator &planner, LogicalUpdate &op,
                                              PhysicalOperator &plan) {
	if (op.return_chunk) {
		throw BinderException("RETURNING clause not yet supported for updates of a Teradata table");
	}

	auto &execute =
	    planner.Make<TeradataDeleteUpdate>(op, op.table, "TERADATA_UPDATE", ConstructUpdateStatement(op, plan));
	execute.children.push_back(plan);
	return execute;
}

} // namespace duckdb
