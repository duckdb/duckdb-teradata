#pragma once

#include "duckdb.hpp"

namespace duckdb {

class TeradataColumnWriter {
public:
	virtual ~TeradataColumnWriter() = default;
	void Init(Vector &vec, idx_t count);
	void SetPresenceBits(idx_t count, idx_t col_idx, char *records[]) const;

	virtual void ComputeSizes(idx_t count, int32_t lengths[]) = 0;
	virtual void EncodeVector(idx_t count, char *records[]) = 0;

	static unique_ptr<TeradataColumnWriter> Make(const LogicalType &type);

protected:
	UnifiedVectorFormat format;
};

inline void TeradataColumnWriter::Init(Vector &vec, idx_t count) {
	vec.ToUnifiedFormat(count, format);
}

} // namespace duckdb