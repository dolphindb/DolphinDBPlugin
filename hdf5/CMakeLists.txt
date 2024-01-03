cmake_minimum_required(VERSION 3.0)
project(PluginHdf5 LANGUAGES CXX)

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

set(PLUGIN_COMPILE_DEFINITIONS
    "-DLOCKFREE_SYMBASE"
)
set(PLUGIN_COMPILE_OPTIONS
    "-std=c++11"
    "-Wall"
    "-fPIC"
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

set(destPluginTxt "PluginHdf5.txt")

# Platform
if (UNIX)
    MESSAGE(STATUS "Platform: UNIX")
    list(APPEND PLUGIN_COMPILE_DEFINITIONS "-DLINUX" "-D_GLIBCXX_USE_CXX11_ABI=0")
    set(installType linux)
    set(pluginFileTxt "${CMAKE_CURRENT_LIST_DIR}/PluginHdf5.txt")
elseif (WIN32)
    MESSAGE(STATUS "Platform: WINDOWS")
    list(APPEND PLUGIN_COMPILE_DEFINITIONS "-DWINDOWS")
    set(installType win)
    set(pluginFileTxt "${CMAKE_CURRENT_LIST_DIR}/PluginHdf5_win.txt")
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
file(GLOB PluginHdf5_HEADER CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.h)
file(GLOB PluginHdf5_SOURCE CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/src/blosc/*.cpp)


message(${PluginHdf5_HEADER})
# Options & Dependencies
list(APPEND PLUGIN_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/../include/ddbplugin ${PROJECT_SOURCE_DIR}/src)
    list(APPEND PLUGIN_COMPILE_OPTIONS "-fno-strict-aliasing")
if (UNIX)
    list(APPEND PLUGIN_INCLUDE_DIRECTORIES
         ${CMAKE_SOURCE_DIR}/include/hdf5
         ${CMAKE_SOURCE_DIR}/include/c-blosc
         ${CMAKE_SOURCE_DIR}/include/hdf5-blosc
        )
    list(APPEND PLUGIN_LINK_DIRECTORIES ${CMAKE_SOURCE_DIR}/lib)
    list(APPEND PLUGIN_LINK_LIBRARIES hdf5_hl hdf5_cpp hdf5 blosc dl z)
elseif (WIN32)
    list(APPEND PLUGIN_INCLUDE_DIRECTORIES
         ${CMAKE_SOURCE_DIR}/include_win/hdf5
         ${CMAKE_SOURCE_DIR}/include_win/c-blosc
         ${CMAKE_SOURCE_DIR}/include_win/hdf5-blosc
        )
    list(APPEND PLUGIN_LINK_DIRECTORIES ${CMAKE_SOURCE_DIR}/lib)
    list(APPEND PLUGIN_LINK_LIBRARIES hdf5_hl hdf5_cpp hdf5 blosc z)
else ()
    message(FATAL_ERROR "Unsupported platform.")
endif ()

# Build Target
add_library(PluginHdf5 SHARED)
set_property(TARGET PluginHdf5 PROPERTY CXX_STANDARD 11)
message(${PluginHdf5_SOURCE})
target_sources(PluginHdf5 PUBLIC ${PluginHdf5_HEADER} PRIVATE ${PluginHdf5_SOURCE})
target_include_directories(PluginHdf5 PRIVATE ${PLUGIN_INCLUDE_DIRECTORIES})
target_link_directories(PluginHdf5 PRIVATE ${PLUGIN_LINK_DIRECTORIES})
target_compile_definitions(PluginHdf5 PRIVATE ${PLUGIN_COMPILE_DEFINITIONS})
target_compile_options(PluginHdf5 PRIVATE ${PLUGIN_COMPILE_OPTIONS})
target_link_libraries(PluginHdf5 ${PLUGIN_LINK_OPTIONS} ${PLUGIN_LINK_LIBRARIES})

# Post-build
install(TARGETS PluginHdf5
        DESTINATION "${PROJECT_SOURCE_DIR}/bin/${installType}"
)
install(FILES ${pluginFileTxt}
    DESTINATION "${PROJECT_SOURCE_DIR}/bin/${installType}"
    RENAME ${destPluginTxt}
)
message(STATUS "Install Destination: ${PROJECT_SOURCE_DIR}/bin/${installType}")

# Compatibale with Jenkins
add_custom_command(
    TARGET PluginHdf5 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${pluginFileTxt} ${PROJECT_SOURCE_DIR}/build/PluginHdf5.txt
    COMMENT "copy PluginHdf5.txt"
)
