cmake_minimum_required(VERSION 3.13)
set(PluginName PluginParquet)
project(${PluginName})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (WIN32 OR MINGW)
    set (platform_macro "-DWINDOWS")
    set (plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginParquet_win.txt")
elseif (UNIX)
    set (platform_macro "-DLINUX")
    set (plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginParquet.txt")
endif ()
message("platform: ${platform_macro}")
add_compile_options("-std=c++11" "-fPIC" ${platform_macro} "-Wall" "-O3")

macro(replace_cxx_release_flag src_flag dest_flag)
    string(REPLACE "${src_flag}" "${dest_flag}" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
endmacro(replace_cxx_release_flag flag)

replace_cxx_release_flag("-O3" "-O2")
if (${DDB_USE_ASAN})
    message("enable asan")
    add_compile_options(
        "-fsanitize=address"                # Enable ASAN.
        "-fno-omit-frame-pointer"           # Nicer stack traces in error messages.
        "-fno-optimize-sibling-calls"       # Disable tail call elimination (perfect stack traces if inlining off).
        )
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "8")
            add_compile_options("-fsanitize-recover=address")
            message("enable asan recover")
        endif()
    endif()
    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-fsanitize=address")
endif ()
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
    )

add_custom_command( TARGET PluginParquet POST_BUILD
                    COMMAND cp ${plugin_file} "${CMAKE_BINARY_DIR}/PluginParquet.txt"
                    COMMENT "copy PluginParquet.txt")


cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME INSTALL_DIR)
install(TARGETS ${PluginName} DESTINATION ${INSTALL_DIR})
install(FILES ${CMAKE_BINARY_DIR}/${PluginName}.txt DESTINATION ${INSTALL_DIR})
