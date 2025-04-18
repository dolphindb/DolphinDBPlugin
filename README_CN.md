# DolphinDB Plugin

DolphinDB database 支持动态载入外部插件，以拓展系统功能。插件仅支持使用 C++ 编写，并且需要编译成 so 共享库或者 dll 共享库文件。

插件共享库依赖 include 目录中声明的类及接口，因此在编译插件时，必须确保插件分支应与 DolphinDB 版本的前三位数字相匹配，即若 DolphinDB 是 1.30.21.x 版本，插件应使用 1.30.21.x，若 DolphinDB 是 2.00.9.x 版本，插件应使用 2.00.9.x，其他版本依此类推。加载插件时也要保持版本匹配。

## 1. 目录结构
* **include**(./include) 目录包含了 DolphinDB 的核心数据结构的类声明和一些工具类声明，这些类是实现插件的重要基础工具。  
* **demo**(./demo) 目录包含了一个 demo 插件的实现。  
* **odbc**(./odbc) 与 **mysql** (./mysql) 等目录包含了 ODBC 与 MySQL 等插件的实现。

## 2. 插件格式

DolphinDB 使用一个文本文件来描述插件。该文件格式如下：首行描述插件名字和共享库文件名，其后每一行都描述一个共享库函数和 DolphinDB 函数的映射关系。  
```
module name, lib file, plugin version
function name in lib, function name in DolphinDB, function type, minParamCount, maxParamCount, isAggregate, isSequential
...
...
```
**解释**：
* module name: 模块名
* lib file: 共享文件库文件名
* plugin version: 插件版本号  
* function name in lib: 共享库中导出的函数名  
* function name in DolphinDB: DolphinDB 中的对应函数名  
* function type: operator 或者 system，分别表示支持一个或两个参数的 Operator Functions 和支持任意数量参数的 System Functions  
* minParamCount: 最少参数个数  
* maxParamCount：最多参数个数  
* isAggregate: 是否为聚合函数，可选值为 0（非聚合函数，默认值）或 1（聚合函数）  
* isSequential：是否为序列函数，可选值为 0（非序列函数，默认值）或 1（序列函数）


