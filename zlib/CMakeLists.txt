cmake_minimum_required(VERSION 3.13)

set(PluginName PluginZlib)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})
if(WIN32)
    target_link_directories(${PluginName} PRIVATE ${PROJECT_SOURCE_DIR}/lib/Win64)
    target_link_directories(${PluginName} PRIVATE ${CMAKE_BINARY_DIR})
    target_link_libraries(${PluginName} PRIVATE zlibstatic)
else()
    AddZlib()
endif()
