cmake_minimum_required(VERSION 3.0)
project(PluginKDB)
add_definitions(-DLOCKFREE_SYMBASE)

set(CMAKE_CXX_STANDARD 11)

if (WIN32)
    add_definitions("-DWINDOWS -D_WIN32_WINNT=0x0600 -DWINVER=0x0600")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginKDB_WIN32.txt")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${platform_macro} -DKXVER=3 -Wall")
    file(COPY ${PROJECT_SOURCE_DIR}/lib/win/c.dll DESTINATION ${CMAKE_BINARY_DIR}/)
    file(COPY ${PROJECT_SOURCE_DIR}/lib/win/lz4.dll DESTINATION ${CMAKE_BINARY_DIR}/)
    file(COPY ${PROJECT_SOURCE_DIR}/lib/win/libsnappy.dll DESTINATION ${CMAKE_BINARY_DIR}/)
    file(COPY ${PROJECT_SOURCE_DIR}/lib/win/zlib1.dll DESTINATION ${CMAKE_BINARY_DIR}/)
elseif(UNIX)
    set(platform_macro "-DLINUX")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginKDB.txt")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${platform_macro} -DKXVER=3 -Wall -fPIC")
endif()

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

include_directories(${CMAKE_SOURCE_DIR}/../include)
include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories("${CMAKE_BINARY_DIR}")

aux_source_directory(${PROJECT_SOURCE_DIR}/src DIR_SRCS)
add_library(PluginKDB SHARED ${DIR_SRCS})

if (WIN32)
    target_link_libraries(PluginKDB
        DolphinDB
        ${PROJECT_SOURCE_DIR}/lib/win/libsnappy.dll
        ${PROJECT_SOURCE_DIR}/lib/win/zlib1.dll
        ${PROJECT_SOURCE_DIR}/lib/win/lz4.dll
        ${PROJECT_SOURCE_DIR}/lib/win/c.lib
        ${PROJECT_SOURCE_DIR}/lib/win/c.dll
    )
elseif(UNIX)
    target_link_libraries(PluginKDB
        DolphinDB
        ${PROJECT_SOURCE_DIR}/lib/linux/c.o
        ${PROJECT_SOURCE_DIR}/lib/linux/libz.a
        ${PROJECT_SOURCE_DIR}/lib/linux/libsnappy.a
        ${PROJECT_SOURCE_DIR}/lib/linux/liblz4.a
    )
endif()

configure_file(${plugin_file} ${CMAKE_BINARY_DIR}/PluginKDB.txt COPYONLY)