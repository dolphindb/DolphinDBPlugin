cmake_minimum_required(VERSION 3.13)
project(Redis)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

## set compile flags
if (WIN32)
    set (platform_macro "-DWINDOWS")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginRedis_win.txt")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${platform_macro} -std=c++11 -Wall -O3")
elseif (UNIX)
    set (platform_macro "-DLINUX")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginRedis.txt")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${platform_macro} -std=c++11 -Wall -O3 -fPIC")
endif ()

if (${DDB_USE_ASAN})
    message("enable asan")
    add_compile_options(
        "-fsanitize=address"                # Enable ASAN.
        "-fno-omit-frame-pointer"           # Nicer stack traces in error messages.
        "-fno-optimize-sibling-calls"       # Disable tail call elimination (perfect stack traces if inlining off).
        )
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "8")
            add_compile_options("-fsanitize-recover=address")
            message("enable asan recover")
        endif()
    endif()
    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-fsanitize=address")
endif ()

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

if (WIN32)
    target_link_libraries(PluginRedis hiredis_static DolphinDB)
    configure_file(${plugin_file} ${CMAKE_BINARY_DIR}/PluginRedis_win.txt COPYONLY)
elseif (UNIX)
    target_link_libraries(PluginRedis hiredis_static)
    configure_file(${plugin_file} ${CMAKE_BINARY_DIR}/PluginRedis.txt COPYONLY)
endif ()
