cmake_minimum_required(VERSION 3.13)

set(PluginName PluginNsq)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

if(WIN32)
    set(NSQ_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/win64/)
    set(plugin_file ${CMAKE_CURRENT_SOURCE_DIR}/PluginNsq_win.txt)
elseif(UNIX)
    set(NSQ_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux.x64/)
    set(plugin_file ${CMAKE_CURRENT_SOURCE_DIR}/PluginNsq.txt)
    set_target_properties(${PluginName} PROPERTIES LINK_FLAGS "-Wl,-rpath,$ORIGIN")
endif()

target_link_directories(${PluginName} PRIVATE ${NSQ_LIB_PATH})
target_link_libraries(${PluginName} PRIVATE HSNsqApi)

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME INSTALL_DIR)
configure_file(${plugin_file} ${CMAKE_BINARY_DIR}/PluginNsq.txt COPYONLY)
install(DIRECTORY ${NSQ_LIB_PATH} DESTINATION ${INSTALL_DIR})
