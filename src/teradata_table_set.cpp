#include "teradata_table_set.hpp"
#include "teradata_table_entry.hpp"
#include "teradata_schema_entry.hpp"
#include "teradata_catalog.hpp"
#include "teradata_request.hpp"

#include <duckdb/parser/parsed_data/create_table_info.hpp>

namespace duckdb {

void TeradataTableSet::LoadEntries(ClientContext &context) {
	const auto &td_catalog = catalog.Cast<TeradataCatalog>();
	const auto &td_schema = schema.Cast<TeradataSchemaEntry>();

	const auto &conn = td_catalog.GetConnection();

	// TODO: Sanitize the schema name
	const auto cdc = TeradataSqlRequest::Execute(conn,
		StringUtil::Format("sel TableName, RequestText, CommentString from dbc.TablesV tv where DataBaseName = '%s' and TableKind = 'T';"));

	CreateTableInfo info;

	// Now iterate over the result and create the table entries


}

} // namespace duckdb