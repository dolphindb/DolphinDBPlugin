cmake_minimum_required(VERSION 3.0)
project(PluginNsq)
add_definitions(-DLOCKFREE_SYMBASE)

set(CMAKE_CXX_STANDARD 11)

set(DOLPHIN_INCLUDE_DIR "../include")

aux_source_directory(src PLUGINNsq_SRC)

include_directories(
    src
    ${DOLPHIN_INCLUDE_DIR}
    include
)

set(BASE_DIR ${PROJECT_SOURCE_DIR})
link_directories(${BASE_DIR}/lib/linux.x64)
link_directories("${CMAKE_BINARY_DIR}")

if(WIN32)
    set(platform_macro "-DWINDOWS")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginNsq_win.txt")
elseif(UNIX)
    set(platform_macro "-DLINUX")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginNsq.txt")
endif()

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    add_compile_options(${platform_macro} -O0 -Wall -g -ggdb)
    message(STATUS "Build: Debug with -O0 -Wall -g -ggdb")
else()
    set(CMAKE_BUILD_TYPE Release)
    add_compile_options(${platform_macro} -O3 -g)
    message(STATUS "Build: Release with -O3")
endif()

if(UNIX)
    add_compile_options(-D_GLIBCXX_USE_CXX11_ABI=0)
endif()

add_library(
    ${PROJECT_NAME} SHARED
    ${PLUGINNsq_SRC})

target_link_libraries(
    ${PROJECT_NAME}
    DolphinDB
    HSNsqApi
)

configure_file(${plugin_file} ${CMAKE_BINARY_DIR}/PluginNsq.txt COPYONLY)

