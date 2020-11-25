# DolphinDB mseed Plugin
DolphinDB的mseed插件可以读取miniSEED文件的数据加载到DolphinDB的内存表中，并且可以将DolphinDB的一段连续时间的采样值写入到miniSEED格式的文件中。

本插件使用了IRIS的[libmseed开源库](https://github.com/iris-edu/libmseed)的读写接口。
## 1. 安装构建

### 1.1 编译安装

在<PluginDir>/mseed/bin目录下有对应与linux64和win64的依赖库，只需要cmake和对应编译器(linux为g++,window为MinGW)即可在本地编译mseed插件。

#### linux

##### 使用cmake构建：

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

**注意**:编译之前请确保libDolphinDB.so在gcc可搜索的路径中。可使用`LD_LIBRARY_PATH`指定其路径，或者直接将其拷贝到build目录下。

编译后目录下会产生文件libPluginMseed.so和PluginMseed.txt。

#### Windows

##### 在Windows环境中需要使用CMake和MinGW编译

- 下载安装[MinGW](http://www.mingw.org/)。确保将bin目录添加到系统环境变量Path中。
- 下载安装[cmake](https://cmake.org/)。

##### 使用cmake构建：

在编译开始之前，要将libDolphinDB.dll拷贝到build文件夹。

构建插件内容：

```
cd <PluginDir>\mseed
mkdir build                                                        # 新建build目录
cp <ServerDir>/libDolphinDB.dll build                 # 拷贝 libDolphinDB.dll 到build目录下
cd build
cmake  ../ -G "MinGW Makefiles"
mingw32-make -j4
```

编译后目录下会产生文件libPluginMseed.dll和PluginMseed.txt。

### 1.2 插件加载
```
 loadPlugin("<PluginDir>/mseed/build/PluginMseed.txt");
```


##  2. 用户接口

### 2.1 mseed::read

#### 语法

mseed::read(file)

#### 详情

读取一个miniSEED文件，返回一个DolphinDB的内存表。

#### 参数

* file: 需要读取的miniSEED文件所在的绝对路径。类型为字符串。

#### 返回值

返回值是一个dolphindb内存表，有如下字段：
* value：读取到的采样值。可以为int, float和 double类型。
* time：采样值对应的时间戳。类型为timestamp类型。
* id：采样值所在块的sid。类型为symbol类型。

#### 例子

```
ret=mseed::read("<FileDir>/SC.JZG.00.BHZ.D.2013.001");
```

### 2.2 mseed::write

#### 语法

mseed::write(file, sid, startTime, sampleRate, value, [overwrite=false])

#### 详情

将一个DolphinDB的vector写入到miniSEED文件。

#### 参数
* file：需要写入的miniSEED文件所在的绝对路径。类型为string。
* sid：写入到miniSEED文件的一个块的sid。类型为string。
* startTime：写入到miniSEED文件的一个块的startTime。类型为timestamp。
* sampleRate：写入到miniSEED文件的sampleRate。类型为double。
* value：写入miniSEED文件的采样值的向量。支持int，float，double类型。
* overwrite：是否覆盖写，默认为否。类型为bool。

#### 返回值

如果写入成功，返回一个bool真值。

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

* data: miniseed格式的字节流。类型为string或者是char类型的vector。

#### 返回值

返回值是一个dolphindb内存表，有如下字段：
* value：读取到的采样值。可以为int, float和double类型。
* time：采样值对应的时间戳。类型为timestamp类型。
* id：采样值所在块的sid。类型为symbol类型。

#### 例子
```
fin=file("/media/zmx/aaa");
buf=fin.readBytes(512);
ret=mseed::parse(buf);

stringBuf=concat(buf);
ret=mseed::parse(stringBuf);
```