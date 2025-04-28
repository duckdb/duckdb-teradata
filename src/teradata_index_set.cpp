#include "teradata_index_set.hpp"

#include "duckdb/parser/parsed_expression_iterator.hpp"
#include "duckdb/parser/expression/columnref_expression.hpp"
#include "duckdb/catalog/catalog_entry/table_catalog_entry.hpp"
#include "duckdb/catalog/catalog_entry/schema_catalog_entry.hpp"

#include <teradata_catalog.hpp>
#include <teradata_connection.hpp>
#include <teradata_schema_entry.hpp>
#include <teradata_transaction.hpp>

namespace duckdb {

TeradataIndexSet::TeradataIndexSet(TeradataSchemaEntry &schema) : TeradataInSchemaSet(schema) {
}

static void UnqualifyColumnReferences(ParsedExpression &expr) {
	if (expr.type == ExpressionType::COLUMN_REF) {
		auto &colref = expr.Cast<ColumnRefExpression>();
		auto name = std::move(colref.column_names.back());
		colref.column_names = {std::move(name)};
		return;
	}
	ParsedExpressionIterator::EnumerateChildren(expr, UnqualifyColumnReferences);
}

static string GetCreateIndexSQL(CreateIndexInfo &info, TableCatalogEntry &tbl) {
	string sql;
	sql = "CREATE";
	if (info.constraint_type == IndexConstraintType::UNIQUE) {
		sql += " UNIQUE";
	}
	sql += " INDEX ";
	sql += KeywordHelper::WriteOptionallyQuoted(info.index_name, '"');
	sql += "(";
	for (idx_t i = 0; i < info.parsed_expressions.size(); i++) {
		if (i > 0) {
			sql += ", ";
		}
		UnqualifyColumnReferences(*info.parsed_expressions[i]);
		sql += info.parsed_expressions[i]->ToString();
	}
	sql += ")";
	sql += " ON ";
	sql += KeywordHelper::WriteOptionallyQuoted(tbl.schema.name, '"') + ".";
	sql += KeywordHelper::WriteOptionallyQuoted(tbl.name, '"');
	return sql;
}

optional_ptr<CatalogEntry> TeradataIndexSet::CreateIndex(ClientContext &context, CreateIndexInfo &info,
                                                         TableCatalogEntry &table) {

	auto &td_transcation = TeradataTransaction::Get(context, table.catalog);
	auto &td_connection = td_transcation.GetConnection();

	const auto sql = GetCreateIndexSQL(info, table);
	td_connection.Execute(sql);

	auto index_entry = make_uniq<TeradataIndexEntry>(schema.ParentCatalog(), schema, info, table.name);
	return CreateEntry(std::move(index_entry));
}

void TeradataIndexSet::LoadEntries(ClientContext &context) {
	const auto &td_catalog = catalog.Cast<TeradataCatalog>();
	auto &conn = td_catalog.GetConnection();

	const auto query = StringUtil::Format("SELECT T.TableName, I.IndexName, I.IndexType, I.ColumnName, I.UniqueFlag "
	                                      "FROM DBC.IndicesV AS I JOIN DBC.TablesV AS T "
	                                      "ON T.TableName = I.TableName AND T.DatabaseName = I.DatabaseName "
	                                      "WHERE T.DatabaseName = '%s' AND T.TableKind = 'T' "
	                                      "ORDER BY T.TableName, I.IndexName, I.IndexType, I.ColumnName, I.UniqueFlag;",
	                                      schema.name);

	const auto result = conn.Query(query, false);

	for (auto &chunk : result->Chunks()) {
		chunk.Flatten();

		const auto count = chunk.size();

		const auto &tbl_name_vec = chunk.data[0];
		const auto &idx_name_vec = chunk.data[1];
		const auto &idx_type_vec = chunk.data[2];
		const auto &col_name_vec = chunk.data[3];
		const auto &idx_uniq_vec = chunk.data[4];

		const auto tbl_names = FlatVector::GetData<string_t>(tbl_name_vec);
		const auto idx_names = FlatVector::GetData<string_t>(idx_name_vec);
		const auto idx_types = FlatVector::GetData<string_t>(idx_type_vec);
		const auto col_names = FlatVector::GetData<string_t>(col_name_vec);
		const auto idx_flags = FlatVector::GetData<string_t>(idx_uniq_vec);

		for (idx_t row_idx = 0; row_idx < count; row_idx++) {

			// TODO: Come up with a temp name if the index name is empty?
			if (FlatVector::IsNull(idx_name_vec, row_idx)) {
				continue;
				;
			}

			auto idx_name = idx_names[row_idx].GetString();
			const auto tbl_name = tbl_names[row_idx].GetString();
			const auto idx_type = idx_types[row_idx].GetString(); // TODO: Parse this
			const auto col_name = col_names[row_idx].GetString();

			auto idx_flag = IndexConstraintType::NONE;
			if (idx_flags[row_idx].GetData()[0] == 'Y') {
				idx_flag = IndexConstraintType::UNIQUE;
			}

			CreateIndexInfo info;
			info.schema = schema.name;
			info.table = tbl_name;
			info.index_name = idx_name;
			info.index_type = idx_type;
			info.constraint_type = idx_flag;

			// TODO: We need to check if there are multiple columns in the index
			info.expressions.push_back(make_uniq<ColumnRefExpression>(col_name));

			auto index_entry = make_uniq<TeradataIndexEntry>(catalog, schema, info, info.table);
			CreateEntry(std::move(index_entry));
		}
	}
}

} // namespace duckdb