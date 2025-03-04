#pragma once
#include "teradata_common.hpp"
#include "duckdb/common/types.hpp"

namespace duckdb {

enum class TeradataTypeId : uint8_t {
	INVALID = 0,

	// An ARRAY data type is used for storing and accessing multidimensional data.
	// The ARRAY data type can store many values of the same specific data type in a sequential or matrix-like format.
	ARRAY,

	// Byte data types store raw data as logical bit streams. These data types are stored in the client system format
	// and are not translated by the database. The data is transmitted directly from the memory of the client system.
	BYTE,
	VARBYTE,
	BLOB,

	// Character data types represent characters that belong to a given character set.
	// These data types represent character data.
	CHAR,
	VARCHAR,
	CLOB,

	// DateTime data types represent date, time, and timestamp values
	DATE,
	TIME,
	TIMESTAMP,
	TIME_TZ,
	TIMESTAMP_TZ,

	// Geospatial data types represent geographic information and provides a way for applications that manage, analyze,
	// and display geographic information to interface with the database.
	ST_GEOMETRY,
	MBR,

	// Interval data types represent a span of time. For example, an interval value can represent a time span that
	// includes a number of years, months, days, hours, minutes, or seconds.
	INTERVAL_YEAR,
	INTERVAL_YEAR_TO_MONTH,
	INTERVAL_MONTH,
	INTERVAL_DAY,
	INTERVAL_DAY_TO_HOUR,
	INTERVAL_DAY_TO_MINUTE,
	INTERVAL_DAY_TO_SECOND,
	INTERVAL_HOUR,
	INTERVAL_HOUR_TO_MINUTE,
	INTERVAL_HOUR_TO_SECOND,
	INTERVAL_MINUTE,
	INTERVAL_MINUTE_TO_SECOND,
	INTERVAL_SECOND,

	// The JSON data type represents data that is in JSON (JavaScript Object Notation) format.
	JSON,

	// Numeric data types represent a numeric value that is an exact numeric number (integer or decimal) or an
	// approximate numeric number (floating point).
	BYTEINT,
	SMALLINT,
	INTEGER,
	BIGINT,
	DECIMAL, // Alias: NUMERIC,
	FLOAT, // Alias: DOUBLE, REAL
	NUMBER, // Distinct from DECIMAL

	// Parameter data types are data types that can be used only with input or result parameters in a function, method,
	// stored procedure, or external stored procedure.
	ANYTYPE,
	VARIANT,

	// A Period data type represents a time period, where a period is a set of contiguous time granules that extends
	// from a beginning bound up to but not including an ending bound.
	PERIOD_DATE,
	PERIOD_TIME,
	PERIOD_TIME_TZ,
	PERIOD_TIMESTAMP,
	PERIOD_TIMESTAMP_TZ,

	// UDT (User-Defined Type) data types are custom data types that you define to model the structure and behavior of
	// the data used by your applications.
	UDT_DISTINCT,
	UDT_STRUCTURED,

	// The XML data type represents XML content. The data is stored in a compact binary form that preserves
	// the information set of the XML document, including the hierarchy information and type information derived from
	// XML validation.
	XML,


	// Extra, only used when binding
	DATE_T,
	DATE_A,
	LONGVARBYTE,
	LONGVARGRAPHIC,
};

class TeradataType {
public:
	TeradataType(TeradataTypeId id) : id(id) {} // NOLINT: Allow implicit conversion from `TeradataTypeId`
	TeradataTypeId GetId() const { return id; }
	int64_t GetLength() const { return width; }
	int64_t GetPrecision() const { return scale; }
	int64_t GetScale() const { return scale; }

	void SetLength(const int64_t length_p) { width = length_p; }
	void SetPrecision(const int64_t precision_p) { width = precision_p; }
	void SetScale(const int64_t scale_p) { scale = scale_p; }

	string ToString() const;
	LogicalType ToDuckDB() const;

	static TeradataTypeId FromShortCode(const string &code) {
		const auto entry = code_map.find(code);
        if (entry == code_map.end()) {
            return TeradataTypeId::INVALID;
        }
        return entry->second;
	}

	bool IsDecimal() const { return id == TeradataTypeId::DECIMAL; }

private:
	TeradataTypeId id = TeradataTypeId::INVALID;
	int64_t width = 0;
	int64_t scale = 0;

