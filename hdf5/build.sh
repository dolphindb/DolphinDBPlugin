
mkdir -p lib

cp -f /hdd/plugins/hdf5-1.13.1/hdf5/include/* ./include/
cp -f /hdd/plugins/hdf5-1.13.1/hdf5/lib/libhdf5.a ./lib/
cp -f /hdd/plugins/hdf5-1.13.1/hdf5/lib/libhdf5_cpp.a ./lib/
cp -f /hdd/plugins/hdf5-1.13.1/hdf5/lib/libhdf5_hl.a ./lib/

source ../build_util.sh

unset FTP_URL
prepare_dir $@
build_plugin
install_plugin

mkdir -p $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f PluginHdf5.txt $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f build/libPluginHdf5.so $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
