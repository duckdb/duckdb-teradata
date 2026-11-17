#include "catch.hpp"

#include "teradata_column_reader.hpp"
#include "teradata_column_writer.hpp"
#include "teradata_type.hpp"
#include "util/binary_reader.hpp"

namespace duckdb {

namespace {

//! Encode a vector the way an INSERT sends it: one record per row
vector<string> EncodeRows(const LogicalType &type, Vector &input, idx_t count) {
	auto writer = TeradataColumnWriter::Make(type);
	writer->Init(input, count);

	vector<int32_t> lengths(count, 0);
	writer->ComputeSizes(count, lengths.data());

	vector<string> records;
	vector<char *> cursors;
	for (idx_t i = 0; i < count; i++) {
		records.emplace_back(lengths[i], '\0');
	}
	for (auto &record : records) {
		cursors.push_back(&record[0]);
	}
	writer->EncodeVector(count, cursors.data());
	return records;
}

//! Decode records the way a query result is read
Vector DecodeRows(const TeradataType &td_type, const vector<string> &records, const vector<bool> &nulls) {
	auto reader = TeradataColumnReader::Make(td_type);
	Vector result(td_type.ToDuckDB(), records.size());
	for (idx_t i = 0; i < records.size(); i++) {
		BinaryReader binary(records[i].data(), records[i].size());
		reader->Decode(binary, result, i, nulls[i]);
	}
	return result;
}

void CheckRoundTrip(const LogicalType &type, const vector<Value> &values) {
	INFO(type.ToString());
	const auto count = values.size();
	Vector input(type, count);
	vector<bool> nulls;
	for (idx_t i = 0; i < count; i++) {
		input.SetValue(i, values[i]);
		nulls.push_back(values[i].IsNull());
	}

	const auto records = EncodeRows(type, input, count);
	auto output = DecodeRows(TeradataType::FromDuckDB(type), records, nulls);

	for (idx_t i = 0; i < count; i++) {
		INFO("row " << i);
		CHECK(Value::NotDistinctFrom(output.GetValue(i), values[i]));
	}
}

} // namespace

TEST_CASE("Column readers exist for the types queries return", "[column_io]") {
	const TeradataType supported[] = {
	    TeradataTypeId::VARCHAR,
	    TeradataType(TeradataTypeId::CHAR, 10),
	    TeradataType(TeradataTypeId::BYTE, 4),
	    TeradataTypeId::VARBYTE,
	    TeradataTypeId::DATE,
	    TeradataTypeId::BYTEINT,
	    TeradataTypeId::SMALLINT,
	    TeradataTypeId::INTEGER,
	    TeradataTypeId::BIGINT,
	    TeradataTypeId::FLOAT,
	    TeradataType(TeradataTypeId::INTERVAL_YEAR, 2),
	    TeradataType(TeradataTypeId::INTERVAL_DAY_TO_SECOND, 2, 6),
	    TeradataType(TeradataTypeId::INTERVAL_HOUR, 2),
	    TeradataType(TeradataTypeId::INTERVAL_SECOND, 2, 6),
	};
	for (auto &type : supported) {
		INFO(type.ToString());
		CHECK(TeradataColumnReader::Make(type) != nullptr);
	}
	REQUIRE_THROWS_AS(TeradataColumnReader::Make(TeradataTypeId::CLOB), NotImplementedException);
}

TEST_CASE("Decimal column readers and writers cover widths 1 to 38", "[column_io]") {
	for (uint8_t width = 1; width <= 38; width++) {
		INFO("DECIMAL(" << int(width) << ")");
		CHECK(TeradataColumnReader::Make(TeradataType(TeradataTypeId::DECIMAL, width, 0)) != nullptr);
		CHECK(TeradataColumnWriter::Make(LogicalType::DECIMAL(width, 0)) != nullptr);
	}
	REQUIRE_THROWS_AS(TeradataColumnReader::Make(TeradataType(TeradataTypeId::DECIMAL, 39, 0)), InvalidInputException);
}

TEST_CASE("Column writers reject types Teradata cannot store", "[column_io]") {
	REQUIRE_THROWS_AS(TeradataColumnWriter::Make(LogicalType::UUID), NotImplementedException);
	REQUIRE_THROWS_AS(TeradataColumnWriter::Make(LogicalType::LIST(LogicalType::INTEGER)), NotImplementedException);
}

TEST_CASE("Values survive the writer to reader round trip", "[column_io]") {
	CheckRoundTrip(LogicalType::TINYINT, {Value::TINYINT(-7), Value(LogicalType::TINYINT), Value::TINYINT(127)});
	CheckRoundTrip(LogicalType::SMALLINT, {Value::SMALLINT(-32768), Value::SMALLINT(42)});
	CheckRoundTrip(LogicalType::INTEGER, {Value::INTEGER(0), Value(LogicalType::INTEGER), Value::INTEGER(-123456)});
	CheckRoundTrip(LogicalType::BIGINT, {Value::BIGINT(NumericLimits<int64_t>::Maximum()), Value::BIGINT(-1)});
	CheckRoundTrip(LogicalType::DOUBLE, {Value::DOUBLE(3.25), Value(LogicalType::DOUBLE), Value::DOUBLE(-1e300)});
	CheckRoundTrip(LogicalType::VARCHAR, {Value("hello"), Value(LogicalType::VARCHAR), Value(""), Value("åäö")});
	CheckRoundTrip(LogicalType::DATE, {Value::DATE(2024, 2, 29), Value::DATE(1900, 1, 1), Value(LogicalType::DATE)});
	CheckRoundTrip(LogicalType::DECIMAL(2, 1), {Value::DECIMAL(int16_t(-99), 2, 1), Value::DECIMAL(int16_t(5), 2, 1)});
	CheckRoundTrip(LogicalType::DECIMAL(9, 2), {Value::DECIMAL(int32_t(123456789), 9, 2)});
	CheckRoundTrip(LogicalType::DECIMAL(18, 4), {Value::DECIMAL(int64_t(-12345678901234), 18, 4)});
}

} // namespace duckdb
