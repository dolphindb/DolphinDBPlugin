cmake_minimum_required(VERSION 2.8)
project(MySQL)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

## set compile flags
if (WIN32)
    set (platform_macro "-DWINDOWS")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginMySQL_win32.txt")
    include_directories("${CMAKE_BINARY_DIR}/curl/include")
elseif (UNIX)
    set (platform_macro "-DLINUX")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginMySQL.txt")
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
endif ()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${platform_macro} -fPIC -std=c++11 -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -DDEBUG")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -DDEBUG_PARSE")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
#add_definitions(-Weffc++)

## set include dires
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
link_directories("${CMAKE_BINARY_DIR}")

## build mysqlclient
add_subdirectory(contrib/mariadb-connector-c-cmake)

## build PluginMySQL
add_library(MySQL SHARED
        src/mysqlxx.cpp
        src/plugin_mysql.cpp)

set_target_properties(MySQL PROPERTIES LINK_DIRECTORIES "${CMAKE_BINARY_DIR}")
if (WIN32)
    target_link_libraries(MySQL DolphinDB mysqlclient)
else ()
    target_link_libraries(MySQL mysqlclient)
endif()

configure_file(${plugin_file} ${CMAKE_BINARY_DIR}/PluginMySQL.txt COPYONLY)
