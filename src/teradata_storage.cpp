#include "teradata_storage.hpp"
#include "teradata_catalog.hpp"
#include "teradata_transcation_manager.hpp"

namespace duckdb {

static unique_ptr<Catalog> TeradataAttach(StorageExtensionInfo *storage_info, ClientContext &context,
                                          AttachedDatabase &db, const string &name, AttachInfo &info,
                                          AccessMode access_mode) {

	// TODO: Parse options, secrets, etc
	return make_uniq_base<Catalog, TeradataCatalog>(db);
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