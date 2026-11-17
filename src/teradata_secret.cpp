#include "duckdb.hpp"
#include "duckdb/main/secret/secret.hpp"
#include "duckdb/main/secret/secret_manager.hpp"
#include "duckdb/main/extension/extension_loader.hpp"

#include "teradata_secret.hpp"

#include <algorithm>

namespace duckdb {

const char *TeradataSecret::TYPE = "teradata";

static const char *const SECRET_KEYS[] = {"host", "user", "database", "password"};

case_insensitive_map_t<string> TeradataSecret::ParseOptions(const case_insensitive_map_t<Value> &options) {
	const auto is_secret_key = [](const string &name) {
		return std::any_of(std::begin(SECRET_KEYS), std::end(SECRET_KEYS),
		                   [&](const char *key) { return StringUtil::CIEquals(name, key); });
	};
	case_insensitive_map_t<string> result;
	for (const auto &option : options) {
		if (!is_secret_key(option.first)) {
			throw InternalException("Unknown named parameter passed to teradata secret: " +
			                        StringUtil::Lower(option.first));
		}
		result[StringUtil::Lower(option.first)] = option.second.ToString();
	}
	const auto is_present = [&](const char *key) {
		return result.count(key) > 0;
	};
	if (!std::all_of(std::begin(SECRET_KEYS), std::end(SECRET_KEYS), is_present)) {
		throw InvalidInputException(
		    "Teradata secret must contain 'HOST', 'USER', 'DATABASE', and 'PASSWORD' parameters");
	}
	return result;
}

static unique_ptr<BaseSecret> CreateTeradataSecretFunction(ClientContext &context, CreateSecretInput &input) {
	vector<string> prefix_paths;
	auto result = make_uniq<KeyValueSecret>(prefix_paths, TeradataSecret::TYPE, "config", input.name);
	for (auto &entry : TeradataSecret::ParseOptions(input.options)) {
		result->secret_map[Identifier(entry.first)] = Value(entry.second);
	}
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

void TeradataSecret::Register(ExtensionLoader &loader) {

	// Register the teradata secret type
	SecretType secret_type;
	secret_type.name = TeradataSecret::TYPE;
	secret_type.deserializer = KeyValueSecret::Deserialize<KeyValueSecret>;
	secret_type.default_provider = "config";

	loader.RegisterSecretType(secret_type);

	// Register the teradata secret creation function
	CreateSecretFunction secret_constructor;
	secret_constructor.secret_type = TeradataSecret::TYPE;
	secret_constructor.provider = "config";
	secret_constructor.function = CreateTeradataSecretFunction;
	secret_constructor.named_parameters["host"] = LogicalType::VARCHAR;
	secret_constructor.named_parameters["password"] = LogicalType::VARCHAR;
	secret_constructor.named_parameters["user"] = LogicalType::VARCHAR;
	secret_constructor.named_parameters["database"] = LogicalType::VARCHAR;

	loader.RegisterFunction(secret_constructor);
}

} // namespace duckdb
