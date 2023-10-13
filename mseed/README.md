# DolphinDB mseed Plugin

DolphinDB的mseed插件可以读取miniSEED文件的数据到DolphinDB的内存表中，且可以将DolphinDB的一段连续时间的采样值写入到miniSEED格式的文件中。

本插件使用了IRIS的[libmseed开源库](https://github.com/iris-edu/libmseed)的读写接口。

## 1. 安装构建

### 1.1 直接加载插件
```
 loadPlugin("<PluginDir>/mseed/build/PluginMseed.txt");
```
请注意，若使用 Windows 插件，加载时必须指定绝对路径，且路径中使用"\\\\"或"/"代替"\\"。

### 1.2 自行编译安装

在<PluginDir>/mseed/bin目录下有linux64和win64对应的依赖库，只需要cmake和对应编译器（Linux为g++；Windows为MinGW）即可在本地编译mseed插件。

#### 1.2.1 Linux

##### 1.2.1.1 使用cmake构建：

安装cmake：

```
sudo apt-get install cmake
```

构建插件内容：

```
cd <PluginDir>/mseed
mkdir build
cd build
cmake  ../
make
```

**注意**：编译之前请确保libDolphinDB.so在gcc可搜索的路径中。可使用`LD_LIBRARY_PATH`指定其路径，或者直接将其拷贝到build目录下。

编译后目录下会产生文件libPluginMseed.so和PluginMseed.txt。

#### 1.2 Windows

##### 1.2.1 在Windows环境中需要使用CMake和MinGW编译

- 下载安装[MinGW](http://www.mingw-w64.org/)。确保将bin目录添加到系统环境变量Path中。
- 下载安装[cmake](https://cmake.org/)。

##### 1.2.2 使用cmake构建：

在编译开始之前，要将libDolphinDB.dll拷贝到build文件夹下。

构建插件内容：

```
cd <PluginDir>\mseed
mkdir build                                             # 新建build目录
COPY <ServerDir>/libDolphinDB.dll build                 # 拷贝 libDolphinDB.dll 到build目录下
cd build
cmake  ../ -G "MinGW Makefiles"
mingw32-make -j4
```

编译后目录下会产生文件libPluginMseed.dll和PluginMseed.txt。

##  2. 用户接口

### 2.1 mseed::read

#### 语法

mseed::read(file)

#### 详情

读取一个miniSEED文件，返回一个DolphinDB的内存表。

#### 参数

* file: 需要读取的miniSEED文件所在的绝对路径。类型为字符串的常量。

#### 返回值

返回一个DolphinDB内存表，包含如下字段：
* value：读取到的采样值。类型为int, float或double。
* time：采样值对应的时间戳。类型为timestamp。
* id：采样值所在块的sid。类型为symbol。

#### 例子
```
ret=mseed::read("<FileDir>/SC.JZG.00.BHZ.D.2013.001");
```

### 2.2 mseed::write

#### 语法

mseed::write(file, sid, startTime, sampleRate, value, [overwrite=false])

#### 详情

将一段连续的采样值写入到miniSEED文件。

#### 参数
* file：需要写入的miniSEED文件所在的绝对路径。类型为string的常量。
* sid：写入到miniSEED文件的一个块的sid。类型为string的常量。
* startTime：写入到miniSEED文件一个块的startTime。类型为timestamp的常量。
* sampleRate：写入到miniSEED文件的sampleRate。类型为int, long, float或double的常量。
* value：写入miniSEED文件的采样值的向量。类型为int, float或double。
* overwrite：是否覆盖之前写入的数据，默认为false。类型为bool的常量。

#### 返回值

如果写入成功，返回true。

#### 例子
```
time=timestamp(2013.01.01);
sampleRate=100.0;
vec=rand(100, 100);
ret=mseed::write("/home/zmx/aaa", "XFDSN:SC_JZG_00_B_H_Z", time, sampleRate, vec);
```

### 2.3 mseed::parse

#### 语法

mseed::parse(data)

#### 详情

解析miniseed格式的字节流，返回一个DolphinDB的内存表。

#### 参数

* data: miniseed格式的字节流。为string类型或char类型的vector。

#### 返回值

返回一个DolphinDB内存表，包含如下字段：
* value：读取到的采样值。类型为int, float或double。
* time：采样值对应的时间戳。类型为timestamp。
* id：采样值所在块的sid。类型为symbol。

#### 例子
```
fin=file("/media/zmx/aaa");
buf=fin.readBytes(512);
ret=mseed::parse(buf);

stringBuf=concat(buf);
ret=mseed::parse(stringBuf);
```

### 2.4 mseed::parseStream

#### 语法

mseed::parseStream(data)

#### 详情

解析miniseed格式的字节流，返回一个字典，包含一个内存表和成功解析的字节流长度。如果解析失败，返回一个仅包含成功解析的字节流长度的字典。

#### 参数

* data: miniseed格式的字节流。为string类型或char类型的vector。

#### 返回值

返回一个dolphindb字典，包含如下键值:
* ret[`data]:
    一个dolphindb内存表，包含如下字段：
    * value：读取到的采样值。类型为int, float或double。
    * time：采样值对应的时间戳。类型为timestamp。
    * id：采样值所在块的sid。类型为symbol。
* ret[`size]:
    成功解析的字节流的长度。类型为long的常量。
* ret[`metaData]：
    一个dolphindb内存表，包含如下字段：
    * id：采样值所在块的sid。类型为symbol。
    * startTime：采样开始时间。类型为timestamp。
    * receivedTime：接收数据时间。类型为timestamp。
    * actualCount：实际解析出来的数据个数。类型为int。
    * expectedCount：miniSEED包头指定的采样值个数。类型为int。
    * sampleRate：miniSEED采样率。类型为double。
    
#### 例子
```
fin=file("/media/zmx/aaa");
buf=fin.readBytes(512);
ret=mseed::parseStream(buf);

stringBuf=concat(buf);
ret=mseed::parseStream(stringBuf);
```

### 2.5 mseed::parseStreamInfo

#### 语法

mseed::parseStreamInfo(data)

#### 详情

解析miniseed格式的字节流的块信息，返回一个字典，包含一个内存表和成功解析的字节流长度。

#### 参数

* data: miniseed格式的字节流。为string类型或char类型的vector。

#### 返回值

返回一个dolphindb字典，包含如下键值:
* ret[`data]:
  一个dolphindb内存表，包含如下字段：
  * sid：读取到的mseed块的分量名称。字符串类型。
  * blockLen：读取到的mseed块的长度。类型为int。
* ret[`size]:
  成功解析的字节流的长度。类型为int的常量。

#### 例子
```
fin=file("/media/zmx/aaa");
buf=fin.readBytes(512);
ret=mseed::parseStreamInfo(buf);

stringBuf=concat(buf);
ret=mseed::parseStreamInfo(stringBuf);
```

### 2.6 mseed::streamize

#### 语法

mseed::streamize(data, sampleRate, [blockSize])

#### 详情

按照所在行数的顺序将表中的采样数据转换成miniseed格式的CHAR Vector。需要提前对sid列、时间戳进行排序。

#### 参数
* data：采样数据信息的一张表，包含如下列：
    * 第一列为sid， 类型为symbol或string。
    * 第二列为时间戳。类型为TimeStamp。
    * 第三列为采样数值。类型为int, float或double。
    * 只会取前三列作为输入参数，第四列及其以后不做处理。
* sampleRate：采样频率。类型为int, long, float或double。
* blockSize：miniSEED格式的块大小，默认是512。类型为int。
#### 使用
```
sidVec = take("XFDSN:SN_C0059_40_E_I_E", 1000).symbol()
tsVec = now() + 1..1000
dataVec = 1..1000
data = table(sidVec as sid, tsVec as ts, dataVec as data)
ret = mseed::streamize(data, 1000)
```

# ReleaseNotes:

## 故障修复

* 接口 mseed::write 新增对参数 *startTime* 非法输入值的报错。（**1.30.22**）
* 接口 mseed::parseStreamInfo 新增对非法数据或非法 SID（Station Identifier）的报错。（**1.30.22**）
* 接口 mseed::parse 新增对空字符串数据的检查。（**1.30.22**）
* 接口 mseed::read 新增对非法参数输入值的检查。（**1.30.22**）
