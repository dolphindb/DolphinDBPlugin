cmake_minimum_required(VERSION 3.13)
set(PluginName PluginZmq)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})
target_link_libraries(${PluginName} ${CMAKE_CURRENT_SOURCE_DIR}/bin/linux64/libzmq.a)
