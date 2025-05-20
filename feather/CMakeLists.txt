cmake_minimum_required(VERSION 3.13)
set(PluginName PluginFeather)
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
    "-fPIC"
)
set(PLUGIN_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)
set(PLUGIN_LINK_DIRECTORIES
    ${CMAKE_BINARY_DIR}
)
set(PLUGIN_LINK_LIBRARIES
)
set(PLUGIN_LINK_OPTIONS
    "-L ./ -Wl,-rpath,$ORIGIN"
)

set(destPluginTxt "PluginFeather.txt")

# Platform
if (UNIX)
    MESSAGE(STATUS "Platform: UNIX")
    list(APPEND PLUGIN_COMPILE_DEFINITIONS "-DLINUX")
    set(installType linux)
    set(pluginFileTxt "${CMAKE_CURRENT_LIST_DIR}/PluginFeather.txt")
elseif (WIN32)
    MESSAGE(STATUS "Platform: WINDOWS")
    list(APPEND PLUGIN_COMPILE_DEFINITIONS "-DWINDOWS")
    set(installType win)
    set(extraBin ${CMAKE_CURRENT_SOURCE_DIR}/lib/win/)
    set(pluginFileTxt "${CMAKE_CURRENT_LIST_DIR}/PluginFeather_win.txt")
else ()
    message(FATAL_ERROR "Unsupported platform.")
endif ()

# Cmake Build Type
# ASAN
include(../buildSetup.cmake)

# Source Code
file(GLOB PluginFeather_HEADER CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)
file(GLOB PluginFeather_SOURCE CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# Options & Dependencies
if (UNIX)
    list(APPEND PLUGIN_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/../include/ddbplugin ${CMAKE_SOURCE_DIR}/include)
    list(APPEND PLUGIN_LINK_DIRECTORIES ${CMAKE_SOURCE_DIR}/lib/linux)
    list(APPEND PLUGIN_LINK_LIBRARIES libarrow.a liblz4.a libz.a libzstd.a libjemalloc_pic.a pthread)
elseif (WIN32)
    list(APPEND PLUGIN_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/../include/ddbplugin ${CMAKE_SOURCE_DIR}/include)
    list(APPEND PLUGIN_LINK_DIRECTORIES ${CMAKE_SOURCE_DIR}/lib/win)
    list(APPEND PLUGIN_LINK_LIBRARIES DolphinDB libarrow.dll liblz4.dll zlib.dll libzstd.dll)
endif ()

# Build Target
add_library(PluginFeather SHARED)
set_property(TARGET PluginFeather PROPERTY CXX_STANDARD 11)
target_sources(PluginFeather PUBLIC ${PluginFeather_HEADER} PRIVATE ${PluginFeather_SOURCE})

target_include_directories(PluginFeather PRIVATE ${PLUGIN_INCLUDE_DIRECTORIES})
target_link_directories(PluginFeather PRIVATE ${PLUGIN_LINK_DIRECTORIES})
target_compile_definitions(PluginFeather PRIVATE ${PLUGIN_COMPILE_DEFINITIONS})
target_compile_options(PluginFeather PRIVATE ${PLUGIN_COMPILE_OPTIONS})
target_link_libraries(PluginFeather ${PLUGIN_LINK_OPTIONS} ${PLUGIN_LINK_LIBRARIES})

# Post-build
if (WIN32)
    install(DIRECTORY ${extraBin}
        DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/bin/${installType}"
    )
elseif (UNIX)
endif()

install(TARGETS PluginFeather
        DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/bin/${installType}"
)
install(FILES ${pluginFileTxt}
    DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/bin/${installType}"
    RENAME ${destPluginTxt}
)
message(STATUS "Install Destination: ${CMAKE_CURRENT_SOURCE_DIR}/bin/${installType}")

# Compatibale with Jenkins
add_custom_command(
    TARGET PluginFeather POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${pluginFileTxt} ${CMAKE_CURRENT_SOURCE_DIR}/build/PluginFeather.txt
    COMMENT "copy PluginFeather.txt"
)
