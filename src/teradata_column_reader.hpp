#pragma once

#include "duckdb.hpp"

namespace duckdb {

//----------------------------------------------------------------------------------------------------------------------
// Teradata Converter
//-------------------------------------------------------------------------------------------------∂---------------------

class BinaryReader;
class TeradataType;

class TeradataColumnReader {
public:
	virtual ~TeradataColumnReader() = default;
	virtual void Decode(BinaryReader &reader, Vector &vec, idx_t row_idx, bool is_null) = 0;

	// Construct a reader from a Teradata type
	static unique_ptr<TeradataColumnReader> Make(const TeradataType &type);
};

}