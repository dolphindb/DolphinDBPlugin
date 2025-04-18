# DolphinDB Plugin

DolphinDB database supports dynamic loading of external plugins to extend system functionality. The plugin can be written in C++, and it needs to be compiled into ".so" shared libraries or ".dll" shared library files.

The shared library depends on the classes and interfaces declared in the include directory. Therefore, when compiling a plugin, you must make sure the first three digits of the plugin branch version match those of the DolphinDB version. For example: if the DolphinDB version is 1.30.21.x, the plugin version must be 1.30.21.x; if the Server version is 2.00.9.x, use plugin branch 2.00.9.x, and so on. The same version alignment is also required when loading the plugin.

## 1. Project Structures


Please switch to the corresponding git branch to view the following listed directories for each plugin. The master branch doesn't contain these directories.

* `include` The directory contains the class declarations and some tool class declarations for the core data structures of DolphinDB. These classes are important basic tools for implementing plugins.
* `demo` The directory contains a demo plugin implementation. 
* Other directories contain the implementation of the plugins.

## 2. DolphinDB Plugin Format

DolphinDB uses a text file to describe the plugin. The file format is as follows: The first line describes the plugin name and shared library file name. Each of the following lines describes the mapping between a shared library function and the DolphinDB function.
```
module name, lib file, plugin version
function name in lib, function name in DolphinDB, function type, minParamCount, maxParamCount, isAggregate, isSequential
...
...
```
**Explanation**: 

* module name: plugin module name  
* lib file: shared library file name 
* plugin version: plugin version
* function name in lib: the function name in the shared library
* function name in DolphinDB: corresponding function name in DolphinDB 
* function type: operator (accepts 1 or 2 parameters) ​​or system (accepts ​​any number of parameters)
* minParamCount: the minimum number of parameters  
* maxParamCount: the maximum number of parameters  
* isAggregate: whether it is an aggregate function. It can be 0 (default, non-aggregate function) or 1 (aggregate function).
* isSequential: whether it is an sequential function. It can be 0 (default, order-insensitive function) or 1 (order-sensitive function).   

## 3. Plugin Installation
DolphinDB develops and publishes a variety of plugins. Users can directly download and install them from the plugin marketplace. Follow the steps below to download and install a plugin:

