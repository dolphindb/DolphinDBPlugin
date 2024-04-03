cmake_minimum_required(VERSION 3.0)
set(CMAKE_SKIP_RPATH TRUE)
project(PluginAWSS3)
add_definitions(-DLOCKFREE_SYMBASE)

set(CMAKE_CXX_STANDARD 11)

set(DOLPHIN_INCLUDE_DIR "../include" CACHE STRING "")
set(DOLPHIN_LIBRARY_DIR "/usr/local/lib" CACHE STRING "")
set(AWS_INCLUDE_DIR "/usr/local/include" CACHE STRING "")
set(AWS_LIBRARY_DIR "/usr/local/lib" CACHE STRING "")
set(ZLIB_INCLUDE_DIR "/usr/local/include" CACHE STRING "")
set(ZLIB_LIBRARY_DIR "/usr/local/lib" CACHE STRING "")

aux_source_directory(src PLUGINAWS_SRC)

include_directories(
	src
        ${AWS_INCLUDE_DIR}
        ${ZLIB_INCLUDE_DIR}
        ${DOLPHIN_INCLUDE_DIR})

if(WIN32)
    set(platform_macro "-DWINDOWS")
elseif(UNIX)
    set(platform_macro "-DLINUX")
endif()

add_compile_options(${platform_macro} -O0 -Wall -g -ggdb)

# if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
#     add_compile_options(${platform_macro} -O0 -Wall -g -ggdb)
#     message(STATUS "Build: Debug with -O0 -Wall -g -ggdb")
# else()
#     set(CMAKE_BUILD_TYPE Release)
#     add_compile_options(${platform_macro} -O3 -std=c++11)
#     message(STATUS "Build: Release with -O3")
# endif()

add_library(
	${PROJECT_NAME} SHARED
        ${PLUGINAWS_SRC})

target_link_libraries(
	${PROJECT_NAME}
        ${DOLPHIN_LIBRARY_DIR}/libDolphinDB.so
        ${AWS_LIBRARY_DIR}/libaws-cpp-sdk-s3.so
        ${AWS_LIBRARY_DIR}/libaws-cpp-sdk-core.so
        ${ZLIB_LIBRARY_DIR}/libz.so)
