cmake_minimum_required(VERSION 3.0)
project(PluginZmq)
add_definitions(-DLOCKFREE_SYMBASE)

set(CMAKE_CXX_STANDARD 11)

if (WIN32)
    set(platform_macro "-DWINDOWS")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginZmq_win32.txt")
elseif(UNIX)
    set(platform_macro "-DLINUX")
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
    set(CMAKE_EXE_LINKER_FLAGS,"-Wl,-rpath,.")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginZmq.txt")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${platform_macro} -fPIC -o3 -Wall ")
set(CMAKE_EXE_LINKER_FLAGS,"-Wl,-rpath,.")
link_libraries(${PROJECT_SOURCE_DIR}/bin/linux64/libzmq.a)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/../include)

link_directories("${CMAKE_BINARY_DIR}")

aux_source_directory(${PROJECT_SOURCE_DIR}/src  DIR_SRCS)

add_library(PluginZmq SHARED ${DIR_SRCS})

target_link_libraries(PluginZmq DolphinDB)

configure_file(${plugin_file} ${CMAKE_BINARY_DIR}/PluginZmq.txt COPYONLY)
