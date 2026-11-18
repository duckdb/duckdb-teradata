#include "teradata_transaction.hpp"
#include "teradata_catalog.hpp"

namespace duckdb {

TeradataTransaction::TeradataTransaction(TeradataCatalog &catalog, TransactionManager &manager, ClientContext &context)
    : Transaction(manager, context), con(catalog.GetConnection()) {

	// TODO:
	// transaction_state = TeradataTransactionState::TRANSACTION_NOT_YET_STARTED;
}

TeradataTransaction::~TeradataTransaction() {
}

TeradataTransaction &TeradataTransaction::Get(ClientContext &context, Catalog &catalog) {
	return Transaction::Get(context, catalog).Cast<TeradataTransaction>();
}

void TeradataTransaction::Start() {
	con.Execute("BEGIN TRANSACTION;");
}

void TeradataTransaction::Commit() {
	con.Execute("END TRANSACTION;");
}

void TeradataTransaction::Rollback() {
	try {
		con.Execute("ABORT;");
	} catch (IOException &ex) {
		// Teradata answers ABORT with failure 3514 ("User-generated transaction ABORT"): the rollback succeeded
		if (!StringUtil::Contains(ex.what(), "code: 3514,")) {
			throw;
		}
	}
}

} // namespace duckdb