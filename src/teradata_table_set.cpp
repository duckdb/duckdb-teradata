#include "teradata_table_set.hpp"
#include "teradata_table_entry.hpp"
#include "teradata_schema_entry.hpp"
#include "teradata_catalog.hpp"
#include "teradata_request.hpp"

#include <duckdb/parser/parsed_data/create_table_info.hpp>

namespace duckdb {

void TeradataTableSet::LoadEntries(ClientContext &context) {
	const auto &td_catalog = catalog.Cast<TeradataCatalog>();
	auto &td_schema = schema.Cast<TeradataSchemaEntry>();

	const auto &conn = td_catalog.GetConnection();

	// TODO: Sanitize the schema name
	const auto query = StringUtil::Format(
			"SELECT T.TableName, C.ColumnName, C.ColumnType FROM dbc.TablesV AS T JOIN dbc.ColumnsV AS C ON T.TableName = C.TableName AND T.DatabaseName = C.DatabaseName WHERE T.DatabaseName = '%s' AND T.TableKind = 'T' ORDER BY T.TableName, C.ColumnName"
		, td_schema.name);

	const auto cdc = TeradataSqlRequest::Execute(conn, query);

	CreateTableInfo info;
	info.schema = td_schema.name;

	for(auto &chunk : cdc->Chunks()) {
		chunk.Flatten();
		const auto count = chunk.size();
		auto &tbl_name_vec = chunk.data[0];
		auto &col_name_vec = chunk.data[1];
		auto &col_type_vec = chunk.data[2];

		for(idx_t row_idx = 0; row_idx < count; row_idx++) {
			const auto tbl_name = FlatVector::GetData<string_t>(tbl_name_vec)[row_idx].GetString();
			const auto col_name = FlatVector::GetData<string_t>(col_name_vec)[row_idx].GetString();
			const auto col_type = FlatVector::GetData<string_t>(col_type_vec)[row_idx].GetString();

			if(tbl_name != info.table) {
				// We have a new table
				if(!info.table.empty()) {
					// Finish the previous table
					entries[info.table] = make_uniq<TeradataTableEntry>(catalog, td_schema, info);
				}

				// Reset the info
				info = CreateTableInfo();
				info.table = tbl_name;
				info.schema = td_schema.name;
			}

			// Add the columns
			try {
				const auto td_type_id = TeradataType::FromShortCode(col_type.c_str());
				const auto td_type = TeradataType(td_type_id, TeradataTypeVariant::STANDARD, 0, 0);

				info.columns.AddColumn(ColumnDefinition(col_name, td_type.GetDuckType()));
			} catch (...) {
				// Ignore the column type for now, just make this work
				info.columns.AddColumn(ColumnDefinition(col_name, LogicalType::BLOB));
			}
		}
	}

	if(!info.table.empty()) {
		// Finish the last table
		entries[info.table] = make_uniq<TeradataTableEntry>(catalog, td_schema, info);
	}
}

} // namespace duckdb