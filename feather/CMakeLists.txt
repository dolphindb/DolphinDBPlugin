cmake_minimum_required(VERSION 3.22)
set(PluginName PluginFeather)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

target_compile_options(${PluginName} PRIVATE "-Wno-error")

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    AddArrow()
endif()
if (CMAKE_SYSTEM_NAME STREQUAL "MSYS")
    target_link_directories(${PluginName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/win)
    target_link_libraries(${PluginName} PRIVATE libarrow.dll liblz4.dll zlib.dll libzstd.dll)
    cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME INSTALL_DIR)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/win/ DESTINATION ${INSTALL_DIR})
endif ()
