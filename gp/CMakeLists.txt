cmake_minimum_required(VERSION 3.0)
project(PluginGp)
add_definitions(-DLOCKFREE_SYMBASE)

set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DHAVE_CONFIG_H -fPIC -g -DDEBUG ")
# set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -DHAVE_CONFIG_H -fPIC -std=c++11 -g -DDEBUG ")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_CONFIG_H -fPIC -o3")
#set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DHAVE_CONFIG_H -fPIC -std=c++11 -o3 ")
if (WIN32)
    set(platform_macro "-DWINDOWS")
    link_directories(${PROJECT_SOURCE_DIR}/bin/windows)
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginGp_win32.txt")
elseif(UNIX)
    set(platform_macro "-DLINUX")
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
    set(CMAKE_EXE_LINKER_FLAGS,"-Wl,-rpath,.")
    link_directories(${PROJECT_SOURCE_DIR}/bin/linux)
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginGp.txt")
    add_compile_options("-D_GLIBCXX_USE_CXX11_ABI=0")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${platform_macro} -DHAVE_CONFIG_H -fPIC -Wall -std=c11 -g -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${platform_macro} -DHAVE_CONFIG_H -fPIC -Wall -std=c++11 -g -O3 ")

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/../include)
include_directories(${PROJECT_SOURCE_DIR}/dolphindbGnuplot)
include_directories(${PROJECT_SOURCE_DIR}/dolphindbGnuplot/src)
include_directories(${PROJECT_SOURCE_DIR}/dolphindbGnuplot/term)

include_directories(${PROJECT_SOURCE_DIR}/include/libgd/include)
include_directories(${PROJECT_SOURCE_DIR}/include/libjpeg/include)
include_directories(${PROJECT_SOURCE_DIR}/include/libpng/include)
include_directories(${PROJECT_SOURCE_DIR}/include/libzlib/include)

aux_source_directory(${PROJECT_SOURCE_DIR} DIR_SRCS)


link_directories("${CMAKE_BINARY_DIR}")

aux_source_directory(${PROJECT_SOURCE_DIR}/dolphindbGnuplot/src DIR_SRCS)

aux_source_directory(${PROJECT_SOURCE_DIR}/src DIR_SRCS)

add_library(PluginGp SHARED ${DIR_SRCS})


target_link_libraries(PluginGp DolphinDB readline tinfo gd)
set_target_properties(PluginGp PROPERTIES LINK_FLAGS "-Wl,-rpath,$ORIGIN:.")

configure_file(${plugin_file} ${CMAKE_BINARY_DIR}/PluginGp.txt COPYONLY)
