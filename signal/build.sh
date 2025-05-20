#!/bin/bash

WORKSPACE=$(pwd)/..
cd thirdParty/linux_x86/lib
cp -f libfftw3_omp.so.3 libfftw3_omp.so
cp -f libfftw3.so.3 libfftw3.so
cd -
mkdir -p $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f thirdParty/linux_x86/lib/libfftw3_omp.so.3 $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f thirdParty/linux_x86/lib/libfftw3.so.3 $CMAKE_INSTALL_PREFIX/$(basename $(pwd))

mkdir build
cd build
cmake ..
cmake --build . -j

cd ..
cp -f build/libPlugin*.so $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f PluginSignal.txt  $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
