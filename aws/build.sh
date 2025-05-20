#!/bin/bash

source ../build_util.sh

prepare_dir $@
build_plugin
install_plugin

mkdir -p $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp build/libPluginAWS.so $CMAKE_INSTALL_PREFIX/$(basename $(pwd))/
cp build/PluginAWS.txt $CMAKE_INSTALL_PREFIX/$(basename $(pwd))/