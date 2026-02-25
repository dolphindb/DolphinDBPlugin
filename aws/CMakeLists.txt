cmake_minimum_required(VERSION 3.22)

set(PluginName PluginAWS)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

find_package(AWSSDK REQUIRED COMPONENTS s3)
target_link_libraries(${PluginName} PRIVATE ${AWSSDK_LINK_LIBRARIES})
AddOpenSSL()
AddCurl()
AddZlib()
