rm -rf build
mkdir build
cd build
cmake ..
make -j
cd ..
mkdir -p $CMAKE_INSTALL_PREFIX/$(basename $(pwd))

cp -f build/libPluginInsight.so  $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f build/PluginInsight.txt $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
#cd  $WORKSPACE
cp -r lib/*.so* $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
#cp -f  $WORKSPACE/insight/lib/libprotobuf.so.11 .
#cp -f  $WORKSPACE/insight/lib/libmdc_gateway_client.so .
#cp -f  $WORKSPACE/insight/lib/libACE.so.6.4.3 .
#cp -f  $WORKSPACE/insight/lib/libACE_SSL.so.6.4.3 .
#cp -f  $WORKSPACE/insight/lib/libcrypto.so.* .
#cp -f  $WORKSPACE/insight/lib/libssl.so.* .
cp -f -r include/cert $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
