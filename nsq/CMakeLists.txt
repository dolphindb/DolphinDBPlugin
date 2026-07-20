cmake_minimum_required(VERSION 3.22)

set(PluginName PluginNsq)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

target_link_libraries(${PluginName} PRIVATE HSNsqApi)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_directories(${PluginName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/lib/win64)
endif()
