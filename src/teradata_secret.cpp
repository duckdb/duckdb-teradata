#include "duckdb.hpp"
#include "duckdb/main/secret/secret.hpp"
#include "duckdb/main/secret/secret_manager.hpp"
#include "duckdb/main/extension_util.hpp"

#include "teradata_secret.hpp"

namespace duckdb {

static unique_ptr<BaseSecret> CreateTeradataSecretFunction(ClientContext &context, CreateSecretInput &input) {
	vector<string> prefix_paths;
	auto result = make_uniq<KeyValueSecret>(prefix_paths, TeradataSecret::TYPE, "config", input.name);
	for (const auto &named_param : input.options) {
		auto lower_name = StringUtil::Lower(named_param.first);
		if (lower_name == "host") {
			result->secret_map["host"] = named_param.second.ToString();
		} else if (lower_name == "user") {
			result->secret_map["user"] = named_param.second.ToString();
		} else if (lower_name == "database") {
			result->secret_map["database"] = named_param.second.ToString();
		} else if (lower_name == "password") {
			result->secret_map["password"] = named_param.second.ToString();
		} else {
			throw InternalException("Unknown named parameter passed to teradata secret: " + lower_name);
		}
	}

	// Set redact keys
	result->redact_keys = {"password"};
	return std::move(result);
}

bool TeradataSecret::TryGet(ClientContext &context, const string &name, TeradataSecret &out) {
	auto &secret_manager = SecretManager::Get(context);
	const auto transaction = CatalogTransaction::GetSystemCatalogTransaction(context);

	const auto secret_entry = secret_manager.GetSecretByName(transaction, name);
	if (!secret_entry) {
		return false;
	}
	if (secret_entry->secret->GetType() != TeradataSecret::TYPE) {
		return false;
	}
	const auto &kv_secret = dynamic_cast<const KeyValueSecret &>(*secret_entry->secret);

	out.host = kv_secret.TryGetValue("host", true).ToString();
	out.user = kv_secret.TryGetValue("user", true).ToString();
	out.database = kv_secret.TryGetValue("database", true).ToString();
	out.password = kv_secret.TryGetValue("password", true).ToString();

	return true;
}

void TeradataSecret::Register(DatabaseInstance &db) {

	// Register the teradata secret type
	SecretType secret_type;
	secret_type.name = TeradataSecret::TYPE;
	secret_type.deserializer = KeyValueSecret::Deserialize<KeyValueSecret>;
	secret_type.default_provider = "config";

	ExtensionUtil::RegisterSecretType(db, secret_type);

	// Register the teradata secret creation function
	CreateSecretFunction secret_constructor;
	secret_constructor.secret_type = TeradataSecret::TYPE;
	secret_constructor.provider = "config";
	secret_constructor.function = CreateTeradataSecretFunction;
	secret_constructor.named_parameters["host"] = LogicalType::VARCHAR;
	secret_constructor.named_parameters["password"] = LogicalType::VARCHAR;
	secret_constructor.named_parameters["user"] = LogicalType::VARCHAR;
	secret_constructor.named_parameters["database"] = LogicalType::VARCHAR;

	ExtensionUtil::RegisterFunction(db, secret_constructor);
}

} // namespace duckdb