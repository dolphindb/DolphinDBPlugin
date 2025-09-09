#!/bin/bash

source ../build_util.sh

prepare_dir $@ -DXGBOOST_VERSION=1.2
build_plugin
install_plugin
cp /c/x86_64-8.1.0-posix-seh-rt_v6-rev0/mingw64/bin/libgomp-1.dll $CMAKE_INSTALL_PREFIX/xgboost/
