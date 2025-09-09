#!/bin/bash

source ../build_util.sh

prepare_dir $@ -DXGBOOST_VERSION=1.2
build_plugin
install_plugin
mv $CMAKE_INSTALL_PREFIX/xgboost $CMAKE_INSTALL_PREFIX/xgboost1.2/
cp /lib64/libgomp.so.1 $CMAKE_INSTALL_PREFIX/xgboost1.2

prepare_dir $@ -DXGBOOST_VERSION=2.0
build_plugin
install_plugin
mv $CMAKE_INSTALL_PREFIX/xgboost $CMAKE_INSTALL_PREFIX/xgboost2.0/
cp /lib64/libgomp.so.1 $CMAKE_INSTALL_PREFIX/xgboost2.0

mkdir -p $CMAKE_INSTALL_PREFIX/xgboost/
mv $CMAKE_INSTALL_PREFIX/xgboost1.2/ $CMAKE_INSTALL_PREFIX/xgboost/1.2/
mv $CMAKE_INSTALL_PREFIX/xgboost2.0/ $CMAKE_INSTALL_PREFIX/xgboost/2.0/
