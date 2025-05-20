cmake_minimum_required(VERSION 3.13)
set(PluginName PluginMseed)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libmseed-3.0.8)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libmseed-3.0.8)
