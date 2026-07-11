cmake_minimum_required(VERSION 3.22)

set(PluginName PluginOPCUA)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

find_package(open62541 REQUIRED)
target_link_libraries(${PluginName} PRIVATE open62541::open62541)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(${PluginName} PRIVATE crypt32 ws2_32)
endif()
