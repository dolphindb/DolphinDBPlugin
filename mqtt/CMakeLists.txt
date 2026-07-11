cmake_minimum_required(VERSION 3.22)

set(PluginName PluginMQTTClient)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

add_subdirectory(lib)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(${PluginName} PRIVATE WS2_32)
endif()

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME INSTALL_DIR)
set(MqttConfig ${CMAKE_CURRENT_BINARY_DIR}/PluginMQTT.txt)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/PluginMQTT.txt ${MqttConfig})
install(FILES ${MqttConfig} DESTINATION ${INSTALL_DIR})
