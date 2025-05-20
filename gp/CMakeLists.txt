cmake_minimum_required(VERSION 3.13)
set(PluginName PluginGp)
project(${PluginName})

set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DHAVE_CONFIG_H -fPIC -g -DDEBUG ")
# set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -DHAVE_CONFIG_H -fPIC -std=c++11 -g -DDEBUG ")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_CONFIG_H -fPIC -o3")
#set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DHAVE_CONFIG_H -fPIC -std=c++11 -o3 ")
if (WIN32)
    set(platform_macro "-DWINDOWS")
    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/bin/windows)
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginGp_win32.txt")
elseif(UNIX)
    set(platform_macro "-DLINUX")
    set(CMAKE_EXE_LINKER_FLAGS,"-Wl,-rpath,.")
    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/bin/linux)
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginGp.txt")
endif()

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

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${platform_macro} -DHAVE_CONFIG_H -fPIC -Wall -std=c11 -g -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${platform_macro} -DHAVE_CONFIG_H -fPIC -Wall -std=c++11 -g -O3 ")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include/ddbplugin)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dolphindbGnuplot)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dolphindbGnuplot/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dolphindbGnuplot/term)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/libgd/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/libjpeg/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/libpng/include)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} DIR_SRCS)


link_directories("${CMAKE_BINARY_DIR}")

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/dolphindbGnuplot/src DIR_SRCS)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src DIR_SRCS)

add_library(PluginGp SHARED ${DIR_SRCS})


target_link_libraries(PluginGp readline tinfo gd)
set_target_properties(PluginGp PROPERTIES LINK_FLAGS "-Wl,-rpath,$ORIGIN:.")

configure_file(${plugin_file} ${CMAKE_BINARY_DIR}/PluginGp.txt COPYONLY)
