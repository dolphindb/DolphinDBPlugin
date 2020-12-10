# DolphinDB mat Plugin

DolphinDB的mat插件可以读取mat文件的数据加载到DolphinDB的一个矩阵的字典中。

## 1. 安装构建

### 1.1 插件加载
本插件使用了MATLAB Runtime Installer的外部接口，需要提前安装MATLAB Runtime Installer。
#### 安装MATLAB Runtime Installer R2016a
```
wget https://ssd.mathworks.com/supportfiles/downloads/R2016a/deployment_files/R2016a/installers/glnxa64/MCR_R2016a_glnxa64_installer.zip
unzip MCR_R2016a_glnxa64_installer.zip -d matlabFile
cd matlabFile
./install -mode silent -agreeToLicense  yes  -destinationFolder  /home/zmx/Matlab //安装时指定安装位置
```
需要指定matlab的接口库的链接位置，
```
cd <DolphinDBServerDir>
export LD_LIBRARY_PATH=/home/zmx/Matlab/v901/bin/glnxa64:$LD_LIBRARY_PATH
./dolphindb
```
然后在DolphinDB客户端加载插件。
```
 loadPlugin("<PluginDir>/mat/build/PluginMat.txt");
```

### 1.2 插件编译

需要cmake和对应编译器(linux为g++,window为MinGW)即可在本地编译mseed插件。

#### linux

##### 使用cmake构建：

安装cmake：

```
sudo apt-get install cmake
```

构建插件内容：

```
cd <PluginDir>/mat
mkdir build
cd build
cmake -DmatlabRoot=/home/zmx/Matlab/v901/ ../   //指定matlab的安装位置
make
```

**注意**:编译之前请确保libDolphinDB.so在gcc可搜索的路径中。可使用`LD_LIBRARY_PATH`指定其路径，或者直接将其拷贝到build目录下。

编译后目录下会产生文件libPluginMat.so和PluginMat.txt。




##  2. 用户接口

### 2.1 mat::extractMatSchema

#### 语法

mat::extractMatSchema(file)

#### 详情

生成mat文件中指定数据集的结构，包括两列：字段名和数据类型。

#### 参数

* file: 需要读取的mat文件所在的绝对路径。类型为字符串。

#### 例子

```
schema=mat::extractMatSchema("<FileDir>/simple.mat");
```

### 2.2 mat::loadMat

#### 语法

mat::loadMat(file, [schema])

#### 详情

读取一个mat文件。返回一个DolphinDB字典。

#### 参数
* file：需要写入的mat文件所在的绝对路径。类型为string。

* schema: 包含列名和列的数据类型的表。若要改变由系统自动决定的列的数据类型，需要在schema表中修改数据类型。

#### 返回值
返回一个字典ret。
varName: 变量名称。字符串类型。
ret[varName]: 变量的数据。类型为对应数据类型的矩阵。如果是一个变量是字符数组类型，返回值则是一个类型为STRING的向量。

#### 例子
```
schema=mat::extractMatSchema("<FileDir>/simple.mat");
ret=mat::loadMat("<FileDir>/simple.mat",schema);
```

### 2.3 mat::convertToDatetime

#### 语法

mat::convertToDatetime(data)

#### 详情

把matlab中以double储藏的时间变量转换到DolphinDB的DATETIME。

#### 参数

* data：需要转换的变量。类型为double类型的scalar,vector,matrix。

#### 返回值

返回值是对应与参数data的timedate类型的scalar,vector,matrix。

#### 例子
```
schema=mat::extractMatSchema("<FileDir>/simple.mat");
ret=loadMat("<FileDir>/simple.mat",schema);
ret=mat::convertToDatetime(ret);
```

## 3. 支持的数据类型

### 3.1 整型

| matlab类型          | 对应的DolphinDB类型 |
| ------------------ | :------------------ |
| int8            | CHAR                |
| uint8            | SHORT                |
| int16            | SHORT                |
| uint16            | INT                |
| int32            | INT                |
| uint32            | LONG                |
| int64                 | LONG               |
| uint64              | (不支持)LONG               |

* DolphinDB中数值类型都为有符号类型,为了防止溢出,所有无符号类型会被转化为高一阶的有符号类型。例如，无符号CHAR转化为有符号SHORT，无符号SHORT转化为有符号INT，等等。64位无符号类型不予支持。
* DolphinDB不支持 unsigned long long 类型，如果matlab中的类型为 bigint unsigned, 可在`loadMat`的schema参数里面设置为 DOUBLE 或者 FLOAT。
* DolphinDB中各类整形的最小值为NULL值，如 CHAR 的-128，SHORT的-32,768，INT的-2,147,483,648以及LONG的-9,223,372,036,854,775,808。
* Matlab中的NaN和Inf会转换到DolphinDB的NULL值。
### 3.2 浮点数类型

| matlab类型  | 对应的DolphinDB类型 |
| ---------- | :------------------ |
| single     | FLOAT              |
| double     | DOUBLE              |

### 3.3 字符串类型

| matlab类型           | 对应的DolphinDB类型 |
| ------------------- | :------------------ |
| character array   | STRING              |