rm -rf build
mkdir build
cd build
cmake ..
cp /dolphindb1.30.19/server/libDolphinDB.so ./
make
