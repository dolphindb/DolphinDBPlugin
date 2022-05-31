cmake_minimum_required(VERSION 3.0)
project(PluginHBase)
add_definitions(-DLOCKFREE_SYMBASE)

set(CMAKE_CXX_STANDARD 11)

aux_source_directory(src PLUGINPREDICT_SRC)

if(WIN32)
    set(platform_macro "-DWINDOWS")
elseif(UNIX)
    set(platform_macro "-DLINUX")
endif()

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    add_compile_options(${platform_macro} -O0 -g -D_GLIBCXX_USE_CXX11_ABI=0)
    message(STATUS "Build: Debug with -O0 -Wall -g -ggdb")
else()
    set(CMAKE_BUILD_TYPE Release)
    add_compile_options(${platform_macro} -O3 -D_GLIBCXX_USE_CXX11_ABI=0)
    message(STATUS "Build: Release with -O3")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${platform_macro} -fPIC -std=c++11 -Wall")

include_directories(${PROJECT_SOURCE_DIR}/../include src)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/thrift)
link_directories(${CMAKE_BINARY_DIR})
link_directories(${PROJECT_SOURCE_DIR}/lib)

add_library(
	${PROJECT_NAME} SHARED
    ${PLUGINPREDICT_SRC})

target_link_libraries(${PROJECT_NAME}
    thrift
    DolphinDB
	)

