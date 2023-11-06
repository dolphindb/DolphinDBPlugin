# DolphinDB Plugin

DolphinDB database 支持动态载入外部插件，以拓展系统功能。插件仅支持使用 C++ 编写，并且需要编译成 so 共享库或者 dll 共享库文件。

## 下载

插件分支应与 DolphinDB Server 的版本相匹配，即若 DolphinDB Server 是 1.30 版本，插件应使用 release130 分支，若 DolphinDB Server 是 2.00 版本，插件应使用 release200 分支，其他版本依此类推。

DolphinDB 目前发布的插件版本为 [release200](https://github.com/dolphindb/DolphinDBPlugin/tree/release200), [release130](https://github.com/dolphindb/DolphinDBPlugin/tree/release130), [release120](https://github.com/dolphindb/DolphinDBPlugin/tree/release120) 和 [release110](https://github.com/dolphindb/DolphinDBPlugin/tree/release110)，请根据当前使用的服务器版本号进行选择。

在 DolphinDB server 发布包的 server/plugins 目录下，已预下载了一些常用插件，如 mysql, ODBC 插件等，可直接加载使用。在每个插件的 bin 目录下已经包含了 Linux 或 Windows 操作系统上可执行的二进制文件。若无定制的需求，可直接下载 bin 文件使用。

## 加载插件

### 通过函数 loadPlugin 加载

使用 [`loadPlugin`](https://www.dolphindb.cn/cn/help/FunctionsandCommands/FunctionReferences/l/loadPlugin.html) 函数加载外部插件。通过该函数加载一个插件文件，该文件描述插件的格式, 例如：

```
loadPlugin("/YOUR_SEVER_PATH/plugins/odbc/PluginODBC.txt");
```

### DolphinDB Server>=1.20.0 版本后可以通过 preloadModules 参数来自动加载

前提是 server 的版本 >=1.20，且需要预先加载的插件存在。否则 sever 启动的时候会有异常。多个插件用逗号分离。
```
preloadModules=plugins::mysql,plugins::odbc
```

## 插件格式

DolphinDB 使用一个文本文件来描述插件。该文件格式如下：首行描述插件名字和共享库文件名，其后每一行都描述一个共享库函数和 DolphinDB 函数的映射关系。
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
* function type: operator或者system，分别表示支持一个或两个参数的 Operator Functions 和支持任意数量参数的 System Functions
* minParamCount: 最少参数个数  
* maxParamCount：最多参数个数  
* isAggregate: 是否为聚合函数，可选值为 0 （非聚合函数，默认值）或 1（聚合函数） 

## 例子
[PluginDemo.txt](https://github.com/dolphindb/DolphinDBPlugin/blob/release200/demo/PluginDemo.txt):
```
demo,libPluginDemo.so
minmax,minmax,operator,1,1,0
echo,echo,system,1,1,0
```
以上描述文件定义了一个名为 demo 的插件，共享库文件名为 libPluginDemo.so。插件导出两个函数，第一个函数为 `minmax`，该函数在 DolphinDB 中名字同样是 `minmax`，operator 类型，接受一个参数；第二个函数名字为 `echo`，DolphinDB 中名字同样是 `echo`，system 类型，接受一个参数。

写完描述文件之后，即可开始编写插件。内容请参考 [demo](https://github.com/dolphindb/DolphinDBPlugin/tree/release200/demo) 文件夹内容。

编译需要用到 DolphinDB 的核心库 libDolphinDB.so 或 libDolphinDB.dll，该核心库实现了 include 目录下声明的类。编译步骤如下（以 Linux 操作系统上编译为例）：
```
cd demo
g++ -DLINUX -fPIC -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0 -DLOCKFREE_SYMBASE -c src/Demo.cpp -I../include -o Demo.o
g++ -fPIC -shared -o libPluginDemo.so Demo.o -lDolphinDB -L/home/DolphinDB_Linux64_V1.20.0/server
```
> 请注意：
> 1. 插件分支应与 DolphinDB Server 的版本相匹配，即若 DolphinDB Server 是 1.00 版本，插件应用 release100 分支，若 DolphinDB Server 是 1.10 版本，插件应该用 release110 分支，其他版本依此类推，server 最新版匹配插件 master 分支。
> 2. 为了兼容旧的编译器，libDolphinDB.so 编译时使用了 - D_GLIBCXX_USE_CXX11_ABI=0 的选项，因此用户在编译插件的时候也应该加入该选项。若 server 是 ABI=1 的最新版，可不加。
> 3. 上面命令中编译选项 - L 的路径，应根据 DolphinDB 动态库的实际路径修改。

编译成功后，目录下会生成一个名为 libPluginDemo.so 的共享文件。

在 DolphinDB 的控制台中输入下列命令加载插件并使用：
```
>loadPlugin(PluginDemo.txt 的路径); // 加载插件
(minmax,echo)
>use demo; // 引入插件的命名空间
>demo::minmax([12,3,4]); // 也可以使用 minmax([12,3,4])
[3,12]
>demo::echo("foo");
foo
>echo(1);
1
```

更复杂的插件实现请参考 [odbc](https://github.com/dolphindb/DolphinDBPlugin/tree/release200/odbc) 目录下的内容。

## Tips
* 建议使用 ld 命令检查下编译器链接是否成功，so 中是否存在未定义的引用。如果 ld 报错，那么 DolphinDB 也无法正确加载插件。
* 如果载入插件之后出现了 crash，可以采取尝试以下步骤。
   1. 确保 include 下的头文件和 libDolphinDB.so 或 libDolphinDB.dll 实现保持一致。
   2. 确保用于编译插件的 ```gcc``` 版本和编译 libDolphinDB.so 或 libDolphinDB.dll 的版本保持一致，以免出现不同版本的编译器 ABI 不兼容的问题。
   3. 插件与 DolphinDB server 在同一个进程中运行，若插件 crash，整个系统就会 crash。因此在开发插件的时候要注意完善错误检测机制，除了插件函数所在线程可以抛出异常（server 在调用插件函数时俘获了异常），其他线程都必须自己俘获异常，并不得抛出异常。
   4. 确保宏 LOCKFREE_SYMBASE 已经添加。
* 如果编译时出现链接问题（undefined reference），并且包含 std::__cxx11 字样，务必检查用于编译插件的 gcc 版本（gcc 6.2.0 且有 --disable-libstdcxx-dual-abi）。

