cmake_minimum_required(VERSION 3.0)
project(Redis)
add_definitions(-DLOCKFREE_SYMBASE)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

## set compile flags
if (WIN32)
    set (platform_macro "-DWINDOWS")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginRedis_win32.txt")
elseif (UNIX)
    set (platform_macro "-DLINUX")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginRedis.txt")
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${platform_macro} -std=c++11")
add_compile_options(-Wall -O3 -fPIC)

## set include dires
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
link_directories("${CMAKE_BINARY_DIR}")

## build hiredis
SET(DISABLE_TESTS ON CACHE BOOL "stop build redis tests")
add_subdirectory(contrib/hiredis)

## build PluginRedis
add_library(PluginRedis SHARED
        src/pluginRedis.cpp
        src/redisConnection.cpp)

set_target_properties(PluginRedis PROPERTIES LINK_DIRECTORIES "${CMAKE_BINARY_DIR}")
target_link_libraries(PluginRedis hiredis_static)

configure_file(${plugin_file} ${CMAKE_BINARY_DIR}/PluginRedis.txt COPYONLY)
