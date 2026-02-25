set "TOOLCHAIN=%~1"
set "workspace=%~2"
set "CMAKE_INSTALL_PREFIX=%~3"
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_C_COMPILER=C:/%TOOLCHAIN%/mingw64/bin/gcc.exe -DCMAKE_CXX_COMPILER=C:/%TOOLCHAIN%/mingw64/bin/g++.exe
C:\%TOOLCHAIN%\mingw64\bin\mingw32-make.exe -j8

cmake --install . --prefix %CMAKE_INSTALL_PREFIX%
