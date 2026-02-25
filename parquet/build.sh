#!/bin/bash

source ../build_util.sh

unset FTP_URL
prepare_dir $@
build_plugin
install_plugin
objcopy --strip-debug  $CMAKE_INSTALL_PREFIX/$(basename $(pwd))/libPluginParquet.so