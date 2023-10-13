cmake_minimum_required(VERSION 3.0)
project(hdfs)
add_definitions(-DLOCKFREE_SYMBASE)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if (WIN32)
    set(platform_macro "-DWINDOWS")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginHdfs.txt")
elseif (UNIX)
    set(platform_macro "-DLINUX")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginHdfs.txt")
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${platform_macro} -fPIC  -O3 -std=c++11 -Wall -rdynamic ")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -fPIC -std=c++11 -Wall -rdynamic ")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fPIC -O3 -std=c++11 -Wall -rdynamic ")

if(NOT DEFINED HADOOP_INCLUDE)
    set(HADOOP_INCLUDE "${HADOOP_DIR}/include")
endif()
if(NOT DEFINED HADOOP)
    set(HADOOP_SO "${HADOOP_DIR}/lib/native")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include ${HADOOP_INCLUDE})
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)

link_directories(${CMAKE_BINARY_DIR} ${HADOOP_SO})

aux_source_directory("./src" plugin_hadoop_src)

add_library(PluginHdfs SHARED ${plugin_hadoop_src})
target_link_libraries(
    PluginHdfs
    hdfs
    libkrb5.a
    libcom_err.a
    libk5crypto.a
    libkrb5support.a
    resolv
    keyutils
)