## DolphinDB introduction
DolphinDB is an extremely fast time series database and data analytical system with unlimited scaling capacity. It has built-in parallel and distributed computing supports. It comes with battery included for real time data processing and analytics across unlimited computing nodes. It is integrated with an easy-to-use fully featured programming language and a high-volume high-velocity streaming analytics system. DolphinDB offers operational simplicity, scalability, fault tolerance, and concurrency. To learn more about DolphinDB, please go to 

```
Downloads: http://www.dolphindb.com/downloads.html
Manual: http://dolphindb.com/help/
Examples: http://dolphindb.com/examples.html
```

## DolphinDB Plugin
DolphinDB supports dynamic loading of external plugins to extend system functionality. The plug-in only supports writing in C++, and it needs to be compiled into ".so" shared libraries or ".dll" shared library files.

## Directory Structures
* ```include```The directory contains the class declarations and some tool class declarations for the core data structures of DolphinDB. These classes are important basic tools for implementing plug-ins.
* ```demo```The directory contains a demo plug-in implementation. 
* ```odbc```The directory contains an implementation of the odbc plugin.
## Loading Plugin
Use the ```loadPlugin``` function to load an external plugin that accepts a file path that describes the format of the plugin.

## DolphinDB Plugin Format

DolphinDB uses a text file to describe the plugin. The file format is as follows:
The first line describes the plug-in name and shared library file name.  
Each of the following lines describes the mapping between a shared library function and the dolphinDB function. 
```
module name，lib file
function name in lib, function name in dolphindb, function type，minParamCount, maxParamCount, isAggregated
...
```
**Explanation**：
* module name: plugin module name  
* lib file: shared library file name 
* function name in lib: The function name in the shared library
* function name in dolphindb: corresponding function name in dolphindb 
* function type: operator or system  
* minParamCount: the minimum number of parameters  
* maxParamCount: the maximum number of parameters  
* isAggregated:  whether it is an aggregate function  

## Example
PluginDemo.txt:
```
demo,libPluginDemo.so 
minmax,minmax,operator,1,1,0
foo,foo,system,1,1,0
```
The above description file defines a plugin named ```demo```. The shared library file is named ```libPluginDemo.so```.

The plug-in exports two functions. The first function is named ```minmax```. The name of the function is also ```minmax``` in dolphinDB. The function type is "operator" and accepts one parameter. The second function name is ```echo```, the name in dolphinDB is also ```echo```, the function type is "system" and accepts one argument. 


After writing the description file, you can start writing plugins. For content, please refer to ```demo``` folder contents.


The compiler needs to use DolphinDB's core library ```libDolphinDB.so```, which implements the classes declared in ```include``` directories.
The compilation steps are as follows
```
cd demo
g++ -DLINUX -fPIC -c src/Demo.cpp -I../include -o Demo.o
g++ -fPIC -shared -o libPluginDemo.so Demo.o -lDolphinDB
```

After successful compilation, a shared library file named ```libPluginDemo.so``` will be generated under the directory.


Enter the following command in the dolphinb console to load the plugin and use it.
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

For more complex plugin implementation please refer to the ```odbc``` directory.

## Tips
* It is recommended that you use the ```ld``` command to check if the compiler link is successful and there are undefined references in the so. If ```ld``` error, then DolphinDB can not load the plug-in correctly.
* If the program is crashed after loading the plug-in, you can try the following steps. 
   1. Make sure that the ```include``` headers are consistent with the ```libDolphinDB.so``` implementation.Make sure that the version of ``gcc``` used to compile the plugin is consistent with the version of ``libBoardDB.so```, so as to avoid the incompatibilities between different versions of the compiler ABI.
