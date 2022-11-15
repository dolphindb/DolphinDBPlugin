rm -rf build
mkdir build
cd build
cp /hdd/ddb/latest/server/libDolphinDB.so ./
cmake ..
make
