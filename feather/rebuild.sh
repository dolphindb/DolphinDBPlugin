rm -rf build
mkdir build
cd build
cp /dolphindb2.00.6/server/libDolphinDB.so ./
cmake ..
# cmake .. -DDEBUG=1
make -j
