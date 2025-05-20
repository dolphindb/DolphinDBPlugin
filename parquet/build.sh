export PATH=/usr/bin:$PATH
export CC=/usr/bin/gcc
export CXX=/usr/bin/g++
source ../build_util.sh

# unset FTP_URL
# prepare_dir $@ 
# build_plugin
# #install_plugin
# mkdir -p $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
# cp build/PluginParquet.txt $CMAKE_INSTALL_PREFIX/$(basename $(pwd))/
# cp build/libPluginParquet.so $CMAKE_INSTALL_PREFIX/$(basename $(pwd))/

download_thirdparty parquet
rm -rf build && mkdir build && cd build
cmake ../
make -j
cd ..
mkdir -p $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp build/PluginParquet.txt $CMAKE_INSTALL_PREFIX/$(basename $(pwd))/
cp build/libPluginParquet.so $CMAKE_INSTALL_PREFIX/$(basename $(pwd))/