	// From eg. "BF" to TeradataTypeId::BYTE
	static unordered_map<string, TeradataTypeId> code_map;
};


/*
//----------------------------------------------------------------------------------------------------------------------
// Teradata Type
//----------------------------------------------------------------------------------------------------------------------
// BO35-2417-103K, p.406
enum class TeradataTypeId : uint8_t {
	INVALID = 0,
	BLOB,
	BLOB_AS_DEFERRED,
	BLOB_AS_LOCATOR,
	CLOB,
	CLOB_AS_DEFERRED,
	CLOB_AS_LOCATOR,
	VARCHAR,
	CHAR,
	LONGVARCHAR,
	VARGRAPHIC,
	GRAPHIC,
	LONGVARGRAPHIC,
	FLOAT,
	DECIMAL,
	INTEGER,
	SMALLINT,
	BIGINT,
	VARBYTE,
	BYTE,
	LONGVARBYTE,
	DATE_A,
	DATE_T,
	BYTEINT,
	PERIOD_DATE,
	PERIOD_TIME,
	PERIOD_TIME_TZ,
	PERIOD_TIMESTAMP,
	PERIOD_TIMESTAMP_TZ,
	XML_CHAR,
	XML_CHAR_DEFERRED,
	XML_CHAR_LOCATOR,
	XML_BINARY,
	XML_BINARY_DEFERRED,
	XML_BINARY_LOCATOR,
	TIMESTAMP,
	DATE,
};

enum class TeradataTypeVariant : uint8_t {
	STANDARD,
	NULLABLE,
	PARAM_IN,
	PARAM_INOUT,
	PARAM_OUT
};

class TeradataType {
public:
	TeradataType(TeradataTypeId id, TeradataTypeVariant variant, int64_t precision, int64_t scale)
	    : id(id), variant(variant), precision(precision), scale(scale) {
	}

	static TeradataTypeId FromShortCode(const char* code);

	TeradataTypeId GetId() const { return id; }
	TeradataTypeVariant GetVariant() const { return variant; }
	LogicalType GetDuckType() const;
	string ToString() const;

	int64_t GetLength() const { return precision; }
	int64_t GetPrecision() const { return precision; }
	int64_t GetScale() const { return scale; }

	void SetLength(int64_t length) { precision = length; }
	void SetPrecision(int64_t precision_p) { precision = precision_p; }
	void SetScale(int64_t scale_p) { scale = scale_p; }

	bool IsDecimal() const {
		return id == TeradataTypeId::DECIMAL;
	}

private:
	TeradataTypeId id = TeradataTypeId::INVALID;
	TeradataTypeVariant variant = TeradataTypeVariant::STANDARD;
	int64_t precision = 0;
	int64_t scale = 0;

	static unordered_map<string, TeradataTypeId> short_codes;
};

inline LogicalType TeradataType::GetDuckType() const {
	switch(id) {
		case TeradataTypeId::BYTEINT:
			return LogicalType::TINYINT;
		case TeradataTypeId::SMALLINT:
			return LogicalType::SMALLINT;
		case TeradataTypeId::INTEGER:
			return LogicalType::INTEGER;
		case TeradataTypeId::BIGINT:
			return LogicalType::BIGINT;
		case TeradataTypeId::BLOB:
			return LogicalType::BLOB;
	    case TeradataTypeId::VARCHAR:
		case TeradataTypeId::DATE_T:
		case TeradataTypeId::DATE_A:
	    	return LogicalType::VARCHAR;
		case TeradataTypeId::CHAR:
	    	return LogicalType::VARCHAR; // This is the closes thing we got
	    case TeradataTypeId::FLOAT:
		    return LogicalType::FLOAT;
		case TeradataTypeId::BYTE:
		case TeradataTypeId::VARBYTE:
			return LogicalType::BLOB;
		case TeradataTypeId::TIMESTAMP:
			return LogicalType::TIMESTAMP;
		case TeradataTypeId::DATE:
            return LogicalType::DATE;
		// TODO: Implement the rest
		case TeradataTypeId::DECIMAL:
			return LogicalType::DECIMAL(precision, scale);
		case TeradataTypeId::BLOB_AS_DEFERRED:
		case TeradataTypeId::BLOB_AS_LOCATOR:
		case TeradataTypeId::CLOB:
		case TeradataTypeId::CLOB_AS_DEFERRED:
		case TeradataTypeId::CLOB_AS_LOCATOR:
		case TeradataTypeId::LONGVARCHAR:
		case TeradataTypeId::VARGRAPHIC:
		case TeradataTypeId::GRAPHIC:
		case TeradataTypeId::LONGVARGRAPHIC:
	    case TeradataTypeId::LONGVARBYTE:
	    case TeradataTypeId::PERIOD_DATE:
	    case TeradataTypeId::PERIOD_TIME:
		case TeradataTypeId::PERIOD_TIME_TZ:
	    case TeradataTypeId::PERIOD_TIMESTAMP:
	    case TeradataTypeId::PERIOD_TIMESTAMP_TZ:
		case TeradataTypeId::XML_CHAR:
	    case TeradataTypeId::XML_CHAR_DEFERRED:
	    case TeradataTypeId::XML_CHAR_LOCATOR:
	    case TeradataTypeId::XML_BINARY:
	    case TeradataTypeId::XML_BINARY_DEFERRED:
	    case TeradataTypeId::XML_BINARY_LOCATOR:
		case TeradataTypeId::INVALID:
			throw NotImplementedException("Unsupported Teradata Type: '%s'", ToString());
		default:
			D_ASSERT(false);
			throw InvalidInputException("Invalid Teradata Type");
	}
}

inline string TeradataType::ToString() const {
	switch(id) {
	case TeradataTypeId::BYTE: return "BYTE";
	case TeradataTypeId::SMALLINT: return "SMALLINT";
	case TeradataTypeId::INTEGER: return "INTEGER";
	case TeradataTypeId::BIGINT: return "BIGINT";
	case TeradataTypeId::BLOB: return "BLOB";
	case TeradataTypeId::VARCHAR: return "VARCHAR";
	case TeradataTypeId::CHAR: return "CHAR";
	case TeradataTypeId::FLOAT: return "FLOAT";
	case TeradataTypeId::INVALID: return "INVALID";
	case TeradataTypeId::BLOB_AS_DEFERRED: return "BLOB_AS_DEFERRED";
	case TeradataTypeId::BLOB_AS_LOCATOR: return "BLOB_AS_LOCATOR";
	case TeradataTypeId::CLOB: return "CLOB";
	case TeradataTypeId::CLOB_AS_DEFERRED: return "CLOB_AS_DEFERRED";
	case TeradataTypeId::CLOB_AS_LOCATOR: return "CLOB_AS_LOCATOR";
	case TeradataTypeId::LONGVARCHAR: return "LONGVARCHAR";
	case TeradataTypeId::VARGRAPHIC: return "VARGRAPHIC";
	case TeradataTypeId::GRAPHIC: return "GRAPHIC";
	case TeradataTypeId::LONGVARGRAPHIC: return "LONGVARGRAPHIC";
	case TeradataTypeId::DECIMAL: return "DECIMAL";
	case TeradataTypeId::VARBYTE: return "VARBYTE";
	case TeradataTypeId::LONGVARBYTE: return "LONGVARBYTE";
	case TeradataTypeId::DATE_A: return "DATE_A";
	case TeradataTypeId::DATE_T: return "DATE_T";
	case TeradataTypeId::BYTEINT: return "BYTEINT";
	case TeradataTypeId::PERIOD_DATE: return "PERIOD_DATE";
	case TeradataTypeId::PERIOD_TIME: return "PERIOD_TIME";
	case TeradataTypeId::PERIOD_TIME_TZ: return "PERIOD_TIME_TZ";
	case TeradataTypeId::PERIOD_TIMESTAMP: return "PERIOD_TIMESTAMP";
	case TeradataTypeId::PERIOD_TIMESTAMP_TZ: return "PERIOD_TIMESTAMP_TZ";
	case TeradataTypeId::XML_CHAR: return "XML_CHAR";
	case TeradataTypeId::XML_CHAR_DEFERRED: return "XML_CHAR_DEFERRED";
	case TeradataTypeId::XML_CHAR_LOCATOR: return "XML_CHAR_LOCATOR";
	case TeradataTypeId::XML_BINARY: return "XML_BINARY";
	case TeradataTypeId::XML_BINARY_DEFERRED: return "XML_BINARY_DEFERRED";
	case TeradataTypeId::XML_BINARY_LOCATOR: return "XML_BINARY_LOCATOR";
	case TeradataTypeId::TIMESTAMP: return "TIMESTAMP";
	case TeradataTypeId::DATE: return "DATE";
	default:
		D_ASSERT(false);
		return "UNKNOWN";
	}
}

inline TeradataTypeId TeradataType::FromShortCode(const char *code) {
	auto entry = short_codes.find(code);
    if (entry == short_codes.end()) {
        return TeradataTypeId::INVALID;
    }
    return entry->second;
}


*/
} // namespace duckdb