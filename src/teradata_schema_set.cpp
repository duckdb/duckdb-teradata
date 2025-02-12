#include "teradata_schema_set.hpp"

#include "teradata_request.hpp"
#include "teradata_catalog.hpp"

namespace duckdb {

void TeradataSchemaSet::LoadEntries(ClientContext &context) {
	const auto &td_catalog = catalog.Cast<TeradataCatalog>();

	// Now we gotta issue some queries to get all the schemas
	const auto &conn = td_catalog.GetConnection();

	TeradataSqlRequest request(conn.GetSessionId(), "");
}

}