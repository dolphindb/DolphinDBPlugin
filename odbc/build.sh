#!/bin/bash

source ../build_util.sh

prepare_dir $@
build_plugin
install_plugin

if [ "$Compiler" = "gcc-4.8.5" ] || [ "$Compiler" = "gcc-6.2.0" ] ||  [ "$Compiler" = "gcc-8.4.0" ]; then
    cp ODBCThirdParty/* $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
fi
