cmake_minimum_required(VERSION 3.22)
set(PluginName PluginHdfs)
project(${PluginName})

include(../template.cmake)
CreatePlugin(${PluginName})

if (NOT HADOOP_DIR)
    message(FATAL_ERROR "Please set HADOOP_DIR.")
endif()
target_include_directories(${PluginName} PRIVATE ${HADOOP_DIR}/include)
target_link_directories(${PluginName} PRIVATE ${HADOOP_DIR}/lib/native)
target_link_libraries(${PluginName} PRIVATE hdfs) # the official libhdfs.a is NOT compiled with -fPIC

AddDependency(krb5 libkrb5_combined.a)
# AddDependency(keyutils libkeyutils.a)
