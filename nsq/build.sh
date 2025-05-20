#!/bin/bash

WORKSPACE=$(pwd)/..

mkdir build
cd build
cmake ..
cmake --build . -j
cd ..
mkdir -p $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f build/lib*.so $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f build/PluginNsq.txt $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f lib/linux.x64/* $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
