cmake_minimum_required(VERSION 3.13)

set(PluginName PluginOPCUA)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})
AddDependency(open62541 open62541)
AddDependency(mbedtls mbedcrypto)
AddDependency(mbedtls mbedx509)
