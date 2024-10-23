# Compiling plugins under Windows

### 1.Download and install

* [vscode](https://code.visualstudio.com/)

> Need to install two plugins: terminal and cmake 

* [cmake](https://cmake.org/)

> Need to configure environment variables. e.g. F:\Program Files (x86)\cmake\bin

* [mingw](http://www.mingw.org/)

> Need to configure environment variables. e.g. H:\mingw64\bin

### 2.Compile plugin

* Create folder

For example, if the file name is called SString, the folder structure is as follows
> src/: SString.cpp SString.h
>
> include/: DolphinDB header files
>
> libDolphinDB.dll: Download the corresponding version of DolphinDB dll lib. Note that this lib file should be consistent with the dependency library of dolphindb.exe.
>
> CmakeLists.txt

* CmakeList.txt: various options for specifying plugin compilation

```cmake
cmake_minimum_required(VERSION 3.00)
project(SSTRING)
include_directories("./include")  
# Library file location

e.g. link_directories("C:/dolphindb/SString")

#libDolphinDb.dll location

aux_source_directory("./src" sstring_plugin_src)
# The file location that needs to be compiled and it is named as sstring_plugin_src.

add_compile_options("-std=c++11" "-fPIC" "-DWINDOWS" "-Wall" "-D_WIN32_WINNT=0x0600" "-DWINVER=0x0600" "-DLOCKFREE_SYMBASE")
# Compiling options

add_library(PluginSString SHARED ${sstring_plugin_src})
target_link_libraries(PluginSString libDolphinDB.dll)
# Compilesstring_plugin_src为PluginSString.dll
# link libDolphinDB.dll
```



* Execute the following commands in the same directory, which can be done under vscode.

```bash
mkdir build
cd build
cmake -G "MinGW Makefiles"  ../
mingw32-make.exe
```

* if compiled successfully, we should find file libPluginSString.dll under the build folder.



