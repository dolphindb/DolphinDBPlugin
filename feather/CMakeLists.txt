cmake_minimum_required(VERSION 3.0)
project(feather)
add_definitions(-DLOCKFREE_SYMBASE)

if (WIN32 OR MINGW)
elseif (UNIX)
    set (platform_macro "-DLINUX")
    add_compile_options("-D_GLIBCXX_USE_CXX11_ABI=0")
    set (plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginFeather.txt")
endif ()
message("platform: ${platform_macro}")

if(DEFINED DEBUG)
    add_compile_options("-std=c++11" "-DDEBUG" "-fPIC" ${platform_macro} "-Wall" "-g")
else()
    add_compile_options("-std=c++11" "-fPIC" ${platform_macro} "-Wall" "-g" "-O3")
endif()

include_directories(${CMAKE_SOURCE_DIR}/../include)
include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(./lib/linux)
link_directories(${CMAKE_BINARY_DIR})

add_library(PluginFeather SHARED ./src/feather.h ./src/feather.cpp)

target_link_libraries(PluginFeather
    ${CMAKE_CURRENT_LIST_DIR}/lib/linux/libarrow.a
    ${CMAKE_CURRENT_LIST_DIR}/lib/linux/liblz4.a
    ${CMAKE_CURRENT_LIST_DIR}/lib/linux/libz.a
    ${CMAKE_CURRENT_LIST_DIR}/lib/linux/libzstd.a
    ${CMAKE_CURRENT_LIST_DIR}/lib/linux/libjemalloc_pic.a
    pthread
    DolphinDB
    )

add_custom_command( TARGET PluginFeather POST_BUILD 
                    COMMAND cp ${plugin_file} "${CMAKE_BINARY_DIR}/PluginFeather.txt" 
                    COMMENT "copy PluginFeather.txt")
