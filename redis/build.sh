#!/bin/bash

WORKSPACE=$(pwd)/..

mkdir build
cd build
cmake .. -DCMAKE_POLICY_VERSION_MINIMUM=3.5
cmake --build . -j
cd ..
mkdir -p $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f build/libPluginRedis.so $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f build/PluginRedis.txt $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
