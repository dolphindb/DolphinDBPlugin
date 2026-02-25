target_include_directories(${PluginName} PRIVATE .)

target_sources(${PluginName} PRIVATE
    Helper.cpp
    TypeConverter.cpp
    TypeException.cpp
    TypeHelper.cpp
)
