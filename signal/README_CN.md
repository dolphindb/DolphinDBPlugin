<!-- @import "[TOC]" {cmd="toc" depthFrom=1 depthTo=2 orderedList=false} -->

<!-- code_chunk_output -->


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
  - [15. signal::abs](#15-signalabs)
    - [语法](#语法)
    - [参数](#参数)
    - [返回值](#返回值)
  - [16. signal::mul](#16-signalmul)
    - [语法](#语法-1)
    - [参数](#参数-1)
    - [返回值](#返回值-1)
- [示例](#示例)
  - [例1 dct 离散余弦变换](#例1-dct-离散余弦变换)
  - [例2 dst 离散正弦变换](#例2-dst-离散正弦变换)
  - [例3 dwt 离散小波变换](#例3-dwt-离散小波变换)
  - [例4 idwt 反离散小波变换](#例4-idwt-反离散小波变换)
  - [例5 dctParallel 离散余弦变换分布式版本](#例5-dctparallel-离散余弦变换分布式版本)
  - [例6 fft 一维快速傅立叶变换](#例6-fft-一维快速傅立叶变换)
  - [例7 ifft 一维快速傅立叶逆变换](#例7-ifft-一维快速傅立叶逆变换)
  - [例8 fft2 二维快速傅立叶变换](#例8-fft2-二维快速傅立叶变换)
  - [例9 fft2 二维快速傅立叶逆变换](#例9-fft2-二维快速傅立叶逆变换)
  - [例10 secc 波形互相关](#例10-secc-波形互相关)

<!-- /code_chunk_output -->

# DolphinDB Signal 插件

DolphinDB的 Signal 插件对四个基础的信号处理函数（离散正弦变换、离散余弦变换、离散小波变换、反离散小波变换）进行了封装。用户可以在 DolphinDB 数据库软件中加载该插件以使用这四个函数进行信号处理。
新增离散余弦变换的分布式版本。

Signal 插件目前支持版本：[release200](https://github.com/dolphindb/DolphinDBPlugin/blob/release200/signal/README_CN.md) 和 [release130](https://github.com/dolphindb/DolphinDBPlugin/blob/release130/signal/README_CN.md)。您当前查看的插件版本为 release200，请使用 DolphinDB 2.00.X 版本 server。若使用其它版本 server，请切换至相应插件分支。

## 构建

该插件使用 CMake 编译（version >= 3.10)

```
mkdir build
cd build
cmake ..
make
```
## 插件加载

编译生成 *libPluginSignal.so* 之后，通过以下脚本加载插件：

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

- X: 输入的离散信号序列，应当是一个 int 或 double 类型的 vector。

**返回值**

返回变换后的序列向量，与输入向量等长，元素类型为 double。

## 2. signal::dst
对离散信号作离散正弦变换，返回变换序列
**语法**

```
signal::dst(X)
```

**参数**

- X: 输入的离散信号序列，应当是一个 int 或 double 类型的 vector。

**返回值**

返回变换后的序列向量，与输入向量等长，元素类型为 double。

## 3. signal::dwt
对离散信号作一维离散小波变换，返回由变换序列组成的 table。
**语法**

```
signal::dwt(X)
```

**参数**

- X: 输入的离散信号序列，应当是一个 int 或 double 类型的 vector。

**返回值**

返回变换序列组成的 table，包含两个字段：cA，cD。cA 对应变换后的近似部分序列，cD 对应变换后的细节部分序列，两个序列等长。

## 4. signal::idwt
对一维离散小波变换得到的两个序列作逆变换，返回得到的信号序列
**语法**

```
signal::idwt(X,Y)
```

**参数**

- X: 输入的近似部分序列（cA），应当是一个 int 或 double 类型的 vector。
- Y: 输入的细节部分序列（cD），应当是一个 int 或 double 类型的 vector。

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

返回变换后的序列向量，与输入向量等长，元素类型为 double。

**注意**

由于 `dctParallel` 函数多用于多线程并发计算，为避免系统负载过高而严重影响性能，请勿多线程使用该函数。

## 6. signal::fft

一维快速傅立叶变换

**语法**

```
signal::fft(X,[n,norm])
```

**参数**


- X：要进行傅立叶变换的数据，类型为一维实数或复数 vector。
- n：傅立叶变换后输出向量的长度，默认为X的长度，类型为 int。
- norm：标准化模式，默认为 "backward" 不做标准化，"forward" 将变换结果乘以 1/n，"ortho"将变换结果乘以 1/sqrt(n)，类型为string。

**返回值**

返回变换后长度为n的复数向量

## 7. signal::fft!

输入向量X可被覆盖的一维快速傅立叶变换。

**注意**

>仅当 X 为复数且 X 长度不小于 n 时，X 前 n 项被输出覆盖。

语法、参数、返回值与 `fft` 相同。

## 8. signal::ifft

一维快速傅立叶逆变换

**语法**

```
signal::ifft(X,[n,norm])
```

**参数**


- X：要进行傅立叶变换的数据，类型为一维实数或复数 vector。
- n：傅立叶变换后输出向量的长度，默认为 X 的长度，类型为 int。
- norm：标准化模式，默认为 "backward",参数为 "forward" 时不做标准化，"backward" 将变换结果乘以 1/n，"ortho" 将变换结果乘以 1/sqrt(n)，类型为 string。

**返回值**

返回变换后长度为n的复数向量

## 9. signal::ifft!

输入向量X可被覆盖的一维快速傅立叶逆变换。

**注意**

> 仅当 X 为复数且 X 长度不小于 n 时，X前n项被输出覆盖。

语法、参数、返回值与ifft相同。

## 10. signal::fft2

二维快速傅立叶变换

**语法**

```
signal::fft2(X,[s,norm])
```

**参数**


- X：要进行傅立叶变换的数据，类型为实数或复数矩阵。
- s：含有两个正整数的向量，分别对应傅立叶变换后输出矩阵的行数和列数，默认为 X 的行数和列数。
- norm：标准化模式，默认为 "backward" 不做标准化，设n为矩阵中元素的个数，则 "forward" 将变换结果乘以 1/n，"ortho" 将变换结果乘以 1/sqrt(n)，类型为 string。

**返回值**

返回变换后行列数为s的复数矩阵

## 11. signal::fft2!

输入矩阵X可被覆盖的二维快速傅立叶变换。

>仅当X为复数矩阵且X行列数均不小于 s 时，X 被输出覆盖。

语法、参数、返回值与 `fft2` 相同。

## 12. signal::ifft2

二维快速傅立叶变换

**语法**

```
signal::ifft2(X,[s,norm])
```

**参数**


- X：要进行傅立叶变换的数据，类型为实数或复数矩阵。
- s：含有两个正整数的向量，分别对应傅立叶变换后输出矩阵的行数和列数，默认为 X 的行数和列数。
- norm：标准化模式，默认为 "backward"。设n为矩阵中元素的个数，则参数为 "forward" 时不做标准化，"backward" 将变换结果乘以 1/n，"ortho" 将变换结果乘以 1/sqrt(n)，类型为string。

**返回值**

返回变换后行列数为 s 的复数矩阵

## 13. signal::ifft2!

输入矩阵 X 可被覆盖的二维快速傅立叶逆变换。

**注意**

>仅当 X 为复数矩阵且 X 行列数均不小于 s 时，X 被输出覆盖。

语法、参数、返回值与 `ifft2` 相同。

## 14. signal::secc

波形互相关

**语法**

```
signal::secc(data,template,k[,moveout,weight])
```

**参数**


+ data：波形数据，长为 l 一维实数向量。
+ tempale：含有 n 段长为 m 波形数据的矩阵，每段数据存为矩阵中的一列，且 l 不小于 m。
+ k：int 型参数，且 k 不小于 2*m，建议为 2 的幂次方。
+ movetout：时差，长度为 n 的 double 型一维向量，默认为全为 0。
+ weight：权重，长度为 n 的 double 型一维向量，默认为全为 1。

**返回值**

cccSum：含有 n 列的矩阵，每列长为 l-m+1，对应 template 的第 n 列与 data 互相关的结果。

## 15. signal::abs

对复数进行求模。

### 语法

```
signal::abs(data)
```

### 参数

+ data: 需要进行求的数据，类型为复数的 scalar 和 vector。

### 返回值

如果参数 data 为复数的 scalar，返回 double的 scalar。如果参数 data 为复数的 vector，返回 double 的 vector。

## 16. signal::mul

对复数进行乘法运算。

### 语法

```
signal::mul(data, num)
```

### 参数

+ data: 需要进行乘法的数据，类型为复数的 scalar 和 vector。
* num: 乘数。类型为 double 的 scalar。

### 返回值

如果参数 data 为复数的 scalar，返回复数的 scalar。如果参数 data 为复数的 vector，返回复数的 vector。

# 示例

## 例1 dct 离散余弦变换

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

## 例2 dst 离散正弦变换

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

## 例3 dwt 离散小波变换

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

## 例4 idwt 反离散小波变换

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
## 例5 dctParallel 离散余弦变换分布式版本
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

## 例6 fft 一维快速傅立叶变换

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

## 例7 ifft 一维快速傅立叶逆变换

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

## 例8 fft2 二维快速傅立叶变换

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

## 例9 fft2 二维快速傅立叶逆变换

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

## 例10 secc 波形互相关

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

# Release Notes

## 2.00.11

### 故障修复

- 修复接口 `signal::ifft` 中参数 *n*（ 傅立叶变换后输出向量的长度）的计算逻辑问题。

## 2.00.10

### 故障修复

- 修复了频繁调用 `signal::secc` 等接口时内存泄漏的问题。
- 修复多线程并发访问 `signal::fft`, `signal::ifft`, `signal::secc` 等接口时 Signal 插件宕机的问题。
- 修复当 `signal::fft` 函数的输入数据为偶数个时，输出值的正负符号错误的问题。
