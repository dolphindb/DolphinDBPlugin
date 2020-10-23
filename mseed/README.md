# DolphinDB Mseed Plugin

* 读取miniSEED文件的数据，加载到DolphinDB的向量中。
* 将DolphinDB的向量写入到一个miniSEED文件。
## 1. 安装构建

#### 1.1 插件加载
#### Linux
如果已经获得编译好的mseed插件动态库(libPluginMseed.so)，执行如下脚本即可加载插件。
```
 loadPlugin("<PluginDir>/mseed/build/PluginMseed.txt");
```

#### 1.2 编译安装

在<PluginDir>/mseed/bin目录下有对应与linux64和win64的依赖库，只需要cmake和g++编译器即可编译插件库。
#### Linux

###### 使用cmake构建：

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
* value：读取到的采样值。可以为int, float, double和string类型。
* time：采样值对应的时间戳。类型为DolphinDB的timestamp类型。
* id：采样值所在块的sid。类型为DolphinDB的symbol类型。
#### 例子

```
ret=mseed::read("<FileDir>/SC.JZG.00.BHZ.D.2013.001");
```

### 2.2 mseed::write

#### 语法

mseed::write(file,sid,startTime,sampleRate,value,[cover])

#### 详情

将一个DolphinDB的vector写入到miniSEED文件。

#### 参数
* file：需要写入的miniSEED文件所在的绝对路径。类型为string。
* sid：写入到miniSEED文件的一个块的sid。类型为string。
* startTime：写入到miniSEED文件的一个块的startTime。类型为timestamp。
* sampleRate：写入到miniSEED文件的sampleRate。类型为double。
* value：写入miniSEED文件的采样值的向量。支持int，float，double类型。
* cover：是否覆盖写，默认为否。类型为bool。

#### 例子
```
time=timestamp(2013.01.01)
sampleRate=100.0
vec=rand(100,100)
ret=mseed::write('/home/zmx/aaa',"XFDSN:SC_JZG_00_B_H_Z",time,sampleRate,vec)
```



