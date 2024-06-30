cmake_minimum_required(VERSION 3.0)
project(PluginAmdQuote LANGUAGES CXX)

#######################################################################################################################
# CMake options and global variables.
#
# CMake build types, specify with -DCMAKE_BUILD_TYPE={option}.
#   Debug, Release (default), RelWithDebInfo.
#
# CMake options, specify with -DDDB_{option}=On.
#   DDB_USE_ASAN                    : Enable ASAN, a fast memory error detector. Default OFF.
#
# CMake custom variables, specify with -D{variable}={value}.
#   AMD_VERSION                     : use to determine AmdQuote lib version, only support '3.9.6'
#   AMDAPIDIR                       : use to specify the directory of AmdQuote sdk.
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
)
set(PLUGIN_LINK_OPTIONS
    "-L ./ -Wl,-rpath,$ORIGIN"
)

set(destPluginTxt "PluginAmdQuote.txt")
set(CMAKE_SKIP_RPATH TRUE)

# Platform
if (UNIX)
    MESSAGE(STATUS "Platform: UNIX")
    list(APPEND PLUGIN_COMPILE_DEFINITIONS "-DLINUX")
    set(installType linux)
    if("${AMD_VERSION}" STREQUAL "3.9.6")
        add_definitions(-DAMD_3_9_6)
        set(pluginFileTxt "${CMAKE_CURRENT_LIST_DIR}/PluginAmdQuote3.9.6.txt")
    else()
        set(pluginFileTxt "${CMAKE_CURRENT_LIST_DIR}/PluginAmdQuote.txt")
    endif()

    if("${AMD_VERSION}" STREQUAL "4.0.1")
        add_definitions(-DAMD_TRADE_ORDER_VOLUME)
    elseif("${AMD_VERSION}" STREQUAL "4.2.0")
        add_definitions(-DAMD_TRADE_ORDER_VOLUME)
    elseif("${AMD_VERSION}" STREQUAL "4.3.0")
        add_definitions(-DAMD_TRADE_ORDER_VOLUME)
    endif()

elseif (WIN32)
    message(FATAL_ERROR "Unsupported platform: Windows.")
else ()
    message(FATAL_ERROR "Unsupported platform.")
endif ()

if(DEFINED AMD_USE_ABI)
    list(APPEND PLUGIN_COMPILE_DEFINITIONS "-D_GLIBCXX_USE_CXX11_ABI=1")
    message(STATUS "Use ABI=1")
else()
    list(APPEND PLUGIN_COMPILE_DEFINITIONS "-D_GLIBCXX_USE_CXX11_ABI=0")
    message(STATUS "Use ABI=0")
endif()


# Cmake Build Type
# ASAN
include(../buildSetup.cmake)

# Source Code
file(GLOB PLUGINAMD_HEADER CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.h)
file(GLOB PLUGINAMD_SOURCE CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cpp)

# Options & Dependencies
if(DEFINED AMDAPIDIR)
    set(amdSourceDir ${AMDAPIDIR})
else()
    set(amdSourceDir ${CMAKE_SOURCE_DIR})
endif()
message(STATUS "AMD_SOURCE_DIR: ${amdSourceDir}")

list(APPEND PLUGIN_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/../include/ddbplugin ${amdSourceDir}/include ${PROJECT_SOURCE_DIR}/src/)
list(APPEND PLUGIN_COMPILE_OPTIONS -rdynamic)
list(APPEND PLUGIN_LINK_DIRECTORIES ${amdSourceDir}/lib)
list(APPEND PLUGIN_LINK_LIBRARIES aaf adk ama ami amigrpc amigrpc++ das entry_wrapper fmdutil llmi rmm rum sample_engine z)
list(APPEND PLUGIN_LINK_OPTIONS "-Wl,-rpath,$ORIGIN,-disable-new-dtags")

set(extraBin "${amdSourceDir}/lib/")

# Build Target
add_library(PluginAmdQuote SHARED)
set_property(TARGET PluginAmdQuote PROPERTY CXX_STANDARD 11)
target_sources(PluginAmdQuote PUBLIC ${PLUGINAMD_HEADER} PRIVATE ${PLUGINAMD_SOURCE})

target_include_directories(PluginAmdQuote PRIVATE ${PLUGIN_INCLUDE_DIRECTORIES})
target_link_directories(PluginAmdQuote PRIVATE ${PLUGIN_LINK_DIRECTORIES})
target_compile_definitions(PluginAmdQuote PRIVATE ${PLUGIN_COMPILE_DEFINITIONS})
target_compile_options(PluginAmdQuote PRIVATE ${PLUGIN_COMPILE_OPTIONS})
target_link_libraries(PluginAmdQuote ${PLUGIN_LINK_OPTIONS} ${PLUGIN_LINK_LIBRARIES})

# Post-build
if (WIN32)
elseif (UNIX)
    install(TARGETS PluginAmdQuote
         DESTINATION "${PROJECT_SOURCE_DIR}/bin/${installType}"
    )
    install(DIRECTORY ${extraBin}
        DESTINATION "${PROJECT_SOURCE_DIR}/bin/${installType}"
    )
    install(FILES ${pluginFileTxt}
        DESTINATION "${PROJECT_SOURCE_DIR}/bin/${installType}"
        RENAME ${destPluginTxt}
    )
    message(STATUS "Install Destination: ${PROJECT_SOURCE_DIR}/bin/${installType}")
endif()

# Compatibale with Jenkins
configure_file(${pluginFileTxt} ${CMAKE_BINARY_DIR}/${destPluginTxt} COPYONLY)
