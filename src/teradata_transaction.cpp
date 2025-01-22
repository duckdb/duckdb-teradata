#include "teradata_transaction.hpp"

namespace duckdb {

TeradataTransaction::TeradataTransaction(TeradataCatalog &catalog, TransactionManager &manager, ClientContext &context)
    : Transaction(manager, context) {

	// TODO:
	transaction_state = TeradataTransactionState::TRANSACTION_NOT_YET_STARTED;
}

TeradataTransaction::~TeradataTransaction() {
}

} // namespace duckdb