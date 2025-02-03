#pragma once
#include "teradata_common.hpp"
#include "duckdb/common/types.hpp"


namespace duckdb {

//----------------------------------------------------------------------------------------------------------------------
// Teradata Type
//----------------------------------------------------------------------------------------------------------------------
enum class TypeVariant { STANDARD, NULLABLE, PARAM_IN, PARAM_INOUT, PARAM_OUT };

struct TeradataColumnType {
	const char *name;
	LogicalType type;
	TypeVariant variant;

	static TeradataColumnType Get(PclInt16 type);
};

inline TeradataColumnType TeradataColumnType::Get(PclInt16 type) {
	// Nullable is 1 + the standard type
	// Param in is 500 + the standard type
	// Param inout is 501 + the standard type
	// Param out is 502 + the standard type
	switch (type) {
	case 472:
		return {"LONGVARGRAPHIC", LogicalType::INVALID, TypeVariant::STANDARD};
	case 473:
		return {"LONGVARGRAPHIC", LogicalType::INVALID, TypeVariant::NULLABLE};
	case 972:
		return {"LONGVARGRAPHIC", LogicalType::INVALID, TypeVariant::PARAM_IN};
	case 973:
		return {"LONGVARGRAPHIC", LogicalType::INVALID, TypeVariant::PARAM_INOUT};
	case 974:
		return {"LONGVARGRAPHIC", LogicalType::INVALID, TypeVariant::PARAM_OUT};

	case 480:
		return {"FLOAT", LogicalType::FLOAT, TypeVariant::STANDARD};
	case 481:
		return {"FLOAT", LogicalType::FLOAT, TypeVariant::NULLABLE};
	case 980:
		return {"FLOAT", LogicalType::FLOAT, TypeVariant::PARAM_IN};
	case 981:
		return {"FLOAT", LogicalType::FLOAT, TypeVariant::PARAM_INOUT};
	case 982:
		return {"FLOAT", LogicalType::FLOAT, TypeVariant::PARAM_OUT};

		/*
		case 484: return { "DECIMAL", LogicalType::DECIMAL, TypeVariant::STANDARD };
		case 485: return { "DECIMAL", LogicalType::DECIMAL, TypeVariant::NULLABLE };
		case 984: return { "DECIMAL", LogicalType::DECIMAL, TypeVariant::PARAM_IN };
		case 985: return { "DECIMAL", LogicalType::DECIMAL, TypeVariant::PARAM_INOUT };
		case 986: return { "DECIMAL", LogicalType::DECIMAL, TypeVariant::PARAM_OUT };
		*/

	case 496:
		return {"INTEGER", LogicalType::INTEGER, TypeVariant::STANDARD};
	case 497:
		return {"INTEGER", LogicalType::INTEGER, TypeVariant::NULLABLE};
	case 996:
		return {"INTEGER", LogicalType::INTEGER, TypeVariant::PARAM_IN};
	case 997:
		return {"INTEGER", LogicalType::INTEGER, TypeVariant::PARAM_INOUT};
	case 998:
		return {"INTEGER", LogicalType::INTEGER, TypeVariant::PARAM_OUT};

	case 500:
		return {"SMALLINT", LogicalType::SMALLINT, TypeVariant::STANDARD};
	case 501:
		return {"SMALLINT", LogicalType::SMALLINT, TypeVariant::NULLABLE};
	case 1000:
		return {"SMALLINT", LogicalType::SMALLINT, TypeVariant::PARAM_IN};
	case 1001:
		return {"SMALLINT", LogicalType::SMALLINT, TypeVariant::PARAM_INOUT};
	case 1002:
		return {"SMALLINT", LogicalType::SMALLINT, TypeVariant::PARAM_OUT};

	case 600:
		return {"BIGINT", LogicalType::BIGINT, TypeVariant::STANDARD};
	case 601:
		return {"BIGINT", LogicalType::BIGINT, TypeVariant::NULLABLE};
	case 1100:
		return {"BIGINT", LogicalType::BIGINT, TypeVariant::PARAM_IN};
	case 1101:
		return {"BIGINT", LogicalType::BIGINT, TypeVariant::PARAM_INOUT};
	case 1102:
		return {"BIGINT", LogicalType::BIGINT, TypeVariant::PARAM_OUT};

	case 688:
		return {"VARBYTE", LogicalType::BLOB, TypeVariant::STANDARD};
	case 689:
		return {"VARBYTE", LogicalType::BLOB, TypeVariant::NULLABLE};
	case 1188:
		return {"VARBYTE", LogicalType::BLOB, TypeVariant::PARAM_IN};
	case 1189:
		return {"VARBYTE", LogicalType::BLOB, TypeVariant::PARAM_INOUT};
	case 1190:
		return {"VARBYTE", LogicalType::BLOB, TypeVariant::PARAM_OUT};

	case 692:
		return {"BYTE", LogicalType::UTINYINT, TypeVariant::STANDARD};
	case 693:
		return {"BYTE", LogicalType::UTINYINT, TypeVariant::NULLABLE};
	case 1192:
		return {"BYTE", LogicalType::UTINYINT, TypeVariant::PARAM_IN};
	case 1193:
		return {"BYTE", LogicalType::UTINYINT, TypeVariant::PARAM_INOUT};
	case 1194:
		return {"BYTE", LogicalType::UTINYINT, TypeVariant::PARAM_OUT};

	case 697:
		return {"LONGVARBYTE", LogicalType::BLOB, TypeVariant::STANDARD};
	case 698:
		return {"LONGVARBYTE", LogicalType::BLOB, TypeVariant::NULLABLE};
	case 1197:
		return {"LONGVARBYTE", LogicalType::BLOB, TypeVariant::PARAM_IN};
	case 1198:
		return {"LONGVARBYTE", LogicalType::BLOB, TypeVariant::PARAM_INOUT};
	case 1199:
		return {"LONGVARBYTE", LogicalType::BLOB, TypeVariant::PARAM_OUT};

	case 748:
		return {"DATE (A)", LogicalType::INVALID, TypeVariant::STANDARD};
	case 749:
		return {"DATE (A)", LogicalType::INVALID, TypeVariant::NULLABLE};
	case 1248:
		return {"DATE (A)", LogicalType::INVALID, TypeVariant::PARAM_IN};
	case 1249:
		return {"DATE (A)", LogicalType::INVALID, TypeVariant::PARAM_INOUT};
	case 1250:
		return {"DATE (A)", LogicalType::INVALID, TypeVariant::PARAM_OUT};

	case 752:
		return {"DATE (B)", LogicalType::INVALID, TypeVariant::STANDARD};
	case 753:
		return {"DATE (B)", LogicalType::INVALID, TypeVariant::NULLABLE};
	case 1252:
		return {"DATE (B)", LogicalType::INVALID, TypeVariant::PARAM_IN};
	case 1253:
		return {"DATE (B)", LogicalType::INVALID, TypeVariant::PARAM_INOUT};
	case 1254:
		return {"DATE (B)", LogicalType::INVALID, TypeVariant::PARAM_OUT};

	case 756:
		return {"BYTEINT", LogicalType::TINYINT, TypeVariant::STANDARD};
	case 757:
		return {"BYTEINT", LogicalType::TINYINT, TypeVariant::NULLABLE};
	case 1256:
		return {"BYTEINT", LogicalType::TINYINT, TypeVariant::PARAM_IN};
	case 1257:
		return {"BYTEINT", LogicalType::TINYINT, TypeVariant::PARAM_INOUT};
	case 1258:
		return {"BYTEINT", LogicalType::TINYINT, TypeVariant::PARAM_OUT};

	default:
		// TODO: More types
		throw NotImplementedException("Unknown data type: %d", type);
	}
}

}