rm -rf build
mkdir build
cd build
CC=aarch64-linux-gnu-gcc-4.8 CXX=aarch64-linux-gnu-g++-4.8 cmake .. -DBUILD_ARM=1
make -j
scp libPluginKafka.so slshen@192.168.100.30:/home/slshen/centoskafka/

