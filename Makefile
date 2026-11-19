PROJ_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# Configuration of extension
EXT_NAME=teradata
EXT_CONFIG=${PROJ_DIR}extension_config.cmake

.PHONY: configure_ci

# Include the Makefile from extension-ci-tools
include extension-ci-tools/makefiles/duckdb_extension.Makefile

# C++ unit tests against an in-process fake of Teradata CLIv2: no Teradata system needed
.PHONY: unittest
unittest:
	$(MAKE) release EXT_FLAGS="$(EXT_FLAGS) -DTERADATA_UNITTESTS=1"
	./build/release/extension/teradata/teradata_unittest
