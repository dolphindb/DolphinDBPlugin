source ../build_util.sh

unset FTP_URL
prepare_dir $@ 
build_plugin
#install_plugin
mkdir -p $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp PluginMQTTClient.txt $CMAKE_INSTALL_PREFIX/$(basename $(pwd))/
cp PluginMQTT.txt $CMAKE_INSTALL_PREFIX/$(basename $(pwd))/
cp build/libPluginMQTTClient.so $CMAKE_INSTALL_PREFIX/$(basename $(pwd))/
