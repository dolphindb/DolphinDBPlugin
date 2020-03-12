# DolphinDB Zlib Plugin

### 1) How to load Plugin

start a DolphinDB instance, then execute the following command:

```
loadPlugin("path/to/DolphinDBPlugin/zlib/PluginZlib.txt");
```

### 2) Supported functions

**compressFile**

args

* inputFileName: the name and path of input file
* level(optional): range[-1, 9], default is -1(currently equivalent to level 6), 1 gives best speed, 9 gives best compression, 0 gives no compression

return

* inputFileName with a suffix ".gz"

e.g.

```
zlib::compressFile("/home/jccai/data.txt");
//Warning: If there already be a file with the same name and path, the existing file will be overwriting.
```

**decompressFile**

args

* inputFileName: the name and path of input file, must end up with ".gz"

return

* inputFileName with the ".gz" being removed

e.g.

```
zlib::decompressFile("/home/jccai/data.txt.gz");
//Warning: If there already be a file with the same name and path, the existing file will be overwriting.
```
