target_include_directories(${PluginName} PRIVATE
    ../lib
)
target_sources(${PluginName} PRIVATE
    client.cpp
    parser.cpp
    pub_client.cpp
    publisher.cpp
    sub_client.cpp
)
