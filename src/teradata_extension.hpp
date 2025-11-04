#pragma once

#include "duckdb.hpp"

namespace duckdb {

class TeradataExtension final : public Extension {
public:
	void Load(ExtensionLoader &loader) override;
	std::string Name() override;
};

} // namespace duckdb
