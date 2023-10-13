# DolphinDB mat Plugin

DolphinDB 的 mat 插件支持读取 mat 文件的数据到 DolphinDB，或将 DolphinDB 变量写入 mat 文件，且在读取或写入时自动进行类型转换。

## 1. 安装构建

### 1.1 插件加载
本插件使用了 MATLAB Runtime Installer 的外部接口，需要提前安装 MATLAB Runtime Installer。
#### 安装 MATLAB Runtime Installer R2016a
```
wget https://ssd.mathworks.com/supportfiles/downloads/R2016a/deployment_files/R2016a/installers/glnxa64/MCR_R2016a_glnxa64_installer.zip
unzip MCR_R2016a_glnxa64_installer.zip -d matlabFile
cd matlabFile
./install -mode silent -agreeToLicense  yes  -destinationFolder  /home/Matlab //安装时指定安装位置
```
确保 matlab 插件依赖库在 gcc 可搜索的路径中。通过以下命令配置：
```
cd <DolphinDBServerDir>
export LD_LIBRARY_PATH=/home/Matlab/v901/bin/glnxa64:$LD_LIBRARY_PATH
./dolphindb
```
然后启动 DolphinDB，加载插件。
```
./dolphindb
login(`admin,`123456)
loadPlugin("<PluginDir>/mat/build/PluginMat.txt");
```

### 1.2 插件编译

需要 CMake 和对应编译器即可在本地编译 mat 插件。

#### Linux

##### 使用 CMake 构建：

安装 CMake：

```
sudo apt-get install cmake
```

构建插件内容：

```
cd <PluginDir>/mat
mkdir build
cd build
cmake -DmatlabRoot=/home/Matlab/v901/ ../   //指定 matlab 的安装位置
make
```

**注意**：编译之前请确保 libDolphinDB.so 在 gcc 可搜索的路径中。可使用 `LD_LIBRARY_PATH` 指定其路径，或者直接将其拷贝到 build 目录下。

编译后目录下会产生文件 libPluginMat.so 和 PluginMat.txt。

##  2. 用户接口

### 2.1 mat::extractMatSchema

**语法**

mat::extractMatSchema(file)

**详情**

生成 mat 文件中指定数据集的结构。包括两列：字段名和数据类型。

**参数**

* file：需要读取的 mat 文件所在的绝对路径。类型为字符串。

**例子**

```
schema=mat::extractMatSchema("<FileDir>/simple.mat");
```

### 2.2 mat::loadMat

**语法**

mat::loadMat(file, [schema])

**详情**

读取一个 mat 文件。返回一个 DolphinDB 字典。

**参数**
* file：需要写入的 mat 文件所在的绝对路径。类型为 string。

* schema：包含列名和列的数据类型的表。若要改变由系统自动决定的列的数据类型，需要在 schema 表中修改数据类型。

**返回值**
返回一个字典。其 key 为变量名称，字符串类型；value 是一个矩阵或向量，为 key 指定变量对应的数据。如果一个变量是字符数组类型，对应返回值为 STRING 类型的向量。

**例子**
```
schema=mat::extractMatSchema("<FileDir>/simple.mat");
ret=loadMat("<FileDir>/simple.mat",schema);
//simple 中 t_num 变量为 double 类型的时间变量
ret=mat::convertToDatetime(ret[`t_num]);
```

### 2.3 mat::convertToDatetime

**语法**

mat::convertToDatetime(data)

**详情**

把 matlab 中以 double 储存的时间变量转换为 DolphinDB 的 DATETIME。

**参数**

* data：需要转换的变量。为 double 类型的 scalar, vector, matrix。

**返回值**

返回值是对应于参数 data 的 DATETIME 类型的 scalar, vector, matrix。

**例子**
```
schema=mat::extractMatSchema("<FileDir>/simple.mat");
ret=loadMat("<FileDir>/simple.mat",schema);
ret=mat::convertToDatetime(ret);
```

### 2.4 mat::writeMat

**语法**

mat::writeMat(file, varName, data)

**详情**

把一个矩阵写入到 mat 文件。

**参数**

* file：被写入文件的文件名。为 string 类型的 scalar。
* varName：data 写入文件后对应的变量名。为 string 类型的 scalar。
* data：需要写入的矩阵。可以是 bool, char, short, int, long, float, double类型。

**例子**
```
data = matrix([1, 1, 1], [2, 2, 2]).float()
mat::writeMat("var.mat", "var1", data)
```

## 3. 支持的数据类型

### 3.1 整型

| matlab 类型         | 对应的 DolphinDB 类型 |
| ------------------ | :------------------ |
| int8            | CHAR                |
| uint8            | SHORT                |
| int16            | SHORT                |
| uint16            | INT                |
| int32            | INT                |
| uint32            | LONG                |
| int64                 | LONG               |
| uint64              | 不支持               |

* DolphinDB 中数值类型都为有符号类型,为了防止溢出,所有无符号类型会被转化为高一阶的有符号类型。例如，无符号 CHAR 转化为有符号 SHORT，无符号 SHORT 转化为有符号 INT，等等。64位无符号类型不予支持。
* DolphinDB 不支持 unsigned long long 类型，如果 matlab 中的类型为 bigint unsigned, 可在 `loadMat` 的 schema 参数里面设置为 DOUBLE 或者 FLOAT。
* DolphinDB 中各类整型的最小值为 NULL，如 CHAR 的-128，SHORT 的-32,768，INT 的-2,147,483,648以及 LONG 的-9,223,372,036,854,775,808。
* Matlab 中的 NaN 和 Inf 会转换为 DolphinDB 的 NULL。
### 3.2 浮点数类型

| matlab 类型 | 对应的 DolphinDB 类型 |
| ---------- | :------------------ |
| single     | FLOAT              |
| double     | DOUBLE              |

### 3.3 字符串类型

| matlab 类型         | 对应的 DolphinDB 类型 |
| ------------------- | :------------------ |
| character array   | STRING              |


# ReleaseNotes:

## 新功能

* 新增支持多线程读写 mat 文件。（**1.30.22**）

# 故障修复

* 接口 mat::writeMat 新增对参数 *varName* 非法输入值的报错。（**1.30.22**）
