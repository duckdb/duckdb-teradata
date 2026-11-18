#include "catch.hpp"

#include "fake_cliv2.hpp"
#include "teradata_column_reader.hpp"
#include "teradata_extension.hpp"
#include "teradata_type.hpp"
#include "util/binary_reader.hpp"

#include <algorithm>
#include <chrono>
#include <iterator>
#include <thread>

namespace duckdb {

namespace {

// Parcel type codes of nullable columns
constexpr int16_t NULLABLE_VARCHAR = 449;
constexpr int16_t NULLABLE_CHAR = 453;
constexpr int16_t NULLABLE_INTEGER = 497;

struct TeradataFixture {
	TeradataFixture() : db(nullptr), con(db) {
		FakeCLIv2::Get().Install();
		db.LoadStaticExtension<TeradataExtension>();
	}

	//! A materialized query result
	struct Rows {
		vector<string> names;
		vector<vector<Value>> rows;

		idx_t RowCount() const {
			return rows.size();
		}
		Value GetValue(idx_t col, idx_t row) const {
			return rows[row][col];
		}
	};

	Rows Run(const string &sql) {
		auto result = con.Query(sql);
		if (result->HasError()) {
			FAIL(sql << "\n" << result->GetError());
		}
		Rows rows;
		for (auto &name : result->GetNames()) {
			rows.names.push_back(name.GetIdentifierName());
		}
		while (auto chunk = result->Fetch()) {
			if (chunk->size() == 0) {
				break;
			}
			for (idx_t row = 0; row < chunk->size(); row++) {
				vector<Value> values;
				for (idx_t col = 0; col < chunk->ColumnCount(); col++) {
					values.push_back(chunk->GetValue(col, row));
				}
				rows.rows.push_back(std::move(values));
			}
		}
		return rows;
	}

	string Error(const string &sql) {
		auto result = con.Query(sql);
		REQUIRE(result->HasError());
		return result->GetError();
	}

	void Attach() {
		Run("ATTACH 'host/user,pass' AS td (TYPE teradata)");
	}

	//! Serve a catalog with schema "user" holding table t(id INTEGER, name VARCHAR(20))
	void ServeCatalog() {
		auto &fake = FakeCLIv2::Get();
		fake.Respond(
		    "DBC.DatabasesV", 'E',
		    {Parcels::Statement({Parcels::DataInfo({{NULLABLE_VARCHAR, 128, ""}, {NULLABLE_VARCHAR, 255, ""}}),
			                     Parcels::Record({false, true}, Parcels::Varchar("user") + Parcels::Varchar(""))})});
		fake.Respond("dbc.TablesV", 'E',
		             {Parcels::Statement({Parcels::DataInfo({{NULLABLE_VARCHAR, 128, ""},
		                                                     {NULLABLE_VARCHAR, 128, ""},
		                                                     {NULLABLE_CHAR, 2, ""},
		                                                     {NULLABLE_INTEGER, 4, ""}}),
		                                  ColumnRow("t", "id", "I ", 4), ColumnRow("t", "name", "CV", 20)})});
	}

	static FakeParcel ColumnRow(const string &table, const string &column, const string &type, int32_t length) {
		return Parcels::Record({false, false, false, false}, Parcels::Varchar(table) + Parcels::Varchar(column) +
		                                                         Parcels::Char(type) + Parcels::Int32(length));
	}

	//! The requests the extension sent, by SQL
	vector<string> RequestSQL() {
		vector<string> result;
		for (auto &request : FakeCLIv2::Get().Requests()) {
			result.push_back(request.sql);
		}
		return result;
	}

