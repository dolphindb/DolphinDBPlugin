cmake_minimum_required(VERSION 3.00)
project(ODBC)
add_definitions(-DLOCKFREE_SYMBASE)

add_definitions("-DLOGGING_LEVEL_2")

if(WIN32 OR MINGW)
    add_definitions("-DWINDOWS -D_WIN32_WINNT=0x0600 -DWINVER=0x0600")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginODBC_win.txt")
elseif(UNIX)
    if (UNIXODBCDIR)
        include_directories(${UNIXODBCDIR}/include)
    endif()
    add_definitions("-DLINUX -DNANODBC_ODBC_VERSION=SQL_OV_ODBC3")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginODBC.txt")
endif()

if(WIN32 OR MINGW)
elseif(UNIX)
    if(ENABLEABI MATCHES 1)
        message("ENABLEABI")
        add_definitions("-D_GLIBCXX_USE_CXX11_ABI=1")
    else()
        message("DISENABLEABI")
        add_definitions("-D_GLIBCXX_USE_CXX11_ABI=0")
    endif()
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "RELEASE")
endif ()

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
message("CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})

if("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -fPIC -std=c++11 -Wall")
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fPIC -std=c++11 -Wall")
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RELWITHDEBINFO")  # release with debug info
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fPIC -O2 -Wall")
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

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/../include)
link_directories(${CMAKE_BINARY_DIR})

aux_source_directory(${PROJECT_SOURCE_DIR}/src  DIR_SRCS)

add_library(PluginODBC SHARED ${DIR_SRCS})


if(WIN32 OR MINGW)
    target_link_libraries(PluginODBC DolphinDB odbc32 odbccp32)
elseif(UNIX)
    if(UNIXODBCDIR)
        target_link_libraries(PluginODBC DolphinDB ${UNIXODBCDIR}/lib/libodbc.a)
    else()
        target_link_libraries(PluginODBC DolphinDB odbc)
    endif()
endif()

configure_file(${plugin_file} ${CMAKE_BINARY_DIR}/PluginODBC.txt COPYONLY)
