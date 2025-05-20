cmake_minimum_required(VERSION 3.13)

set(PluginName PluginOPCUA)
project(${PluginName})

add_compile_options(-flto)
add_link_options(-flto)

include(../template.cmake)
CreatePlugin(${PluginName})
AddDependency(open62541 open62541)
AddDependency(mbedtls mbedcrypto)
AddDependency(mbedtls mbedx509)
