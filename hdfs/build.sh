plugin_dir=$(pwd)

cd /hdd/libs/krb5-krb5-1.21-final/src/lib
cp -f krb5/libkrb5.a  $plugin_dir/lib/
cp -f ../util/et/libcom_err.a   $plugin_dir/lib/
cp -f crypto/libk5crypto.a  $plugin_dir/lib/
cp -f  ../util/support/libkrb5support.a  $plugin_dir/lib/
cd -

rm -rf build
mkdir build
cd build
cmake .. -DHADOOP_DIR=$2/hadoop-3.2.2
make -j
cd ..
mkdir -p $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f build/libPluginHdfs.so $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f PluginHdfs.txt $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
cp -f $2/hadoop-3.2.2/lib/native/libhdfs.so.0.0.0 $CMAKE_INSTALL_PREFIX/$(basename $(pwd))
#cp -f /lib/jvm/java-1.8.0-openjdk-1.8.0.282.b08-1.el7_9.x86_64/jre/lib/amd64/server/libjvm.so $WORKSPACE/
