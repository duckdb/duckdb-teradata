#include "teradata_type.hpp"

namespace duckdb {

unordered_map<string, TeradataTypeId> TeradataType::short_codes = {
 { "++", TeradataTypeId::BLOB },		// TD_ANYTYPE
 { "A1", TeradataTypeId::BLOB },		// UDT
 { "AT", TeradataTypeId::BLOB },		// TIME
 { "BF", TeradataTypeId::BYTE },
 { "BO", TeradataTypeId::BLOB },		// BINARY LARGE OBJECT
 { "BV", TeradataTypeId::VARBYTE },
 { "CF", TeradataTypeId::CHAR },
 { "CO", TeradataTypeId::CLOB },
 { "CV", TeradataTypeId::VARCHAR },
 { "D ", TeradataTypeId::DECIMAL},
 { "DA", TeradataTypeId::DATE},	// DATE
 { "DH", TeradataTypeId::INVALID},	// "INTERVAL DAY TO HOUR"
 { "DM", TeradataTypeId::INVALID},	// "INTERVAL DAY TO MINUTE"
 { "DS", TeradataTypeId::INVALID},	// "INTERVAL DAY TO SECOND"
 { "DY", TeradataTypeId::INVALID},	// "INTERVAL DAY"
 { "F ", TeradataTypeId::FLOAT},
 { "HM", TeradataTypeId::INVALID},	// "INTERVAL HOUR TO MINUTE"
 { "HR", TeradataTypeId::INVALID},	// "INTERVAL HOUR"
 { "HS", TeradataTypeId::INVALID},	// "INTERVAL HOUR TO SECOND"
 { "I1", TeradataTypeId::BYTEINT},
 { "I2", TeradataTypeId::SMALLINT},
 { "I8", TeradataTypeId::BIGINT},
 { "I ", TeradataTypeId::INTEGER},
 { "MI", TeradataTypeId::INVALID},	// "INTERVAL MINUTE"
 { "MO", TeradataTypeId::INVALID},	// "INTERVAL MONTH"
 { "MS", TeradataTypeId::INVALID},	// "INTERVAL MINUTE TO SECOND"
 { "N ", TeradataTypeId::INVALID},		// NUMBER
 { "PD", TeradataTypeId::INVALID },	// "PERIOD(DATE)"
 { "PM", TeradataTypeId::INVALID },	// "PERIOD(TIMESTAMP WITH TIME ZONE)",
 { "PS", TeradataTypeId::INVALID},	// "PERIOD(TIMESTAMP)"},
 { "PT", TeradataTypeId::INVALID},	// "PERIOD(TIME)"},
 { "PZ", TeradataTypeId::INVALID},	// "PERIOD(TIME WITH TIME ZONE)"},
 { "SC", TeradataTypeId::INVALID},	// "INTERVAL SECOND"},
 { "SZ", TeradataTypeId::INVALID},	// "TIMESTAMP WITH TIME ZONE"},
 { "TS", TeradataTypeId::TIMESTAMP},	// "TIMESTAMP"},
 { "TZ", TeradataTypeId::INVALID},	// "TIME WITH TIME ZONE"},
 { "UT", TeradataTypeId::INVALID},	// "UDT"},
 { "YM", TeradataTypeId::INVALID},	// "INTERVAL YEAR TO MONTH"},
 { "YR", TeradataTypeId::INVALID},	// "INTERVAL YEAR"},
 { "AN", TeradataTypeId::INVALID},	// "UDT"},
 { "XM", TeradataTypeId::INVALID},	// "XML"},
 { "JN", TeradataTypeId::INVALID},	// "JSON"},
 { "DT", TeradataTypeId::INVALID},	// "DATASET"},
 { "??", TeradataTypeId::INVALID },	// "STGEOMETRY"}
};


} // namespace duckdb