#include "teradata_table_set.hpp"
#include "teradata_table_entry.hpp"
#include "teradata_schema_entry.hpp"
#include "teradata_catalog.hpp"
#include "teradata_transaction.hpp"

#include "duckdb/parser/constraints/foreign_key_constraint.hpp"
#include "duckdb/parser/constraints/not_null_constraint.hpp"
#include "duckdb/parser/constraints/unique_constraint.hpp"
#include "duckdb/parser/parsed_data/create_table_info.hpp"
#include "duckdb/planner/parsed_data/bound_create_table_info.hpp"

#include <algorithm>

namespace duckdb {

namespace {

//! The constraints of a table, indexed the way the column definitions need them
struct ConstraintSummary {
	logical_index_set_t not_null_columns;
	logical_index_set_t unique_columns;
	logical_index_set_t primary_key_columns;
	//! Columns of a multi-column primary key
	unordered_set<string> multi_key_pk_columns;
	//! Constraints that go after the columns
	vector<string> table_constraints;
};

void AddUniqueConstraint(ConstraintSummary &summary, const UniqueConstraint &unique) {
	if (unique.index.index != DConstants::INVALID_INDEX) {
		// Single column constraint
		auto &columns = unique.is_primary_key ? summary.primary_key_columns : summary.unique_columns;
		columns.insert(unique.index);
		return;
	}
	// Multi-column constraint, this goes at the end after all columns
	if (unique.is_primary_key) {
		for (auto &col : unique.columns) {
			summary.multi_key_pk_columns.insert(col.GetIdentifierName());
		}
	}
	summary.table_constraints.push_back(unique.ToString());
}

bool IsTableForeignKey(const ForeignKeyConstraint &fk) {
	return fk.info.type == ForeignKeyType::FK_TYPE_FOREIGN_KEY_TABLE ||
	       fk.info.type == ForeignKeyType::FK_TYPE_SELF_REFERENCE_TABLE;
}

ConstraintSummary SummarizeConstraints(const vector<unique_ptr<Constraint>> &constraints) {
	ConstraintSummary summary;
	for (auto &constraint : constraints) {
		switch (constraint->type) {
		case ConstraintType::NOT_NULL:
			summary.not_null_columns.insert(constraint->Cast<NotNullConstraint>().index);
			break;
		case ConstraintType::UNIQUE:
			AddUniqueConstraint(summary, constraint->Cast<UniqueConstraint>());
			break;
		case ConstraintType::FOREIGN_KEY:
			if (IsTableForeignKey(constraint->Cast<ForeignKeyConstraint>())) {
				summary.table_constraints.push_back(constraint->ToString());
			}
			break;
		default:
			summary.table_constraints.push_back(constraint->ToString());
			break;
		}
	}
	return summary;
}

string ColumnDefinitionSQL(const ColumnDefinition &column, const ConstraintSummary &summary) {
	if (column.Generated()) {
		throw NotImplementedException("Generated columns are not supported in Teradata");
	}
	const auto contains = [&](const logical_index_set_t &set) {
		return set.count(column.Logical()) > 0;
	};
	const auto is_single_key_pk = contains(summary.primary_key_columns);
	const auto is_multi_key_pk = summary.multi_key_pk_columns.count(column.Name().GetIdentifierName()) > 0;

	vector<string> parts {KeywordHelper::WriteQuoted(column.Name().GetIdentifierName(), '"'),
	                      TeradataType::FromDuckDB(column.Type()).ToString()};
	if (contains(summary.not_null_columns) && !is_single_key_pk && !is_multi_key_pk) {
		parts.emplace_back("NOT NULL");
	}
	if (is_single_key_pk) {
		parts.emplace_back("PRIMARY KEY NOT NULL");
	}
	if (contains(summary.unique_columns)) {
		parts.emplace_back("UNIQUE");
	}
	if (column.HasDefaultValue()) {
		parts.push_back("DEFAULT (" + column.DefaultValue().ToString() + ")");
	}
	return StringUtil::Join(parts, " ");
}

string GetTeradataColumnsDefSQL(const ColumnList &columns, const vector<unique_ptr<Constraint>> &constraints,
                                bool use_primary_index) {
	const auto summary = SummarizeConstraints(constraints);
	vector<string> definitions;
	for (auto &column : columns.Logical()) {
		definitions.push_back(ColumnDefinitionSQL(column, summary));
	}
	definitions.insert(definitions.end(), summary.table_constraints.begin(), summary.table_constraints.end());
	return "(" + StringUtil::Join(definitions, ", ") + ")" + (use_primary_index ? "" : " NO PRIMARY INDEX");
}

//! A column of a catalog table, or nothing if its type is not supported
optional<pair<ColumnDefinition, TeradataType>> TryConvertColumn(const TeradataColumnRow &row) {
	TeradataType td_type = TeradataType::FromShortCode(row.type_code);
	if (td_type.GetId() == TeradataTypeId::INVALID) {
		return {};
	}
	if (td_type.HasLengthModifier()) {
		td_type.SetLength(row.length);
	}
	try {
		return make_pair(ColumnDefinition(Identifier(row.column_name), td_type.ToDuckDB()), td_type);
	} catch (Exception &) {
		return {};
	}
}

//! The definition of one table from its column rows, or nullptr if any column has an unsupported type
unique_ptr<TeradataTableInfo> TryBuildTableInfo(const Identifier &schema_name,
                                                vector<TeradataColumnRow>::const_iterator begin,
                                                vector<TeradataColumnRow>::const_iterator end) {
	auto info = make_uniq<TeradataTableInfo>();
	info->SetQualifiedName(QualifiedName(Identifier(), schema_name, Identifier(begin->table_name)));
	for (auto row = begin; row != end; ++row) {
		auto column = TryConvertColumn(*row);
		if (!column) {
			return nullptr;
		}
		info->columns.AddColumn(std::move(column->first));
		info->teradata_types.push_back(column->second);
	}
	return info;
}

vector<TeradataColumnRow> ReadColumnRows(TeradataQueryResult &result) {
	vector<TeradataColumnRow> rows;
	for (auto &chunk : result.Chunks()) {
		chunk.Flatten();
		const auto tbl_names = FlatVector::GetData<string_t>(chunk.data[0]);
		const auto col_names = FlatVector::GetData<string_t>(chunk.data[1]);
		const auto col_types = FlatVector::GetData<string_t>(chunk.data[2]);
		const auto col_sizes = FlatVector::GetData<int32_t>(chunk.data[3]);
		for (idx_t row_idx = 0; row_idx < chunk.size(); row_idx++) {
			rows.push_back({tbl_names[row_idx].GetString(), col_names[row_idx].GetString(),
			                col_types[row_idx].GetString(), col_sizes[row_idx]});
		}
	}
	return rows;
}

} // namespace

vector<unique_ptr<TeradataTableInfo>> BuildTableInfos(const Identifier &schema_name,
                                                      const vector<TeradataColumnRow> &rows) {
	vector<unique_ptr<TeradataTableInfo>> result;
	for (auto begin = rows.begin(); begin != rows.end();) {
		const auto end = std::find_if(
		    begin, rows.end(), [&](const TeradataColumnRow &row) { return row.table_name != begin->table_name; });
		auto info = TryBuildTableInfo(schema_name, begin, end);
		if (info) {
			result.push_back(std::move(info));
		}
		begin = end;
	}
	return result;
}

string GetTeradataCreateTableSQL(const CreateTableInfo &info, bool use_primary_index) {
	std::stringstream ss;

	ss << "CREATE TABLE ";
	if (info.on_conflict == OnCreateConflict::IGNORE_ON_CONFLICT) {
		ss << "IF NOT EXISTS ";
	}
	const auto &schema_name = info.GetQualifiedName().Schema();
	if (!schema_name.empty()) {
		ss << KeywordHelper::WriteQuoted(schema_name.GetIdentifierName(), '"');
		ss << ".";
	}
	ss << KeywordHelper::WriteQuoted(info.GetTableName().GetIdentifierName(), '"');
	ss << GetTeradataColumnsDefSQL(info.columns, info.constraints, use_primary_index);
	ss << ";";

	return ss.str();
}

optional_ptr<CatalogEntry> TeradataTableSet::CreateTable(ClientContext &context, BoundCreateTableInfo &info) {
	auto &transaction = TeradataTransaction::Get(context, catalog);

	Value use_primary_index_value;
	bool use_primary_index = true;
	if (context.TryGetCurrentSetting("teradata_use_primary_index", use_primary_index_value)) {
		use_primary_index = use_primary_index_value.GetValue<bool>();
	}

	auto &base = info.Base();
	const auto create_sql = GetTeradataCreateTableSQL(base, use_primary_index);

	// Execute the sql statement
	transaction.GetConnection().Execute(create_sql, context);

	auto tbl_entry = make_uniq<TeradataTableEntry>(catalog, schema, base);
	return CreateEntry(std::move(tbl_entry));
}

void TeradataTableSet::LoadEntries(ClientContext &context) {
	const auto &td_catalog = catalog.Cast<TeradataCatalog>();

	auto &conn = td_catalog.GetConnection();

	/*
	    "The DBC.ColumnsV[X] views provide complete information for table columns but provide only limited information
	    for view columns. For view columns, DBC.ColumnsV[X] provides a NULL value for ColumnType, DecimalTotalDigits,
	    DecimalFractionalDigits, CharType, ColumnLength, and other attributes related to data type."

	    Therefore, we can filter on "ColumnType IS NOT NULL" to get only tables, without having to join with the
	    DBC.TablesV view.
	 */

	// TODO: Sanitize the schema name
	const auto query = StringUtil::Format("SELECT T.TableName, C.ColumnName, C.ColumnType, C.ColumnLength "
	                                      "FROM dbc.TablesV AS T JOIN dbc.ColumnsV AS C "
	                                      "ON T.TableName = C.TableName AND T.DatabaseName = C.DatabaseName "
	                                      "WHERE T.DatabaseName = '%s' AND (T.TableKind = 'T' OR T.TableKind = 'O') "
	                                      "ORDER BY T.TableName, C.ColumnId",
	                                      schema.name.GetIdentifierName());

	const auto result = conn.Query(query, false, context);

	for (auto &info : BuildTableInfos(schema.name, ReadColumnRows(*result))) {
		const auto table_name = info->GetTableName().GetIdentifierName();
		entries[table_name] = make_uniq<TeradataTableEntry>(catalog, schema, *info);
	}
}

} // namespace duckdb