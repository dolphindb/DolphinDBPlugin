cmake_minimum_required(VERSION 3.22)
set(PluginName PluginKafka)
project(${PluginName})

include(../template.cmake)

CreatePlugin(${PluginName})

find_package(CppKafka REQUIRED)
target_link_libraries(${PluginName} PRIVATE CppKafka::cppkafka)

AddDependency(cyrus-sasl sasl2)
AddDependency(krb5 krb5_combined)
target_link_libraries(${PluginName} PRIVATE resolv)
