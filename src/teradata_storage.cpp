#include "teradata_storage.hpp"
#include "teradata_catalog.hpp"
#include "teradata_secret.hpp"
#include "teradata_transcation_manager.hpp"

#include "duckdb/main/secret/secret_manager.hpp"

namespace duckdb {

static unique_ptr<Catalog> TeradataAttach(StorageExtensionInfo *storage_info, ClientContext &context,
                                          AttachedDatabase &db, const string &name, AttachInfo &info,
                                          AccessMode access_mode) {

	// Attempt to load the Teradata CLIV2 library
	TeradataCLIV2::Load();

	string host;
	string user;
	string database;
	string password;

	// First, check if we have a secret, if so use that for the default options
	case_insensitive_map_t<reference<Value>> options;
	for (auto &option : info.options) {
		options.emplace(option.first, option.second);
	}

	auto secret_name_opt = options.find("secret");
	if (secret_name_opt != options.end()) {
		auto secret_name = secret_name_opt->second.get().ToString();

		TeradataSecret secret;
		if (!TeradataSecret::TryGet(context, secret_name, secret)) {
			throw InvalidInputException("No Teradata secret found with name: " + secret_name);
		}

		host = secret.host;
		user = secret.user;
		database = secret.database;
		password = secret.password;
	}

	// Now, try to get options from the ATTACH info
	auto host_opt = options.find("host");
	if (host_opt != options.end()) {
		host = host_opt->second.get().ToString();
	}

	auto user_opt = options.find("user");
	if (user_opt != options.end()) {
		user = user_opt->second.get().ToString();
	}

	auto database_opt = options.find("database");
	if (database_opt != options.end()) {
		database = database_opt->second.get().ToString();
	}

	auto password_opt = options.find("password");
	if (password_opt != options.end()) {
		password = password_opt->second.get().ToString();
	}

	idx_t buffer_size = 1024 * 1024; // Default buffer size is 1MB
	auto buffer_size_opt = options.find("buffer_size");
	if (buffer_size_opt != options.end()) {
		auto &buffer_size_val = buffer_size_opt->second.get();
		if (buffer_size_val.type().id() != LogicalTypeId::INTEGER) {
		}
		auto buffer_size_int = buffer_size_val.GetValue<int32_t>();
		if (buffer_size_int <= 0) {
			throw InvalidInputException("Teradata ATTACH option 'buffer_size' must be a positive integer");
		}
		buffer_size = UnsafeNumericCast<idx_t>(buffer_size_int);
	}

	// Lastly, parse parameters from the logon string
	if (!info.path.empty()) {

		string host_str;
		string user_str;
		string pass_str;

		auto beg = info.path.c_str();
		auto end = beg + info.path.size();
		auto ptr = beg;

		// Parse the logon string format: "host/user[,password][,account]"
		// although, 'account' is unused by us for now.
		while (ptr != end && *ptr != '/') {
			host_str += *ptr;
			ptr++;
		}
		if (ptr != end && *ptr == '/') {
			ptr++; // Skip the '/'

			while (ptr != end && *ptr != ',') {
				user_str += *ptr;
				ptr++;
			}
			if (ptr != end && *ptr == ',') {
				ptr++; // Skip the ','
				while (ptr != end && *ptr != ',') {
					pass_str += *ptr;
					ptr++;
				}
			}
		}

		if (!host_str.empty()) {
			host = host_str;
		}
		if (!user_str.empty()) {
			user = user_str;
		}
		if (!pass_str.empty()) {
			password = pass_str;
		}
	}

	if (user.empty()) {
		throw InvalidInputException("Teradata ATTACH must contain a 'user', either in the logon string, passed as "
		                            "options or in defined in a secret");
	}
	if (host.empty()) {
		throw InvalidInputException("Teradata ATTACH must contain a 'host', either in the logon string, passed as "
		                            "options or in defined in a secret");
	}
	if (database.empty()) {
		database = user; // Default to user database if not specified
	}

	// Construct the connection string
	auto connection_string = host + "/" + user;
	if (!password.empty()) {
		connection_string += "," + password;
	}

	// Create the catalog and connect to the teradata system
	auto result = make_uniq<TeradataCatalog>(db, connection_string, database, buffer_size);

	// Set the database path
	result->GetConnection().Execute("DATABASE " + KeywordHelper::WriteOptionallyQuoted(database) + ";");

	return std::move(result);
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