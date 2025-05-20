cmake_minimum_required(VERSION 3.13)

set(PluginName PluginHttpClient)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

if(UNIX)
    # apt install libcurl4-openssl-dev
    # yum install libcurl-devel
    set(CURL_USE_STATIC_LIBS TRUE)
    find_package(CURL REQUIRED)
    target_link_libraries(${PluginName} PRIVATE CURL::libcurl)
    AddZlib()
    AddOpenSSL()
elseif(WIN32)
    target_include_directories(${PluginName} PRIVATE ${PROJECT_SOURCE_DIR}/thirdParty/curl/windows/include)
    target_include_directories(${PluginName} PRIVATE ${PROJECT_SOURCE_DIR}/thirdParty/ssl/windows/include)
    target_link_directories(${PluginName} PRIVATE ${PROJECT_SOURCE_DIR}/thirdParty/curl/windows/lib)
    target_link_directories(${PluginName} PRIVATE ${PROJECT_SOURCE_DIR}/thirdParty/ssl/windows/lib)

    target_link_libraries( PluginHttpClient PRIVATE 
        curl-x64
        crypto
        pthread)

    cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME INSTALL_DIR)
    install(FILES ${PROJECT_SOURCE_DIR}/bin/Windows/libwinpthread-1.dll DESTINATION ${INSTALL_DIR})
    install(FILES ${PROJECT_SOURCE_DIR}/thirdParty/curl/windows/lib/libcurl-x64.dll DESTINATION ${INSTALL_DIR})
endif()

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/liburlencode/src DIR_SRCS)
set(INC_LIST liburlencode/include)
BuildDependency(urlencode DIR_SRCS INC_LIST)


