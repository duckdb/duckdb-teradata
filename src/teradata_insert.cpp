#include "teradata_insert.hpp"
#include "teradata_catalog.hpp"

#include "teradata_table_entry.hpp"
#include "teradata_transaction.hpp"
#include "teradata_request.hpp"
#include "teradata_query.hpp"

#include "duckdb/parser/statement/insert_statement.hpp"
#include "duckdb/planner/parsed_data/bound_create_table_info.hpp"
#include "duckdb/planner/operator/logical_insert.hpp"
#include "duckdb/execution/operator/scan/physical_table_scan.hpp"
#include "duckdb/execution/physical_plan_generator.hpp"

#include "teradata_column_writer.hpp"
#include "duckdb/planner/operator/logical_create_table.hpp"

#include <algorithm>
#include <iterator>

namespace duckdb {

//======================================================================================================================
// Teradata Insert
//======================================================================================================================

// Normal INSERT INTO table
TeradataInsert::TeradataInsert(PhysicalPlan &plan, LogicalOperator &op, TableCatalogEntry &table,
                               physical_index_vector_t<idx_t> column_index_map_p)
    : PhysicalOperator(plan, PhysicalOperatorType::EXTENSION, op.types, 1), table(&table), schema(nullptr),
      column_index_map(std::move(column_index_map_p)) {
}

// CREATE TABLE AS
TeradataInsert::TeradataInsert(PhysicalPlan &plan, LogicalOperator &op, SchemaCatalogEntry &schema,
                               unique_ptr<BoundCreateTableInfo> info)
    : PhysicalOperator(plan, PhysicalOperatorType::EXTENSION, op.types, 1), table(nullptr), schema(&schema),
      info(std::move(info)) {
}

//----------------------------------------------------------------------------------------------------------------------
// State
//----------------------------------------------------------------------------------------------------------------------
class TeradataInsertGlobalState final : public GlobalSinkState {
public:
	TeradataTableEntry *table = nullptr;
	idx_t insert_count = 0;
	string insert_sql;

	// TODO: move these into a state object
	ArenaAllocator arena;
	vector<unique_ptr<TeradataColumnWriter>> writers;

