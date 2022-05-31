cmake_minimum_required(VERSION 3.0)
project(PluginSVM)
add_definitions(-DLOCKFREE_SYMBASE)

set(CMAKE_CXX_STANDARD 11)

set(DOLPHIN_INCLUDE_DIR "../include")
set(DOLPHIN_LIBRARY_DIR "/usr/lib")

aux_source_directory(src PLUGINSVM_SRC)

include_directories(
    src
    ${DOLPHIN_INCLUDE_DIR}
)

if(WIN32)
    set(platform_macro "-DWINDOWS")
elseif(UNIX)
    set(platform_macro "-DLINUX")
endif()

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    add_compile_options(${platform_macro} -O0 -Wall -g -ggdb -D_GLIBCXX_USE_CXX11_ABI=0)
    message(STATUS "Build: Debug with -O0 -Wall -g -ggdb -D_GLIBCXX_USE_CXX11_ABI=0")
else()
    set(CMAKE_BUILD_TYPE Release)
    add_compile_options(${platform_macro} -O3 -D_GLIBCXX_USE_CXX11_ABI=0)
    message(STATUS "Build: Release with -O3 -D_GLIBCXX_USE_CXX11_ABI=0")
endif()

add_library(
    ${PROJECT_NAME} SHARED
    ${PLUGINSVM_SRC})

target_link_libraries(
    ${PROJECT_NAME}
    ${DOLPHIN_LIBRARY_DIR}/libDolphinDB.so
)


