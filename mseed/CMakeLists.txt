cmake_minimum_required(VERSION 3.0)
project(PluginMongodb)

set(CMAKE_CXX_STANDARD 11)

if (WIN32)
    set(platform_macro "-DWINDOWS")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginMseed_win32.txt")
elseif(UNIX)
    set(platform_macro "-DLINUX")
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
    set(CMAKE_EXE_LINKER_FLAGS,"-Wl,-rpath,.")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginMseed.txt")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${platform_macro} -fPIC -o3  -Wall ")

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/../include)
include_directories(${PROJECT_SOURCE_DIR}/libmseed-3.0.8)

link_directories("${CMAKE_BINARY_DIR}")

aux_source_directory(${PROJECT_SOURCE_DIR}/src  DIR_SRCS)

aux_source_directory(${PROJECT_SOURCE_DIR}/libmseed-3.0.8 DIR_SRCS)

add_library(PluginMseed SHARED ${DIR_SRCS})

target_link_libraries(PluginMseed DolphinDB)

configure_file(${plugin_file} ${CMAKE_BINARY_DIR}/PluginMseed.txt COPYONLY)
