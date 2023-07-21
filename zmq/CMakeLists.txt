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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${platform_macro} -fPIC -std=c++11 -Wall")

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "RELEASE")
endif ()

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
message("CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})

if("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RELWITHDEBINFO")  # release with debug info
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2")
else ()
    message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}")
endif ()

if (${DDB_USE_ASAN})
    add_compile_options(
        "-fsanitize=address"                # Enable ASAN.
        "-fno-omit-frame-pointer"           # Nicer stack traces in error messages.
        "-fno-optimize-sibling-calls"       # Disable tail call elimination (perfect stack traces if inlining off).
        )
    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-fsanitize=address")
endif ()

set(CMAKE_EXE_LINKER_FLAGS,"-Wl,-rpath,.")
link_libraries(${PROJECT_SOURCE_DIR}/bin/linux64/libzmq.a)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/../include)

link_directories("${CMAKE_BINARY_DIR}")

aux_source_directory(${PROJECT_SOURCE_DIR}/src  DIR_SRCS)

add_library(PluginZmq SHARED ${DIR_SRCS})

target_link_libraries(PluginZmq DolphinDB)

configure_file(${plugin_file} ${CMAKE_BINARY_DIR}/PluginZmq.txt COPYONLY)
