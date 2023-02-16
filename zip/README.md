# DolphinDB zip Plugin

该插件用于解压 ZIP 格式文件。

## 1. 安装构建

### 1.1. 预编译安装

**Linux**

预先编译的插件文件存放在 DolphinDBPlugin/releasexxx/zip/bin/linux64 目录下。将其下载至 /DolphinDB/server/plugins/zip。

### 1.2. 使用 CMake 编译构建

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

### 1.3. 插件加载

通过以下脚本加载插件：

```
loadPlugin("/path_to_pluginZip/PluginZip.txt");
```

## 2. 接口说明

### 2.1. zip::unzip

**语法**

zip::unzip(zipFileName, outputDir, callback)

**参数**

`zipFileName` 字符串，表示 ZIP 文件路径。仅支持绝对路径。

`outputDir` 字符串，表示解压文件的输出路径，可选。仅支持绝对路径。若该参数不传或为""时，则解压路径和压缩包路径相同。注意：指定路径下的同名文件将被覆盖。

`callback` 一个函数，仅接收一个 STRING 类型的参数，可选。

**详情**

用于解压指定的 ZIP 格式文件。返回一个由解压文件路径组成的字符串向量。支持通过回调函数，对解压出的文件进行处理。当 ZIP 文件中包含多个文件时，可以实现每解压出一个文件，便被回调函数处理，提高 unzip 的处理效率。

## 示例

```python
filenames = zip::unzip("/path_to_zipFile/test.zip", "/path_to_output/", func);

print(filenames)
["/path_to_output/test.csv"]
```