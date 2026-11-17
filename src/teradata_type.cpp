#include "teradata_type.hpp"

#include <algorithm>

namespace duckdb {

namespace {

template <class T, std::size_t N, class PRED>
const T *FindEntry(const T (&table)[N], PRED &&pred) {
	const auto entry = std::find_if(std::begin(table), std::end(table), pred);
	return entry == std::end(table) ? nullptr : entry;
}

//----------------------------------------------------------------------------------------------------------------------
// Teradata type names
//----------------------------------------------------------------------------------------------------------------------
enum class TypeNameFormat : uint8_t {
	NAME,             // DATE
	NAME_WIDTH,       // VARCHAR(10)
	NAME_WIDTH_SCALE, // DECIMAL(10, 2)
	NAME_WIDTH_TZ     // TIME(6) WITH TIME ZONE
};

struct TypeNameEntry {
	TeradataTypeId id;
	const char *name;
	TypeNameFormat format;
};

const TypeNameEntry TYPE_NAMES[] = {
    {TeradataTypeId::INVALID, "INVALID", TypeNameFormat::NAME},
    {TeradataTypeId::ARRAY, "ARRAY", TypeNameFormat::NAME},
    {TeradataTypeId::BYTE, "BYTE", TypeNameFormat::NAME_WIDTH},
    {TeradataTypeId::VARBYTE, "VARBYTE", TypeNameFormat::NAME_WIDTH},
    {TeradataTypeId::BLOB, "BLOB", TypeNameFormat::NAME_WIDTH},
    {TeradataTypeId::CHAR, "CHAR", TypeNameFormat::NAME_WIDTH},
    {TeradataTypeId::VARCHAR, "VARCHAR", TypeNameFormat::NAME_WIDTH},
    {TeradataTypeId::CLOB, "CLOB", TypeNameFormat::NAME_WIDTH},
    {TeradataTypeId::DATE, "DATE", TypeNameFormat::NAME},
    {TeradataTypeId::TIME, "TIME", TypeNameFormat::NAME_WIDTH},
    {TeradataTypeId::TIMESTAMP, "TIMESTAMP", TypeNameFormat::NAME_WIDTH},
    {TeradataTypeId::TIME_TZ, "TIME", TypeNameFormat::NAME_WIDTH_TZ},
    {TeradataTypeId::TIMESTAMP_TZ, "TIMESTAMP", TypeNameFormat::NAME_WIDTH_TZ},
    {TeradataTypeId::ST_GEOMETRY, "ST_GEOMETRY", TypeNameFormat::NAME},
    {TeradataTypeId::MBR, "MBR", TypeNameFormat::NAME},
    {TeradataTypeId::INTERVAL_YEAR, "INTERVAL_YEAR", TypeNameFormat::NAME},
    {TeradataTypeId::INTERVAL_YEAR_TO_MONTH, "INTERVAL_YEAR_TO_MONTH", TypeNameFormat::NAME},
    {TeradataTypeId::INTERVAL_MONTH, "INTERVAL_MONTH", TypeNameFormat::NAME},
    {TeradataTypeId::INTERVAL_DAY, "INTERVAL_DAY", TypeNameFormat::NAME},
    {TeradataTypeId::INTERVAL_DAY_TO_HOUR, "INTERVAL_DAY_TO_HOUR", TypeNameFormat::NAME},
    {TeradataTypeId::INTERVAL_DAY_TO_MINUTE, "INTERVAL_DAY_TO_MINUTE", TypeNameFormat::NAME},
    {TeradataTypeId::INTERVAL_DAY_TO_SECOND, "INTERVAL_DAY_TO_SECOND", TypeNameFormat::NAME},
    {TeradataTypeId::INTERVAL_HOUR, "INTERVAL_HOUR", TypeNameFormat::NAME},
    {TeradataTypeId::INTERVAL_HOUR_TO_MINUTE, "INTERVAL_HOUR_TO_MINUTE", TypeNameFormat::NAME},
    {TeradataTypeId::INTERVAL_HOUR_TO_SECOND, "INTERVAL_HOUR_TO_SECOND", TypeNameFormat::NAME},
    {TeradataTypeId::INTERVAL_MINUTE, "INTERVAL_MINUTE", TypeNameFormat::NAME},
    {TeradataTypeId::INTERVAL_MINUTE_TO_SECOND, "INTERVAL_MINUTE_TO_SECOND", TypeNameFormat::NAME},
    {TeradataTypeId::INTERVAL_SECOND, "INTERVAL_SECOND", TypeNameFormat::NAME},
    {TeradataTypeId::JSON, "JSON", TypeNameFormat::NAME},
    {TeradataTypeId::BYTEINT, "BYTEINT", TypeNameFormat::NAME},
    {TeradataTypeId::SMALLINT, "SMALLINT", TypeNameFormat::NAME},
    {TeradataTypeId::INTEGER, "INTEGER", TypeNameFormat::NAME},
    {TeradataTypeId::BIGINT, "BIGINT", TypeNameFormat::NAME},
    {TeradataTypeId::DECIMAL, "DECIMAL", TypeNameFormat::NAME_WIDTH_SCALE},
    {TeradataTypeId::FLOAT, "FLOAT", TypeNameFormat::NAME},
    {TeradataTypeId::NUMBER, "NUMBER", TypeNameFormat::NAME},
    {TeradataTypeId::ANYTYPE, "ANYTYPE", TypeNameFormat::NAME},
    {TeradataTypeId::VARIANT, "VARIANT", TypeNameFormat::NAME},
    {TeradataTypeId::PERIOD_DATE, "PERIOD_DATE", TypeNameFormat::NAME},
    {TeradataTypeId::PERIOD_TIME, "PERIOD_TIME", TypeNameFormat::NAME},
    {TeradataTypeId::PERIOD_TIME_TZ, "PERIOD_TIME_TZ", TypeNameFormat::NAME},
    {TeradataTypeId::PERIOD_TIMESTAMP, "PERIOD_TIMESTAMP", TypeNameFormat::NAME},
    {TeradataTypeId::PERIOD_TIMESTAMP_TZ, "PERIOD_TIMESTAMP_TZ", TypeNameFormat::NAME},
    {TeradataTypeId::UDT_DISTINCT, "UDT_DISTINCT", TypeNameFormat::NAME},
    {TeradataTypeId::UDT_STRUCTURED, "UDT_STRUCTURED", TypeNameFormat::NAME},
    {TeradataTypeId::XML, "XML", TypeNameFormat::NAME},
};

string FormatTypeName(const TypeNameEntry &entry, int64_t width, int64_t scale) {
	const string name = entry.name;
	switch (entry.format) {
	case TypeNameFormat::NAME_WIDTH:
		return name + "(" + to_string(width) + ")";
	case TypeNameFormat::NAME_WIDTH_SCALE:
		return name + "(" + to_string(width) + ", " + to_string(scale) + ")";
	case TypeNameFormat::NAME_WIDTH_TZ:
		return name + "(" + to_string(width) + ") WITH TIME ZONE";
	default:
		return name;
	}
}

//----------------------------------------------------------------------------------------------------------------------
// Teradata -> DuckDB
//----------------------------------------------------------------------------------------------------------------------
struct DuckDBTypeEntry {
	TeradataTypeId id;
	LogicalTypeId duck_id;
};

// Types that map to a DuckDB type without any parameters
const DuckDBTypeEntry SIMPLE_DUCKDB_TYPES[] = {
    {TeradataTypeId::INVALID, LogicalTypeId::INVALID},
    {TeradataTypeId::BYTE, LogicalTypeId::BLOB},
    {TeradataTypeId::VARBYTE, LogicalTypeId::BLOB},
    {TeradataTypeId::BLOB, LogicalTypeId::BLOB},
    {TeradataTypeId::CHAR, LogicalTypeId::VARCHAR},
    {TeradataTypeId::VARCHAR, LogicalTypeId::VARCHAR},
    {TeradataTypeId::CLOB, LogicalTypeId::VARCHAR},
    {TeradataTypeId::DATE, LogicalTypeId::DATE},
    {TeradataTypeId::TIME, LogicalTypeId::TIME},
    {TeradataTypeId::TIME_TZ, LogicalTypeId::TIME_TZ},
    {TeradataTypeId::TIMESTAMP_TZ, LogicalTypeId::TIMESTAMP_TZ},
    {TeradataTypeId::BYTEINT, LogicalTypeId::TINYINT},
    {TeradataTypeId::SMALLINT, LogicalTypeId::SMALLINT},
    {TeradataTypeId::INTEGER, LogicalTypeId::INTEGER},
    {TeradataTypeId::BIGINT, LogicalTypeId::BIGINT},
    // Teradata only supports 8-byte floats (REAL, DOUBLE PRECISION and FLOAT are all the same)
    {TeradataTypeId::FLOAT, LogicalTypeId::DOUBLE},
    {TeradataTypeId::INTERVAL_HOUR_TO_SECOND, LogicalTypeId::INTERVAL},
};

// Teradata timestamp precision (fractional digits 0-6), rounded up to the closest DuckDB timestamp type
const LogicalTypeId TIMESTAMP_BY_PRECISION[] = {
    LogicalTypeId::TIMESTAMP_SEC, LogicalTypeId::TIMESTAMP_MS, LogicalTypeId::TIMESTAMP_MS, LogicalTypeId::TIMESTAMP_MS,
    LogicalTypeId::TIMESTAMP,     LogicalTypeId::TIMESTAMP,    LogicalTypeId::TIMESTAMP,
};

LogicalType TimestampForPrecision(int64_t precision) {
	const auto count = static_cast<int64_t>(sizeof(TIMESTAMP_BY_PRECISION) / sizeof(TIMESTAMP_BY_PRECISION[0]));
	if (precision < 0 || precision >= count) {
		throw NotImplementedException("Unimplemented Teradata type");
	}
	return LogicalType(TIMESTAMP_BY_PRECISION[precision]);
}

//----------------------------------------------------------------------------------------------------------------------
// DuckDB -> Teradata
//----------------------------------------------------------------------------------------------------------------------
struct TeradataTypeEntry {
	LogicalTypeId duck_id;
	TeradataTypeId id;
	int64_t width;
};

// DuckDB types are variable size, so strings and blobs map to the maximum Teradata length.
// Temporal types carry their fractional second precision as width.
const TeradataTypeEntry TERADATA_TYPES[] = {
    // Booleans do not exist in Teradata, so map to byteint
    {LogicalTypeId::BOOLEAN, TeradataTypeId::BYTEINT, 0},
    {LogicalTypeId::TINYINT, TeradataTypeId::BYTEINT, 0},
    {LogicalTypeId::SMALLINT, TeradataTypeId::SMALLINT, 0},
    {LogicalTypeId::INTEGER, TeradataTypeId::INTEGER, 0},
    {LogicalTypeId::BIGINT, TeradataTypeId::BIGINT, 0},
    {LogicalTypeId::DOUBLE, TeradataTypeId::FLOAT, 0},
    {LogicalTypeId::TIMESTAMP_SEC, TeradataTypeId::TIMESTAMP, 0},
    {LogicalTypeId::TIMESTAMP_MS, TeradataTypeId::TIMESTAMP, 3},
    {LogicalTypeId::TIMESTAMP, TeradataTypeId::TIMESTAMP, 6},
    {LogicalTypeId::TIMESTAMP_TZ, TeradataTypeId::TIMESTAMP_TZ, 6},
    {LogicalTypeId::DATE, TeradataTypeId::DATE, 0},
    {LogicalTypeId::TIME, TeradataTypeId::TIME, 6},
    {LogicalTypeId::TIME_TZ, TeradataTypeId::TIME_TZ, 6},
    {LogicalTypeId::VARCHAR, TeradataTypeId::VARCHAR, TeradataType::MAX_TYPE_LENGTH},
    {LogicalTypeId::BLOB, TeradataTypeId::VARBYTE, TeradataType::MAX_TYPE_LENGTH},
};

const TeradataTypeId LENGTH_MODIFIED_TYPES[] = {
    TeradataTypeId::BYTE, TeradataTypeId::VARBYTE, TeradataTypeId::BLOB,
    TeradataTypeId::CHAR, TeradataTypeId::VARCHAR, TeradataTypeId::CLOB,
};

//----------------------------------------------------------------------------------------------------------------------
// Short codes (as used in the data dictionary, e.g. DBC.ColumnsV)
//----------------------------------------------------------------------------------------------------------------------
struct ShortCodeEntry {
	const char *code;
	TeradataTypeId id;
};

const ShortCodeEntry SHORT_CODES[] = {
    {"++", TeradataTypeId::ANYTYPE},
    {"A1", TeradataTypeId::INVALID}, // UDT
    {"AT", TeradataTypeId::TIME},
    {"BF", TeradataTypeId::BYTE},
    {"BO", TeradataTypeId::BLOB},
    {"BV", TeradataTypeId::VARBYTE},
    {"CF", TeradataTypeId::CHAR},
    {"CO", TeradataTypeId::CLOB},
    {"CV", TeradataTypeId::VARCHAR},
    {"D ", TeradataTypeId::DECIMAL},
    {"DA", TeradataTypeId::DATE},
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
    {"UT", TeradataTypeId::INVALID}, // UDT
    {"YM", TeradataTypeId::INTERVAL_YEAR_TO_MONTH},
    {"YR", TeradataTypeId::INTERVAL_YEAR},
    {"AN", TeradataTypeId::INVALID}, // UDT
    {"XM", TeradataTypeId::XML},
    {"JN", TeradataTypeId::JSON},
    {"DT", TeradataTypeId::INVALID}, // DATASET
    {"??", TeradataTypeId::ST_GEOMETRY},
};

//----------------------------------------------------------------------------------------------------------------------
// Parcel type codes
//----------------------------------------------------------------------------------------------------------------------
// Each type has a base code; the variants are encoded as an offset from it
struct ParcelVariantOffset {
	int16_t offset;
	TeradataColumnTypeVariant variant;
};

const ParcelVariantOffset PARCEL_VARIANTS[] = {
    {0, TeradataColumnTypeVariant::STANDARD},    {1, TeradataColumnTypeVariant::NULLABLE},
    {500, TeradataColumnTypeVariant::PARAM_IN},  {501, TeradataColumnTypeVariant::PARAM_INOUT},
    {502, TeradataColumnTypeVariant::PARAM_OUT},
};

struct ParcelBaseType {
	int16_t base_code;
	TeradataTypeId id;
	// Some types only exist in their standard variant
	bool has_variants;
};

const ParcelBaseType PARCEL_BASE_TYPES[] = {
    {400, TeradataTypeId::BLOB, false},          {448, TeradataTypeId::VARCHAR, true},
    {452, TeradataTypeId::CHAR, true},           {456, TeradataTypeId::VARCHAR, true}, // LONG VARCHAR
    {472, TeradataTypeId::LONGVARGRAPHIC, true}, {480, TeradataTypeId::FLOAT, true},
    {484, TeradataTypeId::DECIMAL, true},        {496, TeradataTypeId::INTEGER, true},
    {500, TeradataTypeId::SMALLINT, true},       {600, TeradataTypeId::BIGINT, true},
    {688, TeradataTypeId::VARBYTE, true},        {692, TeradataTypeId::BYTE, true},
    {697, TeradataTypeId::VARBYTE, true}, // LONG VARBYTE, treated as VARBYTE
    {748, TeradataTypeId::DATE, true},    // DATE_A (ANSI date)
    {752, TeradataTypeId::DATE, true},    // DATE_T (integer date)
    {756, TeradataTypeId::BYTEINT, true},
};

optional<TeradataColumnType> TryDecodeVariant(int16_t code, const ParcelVariantOffset &variant) {
	const auto base_code = static_cast<int16_t>(code - variant.offset);
	const auto base = FindEntry(PARCEL_BASE_TYPES, [&](const ParcelBaseType &entry) {
		return entry.base_code == base_code && (entry.has_variants || variant.offset == 0);
	});
	if (!base) {
		return {};
	}
	return TeradataColumnType {base->id, variant.variant};
}

} // namespace

//----------------------------------------------------------------------------------------------------------------------
// TeradataType
//----------------------------------------------------------------------------------------------------------------------
string TeradataType::ToString() const {
	const auto entry = FindEntry(TYPE_NAMES, [&](const TypeNameEntry &e) { return e.id == id; });
	if (!entry) {
		throw NotImplementedException("Unimplemented Teradata type");
	}
	return FormatTypeName(*entry, width, scale);
}

LogicalType TeradataType::ToDuckDB() const {
	switch (id) {
	case TeradataTypeId::TIMESTAMP:
		return TimestampForPrecision(width);
	case TeradataTypeId::DECIMAL:
		// TODO: Not correct.
		return LogicalType::DECIMAL(width, scale);
	case TeradataTypeId::JSON:
		return LogicalType::JSON();
	default:
		break;
	}
	const auto entry = FindEntry(SIMPLE_DUCKDB_TYPES, [&](const DuckDBTypeEntry &e) { return e.id == id; });
	if (!entry) {
		throw NotImplementedException("Unimplemented Teradata type");
	}
	return LogicalType(entry->duck_id);
}

TeradataType TeradataType::FromDuckDB(const LogicalType &type) {
	if (type.IsJSONType()) {
		return TeradataTypeId::JSON;
	}
	if (type.id() == LogicalTypeId::DECIMAL) {
		return TeradataType(TeradataTypeId::DECIMAL, DecimalType::GetWidth(type), DecimalType::GetScale(type));
	}
	const auto entry = FindEntry(TERADATA_TYPES, [&](const TeradataTypeEntry &e) { return e.duck_id == type.id(); });
	if (!entry) {
		throw NotImplementedException("Cannot convert DuckDB type '%s' to Teradata type", type.ToString());
	}
	return TeradataType(entry->id, entry->width);
}

bool TeradataType::HasLengthModifier() const {
	return std::any_of(std::begin(LENGTH_MODIFIED_TYPES), std::end(LENGTH_MODIFIED_TYPES),
	                   [&](TeradataTypeId length_id) { return length_id == id; });
}

TeradataTypeId TeradataType::FromShortCode(const string &code) {
	const auto entry = FindEntry(SHORT_CODES, [&](const ShortCodeEntry &e) { return code == e.code; });
	return entry ? entry->id : TeradataTypeId::INVALID;
}

TeradataColumnType DecodeParcelType(int16_t code) {
	for (const auto &variant : PARCEL_VARIANTS) {
		auto result = TryDecodeVariant(code, variant);
		if (result) {
			return *result;
		}
	}
	// TODO: More types
	throw NotImplementedException("Unknown data type: %d", code);
}

} // namespace duckdb
