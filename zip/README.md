# DolphinDB zip Plugin

该插件用于解压.zip文件

## 构建

### 使用cmake编译构建

安装cmake

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

libPluginZip.so文件会在编译后生成。

## 插件加载

编译生成 libPluginZip.so 之后，通过以下脚本加载插件：

```
loadPlugin("/path_to_pluginZip/PluginZip.txt");
```

## 接口说明

**zip::unzip(zipFileName, outputDir, callback)**

**参数**

`zipFileName` 为字符串标量，是压缩包路径，仅支持绝对路径。

`outputDir` 为字符串标量，可选，是解压文件输出路径，仅支持绝对路径。不传该参数或者该参数为""时，解压路径默认和压缩包路径相同，如果指定路径下有同名文件将直接覆盖。

`callback` 为函数标量, 可选，用于处理解压文件。该函数有一个参数，该参数是字符串标量。

**函数详情**

该函数用于解压文件。需指定压缩文件，可指定解压文件路径和回调函数，传入回调函数后即可实现边解压边处理文件，更加高效。函数返回解压生成的文件名字。

## 使用示例

filenames = zip::unzip("/path_to_zipFile/test.zip", "/path_to_output/", func);