	explicit TeradataInsertGlobalState(ClientContext &context) : arena(BufferAllocator::Get(context)) {
	}
};

vector<PhysicalIndex> GetInsertColumnOrder(const physical_index_vector_t<idx_t> &column_index_map,
                                           idx_t table_column_count) {
	vector<PhysicalIndex> result;
	if (column_index_map.empty()) {
		// All columns, in table order
		for (idx_t i = 0; i < table_column_count; i++) {
			result.emplace_back(i);
		}
		return result;
	}
	// column_index_map maps each table column to its position in the input, or INVALID_INDEX if not inserted
	vector<pair<idx_t, PhysicalIndex>> positioned;
	for (idx_t col_idx = 0; col_idx < column_index_map.size(); col_idx++) {
		const auto input_idx = column_index_map[PhysicalIndex(col_idx)];
		if (input_idx != DConstants::INVALID_INDEX) {
			positioned.emplace_back(input_idx, PhysicalIndex(col_idx));
		}
	}
	std::sort(
	    positioned.begin(), positioned.end(),
	    [](const pair<idx_t, PhysicalIndex> &a, const pair<idx_t, PhysicalIndex> &b) { return a.first < b.first; });
	std::transform(positioned.begin(), positioned.end(), std::back_inserter(result),
	               [](const pair<idx_t, PhysicalIndex> &entry) { return entry.second; });
	return result;
}

string BuildInsertSQL(const string &schema_name, const string &table_name, const vector<InsertColumn> &columns) {
	vector<string> params;
	vector<string> values;
	for (auto &column : columns) {
		const auto quoted = KeywordHelper::WriteQuoted(column.name, '"');
		params.push_back(quoted + " " + TeradataType::FromDuckDB(column.type).ToString());
		values.push_back(":" + quoted);
	}
	const auto table = (schema_name.empty() ? "" : KeywordHelper::WriteQuoted(schema_name, '"') + ".") +
	                   KeywordHelper::WriteQuoted(table_name, '"');

	// The USING descriptor declares the parameters (names and types) that the VALUES clause refers to
	return "USING (" + StringUtil::Join(params, ", ") + ") INSERT INTO " + table + " VALUES (" +
	       StringUtil::Join(values, ", ") + ");";
}

static string GetInsertSQL(const TeradataInsert &insert, const TeradataTableEntry &entry) {
	auto &table_columns = entry.GetColumns();
	vector<InsertColumn> columns;
	for (auto &index : GetInsertColumnOrder(insert.column_index_map, table_columns.LogicalColumnCount())) {
		auto &column = table_columns.GetColumn(index);
		columns.push_back({column.GetName().GetIdentifierName(), column.GetType()});
	}
	return BuildInsertSQL(entry.schema.name.GetIdentifierName(), entry.name.GetIdentifierName(), columns);
}

unique_ptr<GlobalSinkState> TeradataInsert::GetGlobalSinkState(ClientContext &context) const {

	// If no table supplied, this is a CTAS
	if (!table) {
		// CTAS is not supported. Teradata doesnt support mixing DML and DDL in transactions
		throw BinderException("CREATE TABLE AS is not supported for Teradata tables. Please create the table first and "
		                      "then insert into it.");
	}

	// Prepare just to see that we type check
	// TODO: We should type check the statement somehow...
	// auto &transaction = TeradataTransaction::Get(context, insert_table->catalog);
	// auto &conn = transaction.GetConnection();
	const auto insert_table = &table.get_mutable()->Cast<TeradataTableEntry>();

	auto result = make_uniq<TeradataInsertGlobalState>(context);
	result->table = insert_table;
	result->insert_count = 0;
	result->insert_sql = GetInsertSQL(*this, *insert_table);

	const auto table_types = insert_table->GetTypes();
	result->writers.reserve(table_types.size());
	for (auto &type : table_types) {
		// Initialize writers
		result->writers.push_back(TeradataColumnWriter::Make(type));
	}

	return std::move(result);
}

//----------------------------------------------------------------------------------------------------------------------
// Sink
//----------------------------------------------------------------------------------------------------------------------
SinkResultType TeradataInsert::Sink(ExecutionContext &context, DataChunk &chunk, OperatorSinkInput &input) const {

	// Sink into the Teradata table
	auto &state = sink_state->Cast<TeradataInsertGlobalState>();
	auto &transaction = TeradataTransaction::Get(context.client, state.table->catalog);
	auto &conn = transaction.GetConnection();

	// Reset the arena before executing the query
	state.arena.Reset();

	// Execute, passing the data chunk as parameters.
	conn.Execute(state.insert_sql, chunk, state.arena, state.writers, context.client);
	state.insert_count += chunk.size();

	return SinkResultType::NEED_MORE_INPUT;
}

//----------------------------------------------------------------------------------------------------------------------
// Finalize
//----------------------------------------------------------------------------------------------------------------------
SinkFinalizeType TeradataInsert::Finalize(Pipeline &pipeline, Event &event, ClientContext &context,
                                          OperatorSinkFinalizeInput &input) const {
	return SinkFinalizeType::READY;
}

//----------------------------------------------------------------------------------------------------------------------
// GetData
//----------------------------------------------------------------------------------------------------------------------

SourceResultType TeradataInsert::GetDataInternal(ExecutionContext &context, DataChunk &chunk,
                                                 OperatorSourceInput &input) const {

	const auto &state = sink_state->Cast<TeradataInsertGlobalState>();
	chunk.SetCardinality(1);
	chunk.SetValue(0, 0, Value::BIGINT(UnsafeNumericCast<int64_t>(state.insert_count)));

	return SourceResultType::FINISHED;
}

//----------------------------------------------------------------------------------------------------------------------
// Misc
//----------------------------------------------------------------------------------------------------------------------

string TeradataInsert::GetName() const {
	return "TeradataInsert";
}

InsertionOrderPreservingMap<string> TeradataInsert::ParamsToString() const {
	InsertionOrderPreservingMap<string> result;
	result["Table Name"] = (table ? table->name : info->Base().GetTableName()).GetIdentifierName();
	return result;
}

//----------------------------------------------------------------------------------------------------------------------
// Plan
//----------------------------------------------------------------------------------------------------------------------

static void MaterializeTeradataScans(PhysicalOperator &op) {
	if (op.type == PhysicalOperatorType::TABLE_SCAN) {
		auto &table_scan = op.Cast<PhysicalTableScan>();
		if (TeradataCatalog::IsTeradataScan(table_scan.function.GetName().GetIdentifierName())) {
			auto &bind_data = table_scan.bind_data->Cast<TeradataBindData>();

			// If we are inserting into Teradata, materialize the all td-scans in this part of the plan
			bind_data.is_materialized = true;
		}
	}

	for (auto &child : op.children) {
		MaterializeTeradataScans(child);
	}
}

PhysicalOperator &TeradataCatalog::PlanInsert(ClientContext &context, PhysicalPlanGenerator &planner, LogicalInsert &op,
                                              optional_ptr<PhysicalOperator> plan) {

	if (op.return_chunk) {
		throw BinderException("RETURNING clause not yet supported for insertion into Teradata tables");
	}
	if (op.on_conflict_info.action_type != OnConflictAction::THROW) {
		throw BinderException("ON CONFLICT clause not yet supported for insertion into Teradata tables");
	}

	D_ASSERT(plan);

	MaterializeTeradataScans(*plan);

	// TODO: This is where we would cast to TD types if needed
	// plan = AddCastToTeradataTypes(context, std::move(plan));

	auto &insert = planner.Make<TeradataInsert>(op, op.table, op.column_index_map);
	insert.children.push_back(*plan);

	return insert;
}

PhysicalOperator &TeradataCatalog::PlanCreateTableAs(ClientContext &context, PhysicalPlanGenerator &planner,
                                                     LogicalCreateTable &op, PhysicalOperator &plan) {

	// TODO: This is where we would cast to TD types if needed
	// plan = AddCastToTeradataTypes(context, std::move(plan));

	MaterializeTeradataScans(plan);

	auto &insert = planner.Make<TeradataInsert>(op, op.schema, std::move(op.info));
	insert.children.push_back(plan);
	return insert;
}

} // namespace duckdb
