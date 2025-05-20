rm -rf build
mkdir build
cd build
cmake ..
make -j
cd ..
mkdir -p $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp build/libPlugin*.so $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp Plugin*.txt $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp contrib/lib/lib*.so $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
