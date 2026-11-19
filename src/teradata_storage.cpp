#include "teradata_storage.hpp"
#include "teradata_catalog.hpp"
#include "teradata_secret.hpp"
#include "teradata_logon.hpp"
#include "teradata_transcation_manager.hpp"

#include "duckdb/main/secret/secret_manager.hpp"
#include "duckdb/parser/parsed_data/attach_info.hpp"

namespace duckdb {

static optional<TeradataSecret> LookupSecret(ClientContext &context, const unordered_map<string, Value> &options) {
	const case_insensitive_map_t<Value> ci_options(options.begin(), options.end());
	const auto secret_name = ci_options.find("secret");
	if (secret_name == ci_options.end()) {
		return {};
	}
	TeradataSecret secret;
	if (!TeradataSecret::TryGet(context, secret_name->second.ToString(), secret)) {
		throw InvalidInputException("No Teradata secret found with name: " + secret_name->second.ToString());
	}
	return secret;
}

static unique_ptr<Catalog> TeradataAttach(optional_ptr<StorageExtensionInfo> storage_info, ClientContext &context,
                                          AttachedDatabase &db, const string &name, AttachInfo &info,
                                          AttachOptions &attach_options) {

	// Attempt to load the Teradata CLIV2 library
	TeradataCLIV2::Load(context);

	const auto logon = ResolveLogon(LookupSecret(context, info.options), info.options, info.path);

	// Create the catalog and connect to the teradata system
	auto result = make_uniq<TeradataCatalog>(db, logon.ConnectionString(), logon.database, logon.buffer_size);

	// Set the database path
	result->GetConnection().Execute("DATABASE " + KeywordHelper::WriteOptionallyQuoted(logon.database) + ";");

	return std::move(result);
}

static unique_ptr<TransactionManager> TeradataCreateTransactionManager(optional_ptr<StorageExtensionInfo> storage_info,
                                                                       AttachedDatabase &db, Catalog &catalog) {
	auto &td_catalog = catalog.Cast<TeradataCatalog>();
	return make_uniq<TeradataTransactionManager>(db, td_catalog);
}

TeradataStorageExtension::TeradataStorageExtension() {
	attach = TeradataAttach;
	create_transaction_manager = TeradataCreateTransactionManager;
}

} // namespace duckdb
