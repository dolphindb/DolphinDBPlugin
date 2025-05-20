source ../build_util.sh

rm -rf include
cp -r include_win include

sshpass -p DolphinDB123 scp ftpuser@192.168.1.204:/hdd/ftp/PluginThirdParty/hdf5.tar.gz .

tar -xzvf hdf5.tar.gz
cp CMakeLists_win.txt CMakeLists.txt

unset FTP_URL
prepare_dir $@
build_plugin
install_plugin

mkdir -p $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f PluginHdf5.txt $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f build/libPluginHdf5.so $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
