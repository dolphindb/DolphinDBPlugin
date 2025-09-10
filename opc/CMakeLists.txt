cmake_minimum_required(VERSION 3.22)
set(PluginName PluginOPC)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})
target_compile_options(${PluginName} PRIVATE "-Wno-error")
target_include_directories(${PluginName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/OPCClientToolKit)
target_link_libraries(${PluginName} PRIVATE Ole32 oleaut32 uuid)
