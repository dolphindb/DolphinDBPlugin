cmake_minimum_required (VERSION 2.8)

project (dolphindb_zip_plugin)
add_definitions(-DLOCKFREE_SYMBASE)

set(CMAKE_CXX_COMPILER g++)
add_compile_options(-std=c++11 -DLINUX -D_GLIBCXX_USE_CXX11_ABI=0 -Wall -O3 -g -fPIC)

aux_source_directory(src DIR_LIB_SRCS)
aux_source_directory(lib_src DIR_LIB_SRCS)
include_directories(
    ${PROJECT_SOURCE_DIR}/../include
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/lib_src
)
link_directories(
    ${PROJECT_SOURCE_DIR}/lib
    ${CMAKE_BINARY_DIR}
)

add_library(PluginZip SHARED ${DIR_LIB_SRCS})
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)
target_link_libraries(PluginZip
    DolphinDB
    z
    # unzip
    # ioapi
)

set(plugin_file "${PROJECT_SOURCE_DIR}/PluginZip.txt")
add_custom_command(
    TARGET PluginZip POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${plugin_file} ${PROJECT_SOURCE_DIR}/build/PluginZip.txt
    COMMENT "copy PluginZip.txt"
)
