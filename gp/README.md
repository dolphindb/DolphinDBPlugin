# DolphinDB GP Plugin

使用该插件可以使用DolphinDB的vector和table中的数据进行画图保存到本地。

## 1. 安装构建

## 1.1 预编译安装

在DolphinDBPlugin/gp/bin/linux64目录下有预先编译的插件文件，在DolphinDB中执行以下命令导入gp插件：
```
cd DolphinDB/server //进入DolphinDB server目录
export LD_LIBRARY_PATH=<PluginDir>/gp/bin/linux64:$LD_LIBRARY_PATH //指定动态库位置 
export GNUPLOT_PS_DIR=<PluginDir>/gp/bin/linux64/PostScript //指定gp插件PostScript的目录位置
./dolphindb //启动 DolphinDB server
loadPlugin("<PluginDir>/gp/bin/linux64/PluginGp.txt");
```

## 1.2 编译安装

### 使用cmake构建：

安装cmake：

```
sudo apt-get install cmake
```

构建插件内容：

```
cd <PluginDir>/gp
mkdir build
cd build
cmake  ../
make
```

**注意**:编译之前请确保libDolphinDB.so在gcc可搜索的路径中。可使用`LD_LIBRARY_PATH`指定其路径，或者直接将其拷贝到build目录下。

编译后目录下会产生文件libPluginGp.so和PluginGp.txt。


##  2. 用户接口

### 2.1 gp::plot

#### 语法

gp::plot(data, style, path, [props]）

#### 详情

使用DolphinDB中的数据进行画图，并以eps的文件格式保存到本地。

#### 参数

* data: 画图数据。一个向量或者是一个表为一组数据，其中表以第一列和第二列作为x、y轴坐标，可以使用ANY类型的VECTOR作为多组数据画在同一张图片上。
* style: 画图的样式。类型为String常量，值可以是line，point，linesoint，impulses，dots，step，errorbars，histogram，boxes，boxerrorbars，ellipses，circles。
* path: 保存图片路径。类型为String常量。
* props: 画图特性，类型为一个字典。可以有如下键值:
    * title: 每个数据组的标识，类型为String常量或者是String向量。
    * xRange: 图片的X轴范围。类型为数值类型的向量，包含两个元素。
    * yRange: 图片的Y轴范围。类型为数值类型的向量，包含两个元素。
    * xLabel: X轴标签。类型为String常量。
    * yLabel: Y轴标签。类型为String常量。
    * size: 图片大小，大小为英寸。类型为数值类型的向量，包含两个元素。
    * xTics: X轴单位间隔。类型为数值类型的常量。
    * yTics: Y轴单位间隔。类型为数值类型的常量。
    以下属性可以设置每个独立图像的特性。
    * lineColor: 线条颜色。类型为字符串常量或者是向量。颜色可以是black, red, green, blue, cyan, magenta, yellow, navy, purple, olive,  orange, violet, pink, white, gray。
    * lineWidth: 线条宽度。类型为数值类型的常量或者是向量。
    * pointType: 画点的形状。类型为数值类型的常量或者是向量。取值为0到13。
    * pointSize: 点大小。类型为数值类型的常量或者是向量。

#### 返回值

#### 例子

```
data=rand(1000.0,10000)
a=array(ANY,2)
data1=rand(1000.0,10000)
a[0]=data
a[1]=data1
gp::plot(a,"line","/home/zmx/precipitation.eps")
```

