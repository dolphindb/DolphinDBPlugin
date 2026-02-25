#!/bin/bash

source ../build_util.sh

prepare_dir $@
cd gnuplot
./prepare
rm -rf build && mkdir build && cd build
export CFLAGS="-I$pwd/third_party/include -fPIC"
export LDFLAGS="-L$pwd/third_party/lib"
../configure --without-readline
make -j$(nproc)
cd ../..
build_plugin
install_plugin
cp third_party/lib/* $CMAKE_INSTALL_PREFIX/$(basename $(pwd))/
