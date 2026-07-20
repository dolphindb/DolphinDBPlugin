cmake_minimum_required(VERSION 3.22)

set(PluginName PluginHttpClient)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

AddCurl()
AddZlib()
AddOpenSSL()
