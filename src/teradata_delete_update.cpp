#include "teradata_delete_update.hpp"
#include "teradata_catalog.hpp"
#include "teradata_transaction.hpp"
#include "duckdb/catalog/catalog_entry/table_catalog_entry.hpp"
#include "duckdb/execution/operator/filter/physical_filter.hpp"
#include "duckdb/execution/operator/projection/physical_projection.hpp"
#include "duckdb/execution/operator/scan/physical_table_scan.hpp"
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
	connection.Execute(query);

	gstate.affected_rows = 0; // result->AffectedRows();
	return SinkFinalizeType::READY;
}

SourceResultType TeradataDeleteUpdate::GetData(ExecutionContext &context, DataChunk &chunk,
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
	result["Table Name"] = table.name;
	return result;
}

//----------------------------------------------------------------------------------------------------------------------
// Plan
//----------------------------------------------------------------------------------------------------------------------

// Attempt to reconstruct the WHERE clause from the physical operator tree
static string ExtractFilters(PhysicalOperator &child, const string &statement) {
	switch (child.type) {
	case PhysicalOperatorType::FILTER: {
		const auto &filter = child.Cast<PhysicalFilter>();
		const auto result = ExtractFilters(child.children[0], statement);
		auto filter_str = filter.expression->ToString();
		if (result.empty()) {
			return filter_str;
		}
		return result + " AND " + filter_str;
	}
	case PhysicalOperatorType::PROJECTION: {
		const auto &proj = child.Cast<PhysicalProjection>();
		for (auto &expr : proj.select_list) {
			switch (expr->type) {
			case ExpressionType::BOUND_REF:
			case ExpressionType::BOUND_COLUMN_REF:
			case ExpressionType::VALUE_CONSTANT:
				break;
			default:
				throw NotImplementedException("Unsupported expression type in projection - only simple deletes/updates "
				                              "are supported in the Teradata connector");
			}
		}
		return ExtractFilters(child.children[0], statement);
	}
	case PhysicalOperatorType::TABLE_SCAN: {
		const auto &table_scan = child.Cast<PhysicalTableScan>();
		if (!table_scan.table_filters) {
			return string();
		}
		string result;
		for (auto &entry : table_scan.table_filters->filters) {
			const auto column_index = entry.first;
			const auto &filter = entry.second;
			string column_name;
			if (column_index < table_scan.names.size()) {
				const auto col_id = table_scan.column_ids[column_index].GetPrimaryIndex();
				if (col_id == COLUMN_IDENTIFIER_ROW_ID) {
					throw NotImplementedException("RowID column not supported in Teradata DELETE statements - "
					                              "use a primary key or unique index instead");
				}
				column_name = table_scan.names[col_id];
			}
			BoundReferenceExpression bound_ref(std::move(column_name), LogicalTypeId::INVALID, 0);
			const auto filter_expr = filter->ToExpression(bound_ref);
			auto filter_str = filter_expr->ToString();
			if (result.empty()) {
				result = std::move(filter_str);
			} else {
				result += " AND " + filter_str;
			}
		}
		return result;
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
	result += KeywordHelper::WriteQuoted(op.table.schema.name, '"');
	result += ".";
	result += KeywordHelper::WriteQuoted(op.table.name, '"');
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
	result += KeywordHelper::WriteQuoted(op.table.schema.name, '"');
	result += ".";
	result += KeywordHelper::WriteQuoted(op.table.name, '"');
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
		result += KeywordHelper::WriteQuoted(col.GetName(), '"');
		result += " = ";
		if (op.expressions[c]->type == ExpressionType::VALUE_DEFAULT) {
			result += "DEFAULT";
			continue;
		}
		if (op.expressions[c]->type != ExpressionType::BOUND_REF) {
			throw NotImplementedException("Teradata Update not supported - Expected a bound reference expression");
		}
		auto &ref = op.expressions[c]->Cast<BoundReferenceExpression>();
		result += proj.select_list[ref.index]->ToString();
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
