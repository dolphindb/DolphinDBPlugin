#!/bin/bash

source ../build_util.sh

prepare_dir $@
build_plugin
install_plugin

rm -f $CMAKE_INSTALL_PREFIX/nsq/HSNsqApi.lib
rm -f $CMAKE_INSTALL_PREFIX/nsq/HSNsqApi.pdb
