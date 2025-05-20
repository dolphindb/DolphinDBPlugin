#!/bin/bash

mkdir build
cd build
cmake ..
cmake --build . -j
cd ..
mkdir -p $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f build/libPluginEncoderDecoder.so $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f build/PluginEncoderDecoder.txt $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
