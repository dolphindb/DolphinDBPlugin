cmake_minimum_required(VERSION 3.22)
set(PluginName PluginHBase)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

target_compile_options(${PluginName} PRIVATE -w)

find_package(Boost REQUIRED)
target_link_libraries(${PluginName} PRIVATE Boost::boost)
find_package(Thrift)
target_link_libraries(${PluginName} PRIVATE thrift::thrift)
