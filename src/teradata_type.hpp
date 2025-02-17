#pragma once
#include "teradata_common.hpp"
#include "duckdb/common/types.hpp"

namespace duckdb {

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
		// TODO: Implement the rest
		case TeradataTypeId::DECIMAL:
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
			throw NotImplementedException("Unsupported Teradata Type: '%s'", ToString());
		case TeradataTypeId::INVALID:
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
	default:
		D_ASSERT(false);
		return "UNKNOWN";
	}
}


} // namespace duckdb