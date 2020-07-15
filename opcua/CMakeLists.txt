cmake_minimum_required(VERSION 3.0)
project(OPC_UA)
if (WIN32 OR MINGW)
    set (platform_macro "-DWINDOWS")
    set (plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginOpcUa_win.txt")
elseif (UNIX)
    set (platform_macro "-DLINUX")
    add_compile_options("-D_GLIBCXX_USE_CXX11_ABI=0")
    set (plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginOpcUa.txt")
endif ()
message("platform: ${platform_macro}")
add_compile_options("-g" "-std=c++11" "-fPIC" ${platform_macro} "-Wall" "-Werror")

macro(replace_cxx_release_flag src_flag dest_flag)
    string(REPLACE "${src_flag}" "${dest_flag}" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
endmacro(replace_cxx_release_flag flag)

replace_cxx_release_flag("-O3" "-O2") 
include_directories("./open62541/include" "../include")

link_directories(./lib/centos)
# link_directories(path to libDolphindb.so)
add_library(PluginOPCUA SHARED ./src/opc_ua.cpp)
target_link_libraries(PluginOPCUA
        DolphinDB
        open62541
        )
add_custom_command( TARGET PluginOPCUA POST_BUILD 
                    COMMAND cp ${plugin_file} "${CMAKE_BINARY_DIR}/PluginOpcUa.txt" 
                    COMMENT "copy PluginOpcUa.txt")
