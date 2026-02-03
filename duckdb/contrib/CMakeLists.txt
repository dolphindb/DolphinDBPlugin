# DuckDB as a subdirectory
# Reference: mysql/contrib/mariadb-connector-c-cmake/CMakeLists.txt

# Check if DuckDB source exists
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/duckdb/CMakeLists.txt")
    message(STATUS "DuckDB source not found. Please run: git submodule update --init --recursive")
    message(FATAL_ERROR "DuckDB submodule is required. See README_CN.md for instructions.")
endif()

add_subdirectory(duckdb)
