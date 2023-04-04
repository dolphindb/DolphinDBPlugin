cmake_minimum_required(VERSION 3.0)
project(PluginAmdQuote)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if (WIN32)
    set(platform_macro "-DWINDOWS")
elseif (UNIX)
    set(platform_macro "-DLINUX")
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
endif ()
set(plugin_file "${CMAKE_CURRENT_LIST_DIR}/PluginAmdQuote.txt")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${platform_macro} -fPIC -std=c++11 -Wall -rdynamic ")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -fPIC -std=c++11 -Wall -rdynamic ")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -std=c++11 -Wall -rdynamic ")

if(DEFINED AMDAPIDIR)
    link_directories(${AMDAPIDIR}/lib)
else()
    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)
endif()
link_directories("${CMAKE_BINARY_DIR}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include/ddbplugin)
if(DEFINED AMDAPIDIR)
    include_directories(${AMDAPIDIR}/include)
else()  
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src plugin_amdquote_src)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/test test_amdquote_src)

add_library(${PROJECT_NAME} SHARED 
    ${plugin_amdquote_src}
)
set_target_properties(PluginAmdQuote PROPERTIES LINK_FLAGS "-Wl,-rpath,$ORIGIN")

#add_executable(amdQuoteTest
#    ${test_amdquote_src}
#)

target_link_libraries(${PROJECT_NAME}
    DolphinDB
    aaf
    adk
    ama
    ami 
    amigrpc
    amigrpc++
    crypto 
    das 
    entry_wrapper 
    fmdutil 
    llmi 
    rmm 
    rum 
    sample_engine 
    ssl
    z
    pthread
)


configure_file(${plugin_file} ${CMAKE_BINARY_DIR}/PluginAmdQuote.txt COPYONLY)
#target_link_libraries(amdQuoteTest
#    PluginAmdQuote
#    pthread
#)
