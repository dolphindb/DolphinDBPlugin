cmake_minimum_required(VERSION 3.0)
project(PluginFormatArrow)
add_definitions(-DLOCKFREE_SYMBASE)

if (${DDB_USE_JIT})
    message("building with DOLPHINDB_JIT")
    add_compile_options("-DDOLPHINDB_JIT")
endif ()

if (WIN32 OR MINGW)
    set (platform_macro "-DWINDOWS")
    set (plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginFormatArrow.txt")
elseif (UNIX)
    set (platform_macro "-DLINUX")
    if (DEFINED USE_ABI_1)
        add_compile_options("-D_GLIBCXX_USE_CXX11_ABI=1")
    else()
        add_compile_options("-D_GLIBCXX_USE_CXX11_ABI=0")
    endif()
    set (plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginFormatArrow.txt")
endif()

message("platform: ${platform_macro}")
message("use_abi_1: ${USE_ABI_1}")

if(DEFINED DEBUG)
    add_compile_options("-std=c++11" "-DDEBUG" "-fPIC" ${platform_macro} "-Wall" "-g" "-O0")
else()
    add_compile_options("-std=c++11" "-fPIC" ${platform_macro} "-Wall" "-g" "-O3")
endif()

include_directories(${CMAKE_SOURCE_DIR}/../include)
include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_BINARY_DIR})

add_library(PluginFormatArrow SHARED ./src/FormatArrow.h ./src/FormatArrow.cpp)
set_target_properties(PluginFormatArrow PROPERTIES LINK_FLAGS "-Wl,-rpath,$ORIGIN")
target_link_libraries(PluginFormatArrow
    arrow
    pthread
    DolphinDB
    )

if(UNIX)
add_custom_command( TARGET PluginFormatArrow POST_BUILD 
                    COMMAND cp ${plugin_file} "${CMAKE_BINARY_DIR}/PluginFormatArrow.txt" 
                    COMMENT "copy PluginFormatArrow.txt")
endif()
