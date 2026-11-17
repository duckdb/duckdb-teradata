#include "catch.hpp"

#include "teradata_insert.hpp"
#include "teradata_logon.hpp"
#include "teradata_query.hpp"
#include "teradata_secret.hpp"
#include "teradata_table_set.hpp"

#include "duckdb/parser/constraints/not_null_constraint.hpp"
#include "duckdb/parser/constraints/unique_constraint.hpp"
#include "duckdb/parser/parser.hpp"
#include "duckdb/parser/parsed_data/create_table_info.hpp"

namespace duckdb {

//----------------------------------------------------------------------------------------------------------------------
// Logon
//----------------------------------------------------------------------------------------------------------------------
TEST_CASE("Logon strings are split into host, user and password", "[logon]") {
	const auto check = [](const string &logon, const string &host, const string &user, const string &password) {
		INFO(logon);
		const auto parts = ParseLogonString(logon);
		CHECK(parts.host == host);
		CHECK(parts.user == user);
		CHECK(parts.password == password);
	};
	check("host/user,pass", "host", "user", "pass");
	check("host/user,pass,account", "host", "user", "pass");
	check("host/user", "host", "user", "");
	check("host", "host", "", "");
	check("", "", "", "");
	check("/user,pass", "", "user", "pass");
	check("host/user,", "host", "user", "");
}

TEST_CASE("Logon sources override each other: secret < options < logon string", "[logon]") {
	TeradataSecret secret;
	secret.host = "secret-host";
	secret.user = "secret-user";
	secret.password = "secret-pass";
	secret.database = "secret-db";

	const auto from_secret = ResolveLogon(secret, {}, "");
	CHECK(from_secret.ConnectionString() == "secret-host/secret-user,secret-pass");
	CHECK(from_secret.database == "secret-db");

	const auto with_options = ResolveLogon(secret, {{"HOST", Value("option-host")}, {"database", Value("db")}}, "");
	CHECK(with_options.host == "option-host");
	CHECK(with_options.user == "secret-user");
	CHECK(with_options.database == "db");

	const auto with_logon = ResolveLogon(secret, {{"host", Value("option-host")}}, "logon-host/logon-user");
	CHECK(with_logon.host == "logon-host");
	CHECK(with_logon.user == "logon-user");
	CHECK(with_logon.password == "secret-pass");
}

TEST_CASE("The logon database defaults to the user, and host and user are required", "[logon]") {
	const auto logon = ResolveLogon({}, {}, "host/user");
	CHECK(logon.database == "user");
	CHECK(logon.ConnectionString() == "host/user");
	CHECK(logon.buffer_size == TeradataLogon::DEFAULT_BUFFER_SIZE);

	REQUIRE_THROWS_AS(ResolveLogon({}, {}, "host"), InvalidInputException);
	REQUIRE_THROWS_AS(ResolveLogon({}, {{"user", Value("u")}}, ""), InvalidInputException);
}

TEST_CASE("The buffer size must be positive", "[logon]") {
	CHECK(ParseBufferSize({}) == TeradataLogon::DEFAULT_BUFFER_SIZE);
	CHECK(ParseBufferSize(Value::INTEGER(4096)) == 4096);
	CHECK(ParseBufferSize(Value("2048")) == 2048);
	REQUIRE_THROWS_AS(ParseBufferSize(Value::INTEGER(0)), InvalidInputException);
	REQUIRE_THROWS_AS(ParseBufferSize(Value::INTEGER(-1)), InvalidInputException);
	CHECK(ResolveLogon({}, {{"buffer_size", Value::INTEGER(100)}}, "h/u").buffer_size == 100);
}

TEST_CASE("Secret options must be exactly host, user, database and password", "[secret]") {
	const case_insensitive_map_t<Value> complete {
	    {"HOST", Value("h")}, {"user", Value("u")}, {"Database", Value("d")}, {"password", Value("p")}};
	const auto parsed = TeradataSecret::ParseOptions(complete);
	CHECK(parsed.at("host") == "h");
	CHECK(parsed.at("database") == "d");

	auto missing = complete;
	missing.erase("password");
	REQUIRE_THROWS_AS(TeradataSecret::ParseOptions(missing), InvalidInputException);

	auto unknown = complete;
	unknown["port"] = Value("1025");
	REQUIRE_THROWS_AS(TeradataSecret::ParseOptions(unknown), InternalException);
}

//----------------------------------------------------------------------------------------------------------------------
// Queries
//----------------------------------------------------------------------------------------------------------------------
TEST_CASE("Trailing semicolons and whitespace are stripped", "[query]") {
	CHECK(StripTrailingSemicolons("SELECT 1;") == "SELECT 1");
	CHECK(StripTrailingSemicolons("SELECT 1 ; ;\n\t") == "SELECT 1");
	CHECK(StripTrailingSemicolons("SELECT ';' FROM t") == "SELECT ';' FROM t");
	CHECK(StripTrailingSemicolons(" ; ").empty());
}

TEST_CASE("Scan SQL comes from the query or the table, plus the pushed-down filters", "[query]") {
	CHECK(BuildScanSQL("SELECT 1", "", "", "") == "SELECT 1");
	CHECK(BuildScanSQL("", "db", "tbl", "") == "SELECT * FROM db.tbl");
	CHECK(BuildScanSQL("", "db", "tbl", "\"a\" = '1'") == "SELECT * FROM db.tbl WHERE \"a\" = '1'");
	REQUIRE_THROWS_AS(BuildScanSQL("", "db", "", ""), InvalidInputException);
}

TEST_CASE("Temporal columns may arrive as CHAR", "[query]") {
	const TeradataType ts6(TeradataTypeId::TIMESTAMP, 6);
	CHECK(IsTransmittedAs(ts6, ts6));
	CHECK(IsTransmittedAs(ts6, TeradataType(TeradataTypeId::CHAR, 26)));
	CHECK(IsTransmittedAs(TeradataType(TeradataTypeId::TIMESTAMP, 3), TeradataType(TeradataTypeId::CHAR, 23)));
	CHECK(IsTransmittedAs(TeradataType(TeradataTypeId::TIMESTAMP, 0), TeradataType(TeradataTypeId::CHAR, 19)));
	CHECK_FALSE(IsTransmittedAs(TeradataType(TeradataTypeId::TIMESTAMP, 3), TeradataType(TeradataTypeId::CHAR, 26)));
	CHECK(IsTransmittedAs(TeradataType(TeradataTypeId::TIMESTAMP_TZ, 2), TeradataType(TeradataTypeId::CHAR, 32)));
	CHECK(IsTransmittedAs(TeradataType(TeradataTypeId::TIME, 6), TeradataType(TeradataTypeId::CHAR, 15)));
	CHECK(IsTransmittedAs(TeradataType(TeradataTypeId::TIME_TZ, 6), TeradataType(TeradataTypeId::CHAR, 21)));
	CHECK_FALSE(IsTransmittedAs(TeradataTypeId::INTEGER, TeradataTypeId::BIGINT));
	CHECK_FALSE(IsTransmittedAs(TeradataTypeId::INTEGER, TeradataType(TeradataTypeId::CHAR, 15)));
}

TEST_CASE("A changed result schema is reported with the column name", "[query]") {
	const vector<TeradataType> bound {TeradataTypeId::INTEGER, TeradataType(TeradataTypeId::VARCHAR, 10)};
	CHECK_NOTHROW(CheckBoundTypes({"a", "b"}, bound, bound));
	REQUIRE_THROWS_WITH(CheckBoundTypes({"a", "b"}, bound, {TeradataTypeId::INTEGER, TeradataTypeId::BIGINT}),
	                    Catch::Contains("'b'"));
}

//----------------------------------------------------------------------------------------------------------------------
// INSERT
//----------------------------------------------------------------------------------------------------------------------
TEST_CASE("INSERT statements declare their parameters in a USING clause", "[insert]") {
	CHECK(BuildInsertSQL("db", "t", {{"id", LogicalType::INTEGER}, {"name", LogicalType::VARCHAR}}) ==
	      "USING (\"id\" INTEGER, \"name\" VARCHAR(64000)) INSERT INTO \"db\".\"t\" VALUES (:\"id\", :\"name\");");
	CHECK(BuildInsertSQL("", "t", {{"x", LogicalType::DOUBLE}}) ==
	      "USING (\"x\" FLOAT) INSERT INTO \"t\" VALUES (:\"x\");");
}

TEST_CASE("INSERT writes the mapped columns in input order", "[insert]") {
	const auto indexes = [](const vector<PhysicalIndex> &columns) {
		vector<idx_t> result;
		for (auto &column : columns) {
			result.push_back(column.index);
		}
		return result;
	};
	CHECK(indexes(GetInsertColumnOrder({}, 3)) == vector<idx_t> {0, 1, 2});

	// INSERT INTO t (c, a): table column 0 is input 1, column 1 is not inserted, column 2 is input 0
	physical_index_vector_t<idx_t> map;
	map.push_back(1);
	map.push_back(DConstants::INVALID_INDEX);
	map.push_back(0);
	CHECK(indexes(GetInsertColumnOrder(map, 3)) == vector<idx_t> {2, 0});
}

//----------------------------------------------------------------------------------------------------------------------
// CREATE TABLE
//----------------------------------------------------------------------------------------------------------------------
TEST_CASE("CREATE TABLE renders columns, constraints and the primary index option", "[create_table]") {
	CreateTableInfo info;
	info.SetQualifiedName(QualifiedName(Identifier(), Identifier("db"), Identifier("t")));
	info.columns.AddColumn(ColumnDefinition(Identifier("id"), LogicalType::INTEGER));
	info.columns.AddColumn(ColumnDefinition(Identifier("name"), LogicalType::VARCHAR));
	ColumnDefinition score(Identifier("score"), LogicalType::DOUBLE);
	score.SetDefaultValue(std::move(Parser::GetBuiltinParser().ParseExpressionList("1.5")[0]));
	info.columns.AddColumn(std::move(score));
	info.constraints.push_back(make_uniq<UniqueConstraint>(LogicalIndex(0), Identifier("id"), true));
	info.constraints.push_back(make_uniq<NotNullConstraint>(LogicalIndex(0)));
	info.constraints.push_back(make_uniq<NotNullConstraint>(LogicalIndex(1)));

	CHECK(GetTeradataCreateTableSQL(info, true) ==
	      "CREATE TABLE \"db\".\"t\"(\"id\" INTEGER PRIMARY KEY NOT NULL, \"name\" VARCHAR(64000) NOT NULL, "
	      "\"score\" FLOAT DEFAULT (1.5));");

	info.on_conflict = OnCreateConflict::IGNORE_ON_CONFLICT;
	const auto sql = GetTeradataCreateTableSQL(info, false);
	CHECK(StringUtil::StartsWith(sql, "CREATE TABLE IF NOT EXISTS "));
	CHECK(StringUtil::EndsWith(sql, ") NO PRIMARY INDEX;"));
}

TEST_CASE("CREATE TABLE puts multi-column keys after the columns", "[create_table]") {
	CreateTableInfo info;
	info.SetQualifiedName(QualifiedName(Identifier("t")));
	info.columns.AddColumn(ColumnDefinition(Identifier("a"), LogicalType::INTEGER));
	info.columns.AddColumn(ColumnDefinition(Identifier("b"), LogicalType::INTEGER));
	info.constraints.push_back(make_uniq<NotNullConstraint>(LogicalIndex(0)));
	info.constraints.push_back(make_uniq<UniqueConstraint>(vector<Identifier> {"a", "b"}, true));

	// NOT NULL is implied by the multi-column primary key, so it is not repeated
	CHECK(GetTeradataCreateTableSQL(info, true) ==
	      "CREATE TABLE \"t\"(\"a\" INTEGER, \"b\" INTEGER, PRIMARY KEY(a, b));");
}

//----------------------------------------------------------------------------------------------------------------------
// Catalog loading
//----------------------------------------------------------------------------------------------------------------------
TEST_CASE("Column rows are grouped into tables, skipping unsupported ones", "[catalog]") {
	const vector<TeradataColumnRow> rows {
	    {"orders", "id", "I ", 4},     {"orders", "note", "CV", 200}, {"shapes", "id", "I ", 4},
	    {"shapes", "geom", "??", 100}, {"users", "name", "CF", 30},
	};
	const auto infos = BuildTableInfos(Identifier("db"), rows);

	REQUIRE(infos.size() == 2);
	CHECK(infos[0]->GetTableName() == "orders");
	CHECK(infos[0]->GetQualifiedName().Schema() == "db");
	REQUIRE(infos[0]->columns.LogicalColumnCount() == 2);
	CHECK(infos[0]->columns.GetColumn(LogicalIndex(1)).Type() == LogicalType::VARCHAR);
	CHECK(infos[0]->teradata_types[1] == TeradataType(TeradataTypeId::VARCHAR, 200));

	CHECK(infos[1]->GetTableName() == "users");
	CHECK(infos[1]->teradata_types[0] == TeradataType(TeradataTypeId::CHAR, 30));

	CHECK(BuildTableInfos(Identifier("db"), {}).empty());
	CHECK(BuildTableInfos(Identifier("db"), {{"udt", "x", "UT", 4}}).empty());
}

} // namespace duckdb
