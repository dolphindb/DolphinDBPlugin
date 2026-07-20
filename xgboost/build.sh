#!/bin/bash

source ../build_util.sh

if [[ "$OSTYPE" != linux-gnu* ]]; then
    prepare_dir $@ -DXGBOOST_VERSION=1.2
    build_plugin
    install_plugin
    mv $CMAKE_INSTALL_PREFIX/xgboost/ 1.2
    mkdir -p $CMAKE_INSTALL_PREFIX/xgboost/
    mv 1.2 $CMAKE_INSTALL_PREFIX/xgboost/1.2/
    cp /c/x86_64-8.1.0-posix-seh-rt_v6-rev0/mingw64/bin/libgomp-1.dll $CMAKE_INSTALL_PREFIX/xgboost/1.2
else
    if [[ $CC != gcc ]];then
        unset FTP_URL
        prepare_dir $@ -DXGBOOST_VERSION=3.0 -DSTATIC_LIBSTDCXX=OFF
        build_plugin
        install_plugin
        mkdir -p $CMAKE_INSTALL_PREFIX/xgboost/3.1/
        cp -r $CMAKE_INSTALL_PREFIX/xgboost 3.1
        mv 3.1 $CMAKE_INSTALL_PREFIX/xgboost
    else
        prepare_dir $@ -DXGBOOST_VERSION=1.2
        build_plugin
        install_plugin
        mv $CMAKE_INSTALL_PREFIX/xgboost/ $CMAKE_INSTALL_PREFIX/xgboost-1.2/

        prepare_dir $@ -DXGBOOST_VERSION=3.1
        build_plugin
        install_plugin
        mv $CMAKE_INSTALL_PREFIX/xgboost/ $CMAKE_INSTALL_PREFIX/xgboost-3.1/

        mkdir -p $CMAKE_INSTALL_PREFIX/xgboost
        mv $CMAKE_INSTALL_PREFIX/xgboost-1.2 $CMAKE_INSTALL_PREFIX/xgboost/1.2
        mv $CMAKE_INSTALL_PREFIX/xgboost-3.1 $CMAKE_INSTALL_PREFIX/xgboost/3.1
    fi
fi
