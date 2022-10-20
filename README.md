# DolphinDB Plugin

DolphinDB plugins currently have release200, release130, release120 and release110. Each plugin version corresponds to a DolphinDB server version. Please refer to the corresponding branch according to the server version you are using.

DolphinDB database supports dynamic loading of external plugins to extend system functionality. The plug-in can be written in C++, and it needs to be compiled into ".so" shared libraries or ".dll" shared library files.

- [DolphinDB Plugin](#dolphindb-plugin)
  - [1. Directory Structures](#1-directory-structures)
  - [2. Load Plugins](#2-load-plugins)
    - [2.1 Load via function loadPlugin](#21-load-via-function-loadplugin)
    - [2.2 Load through configuration parameter preloadModules](#22-load-through-configuration-parameter-preloadmodules)
  - [3. DolphinDB Plugin Format](#3-dolphindb-plugin-format)
  - [4. Example](#4-example)
  - [5. Tips](#5-tips)

## 1. Directory Structures

Please switch to the corresponding branch to view the following listed directories for each plugin. The master branch doesn’t contain these directories.

* ```include```The directory contains the class declarations and some tool class declarations for the core data structures of DolphinDB. These classes are important basic tools for implementing plug-ins.
* ```demo```The directory contains a demo plug-in implementation. 
* Other directories contain the implementation of the plugins.

## 2. Load Plugins

### 2.1 Load via function loadPlugin

Use the [`loadPlugin`](https://www.dolphindb.cn/cn/help/loadPlugin.html) function to load external plugins. This function accepts a file path, which describes the format of the plugin, for example:

```
loadPlugin("/YOUR_SEVER_PATH/plugins/odbc/PluginODBC.txt"); 
```

### 2.2 Load through configuration parameter preloadModules

Starting from server version 1.20.0, plugins can be automatically loaded through the configuration parameter *preloadModules*. Please note that the plugin that needs to be preloaded must exist. Otherwise, there will be an exception when the server starts. Multiple plugins are separated by ',', for example:

```
preloadModules=plugins::mysql,plugins::odbc
```


## 3. DolphinDB Plugin Format

DolphinDB uses a text file to describe the plugin. The file format is as follows: The first line describes the plugin name and shared library file name.
Each of the following lines describes the mapping between a shared library function and the DolphinDB function.

```
module name, lib file
function name in lib, function name in DolphinDB, function type, minParamCount, maxParamCount, isAggregate
...
```
**Explanation**：

* module name: plugin module name  
* lib file: shared library file name 
* function name in lib: the function name in the shared library
* function name in DolphinDB: corresponding function name in DolphinDB 
* function type: operator or system 
* minParamCount: the minimum number of parameters  
* maxParamCount: the maximum number of parameters  
* isAggregate: whether it is an aggregate function  


## 4. Example
PluginDemo.txt:
```
demo,libPluginDemo.so 
minmax,minmax,operator,1,1,0
foo,foo,system,1,1,0
```
The description file above defines a plugin named ```demo```. The shared library file is named ```libPluginDemo.so```.

The plugin exports two functions. The first function is named ```minmax```. The name of the function is also ```minmax``` in DolphinDB. The function type is "operator" and accepts one parameter. The second function name is ```echo```, the name in DolphinDB is also ```echo```, the function type is "system" and accepts one argument. 

After finishing the description file, you can start writing plugins. For content, please refer to ```demo``` folder contents.

The compiler needs to use DolphinDB's core library ```libDolphinDB.so```, which implements the classes declared in ```include``` directories.

```
cd demo
g++ -DLINUX -fPIC -DLOCKFREE_SYMBASE -c src/Demo.cpp -I../include -o Demo.o
g++ -fPIC -shared -o libPluginDemo.so Demo.o -lDolphinDB
```

**Note:**

- To be compatible with older compilers, you can compile libDolphinDB.so with option -D_GLIBCXX_USE_CXX11_ABI=0. If the server is the latest version of ABI=1, it is not necessary to add the option. 
- For the path specified by compiling option -L, you can modify it to the path of DolphinDB dynamic library.

After successful compilation, a shared library file named *libPluginDemo.so* will be generated under the directory.

Enter the following command in the DolphinDB console to load the plugin and use it.

```
>loadPlugin(Path to PluginDemo.txt); // Load the plugin
(minmax,echo)
>use demo; // Import the plugin's namespace
>demo::minmax([12,3,4]); // You can also use minmax([12,3,4])
[3,12]
>demo::echo("foo");
foo
>echo(1);
1
```
## 5. Tips
* We recommend that you use the command ld to check if the compiler link is successful and if there are undefined references in the so. If the command ld generates an error, then DolphinDB can not load the plug-in correctly.
* If the program crashes after loading the plugin, try the following steps:
   1. Make sure that the [include](https://github.com/dolphindb/DolphinDBPlugin/tree/release200/include) headers are consistent with the libDolphinDB.so implementation.
   2. Make sure that the version of gcc used to compile the plugin is consistent with the version of `libBoardDB.so` in order to avoid the incompatibilities between different versions of the compiler ABI.
   3. The plugin and DolphinDB server run in the same process. If a plugin crashes, then the whole system will crash. Therefore, efforts should be made to improve the error detection mechanism. Only the thread where the plugin is located can throw an exception which is captured by the server. Except for the thread, all threads cannot throw exceptions and they must capture exceptions by themselves.
   4. Please make sure LOCKFREE_SYMBASE has been added.
* If an undefined reference occurs when compiling the plugin and the error message contains std::__cxx11, please check the gcc version used to compile the plugin (gcc 6.2.0 with --disable-libstdcxx-dual-abi).
