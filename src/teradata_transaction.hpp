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

class TeradataTransaction final : public Transaction {
public:
	TeradataTransaction(TeradataCatalog &catalog, TransactionManager &manager, ClientContext &context);
	~TeradataTransaction() override;

	TeradataConnection &GetConnection() const {
		return con;
	}

	void Start();
	void Commit();
	void Rollback();

	static TeradataTransaction &Get(ClientContext &context, Catalog &catalog);

private:
	// TODO: This shouldnt be passed like this
	TeradataConnection &con;
};

} // namespace duckdb
