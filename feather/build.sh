cp -rf /hdd/plugins/arrow/cpp/src/arrow ./include
cp -f /hdd/plugins/arrow/cpp/build/release/libarrow.a ./lib/linux
cp -f /hdd/plugins/arrow/cpp/build/jemalloc_ep-prefix/src/jemalloc_ep/lib/libjemalloc_pic.a ./lib/linux
cp -f /hdd/plugins/arrow/cpp/build/zstd_ep-install/lib64/libzstd.a ./lib/linux
cp -f /hdd/plugins/zlib/lib/libz.a ./lib/linux
cp -f /hdd/plugins/arrow/cpp/build/lz4_ep-prefix/src/lz4_ep/lib/liblz4.a  ./lib/linux 
rm -rf build
mkdir build
cd build
cmake ..
make -j
cd ..
mkdir -p $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f build/libPluginFeather.so $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f build/PluginFeather.txt $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
