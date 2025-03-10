#include "teradata_insert.hpp"
#include "teradata_catalog.hpp"

#include "teradata_table_entry.hpp"
#include "teradata_transaction.hpp"
#include "teradata_request.hpp"

#include "duckdb/parser/statement/insert_statement.hpp"
#include "duckdb/planner/parsed_data/bound_create_table_info.hpp"
#include "duckdb/planner/operator/logical_insert.hpp"
#include "duckdb/execution/operator/scan/physical_table_scan.hpp"

namespace duckdb {

//======================================================================================================================
// Teradata Insert
//======================================================================================================================

// Normal INSERT INTO table
TeradataInsert::TeradataInsert(LogicalOperator &op, TableCatalogEntry &table,
							   physical_index_vector_t<idx_t> column_index_map_p)
	: PhysicalOperator(PhysicalOperatorType::EXTENSION, op.types, 1), table(&table), schema(nullptr),
	  column_index_map(std::move(column_index_map_p)) {
}

//----------------------------------------------------------------------------------------------------------------------
// State
//----------------------------------------------------------------------------------------------------------------------
class TeradataInsertGlobalState final : public GlobalSinkState {
public:
	TeradataTableEntry *table;
	idx_t insert_count;
	string insert_sql;
};

static string GetInsertSQL(const TeradataInsert& insert, const TeradataTableEntry &entry) {

	// First, figure out what columns we are inserting
	auto &columns = entry.GetColumns();
	idx_t column_count = columns.LogicalColumnCount();
	vector<PhysicalIndex> column_indices;

	if(insert.column_index_map.empty()) {
		for(idx_t i = 0; i < column_count; i++) {
			column_indices.emplace_back(i);
		}
	} else {
		column_indices.resize(column_count, PhysicalIndex(DConstants::INVALID_INDEX));
		column_count = 0;
		for(idx_t entry_idx = 0; entry_idx < insert.column_index_map.size(); entry_idx++) {
			const auto col_idx = PhysicalIndex(entry_idx);
			const auto mapped_idx = insert.column_index_map[col_idx];
			if(mapped_idx == DConstants::INVALID_INDEX) {
				// Column not specified
				continue;
			}
			column_indices[mapped_idx] = col_idx;
			column_count++;
		}
	}

	// Now construct the SQL string.
	// We first construct the USING descriptor with the column names and types so that we can pass parameters.
	// Then we construct the INSERT INTO statement and the VALUES clause with the parameter names.

	string result = "USING ";
	result += "(";
	for(idx_t i = 0; i < column_count; i++) {
		if(i > 0) {
			result += ", ";
		}

		auto &col = columns.GetColumn(column_indices[i]);
		result += KeywordHelper::WriteOptionallyQuoted(col.GetName());
		result += " ";

		// TODO: we need to convert to the Teradata type first
		result += col.GetType().ToString();
	}
	result += ") ";

	result += "INSERT INTO ";

	if(!entry.schema.name.empty()) {
		result += KeywordHelper::WriteOptionallyQuoted(entry.schema.name) + ".";
	}
	result += KeywordHelper::WriteOptionallyQuoted(entry.name);

	result += " VALUES (";

	for(idx_t i = 0; i < column_count; i++) {
		if(i > 0) {
			result += ", ";
		}
		auto &col = columns.GetColumn(column_indices[i]);
		result += ":" + KeywordHelper::WriteOptionallyQuoted(col.GetName());
	}
	result += ");";
	return result;
}

unique_ptr<GlobalSinkState> TeradataInsert::GetGlobalSinkState(ClientContext &context) const {

	if(!table) {
		throw NotImplementedException("Teradata CTAS");
	}

	TeradataTableEntry *insert_table = &table.get_mutable()->Cast<TeradataTableEntry>();

	// Prepare just to see that we type check
	// TODO: We should type check the statement somehow...
	//auto &transaction = TeradataTransaction::Get(context, insert_table->catalog).Cast<TeradataTransaction>();
	//auto &conn = transaction.GetConnection();

	auto result = make_uniq<TeradataInsertGlobalState>();
	result->table = insert_table;
	result->insert_count = 0;
	result->insert_sql = GetInsertSQL(*this, *insert_table);

	return std::move(result);
}

//----------------------------------------------------------------------------------------------------------------------
// Sink
//----------------------------------------------------------------------------------------------------------------------
SinkResultType TeradataInsert::Sink(ExecutionContext &context, DataChunk &chunk, OperatorSinkInput &input) const {

	// Sink into the Teradata table
	auto &state = sink_state->Cast<TeradataInsertGlobalState>();
	auto &transaction = TeradataTransaction::Get(context.client, state.table->catalog).Cast<TeradataTransaction>();
	auto &conn = transaction.GetConnection();

	conn.Execute(state.insert_sql, chunk);
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

SourceResultType TeradataInsert::GetData(ExecutionContext &context, DataChunk &chunk,
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
	result["Table Name"] = table ? table->name : info->Base().table;
	return result;
}

//----------------------------------------------------------------------------------------------------------------------
// Plan
//----------------------------------------------------------------------------------------------------------------------

static void MaterializeTeradataScans(PhysicalOperator &op) {
	if(op.type == PhysicalOperatorType::TABLE_SCAN) {
		auto &table_scan = op.Cast<PhysicalTableScan>();
		if(TeradataCatalog::IsTeradataScan(table_scan.function.name)) {
			//auto &bind_data = table_scan.bind_data->Cast<TeradataBindData>();
			throw NotImplementedException("Cannot yet insert into Teradata tables while also scanning from them");
		}
	}

	for(auto &child : op.children) {
		MaterializeTeradataScans(*child);
	}
}

unique_ptr<PhysicalOperator> TeradataCatalog::PlanInsert(ClientContext &context, LogicalInsert &op,
														 unique_ptr<PhysicalOperator> plan) {

	if(op.return_chunk) {
		throw BinderException("RETURNING clause not yet supported for insertion into Teradata tables");
	}
	if(op.action_type != OnConflictAction::THROW) {
		throw BinderException("ON CONFLICT clause not yet supported for insertion into Teradata tables");
	}

	MaterializeTeradataScans(*plan);

	//plan = AddCastToTeradataTypes(context, std::move(plan));

	auto insert = make_uniq<TeradataInsert>(op, op.table, op.column_index_map);
	insert->children.push_back(std::move(plan));
	return std::move(insert);
}


} // namespace duckdb