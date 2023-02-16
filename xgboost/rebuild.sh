rm -rf build
mkdir build
cd build
cp /dolphindb2.00.8/server/libDolphinDB.so ./
cmake .. 
make -j
