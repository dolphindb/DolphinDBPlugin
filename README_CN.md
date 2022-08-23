# DolphinDB Plugin

DolphinDB database 支持动态载入外部插件，以拓展系统功能。插件仅支持使用C++编写，并且需要编译成so共享库或者dll共享库文件。

## 下载

插件分支应与DolphinDB Server的版本相匹配，即若DolphinDB Server是1.30版本，插件应用release130分支，若DolphinDB Server是2.00版本，插件应该用release200分支，其他版本依此类推。

在DolphinDB server发布包的server/plugins目录下，已预下载了一些常用插件，如mysql、ODBC插件等，可直接加载使用。在每个插件的bin目录下已经包含了linux或windows操作系统上可执行的二进制文件，若无定制的需求，可直接下载bin文件使用。

## 加载插件

### 通过函数loadPlugin加载

使用[`loadPlugin`](https://www.dolphindb.cn/cn/help/loadPlugin.html)函数加载外部插件。该函数接受一个文件路径，该文件描述插件的格式,例如：

```
loadPlugin("/YOUR_SEVER_PATH/plugins/odbc/PluginODBC.txt"); 
```

### DolphinDB Server>=1.20.0 版本后可以通过preloadModules参数来自动加载

前提是server的版本>=1.20; 需要预先加载的插件存在。否则sever启动的时候会有异常。多个插件用都好分离。
```
preloadModules=plugins::mysql,plugins::odbc
```

  

## 插件格式

DolphinDB使用一个文本文件来描述插件。该文件格式如下：首行描述插件名字和共享库文件名，其后每一行都描述一个共享库函数和DolphinDB函数的映射关系。  
```
module name, lib file
function name in lib, function name in DolphinDB, function type, minParamCount, maxParamCount, isAggregate
...
...
```
**解释**：
* module name: 模块名  
* lib file: 共享文件库文件名  
* function name in lib: 共享库中导出的函数名  
* function name in DolphinDB: DolphinDB中的对应函数名  
* function type: operator或者system  
* minParamCount: 最少参数个数  
* maxParamCount：最多参数个数  
* isAggregate: 是否为聚合函数  

## 例子
PluginDemo.txt:
```
demo,libPluginDemo.so 
minmax,minmax,operator,1,1,0
foo,foo,system,1,1,0
```
以上描述文件定义了一个名为 demo 的插件，共享库文件名为 libPluginDemo.so。插件导出两个函数，第一个函数为`minmax`，该函数在DolphinDB中名字同样是`minmax`，operator类型，接受一个参数；第二个函数名字为`echo`，DolphinDB中名字同样是`echo`，system类型，接受一个参数。  

写完描述文件之后，即可开始编写插件。内容请参考demo文件夹内容。

编译需要用到DolphinDB的核心库 libDolphinDB.so 或 libDolphinDB.dll，该核心库实现了include目录下声明的类。编译步骤如下（以Linux操作系统上编译为例）：
```
cd demo
g++ -DLINUX -fPIC -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0 -DLOCKFREE_SYMBASE -c src/Demo.cpp -I../include -o Demo.o
g++ -fPIC -shared -o libPluginDemo.so Demo.o -lDolphinDB -L/home/DolphinDB_Linux64_V1.20.0/server
```
> 请注意： 
> 1. 插件分支应与DolphinDB Server的版本相匹配，即若DolphinDB Server是1.00版本，插件应用release100分支，若DolphinDB Server是1.10版本，插件应该用release110分支，其他版本依此类推，server最新版匹配插件master分支。
> 2. 为了兼容旧的编译器，libDolphinDB.so编译时使用了-D_GLIBCXX_USE_CXX11_ABI=0的选项，因此用户在编译插件的时候也应该加入该选项。若server是ABI=1的最新版，可不加。
> 3. 上面命令中编译选项-L的路径，应根据DolphinDB动态库的实际路径修改。
 
编译成功后，目录下会生成一个名为 libPluginDemo.so 的共享文件。

在DolphinDB的控制台中输入下列命令加载插件并使用：
```
>loadPlugin(PluginDemo.txt的路径); // 加载插件
(minmax,echo)
>use demo; // 引入插件的命名空间
>demo::minmax([12,3,4]); // 也可以使用minmax([12,3,4])
[3,12]
>demo::echo("foo");
foo
>echo(1);
1
```

更复杂的插件实现请参考odbc目录下的内容。

## Tips
* 建议使用 ld 命令检查下编译器链接是否成功，so中是否存在未定义的引用。如果 ld 报错，那么DolphinDB也无法正确加载插件。
* 如果载入插件之后出现了crash，可以采取尝试以下步骤。  
   1. 确保include下的头文件和 libDolphinDB.so 或 libDolphinDB.dll 实现保持一致.
   2. 确保用于编译插件的```gcc```版本和编译 libDolphinDB.so 或 libDolphinDB.dll 的版本保持一致，以免出现不同版本的编译器ABI不兼容的问题。
   3. 插件与DolphinDB server在同一个进程中运行，若插件crash，那整个系统就会crash。因此在开发的插件时候要注意完善错误检测机制，除了插件函数所在线程可以抛出异常（server在调用插件函数时俘获了异常），其他线程都必须自己俘获异常，并不得抛出异常。
   4. 确保宏LOCKFREE_SYMBASE已经添加。
* 如果编译时出现链接问题（undefined reference），并且包含 std::__cxx11 字样，务必检查用于编译插件的gcc版本（gcc 6.2.0 且有--disable-libstdcxx-dual-abi）。

