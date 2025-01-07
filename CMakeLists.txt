cmake_minimum_required(VERSION 3.5)

# Set extension name here
set(TARGET_NAME teradata)

set(EXTENSION_NAME ${TARGET_NAME}_extension)
set(LOADABLE_EXTENSION_NAME ${TARGET_NAME}_loadable_extension)

project(${TARGET_NAME})

include_directories(src/include)
set(EXTENSION_SOURCES
        src/teradata_extension.cpp
        src/teradata_secret.cpp
        src/teradata_scan.cpp
)

# Teradata
set(TD_CLIV2_LIB /Library/Application\ Support/teradata/client/20.00/lib/libcliv2.dylib)
set(TD_USR_LIB /Library/Application\ Support/teradata/client/20.00/lib/libtdusr.dylib)
set(TD_INCLUDE_DIR /Library/Application\ Support/teradata/client/20.00/include)

# build_static_extension(${TARGET_NAME} ${EXTENSION_SOURCES})
build_loadable_extension(${TARGET_NAME} " " ${EXTENSION_SOURCES})

# Link OpenSSL in both the static library as the loadable extension
# target_link_libraries(${EXTENSION_NAME} OpenSSL::SSL OpenSSL::Crypto)
# target_link_libraries(${LOADABLE_EXTENSION_NAME} OpenSSL::SSL OpenSSL::Crypto)
target_include_directories(${LOADABLE_EXTENSION_NAME} PRIVATE include ${TD_INCLUDE_DIR})
target_link_libraries(${LOADABLE_EXTENSION_NAME} ${TD_CLIV2_LIB} ${TD_USR_LIB})


#install(
#  TARGETS ${EXTENSION_NAME}
#  EXPORT "${DUCKDB_EXPORT_SET}"
#  LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
#  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}")
