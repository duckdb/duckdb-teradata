# This file is included by DuckDB's build system. It specifies which extension to load

# Extension from this repo
duckdb_extension_load(teradata
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}
    INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/src
    LOAD_TESTS
)

# Any extra extensions that should be built
# e.g.: duckdb_extension_load(json)
