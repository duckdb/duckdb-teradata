cmake_minimum_required(VERSION 3.5)

# Set extension name here
set(TARGET_NAME teradata)

set(EXTENSION_NAME ${TARGET_NAME}_extension)
set(LOADABLE_EXTENSION_NAME ${TARGET_NAME}_loadable_extension)

project(${TARGET_NAME})

include_directories(src)
set(EXTENSION_SOURCES
    src/teradata_common.cpp
    src/teradata_extension.cpp
    src/teradata_secret.cpp
    src/teradata_storage.cpp
    src/teradata_logon.cpp
    src/teradata_catalog.cpp
    src/teradata_connection.cpp
    src/teradata_transaction_manager.cpp
    src/teradata_transaction.cpp
    src/teradata_query.cpp
    src/teradata_insert.cpp
    src/teradata_request.cpp
    src/teradata_schema_entry.cpp
    src/teradata_schema_set.cpp
    src/teradata_catalog_set.cpp
    src/teradata_table_set.cpp
    src/teradata_table_entry.cpp
    src/teradata_index.cpp
    src/teradata_index_set.cpp
    src/teradata_index_entry.cpp
    src/teradata_type.cpp
    src/teradata_execute.cpp
    src/teradata_clear_cache.cpp
    src/teradata_filter.cpp
    src/teradata_column_reader.cpp
    src/teradata_column_writer.cpp
    src/teradata_delete_update.cpp)

# Teradata on macos if(APPLE) set(TD_INCLUDE_DIR "/Library/Application\
# Support/teradata/client/20.00/include") endif()

# on linux (ubuntu has the lib64 suffix) if(UNIX AND NOT APPLE)
# set(TD_INCLUDE_DIR "/opt/teradata/client/20.00/include") endif()

# on windows if (WIN32) set(TD_INCLUDE_DIR "C:/Program
# Files/teradata/client/20.00/include") endif ()

set(TD_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/teradata")

build_static_extension(${TARGET_NAME} ${EXTENSION_SOURCES})
build_loadable_extension(${TARGET_NAME} " " ${EXTENSION_SOURCES})

# Suppress warnings from Teradata headers
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL
                                             "AppleClang")
  target_compile_options(${LOADABLE_EXTENSION_NAME}
                         PRIVATE -Wno-deprecated-register -Wno-register)
  target_compile_options(${EXTENSION_NAME} PRIVATE -Wno-deprecated-register
                                                   -Wno-register)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # The Teradata headers use the 'register' keyword, which C++17 disallows
  target_compile_options(${LOADABLE_EXTENSION_NAME} PRIVATE -Wno-register)
  target_compile_options(${EXTENSION_NAME} PRIVATE -Wno-register)
endif()

target_include_directories(${LOADABLE_EXTENSION_NAME} PRIVATE include
                                                              ${TD_INCLUDE_DIR})
target_include_directories(${EXTENSION_NAME} PRIVATE include ${TD_INCLUDE_DIR})

install(
  TARGETS ${EXTENSION_NAME}
  EXPORT "${DUCKDB_EXPORT_SET}"
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}")

# C++ unit tests, run with an in-process fake of the Teradata CLIv2 library (no Teradata system needed).
# Enable with -DTERADATA_UNITTESTS=1, e.g. `make unittest`.
option(TERADATA_UNITTESTS "Build the Teradata extension C++ unit tests" OFF)
if(TERADATA_UNITTESTS)
  add_executable(
    teradata_unittest
    test/cpp/test_main.cpp
    test/cpp/fake_cliv2.cpp
    test/cpp/test_types.cpp
    test/cpp/test_column_io.cpp
    test/cpp/test_filter.cpp
    test/cpp/test_sql.cpp
    test/cpp/test_integration.cpp)
  target_include_directories(
    teradata_unittest PRIVATE src ${TD_INCLUDE_DIR} test/cpp
                              ${CMAKE_SOURCE_DIR}/third_party/catch)
  target_link_libraries(teradata_unittest ${EXTENSION_NAME} duckdb_static)
  link_extension_libraries(teradata_unittest "")
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL
                                               "AppleClang")
    target_compile_options(teradata_unittest PRIVATE -Wno-deprecated-register
                                                     -Wno-register)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(teradata_unittest PRIVATE -Wno-register)
  endif()
endif()
