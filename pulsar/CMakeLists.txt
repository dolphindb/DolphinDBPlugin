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
