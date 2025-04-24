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

namespace duckdb {

static string GetDuckDBToTeradataTypeString(const LogicalType &type) {
	// Convert the DuckDB type to a Teradata type, then get the string representation of that type
	return TeradataType::FromDuckDB(type).ToString();
}

static string GetTeradataColumnsDefSQL(const ColumnList &columns, const vector<unique_ptr<Constraint>> &constraints) {
	std::stringstream ss;

	ss << "(";

	// find all columns that have NOT NULL specified, but are NOT primary key
	// columns
	logical_index_set_t not_null_columns;
	logical_index_set_t unique_columns;
	logical_index_set_t pk_columns;
	unordered_set<string> multi_key_pks;
	vector<string> extra_constraints;
	for (auto &constraint : constraints) {
		if (constraint->type == ConstraintType::NOT_NULL) {
			auto &not_null = constraint->Cast<NotNullConstraint>();
			not_null_columns.insert(not_null.index);
		} else if (constraint->type == ConstraintType::UNIQUE) {
			auto &pk = constraint->Cast<UniqueConstraint>();
			vector<string> constraint_columns = pk.columns;
			if (pk.index.index != DConstants::INVALID_INDEX) {
				// no columns specified: single column constraint
				if (pk.is_primary_key) {
					pk_columns.insert(pk.index);
				} else {
					unique_columns.insert(pk.index);
				}
			} else {
				// multi-column constraint, this constraint needs to go at the end after
				// all columns
				if (pk.is_primary_key) {
					// multi key pk column: insert set of columns into multi_key_pks
					for (auto &col : pk.columns) {
						multi_key_pks.insert(col);
					}
				}
				extra_constraints.push_back(constraint->ToString());
			}
		} else if (constraint->type == ConstraintType::FOREIGN_KEY) {
			auto &fk = constraint->Cast<ForeignKeyConstraint>();
			if (fk.info.type == ForeignKeyType::FK_TYPE_FOREIGN_KEY_TABLE ||
			    fk.info.type == ForeignKeyType::FK_TYPE_SELF_REFERENCE_TABLE) {
				extra_constraints.push_back(constraint->ToString());
			}
		} else {
			extra_constraints.push_back(constraint->ToString());
		}
	}

	for (auto &column : columns.Logical()) {
		if (column.Oid() > 0) {
			ss << ", ";
		}
		ss << KeywordHelper::WriteQuoted(column.Name(), '"') << " ";
		ss << GetDuckDBToTeradataTypeString(column.Type());
		bool not_null = not_null_columns.find(column.Logical()) != not_null_columns.end();
		bool is_single_key_pk = pk_columns.find(column.Logical()) != pk_columns.end();
		bool is_multi_key_pk = multi_key_pks.find(column.Name()) != multi_key_pks.end();
		bool is_unique = unique_columns.find(column.Logical()) != unique_columns.end();
		if (not_null && !is_single_key_pk && !is_multi_key_pk) {
			// NOT NULL but not a primary key column
			ss << " NOT NULL";
		}
		if (is_single_key_pk) {
			// single column pk: insert constraint here
			ss << " PRIMARY KEY";
		}
		if (is_unique) {
			// single column unique: insert constraint here
			ss << " UNIQUE";
		}
		if (column.Generated()) {
			throw NotImplementedException("Generated columns are not supported in Teradata");
		} else if (column.HasDefaultValue()) {
			ss << " DEFAULT (" << column.DefaultValue().ToString() << ")";
		}
	}
	// print any extra constraints that still need to be printed
	for (auto &extra_constraint : extra_constraints) {
		ss << ", ";
		ss << extra_constraint;
	}

	ss << ")";
	return ss.str();
}

static string GetTeradataCreateTableSQL(const CreateTableInfo &info) {
	std::stringstream ss;

	ss << "CREATE TABLE ";
	if (info.on_conflict == OnCreateConflict::IGNORE_ON_CONFLICT) {
		ss << "IF NOT EXISTS ";
	}
	if (!info.schema.empty()) {
		ss << KeywordHelper::WriteQuoted(info.schema, '"');
		ss << ".";
	}
	ss << KeywordHelper::WriteQuoted(info.table, '"');
	ss << GetTeradataColumnsDefSQL(info.columns, info.constraints);
	ss << ";";
	return ss.str();
}

optional_ptr<CatalogEntry> TeradataTableSet::CreateTable(ClientContext &context, BoundCreateTableInfo &info) {
	auto &transaction = TeradataTransaction::Get(context, catalog).Cast<TeradataTransaction>();

	auto &base = info.Base();
	const auto create_sql = GetTeradataCreateTableSQL(base);

	// Execute the sql statement
	transaction.GetConnection().Execute(create_sql);

	auto tbl_entry = make_uniq<TeradataTableEntry>(catalog, schema, base);
	return CreateEntry(std::move(tbl_entry));
}

void TeradataTableSet::LoadEntries(ClientContext &context) {
	const auto &td_catalog = catalog.Cast<TeradataCatalog>();
	auto &td_schema = schema.Cast<TeradataSchemaEntry>();

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
	                                      "WHERE T.DatabaseName = '%s' AND T.TableKind = 'T' "
	                                      "ORDER BY T.TableName, C.ColumnId",
	                                      td_schema.name);

	/*
	const auto query = StringUtil::Format("SELECT TableName, ColumnName, ColumnType FROM dbc.ColumnsV "
	                                      "WHERE DatabaseName = '%s' AND ColumnType IS NOT NULL "
	                                      "ORDER BY TableName, ColumnId",
	                                      td_schema.name);
	*/
	const auto result = conn.Query(query, false);

	TeradataTableInfo info;
	info.schema = td_schema.name;

	bool skip_table = false;

	// Iterate over the chunks in the result
	for (auto &chunk : result->Chunks()) {
		chunk.Flatten();
		const auto count = chunk.size();
		auto &tbl_name_vec = chunk.data[0];
		auto &col_name_vec = chunk.data[1];
		auto &col_type_vec = chunk.data[2];
		auto &col_size_vec = chunk.data[3];

		for (idx_t row_idx = 0; row_idx < count; row_idx++) {
			const auto tbl_name = FlatVector::GetData<string_t>(tbl_name_vec)[row_idx];
			const auto col_name = FlatVector::GetData<string_t>(col_name_vec)[row_idx];
			const auto col_type = FlatVector::GetData<string_t>(col_type_vec)[row_idx];
			const auto col_size = FlatVector::GetData<int32_t>(col_size_vec)[row_idx];

			if (tbl_name != info.table) {
				// We have a new table
				if (!info.table.empty() && !skip_table) {
					// Finish the previous table
					entries[info.table] = make_uniq<TeradataTableEntry>(catalog, td_schema, info);
				}
				skip_table = false;

				// Reset the info
				info = TeradataTableInfo();
				info.table = tbl_name.GetString();
				info.schema = td_schema.name;
			}

			// Add the columns
			try {
				TeradataType td_type = TeradataType::FromShortCode(col_type.GetData());
				if (td_type.HasLengthModifier()) {
					td_type.SetLength(col_size);
				}

				const auto duck_type = td_type.ToDuckDB();
				info.columns.AddColumn(ColumnDefinition(col_name.GetString(), duck_type));
				info.teradata_types.push_back(td_type);
			} catch (...) {
				// Ignore the column type (and this table) for now, just make this work
				skip_table = true;
			}
		}
	}

	if (!info.table.empty() && !skip_table) {
		// Finish the last table
		entries[info.table] = make_uniq<TeradataTableEntry>(catalog, td_schema, info);
	}
}

} // namespace duckdb