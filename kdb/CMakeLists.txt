cmake_minimum_required(VERSION 3.0)
project(PluginKDB)

set(CMAKE_CXX_STANDARD 11)

if (WIN32)
    add_definitions("-DWINDOWS -D_WIN32_WINNT=0x0600 -DWINVER=0x0600")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginKDB_WIN32.txt")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${platform_macro} -DKXVER=3 -g -O3 -lpthread -lz")
    file(COPY ${PROJECT_SOURCE_DIR}/lib/w64/c.dll DESTINATION ${CMAKE_BINARY_DIR}/)
elseif(UNIX)
    set(platform_macro "-DLINUX")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginKDB.txt")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${platform_macro} -DKXVER=3 -g -fPIC -O3 -lpthread -lz")
endif()


include_directories(${CMAKE_SOURCE_DIR}/../include)
link_directories("${CMAKE_BINARY_DIR}")

aux_source_directory(${PROJECT_SOURCE_DIR}/src DIR_SRCS)
add_library(PluginKDB SHARED ${DIR_SRCS})

if (WIN32)
target_link_libraries(PluginKDB DolphinDB ${PROJECT_SOURCE_DIR}/lib/w64/c.lib  ${PROJECT_SOURCE_DIR}/lib/w64/c.dll z)
elseif(UNIX)
target_link_libraries(PluginKDB DolphinDB ${PROJECT_SOURCE_DIR}/lib/c.o z)
endif()

configure_file(${plugin_file} ${CMAKE_BINARY_DIR}/PluginKDB.txt COPYONLY)