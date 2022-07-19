cd build
mingw32-make clean
cp ../libDolphinDB.dll libDolphinDB.dll
cmake ../ -G "MinGW Makefiles"
mingw32-make -j4
