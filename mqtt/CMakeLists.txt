cmake_minimum_required(VERSION 3.22)

set(PluginName PluginMQTTClient)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

add_subdirectory(lib)

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME INSTALL_DIR)
set(ALIAS_PLUGIN_TXT ${CMAKE_CURRENT_SOURCE_DIR}/PluginMQTT.txt)
set(ALIAS_PLUGIN_TXT_DEST ${CMAKE_CURRENT_BINARY_DIR}/PluginMQTT.txt)

add_custom_command(TARGET ${PluginName} POST_BUILD COMMAND cp ${ALIAS_PLUGIN_TXT} ${ALIAS_PLUGIN_TXT_DEST})
add_custom_command(TARGET ${PluginName} POST_BUILD COMMAND sed -i "\'s/<version>/${PluginVersion}/g\'" ${ALIAS_PLUGIN_TXT_DEST})

if (WIN32)
    add_custom_command(TARGET ${PluginName} POST_BUILD COMMAND sed -i "\'s/.so,/.dll,/g\'" ${ALIAS_PLUGIN_TXT_DEST})
    target_link_libraries(${PluginName} PRIVATE WS2_32)
endif()

install(FILES ${ALIAS_PLUGIN_TXT_DEST} DESTINATION ${INSTALL_DIR})


