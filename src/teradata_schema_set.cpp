#include "teradata_schema_set.hpp"
#include "teradata_catalog.hpp"
#include "teradata_schema_entry.hpp"

#include "duckdb/parser/parsed_data/create_schema_info.hpp"

namespace duckdb {

void TeradataSchemaSet::LoadEntries(ClientContext &context) {
	const auto &td_catalog = catalog.Cast<TeradataCatalog>();

	// We have to issue a query to get the list of schemas
	auto &conn = td_catalog.GetConnection();

	const auto result = conn.Query("SELECT DatabaseName, CommentString FROM DBC.DatabasesV", false);

	// Now iterate over the result and create the schema entries
	for(auto &chunk : result->Chunks()) {
		chunk.Flatten();
		const auto count = chunk.size();
		auto &name_vec = chunk.data[0];
		auto &comment_vec = chunk.data[1];

		for(idx_t row_idx = 0; row_idx < count; row_idx++) {

			const auto name = FlatVector::GetData<string_t>(name_vec)[row_idx].GetString();

			CreateSchemaInfo info;
			info.catalog = catalog.GetName();
			info.schema = name;
			info.comment = comment_vec.GetValue(row_idx);

			entries[name] = make_uniq<TeradataSchemaEntry>(catalog, info);
		}
	}
}

}