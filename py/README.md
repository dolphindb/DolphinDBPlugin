# DolphinDB Py Plugin
利用[python C-API](https://docs.python.org/zh-cn/3.7/c-api/index.html)协议，实现DolphinDB内调用python环境中的第三方库。本插件使用了[pybind11](https://github.com/pybind/pybind11) 库。

Py插件目前支持版本：[relsease200](https://github.com/dolphindb/DolphinDBPlugin/blob/release200/py/README.md), [release130](https://github.com/dolphindb/DolphinDBPlugin/blob/release130/py/README.md)。您当前查看的插件版本为release200，请使用DolphinDB 2.00.X版本server。若使用其它版本server，请切换至相应插件分支。

## 1. 准备工作

### 1.1 依赖库 

- libpython3.7m.so（Windows中为python37.dll）

  如果版本不同，在使用cmake构建插件时可用`DPYTHON:STRING`指定对应版本。目前只支持python3.6、3.7和3.8版本。

- libDolphinDB.so（release130分支动态链接openblas.so动态库版本；Windows中为libDolphinDB.dll）
  
  请参考[插件开发教程](https://github.com/dolphindb/Tutorials_CN/blob/master/plugin_development_tutorial.md)进行配置。

- libOpenblas.so（Windows中为libOpenblas.dll）

  如果无法找到，需要自行安装[OpenBLAS](http://www.openblas.net/)。Linux中可直接用以下命令安装：

  ```
  git clone https://github.com/xianyi/OpenBLAS.git
  make
  make install PREFIX=your_installation_directory
  ```
  
  默认安装目录为/opt/OpenBLAS，请将该路径添加到环境变量中。

### 1.2 修改配置文件

需要修改配置文件dolphindb.cfg，添加libpython3.7m.so（python37.dll）动态库绝对路径。 

```
globalDynamicLib=/path_to_libpython3.7m.so/libpython3.7m.so
```

如果python版本不同请修改成对应版本。 例如：

```
globalDynamicLib=/DolphinDB/server/plugins/py/libpython3.6m.so.1.0
```


**注意**：开发过程中需要有完整的python运行环境，且插件运行时sys.path打印的路径要和安装的运行环境一致。

## 2. 安装

### 2.1 编译安装

#### Linux

##### 使用cmake构建：

安装cmake：
```
sudo apt-get install cmake
```

构建插件内容：
```
mkdir build
cd build
cmake -DPYTHON:STRING=3.7 ../
make
```

**注意**：编译之前请确保libDolphinDB.so在gcc可搜索的路径中。可使用 LD_LIBRARY_PATH 指定其路径，或者直接将其拷贝到build目录下。不同python版本可用-DPYTHON:STRING指定，目前只支持python3.6、3.7和3.8版本。

编译后目录下会产生libPluginPy.so文件。

#### Windows

##### 在Windows环境中需要使用CMake和MinGW编译

- 下载安装[MinGW](http://www.mingw.org/)。确保将bin目录添加到系统环境变量Path中。
- 下载安装[cmake](https://cmake.org/)。

##### 使用cmake构建：

在编译开始之前，要将libDolphinDB.dll拷贝到build文件夹。不同python版本可用-DPYTHON:STRING指定，目前只支持python3.6、3.7和3.8版本。

构建插件内容：

```
mkdir build                                                        # 新建build目录
cp path_to_libDolphinDB.dll/libDolphinDB.dll build                 # 拷贝 libDolphinDB.dll 到build目录下
cd build
cmake -DPYTHON:STRING=3.7 ../ -G "MinGW Makefiles"
mingw32-make -j4
```

编译后目录下会产生libPluginPy.dll文件。

### 2.2 加载插件

```
loadPlugin("/path_to_PluginPy/PluginPy.txt");
```

**注意**：由于数据类型转换时需要用到numpy和pandas中的数据类型，python环境中需要安装numpy和pandas模块。如果不安装numpy和pandas模块，加载的时候将会导致系统崩溃（常见出错信息如下）。 Windows环境下请保证编译时的python版本与本地的python版本一致，否则加载插件时会出错。此外如果使用的是Anaconda，因为Anaconda自带的libstdc++.so.6动态库版本较高，而DolphinDB为了保证兼容性使用的是较低版本的libstdc++.so.6，两者冲突会导致加载插件失败，所以需要使用 pip uninstall pandas 卸载原来的pandas，然后使用 pip install pandas 重新安装pandas，不能使用 conda install 进行安装，否则会链接高版本的libstdc++.so.6。同理插件中要使用到的有链接libstdc++.so.6动态库的模块都需要使用 pip uninstall 和 pip intall 重新安装。除了上述重新安装的方法，还可用Anaconda里lib目录下的libstdc++.so.6替换DolphinDB目录下的libstdc++.so.6（为了防止意外，请先将原文件进行备份），这样就不用重新安装模块了。

```
terminate called after throwing an instance of 'pybind11::error_already_set'
  what():  ModuleNotFoundError: No module named 'numpy'
```

## 3. 插件函数说明

### 3.1 py::toPy

#### 语法

py::toPy(obj)

#### 参数

- obj: 需要转换的dolphindb数据对象。

#### 详情

将dolphindb数据类型转换为python对象，目前支持的数据类型见[5.1小节](#51-dolphindb数据类型转成python对象)。

#### 例子

```
 x = 1 2 3 4;
 pyArray = py::toPy(x);
 
 y = 2 3 4 5;
 d = dict(x, y);
 pyDict = py::toPy(d);
```

### 3.2 py::fromPy

#### 语法

py::fromPy(obj, [addIndex=false])

#### 参数

- obj: 需要转换的python对象。
- addIndex: 当obj为pandas.DataFrame才需设置，若为true表示将pandas.DataFrame的index作为转换后的table的第一列；若为false表示舍弃pandas.DataFrame的index列，默认为false。

#### 详情

将python对象转换为dolphindb数据类型，目前支持的数据类型见[5.2小节](#52-python对象转成dolphindb对象)。pandas.DataFrame转换成table保留index的例子见[4.7小节](#47-dataframe转换成table保留index)。

#### 例子

```
 x = 1 2 3 4;
 l = py::toPy(x);
 re = py::fromPy(l);
 re;
```

### 3.3 py::importModule

#### 语法

py::importModule(moduleName)

#### 参数

- moduleName: 需要导入的模块名称，类型为字符串标量。

#### 详情

导入python模块（子模块），需要确保环境中已安装对应模块，可用 pip3(pip3.7) list 查看是否安装，若未安装则请使用 pip3 install 命令进行安装。

#### 例子

```
np = py::importModule("numpy"); //导入numpy

linear_model = py::importModule("sklearn.linear_model"); //导入sklearn子模块linear_model
```

注：Windows环境下使用GUI第一次加载模块可能会卡住，如果卡住需要在命令行执行py::importModule，后续就可以在GUI中正常使用了。如果需要导入自己写的模块，需要将该模块文件拷贝到sys.path打印的lib路径下或者dolphindb所在的目录下，具体例子可参考[4.6小节](#46-导入自己写的模块并调用其中的静态方法)。

### 3.4 py::cmd

#### 语法

py::cmd(command)

#### 参数

- command: 要运行的python脚本，类型为字符串标量。

#### 详情

运行python脚本。

#### 例子

```
 sklearn = py::importModule("sklearn"); //导入sklearn
 py::cmd("from sklearn import linear_model"); //从sklearn导入linear_model模块
```

### 3.5 py::getObj

#### 语法

py::getObj(module, objName)

#### 参数

- module: 预先导入的模块，如py::importModule的返回值。
- objName: 目标对象名称，类型为字符串标量。

#### 详情

获取模块（或对象）的子模块（或属性）。获取子模块时要确保子模块已经导入，若没有导入，可用 py::cmd 执行 from ... import ... 语句导入。

#### 例子

```
np = py::importModule("numpy"); //导入numpy
random = py::getObj(np, "random"); //获取numpy子模块random

sklearn = py::importModule("sklearn"); //导入sklearn
py::cmd("from sklearn import linear_model"); //导入sklearn子模块
linear_model = py::getObj(sklearn, "linear_model"); //获取sklearn子模块linear_model
```

注：导入numpy时会自动导入random子模块，所以不用加 py::cmd("from numpy import random") 就可直接执行py::getObject获取子模块；而导入sklearn时不会自动导入子模块，所以要加 py::cmd("from sklearn import linear_model") 导入子模块后才能执行py::getObject获取子模块。若只使用子模块功能可用 linear_model=py::importModule("sklearn.linear_model") 获取子模块更加方便。

### 3.6 py::getFunc

#### 语法

py::getFunc(module, funcName, [convert=true])

#### 参数

- module: 预先导入的模块，如py::importModule和py::getObj的返回值。
- funcName: 需要获取的函数名称，类型为字符串标量。
- convert: 表示在调用该函数后结果是否需要自动转换成dolphindb对应的数据类型，默认为true。

#### 详情

获取python模块内的**静态**方法。返回的函数对象能直接调用，函数参数可直接接受dolphindb数据类型，不需要预先转换。目前函数参数不支持key参数的形式，若设置convert=true，则调用结果会直接返回dolphindb数据类型（在能够转换的情况），若设置convert=true，则调用结果不进行转换为python对象。

#### 例子

```
np = py::importModule("numpy"); //导入numpy
eye = py::getFunc(np, "eye"); //获取numpy中的eye函数

np = py::importModule("numpy"); //导入numpy
random = py::getObj(np, "random"); //获取numpy子模块random
randint = py::getFunc(random, "randint"); //获取random中的randint函数
```

### 3.7 py::getInstanceFromObj

#### 语法

py::getInstanceFromObj(obj, [args])

#### 参数

- obj: 预先获得的python类对象，如py::getObj的返回值。
- args: 要传给实例对象的参数，若没有则不填。

#### 详情见

通过预先获得的python类对象获取python类实例对象。返回的对象支持以"``` . ```"方式访问类属性与类方法。会直接返回dolphindb数据类型（在能够转换的情况）或 python对象（不能转换）。

#### 例子

```
 sklearn = py::importModule("sklearn");
 py::cmd("from sklearn import linear_model");
 linearR = py::getObj(sklearn,"linear_model.LinearRegression")
 linearInst = py::getInstanceFromObj(linearR);
```

### 3.8 py::getInstance

#### 语法

py::getInstance(module, objName, [args])

#### 参数

- module: 预先导入的模块，如py::importModule的返回值。
- objName: 目标对象名称，类型为字符串标量。
- args: 要传给实例对象的参数，若没有则不填。

#### 详情

直接从模块中获取python类实例对象。返回的对象支持以"``` . ```"方式访问类属性与类方法。会直接返回dolphindb数据类型（在能够转换的情况）或 python对象（不能转换）。

#### 例子

```
linear_model = py::importModule("sklearn.linear_model"); //导入sklearn子模块linear_model
linearInst = py::getInstance(linear_model,"LinearRegression") 
```

**注意**：py::getFunc获取的是模块中的静态方法。如果要调用实例方法，需要用py::getInstanceFromObj或py::getInstance获取类实例对象，然后用"``` . ```"方式访问类方法。

### 3.9 py::reloadModule

py::reloadModule(module)

#### 参数

- module: 预先导入的模块，如py::importModule的返回值。

#### 详情

如果修改了之前导入的模块，重新执行importModule并不能导入修改后的模块，需要调用reloadModule重新导入该模块才能获取到修改后的模块。

#### 例子

```
model = py::importModule("fibo"); //fibo为4.6小节自己实现的模块

model = py::reloadModule(model);  //如果修改了fibo.py需要调用reloadModule重新导入该模块才能获取到修改后的模块
```

## 4. 实例

### 4.1 加载插件并初始化

   ```
   loadPlugin("/path/to/plugin/PluginPy.txt");
   use py;
   ```

### 4.2 数据结构互转

   ```
   x = 1 2 3 4;
   y = 2 3 4 5;
   d = dict(x, y);
   pyDict = py::toPy(d);
   Dict = py::fromPy(pyDict);
   Dict;
   ```

### 4.3 调用系统库打印python默认路径

   ```
   sys = py::importModule("sys");
   path = py::getObj(sys, "path");
   dpath = py::fromPy(path);
   dpath;
   ```

### 4.4 导入numpy并执行静态方法

   ```
   np = py::importModule("numpy"); //导入numpy
   eye = py::getFunc(np, "eye"); //获取numpy中的eye函数
   re = eye(3); //执行eye函数生成对角矩阵
   re;
   
   random = py::getObj(np, "random"); //获取numpy子模块random
   randint = py::getFunc(random, "randint"); //获取random中的randint函数见
   re = randint(0,1000,[2,3]); //执行randint函数
   re;
   ```

### 4.5 导入sklearn并执行实例方法

   ```
   //方法一
   linear_model = py::importModule("sklearn.linear_model"); //导入sklearn子模块linear_model
   linearInst = py::getInstance(linear_model,"LinearRegression") 
   //方法二
   sklearn = py::importModule("sklearn"); //导入sklearn
   py::cmd("from sklearn import linear_model"); //从sklearn导入linear_model模块
   linearR = py::getObj(sklearn,"linear_model.LinearRegression")
   linearInst = py::getInstanceFromObj(linearR);
   
   X = [[0,0],[1,1],[2,2]];
   Y = [0,1,2];
   linearInst.fit(X, Y); //调用fit函数
   linearInst.coef_; // output: [0.5,0.5]
   linearInst.intercept_; // output: 1.110223E-16 ~ 0
   Test = [[3,4],[5,6],[7,8]];
   re = linearInst.predict(Test); //调用predict函数
   re; //output: [3.5, 5.5, 7.5]
   
   datasets = py::importModule("sklearn.datasets");
   load_iris = py::getFunc(datasets, "load_iris"); //获取静态函数load_iris
   iris = load_iris(); //调用静态函数load_iris
   
   datasets = py::importModule("sklearn.datasets");
   decomposition = py::importModule("sklearn.decomposition");
   PCA = py::getInstance(decomposition, "PCA");
   py_pca=PCA.fit_transform(iris['data'].row(0:3)); //取iris['data']前三行数据进行训练
   py_pca.row(0);  //output:[0.334781147691283, -0.011991887788418, 2.926917846106032e-17]
   ```

 **注意**：DolphinDB中若要从矩阵中取行数据要用`row`函数，如上例中的 iris['data'].row(0:3) 为取前三行数据。iris['data'][0:3] 为取前三列数据。

### 4.6 导入自己写的模块并调用其中的静态方法

本例中我们自己实现了一个如下所示的python模块，里面有两个静态方法，fib(n)打印从0到n的Fibonacci数列，fib2(n)返回从0到n的Fibonacci数列。我们将该模块保存为fibo.py，并将其拷贝到dolphindb所在的目录下（或者拷贝到sys.path打印的lib路径下）：

```python
def fib(n):    # write Fibonacci series up to n
    a, b = 0, 1
    while a < n:
        print(a, end=' ')
        a, b = b, a+b
    print()

def fib2(n):   # return Fibonacci series up to n
    result = []
    a, b = 0, 1
    while a < n:
        result.append(a)
        a, b = b, a+b
    return result
```

之后我们便能加载插件，在dolphindb中导入该模块进行使用：

```
loadPlugin("/path/to/plugin/PluginPy.txt"); //加载插件

fibo = py::importModule("fibo");  //导入该模块
fib = py::getFunc(fibo,"fib");  //获取模块中的fib函数
fib(10);  //调用fib函数，打印出0 1 1 2 3 5 8
fib2 = py::getFunc(fibo,"fib2"); //获取模块中的fib2函数
re = fib2(10);  //调用fib2函数
re;   //output: 0 1 1 2 3 5 8
```

### 4.7 DataFrame转换成table保留index

在调用某些python函数时如果返回DataFrame，若要保留index作为转换后table的第一列，需要在调用getFunc时指定convert=false，然后手动调用fromPy函数将结果转换成table，需要设置addIndex=true。首先实现一个函数返回一个pandas.DataFrame，将其保存成demo.py，并将其拷贝到dolphindb所在的目录下（或者拷贝到sys.path打印的lib路径下）：

```python
import pandas as pd
import numpy as np
def createDF():
    index=pd.Index(['a','b','c'])
    df=pd.DataFrame(np.random.randint(1,10,(3,3)),index=index)
    return df
```

之后加载插件，导入模块加载函数并调用，这样返回的结果中第一列便为dataframe的index：

```
loadPlugin("/path/to/plugin/PluginPy.txt"); //加载插件

model = py::importModule("demo");
func1 = py::getFunc(model, "createDF", false)
tem = func1()
re =  py::fromPy(tem, true)
```

## 5. 支持的数据类型

### 5.1 DolphinDB数据类型转成Python对象

| DolphinDB数据类型 | Python数据类型   |
| ----------------- | ---------------- |
| BOOL              | bool             |
| CHAR              | int64            |
| SHORT             | int64            |
| INT               | int64            |
| LONG              | int64            |
| DOUBLE            | float64          |
| FLOAT             | float64          |
| STRING            | String           |
| DATE              | datetime64[D]    |
| MONTH             | datetime64[M]    |
| TIME              | datetime64[ms]   |
| MINUTE            | datetime64[m]    |
| SECOND            | datetime64[s]    |
| DATETIME          | datetime64[s]    |
| TIMESTAMP         | datetime64[ms]   |
| NANOTIME          | datetime64[ns]   |
| NANOTIMESTAMP     | datetime64[ns]   |
| DATEHOUR          | datetime64[s]    |
| vector            | NumPy.array      |
| matrix            | NumPy.array      |
| set               | Set              |
| dictionary        | Dictionary       |
| table             | pandas.DataFrame |

- DolphinDB CHAR类型会被转换成Python int64类型。
- vector和matrix都会转成numpy.array，时间类型都会转成python pandas中的时间类型，所以python环境中需要安装numpy和pandas模块。
- 由于python pandas中所有有关时间的数据类型均为datetime64[ns]，DolphinDB中table的所有时间类型数据均会被转换为datetime64[ns]类型。MONTH类型，如2012.06M，会被转换为2012-06-01（即月份当月的第一天），由于pandas时间戳范围限制，MONTH范围要在1970.01M-2262.04M之间，DATE和DATETIME日期范围要在1677.09.22-2062.04.11之间。TIME, MINUTE, SECOND与NANOTIME类型不包含日期信息，转换时会自动添加1970-01-01，例如13:30m会被转换为1970-01-01 13:30:00。
- DolphinDB中的逻辑型、数值型和时序类型的NULL值默认情况下会转换成NaN或NaT ，字符串的NULL值为空字符串。如果Vector中有NULL值，数据类型可能会发生改变，比如bool类型的Vector如果包含NULL值，NULL会转换成NaN，因此数据类型变成float64，True会变成1，False会变成0。

### 5.2 Python对象转成DolphinDB对象

| Python数据类型 | DolphinDB数据类型 |
| -------------- | ----------------- |
| bool           | BOOL              |
| int8           | CHAR |
| int16          | SHORT |
| int32            | INT |
| int64           | LONG |
| float32         | FLOAT |
| float64          | DOUBLE |
| String         | STRING |
| datetime64[M] | MONTH |
| datetime64[D] | DATE |
| datetime64[m] | MINUTE |
| datetime64[s] | DATETIME |
| datetime64[h] | DATEHOUR |
| datetime64[ms] | TIMESTAMP |
| datetime64[us] | NANOTIMESTAMP |
| datetime64[ns] | NANOTIMESTAMP |
| Tuple | vector |
| List | vector |
| Dictionary | dictionary |
| Set       | set |
| NumPy.array | vector(1维) / matrix(2维) |
| pandas.DataFrame | table |

- numpy.array会根据维度转换成vector（1维）或者matrix（2维）。

- pandas.DataFrame中的时间数据类型都是datetime64[ns]，所以在转换成table时，时间类型都会转换成NANOTIMESTAMP类型。

- 从pandas.DataFrame转换成table时，如果列名不符合dolphindb支持的列名，则会根据以下规则自动调整列名：

  - 若数据中列名存在中文或英文字母、数字或下划线之外的字符，将其转换为下划线。
  - 若数据中列名第一个字符不是中文或英文字母，添加”c”作为该列名首字符。

  
