# DolphinDB Zlib插件

### 1) 加载插件

启动DolphinDB实例，执行下述命令: 

```
loadPlugin("path/to/DolphinDBPlugin/zlib/PluginZlib.txt");
```

### 2) 支持的功能

**compressFile**

压缩指定的文件，注意若有同名输出文件，则会覆盖已有文件

参数

* inputFileName: 输入文件
* level: 压缩系数，默认为-1，相当于6，取值范围为[-1,9]，0为没有压缩，9为最高压缩

返回值

* 输入文件名添加".gz"后缀

使用案例

```
zlib::compressFile("/home/jccai/data.txt");
```

**decompressFile**

解压缩指定的文件，注意若有同名输出文件，则会覆盖已有文件

参数

* inputFileName: 输入文件，必须以".gz"结尾

返回值

* 输入文件名去掉".gz"后缀

使用案例

```
zlib::decompressFile("/home/jccai/data.txt.gz");
```