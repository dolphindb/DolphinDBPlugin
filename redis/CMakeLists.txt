cmake_minimum_required(VERSION 3.22)

set(PluginName PluginRedis)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

## build hiredis
SET(DISABLE_TESTS ON CACHE BOOL "stop build redis tests")
add_subdirectory(contrib/hiredis)

target_link_libraries(${PluginName} PRIVATE hiredis_static)
