cmake_minimum_required(VERSION 3.22)
set(PluginName PluginParquet)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

find_package(Arrow REQUIRED)
find_package(Parquet REQUIRED)
target_compile_options(${PluginName} PRIVATE -Wno-error=unused-parameter)
target_link_libraries(${PluginName} PRIVATE parquet_static arrow_static)