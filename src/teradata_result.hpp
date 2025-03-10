#pragma once

#include "teradata_type.hpp"
#include "teradata_request.hpp"

namespace duckdb {

class TeradataQueryResult;

//----------------------------------------------------------------------------------------------------------------------
// Query Result Iterator
//----------------------------------------------------------------------------------------------------------------------
class TeradataQueryResultIterator {
public:
	TeradataQueryResultIterator(TeradataQueryResult *result_p);
	TeradataQueryResultIterator &operator++();
	bool operator!=(const TeradataQueryResultIterator &other) const;
	DataChunk &operator*() const;
	void Next();
private:
	TeradataQueryResult *result;
	shared_ptr<DataChunk> scan_chunk;
	idx_t row_index;
};

class TeradataQueryResultIteratorPair {
public:
	explicit TeradataQueryResultIteratorPair(TeradataQueryResult &result_p)
		: result(result_p) { }

	TeradataQueryResultIterator begin() { return TeradataQueryResultIterator(&result); }
	TeradataQueryResultIterator end() { return TeradataQueryResultIterator(nullptr); }
private:
	TeradataQueryResult &result;
};

//----------------------------------------------------------------------------------------------------------------------
// Query Result
//----------------------------------------------------------------------------------------------------------------------
class TeradataQueryResult {
public:
	virtual ~TeradataQueryResult() = default;
	// Not copyable or movable
	TeradataQueryResult(const TeradataQueryResult &) = delete;
	TeradataQueryResult &operator=(const TeradataQueryResult &) = delete;
	TeradataQueryResult(TeradataQueryResult &&) = delete;
	TeradataQueryResult &operator=(TeradataQueryResult &&) = delete;

	// Is this result materialized?
	virtual bool IsMaterialized() const = 0;

	// Get the types of the result set
	const vector<TeradataType> &GetTypes() const { return types; }

	// Consumes this result, returning the next chunk of data
	// Returns false if there is no more data to fetch
	virtual bool Scan(DataChunk &chunk) = 0;

	void InitScanChunk(DataChunk &chunk) const {
		vector<LogicalType> duck_types;
		for(const auto &td_type : types) {
			duck_types.push_back(td_type.ToDuckDB());
		}
		chunk.Initialize(Allocator::DefaultAllocator(), duck_types);
	}

	TeradataQueryResultIteratorPair Chunks() { return TeradataQueryResultIteratorPair(*this); }

protected:
	explicit TeradataQueryResult(vector<TeradataType> types_p) : types(std::move(types_p)) { }

	vector<TeradataType> types;
};


class StreamingTeradataQueryResult final : public TeradataQueryResult {
public:

	StreamingTeradataQueryResult(vector<TeradataType> types_p, unique_ptr<TeradataRequestContext> ctx_p)
		: TeradataQueryResult(std::move(types_p)),
			ctx(std::move(ctx_p)) { }

	bool IsMaterialized() const override { return false; }

	bool Scan(DataChunk &chunk) override {
		return ctx->Fetch(chunk, types);
	}
private:
	unique_ptr<TeradataRequestContext> ctx;
};

class MaterializedTeradataQueryResult final : public TeradataQueryResult {
public:

	MaterializedTeradataQueryResult(vector<TeradataType> types_p, unique_ptr<ColumnDataCollection> cdc_p)
		: TeradataQueryResult(std::move(types_p)),
			cdc(std::move(cdc_p)) {

		// Initialize scan state
		cdc->InitializeScan(scan_state);
	}

	bool IsMaterialized() const override { return true; }

	bool Scan(DataChunk &chunk) override {
		return cdc->Scan(scan_state, chunk);
	}
private:
	unique_ptr<ColumnDataCollection> cdc;
	ColumnDataScanState scan_state;
};

//----------------------------------------------------------------------------------------------------------------------
// Inlined methods
//----------------------------------------------------------------------------------------------------------------------
inline TeradataQueryResultIterator::TeradataQueryResultIterator(TeradataQueryResult *result_p)
		: result(result_p), row_index(0) {

	if(!result) {
		 return;
	}

	scan_chunk = make_shared_ptr<DataChunk>();
	result->InitScanChunk(*scan_chunk);
	result->Scan(*scan_chunk);
	row_index += scan_chunk->size();
}

inline DataChunk &TeradataQueryResultIterator::operator*() const {
	return *scan_chunk;
}

inline void TeradataQueryResultIterator::Next() {
	if(!result) {
		return;
	}
    if(!result->Scan(*scan_chunk)) {
        result = nullptr;
        row_index = 0;
    } else {
	    row_index += scan_chunk->size();
    }
}

inline bool TeradataQueryResultIterator::operator!=(const TeradataQueryResultIterator &other) const {
	return result != other.result || row_index != other.row_index;
}

inline TeradataQueryResultIterator &TeradataQueryResultIterator::operator++() {
	Next();
	return *this;
}

} // namespace duckdb