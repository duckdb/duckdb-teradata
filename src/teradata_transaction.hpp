//===----------------------------------------------------------------------===//
//                         DuckDB
//
// storage/postgres_transaction.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/transaction/transaction.hpp"

namespace duckdb {

class TeradataCatalog;
class TeradataConnection;

enum class TeradataTransactionState { TRANSACTION_NOT_YET_STARTED, TRANSACTION_STARTED, TRANSACTION_FINISHED };

class TeradataTransaction final : public Transaction {
public:
	TeradataTransaction(TeradataCatalog &catalog, TransactionManager &manager, ClientContext &context);
	~TeradataTransaction() override;

	TeradataConnection &GetConnection() const { return con; }

	// TODO:
	/*
	    void Start();
	    void Commit();
	    void Rollback();

	    PostgresConnection &GetConnection();
	    string GetDSN();
	    unique_ptr<PostgresResult> Query(const string &query);
	    vector<unique_ptr<PostgresResult>> ExecuteQueries(const string &queries);
	    static PostgresTransaction &Get(ClientContext &context, Catalog &catalog);

	    string GetTemporarySchema();
	 */

private:
	TeradataTransactionState transaction_state;

	// TODO: This shouldnt be passed like this
	TeradataConnection &con;

	/*
	    PostgresPoolConnection connection;
	    AccessMode access_mode;
	    string temporary_schema;
	 */

private:
};

} // namespace duckdb
