cmake_minimum_required(VERSION 3.22)

set(PluginName PluginDuckDB)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

# Add DuckDB dependency
add_subdirectory(contrib/duckdb)

# Link DuckDB library
target_link_libraries(${PluginName} PRIVATE duckdb)

# Suppress warnings
target_compile_options(${PluginName} PRIVATE -Wno-noexcept-type)
