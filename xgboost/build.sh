#!/bin/bash

cd $WORKSPACE/xgboost/

cp CMakeLists_old.txt CMakeLists.txt

# compile 2.0
rm -rf $WORKSPACE/output/xgboost/

rm -rf build
mkdir build
cd build

cmake .. -DXGBOOST_VERSION=2.0 -DCMAKE_CXX_COMPILER=/home/api/toolchain/gcc-8/bin/g++ -DCMAKE_C_COMPILER=/home/api/toolchain/gcc-8/bin/gcc
make -j

mkdir -p $CMAKE_INSTALL_PREFIX/xgboost/2.0
cp -f libPluginXgboost.so $CMAKE_INSTALL_PREFIX/xgboost/2.0
cp -f PluginXgboost.txt $CMAKE_INSTALL_PREFIX/xgboost/2.0
cp /lib64/libgomp.so.1 $CMAKE_INSTALL_PREFIX/xgboost/2.0



# compile 1.2

cd $WORKSPACE/xgboost/
rm -rf build
mkdir build
cd build

CC=/home/api/toolchain/gcc-8/bin/gcc CXX=/home/api/toolchain/gcc-8/bin/g++  cmake ..
make -j


mkdir -p $CMAKE_INSTALL_PREFIX/xgboost/1.2
cp -f libPluginXgboost.so $CMAKE_INSTALL_PREFIX/xgboost/1.2
cp -f PluginXgboost.txt $CMAKE_INSTALL_PREFIX/xgboost/1.2
cp /lib64/libgomp.so.1 $CMAKE_INSTALL_PREFIX/xgboost/1.2

