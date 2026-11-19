#pragma once

#include "duckdb.hpp"
#include "teradata_secret.hpp"

namespace duckdb {

//! The parts of a Teradata logon string: "host/user[,password][,account]"
struct TeradataLogonString {
	string host;
	string user;
	string password;
};

//! Everything needed to connect to a Teradata system
struct TeradataLogon {
	static constexpr idx_t DEFAULT_BUFFER_SIZE = 1024 * 1024;

	string host;
	string user;
	string password;
	string database;
	idx_t buffer_size = DEFAULT_BUFFER_SIZE;

	//! The CLIv2 logon string: "host/user[,password]"
	string ConnectionString() const;
};

TeradataLogonString ParseLogonString(const string &logon_string);

//! The 'buffer_size' ATTACH option, or the default if it is absent. Throws unless positive.
idx_t ParseBufferSize(const optional<Value> &option);

//! Combine the logon sources, later ones taking precedence: the secret, the ATTACH options, then the logon string.
//! Throws if no host or user is given; the database defaults to the user.
TeradataLogon ResolveLogon(const optional<TeradataSecret> &secret, const unordered_map<string, Value> &options,
                           const string &logon_string);

} // namespace duckdb
