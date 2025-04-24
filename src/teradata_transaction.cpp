#include "teradata_transaction.hpp"
#include "teradata_catalog.hpp"

namespace duckdb {

TeradataTransaction::TeradataTransaction(TeradataCatalog &catalog, TransactionManager &manager, ClientContext &context)
    : Transaction(manager, context), con(catalog.GetConnection()) {

	// TODO:
	transaction_state = TeradataTransactionState::TRANSACTION_NOT_YET_STARTED;
}

TeradataTransaction::~TeradataTransaction() {
}

TeradataTransaction &TeradataTransaction::Get(ClientContext &context, Catalog &catalog) {
	return Transaction::Get(context, catalog).Cast<TeradataTransaction>();
}

} // namespace duckdb