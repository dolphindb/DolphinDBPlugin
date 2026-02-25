cmake_minimum_required(VERSION 3.22)

set(PluginName PluginNsq)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

target_compile_options(${PluginName} PRIVATE -Wno-error)

if (CMAKE_SYSTEM_NAME STREQUAL "MSYS")
    set(NSQ_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/win64/)
endif()
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(NSQ_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux.x64/)
    set_target_properties(${PluginName} PROPERTIES LINK_FLAGS "-Wl,-rpath,$ORIGIN")
endif()

target_link_directories(${PluginName} PRIVATE ${NSQ_LIB_PATH})
target_link_libraries(${PluginName} PRIVATE HSNsqApi)

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME INSTALL_DIR)
install(DIRECTORY ${NSQ_LIB_PATH} DESTINATION ${INSTALL_DIR})