(1) Use `listRemotePlugins` to check plugin information in the plugin repository.
```
login("admin", "123456")
listRemotePlugins()
```
**Note**:  For plugins not included in the provided list, you can compile from source. These files can be accessed from our GitHub repository by switching to the appropriate version branch.  For compilation instructions, refer to [Chapter 5](#5-compilation-example)

(2) Invoke `installPlugin` for plugin installation.
```
installPlugin("mysql")
```

## 4. Plugin Loading
Ensure that the first three digits of the plugin branch version match those of the DolphinDB version when loading plugins. If the versions do not match, it will throw an exception.  Ensure the plugins are updated promptly if you upgrades the DolphinDB server and do not modify version information manually.


### 4.1 Loading via function `loadPlugin`

Use the [`loadPlugin`](../Functions/l/loadPlugin.dita) function to load external plugins. This function accepts a file path, which describes the format of the plugin, for example:

```
loadPlugin("/YOUR_SERVER_PATH/plugins/odbc/PluginODBC.txt"); 
```

**Note**: Since version 2.00.11/1.30.23, it can be specified as the plugin name (case sensitive). The system will load the plugin through the plugin name and the configuration parameter *pluginDir*.

### 4.2 Loading through configuration parameter preloadModules

Starting from server version 1.20.0, plugins can be automatically loaded through the configuration parameter *preloadModules*. Please note that the plugin that needs to be preloaded must exist. Otherwise, there will be an exception when the server starts. Multiple plugins are separated by ',', for example:

```
preloadModules=plugins::mysql,plugins::odbc
```

## 5. Compilation Example
This section provides a detailed guide on how to compile a plugin, using the development of a demo plugin as an example.

### 5.1 Environment Setup and Dependencies

Before starting to compile a DolphinDB plugin, ensure that the following dependencies and environment setup are prepared.

#### 5.1.1 Required Dependencies

**C++ Compiler**: Use a g++ version compatible with the DolphinDB version. Refer to the table below for details.

**DolphinDB Header Files**: Download and use the `include/` directory from the plugin branch that matches the DolphinDB version.

**DolphinDB Dynamic Library**: The libDolphinDB.dll. is needed for Windows compilation.

**Compiler Version Requirements (by operating environment)**

| DolphinDB Feature  | Server Version                | Recommended g++ Version |
|--------------------|-------------------------------|-------------------------|
| Linux / Linux_JIT  | 2.00.x, 3.00.0.x, 3.00.1.x    | g++ 4.8.5               |
| Linux / Linux_JIT  | 3.00.2 and above              | g++ 8.4.0               |
| Linux_ABI          | 2.00.x, 3.00.0.x, 3.00.1.x    | g++ 6.2.0               |
| Linux_ABI          | 3.00.2 and above              | g++ 8.4.0               |

#### 5.1.2 Compilation Macro Definitions
When compiling the plugin, it is recommended to add the following macros:

- `LOCKFREE_SYMBASE`: Enable `SymbolBase` optimization.
- `_GLIBCXX_USE_CXX11_ABI=0`: To ensure compatibility with older compilers, the libDolphinDB.so is compiled with the `_GLIBCXX_USE_CXX11_ABI=0` option by default. Therefore, you should also include this option when compiling the plugin. Ignore this if the DolphinDB sets `ABI=1`.
- `LINUX`: Specify the system version as Linux.
### 5.2 Plugin Descriptor File
```
demo,libPluginDemo.so,2.00.10
minmax,minmax,operator,1,1,0
foo,foo,system,1,1,0
```
The description file above defines a plugin named ```demo```. The shared library file is named *libPluginDemo.so*. The plugin version is 2.00.10 (modify it according to the plugin used).

The plugin exports two functions. The first function is named ```minmax```. The name of the function is also `minmax` in DolphinDB. The function type is "operator" and accepts one parameter. The second function name is `echo`, the name in DolphinDB is also `echo`, the function type is "system" and accepts one argument. 

After finishing the description file, you can start writing plugins. For content, please refer to *demo* folder contents.


### 5.3 Compilation Steps

The compiler requires header files. Below are the compilation steps on a Linux system:

```
cd demo
g++ -DLINUX -fPIC -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0 -DLOCKFREE_SYMBASE -c src/Demo.cpp -I../include -o Demo.o
g++ -fPIC -shared -o libPluginDemo.so Demo.o
```

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

## 6. Tips

* If the program crashes after loading the plugin, try the following steps:
   1. Make sure that the [include](https://github.com/dolphindb/DolphinDBPlugin/tree/release200/include) headers are consistent with the libDolphinDB.so implementation.
   2. Make sure that the version of gcc used to compile the plugin is consistent with the version of `libBoardDB.so` in order to avoid the incompatibilities between different versions of the compiler ABI.
   3. The plugin and DolphinDB server run in the same process. If a plugin crashes, then the whole system will crash. Therefore, efforts should be made to improve the error detection mechanism. Only the thread where the plugin is located can throw an exception which is captured by the server. Except for the thread, all threads cannot throw exceptions and they must capture exceptions by themselves.
   4. Please make sure that the appropriate compilation macros are defined. Refer to Section 5.1.2 for details.
* If an undefined reference occurs when compiling the plugin and the error message contains std::__cxx11, please check the g++ version and `_GLIBCXX_USE_CXX11_ABI option`.
* Select a compatible g++ version based on the DolphinDB feature and version. See the compiler version requirements table for details.
* If undefined references occur when loading the plugin, try linking libDolphinDB.so during compilation. Then use the ld command to check for undefined symbol. If ld also reports errors, the plugin cannot be correctly loaded by DolphinDB.
