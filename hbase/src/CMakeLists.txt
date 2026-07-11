target_sources(${PluginName} PRIVATE
    ${HBASE_THRIFT_GENERATED_DIR}/Hbase_types.cpp
    ${HBASE_THRIFT_GENERATED_DIR}/Hbase.cpp
    PluginHbase.cpp
    plugin.cpp
)

target_include_directories(${PluginName} PRIVATE ${HBASE_THRIFT_GENERATED_DIR})
