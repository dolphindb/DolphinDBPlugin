cmake_minimum_required(VERSION 3.0)
set(CMAKE_SKIP_RPATH TRUE)
project(PluginHTTP)
add_definitions(-DLOCKFREE_SYMBASE)

set(CMAKE_CXX_STANDARD 11)

if (WIN32)
    add_definitions("-DWINDOWS")
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginHttpClient_win.txt")
    set(target_path "${CMAKE_CURRENT_LIST_DIR}/bin/Windows/")
    configure_file(${PROJECT_SOURCE_DIR}/thirdParty/curl/windows/lib/libcurl-x64.dll ${target_path}/libcurl-x64.dll COPYONLY)
elseif(UNIX)
    add_definitions("-DLINUX")
    add_definitions("-D_GLIBCXX_USE_CXX11_ABI=0") 
    set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginHttpClient.txt")
    set(target_path "${CMAKE_CURRENT_LIST_DIR}/bin/Linux/")
endif()

include_directories(${PROJECT_SOURCE_DIR}/liburlencode/include)

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "RELEASE")
endif ()

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
message("CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})

if("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -std=c++11 -Wall")
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++11 -Wall")
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RELWITHDEBINFO")  # release with debug info
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -O2 -Wall")
else ()
    message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}")
endif ()

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

if (${DDB_USE_ASAN})
    add_compile_options(
        "-fsanitize=address"                # Enable ASAN.
        "-fno-omit-frame-pointer"           # Nicer stack traces in error messages.
        "-fno-optimize-sibling-calls"       # Disable tail call elimination (perfect stack traces if inlining off).
        )
    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-fsanitize=address")
endif ()

if(DEFINED BUILD_ARM)
	message("building arm")
	include_directories(${PROJECT_SOURCE_DIR}/thirdParty/curl/arm_linux/include)
	include_directories(${PROJECT_SOURCE_DIR}/thirdParty/ssl/arm_linux/include)
	include_directories(${PROJECT_SOURCE_DIR}/thirdParty/zlib/arm_linux/include)
	include_directories(${PROJECT_SOURCE_DIR}/../include)
    link_directories(${CMAKE_BINARY_DIR})
    link_directories(${PROJECT_SOURCE_DIR}/thirdParty/curl/arm_linux/lib)
    link_directories(${PROJECT_SOURCE_DIR}/thirdParty/ssl/arm_linux/lib)
    link_directories(${PROJECT_SOURCE_DIR}/thirdParty/zlib/arm_linux/lib)

else ()
    if (WIN32)
        include_directories(${PROJECT_SOURCE_DIR}/thirdParty/curl/windows/include)
        include_directories(${PROJECT_SOURCE_DIR}/thirdParty/ssl/windows/include)
        include_directories(${PROJECT_SOURCE_DIR}/../include)
        link_directories(${PROJECT_SOURCE_DIR}/thirdParty/curl/windows/lib)
        link_directories(${PROJECT_SOURCE_DIR}/thirdParty/ssl/windows/lib)
        link_directories(${CMAKE_BINARY_DIR})
        message(STATUS ${CMAKE_BINARY_DIR})
    elseif(UNIX)
        message(STATUS ${CURL_DIR}/include)
        message(STATUS ${SSL_DIR}/include)
        message(STATUS ${Z_DIR}/include)
        include_directories(${CURL_DIR}/include)
        include_directories(${SSL_DIR}/include)
        include_directories(${Z_DIR}/include)
        include_directories(${PROJECT_SOURCE_DIR}/../include)
        link_directories("${CMAKE_BINARY_DIR}")
    endif()
endif()


aux_source_directory(${PROJECT_SOURCE_DIR}/src DIR_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/liburlencode/src DIR_SRCS)

add_library(PluginHttpClient SHARED ${DIR_SRCS})


if(DEFINED BUILD_ARM)
    target_link_libraries(
        PluginHttpClient 
        DolphinDB
        ${PROJECT_SOURCE_DIR}/thirdParty/curl/arm_linux/lib/libcurl.a
        ${PROJECT_SOURCE_DIR}/thirdParty/ssl/arm_linux/lib/libssl.a
        ${PROJECT_SOURCE_DIR}/thirdParty/ssl/arm_linux/lib/libcrypto.a
        ${PROJECT_SOURCE_DIR}/thirdParty/zlib/arm_linux/lib/libz.a
        )
    message(${PROJECT_SOURCE_DIR}/tmp/ssl/lib/libssl.a)
    message("link arm")
elseif(WIN32 OR MINGW)
    target_link_libraries(
        PluginHttpClient 
        DolphinDB 
        curl-x64
        crypto
        pthread)
elseif(UNIX)
    target_link_libraries(
        PluginHttpClient 
        ${CURL_DIR}/lib/libcurl.a
        ${SSL_DIR}/lib/libssl.a
        ${SSL_DIR}/lib/libcrypto.a
        ${Z_DIR}/lib/libz.a
        )
endif()
set(LIBRARY_OUTPUT_PATH ${target_path})
configure_file(${plugin_file} ${target_path}/PluginHttpClient.txt COPYONLY)

