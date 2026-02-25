cmake_minimum_required(VERSION 3.22)

set(PluginName PluginRedis)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

find_package(hiredis)
target_link_libraries(${PluginName} PRIVATE hiredis::hiredis)
if (CMAKE_SYSTEM_NAME STREQUAL "MSYS")
    target_link_libraries(${PluginName} PRIVATE ws2_32 crypt32 shlwapi secur32 bcrypt)
endif()
