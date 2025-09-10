#!/bin/bash

WORKSPACE=$(pwd)/..

cp -r /hdd/jenkins/workspace/build_plugins/mysql/contrib /hdd/jenkins/workspace/build_plugin_all/mysql/

mkdir build
cd build
cmake ..
cmake --build . -j
cd ..
mkdir -p $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f build/lib*.so $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f build/PluginMySQL.txt $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
