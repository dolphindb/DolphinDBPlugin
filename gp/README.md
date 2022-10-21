# DolphinDB GP Plugin

通过该插件可以对 DolphinDB 的 vector 和 table 中的数据进行画图，并保存文件到本地。

## 1. 安装构建

## 1.1 预编译安装

需要执行以下命令，指定插件依赖文件 PostScript 的位置：
```
export GNUPLOT_PS_DIR=<PluginDir>/gp/bin/linux64/PostScript //指定 gp 插件依赖文件 PostScript 的位置
```
注意：可从 DolphinDBPlugin/gp/bin/linux64 目录中下载  PostScript。

DolphinDBPlugin/gp/bin/linux64 目录下存放了预先编译的插件文件，在 DolphinDB 中执行以下命令导入 gp 插件：
```
cd DolphinDB/server //进入 DolphinDB server 目录
./dolphindb //启动 DolphinDB server
loadPlugin("<PluginDir>/gp/bin/linux64/PluginGp.txt");
```

## 1.2 编译安装

编译依赖库:

安装(zlib-1.2.12)[https://www.zlib.net/fossils/zlib-1.2.12.tar.gz]
```
tar -zxvf zlib-1.2.12.tar.gz
cd zlib-1.2.12
mkdir build && cd build
cmake  -DCMAKE_INSTALL_PREFIX=/tmp/libzlib ..
make -j
make install
```

安装(libpng-1.6.35)[https://codeload.github.com/glennrp/libpng/zip/refs/tags/v1.6.35]
```
unzip libpng-1.6.35.zip
cd libpng-1.6.35
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/tmp/libpng/ ..
make -j
make install
```

安装(libjpeg-8.4.0)[https://codeload.github.com/LuaDist/libjpeg/zip/refs/tags/8.4.0]
```
unzip libjpeg-8.4.0.zip
cd libjpeg-8.4.0
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/tmp/libjpeg/ ..
make -j
make install
```

安装(libgd-gd-2.3.3)[https://codeload.github.com/libgd/libgd/zip/refs/tags/gd-2.3.3]
```
unzip libgd-gd-2.3.3.zip
cd libgd-gd-2.3.3
```
在 CMakeLists.txt 第319行添加
```
set_target_properties(gd PROPERTIES LINK_FLAGS "-Wl,-rpath,$ORIGIN:.")
mkdir build && cd build
CMAKE_PREFIX_PATH=/tmp/libzlib:/tmp/libpng:/tmp/libjpeg  cmake -DCMAKE_INSTALL_PREFIX=/tmp/libgd/  -DENABLE_PNG=1 -DENABLE_JPEG=1 ..
make -j
make intall
```

安装 readline-devel
```
yum install readline-devel
```

构建插件内容：

```
cd <PluginDir>/gp
mkdir build
cd build

```
拷贝前面安装好的 zlib, png, jpeg, gd 库到编译目录 build。
```
cp /tmp/libgd/lib64/libgd.so.3.0.11 ./libgd.so.3
cp /tmp/libpng/lib64/libpng16.so.16 .
cp /tmp/libjpeg/lib/libjpeg.so .
cp /tmp/libzlib/lib/libz.so.1.2.12 ./libz.so.1
ln -s libgd.so.3 libgd.so
```

```
cmake  ../
make
```

**注意**：编译之前请确保 libDolphinDB.so 在 gcc 可搜索的路径中。直接将其拷贝到 build 目录下。

编译后将在目录下产生文件 libPluginGp.so 和 PluginGp.txt。


##  2. 用户接口

### 2.1 gp::plot

#### 语法

gp::plot(data, style, path, [props]）

#### 详情

使用 DolphinDB 中的数据进行画图，并以 eps 的文件格式保存到本地。

#### 参数

* data: 画图数据。一个向量、由向量组成的 tuple 或一个表。若为表，则用其第一列和第二列分别表示 x 轴、y 轴数据。
* style: 字符串，表示画图的样式。包含以下值："line", "point", "linesoint", "impulses", "dots", "step", "errorbars", "histogram", "boxes", "boxerrorbars", "ellipses", "circles"。
* path: 字符串，表示保存图片的路径。
* props: 字典，表示画图特性。包含以下键值:
    * title: 字符串标量或向量，表示每个数据组的标识。
    * xRange: 数值型向量，表示图片的 X 轴范围。为数值类型的向量，包含两个元素。
    * yRange: 图片的 Y 轴范围。为数值类型的向量，包含两个元素。
    * xLabel: 字符串，表示 X 轴标签。
    * yLabel: 字符串，表示 Y 轴标签。
    * size: 图片比例，1为初始长度。为数值类型的向量，包含两个元素，表示长和宽的比列。
    * xTics: 数值型标量，表示 X 轴的单位间隔。
    * yTics: 数值型标量，表示 Y 轴的单位间隔。
  
    以下属性可以设置每个独立图像的特性。
    * lineColor: 字符串标量或向量，表示线条颜色。包含以下值："black", "red", "green", "blue", "cyan", "magenta", "yellow", "navy", "purple", "olive",  "orange", "violet", "pink", "white", "gray"。
    * lineWidth: 数值型标量或向量，表示线条宽度。
    * pointType: 整型标量或向量，表示画点的形状。取值为0到13。
    * pointSize: 数值型标量或向量，表示点的大小。
    * smooth: 字符串标量或向量，表示数据平滑程度。可以为 "csplines" 或者是 "bezier"。

#### 返回值

#### 例子

```
data=(rand(20,20),rand(20,20),rand(20,20),rand(20,20),rand(20,20),rand(20,20),rand(20,20),rand(20,20),rand(20,20),rand(20,20),rand(20,20),rand(20,20),rand(20,20),rand(20,20),rand(20,20))
prop=dict(STRING,ANY)
prop[`lineColor]=["black", "red", "green", "blue", "cyan", "magenta", "yellow", "navy", "purple", "olive",  "orange", "violet", "pink", "white", "gray"]
prop["xTics"]=2
prop["yTics"]=5
prop["title"]="l"+string(1..15)
re=gp::plot(data,"line",WORK_DIR+"/test.eps",prop)
re=gp::plot(data,"line",WORK_DIR+"/test.png",prop)
re=gp::plot(data,"line",WORK_DIR+"/test.jpeg",prop)
```