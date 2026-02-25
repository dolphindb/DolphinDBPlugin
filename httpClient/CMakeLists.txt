cmake_minimum_required(VERSION 3.22)

set(PluginName PluginHttpClient)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

AddCurl()
AddZlib()
AddOpenSSL()

#if (CMAKE_SYSTEM_NAME STREQUAL "MSYS")
#    target_include_directories(${PluginName} PRIVATE ${PROJECT_SOURCE_DIR}/thirdParty/curl/windows/include)
#    target_include_directories(${PluginName} PRIVATE ${PROJECT_SOURCE_DIR}/thirdParty/ssl/windows/include)
#    target_link_directories(${PluginName} PRIVATE ${PROJECT_SOURCE_DIR}/thirdParty/curl/windows/lib)
#    target_link_directories(${PluginName} PRIVATE ${PROJECT_SOURCE_DIR}/thirdParty/ssl/windows/lib)

#    target_link_libraries( PluginHttpClient PRIVATE 
#        curl-x64
#        crypto
#        pthread)

#    cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME INSTALL_DIR)
#    install(FILES ${PROJECT_SOURCE_DIR}/bin/Windows/libwinpthread-1.dll DESTINATION ${INSTALL_DIR})
#    install(FILES ${PROJECT_SOURCE_DIR}/thirdParty/curl/windows/lib/libcurl-x64.dll DESTINATION ${INSTALL_DIR})
#endif()

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/liburlencode/src DIR_SRCS)
set(INC_LIST liburlencode/include)
BuildDependency(urlencode DIR_SRCS INC_LIST)


