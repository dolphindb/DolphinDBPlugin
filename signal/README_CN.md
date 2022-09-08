
<!-- @import "[TOC]" {cmd="toc" depthFrom=1 depthTo=2 orderedList=false} -->

<!-- code_chunk_output -->

- [DolphinDB Signal Plugin](#dolphindb-signal-plugin)
  - [构建](#构建)
  - [插件加载](#插件加载)
- [API](#api)
  - [1. signal::dct](#1-signaldct)
  - [2. signal::dst](#2-signaldst)
  - [3. signal::dwt](#3-signaldwt)
  - [4. signal::idwt](#4-signalidwt)
  - [5. signal::dctParallel](#5-signaldctparallel)
  - [6. signal::fft](#6-signalfft)
  - [7. signal::fft!](#7-signalfft)
  - [8. signal::ifft](#8-signalifft)
  - [9. signal::ifft!](#9-signalifft)
  - [10. signal::fft2](#10-signalfft2)
  - [11. signal::fft2!](#11-signalfft2)
  - [12. signal::ifft2](#12-signalifft2)
  - [13. signal::ifft2!](#13-signalifft2)
  - [14. signal::secc](#14-signalsecc)
- [示例](#示例)
  - [例1 dct离散余弦变换](#例1-dct离散余弦变换)
  - [例2 dst离散正弦变换](#例2-dst离散正弦变换)
  - [例3 dwt离散小波变换](#例3-dwt离散小波变换)
  - [例4 idwt反离散小波变换](#例4-idwt反离散小波变换)
  - [例5 dctParallel离散余弦变换分布式版本](#例5-dctparallel离散余弦变换分布式版本)
  - [例6 fft一维快速傅立叶变换](#例6-fft一维快速傅立叶变换)
  - [例7 ifft一维快速傅立叶逆变换](#例7-ifft一维快速傅立叶逆变换)
  - [例8 fft2二维快速傅立叶变换](#例8-fft2二维快速傅立叶变换)
  - [例9 fft2二维快速傅立叶逆变换](#例9-fft2二维快速傅立叶逆变换)
  - [例10 secc波形互相关](#例10-secc波形互相关)

<!-- /code_chunk_output -->

# DolphinDB Signal Plugin

DolphinDB的signal插件对四个基础的信号处理函数（离散正弦变换、离散余弦变换、离散小波变换、反离散小波变换）进行了封装。用户可以在DolphinDB数据库软件中加载该插件以使用这四个函数进行信号处理。
新增离散余弦变换的分布式版本。

signal插件目前支持版本：[relsease200](https://github.com/dolphindb/DolphinDBPlugin/blob/release200/signal/README_CN.md), [release130](https://github.com/dolphindb/DolphinDBPlugin/blob/release130/signal/README_CN.md)。您当前查看的插件版本为release200，请使用DolphinDB 2.00.X版本server。若使用其它版本server，请切换至相应插件分支。

## 构建

该插件使用CMake编译（version >= 3.10)

```
mkdir build
cd build
cmake ..
make
```
## 插件加载

编译生成 libPluginSignal.so 之后，通过以下脚本加载插件：

```
loadPlugin("/path/to/PluginSignal.txt");
```

# API
## 1. signal::dct
对离散信号作离散余弦变换，返回变换序列
**语法**

```
signal::dct(X)
```

**参数**
- X: 输入的离散信号序列，应当是一个int或double类型的vector。

**返回值**

返回变换后的序列向量，与输入向量等长，元素类型为double。

## 2. signal::dst
对离散信号作离散正弦变换，返回变换序列
**语法**

```
signal::dst(X)
```

**参数**
- X: 输入的离散信号序列，应当是一个int或double类型的vector。

**返回值**

返回变换后的序列向量，与输入向量等长，元素类型为double。

## 3. signal::dwt
对离散信号作一维离散小波变换，返回由变换序列组成的table
**语法**

```
signal::dwt(X)
```

**参数**
- X: 输入的离散信号序列，应当是一个int或double类型的vector。

**返回值**

返回变换序列组成的table，包含两个字段：cA，cD。cA对应变换后的近似部分序列，cD对应变换后的细节部分序列，两个序列等长。

## 4. signal::idwt
对一维离散小波变换得到的两个序列作逆变换，返回得到的信号序列
**语法**

```
signal::idwt(X,Y)
```

**参数**
- X: 输入的近似部分序列（cA），应当是一个int或double类型的vector。
- Y: 输入的细节部分序列（cD），应当是一个int或double类型的vector。

**返回值**

返回逆变换得到的信号序列。

## 5. signal::dctParallel
离散余弦变换的分布式版本，对离散信号作离散余弦变换，返回变换序列
**语法**

```
signal::dct(ds)
```

**参数**
- ds: 输入的数据源元组，包含若干个分区，分布在若干个控制节点中。

**返回值**

返回变换后的序列向量，与输入向量等长，元素类型为double。

## 6. signal::fft

一维快速傅立叶变换

**语法**

```
signal::fft(X,[n,norm])
```

**参数**

- X：要进行傅立叶变换的数据，类型为一维实数或复数vector。
- n：傅立叶变换后输出向量的长度，默认为X的长度,类型为int。
- norm：标准化模式，默认为"backward"不做标准化，"forward"将变换结果乘以1/n，"ortho"将变换结果乘以1/sqrt(n),类型为string。

**返回值**

返回变换后长度为n的复数向量

## 7. signal::fft!

输入向量X可被覆盖的一维快速傅立叶变换。

>仅当X为复数且X长度不小于n时，X前n项被输出覆盖。

语法、参数、返回值与fft相同。

## 8. signal::ifft

一维快速傅立叶逆变换

**语法**

```
signal::ifft(X,[n,norm])
```

**参数**

- X：要进行傅立叶变换的数据，类型为一维实数或复数vector。
- n：傅立叶变换后输出向量的长度，默认为X的长度,类型为int。
- norm：标准化模式，默认为"backward",参数为"forward"时不做标准化，"backward"将变换结果乘以1/n，"ortho"将变换结果乘以1/sqrt(n),类型为string。

**返回值**

返回变换后长度为n的复数向量

## 9. signal::ifft!

输入向量X可被覆盖的一维快速傅立叶逆变换。

>仅当X为复数且X长度不小于n时，X前n项被输出覆盖。

语法、参数、返回值与ifft相同。

## 10. signal::fft2

二维快速傅立叶变换

**语法**

```
signal::fft2(X,[s,norm])
```

**参数**

- X：要进行傅立叶变换的数据，类型为实数或复数矩阵。
- s：含有两个正整数的向量，分别对应傅立叶变换后输出矩阵的行数和列数，默认为X的行数和列数。
- norm：标准化模式，默认为"backward"不做标准化，设n为矩阵中元素的个数，则"forward"将变换结果乘以1/n，"ortho"将变换结果乘以1/sqrt(n),类型为string。

**返回值**

返回变换后行列数为s的复数矩阵

## 11. signal::fft2!

输入矩阵X可被覆盖的二维快速傅立叶变换。

>仅当X为复数矩阵且X行列数均不小于s时，X被输出覆盖。

语法、参数、返回值与fft2相同。

## 12. signal::ifft2

二维快速傅立叶变换

**语法**

```
signal::ifft2(X,[s,norm])
```

**参数**

- X：要进行傅立叶变换的数据，类型为实数或复数矩阵。
- s：含有两个正整数的向量，分别对应傅立叶变换后输出矩阵的行数和列数，默认为X的行数和列数。
- norm：标准化模式，默认为"backward"。设n为矩阵中元素的个数，则参数为"forward"时不做标准化，"backward"将变换结果乘以1/n，"ortho"将变换结果乘以1/sqrt(n),类型为string。

**返回值**

返回变换后行列数为s的复数矩阵

## 13. signal::ifft2!

输入矩阵X可被覆盖的二维快速傅立叶逆变换。

>仅当X为复数矩阵且X行列数均不小于s时，X被输出覆盖。

语法、参数、返回值与ifft2相同。

## 14. signal::secc

波形互相关

**语法**

```
signal::secc(data,template,k[,moveout,weight])
```

**参数**

+ data：波形数据，长为l一维实数向量。
+ tempale：含有n段长为m波形数据的矩阵，每段数据存为矩阵中的一列，且l不小于m。
+ k：int型参数，且k不小于2*m，建议为2的幂次方。
+ movetout：时差，长度为n的double型一维向量，默认为全为0。
+ weight：权重,长度为n的double型一维向量，默认为全为1。

**返回值**

cccSum：含有n列的矩阵，每列长为l-m+1，对应template的第n列与data互相关的结果。

# 示例

## 例1 dct离散余弦变换

```
path="/path/to/PluginSignal.txt"
loadPlugin(path)
X = [1,2,3,4]
```
对信号作离散余弦变换：

```
> signal::dct(X)
[5,-2.23044235292127,-2.411540739456585E-7,-0.15851240125353]
```

## 例2 dst离散正弦变换

```
path="/path/to/Pluginignal.txt"
loadPlugin(path)
X = [1,2,3,4]
```
对信号作离散正弦变换：

```
> signal::dst(X)
[15.388417979126893,-6.88190937668141,3.632712081813623,-1.624597646358306]
```

## 例3 dwt离散小波变换

```
path="/path/to/PluginSignal.txt"
loadPlugin(path)
X = [1,2,3]
```
对信号作离散小波变换：

```
> signal::dwt(X)
cA                cD                
----------------- ------------------
2.121320343559643 -0.707106781186548
4.949747468305834 -0.707106781186548
```

## 例4 idwt反离散小波变换

```
path="/path/to/PluginSignal.txt"
loadPlugin(path)
X = [2.121320343559643,4.949747468305834]
Y = [-0.707106781186548,-0.707106781186548]
```
对序列作反离散小波变换：

```
> signal::dwt(x,y)
[1,2,3.000000000000001,4.000000000000001]
```
## 例5 dctParallel离散余弦变换分布式版本
```
f1=0..9999
f2=1..10000
t=table(f1,f2)
db = database("dfs://rangedb_data", RANGE, 0 5000 10000)
signaldata = db.createPartitionedTable(t, "signaldata", "f1")
signaldata.append!(t)
signaldata=loadTable(db,"signaldata")
ds=sqlDS(<select * from signaldata >)
loadPlugin("/path/to/PluginSignal.txt")
use signal
signal::dctParallel(ds);
```

## 例6 fft一维快速傅立叶变换

```
path="/path/to/PluginSignal.txt"
loadPlugin(path)
X = [1,2,3,4]
```
对X作一维快速傅立叶变换:

```
>signal::fft(X);
[10+0i,-2+2i,-2+0i,-2-2i]
```

## 例7 ifft一维快速傅立叶逆变换

```
path="/path/to/Pluginignal.txt"
loadPlugin(path)
X = [1,2,3,4]
```

对X作一维快速傅立叶逆变换:
```
>signal::ifft(X);
[2.5+0i,-0.5-0.5i,-0.5+0i,-0.5+0.5i]
```

## 例8 fft2二维快速傅立叶变换

```
path="/path/to/Pluginignal.txt"
loadPlugin(path)
X = matrix([1,2,3,4],[4,3,2,1])
```

对X作二维快速傅立叶变换:
```
>fft2(X);
#0    #1   
----- -----
20+0i 0+0i 
0+0i  -4+4i
0+0i  -4+0i
0+0i  -4-4i
```

## 例9 fft2二维快速傅立叶逆变换

```
path="/path/to/Pluginignal.txt"
loadPlugin(path)
X = matrix([1,2,3,4],[4,3,2,1])
```

对X作二维快速傅立叶逆变换:
```
>ifft2(X);
#0     #1       
------ ---------
2.5+0i 0+0i     
0+0i   -0.5-0.5i
0+0i   -0.5+0i  
0+0i   -0.5+0.5i
```

## 例10 secc波形互相关

```
path="/path/to/Pluginignal.txt"
loadPlugin(path)
```

然后计算互相关

```
>x=[1, 2, 1, -1, 0, 3];
>y=matrix([1,3,2],[4,1,5]);
>secc(x,y,4);
#0                 #1               
------------------ -----------------
0.981980506061966  0.692934867183583
0.327326835353989  0.251976315339485
-0.377964473009227 0.327326835353989
0.422577127364258  0.536745040121693
```