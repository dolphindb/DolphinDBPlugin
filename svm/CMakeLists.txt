cmake_minimum_required(VERSION 3.0)
project(PluginSVM LANGUAGES CXX)

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
set(destPluginTxt "PluginSVM.txt")

# Platform
if (UNIX)
    MESSAGE(STATUS "Platform: UNIX")
    list(APPEND PLUGIN_COMPILE_DEFINITIONS "-DLINUX")
    set(installType linux)
    set(pluginFileTxt "${CMAKE_CURRENT_LIST_DIR}/PluginSVM.txt")
    list(APPEND PLUGIN_COMPILE_DEFINITIONS "-D_GLIBCXX_USE_CXX11_ABI=0")
elseif (WIN32)
    MESSAGE(STATUS "Platform: WINDOWS")
    list(APPEND PLUGIN_COMPILE_DEFINITIONS "-DWINDOWS")
    set(installType win)
    set(pluginFileTxt "${CMAKE_CURRENT_LIST_DIR}/PluginSVM_win.txt")
else ()
    message(FATAL_ERROR "Unsupported platform.")
endif ()

# Cmake Build Type & Asan
include(../buildSetup.cmake)

# Source Code & Options & Dependencies
if (UNIX)
    file(GLOB PLUGINSVM_HEADER CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.h)
    file(GLOB PLUGINSVM_SOURCE CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cpp)
elseif (WIN32)
    file(GLOB PLUGINSVM_HEADER CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.h)
    set(PLUGINSVM_SOURCE ${PROJECT_SOURCE_DIR}/src/svmPlugin.cpp)
    set(extraBin "${CMAKE_CURRENT_LIST_DIR}/lib/win/")
    list(APPEND PLUGIN_LINK_LIBRARIES svm DolphinDB)
endif ()

# Build Target
add_library(PluginSVM SHARED)
set_property(TARGET PluginSVM PROPERTY CXX_STANDARD 11)
target_sources(PluginSVM PUBLIC ${PLUGINSVM_HEADER} PRIVATE ${PLUGINSVM_SOURCE})
target_include_directories(PluginSVM PRIVATE ${PLUGIN_INCLUDE_DIRECTORIES})
target_link_directories(PluginSVM PRIVATE ${PLUGIN_LINK_DIRECTORIES})
target_compile_definitions(PluginSVM PRIVATE ${PLUGIN_COMPILE_DEFINITIONS})
target_compile_options(PluginSVM PRIVATE ${PLUGIN_COMPILE_OPTIONS})
target_link_libraries(PluginSVM ${PLUGIN_LINK_OPTIONS} ${PLUGIN_LINK_LIBRARIES})

# Post-build
install(TARGETS PluginSVM
        DESTINATION "${PROJECT_SOURCE_DIR}/bin/${installType}"
)
install(FILES ${pluginFileTxt}
    DESTINATION "${PROJECT_SOURCE_DIR}/bin/${installType}"
    RENAME ${destPluginTxt}
)
if (WIN32)
    configure_file(${CMAKE_CURRENT_LIST_DIR}/lib/win/libsvm.dll ${CMAKE_BINARY_DIR}/libsvm.dll COPYONLY)
    install(DIRECTORY ${extraBin}
        DESTINATION "${PROJECT_SOURCE_DIR}/bin/${installType}"
    )
endif()
message(STATUS "Install Destination: ${PROJECT_SOURCE_DIR}/bin/${installType}")

configure_file(${pluginFileTxt} ${CMAKE_BINARY_DIR}/${destPluginTxt} COPYONLY)