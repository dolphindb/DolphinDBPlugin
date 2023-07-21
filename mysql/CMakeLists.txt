cmake_minimum_required(VERSION 3.0)
project(MySQL)
add_definitions(-DLOCKFREE_SYMBASE)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

## set compile flags
if (WIN32)
    set (platform_macro "-DWINDOWS")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginMySQL_win32.txt")
    include_directories("${CMAKE_BINARY_DIR}/curl/include")
elseif (UNIX)
    set (platform_macro "-DLINUX")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginMySQL.txt")
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
endif ()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${platform_macro} -fPIC -std=c++11 -Wall")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -DDEBUG_PARSE")
#add_definitions(-Weffc++)

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "RELEASE")
endif ()

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
message("CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})

if("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fPIC")
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RELWITHDEBINFO")  # release with debug info
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2")
else ()
    message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}")
endif ()

if (${DDB_USE_ASAN})
    add_compile_options(
        "-fsanitize=address"                # Enable ASAN.
        "-fno-omit-frame-pointer"           # Nicer stack traces in error messages.
        "-fno-optimize-sibling-calls"       # Disable tail call elimination (perfect stack traces if inlining off).
        )
    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-fsanitize=address")
endif ()

## set include dires
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
link_directories("${CMAKE_BINARY_DIR}")

## build mysqlclient
add_subdirectory(contrib/mariadb-connector-c-cmake)

## build PluginMySQL
add_library(PluginMySQL SHARED
        src/mysqlxx.cpp
        src/plugin_mysql.cpp)

set_target_properties(PluginMySQL PROPERTIES LINK_DIRECTORIES "${CMAKE_BINARY_DIR}")
target_link_libraries(PluginMySQL DolphinDB mysqlclient)

configure_file(${plugin_file} ${CMAKE_BINARY_DIR}/PluginMySQL.txt COPYONLY)
