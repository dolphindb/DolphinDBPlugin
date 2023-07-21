cmake_minimum_required(VERSION 3.0)
project(PluginZlib)
add_definitions(-DLOCKFREE_SYMBASE)

set(CMAKE_CXX_STANDARD 11)

include_directories(
        src
        ${ZLIB_INCLUDE_DIR}
        ${DOLPHIN_INCLUDE_DIR})

if(WIN32)
    set(platform_macro "-DWINDOWS")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginZlib_win32.txt")
elseif(UNIX)
    set(platform_macro "-DLINUX")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginZlib.txt")
endif()

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    add_compile_options(${platform_macro} -O0 -Wall -g -ggdb)
    message(STATUS "Build: Debug with -O0 -Wall -g -ggdb")
else()
    set(CMAKE_BUILD_TYPE Release)
    add_compile_options(${platform_macro}  -g -O3)
    add_compile_options(${platform_macro} -O3)
    message(STATUS "Build: Release with -O3")
endif()

if (${DDB_USE_ASAN})
    add_compile_options(
            "-fsanitize=address"                # Enable ASAN.
            "-fno-omit-frame-pointer"           # Nicer stack traces in error messages.
            "-fno-optimize-sibling-calls"       # Disable tail call elimination (perfect stack traces if inlining off).
    )
    set(LINK_FLAGS ${LINK_FLAGS} "-fsanitize=address")
endif ()

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/../include)

link_directories("${CMAKE_BINARY_DIR}")

aux_source_directory(${PROJECT_SOURCE_DIR}/src  DIR_SRCS)

add_library(PluginZlib SHARED ${DIR_SRCS})
target_link_libraries(PluginZlib DolphinDB z)
configure_file(${plugin_file} ${CMAKE_BINARY_DIR}/PluginZlib.txt COPYONLY)
