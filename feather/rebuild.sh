rm -rf build
mkdir build
cd build
cp /dolphindb1.30.19/server/libDolphinDB.so ./
cmake ..
# cmake .. -DDEBUG=1
make -j
