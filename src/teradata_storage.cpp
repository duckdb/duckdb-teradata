#include "teradata_storage.hpp"
#include "teradata_catalog.hpp"
#include "teradata_transcation_manager.hpp"

#include "duckdb/main/secret/secret_manager.hpp"

namespace duckdb {

static unique_ptr<Catalog> TeradataAttach(StorageExtensionInfo *storage_info, ClientContext &context,
                                          AttachedDatabase &db, const string &name, AttachInfo &info,
                                          AccessMode access_mode) {

	// TODO: Parse options, secrets, etc
	// We could pass in a default_schema to load here

	// Default: "127.0.0.1/dbc,dbc";
	string connection_string = info.path;

	// Split the connection string into host and user, to get the user database
	string host = connection_string.substr(0, connection_string.find('/'));
	string user_db = connection_string.substr(connection_string.find('/') + 1);
	string user = user_db.substr(0, user_db.find(','));

	// If no option is set, connect to the user database
	string database = user;

	// Try to get the database name from the options
	for(auto &option : info.options) {
		auto lower_name = StringUtil::Lower(option.first);
		if (lower_name == "database") {
			database = option.second.ToString();
		}
	}

	return make_uniq_base<Catalog, TeradataCatalog>(db, connection_string, database);
}

static unique_ptr<TransactionManager> TeradataCreateTransactionManager(StorageExtensionInfo *storage_info,
                                                                       AttachedDatabase &db, Catalog &catalog) {
	auto &td_catalog = catalog.Cast<TeradataCatalog>();
	return make_uniq<TeradataTransactionManager>(db, td_catalog);
}

TeradataStorageExtension::TeradataStorageExtension() {
	attach = TeradataAttach;
	create_transaction_manager = TeradataCreateTransactionManager;
}

} // namespace duckdb