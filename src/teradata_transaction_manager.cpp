#include "teradata_transcation_manager.hpp"

namespace duckdb {

TeradataTransactionManager::TeradataTransactionManager(AttachedDatabase &db, TeradataCatalog &catalog)
    : TransactionManager(db), teradata_catalog(catalog) {
}

// TODO: Everything below this point is a stub
Transaction &TeradataTransactionManager::StartTransaction(ClientContext &context) {
	auto transaction = make_uniq<TeradataTransaction>(teradata_catalog, *this, context);

	auto &result = *transaction;
	lock_guard<mutex> l(transaction_lock);
	transactions[result] = std::move(transaction);
	return result;
}

ErrorData TeradataTransactionManager::CommitTransaction(ClientContext &context, Transaction &transaction) {
	return ErrorData();
}

void TeradataTransactionManager::RollbackTransaction(Transaction &transaction) {
	transactions.erase(transaction);
}

void TeradataTransactionManager::Checkpoint(ClientContext &context, bool force) {
}

} // namespace duckdb