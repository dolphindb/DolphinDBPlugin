cmake_minimum_required(VERSION 3.22)
set(PluginName PluginZmq)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

find_package(ZeroMQ REQUIRED)
target_link_libraries(${PluginName} PRIVATE libzmq-static)
target_include_directories(${PluginName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/cppzmq)
