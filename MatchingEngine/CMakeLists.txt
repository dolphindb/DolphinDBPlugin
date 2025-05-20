cmake_minimum_required(VERSION 3.13)

set(PluginName PluginMatchingEngine)
project(${PluginName})

include(../template.cmake)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)
CreatePlugin(${PluginName})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PLUGIN_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(${PluginName} PRIVATE ${PLUGIN_INCLUDE_DIRECTORIES})
