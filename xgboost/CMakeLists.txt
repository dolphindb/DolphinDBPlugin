cmake_minimum_required(VERSION 3.0)

project(dolphindb_xgboost_plugin)
add_definitions(-DLOCKFREE_SYMBASE)

if (WIN32 OR MINGW)
    set (platform_macro "-DWINDOWS")
    set (plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginXgboost_win.txt")
    set (xgboost_lib "xgboost.dll")
elseif (UNIX)
    set (platform_macro "-DLINUX")
    add_compile_options("-D_GLIBCXX_USE_CXX11_ABI=0")
    set (xgboost_lib "libxgboost.a")
    set (plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginXgboost.txt")
endif ()

message("platform: ${platform_macro}")
add_compile_options("-std=c++11" "-fPIC" ${platform_macro} "-Wall" "-Werror")

macro(replace_cxx_release_flag src_flag dest_flag)
    string(REPLACE "${src_flag}" "${dest_flag}" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
endmacro(replace_cxx_release_flag flag)

replace_cxx_release_flag("-O3" "-O2") 
include_directories("../include" "./xgboost_static/include")
link_directories(${LIBDOLPHINDB})
add_library(PluginXgboost SHARED ./src/xgboost.cpp)
target_link_libraries(PluginXgboost
        ${CMAKE_CURRENT_LIST_DIR}/xgboost_static/lib/${xgboost_lib}
        ${CMAKE_CURRENT_LIST_DIR}/xgboost_static/lib/librabit.a
        ${CMAKE_CURRENT_LIST_DIR}/xgboost_static/lib/libdmlc.a
        DolphinDB
        )
set_target_properties(PluginXgboost
    PROPERTIES
    LINK_FLAGS "-fopenmp -static-libstdc++"
    )

if (WIN32 OR MINGW)
    configure_file(${plugin_file} ${CMAKE_BINARY_DIR}/PluginXgboost.txt COPYONLY)
elseif (UNIX)
    add_custom_command( TARGET PluginXgboost POST_BUILD 
                    COMMAND cp ${plugin_file} "${CMAKE_BINARY_DIR}/PluginXgboost.txt"
                    COMMENT "copy PluginXgboost.txt")
endif ()
