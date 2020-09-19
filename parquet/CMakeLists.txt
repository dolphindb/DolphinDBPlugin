cmake_minimum_required(VERSION 3.0)
project(parquet)
if (WIN32 OR MINGW)
    set (platform_macro "-DWINDOWS")
    set (plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginParquet_win.txt")
elseif (UNIX)
    set (platform_macro "-DLINUX")
    add_compile_options("-D_GLIBCXX_USE_CXX11_ABI=0")
    set (plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginParquet.txt")
endif ()
message("platform: ${platform_macro}")
add_compile_options("-std=c++11" "-fPIC" ${platform_macro} "-Wall")

macro(replace_cxx_release_flag src_flag dest_flag)
    string(REPLACE "${src_flag}" "${dest_flag}" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
endmacro(replace_cxx_release_flag flag)

replace_cxx_release_flag("-O3" "-O2") 
include_directories("./parquetApi/include" "../include")
link_directories(./parquetApi/lib/linux)
# link_directories(/mnt/data/zsy/thriftLib/lib)
link_directories(${LIBDOLPHINDB})
add_library(PluginParquet SHARED ./src/parquet_plugin.cpp)
target_link_libraries(PluginParquet
    ${CMAKE_CURRENT_LIST_DIR}/parquetApi/lib/linux/libparquet.a
    ${CMAKE_CURRENT_LIST_DIR}/parquetApi/lib/linux/libarrow.a
    ${CMAKE_CURRENT_LIST_DIR}/parquetApi/lib/linux/libsnappy.a
    ${CMAKE_CURRENT_LIST_DIR}/parquetApi/lib/linux/libbz2.a
    ${CMAKE_CURRENT_LIST_DIR}/parquetApi/lib/linux/liblz4.a
    ${CMAKE_CURRENT_LIST_DIR}/parquetApi/lib/linux/libz.a
    ${CMAKE_CURRENT_LIST_DIR}/parquetApi/lib/linux/libzstd.a
    ${CMAKE_CURRENT_LIST_DIR}/parquetApi/lib/linux/libutf8proc.a
    ${CMAKE_CURRENT_LIST_DIR}/parquetApi/lib/linux/libjemalloc_pic.a
    pthread
    rt
    ${CMAKE_CURRENT_LIST_DIR}/parquetApi/lib/linux/libboost_regex.a
    ${CMAKE_CURRENT_LIST_DIR}/parquetApi/lib/linux/libthrift.a
    DolphinDB
    )

add_custom_command( TARGET PluginParquet POST_BUILD 
                    COMMAND cp ${plugin_file} "${CMAKE_BINARY_DIR}/PluginParquet.txt" 
                    COMMENT "copy PluginParquet.txt")
