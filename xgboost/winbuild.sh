rm -rf build
mkdir build
cd build
cp  /d/dolphindb/2.00.8_JIT/server/libDolphinDB.dll ./
cmake .. -G "MinGW Makefiles"
mingw32-make
