# DolphinDB Zlib Plugin

DolphinDB 的 zlib 插件，支持文件到文件的 zlib 压缩与解压缩。

## 在插件市场安装插件

### 版本要求

zlib插件目前支持版本：[relsease200](https://github.com/dolphindb/DolphinDBPlugin/blob/release200/zlib/README_CN.md), [release130](https://github.com/dolphindb/DolphinDBPlugin/blob/release130/zlib/README_CN.md), [release120](https://github.com/dolphindb/DolphinDBPlugin/blob/release120/zlib/README_CN.md), [release110](https://github.com/dolphindb/DolphinDBPlugin/blob/release110/zlib/README_CN.md)。您当前查看的插件版本为release200，请使用DolphinDB 2.00.X版本server。若使用其它版本server，请切换至相应插件分支。

### 安装步骤

1. 在DolphinDB 客户端中使用 [listRemotePlugins](../../funcs/l/listRemotePlugins.dita) 命令查看插件仓库中的插件信息。

    ```
    login("admin", "123456")
    listRemotePlugins(, "http://plugins.dolphindb.cn/plugins/")
    ```

2. 使用 [installPlugin](../../funcs/i/installPlugin.dita) 命令完成插件安装。

    ```
    installPlugin("zlib")
    ```

   返回：<path_to_Zlib_plugin>/PluginZlib.txt


3. 使用 loadPlugin 命令加载插件（即上一步返回的.txt文件）。

    ```
    loadPlugin("<path_to_Zlib_plugin>/PluginZlib.txt")
    ```

   **注意**：若使用 Windows 插件，加载时必须指定绝对路径，且路径中使用"\\\\"或"/"代替"\\"。

## 接口说明

请注意：使用插件函数前需使用`loadPlugin`函数导入插件。

### zlib::compressFile

**语法**

zlib::compressFile(inputFileName, [level])

**参数**

* `inputFileName`： 输入文件名及路径，类型为 string
* `level`：压缩等级（可选），范围[-1, 9]，默认为-1（当前等同于级别6），1提供最佳速度，9提供最佳压缩，0不提供压缩

**详情**

将输入文件压缩为 .gz 文件，返回压缩后的文件名

**例子**
      
```shell
zlib::compressFile("/home/jccai/data.txt");
# 将/home/jccai/data.txt压缩为/home/jccai/data.txt.gz
# 注意：若输出文件有同名文件，则会被覆盖
```

### zlib::decompressFile

**语法**

zlib::decompressFile(inputFileName)

**参数**

* `inputFileName`：压缩文件的文件名及路径，应以 .gz 结尾，类型为 string

**详情**

将输入文件解压缩，并返回加压缩后的文件名

**例子**

```shell
zlib::decompressFile("/home/jccai/data.txt.gz");
# 将/home/jccai/data.txt.gz解压为/home/jccai/data.txt
# 注意：若输出文件有同名文件，则会被覆盖
```

## 附录：（预）编译安装（可选）

如果不通过插件市场安装插件，也可以选择预编译安装或编译安装方式。

### 预编译安装

用户可以导入预编译好的Zlib插件（DolphinDB安装包中或者bin目录下)。

在DolphinDB中执行以下命令导入Zlib插件：

Linux环境：

```
loadPlugin("/path/to/plugins/zlib/PluginZlib.txt")
```

Windows环境(假设安装在C盘上)：

```
loadPlugin("C:/path/to/zlib/PluginZlib.txt")
```

**注意**：若使用 Windows 插件，加载时必须指定绝对路径，且路径中使用"\\\\"或"/"代替"\\"。

### 编译安装

可使用以下方法编译Zlib插件，编译成功后通过以上方法导入。

#### 在Linux环境中编译安装

##### cmake编译

```
mkdir build
cd build
cmake ..
make
```

**注意:** 编译之前请确保libDolphinDB.so在gcc可搜索的路径中。可使用LD_LIBRARY_PATH指定其路径，或者直接将其拷贝到build目录下。

编译之后目录下会产生libPluginZlib.so文件

## ReleaseNotes:

### v2.00.10

#### 优化

- 优化部分报错信息

#### 新功能

- 增加对压缩文件夹下的所有文件的支持
