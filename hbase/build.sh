#!/bin/bash

rm -rf build
mkdir build
cd build

toolchain_dir=$2
toolchain_arg="-DBOOST_ROOT=$(ls -d $toolchain_dir/boost-*) $toolchain_arg"

cmake .. $toolchain_arg
make -j
cd ..
mkdir -p $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f build/libPluginHBase.so $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f PluginHBase.txt $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
