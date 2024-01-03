# DolphinDB zip Plugin

该插件用于解压 ZIP 格式文件。

## 1. 安装构建

### 1.1. 预编译安装

**Linux**

预先编译的插件文件存放在 DolphinDBPlugin/releasexxx/zip/bin/linux64 目录下。将其下载至 /DolphinDB/server/plugins/zip。

### 1.2. 使用 CMake 编译构建


### 1.2.1 LINUX
安装 CMake

```bash
sudo apt install cmake
```

编译整个项目
```bash
mkdir build
cd build
cp /path_to_dolphindb/libDolphinDB.so ../lib
cmake ..
make -j
```

编译后将生成 libPluginZip.so 文件。


### 1.2.2 WINDOWS
```
mkdir build                                           # 新建build目录
cp <ServerDir>/libDolphinDB.dll build                 # 拷贝 libDolphinDB.dll 到build目录下
cd build
cmake  ../ -G "MinGW Makefiles"
mingw32-make -j4
```
编译后将生成 libPluginZip.dll 文件。

### 1.3 编译依赖库zlib-1.2.11
下载zlib-1.2.11[https://github.com/madler/zlib/archive/refs/tags/v1.2.11.zip]
解压zlib-1.2.11.zip
然后进行编译
```
mkdir build
cd build
cmake  ../ -G "MinGW Makefiles"
mingw32-make -j4
```
复制zlib静态库
```
copy .\libzlibstatic.a PATH_TO_PLUGIN_ZIP\lib\Win64 #PATH_TO_PLUGIN_ZIP为zip插件路径
```
复制zlib头文件
```
copy ..\zlib.h PATH_TO_PLUGIN_ZIP\lib_src #PATH_TO_PLUGIN_ZIP为zip插件路径
```

### 1.4. 插件加载

通过以下脚本加载插件：

```
loadPlugin("/path_to_pluginZip/PluginZip.txt");
```

## 2. 接口说明

### 2.1. zip::unzip

**语法**

zip::unzip(zipFileName, [outputDir], [callback], [zipEncode], [password])

**参数**

- `zipFileName` 字符串标量，表示 ZIP 文件路径。仅支持绝对路径。

- `outputDir` 字符串标量，表示解压文件的输出路径，可选。仅支持绝对路径。若该参数不传或为""时，则解压路径和压缩包路径相同。注意：指定路径下的同名文件将被覆盖。

- `callback` 函数，仅接收一个 STRING 类型的参数，可选。

- `zipEncode` 字符串标量，仅接收一个 STRING 类型的参数，可选。表示zip文件内部的文件名编码。目前仅支持 gbk 和 utf-8 两种编码。
  在Windows系统上，默认为 gbk 编码，Linux系统上，默认为 utf-8 编码。例子：如在Windows上如果要解压以 utf-8 编码的zip文件则需要指定该参数为 "utf-8"

- `password` 字符串标量。压缩包的密码。

**详情**

用于解压指定的 ZIP 格式文件。返回一个由解压文件路径组成的字符串向量。支持通过回调函数，对解压出的文件进行处理。当 ZIP 文件中包含多个文件时，可以实现每解压出一个文件，便被回调函数处理，提高 unzip 的处理效率。同时可以指定zip文件内部的文件名编码格式，确保解压后的文件路径编码正确。

## 示例

```dolphindb
filenames = zip::unzip("/path_to_zipFile/test.zip", "/path_to_output/", func)

print(filenames)
["/path_to_output/test.csv"]
```

### 2.2. zip::zip

**语法**

zip::zip(zipFileName, fileOrFolderPath, [compressionLevel], [password])

**参数**

- `zipFileName`  zip文件的路径。类型为STRING的SCALAR。

- `fileOrFolderPath` 需要压缩的文件夹或者是文件的文件路径。类型为STRING的SCALAR。

- `compressionLevel` 压缩等级。目前支持”faster”和“better”。”faster“会以最快的压缩速度进行压缩，“better”则会以最高的压缩率来进行压缩。类型为STRING的SCALAR。

- `password` 压缩密码。类型为STRING的SCALAR。

**详情**

压缩一个文件夹或者一个文件。

## 示例

```dolphindb
zip::zip("/hdd1/commit/DolphinDBPlugin/zip/test/test.zip", "/hdd1/commit/DolphinDBPlugin/zip/data") 
```

# Release Notes

## 1.30.23

### 新增功能

- 新增接口 `zip::zip`，支持压缩文件和文件夹。
- 接口 `zip::unzip` 新增参数 *password*，支持解压时的解密功能。

## 1.30.22

### 新增功能

- 新增对 Windows 版本的支持。
- 增加 zipEncode 参数，提供指定zip文件中文件名编码格式的功能。

### 功能优化

- 优化部分报错信息及终端输出。

### 故障修复

- 修复使用 `zip::unzip` 时若抛出异常时，已有 handle 未及时关闭的问题。
- 修复了路径为中文时的乱码问题。
