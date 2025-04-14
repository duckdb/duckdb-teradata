#include "teradata_type.hpp"

namespace duckdb {

string TeradataType::ToString() const {
	switch (id) {
	case TeradataTypeId::INVALID:
		return "INVALID";
	case TeradataTypeId::ARRAY:
		return "ARRAY";
	case TeradataTypeId::BYTE:
		return "BYTE(" + to_string(width) + ")";
	case TeradataTypeId::VARBYTE:
		return "VARBYTE(" + to_string(width) + ")";
	case TeradataTypeId::BLOB:
		return "BLOB(" + to_string(width) + ")";
	case TeradataTypeId::CHAR:
		return "CHAR(" + to_string(width) + ")";
	case TeradataTypeId::VARCHAR:
		return "VARCHAR(" + to_string(width) + ")";
	case TeradataTypeId::CLOB:
		return "CLOB(" + to_string(width) + ")";
	case TeradataTypeId::DATE:
		return "DATE";
	case TeradataTypeId::TIME:
		return "TIME";
	case TeradataTypeId::TIMESTAMP:
		return "TIMESTAMP";
	case TeradataTypeId::TIME_TZ:
		return "TIME_TZ";
	case TeradataTypeId::TIMESTAMP_TZ:
		return "TIMESTAMP_TZ";
	case TeradataTypeId::ST_GEOMETRY:
		return "ST_GEOMETRY";
	case TeradataTypeId::MBR:
		return "MBR";
	case TeradataTypeId::INTERVAL_YEAR:
		return "INTERVAL_YEAR";
	case TeradataTypeId::INTERVAL_YEAR_TO_MONTH:
		return "INTERVAL_YEAR_TO_MONTH";
	case TeradataTypeId::INTERVAL_MONTH:
		return "INTERVAL_MONTH";
	case TeradataTypeId::INTERVAL_DAY:
		return "INTERVAL_DAY";
	case TeradataTypeId::INTERVAL_DAY_TO_HOUR:
		return "INTERVAL_DAY_TO_HOUR";
	case TeradataTypeId::INTERVAL_DAY_TO_MINUTE:
		return "INTERVAL_DAY_TO_MINUTE";
	case TeradataTypeId::INTERVAL_DAY_TO_SECOND:
		return "INTERVAL_DAY_TO_SECOND";
	case TeradataTypeId::INTERVAL_HOUR_TO_MINUTE:
		return "INTERVAL_HOUR_TO_MINUTE";
	case TeradataTypeId::INTERVAL_HOUR_TO_SECOND:
		return "INTERVAL_HOUR_TO_SECOND";
	case TeradataTypeId::INTERVAL_MINUTE:
		return "INTERVAL_MINUTE";
	case TeradataTypeId::INTERVAL_MINUTE_TO_SECOND:
		return "INTERVAL_MINUTE_TO_SECOND";
	case TeradataTypeId::INTERVAL_SECOND:
		return "INTERVAL_SECOND";
	case TeradataTypeId::JSON:
		return "JSON";
	case TeradataTypeId::BYTEINT:
		return "BYTEINT";
	case TeradataTypeId::SMALLINT:
		return "SMALLINT";
	case TeradataTypeId::INTEGER:
		return "INTEGER";
	case TeradataTypeId::BIGINT:
		return "BIGINT";
	case TeradataTypeId::DECIMAL:
		return "DECIMAL(" + to_string(width) + ", " + to_string(scale) + ")";
	case TeradataTypeId::FLOAT:
		return "FLOAT";
	case TeradataTypeId::NUMBER:
		return "NUMBER";
	case TeradataTypeId::ANYTYPE:
		return "ANYTYPE";
	case TeradataTypeId::VARIANT:
		return "VARIANT";
	case TeradataTypeId::PERIOD_DATE:
		return "PERIOD_DATE";
	case TeradataTypeId::PERIOD_TIME:
		return "PERIOD_TIME";
	case TeradataTypeId::PERIOD_TIME_TZ:
		return "PERIOD_TIME_TZ";
	case TeradataTypeId::PERIOD_TIMESTAMP:
		return "PERIOD_TIMESTAMP";
	case TeradataTypeId::PERIOD_TIMESTAMP_TZ:
		return "PERIOD_TIMESTAMP_TZ";
	case TeradataTypeId::UDT_DISTINCT:
		return "UDT_DISTINCT";
	case TeradataTypeId::UDT_STRUCTURED:
		return "UDT_STRUCTURED";
	case TeradataTypeId::XML:
		return "XML";
	case TeradataTypeId::DATE_A:
		return "DATE (ANSI)";
	case TeradataTypeId::DATE_T:
		return "DATE (TERADATA)";
	default:
		throw NotImplementedException("Unimplemented Teradata type");
	}
}

LogicalType TeradataType::ToDuckDB() const {
	switch (id) {
	case TeradataTypeId::INVALID:
		return LogicalType::INVALID;
	case TeradataTypeId::BYTE:
	case TeradataTypeId::VARBYTE:
	case TeradataTypeId::BLOB:
		return LogicalType::BLOB;
	case TeradataTypeId::CHAR:
	case TeradataTypeId::VARCHAR:
	case TeradataTypeId::CLOB:
		return LogicalType::VARCHAR;
	case TeradataTypeId::DATE:
		return LogicalType::DATE;
	case TeradataTypeId::TIME:
		return LogicalType::TIME;
	case TeradataTypeId::TIMESTAMP:
		return LogicalType::TIMESTAMP;
	case TeradataTypeId::TIME_TZ:
		return LogicalType::TIME_TZ;
	case TeradataTypeId::TIMESTAMP_TZ:
		return LogicalType::TIMESTAMP_TZ;
	case TeradataTypeId::BYTEINT:
		return LogicalType::TINYINT;
	case TeradataTypeId::SMALLINT:
		return LogicalType::SMALLINT;
	case TeradataTypeId::INTEGER:
		return LogicalType::INTEGER;
	case TeradataTypeId::BIGINT:
		return LogicalType::BIGINT;
	case TeradataTypeId::DECIMAL:
		// TODO: Not correct.
		return LogicalType::DECIMAL(width, scale);
	case TeradataTypeId::FLOAT:
		// Teradata only supports 8-byte floats (REAL and DOUBLE PRECISION and FLOAT are all the same)
		return LogicalType::DOUBLE;
	case TeradataTypeId::DATE_T:
	case TeradataTypeId::DATE_A:
		return LogicalType::DATE;
	case TeradataTypeId::JSON:
		return LogicalType::JSON();
	case TeradataTypeId::INTERVAL_HOUR_TO_SECOND:
		// Convert to interval
		return LogicalType::INTERVAL;
	default:
		throw NotImplementedException("Unimplemented Teradata type");
	}
}

unordered_map<string, TeradataTypeId> TeradataType::code_map = {
    {"++", TeradataTypeId::ANYTYPE}, // TD_ANYTYPE
    {"A1", TeradataTypeId::INVALID}, // UDT
    {"AT", TeradataTypeId::TIME},
    {"BF", TeradataTypeId::BYTE},
    {"BO", TeradataTypeId::BLOB},
    {"BV", TeradataTypeId::VARBYTE},
    {"CF", TeradataTypeId::CHAR},
    {"CO", TeradataTypeId::CLOB},
    {"CV", TeradataTypeId::VARCHAR},
    {"D ", TeradataTypeId::DECIMAL},
    {"DA", TeradataTypeId::DATE}, // DATE
    {"DH", TeradataTypeId::INTERVAL_DAY_TO_HOUR},
    {"DM", TeradataTypeId::INTERVAL_DAY_TO_MINUTE},
    {"DS", TeradataTypeId::INTERVAL_DAY_TO_SECOND},
    {"DY", TeradataTypeId::INTERVAL_DAY},
    {"F ", TeradataTypeId::FLOAT},
    {"HM", TeradataTypeId::INTERVAL_HOUR_TO_MINUTE},
    {"HR", TeradataTypeId::INTERVAL_HOUR},
    {"HS", TeradataTypeId::INTERVAL_HOUR_TO_SECOND},
    {"I1", TeradataTypeId::BYTEINT},
    {"I2", TeradataTypeId::SMALLINT},
    {"I8", TeradataTypeId::BIGINT},
    {"I ", TeradataTypeId::INTEGER},
    {"MI", TeradataTypeId::INTERVAL_MINUTE},
    {"MO", TeradataTypeId::INTERVAL_MONTH},
    {"MS", TeradataTypeId::INTERVAL_MINUTE_TO_SECOND},
    {"N ", TeradataTypeId::NUMBER},
    {"PD", TeradataTypeId::PERIOD_DATE},
    {"PM", TeradataTypeId::PERIOD_TIMESTAMP_TZ},
    {"PS", TeradataTypeId::PERIOD_TIMESTAMP},
    {"PT", TeradataTypeId::PERIOD_TIME},
    {"PZ", TeradataTypeId::PERIOD_TIME_TZ},
    {"SC", TeradataTypeId::INTERVAL_SECOND},
    {"SZ", TeradataTypeId::TIMESTAMP_TZ},
    {"TS", TeradataTypeId::TIMESTAMP},
    {"TZ", TeradataTypeId::TIME_TZ},
    {"UT", TeradataTypeId::INVALID}, // "UDT"
    {"YM", TeradataTypeId::INTERVAL_YEAR_TO_MONTH},
    {"YR", TeradataTypeId::INTERVAL_YEAR},
    {"AN", TeradataTypeId::INVALID}, // "UDT",
    {"XM", TeradataTypeId::XML},
    {"JN", TeradataTypeId::JSON},
    {"DT", TeradataTypeId::INVALID}, // "DATASET",
    {"??", TeradataTypeId::ST_GEOMETRY},
};

bool TeradataType::HasLengthModifier() const {
	switch (id) {
	case TeradataTypeId::BYTE:
	case TeradataTypeId::VARBYTE:
	case TeradataTypeId::BLOB:
	case TeradataTypeId::CHAR:
	case TeradataTypeId::VARCHAR:
	case TeradataTypeId::CLOB:
		return true;
	default:
		return false;
	}
}

TeradataType TeradataType::FromDuckDB(const LogicalType &type) {
	if (type.IsJSONType()) {
		return TeradataTypeId::JSON;
	}

	switch (type.id()) {
	// Boolean (does not exist in Teradata, so map to byteint)
	case LogicalTypeId::BOOLEAN:
		return TeradataTypeId::BYTEINT;

	// Integer types
	case LogicalTypeId::TINYINT:
		return TeradataTypeId::BYTEINT;
	case LogicalTypeId::SMALLINT:
		return TeradataTypeId::SMALLINT;
	case LogicalTypeId::INTEGER:
		return TeradataTypeId::INTEGER;
	case LogicalTypeId::BIGINT:
		return TeradataTypeId::BIGINT;

	// Only DOUBLEs are supported
	case LogicalTypeId::DOUBLE: {
		return TeradataTypeId::FLOAT;
	}

	// Decimal type
	case LogicalTypeId::DECIMAL: {
		TeradataType decimal_type = TeradataTypeId::DECIMAL;
		decimal_type.SetWidth(DecimalType::GetWidth(type));
		decimal_type.SetScale(DecimalType::GetScale(type));
		return decimal_type;
	}

	// Time types
	case LogicalTypeId::TIMESTAMP:
		return TeradataTypeId::TIMESTAMP;
	case LogicalTypeId::DATE:
		return TeradataTypeId::DATE;
	case LogicalTypeId::TIME:
		return TeradataTypeId::TIME;
	case LogicalTypeId::TIME_TZ:
		return TeradataTypeId::TIME_TZ;
	case LogicalTypeId::TIMESTAMP_TZ:
		return TeradataTypeId::TIMESTAMP_TZ;

	// Varchar
	case LogicalTypeId::VARCHAR: {
		// Since DuckDB types are variable size, set the length to the maximum
		TeradataType char_type = TeradataTypeId::VARCHAR;
		char_type.SetLength(TeradataType::MAX_TYPE_LENGTH);
		return char_type;
	}

	// Blob
	case LogicalTypeId::BLOB: {
		// Since DuckDB types are variable size, set the length to the maximum
		TeradataType blob_type = TeradataTypeId::VARBYTE;
		blob_type.SetLength(TeradataType::MAX_TYPE_LENGTH);
		return blob_type;
	}
	default:
		throw NotImplementedException("Cannot convert DuckDB type '%s' to Teradata type", type.ToString());
	}
}

} // namespace duckdb