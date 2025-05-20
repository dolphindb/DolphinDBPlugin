source ../build_util.sh

prepare_dir $@
cp gpThirdParty/lib*so* build/

cd build
make -j VERBOSE=1
cd ..
mkdir -p $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f build/libPluginGp.so $CMAKE_INSTALL_PREFIX/$(basename $(pwd))/
cp -f build/PluginGp.txt $CMAKE_INSTALL_PREFIX/$(basename $(pwd))/
cp -f build/libpng16.so.16 $CMAKE_INSTALL_PREFIX/$(basename $(pwd))/
cp -f build/libgd.so $CMAKE_INSTALL_PREFIX/$(basename $(pwd))/
cp -f build/libjpeg.so $CMAKE_INSTALL_PREFIX/$(basename $(pwd))/
cp -f build/libgd.so.3 $CMAKE_INSTALL_PREFIX/$(basename $(pwd))/
cp -f build/libreadline.so.6 $CMAKE_INSTALL_PREFIX/$(basename $(pwd))/
cp -f build/libz.so.1 $CMAKE_INSTALL_PREFIX/$(basename $(pwd))/
