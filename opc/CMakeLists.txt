cmake_minimum_required(VERSION 3.13)
set(PluginName PluginOPC)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

target_include_directories(${PluginName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/OPCClientToolKit)
target_link_libraries(PluginOPC PRIVATE Ole32 oleaut32 uuid)
