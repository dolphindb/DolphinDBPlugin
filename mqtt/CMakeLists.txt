cmake_minimum_required(VERSION 3.0)
project(PluginMQTTClient)

set(CMAKE_CXX_STANDARD 11)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/")

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/ PluginSrc)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/lib/ MQTTSrc)

if (WIN32)
    set(platform_macro "-DWINDOWS")
elseif (UNIX)
    set(platform_macro "-DLINUX")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall ${platform_macro} -D_GLIBCXX_USE_CXX11_ABI=0")
add_compile_options( "-std=c++11")
add_library(${PROJECT_NAME} SHARED ${PluginSrc} ${MQTTSrc} ./src/templates/posix_sockets.c)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} DolphinDB pthread)
elseif (UNIX)
    target_link_libraries(${PROJECT_NAME} pthread)
endif ()