	DuckDB db;
	Connection con;
};

bool Contains(const vector<string> &values, const string &value) {
	return std::find(values.begin(), values.end(), value) != values.end();
}

} // namespace

//----------------------------------------------------------------------------------------------------------------------
// Secrets and ATTACH
//----------------------------------------------------------------------------------------------------------------------
TEST_CASE_METHOD(TeradataFixture, "Teradata secrets need all four parameters", "[integration]") {
	Run("CREATE SECRET s (TYPE teradata, HOST 'h', USER 'u', DATABASE 'd', PASSWORD 'p')");
	auto result = Run("SELECT secret_string FROM duckdb_secrets() WHERE name = 's'");
	CHECK(StringUtil::Contains(result.GetValue(0, 0).ToString(), "password=redacted"));

	CHECK(StringUtil::Contains(Error("CREATE SECRET s2 (TYPE teradata, HOST 'h', USER 'u')"),
	                           "must contain 'HOST', 'USER', 'DATABASE', and 'PASSWORD'"));
}

TEST_CASE_METHOD(TeradataFixture, "ATTACH logs on and selects the database", "[integration]") {
	Run("CREATE SECRET s (TYPE teradata, HOST 'h', USER 'secret_user', DATABASE 'sales', PASSWORD 'p')");
	Run("ATTACH '' AS td (TYPE teradata, SECRET s)");

	CHECK(FakeCLIv2::Get().Calls().front() == "CON");
	CHECK(Contains(RequestSQL(), "DATABASE sales;"));
}

TEST_CASE_METHOD(TeradataFixture, "ATTACH reports missing logon details", "[integration]") {
	CHECK(StringUtil::Contains(Error("ATTACH 'host' AS td (TYPE teradata)"), "must contain a 'user'"));
	CHECK(StringUtil::Contains(Error("ATTACH '' AS td (TYPE teradata, SECRET nope)"), "No Teradata secret found"));
}

//----------------------------------------------------------------------------------------------------------------------
// teradata_execute and teradata_query
//----------------------------------------------------------------------------------------------------------------------
TEST_CASE_METHOD(TeradataFixture, "teradata_execute runs a statement in a transaction", "[integration]") {
	Attach();
	Run("SELECT * FROM teradata_execute('td', 'DELETE FROM x')");

	const auto sql = RequestSQL();
	const auto stmt = std::find(sql.begin(), sql.end(), "DELETE FROM x");
	REQUIRE(stmt != sql.end());
	REQUIRE(stmt != sql.begin());
	REQUIRE(stmt + 1 != sql.end());
	CHECK(*(stmt - 1) == "BEGIN TRANSACTION;");
	CHECK(*(stmt + 1) == "END TRANSACTION;");
}

TEST_CASE_METHOD(TeradataFixture, "teradata_execute and teradata_query check their arguments", "[integration]") {
	Attach();
	CHECK(StringUtil::Contains(Error("SELECT * FROM teradata_execute('nope', 'SELECT 1')"),
	                           "Failed to find attached database \"nope\" referenced in teradata_execute"));
	CHECK(StringUtil::Contains(Error("SELECT * FROM teradata_query('memory', 'SELECT 1')"),
	                           "does not refer to a Teradata database"));
	CHECK(StringUtil::Contains(Error("SELECT * FROM teradata_query(NULL, 'SELECT 1')"), "cannot be NULL"));
}

TEST_CASE_METHOD(TeradataFixture, "teradata_query prepares, then reads the records", "[integration]") {
	Attach();
	auto &fake = FakeCLIv2::Get();
	const vector<FakeColumn> columns {{NULLABLE_INTEGER, 4, "id"}, {NULLABLE_VARCHAR, 20, "name"}};
	fake.Respond("FROM people", 'P', {Parcels::Statement({Parcels::PrepInfo(columns)})});
	fake.Respond("FROM people", 'E',
	             {Parcels::Statement({Parcels::DataInfo(columns),
	                                  Parcels::Record({false, false}, Parcels::Int32(1) + Parcels::Varchar("ann")),
	                                  Parcels::Record({false, true}, Parcels::Int32(2) + Parcels::Varchar(""))})});

	auto result = Run("SELECT id, name FROM teradata_query('td', 'SELECT id, name FROM people;') ORDER BY id");
	REQUIRE(result.RowCount() == 2);
	CHECK(result.names == vector<string> {"id", "name"});
	CHECK(result.GetValue(0, 0) == Value::INTEGER(1));
	CHECK(result.GetValue(1, 0) == Value("ann"));
	CHECK(result.GetValue(1, 1).IsNull());

	// The trailing semicolon is not sent
	CHECK(Contains(RequestSQL(), "SELECT id, name FROM people"));
}

TEST_CASE_METHOD(TeradataFixture, "Records larger than the parcel buffer are fetched whole", "[integration]") {
	Attach();
	const string big(20000, 'x');
	const vector<FakeColumn> columns {{NULLABLE_VARCHAR, 30000, "doc"}};
	auto &fake = FakeCLIv2::Get();
	fake.Respond("FROM docs", 'P', {Parcels::Statement({Parcels::PrepInfo(columns)})});
	fake.Respond("FROM docs", 'E',
	             {Parcels::Statement({Parcels::DataInfo(columns), Parcels::Record({false}, Parcels::Varchar(big))})});

	auto result = Run("SELECT doc FROM teradata_query('td', 'SELECT doc FROM docs')");
	CHECK(result.GetValue(0, 0) == Value(big));
}

TEST_CASE_METHOD(TeradataFixture, "Teradata errors are reported with their code", "[integration]") {
	Attach();
	auto &fake = FakeCLIv2::Get();
	fake.Respond("bad sql", 'E', {{{PclSUCCESS, ""}, Parcels::Failure(3706, "Syntax error")}});
	fake.Respond("dup key", 'E', {{{PclSUCCESS, ""}, Parcels::Failure(2801, "Duplicate unique prime key")}});

	const auto syntax = Error("SELECT * FROM teradata_execute('td', 'bad sql')");
	CHECK(StringUtil::Contains(syntax, "code: 3706"));
	CHECK(StringUtil::Contains(syntax, "Syntax error"));
	CHECK(StringUtil::Contains(Error("SELECT * FROM teradata_execute('td', 'dup key')"), "Constraint Error"));

	// The session is still usable
	Run("SELECT * FROM teradata_execute('td', 'SELECT 1')");
}

//----------------------------------------------------------------------------------------------------------------------
// Catalog, scans and INSERT
//----------------------------------------------------------------------------------------------------------------------
TEST_CASE_METHOD(TeradataFixture, "Tables are loaded from the data dictionary and scanned", "[integration]") {
	Attach();
	ServeCatalog();
	const vector<FakeColumn> columns {{NULLABLE_INTEGER, 4, ""}, {NULLABLE_VARCHAR, 20, ""}};
	FakeCLIv2::Get().Respond(
	    "SELECT * FROM user.t", 'E',
	    {Parcels::Statement({Parcels::DataInfo(columns),
		                     Parcels::Record({false, false}, Parcels::Int32(7) + Parcels::Varchar("seven"))})});

	auto columns_result = Run("SELECT column_name, data_type FROM duckdb_columns() WHERE database_name = 'td' "
	                          "ORDER BY column_index");
	REQUIRE(columns_result.RowCount() == 2);
	CHECK(columns_result.GetValue(0, 0) == Value("id"));
	CHECK(columns_result.GetValue(1, 1) == Value("VARCHAR"));

	auto rows = Run("SELECT id, name FROM td.user.t");
	REQUIRE(rows.RowCount() == 1);
	CHECK(rows.GetValue(0, 0) == Value::INTEGER(7));
	CHECK(rows.GetValue(1, 0) == Value("seven"));
}

TEST_CASE_METHOD(TeradataFixture, "Filters are pushed into the scan SQL", "[integration]") {
	Attach();
	ServeCatalog();
	FakeCLIv2::Get().Respond(
	    "SELECT * FROM user.t", 'E',
	    {Parcels::Statement({Parcels::DataInfo({{NULLABLE_INTEGER, 4, ""}, {NULLABLE_VARCHAR, 20, ""}})})});

	Run("SELECT * FROM td.user.t WHERE id = 42");
	const auto sql = RequestSQL();
	const auto scan = std::find_if(sql.begin(), sql.end(),
	                               [](const string &s) { return StringUtil::StartsWith(s, "SELECT * FROM user.t"); });
	REQUIRE(scan != sql.end());
	CHECK(*scan == "SELECT * FROM user.t WHERE \"id\" = '42'");
}

TEST_CASE_METHOD(TeradataFixture, "A dropped table is gone under any spelling, and can be recreated", "[integration]") {
	Attach();
	ServeCatalog();
	REQUIRE(Run("SELECT * FROM td.user.T").names.size() == 2);

	// Dropped under a different spelling than the one it is cached under
	Run("DROP TABLE td.user.T");
	CHECK(Contains(RequestSQL(), "DROP Table \"user\".\"T\""));
	CHECK(StringUtil::Contains(Error("SELECT * FROM td.user.t"), "does not exist"));
	CHECK(StringUtil::Contains(Error("SELECT * FROM td.user.T"), "does not exist"));

	// Recreated with yet another spelling, it is found case-insensitively
	Run("CREATE TABLE td.user.SalesLog (x INTEGER)");
	FakeCLIv2::Get().Respond("SELECT * FROM user.SalesLog", 'E',
	                         {Parcels::Statement({Parcels::DataInfo({{NULLABLE_INTEGER, 4, ""}})})});
	CHECK(Run("SELECT * FROM td.user.saleslog").RowCount() == 0);
}

TEST_CASE_METHOD(TeradataFixture, "Indexes loaded from the data dictionary list their column", "[integration]") {
	Attach();
	ServeCatalog();
	FakeCLIv2::Get().Respond(
	    "DBC.IndicesV", 'E',
	    {Parcels::Statement({Parcels::DataInfo({{NULLABLE_VARCHAR, 128, ""},
		                                        {NULLABLE_VARCHAR, 128, ""},
		                                        {NULLABLE_CHAR, 1, ""},
		                                        {NULLABLE_VARCHAR, 128, ""},
		                                        {NULLABLE_CHAR, 1, ""}}),
		                     Parcels::Record({false, false, false, false, false},
		                                     Parcels::Varchar("t") + Parcels::Varchar("t_id") + Parcels::Char("P") +
		                                         Parcels::Varchar("id") + Parcels::Char("Y"))})});

	auto indexes = Run("SELECT index_name, sql FROM duckdb_indexes() WHERE database_name = 'td'");
	REQUIRE(indexes.RowCount() == 1);
	CHECK(indexes.GetValue(0, 0) == Value("t_id"));
	CHECK(StringUtil::Contains(indexes.GetValue(1, 0).ToString(), "(id)"));
}

TEST_CASE_METHOD(TeradataFixture, "Tables created without a schema go to the attached database", "[integration]") {
	Run("ATTACH 'host/user,pass' AS td (TYPE teradata, DATABASE 'sales')");
	FakeCLIv2::Get().Respond(
	    "DBC.DatabasesV", 'E',
	    {Parcels::Statement({Parcels::DataInfo({{NULLABLE_VARCHAR, 128, ""}, {NULLABLE_VARCHAR, 255, ""}}),
		                     Parcels::Record({false, true}, Parcels::Varchar("sales") + Parcels::Varchar(""))})});
	Run("CREATE TABLE td.events (id INTEGER)");
	CHECK(Contains(RequestSQL(), "CREATE TABLE \"sales\".\"events\"(\"id\" INTEGER);"));
}

TEST_CASE_METHOD(TeradataFixture, "INSERT sends one parameter record per row", "[integration]") {
	Attach();
	ServeCatalog();
	Run("INSERT INTO td.user.t VALUES (1, 'a'), (2, NULL)");

	const auto requests = FakeCLIv2::Get().Requests();
	const auto insert = std::find_if(requests.begin(), requests.end(),
	                                 [](const FakeRequest &r) { return StringUtil::Contains(r.sql, "INSERT INTO"); });
	REQUIRE(insert != requests.end());
	CHECK(insert->sql ==
	      "USING (\"id\" INTEGER, \"name\" VARCHAR(64000)) INSERT INTO \"user\".\"t\" VALUES (:\"id\", :\"name\");");
	REQUIRE(insert->records.size() == 2);

	// Decode the records the way Teradata would read them back
	const auto decode = [](const string &record) {
		const auto null_bits = static_cast<uint8_t>(record[0]);
		BinaryReader reader(record.data() + 1, record.size() - 1);
		Vector id(LogicalType::INTEGER, 1);
		Vector name(LogicalType::VARCHAR, 1);
		TeradataColumnReader::Make(TeradataTypeId::INTEGER)->Decode(reader, id, 0, (null_bits & 0x80) != 0);
		TeradataColumnReader::Make(TeradataTypeId::VARCHAR)->Decode(reader, name, 0, (null_bits & 0x40) != 0);
		return make_pair(id.GetValue(0), name.GetValue(0));
	};
	const auto first = decode(insert->records[0]);
	CHECK(first.first == Value::INTEGER(1));
	CHECK(first.second == Value("a"));
	const auto second = decode(insert->records[1]);
	CHECK(second.first == Value::INTEGER(2));
	CHECK(second.second.IsNull());
}

//----------------------------------------------------------------------------------------------------------------------
// Cancellation
//----------------------------------------------------------------------------------------------------------------------
namespace {

// Teradata takes this long to answer SLOW: far longer than the cancellations below wait, but finite,
// so a broken cancellation fails the test instead of hanging it
FakeResponse SlowResponse() {
	return {Parcels::Statement(), std::chrono::seconds(10)};
}

} // namespace

TEST_CASE_METHOD(TeradataFixture, "A timed out request is aborted and the session stays usable", "[integration]") {
	Attach();
	FakeCLIv2::Get().Respond("SLOW", 'E', SlowResponse());

	Run("SET max_execution_time = 200");
	const auto start = std::chrono::steady_clock::now();
	CHECK(StringUtil::Contains(Error("SELECT * FROM teradata_execute('td', 'SELECT SLOW')"), "maximum execution time"));
	CHECK(std::chrono::steady_clock::now() - start < std::chrono::seconds(5));

	const auto requests = FakeCLIv2::Get().Requests();
	const auto slow =
	    std::find_if(requests.begin(), requests.end(), [](const FakeRequest &r) { return r.sql == "SELECT SLOW"; });
	REQUIRE(slow != requests.end());
	CHECK(slow->non_blocking);

	// The request was aborted, ended, and the transaction rolled back
	const auto calls = FakeCLIv2::Get().Calls();
	const auto abort = std::find(calls.begin(), calls.end(), "ABT");
	REQUIRE(abort != calls.end());
	REQUIRE(std::distance(abort, calls.end()) >= 3);
	CHECK(*(abort + 1) == "ERQ");
	CHECK(*(abort + 2) == "IRQ ABORT;");

	Run("SET max_execution_time = 0");
	Run("SELECT * FROM teradata_execute('td', 'SELECT 2')");
	CHECK(RequestSQL().back() == "END TRANSACTION;");
}

TEST_CASE_METHOD(TeradataFixture, "An interrupted request is aborted", "[integration]") {
	Attach();
	FakeCLIv2::Get().Respond("SLOW", 'E', SlowResponse());

	// Interrupt only once the request is in flight: an interrupt before the query starts would be cleared
	bool saw_request = false;
	std::thread interrupter([&]() {
		saw_request = FakeCLIv2::Get().WaitForRequest("SELECT SLOW", std::chrono::seconds(5));
		if (saw_request) {
			con.Interrupt();
		}
	});
	auto result = con.Query("SELECT * FROM teradata_execute('td', 'SELECT SLOW')");
	interrupter.join();

	REQUIRE(saw_request);
	REQUIRE(result->HasError());
	CHECK(StringUtil::Contains(result->GetError(), "Interrupted"));
	CHECK(Contains(FakeCLIv2::Get().Calls(), "ABT"));
}

TEST_CASE_METHOD(TeradataFixture, "Transaction statements are never interrupted", "[integration]") {
	Attach();
	Run("SELECT * FROM teradata_execute('td', 'SELECT 1')");

	vector<FakeRequest> transaction_requests;
	for (auto &request : FakeCLIv2::Get().Requests()) {
		if (StringUtil::Contains(request.sql, "TRANSACTION")) {
			transaction_requests.push_back(request);
		}
	}
	const auto sql = RequestSQL();
	REQUIRE(Contains(sql, "BEGIN TRANSACTION;"));
	REQUIRE(Contains(sql, "END TRANSACTION;"));
	for (auto &request : transaction_requests) {
		INFO(request.sql);
		CHECK_FALSE(request.non_blocking);
	}
}

} // namespace duckdb
