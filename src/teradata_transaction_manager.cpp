#include "teradata_transcation_manager.hpp"

namespace duckdb {

TeradataTransactionManager::TeradataTransactionManager(AttachedDatabase &db, TeradataCatalog &catalog)
    : TransactionManager(db), teradata_catalog(catalog) {
}

// TODO: Everything below this point is a stub
Transaction &TeradataTransactionManager::StartTransaction(ClientContext &context) {
	auto transaction = make_uniq<TeradataTransaction>(teradata_catalog, *this, context);

	transaction->Start();

	auto &result = *transaction;
	lock_guard<mutex> l(transaction_lock);
	transactions[result] = std::move(transaction);
	return result;
}

ErrorData TeradataTransactionManager::CommitTransaction(ClientContext &context, Transaction &transaction) {
	auto &td_transaction = transaction.Cast<TeradataTransaction>();

	td_transaction.Commit();

	lock_guard<mutex> l(transaction_lock);
	transactions.erase(transaction);

	return ErrorData();
}

void TeradataTransactionManager::RollbackTransaction(Transaction &transaction) {
	auto &td_transaction = transaction.Cast<TeradataTransaction>();

	td_transaction.Rollback();

	lock_guard<mutex> l(transaction_lock);
	transactions.erase(transaction);
}

void TeradataTransactionManager::Checkpoint(ClientContext &context, bool force) {
	// Not applicable?
}

} // namespace duckdb