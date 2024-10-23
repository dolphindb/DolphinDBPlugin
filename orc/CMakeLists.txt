cmake_minimum_required(VERSION 3.0)
project(PluginOrc LANGUAGES CXX)

#######################################################################################################################
# CMake options and global variables.
#
# CMake build types, specify with -DCMAKE_BUILD_TYPE={option}.
#   Debug, Release (default), RelWithDebInfo.
#
# CMake options, specify with -D{option}=On.
#   DDB_USE_ASAN                    : Enable ASAN, a fast memory error detector. Default OFF.
#   BUILD_ARM                       : Enable ARM build.
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
option(BUILD_ARM
    "Enable ARM build"
    OFF)

set(PLUGIN_COMPILE_DEFINITIONS
    "-DLOCKFREE_SYMBASE"
    "-D_GLIBCXX_USE_CXX11_ABI=0"
)
set(PLUGIN_COMPILE_OPTIONS
    "-std=c++11"
    "-Wall"
)
set(PLUGIN_INCLUDE_DIRECTORIES
    ${PROJECT_SOURCE_DIR}/../include
)
set(PLUGIN_LINK_DIRECTORIES
    ${CMAKE_BINARY_DIR}
)
set(PLUGIN_LINK_LIBRARIES
    DolphinDB
)
set(PLUGIN_LINK_OPTIONS
    "-L ./ -Wl,-rpath,$ORIGIN"
)

set(destPluginTxt "PluginOrc.txt")

# Platform
if (UNIX)
    MESSAGE(STATUS "Platform: UNIX")
    list(APPEND PLUGIN_COMPILE_DEFINITIONS "-DLINUX")
    set(installType linux)
    set(pluginFileTxt "${CMAKE_CURRENT_LIST_DIR}/PluginOrc.txt")
elseif (WIN32)
    message(FATAL_ERROR "Unsupported platform WINDOWS.")
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
file(GLOB PluginOrc_HEADER CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.h)
file(GLOB PluginOrc_SOURCE CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cpp)

# Options & Dependencies
list(APPEND PLUGIN_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/include)

message(STATUS "BUILD ARM: ${BUILD_ARM}")
if(${BUILD_ARM})
    list(APPEND PLUGIN_COMPILE_OPTIONS -fsigned-char -fPIC)
    list(APPEND PLUGIN_LINK_LIBRARIES orc protobuf snappy z zstd)
else ()
    list(APPEND PLUGIN_LINK_DIRECTORIES lib/linux /usr/lib64)
    list(APPEND PLUGIN_LINK_LIBRARIES orc protobuf snappy lz4 z zstd hdfspp_static pthread sasl2 crypto)
endif ()

# Build Target
add_library(PluginOrc SHARED)
set_property(TARGET PluginOrc PROPERTY CXX_STANDARD 11)
target_sources(PluginOrc PUBLIC ${PluginOrc_HEADER} PRIVATE ${PluginOrc_SOURCE})

target_include_directories(PluginOrc PRIVATE ${PLUGIN_INCLUDE_DIRECTORIES})
target_link_directories(PluginOrc PRIVATE ${PLUGIN_LINK_DIRECTORIES})
target_compile_definitions(PluginOrc PRIVATE ${PLUGIN_COMPILE_DEFINITIONS})
target_compile_options(PluginOrc PRIVATE ${PLUGIN_COMPILE_OPTIONS})
target_link_libraries(PluginOrc ${PLUGIN_LINK_OPTIONS} ${PLUGIN_LINK_LIBRARIES})

# Post-build
if (WIN32)
elseif (UNIX)
    install(TARGETS PluginOrc
         DESTINATION "${PROJECT_SOURCE_DIR}/bin/${installType}"
    )
    install(FILES ${pluginFileTxt}
        DESTINATION "${PROJECT_SOURCE_DIR}/bin/${installType}"
        RENAME ${destPluginTxt}
    )
    message(STATUS "Install Destination: ${PROJECT_SOURCE_DIR}/bin/${installType}")
endif()

# compatible with jenkins
set(plugin_file ${CMAKE_CURRENT_LIST_DIR}/PluginOrc.txt)
add_custom_command(
    TARGET PluginOrc POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${plugin_file} ${CMAKE_BINARY_DIR}/PluginOrc.txt
    COMMENT "copy PluginOrc.txt"
)
