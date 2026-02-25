cmake_minimum_required(VERSION 3.22)

set(PluginName PluginOPCUA)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

target_link_options(${PluginName} PRIVATE "-flto")
AddDependency(open62541 open62541)
AddDependency(mbedtls mbedcrypto)
AddDependency(mbedtls mbedx509)
if (CMAKE_SYSTEM_NAME STREQUAL "MSYS")
    target_link_libraries(${PluginName} PRIVATE ws2_32)
endif()
