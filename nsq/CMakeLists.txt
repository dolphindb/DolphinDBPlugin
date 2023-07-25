cmake_minimum_required(VERSION 3.0)
project(PluginNsq)
add_definitions(-DLOCKFREE_SYMBASE)

set(CMAKE_CXX_STANDARD 11)

set(DOLPHIN_INCLUDE_DIR "../include")

aux_source_directory(src PLUGINNsq_SRC)

include_directories(
    src
    ${DOLPHIN_INCLUDE_DIR}
    include
)

set(BASE_DIR ${PROJECT_SOURCE_DIR})
link_directories(${BASE_DIR}/lib/linux.x64)
link_directories("${CMAKE_BINARY_DIR}")

if(WIN32)
    set(platform_macro "-DWINDOWS")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginNsq_win.txt")
elseif(UNIX)
    set(platform_macro "-DLINUX")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginNsq.txt")
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "RELEASE")
endif ()

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
message("CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})

if("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${platform_macro} -O0 -Wall -fPIC -g -ggdb")
    message(STATUS "Build: Debug with -O0 -Wall -g -ggdb")
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${platform_macro} -O3 -g -fPIC -Wall")
    message(STATUS "Build: Release with -O3")
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RELWITHDEBINFO")  # release with debug info
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${platform_macro} -g -fPIC -O2 -Wall")
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

if(UNIX)
    add_compile_options(-D_GLIBCXX_USE_CXX11_ABI=0)
endif()

add_library(
    ${PROJECT_NAME} SHARED
    ${PLUGINNsq_SRC})

target_link_libraries(
    ${PROJECT_NAME}
    DolphinDB
    HSNsqApi
)

if(UNIX)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,-rpath,$ORIGIN")
endif()

configure_file(${plugin_file} ${CMAKE_BINARY_DIR}/PluginNsq.txt COPYONLY)

