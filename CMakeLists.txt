cmake_minimum_required(VERSION 3.5)

# Set extension name here
set(TARGET_NAME teradata)

set(EXTENSION_NAME ${TARGET_NAME}_extension)
set(LOADABLE_EXTENSION_NAME ${TARGET_NAME}_loadable_extension)

# Set c++ standard to c++11
set(CMAKE_CXX_STANDARD 11)

project(${TARGET_NAME})

include_directories(src)
set(EXTENSION_SOURCES
    src/teradata_extension.cpp
    src/teradata_secret.cpp
    src/teradata_scan.cpp
    src/teradata_storage.cpp
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
    src/teradata_type.cpp
    src/teradata_execute.cpp)

# Teradata on macos
if(APPLE)
  set(TD_INCLUDE_DIR
      "/Library/Application\ Support/teradata/client/20.00/include")
  set(TD_LIBRARY_DIR "/Library/Application\ Support/teradata/client/20.00/lib")
endif()

# on linux (ubuntu has the lib64 suffix)
if(UNIX AND NOT APPLE)
  set(TD_INCLUDE_DIR "/opt/teradata/client/20.00/include")
  set(TD_LIBRARY_DIR "/opt/teradata/client/20.00/lib64")
endif()

find_library(
  TD_CLIV2
  NAMES libcliv2 libcliv2.so libcliv2.dylib
  PATHS ${TD_LIBRARY_DIR} REQUIRED)
find_library(
  TD_USR
  NAMES libtdusr libtdusr.so libtdusr.dylib
  PATHS ${TD_LIBRARY_DIR} REQUIRED)

build_static_extension(${TARGET_NAME} ${EXTENSION_SOURCES})
build_loadable_extension(${TARGET_NAME} " " ${EXTENSION_SOURCES})

# Suppress warnings from Teradata headers
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL
                                             "AppleClang")
  target_compile_options(${LOADABLE_EXTENSION_NAME}
                         PRIVATE -Wno-deprecated-register)
  target_compile_options(${EXTENSION_NAME} PRIVATE -Wno-deprecated-register)
endif()

target_include_directories(${LOADABLE_EXTENSION_NAME} PRIVATE include
                                                              ${TD_INCLUDE_DIR})
target_include_directories(${EXTENSION_NAME} PRIVATE include ${TD_INCLUDE_DIR})

target_link_libraries(${LOADABLE_EXTENSION_NAME} ${TD_CLIV2} ${TD_USR})
target_link_libraries(${EXTENSION_NAME} ${TD_CLIV2} ${TD_USR})

# This is garbage file(COPY ${TD_CLIV2} DESTINATION "${CMAKE_BINARY_DIR}")
# file(COPY ${TD_USR} DESTINATION "${CMAKE_BINARY_DIR}")
file(COPY ${TD_CLIV2} DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}")
file(COPY ${TD_USR} DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}")

install(
  TARGETS ${EXTENSION_NAME}
  EXPORT "${DUCKDB_EXPORT_SET}"
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}")
