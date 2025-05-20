cmake_minimum_required(VERSION 3.13)
set(CMAKE_SKIP_RPATH TRUE)
set(PluginName PluginHBase)
project(${PluginName})

set(CMAKE_CXX_STANDARD 11)

aux_source_directory(src PLUGINPREDICT_SRC)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${platform_macro} -fPIC -std=c++11 -Wall")

if (WIN32)
    set(platform_macro "-DWINDOWS")
elseif (UNIX)
    set(platform_macro "-DLINUX")
endif ()
add_compile_options(${platform_macro})

if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "RELEASE")
endif ()

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
message("CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})

if ("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -ggdb")
    message(STATUS "Build: Debug with -O0 -Wall -g -ggdb")
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    message(STATUS "Build: Release with -O3")
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


include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include/ddbplugin)
if(DEFINED BUILD_ARM)
    message("building arm")
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/thrift/ARM/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/thrift/x86/include/thrift)
    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/thrift/ARM/lib)
else ()
    message("building x86")
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/thrift/x86/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/thrift/x86/include/thrift)
    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/thrift/x86/lib)
endif()
link_directories(${CMAKE_BINARY_DIR})


add_library(
        ${PROJECT_NAME} SHARED
        ${PLUGINPREDICT_SRC})

target_link_libraries(
        ${PROJECT_NAME}
        thrift
        )

set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginHBase.txt")

configure_file(
        ${plugin_file}
        ${CMAKE_BINARY_DIR}/PluginHBase.txt
        COPYONLY)