## 3. 下载插件
DolphinDB 官方开发并发布了多种插件，用户可通过[插件市场](https://marketplace.dolphindb.cn/)直接下载安装。可通过如下步骤下载并安装插件：

(1) 在 DolphinDB 客户端中使用 [listRemotePlugins](https://docs.dolphindb.cn/zh/funcs/l/listRemotePlugins.html) 命令查看插件仓库中的插件信息。
```
login("admin", "123456")
listRemotePlugins()  
```
注意：如果仅展示当前操作系统和 server 版本支持的插件。若无预期插件，您可以通过我们的 [GitHub 仓库](https://github.com/dolphindb/DolphinDBPlugin/tree/master)（选择对应版本分支）下载源代码自行编译。编译方法参考[第5章编译](#5-编译插件)。

(2) 使用 [installPlugin](https://docs.dolphindb.cn/zh/funcs/i/installPlugin.html) 命令安装插件。
``` 
installPlugin("mysql")
```

## 4. 下载插件
加载插件时需要确保插件分支应与 DolphinDB Server 版本的前三位数字相匹配，一旦版本号不匹配，在加载插件时会报错并停止加载。请在升级服务器后及时更新相应的插件，且不要随意修改已发布插件 txt 文件中的版本号。


### 4.1 通过 `loadPlugin` 函数加载插件
可使用 `loadPlugin` 函数加载外部插件。为该函数传入一个文件路径参数，指定插件的位置，例如：
```
loadPlugin("/YOUR_SERVER_PATH/plugins/odbc/PluginODBC.txt"); 
```
注意：自 Server 版本 2.00.11 / 1.30.23 起，`loadPlugin` 支持直接使用插件名称（大小写敏感）。系统将根据插件名称和配置参数 `pluginDir` 自动加载插件。
```
loadPlugin("odbc"); 
```
### 4.2 通过配置参数 *preloadModules* 加载插件
也可以通过配置参数 *preloadModules* 自动加载。需要确保预加载的插件必须实际存在，否则会在服务器启动时抛出异常。多个插件名称可使用英文逗号` , `分隔，例如：
```
preloadModules=plugins::mysql,plugins::odbc
```

## 5. 编译插件
本节以开发一个 demo 插件为例，详细介绍如何编译插件。

### 5.1 环境准备与依赖
在开始编译 DolphinDB 插件前，请确保以下依赖项和环境配置已准备完毕：

#### 5.1.1 必备依赖

**C++ 编译器**：必须使用与 DolphinDB Server 版本兼容的 g++ 版本，详见下表。

**DolphinDB 头文件**：下载并使用与 DolphinDB Server 版本一致的插件分支中的 include/ 目录。

**DolphinDB 动态库**：Windows 平台编译时，需要用到 libDolphinDB.dll。

**编译器版本要求（按运行环境区分）**

| 操作系统类型     | DolphinDB Server 版本          | 推荐 g++ 版本 |
|------------------|--------------------------------|---------------|
| Linux/Linux_JIT  | 2.00.x、3.00.0.x、3.00.1.x     | g++ 4.8.5     |
| Linux/Linux_JIT  | 3.00.2 及以上                  | g++ 8.4.0     |
| Linux_ABI        | 2.00.x、3.00.0.x、3.00.1.x     | g++ 6.2.0     |
| Linux_ABI        | 3.00.2 及以上                  | g++ 8.4.0     |

#### 5.1.2 编译宏定义
编译插件时建议添加以下宏：
- `LOCKFREE_SYMBASE`：启用 SymbolBase 类的优化；
- `_GLIBCXX_USE_CXX11_ABI=0`：为了兼容老版本编译器，编译 libDolphinDB.so 时默认使用了 `_GLIBCXX_USE_CXX11_ABI=0` 选项，因此用户在编译插件时也应该加上该选项。若使用的是开启 `ABI=1` 的服务器，则可以省略；
- `LINUX`: 指定系统版本为 Linux。

### 5.2 定义插件描述文件
```
demo,libPluginDemo.so,2.00.10
minmax,minmax,operator,1,1,0
foo,foo,system,1,1,0
```

以上描述文件 PluginDemo.txt 定义了一个名为 demo 的插件，共享库文件名为 libPluginDemo.so。插件版本号为2.00.10（请根据实际使用的插件版本进行修改）。该插件导出两个函数：
- `minmax`：在 DolphinDB 中注册为 `minmax`，类型为 operator，接受一个参数。
- `foo`：在 DolphinDB 中注册为 `foo`，类型为 system，同样接受一个参数。

写完描述文件之后，即可开始编写插件。内容请参考 [demo (*./demo*)](https://github.com/dolphindb/DolphinDBPlugin/tree/release200.15/demo) 文件夹内容。

### 5.3 编译插件

编译需要用到 [include](https://github.com/dolphindb/DolphinDBPlugin/tree/release200.15/include) 头文件，请提前选择正确的分支下载文件。编译步骤如下（以在 Linux 操作系统上编译为例）：

```
cd demo
g++ -DLINUX -fPIC -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0 -DLOCKFREE_SYMBASE -c src/Demo.cpp -I../include -o Demo.o
g++ -fPIC -shared -o libPluginDemo.so Demo.o
```
编译成功后，目录下会生成一个名为 *libPluginDemo.so* 的共享文件。

在 DolphinDB 的控制台中输入下列命令加载插件并使用：

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
更复杂的插件实现请参考 [odbc](https://github.com/dolphindb/DolphinDBPlugin/blob/release200.15/odbc) 目录下的内容。

## 6. 插件编译建议与故障排查

* 如果载入插件之后出现了 crash，可以采取尝试以下步骤。  
   1. 确保 include(*./include*) 下的头文件和 *libDolphinDB.so* 或 *libDolphinDB.dll* 实现保持一致。
   2. 确保用于编译插件的 ```gcc``` 版本和编译 *libDolphinDB.so* 或 *libDolphinDB.dll* 的版本保持一致，以免出现不同版本的编译器 ABI 不兼容的问题。
   3. 插件与 DolphinDB server 在同一个进程中运行，若插件 crash，那整个系统就会 crash。因此在开发的插件时候要注意完善错误检测机制，除了插件函数所在线程可以抛出异常（server 在调用插件函数时俘获了异常），其他线程都必须自己俘获异常，并不得抛出异常。
   4. 请确保已正确添加编译宏，详见 [5.1.2 宏定义](#512-编译宏定义)。
* 如果编译时出现链接问题（undefined reference），并且包含 std::__cxx11 字样，务必检查用于编译插件的 g++ 版本和_GLIBCXX_USE_CXX11_ABI 选项，确保与 DolphinDB 所使用的设置保持一致。
* 需要根据 DolphinDB 特性及版本号，正确选择 g++ 版本，详见编译器版本说明。
* 如果加载插件时报存在未定义的引用，可以尝试编译时链接 libDolphinDB.so，然后用 ld 命令查看是否还有其他未定义的引用。如果这时 ld 也会报错，则 DolphinDB 也无法正确加载插件。