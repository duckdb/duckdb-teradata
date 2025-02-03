#pragma once

#include "duckdb/storage/storage_extension.hpp"

namespace duckdb {

class TeradataStorageExtension final : public StorageExtension {
public:
	TeradataStorageExtension();
};

} // namespace duckdb