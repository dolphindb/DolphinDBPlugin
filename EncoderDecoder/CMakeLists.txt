cmake_minimum_required(VERSION 3.22)

set(PluginName PluginEncoderDecoder)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

UseSignedCharOnARM()
AddProtobuf()
