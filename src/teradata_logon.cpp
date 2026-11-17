#include "teradata_logon.hpp"

namespace duckdb {

namespace {

//! Split at the first separator: the part before it, and the rest if the separator was found
pair<string, optional<string>> SplitOnce(const string &str, char separator) {
	const auto pos = str.find(separator);
	if (pos == string::npos) {
		return {str, {}};
	}
	return {str.substr(0, pos), str.substr(pos + 1)};
}

string NonEmptyOr(const string &value, const string &fallback) {
	return value.empty() ? fallback : value;
}

TeradataLogon FromSecret(const optional<TeradataSecret> &secret) {
	TeradataLogon result;
	if (secret) {
		result.host = secret->host;
		result.user = secret->user;
		result.password = secret->password;
		result.database = secret->database;
	}
	return result;
}

TeradataLogon WithOptions(TeradataLogon logon, const case_insensitive_map_t<Value> &options) {
	const auto option_or = [&](const char *name, const string &fallback) {
		const auto entry = options.find(name);
		return entry == options.end() ? fallback : entry->second.ToString();
	};
	logon.host = option_or("host", logon.host);
	logon.user = option_or("user", logon.user);
	logon.password = option_or("password", logon.password);
	logon.database = option_or("database", logon.database);
	const auto buffer_size = options.find("buffer_size");
	logon.buffer_size =
	    ParseBufferSize(buffer_size == options.end() ? optional<Value>() : optional<Value>(buffer_size->second));
	return logon;
}

TeradataLogon WithLogonString(TeradataLogon logon, const TeradataLogonString &parts) {
	logon.host = NonEmptyOr(parts.host, logon.host);
	logon.user = NonEmptyOr(parts.user, logon.user);
	logon.password = NonEmptyOr(parts.password, logon.password);
	return logon;
}

TeradataLogon Validated(TeradataLogon logon) {
	if (logon.user.empty()) {
		throw InvalidInputException("Teradata ATTACH must contain a 'user', either in the logon string, passed as "
		                            "options or in defined in a secret");
	}
	if (logon.host.empty()) {
		throw InvalidInputException("Teradata ATTACH must contain a 'host', either in the logon string, passed as "
		                            "options or in defined in a secret");
	}
	// Default to the user database if not specified
	logon.database = NonEmptyOr(logon.database, logon.user);
	return logon;
}

} // namespace

string TeradataLogon::ConnectionString() const {
	return host + "/" + user + (password.empty() ? "" : "," + password);
}

TeradataLogonString ParseLogonString(const string &logon_string) {
	// 'account' is unused for now
	const auto host_rest = SplitOnce(logon_string, '/');
	if (!host_rest.second) {
		return {host_rest.first, "", ""};
	}
	const auto user_rest = SplitOnce(*host_rest.second, ',');
	const auto password = user_rest.second ? SplitOnce(*user_rest.second, ',').first : "";
	return {host_rest.first, user_rest.first, password};
}

idx_t ParseBufferSize(const optional<Value> &option) {
	if (!option) {
		return TeradataLogon::DEFAULT_BUFFER_SIZE;
	}
	const auto buffer_size = option->GetValue<int32_t>();
	if (buffer_size <= 0) {
		throw InvalidInputException("Teradata ATTACH option 'buffer_size' must be a positive integer");
	}
	return UnsafeNumericCast<idx_t>(buffer_size);
}

TeradataLogon ResolveLogon(const optional<TeradataSecret> &secret, const unordered_map<string, Value> &options,
                           const string &logon_string) {
	const case_insensitive_map_t<Value> ci_options(options.begin(), options.end());
	return Validated(WithLogonString(WithOptions(FromSecret(secret), ci_options), ParseLogonString(logon_string)));
}

} // namespace duckdb
