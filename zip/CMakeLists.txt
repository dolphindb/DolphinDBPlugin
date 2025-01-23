cmake_minimum_required (VERSION 2.8)

project (dolphindb_zip_plugin)
add_definitions(-DLOCKFREE_SYMBASE)

set(CMAKE_CXX_STANDARD 11)

if(WIN32)
    set(platform_macro "-DWINDOWS -DUSE_WINDOWS")
elseif(UNIX)
    set(platform_macro "-DLINUX")
    if(ENABLEABI MATCHES 1)
        message("ENABLEABI")
        add_compile_options(${platform_macro} -D_GLIBCXX_USE_CXX11_ABI=1  -fPIC)
    else()
        message("DISENABLEABI")
        add_compile_options(${platform_macro} -D_GLIBCXX_USE_CXX11_ABI=0  -fPIC)
    endif()
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${platform_macro} -std=c++11 -Wall -DHAVE_AES")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${platform_macro} -fPIC -DHAVE_AES")

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "RELEASE")
endif ()

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
message("CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})

if("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -ggdb")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0 -ggdb")
    message(STATUS "Build: Debug with -O0 -Wall -g -ggdb")
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
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
    set(LINK_FLAGS ${LINK_FLAGS} "-fsanitize=address")
endif ()

aux_source_directory(src DIR_LIB_SRCS)
aux_source_directory(minizip/aes DIR_LIB_SRCS)
aux_source_directory(minizip DIR_LIB_SRCS)
aux_source_directory(zipper DIR_LIB_SRCS)
include_directories(
    ${PROJECT_SOURCE_DIR}/../include
    ${PROJECT_SOURCE_DIR}/minizip
    ${PROJECT_SOURCE_DIR}/zipper
)
if(WIN32)
    include_directories(
    ${PROJECT_SOURCE_DIR}/minizip/win32
    )
    aux_source_directory(minizip/win32 DIR_LIB_SRCS)
endif()

if (WIN32)
    link_directories(
        ${PROJECT_SOURCE_DIR}/lib/Win64
        ${CMAKE_BINARY_DIR}
    )
elseif(UNIX)
    link_directories(
        ${PROJECT_SOURCE_DIR}/lib/Linux64
        ${CMAKE_BINARY_DIR}
    )
endif()

add_library(PluginZip SHARED ${DIR_LIB_SRCS})
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)

if (WIN32)
    target_link_libraries(PluginZip
        DolphinDB
        zlibstatic
        # unzip
        # ioapi
    )
elseif(BUILD_ARM)
    target_link_libraries(PluginZip
        ${PROJECT_SOURCE_DIR}/lib/ARM64/libz.a
        # unzip
        # ioapi
    )
else()
    target_link_libraries(PluginZip
        DolphinDB
        z
        # unzip
        # ioapi
    )
endif()

if (WIN32)
set_target_properties(PluginZip PROPERTIES LINK_FLAGS "-static-libstdc++")
endif()

if (WIN32)
    set(plugin_file "${PROJECT_SOURCE_DIR}/PluginZip_win32.txt")
        add_custom_command(
            TARGET PluginZip POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${plugin_file} ${PROJECT_SOURCE_DIR}/build/PluginZip.txt
            COMMENT "copy PluginZip.txt"
    )
elseif(UNIX)
    set(plugin_file "${PROJECT_SOURCE_DIR}/PluginZip.txt")
    add_custom_command(
        TARGET PluginZip POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${plugin_file} ${PROJECT_SOURCE_DIR}/build/PluginZip.txt
        COMMENT "copy PluginZip.txt"
    )
endif()
