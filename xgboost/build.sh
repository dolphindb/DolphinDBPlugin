#!/bin/bash

source ../build_util.sh

if [[ "$OSTYPE" != "linux-gnu"* ]]; then
    prepare_dir $@ -DXGBOOST_VERSION=1.2
    build_plugin
    install_plugin
    cp /c/x86_64-8.1.0-posix-seh-rt_v6-rev0/mingw64/bin/libgomp-1.dll $CMAKE_INSTALL_PREFIX/xgboost/1.2
    rm -rf $CMAKE_INSTALL_PREFIX/xgboost/libPluginXgboost.dll
    rm -rf $CMAKE_INSTALL_PREFIX/xgboost/PluginXgboost.txt
else 
    echo "prepare argument: $@"
    prepare_dir $@ -DXGBOOST_VERSION=1.2
    build_plugin
    install_plugin

    # prepare_dir $@ -DXGBOOST_VERSION=2.0
    # build_plugin
    # install_plugin

    prepare_dir $@ -DXGBOOST_VERSION=3.1
    build_plugin
    install_plugin

    cp /lib64/libgomp.so.1 $CMAKE_INSTALL_PREFIX/xgboost/1.2
    # cp /lib64/libgomp.so.1 $CMAKE_INSTALL_PREFIX/xgboost/2.0
    cp /lib64/libgomp.so.1 $CMAKE_INSTALL_PREFIX/xgboost/3.1

    rm -rf $CMAKE_INSTALL_PREFIX/xgboost/libPluginXgboost.so
    rm -rf $CMAKE_INSTALL_PREFIX/xgboost/PluginXgboost.txt
fi


