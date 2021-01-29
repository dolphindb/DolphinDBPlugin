#cmake_minimum_required(VERSION 3.0)
project(mysqlPlugin)


set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/..)

set (platform_macro "-DLINUX")
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${platform_macro} -fPIC -std=c++11 -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

AUX_SOURCE_DIRECTORY(src DIR_SRCS)

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/../include")
link_directories("${CMAKE_BINARY_DIR}/../bin/linux_x64/ABI0/")
#link_directories("your ssl lib path")
#link_directories("your mysql lib path")

add_library(dolSync SHARED ${DIR_SRCS})
set_target_properties (dolSync PROPERTIES OUTPUT_NAME "dolSync")
target_link_libraries(dolSync -ldl -luuid -lDolphinDBAPI -lpthread -lmysqlclient -lssl -lcrypto)
