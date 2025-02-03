#include "teradata_storage.hpp"
#include "teradata_catalog.hpp"
#include "teradata_transcation_manager.hpp"

namespace duckdb {

static unique_ptr<Catalog> TeradataAttach(StorageExtensionInfo *storage_info, ClientContext &context,
                                          AttachedDatabase &db, const string &name, AttachInfo &info,
                                          AccessMode access_mode) {

	// TODO: Parse options, secrets, etc
	// We could pass in a default_schema to load here
	
	// Default: "127.0.0.1/dbc,dbc";
	string connection_string = info.path;

	return make_uniq_base<Catalog, TeradataCatalog>(db, connection_string);
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