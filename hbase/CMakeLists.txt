cmake_minimum_required(VERSION 3.22)
set(PluginName PluginHBase)
project(${PluginName})

include(../template.cmake)

set(HBASE_THRIFT_GENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp)
CreatePlugin(${PluginName})

find_package(Boost REQUIRED)
target_link_libraries(${PluginName} PRIVATE Boost::boost)
find_package(Thrift REQUIRED)
target_link_libraries(${PluginName} PRIVATE thrift::thrift)
