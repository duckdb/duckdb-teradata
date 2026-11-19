#include "catch.hpp"

#include "teradata_type.hpp"

namespace duckdb {

namespace {

struct ParcelTypeCase {
	int16_t code;
	TeradataTypeId id;
	TeradataColumnTypeVariant variant;
};

const ParcelTypeCase PARCEL_TYPE_CASES[] = {
#include "parcel_type_cases.inc"
};

} // namespace

TEST_CASE("DecodeParcelType matches every code of the original switch", "[types]") {
	for (const auto &c : PARCEL_TYPE_CASES) {
		INFO("parcel type code " << c.code);
		const auto decoded = DecodeParcelType(c.code);
		CHECK(decoded.type.GetId() == c.id);
		CHECK(decoded.variant == c.variant);
	}
}

TEST_CASE("DecodeParcelType rejects unknown codes", "[types]") {
	// 401..402 would be BLOB variants, which Teradata does not send
	// 698 and 1199 were LONG VARBYTE codes in the original, off-by-one switch
	for (const int16_t code : {0, 1, 401, 900, 449 + 2, 698, 1199, 1300, -1}) {
		INFO("parcel type code " << code);
		REQUIRE_THROWS_AS(DecodeParcelType(code), NotImplementedException);
	}
}

TEST_CASE("TeradataType::ToString", "[types]") {
	CHECK(TeradataType(TeradataTypeId::INTEGER).ToString() == "INTEGER");
	CHECK(TeradataType(TeradataTypeId::VARCHAR, 64000).ToString() == "VARCHAR(64000)");
	CHECK(TeradataType(TeradataTypeId::BYTE, 4).ToString() == "BYTE(4)");
	CHECK(TeradataType(TeradataTypeId::DECIMAL, 18, 2).ToString() == "DECIMAL(18, 2)");
	CHECK(TeradataType(TeradataTypeId::TIMESTAMP, 6).ToString() == "TIMESTAMP(6)");
	CHECK(TeradataType(TeradataTypeId::TIME_TZ, 6).ToString() == "TIME(6) WITH TIME ZONE");
	CHECK(TeradataType(TeradataTypeId::TIMESTAMP_TZ, 3).ToString() == "TIMESTAMP(3) WITH TIME ZONE");
	CHECK(TeradataType(TeradataTypeId::INTERVAL_HOUR).ToString() == "INTERVAL_HOUR");
	CHECK(TeradataType(TeradataTypeId::DATE).ToString() == "DATE");
	REQUIRE_THROWS_AS(TeradataType(TeradataTypeId::LONGVARBYTE).ToString(), NotImplementedException);
}

TEST_CASE("TeradataType::ToDuckDB", "[types]") {
	CHECK(TeradataType(TeradataTypeId::BYTEINT).ToDuckDB() == LogicalType::TINYINT);
	CHECK(TeradataType(TeradataTypeId::INTEGER).ToDuckDB() == LogicalType::INTEGER);
	CHECK(TeradataType(TeradataTypeId::FLOAT).ToDuckDB() == LogicalType::DOUBLE);
	CHECK(TeradataType(TeradataTypeId::CHAR, 3).ToDuckDB() == LogicalType::VARCHAR);
	CHECK(TeradataType(TeradataTypeId::VARBYTE, 3).ToDuckDB() == LogicalType::BLOB);
	CHECK(TeradataType(TeradataTypeId::DECIMAL, 10, 2).ToDuckDB() == LogicalType::DECIMAL(10, 2));
	CHECK(TeradataType(TeradataTypeId::JSON).ToDuckDB().IsJSONType());
	CHECK(TeradataType(TeradataTypeId::INVALID).ToDuckDB() == LogicalType::INVALID);
	REQUIRE_THROWS_AS(TeradataType(TeradataTypeId::PERIOD_DATE).ToDuckDB(), NotImplementedException);
}

TEST_CASE("Teradata timestamps round up to the closest DuckDB precision", "[types]") {
	const LogicalType expected[] = {LogicalType::TIMESTAMP_S,  LogicalType::TIMESTAMP_MS, LogicalType::TIMESTAMP_MS,
	                                LogicalType::TIMESTAMP_MS, LogicalType::TIMESTAMP,    LogicalType::TIMESTAMP,
	                                LogicalType::TIMESTAMP};
	for (int64_t precision = 0; precision <= 6; precision++) {
		INFO("TIMESTAMP(" << precision << ")");
		CHECK(TeradataType(TeradataTypeId::TIMESTAMP, precision).ToDuckDB() == expected[precision]);
	}
	REQUIRE_THROWS_AS(TeradataType(TeradataTypeId::TIMESTAMP, 7).ToDuckDB(), NotImplementedException);
	REQUIRE_THROWS_AS(TeradataType(TeradataTypeId::TIMESTAMP, -1).ToDuckDB(), NotImplementedException);
}

TEST_CASE("TeradataType::FromDuckDB", "[types]") {
	CHECK(TeradataType::FromDuckDB(LogicalType::BOOLEAN) == TeradataType(TeradataTypeId::BYTEINT));
	CHECK(TeradataType::FromDuckDB(LogicalType::BIGINT) == TeradataType(TeradataTypeId::BIGINT));
	CHECK(TeradataType::FromDuckDB(LogicalType::DECIMAL(12, 4)) == TeradataType(TeradataTypeId::DECIMAL, 12, 4));
	CHECK(TeradataType::FromDuckDB(LogicalType::TIMESTAMP_MS) == TeradataType(TeradataTypeId::TIMESTAMP, 3));
	CHECK(TeradataType::FromDuckDB(LogicalType::TIME_TZ) == TeradataType(TeradataTypeId::TIME_TZ, 6));
	CHECK(TeradataType::FromDuckDB(LogicalType::VARCHAR) ==
	      TeradataType(TeradataTypeId::VARCHAR, TeradataType::MAX_TYPE_LENGTH));
	CHECK(TeradataType::FromDuckDB(LogicalType::JSON()) == TeradataType(TeradataTypeId::JSON));
	REQUIRE_THROWS_AS(TeradataType::FromDuckDB(LogicalType::UUID), NotImplementedException);
}

TEST_CASE("DuckDB types survive a round trip through Teradata types", "[types]") {
	const LogicalType types[] = {LogicalType::TINYINT,      LogicalType::SMALLINT,     LogicalType::INTEGER,
	                             LogicalType::BIGINT,       LogicalType::DOUBLE,       LogicalType::DECIMAL(9, 3),
	                             LogicalType::DATE,         LogicalType::TIME,         LogicalType::TIME_TZ,
	                             LogicalType::TIMESTAMP_S,  LogicalType::TIMESTAMP_MS, LogicalType::TIMESTAMP,
	                             LogicalType::TIMESTAMP_TZ, LogicalType::VARCHAR,      LogicalType::BLOB};
	for (auto &type : types) {
		INFO(type.ToString());
		CHECK(TeradataType::FromDuckDB(type).ToDuckDB() == type);
	}
}

TEST_CASE("TeradataType::HasLengthModifier", "[types]") {
	for (auto id : {TeradataTypeId::BYTE, TeradataTypeId::VARBYTE, TeradataTypeId::BLOB, TeradataTypeId::CHAR,
	                TeradataTypeId::VARCHAR, TeradataTypeId::CLOB}) {
		CHECK(TeradataType(id).HasLengthModifier());
	}
	for (auto id : {TeradataTypeId::INTEGER, TeradataTypeId::DECIMAL, TeradataTypeId::TIMESTAMP}) {
		CHECK_FALSE(TeradataType(id).HasLengthModifier());
	}
}

TEST_CASE("TeradataType::FromShortCode", "[types]") {
	CHECK(TeradataType::FromShortCode("CV") == TeradataTypeId::VARCHAR);
	CHECK(TeradataType::FromShortCode("I ") == TeradataTypeId::INTEGER);
	CHECK(TeradataType::FromShortCode("D ") == TeradataTypeId::DECIMAL);
	CHECK(TeradataType::FromShortCode("??") == TeradataTypeId::ST_GEOMETRY);
	CHECK(TeradataType::FromShortCode("UT") == TeradataTypeId::INVALID);
	CHECK(TeradataType::FromShortCode("XX") == TeradataTypeId::INVALID);
	CHECK(TeradataType::FromShortCode("") == TeradataTypeId::INVALID);
}

} // namespace duckdb
