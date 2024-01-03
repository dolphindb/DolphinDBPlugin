cmake_minimum_required(VERSION 3.0)
project(PluginMQTTClient)
add_definitions(-DLOCKFREE_SYMBASE)

set(CMAKE_CXX_STANDARD 11)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/")

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/ PluginSrc)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/lib/ MQTTSrc)

# set(CMAKE_VERBOSE_MAKEFILE ON)
if (WIN32)
    set (platform_macro "-DWINDOWS")
elseif (UNIX)
    set (platform_macro "-DLINUX")
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
endif ()
message("platform: ${platform_macro}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall -fPIC -DLOGGING_LEVEL_2  ${platform_macro}  ")
#add_compile_options("-std=c++11" "-fPIC" ${platform_macro} "-Wall")

if (WIN32)
    link_directories("${CMAKE_BINARY_DIR}")
endif ()
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-rpath,.")

add_library(${PROJECT_NAME} SHARED ${PluginSrc} ${MQTTSrc} ./src/templates/posix_sockets.c)

if (WIN32)
    target_link_libraries(${PROJECT_NAME}   WS2_32 DolphinDB pthread)
elseif (UNIX)
    target_link_libraries(${PROJECT_NAME} pthread)
endif ()
