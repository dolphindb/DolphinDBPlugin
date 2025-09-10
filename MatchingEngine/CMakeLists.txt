cmake_minimum_required(VERSION 3.22)

set(PluginName PluginMatchingEngine)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

target_compile_options(${PluginName} PRIVATE -Wno-error)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PLUGIN_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(${PluginName} PRIVATE ${PLUGIN_INCLUDE_DIRECTORIES})
