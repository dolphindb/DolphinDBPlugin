cmake_minimum_required(VERSION 3.22)
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
cmake_policy(SET CMP0167 OLD)
find_package(Boost)
target_link_libraries(${PluginName} PRIVATE Boost::boost)