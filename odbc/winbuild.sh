rm -rf build
mkdir build
cd build
cp C:/DDB2.00.6/server/libDolphinDB.dll libDolphinDB.dll
cmake ../ -G "MinGW Makefiles"
mingw32-make -j4

~