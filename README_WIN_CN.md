# Windows下编译插件

### 1.下载软件

* [vscode](https://code.visualstudio.com/)

> 需要安装terminal, cmake两个插件

* [cmake](https://cmake.org/)

> 需要配置环境变量如 F:\Program Files (x86)\cmake\bin

* [mingw](http://www.mingw.org/)

> 需要配置环境变量如 H:\mingw64\bin

### 2.编译插件

* 创建文件夹

> 比如文件名叫做 SString 内部结构如下
>
> src/ SStrin.cpp SString.h
>
> include/ 各种我们需要的dophindb头文件
>
> libDolphinDB.dll 需要从sftp上的SERVER/NEW/DolWin中下载，注意这个文件应该和最后运行dolphindb程序的依赖库一致
>
> CmakeLists.txt

* CmakeList.txt 规定了插件编译的各种细节选项

```cmake
cmake_minimum_required(VERSION 3.00)
project(SSTRING)
include_directories("./include")  
#库文件的地址

link_directories("C:/Users/zhu/Desktop/SString")
#libDolphinDb.dll的地址

aux_source_directory("./src" sstring_plugin_src)
#需要编译的文件地址，并命名为sstring_plugin_src

add_compile_options("-std=c++11" "-fPIC" "-DWINDOWS" "-Wall" "-D_WIN32_WINNT=0x0600" "-DWINVER=0x0600" "-DLOCKFREE_SYMBASE")
#编译选项

add_library(PluginSString SHARED ${sstring_plugin_src})
target_link_libraries(PluginSString libDolphinDB.dll)
#编译sstring_plugin_src为PluginSString.dll
#并链接libDolphinDB.dll
```



* 在文件夹下的同级目录下进行操作，这个可以在vscode下做

```bash
mkdir build
cd build
cmake -G "MinGW Makefiles"  ../
mingw32-make.exe
```

* 这样就可以在build文件夹里面找到我们要用的libPluginSString.dll了

> 和linux里面的.so文件一样的意思，注意在引入插件的txt文件中将.so改成链接.dll文件即可