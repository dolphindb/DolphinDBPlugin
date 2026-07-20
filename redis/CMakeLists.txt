cmake_minimum_required(VERSION 3.22)

set(PluginName PluginRedis)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

find_package(hiredis)
target_link_libraries(${PluginName} PRIVATE hiredis::hiredis)
