cmake_minimum_required(VERSION 3.13)
set(PluginName PluginGurobi)
project(${PluginName})

#######################################################################################################################
# CMake options and global variables.
#
# CMake build types, specify with -DCMAKE_BUILD_TYPE={option}.
#   Debug, Release (default), RelWithDebInfo.
#
# CMake options, specify with -DDDB_{option}=On.
#   DDB_USE_ASAN                    : Enable ASAN, a fast memory error detector. Default OFF.
#
# CMake global variables. These are NOT CMake options, i.e., these variables are internal.
#   PLUGIN_COMPILE_DEFINITIONS      : Compile definitions to be added to plugin.
#   PLUGIN_COMPILE_OPTIONS          : Compile options to be added to plugin.
#   PLUGIN_INCLUDE_DIRECTORIES      : Include directories to be used for plugin.
#   PLUGIN_LINK_DIRECTORIES         : Link directories to be used for plugin.
#   PLUGIN_LINK_LIBRARIES           : Link libraries to be added to plugin.
#   PLUGIN_LINK_OPTIONS             : Link options to be added to plugin.
#######################################################################################################################

# Initial setup
option(DDB_USE_ASAN
        "Enable ASAN, a fast memory error detector. https://clang.llvm.org/docs/AddressSanitizer.html"
        OFF)
set(PLUGIN_COMPILE_OPTIONS
        "-std=c++11"
        "-Wall"
)
set(PLUGIN_INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)
set(PLUGIN_LINK_DIRECTORIES
        ${CMAKE_BINARY_DIR}
)
set(PLUGIN_LINK_LIBRARIES)
set(PLUGIN_LINK_OPTIONS
        "-L ./ -Wl,-rpath,$ORIGIN"
)

set(destPluginTxt "PluginGurobi.txt")

# Platform
if (UNIX)
    MESSAGE(STATUS "Platform: UNIX")
    list(APPEND PLUGIN_COMPILE_DEFINITIONS "-DLINUX")
    set(installType linux)
    set(pluginFileTxt "${CMAKE_CURRENT_LIST_DIR}/PluginGurobi.txt")
elseif (WIN32)
    MESSAGE(STATUS "Platform: WINDOWS")
    list(APPEND PLUGIN_COMPILE_DEFINITIONS "-DWINDOWS")
    set(installType win)
    set(pluginFileTxt "${CMAKE_CURRENT_LIST_DIR}/PluginGurobi_win32.txt")
else ()
    message(FATAL_ERROR "Unsupported platform.")
endif ()

# Cmake Build Type
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RELEASE)
endif (NOT CMAKE_BUILD_TYPE)

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
if("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
    list(APPEND PLUGIN_COMPILE_OPTIONS "-ggdb" "-O0" "-fno-omit-frame-pointer" "-fno-optimize-sibling-calls")
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
    list(APPEND PLUGIN_COMPILE_DEFINITIONS "-DNDEBUG")
    list(APPEND PLUGIN_COMPILE_OPTIONS "-O3")
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RELWITHDEBINFO")
    list(APPEND PLUGIN_COMPILE_DEFINITIONS "-DNDEBUG")
    list(APPEND PLUGIN_COMPILE_OPTIONS "-ggdb" "-O2")
else ()
    message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}")
endif ()
message(STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})

# ASAN
if (${DDB_USE_ASAN})
    set(DDB_ASAN_FLAGS
            "-fsanitize=address"                # Enable ASAN.
            "-fno-omit-frame-pointer"           # Nicer stack traces in error messages.
            "-fno-optimize-sibling-calls"       # Disable tail call elimination (perfect stack traces if inlining off).
    )
    list(APPEND PLUGIN_COMPILE_OPTIONS ${DDB_ASAN_FLAGS})
    # list(APPEND PLUGIN_LINK_OPTIONS "-fsanitize=address")
    unset(DDB_ASAN_FLAGS)
endif ()
message(STATUS "ASAN: ${DDB_USE_ASAN}")

# Source Code
file(GLOB PluginGurobi_HEADER CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)
file(GLOB PluginGurobi_SOURCE CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# Options & Dependencies
list(APPEND PLUGIN_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/contrib/include)
list(APPEND PLUGIN_LINK_OPTIONS "-fvisibility=hidden -Wl,-Bsymbolic -Wl,-rpath,$ORIGIN,-disable-new-dtags")
list(APPEND PLUGIN_LINK_DIRECTORIES ${CMAKE_SOURCE_DIR}/contrib/lib)
list(APPEND PLUGIN_LINK_LIBRARIES libgurobi_c++.a gurobi100)

# Build Target
add_library(PluginGurobi SHARED)
set_property(TARGET PluginGurobi PROPERTY CXX_STANDARD 11)
target_sources(PluginGurobi PUBLIC ${PluginGurobi_HEADER} PRIVATE ${PluginGurobi_SOURCE})

target_include_directories(PluginGurobi PRIVATE ${PLUGIN_INCLUDE_DIRECTORIES})
target_link_directories(PluginGurobi PRIVATE ${PLUGIN_LINK_DIRECTORIES})
target_compile_definitions(PluginGurobi PRIVATE ${PLUGIN_COMPILE_DEFINITIONS})
target_compile_options(PluginGurobi PRIVATE ${PLUGIN_COMPILE_OPTIONS})
target_link_libraries(PluginGurobi ${PLUGIN_LINK_OPTIONS} ${PLUGIN_LINK_LIBRARIES})

# Post-build
if (WIN32)
elseif (UNIX)
    install(TARGETS PluginGurobi
            DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/bin/${installType}"
    )
    install(FILES ${pluginFileTxt}
            DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/bin/${installType}"
            RENAME ${destPluginTxt}
    )
    install(FILES "${CMAKE_CURRENT_LIST_DIR}/contrib/lib/libgurobi100.so"
            DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/bin/${installType}"
    )
    message(STATUS "Install Destination: ${CMAKE_CURRENT_SOURCE_DIR}/bin/${installType}")
endif()
