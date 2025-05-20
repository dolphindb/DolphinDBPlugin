cmake_minimum_required(VERSION 3.13)
set(PluginName PluginKafka)
project(${PluginName})

include(../template.cmake)

CreatePlugin(${PluginName})

AddDependency(cppkafka cppkafka)
AddDependency(librdkafka librdkafka.a)
# librdkafka dependencies
AddOpenSSL()
AddDependency(cyrus-sasl sasl2)
AddZlib()
AddDependency(lz4 liblz4.a)