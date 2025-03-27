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
	FLOAT,   // Alias: DOUBLE, REAL
	NUMBER,  // Distinct from DECIMAL

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
	// NOLINTNEXTLINE: Allow implicit conversion from `TeradataTypeId`
	TeradataType(TeradataTypeId id) : id(id) {
	}

	TeradataTypeId GetId() const {
		return id;
	}
	int64_t GetLength() const {
		return width;
	}
	int64_t GetPrecision() const {
		return scale;
	}
	int64_t GetScale() const {
		return scale;
	}

	void SetLength(const int64_t length_p) {
		width = length_p;
	}
	void SetPrecision(const int64_t precision_p) {
		width = precision_p;
	}
	void SetScale(const int64_t scale_p) {
		scale = scale_p;
	}
	bool HasLengthModifier() const;

	string ToString() const;
	LogicalType ToDuckDB() const;

	static TeradataTypeId FromShortCode(const string &code) {
		const auto entry = code_map.find(code);
		if (entry == code_map.end()) {
			return TeradataTypeId::INVALID;
		}
		return entry->second;
	}

	static TeradataType FromDuckDB(const LogicalType &type);

	bool IsDecimal() const {
		return id == TeradataTypeId::DECIMAL;
	}

	bool operator==(const TeradataType &rhs) const {
		return id == rhs.id && width == rhs.width && scale == rhs.scale;
	}
	bool operator!=(const TeradataType &rhs) const {
		return !(*this == rhs);
	}

private:
	TeradataTypeId id = TeradataTypeId::INVALID;
	int64_t width = 0;
	int64_t scale = 0;

	// From eg. "BF" to TeradataTypeId::BYTE
	static unordered_map<string, TeradataTypeId> code_map;

	static constexpr auto MAX_TYPE_LENGTH = 64000;
};

} // namespace duckdb