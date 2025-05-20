cmake_minimum_required(VERSION 3.13)
set(PluginName PluginMseed)
project(${PluginName})

set(CMAKE_CXX_STANDARD 11)

if (WIN32)
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginMseed_win32.txt")
elseif(UNIX)
    set(CMAKE_EXE_LINKER_FLAGS,"-Wl,-rpath,.")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginMseed.txt")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${platform_macro} -fPIC -O3  -Wall ")

if (${DDB_USE_ASAN})
    add_compile_options(
        "-fsanitize=address"                # Enable ASAN.
        "-fno-omit-frame-pointer"           # Nicer stack traces in error messages.
        "-fno-optimize-sibling-calls"       # Disable tail call elimination (perfect stack traces if inlining off).
        )
    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-fsanitize=address")
    message("enable asan")
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include/ddbplugin)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libmseed-3.0.8)

link_directories("${CMAKE_BINARY_DIR}")

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src  DIR_SRCS)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/libmseed-3.0.8 DIR_SRCS)

add_library(PluginMseed SHARED ${DIR_SRCS})

if (WIN32)
    target_link_libraries(PluginMseed DolphinDB)
endif()

configure_file(${plugin_file} ${CMAKE_BINARY_DIR}/PluginMseed.txt COPYONLY)
