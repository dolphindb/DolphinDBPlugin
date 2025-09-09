#!/bin/bash

source ../build_util.sh

prepare_dir $@
build_plugin
install_plugin
cd $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
mv PluginOPC.txt PluginOpc.txt