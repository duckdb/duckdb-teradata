#pragma once

#include "duckdb/transaction/transaction_manager.hpp"

#include "teradata_catalog.hpp"
#include "teradata_transaction.hpp"

namespace duckdb {

class TeradataTransactionManager final : public TransactionManager {
public:
	explicit TeradataTransactionManager(AttachedDatabase &db, TeradataCatalog &catalog);

	Transaction &StartTransaction(ClientContext &context) override;
	ErrorData CommitTransaction(ClientContext &context, Transaction &transaction) override;
	void RollbackTransaction(Transaction &transaction) override;

	void Checkpoint(ClientContext &context, bool force) override;

private:
	TeradataCatalog &teradata_catalog;
	mutex transaction_lock;
	reference_map_t<Transaction, unique_ptr<TeradataTransaction>> transactions;
};

} // namespace duckdb