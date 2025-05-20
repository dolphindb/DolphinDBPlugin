cmake_minimum_required(VERSION 3.13)

set(PluginName PluginHttpClient)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

# apt install libcurl4-openssl-dev
# yum install libcurl-devel
set(CURL_USE_STATIC_LIBS TRUE)
find_package(CURL REQUIRED)
target_link_libraries(${PluginName} PRIVATE CURL::libcurl)
AddZlib()
AddOpenSSL()

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/liburlencode/src DIR_SRCS)
set(INC_LIST liburlencode/include)
BuildDependency(urlencode DIR_SRCS INC_LIST)
