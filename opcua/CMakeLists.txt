cmake_minimum_required(VERSION 3.0)
project(OPC_UA)
add_definitions(-DLOCKFREE_SYMBASE)
if (WIN32 OR MINGW)
    set (platform_macro "-DWINDOWS")
    set (plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginOPCUA_win.txt")
    add_definitions(-DOPCUA)
elseif (UNIX)
    set (platform_macro "-DLINUX")
    add_compile_options("-D_GLIBCXX_USE_CXX11_ABI=0")
    set (plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginOPCUA.txt")
endif ()
message("platform: ${platform_macro}")
add_compile_options("-std=c++11" "-fPIC" ${platform_macro} "-Wall")

macro(replace_cxx_release_flag src_flag dest_flag)
    string(REPLACE "${src_flag}" "${dest_flag}" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
endmacro(replace_cxx_release_flag flag)

replace_cxx_release_flag("-O3" "-O2") 
include_directories("./include" "../include")

link_directories(./lib)
link_directories(${LIBDOLPHINDB})
add_library(PluginOPCUA SHARED ./src/opc_ua.cpp)
if (WIN32 OR MINGW)
    target_link_libraries(PluginOPCUA
        ${CMAKE_CURRENT_LIST_DIR}/lib/libmbedcrypto.a
        ${CMAKE_CURRENT_LIST_DIR}/lib/libmbedx509.a
        ${CMAKE_CURRENT_LIST_DIR}/lib/libmbedtls.a
        open62541
        DolphinDB
        )
elseif (UNIX)
    target_link_libraries(PluginOPCUA
        open62541
        DolphinDB
        )
endif ()

add_custom_command( TARGET PluginOPCUA POST_BUILD 
                    COMMAND ${CMAKE_COMMAND} -E copy ${plugin_file} ${CMAKE_BINARY_DIR}/PluginOPCUA.txt
                    COMMENT "copy PluginOPCUA.txt")
