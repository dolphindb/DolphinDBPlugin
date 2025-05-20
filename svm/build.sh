#!/bin/bash

mkdir build
cd build
cmake ..
cmake --build . -j
cd ..
mkdir -p $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f build/libPluginSVM.so $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f PluginSVM.txt  $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
