cmake_minimum_required(VERSION 3.13)

set(PluginName PluginMQTTClient)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})
add_subdirectory(lib)
if (WIN32)
    target_link_libraries(${PluginName} PRIVATE WS2_32)
endif()
