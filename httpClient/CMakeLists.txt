cmake_minimum_required(VERSION 3.0)
project(PluginHTTP)

set(CMAKE_CXX_STANDARD 11)


if (WIN32)
    add_definitions("-DWINDOWS")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginHttpClient_win.txt")
elseif(UNIX)
    add_definitions("-DLINUX")
    add_definitions("-D_GLIBCXX_USE_CXX11_ABI=0") 
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginHttpClient.txt")
endif()



if(DEFINED BUILD_ARM)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC -O3 -Wall")
        # add_compile_options("-O3")
	message("build arm")
	include_directories(${PROJECT_SOURCE_DIR}/tmp/curl/include)
	include_directories(${PROJECT_SOURCE_DIR}/tmp/ssl/include)
	include_directories(${PROJECT_SOURCE_DIR}/tmp/zlib/include)
	include_directories(${PROJECT_SOURCE_DIR}/../include)
    link_directories(${CMAKE_BINARY_DIR})

else ()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC -O3 -Wall")
	message(STATUS ${CURL_DIR}/include)
	message(STATUS ${SSL_DIR}/include)
	message(STATUS ${Z_DIR}/include)
	include_directories(${CURL_DIR}/include)
	include_directories(${SSL_DIR}/include)
	include_directories(${Z_DIR}/include)
	include_directories(${PROJECT_SOURCE_DIR}/../include)
        link_directories("${CMAKE_BINARY_DIR}")
endif()


aux_source_directory(${PROJECT_SOURCE_DIR}/src DIR_SRCS)

add_library(PluginHttpClient SHARED ${DIR_SRCS})


if(DEFINED BUILD_ARM)
    target_link_libraries(
        PluginHttpClient 
        DolphinDB
        ${PROJECT_SOURCE_DIR}/tmp/curl/lib/libcurl.a
        ${PROJECT_SOURCE_DIR}/tmp/ssl/lib/libssl.a
        ${PROJECT_SOURCE_DIR}/tmp/ssl/lib/libcrypto.a
        ${PROJECT_SOURCE_DIR}/tmp/zlib/lib/libz.a
        )
    message(${PROJECT_SOURCE_DIR}/tmp/ssl/lib/libssl.a)
    message("link arm")
elseif(WIN32 OR MINGW)
    target_link_libraries(
        PluginHttpClient 
        DolphinDB 
        gdi32 
        ws2_32)
elseif(UNIX)
    target_link_libraries(
        PluginHttpClient 
        DolphinDB
        ${CURL_DIR}/lib/libcurl.a
        ${SSL_DIR}/lib/libssl.a
        ${SSL_DIR}/lib/libcrypto.a
        ${Z_DIR}/lib/libz.a
        )
endif()
configure_file(${plugin_file} ${CMAKE_BINARY_DIR}/PluginHttpClient.txt COPYONLY)

