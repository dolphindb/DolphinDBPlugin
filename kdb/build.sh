rm -rf build
mkdir build
cd build
cmake ..
make -j
cd ..
mkdir -p $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f build/libPluginKDB.so $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f build/PluginKDB.txt $CMAKE_INSTALL_PREFIX/$(basename $(pwd))