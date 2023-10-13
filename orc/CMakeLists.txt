cmake_minimum_required(VERSION 3.0)

project(orc)
add_definitions(-DLOCKFREE_SYMBASE)

set(plugin_file ${CMAKE_CURRENT_LIST_DIR}/PluginOrc.txt)
add_compile_options(-std=c++11 -fPIC -DLINUX -Wall -D_GLIBCXX_USE_CXX11_ABI=0)

include_directories(include ../include)
if (BUILD_ARM)
    link_directories("${CMAKE_BINARY_DIR}")
    add_compile_options(-fsigned-char)
else()
    link_directories(lib/linux /usr/lib64 ${CMAKE_BINARY_DIR})
endif()

add_library(PluginOrc SHARED src/orc_plugin.cpp)

if (BUILD_ARM)
    target_link_libraries(PluginOrc DolphinDB orc protobuf snappy  z zstd)
else()
target_link_libraries(PluginOrc orc protobuf snappy lz4 z zstd hdfspp_static pthread sasl2 crypto DolphinDB)
endif()

add_custom_command(
    TARGET PluginOrc POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy ${plugin_file} ${CMAKE_BINARY_DIR}/PluginOrc.txt
    COMMENT "copy PluginOrc.txt"
)
