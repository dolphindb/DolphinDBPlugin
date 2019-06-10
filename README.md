## DolphinDB Plugin
DolphinDB 支持动态的载入外部插件，拓展系统功能。插件仅支持使用C++编写，并且需要编译成so共享库或者dll共享库文件。
## Directory Structures
* ```include```目录包含了DolphinDB的核心数据结构的类声明和一些工具类声明，这些类是实现插件的重要基础工具。  
* ```demo```目录包含了一个demo插件的实现。  
* ```odbc```目录包含了一个odbc插件的实现。
## Loading Plugin
使用```loadPlugin```函数来加载外部插件，该函数接受一个文件路径，文件中描述了插件的格式。  

## DolphinDB Plugin Format
DolphinDB使用一个文本文件来描述插件,该文件格式如下:  
首行描述了插件名字和共享库文件名。  
接下来的每一行都描述了一个共享库函数和dolphinDB函数的映射关系。  
```
module name，lib file
function name in lib, function name in dolphindb, function type，minParamCount, maxParamCount, isAggregated
...
```
**解释**：
* module name: 模块名  
* lib file: 共享文件库文件名  
* function name in lib: 共享库中导出的函数名  
* function name in dolphindb: dolphindb中的对应函数名  
* function type: operator或者system  
* minParamCount: 最小参数个数  
* maxParamCount：最大参数个数  
* isAggregated: 是否为聚集函数  

## Example
PluginDemo.txt:
```
demo,libPluginDemo.so 
minmax,minmax,operator,1,1,0
foo,foo,system,1,1,0
```
上述的描述文件定义了一个名为```demo```的插件，共享库文件名为```libPluginDemo.so```.  
插件导出了两个函数，第一个函数名字为```minmax```,该函数在dolphinDB中名字同样是```minmax```,operator类型，接受一个参数;第二个函数名字为```echo```,dolphinDB中名字同样是```echo```, system类型，接受一个参数.  

写完描述文件之后，就可以开始编写插件了，内容请参考```demo```文件夹内容。

编译需要用到DolphinDB的核心库```libDolphinDB.so```,该核心库实现了```include```目录下声明的类。
编译步骤如下
```
cd demo
g++ -DLINUX -fPIC -c src/Demo.cpp -I../include -o Demo.o
g++ -fPIC -shared -o libPluginDemo.so Demo.o -lDolphinDB
```
编译成功后，目录下会生成一个名为```libPluginDemo.so```的共享文件。

在dolphindb的控制台中输入下列命令加载插件并使用。
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

更复杂的插件实现请参考```odbc```目录下的内容。

## Tips
* 建议使用```ld```命令检查下编译器链接是否成功，so中是否存在未定义的引用。如果```ld```报错，那么DolphinDB也无法正确加载插件。
* 如果载入插件之后出现了crash，可以采取尝试以下步骤。  
   1. 确保```include```下的头文件和```libDolphinDB.so```实现保持一致.
   2. 确保用于编译插件的```gcc```版本和编译```libDolphinDB.so```的版本保持一致，以免出现不同版本的编译器ABI不兼容的问题。
* 如果编译时出现链接问题（```undefined reference```），并且包含```std::__cxx11```字样，务必检查用于编译插件的```gcc```版本（```gcc 6.2.0 且有--disable-libstdcxx-dual-abi```）
