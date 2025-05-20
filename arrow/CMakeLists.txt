cmake_minimum_required(VERSION 3.13)
set(CMAKE_SKIP_RPATH TRUE)
set(PluginName PluginArrow)
project(${PluginName})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (${DDB_USE_JIT})
    message("building with DOLPHINDB_JIT")
    add_compile_options("-DDOLPHINDB_JIT")
endif ()

if (WIN32 OR MINGW)
    set (platform_macro "-DWINDOWS")
    set (plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginArrow.txt")
elseif (UNIX)
    set (platform_macro "-DLINUX")
    set (plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginArrow.txt")
    add_compile_options("-fPIC")
endif()

message("platform: ${platform_macro}")
message("use_abi_1: ${USE_ABI_1}")

if(DEFINED DEBUG)
    add_compile_options("-std=c++11" "-DDEBUG" ${platform_macro} "-g" "-O0")
else()
    add_compile_options("-std=c++11" ${platform_macro} "-g" "-O3")
endif()


if (${DDB_USE_ASAN})
    add_compile_options(
        "-fsanitize=address"                # Enable ASAN.
        "-fno-omit-frame-pointer"           # Nicer stack traces in error messages.
        "-fno-optimize-sibling-calls"       # Disable tail call elimination (perfect stack traces if inlining off).
        )
    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-fsanitize=address")
endif ()
message("use_asan: ${DDB_USE_ASAN}")


include_directories(${CMAKE_SOURCE_DIR}/../include)
include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_BINARY_DIR})

add_library(PluginArrow SHARED ./src/FormatArrow.h ./src/FormatArrow.cpp)
target_compile_options(${PluginName} PRIVATE -Wall -Werror)
# We can ignore this warning as long as this plugin is built with the same compiler as DolphinDB
target_compile_options(${PluginName} PRIVATE -Wno-return-type-c-linkage)

set_target_properties(PluginArrow PROPERTIES LINK_FLAGS "-Wl,-rpath,$ORIGIN")

if (WIN32)
    target_link_libraries(PluginArrow
        arrow
        pthread
        DolphinDB
    )
elseif (UNIX)
    target_link_libraries(PluginArrow
        arrow
        pthread
    )
endif ()


if(UNIX)
add_custom_command( TARGET PluginArrow POST_BUILD 
                    COMMAND cp ${plugin_file} "${CMAKE_BINARY_DIR}/PluginArrow.txt" 
                    COMMENT "copy PluginArrow.txt")
endif()
