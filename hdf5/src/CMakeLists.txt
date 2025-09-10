target_sources(${PluginName} PRIVATE
    hdf5_plugin.cpp
    hdf5_plugin_endian.cpp
    hdf5_plugin.h
    hdf5_plugin_imp.cpp
    hdf5_plugin_imp.h
    hdf5_plugin_obj.cpp
    hdf5_plugin_obj.h
    hdf5_plugin_pandas.cpp
    hdf5_plugin_pandas.h
    hdf5_plugin_type.cpp
    hdf5_plugin_type.h
    hdf5_plugin_util.cpp
    hdf5_plugin_util.h
    blosc/blosc_filter.cpp
    blosc/blosc_plugin.cpp
)
