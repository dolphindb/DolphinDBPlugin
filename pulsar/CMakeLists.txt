cmake_minimum_required(VERSION 3.27)

set(PluginName PluginPulsar)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

AddDependency(pulsar pulsar)
AddCurl()
AddZlib()
AddBoost()
AddProtobuf()
