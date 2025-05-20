#!/bin/bash

source ../build_util.sh

unset FTP_URL
prepare_dir $@
build_plugin
#install_plugin

mkdir -p $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f build/PluginMongodb.txt $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f build/libPluginMongodb.so $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f bin/linux64/libbson-1.0.so.0 $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f bin/linux64/libicudata.so.52 $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f bin/linux64/libicuuc.so.52 $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f bin/linux64/libmongoc-1.0.so.0 $CMAKE_INSTALL_PREFIX/$(basename $(pwd))

