source ../build_util.sh

unset FTP_URL
prepare_dir $@ 
build_plugin
#install_plugin
mkdir -p $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp build/PluginParquet.txt $CMAKE_INSTALL_PREFIX/$(basename $(pwd))/
cp build/libPluginParquet.so $CMAKE_INSTALL_PREFIX/$(basename $(pwd))/