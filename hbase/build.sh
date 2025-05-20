rm -rf build
mkdir build
cd build
cmake ..
make -j
cd ..
mkdir -p $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f build/libPluginHBase.so $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f PluginHBase.txt $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
