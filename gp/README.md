# DolphinDB GP Plugin

DolphinDB 的 gp 插件可以使用 vector 、table 类型的数据进行画图，并绘绘制好的文件保存到本地。gp 插件基于 gnuplot 开发。本文档仅介绍编译构建方法。通过 [文档中心 - GP](https://docs.dolphindb.cn/zh/plugins/gp/gp.html) 查看接口介绍；通过 [CHANGELOG.md](./CHANGELOG.md) 查看版本发布记录。

## 编译构建

编译依赖库:

安装 [zlib-1.2.12](https://www.zlib.net/fossils/zlib-1.2.12.tar.gz)
```
tar -zxvf zlib-1.2.12.tar.gz
cd zlib-1.2.12
mkdir build && cd build
cmake  -DCMAKE_INSTALL_PREFIX=/tmp/libzlib ..
make -j
make install
```

安装 [libpng-1.6.35](https://codeload.github.com/glennrp/libpng/zip/refs/tags/v1.6.35)
```
unzip libpng-1.6.35.zip
cd libpng-1.6.35
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/tmp/libpng/ ..
make -j
make install
```

安装 [libjpeg-8.4.0](https://codeload.github.com/LuaDist/libjpeg/zip/refs/tags/8.4.0)
```
unzip libjpeg-8.4.0.zip
cd libjpeg-8.4.0
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/tmp/libjpeg/ ..
make -j
make install
```

安装 [libgd-gd-2.3.3 (https://codeload.github.com/libgd/libgd/zip/refs/tags/gd-2.3.3)]
```
unzip libgd-gd-2.3.3.zip
cd libgd-gd-2.3.3
```
在 CMakeLists.txt 第 319 行添加一行：
```
set_target_properties(gd PROPERTIES LINK_FLAGS "-Wl,-rpath,$ORIGIN:.")
```
然后开始编译
```
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
cp /tmp/libgd/lib/libgd.so.3.0.11 ./libgd.so.3
cp /tmp/libpng/lib/libpng16.so.16.35.0 ./libpng16.so.16
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
