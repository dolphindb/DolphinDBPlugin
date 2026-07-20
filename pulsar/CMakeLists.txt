cmake_minimum_required(VERSION 3.27)

set(PluginName PluginPulsar)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

AddDependency(pulsar pulsar)
AddCurl()
AddZlib()
AddProtobuf()
find_package(Snappy REQUIRED)
target_link_libraries(${PluginName} PRIVATE Snappy::snappy)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.80.0 EXACT COMPONENTS regex REQUIRED)
target_link_libraries(${PluginName} PRIVATE Boost::regex)
