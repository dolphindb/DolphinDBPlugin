# DolphinDB Py Plugin

The DolphinDB Py plugin has the branches [release 200](https://github.com/dolphindb/DolphinDBPlugin/blob/release200/py/README.md) and [release130](https://github.com/dolphindb/DolphinDBPlugin/blob/release130/py/README.md). Each plugin version corresponds to a DolphinDB server version. You're looking at the plugin documentation for release200. If you use a different DolphinDB server version, please refer to the corresponding branch of the plugin documentation.

  - [1. Prerequisites](#1-prerequisites)
    - [1.1 Dependent Libraries](#11-dependent-libraries)
    - [1.2 Modify configurations](#12-modify-configurations)
  - [2. Install the Plugin](#2-install-the-plugin)
    - [2.1 Compile the Plugin](#21-compile-the-plugin)
    - [2.2 Load the Plugin](#22-load-the-plugin)
  - [3. Methods](#3-methods)
    - [3.1 py::toPy](#31-pytopy)
    - [3.2 py::fromPy](#32-pyfrompy)
    - [3.3 py::importModule](#33-pyimportmodule)
    - [3.4 py::cmd](#34-pycmd)
    - [3.5 py::getObj](#35-pygetobj)
    - [3.6 py::getFunc](#36-pygetfunc)
    - [3.7 py::getInstanceFromObj](#37-pygetinstancefromobj)
    - [3.8 py::getInstance](#38-pygetinstance)
    - [3.9 py::reloadModule](#39-pyreloadmodule)
  - [4. Examples](#4-examples)
    - [4.1 Load Plugin](#41-load-plugin)
    - [4.2 Data Type Conversion to and from Python](#42-data-type-conversion-to-and-from-python)
    - [4.3 Print Default Module Search Path Using Built-In Python Module](#43-print-default-module-search-path-using-built-in-python-module)
    - [4.4 Import NumPy and Call Static Methods](#44-import-numpy-and-call-static-methods)
    - [4.5 Import sklearn and Call Instance Methods](#45-import-sklearn-and-call-instance-methods)
    - [4.6 Import Module and Call Static Methods](#46-import-module-and-call-static-methods)
    - [4.7 Convert DataFrame to Table with Index Retained](#47-convert-dataframe-to-table-with-index-retained)
  - [5. Data Conversion](#5-data-conversion)
    - [5.1 From DolphinDB to Python](#51-from-dolphindb-to-python)
    - [5.2 From Python to DolphinDB](#52-from-python-to-dolphindb)

The DolphinDB Py Plugin is implemented based on [python C-API](https://docs.python.org/3.7/c-api/index.html) protocol, and you can call third-party Python libraries in DolphinDB. The plugin uses [pybind11](https://github.com/pybind/pybind11) library.

## 1. Prerequisites

### 1.1 Dependent Libraries

- libpython3.7m.so

You can specify the Python version using `DPYTHON:STRING` when building the plugin with cmake. Currently only python3.6, 3.7, and 3.8 are supported.

- libDolphinDB.so

### 1.2 Modify configurations

Please specify the absolute path of libpython3.7m.so for the configuration parameter *globalDynamicLib* in the configuration file dolphindb.cfg:

```
globalDynamicLib=/path_to_libpython3.7m.so/libpython3.7m.so
```

If you are using a different Python version, please specify it as:

```
globalDynamicLib=/DolphinDB/server/plugins/py/libpython3.6m.so.1.0
```

Note: The development requires the Python runtime environment. The runtime environment (which can be checked with `sys.path`) should be the same as the installation environment.

## 2. Install the Plugin

### 2.1 Compile the Plugin

#### Linux (with CMake)

Install CMake:

```
sudo apt-get install cmake
```

Build the project:

```
mkdir build
cd build
cmake -DPYTHON:STRING=3.7 ../
make
```

**Note:** Please make sure the file *libDolphinDB.so* is under the GCC search path before compilation. You can add the plugin path to the library search path `LD_LIBRARY_PATH` or copy it to the build directory. You can specify the Python version using `-DPYTHON:STRING`. Currently only python3.6, 3.7, and 3.8 are supported.

After successful compilation, the file *libPluginPy.so* is generated under the directory.

### 2.2 Load the Plugin

```
loadPlugin("/path_to_PluginPy/PluginPy.txt");
```

Note: Please install Python libraries NumPy and pandas before loading the plugin. Otherwise, it will raise an error:

```
terminate called after throwing an instance of 'pybind11::error_already_set'
  what():  ModuleNotFoundError: No module named 'numpy'
```

## 3. Methods

### 3.1 py::toPy

**Syntax**

py::toPy(obj)

**Arguments**

- obj: a DolphinDB object

**Details**

Convert an object of DolphinDB data type to a Python object. For the data type conversion, see [5.1 From DolphinDB to Python](#51-from-dolphindb-to-python).

**Examples**

```
 x = 1 2 3 4;
 pyArray = py::toPy(x);
 
 y = 2 3 4 5;
 d = dict(x, y);
 pyDict = py::toPy(d);
``` 

### 3.2 py::fromPy

**Syntax**

py::fromPy(obj, [addIndex=false])

**Arguments**

- obj: a Python object
- addIndex: this parameter is required only when *obj* is a pandas.DataFrame. True: the index of *obj* will become the first column of the result table after the conversion. False (default): the index of *obj* will be discarded in the conversion.

**Details**

Convert a Python object into a DolphinDB object. For supported data types, see [5.2 From Python to DolphinDB](#52-from-python-to-dolphindb). For an example of converting a pandas.DataFrame into a DolphinDB table while keeping the index, see [4.7 Convert DataFrame to Table with Index Retained](#47-convert-dataframe-to-table-with-index-retained).

**Examples**

```
 x = 1 2 3 4;
 l = py::toPy(x);
 re = py::fromPy(l);
 re;
```

### 3.3 py::importModule

**Syntax**

py::importModule(moduleName)

**Arguments**

- moduleName: a STRING indicating the name of the module to be imported.

**Details**

Import a Python module (or a submodule). The module must have been installed in your environment. Use the `pip3 list` command to check all the installed modules and use the `pip3 install` command to install modules.

**Examples**

```
np = py::importModule("numpy"); //import numpy

linear_model = py::importModule("sklearn.linear_model"); //import the linear_model submodule of sklearn 
```

**Note:** To import your own Python module, place the module file under the same directory as the DolphinDB server or under the "lib" directory (which you can check by calling the `sys.path` function in Python). For examples, see [4.6 Import Module and Call Static Methods](#46-import-module-and-call-static-methods).

### 3.4 py::cmd

**Syntax**

py::cmd(command)

**Arguments**

- command: a STRING indicating a python script.

**Details**

Run a Python script in DolphinDB.

**Examples**

```
 sklearn = py::importModule("sklearn"); // import sklearn
 py::cmd("from sklearn import linear_model"); // import the linear_model submodule from sklearn 
```

### 3.5 py::getObj

**Syntax**

py::getObj(module, objName)

**Arguments**

- module: a module which is already imported to DolphinDB, e.g., the return of `py::importModule`.
- objName: a STRING indicating the name of the object you want to get.

**Details**

Get all imported submodules of a module or get the attributes of an object. If a submodule is not imported to DolphinDB yet, call `py::cmd` to import it from the module.

**Examples**

```
np = py::importModule("numpy"); //import numpy
random = py::getObj(np, "random"); ////get the random submodule of numpy

sklearn = py::importModule("sklearn"); //import sklearn
py::cmd("from sklearn import linear_model"); //import a submodule from sklearn
linear_model = py::getObj(sklearn, "linear_model"); //get the imported submodule
```

Note: 

- The "random" submodule is automatically imported when numpy is imported, so you can directly get the "random" submodule with `py::getObject`. 
- When sklearn is imported, its submodules are not imported. Therefore, to get the submodule of sklearn, you must first import the submodule through `py::cmd("from sklearn import linear_model")` before calling the `py::getObject` method. If you only want the submodule, it's more convenient to simply use the `linear_model=py::importModule("sklearn.linear_model")` statement.

### 3.6 py::getFunc

**Syntax**

py::getFunc(module, funcName, [convert=true])

**Arguments**

- module: the Python module imported previously. For example, the return value of method `py::importModule` or `py::getObj`.
- funcName: a STRING scalar indicating the function to be obtained
- convert: whether to convert the results of the function into DolphinDB data types automatically. The default is true.

**Details**

Return a static method from a Python module. The function object can be executed directly in DolphinDB and its arguments can be of DolphinDB data types. Currently, keyword arguments are not supported.

**Examples**

```
np = py::importModule("numpy"); //import numpy
eye = py::getFunc(np, "eye"); //get function eye

np = py::importModule("numpy"); //import numpy
random = py::getObj(np, "random"); //get submodule random
randint = py::getFunc(random, "randint"); //get function randint
```

### 3.7 py::getInstanceFromObj

**Syntax**

py::getInstanceFromObj(obj, [args])

**Arguments**

- obj: the Python object obtained previously. For example, the return value of method `py::getObj`.
- args: the arguments to be passed to the instance. It is an optional argument.

**Details**

Construct an instance based on the Python object obtained previously. You can access the attributes and methods of the instance with "`.`", which returns value of DolphinDB data type if it is convertible, otherwise returns a Python object.

**Examples**

```
 sklearn = py::importModule("sklearn");
 py::cmd("from sklearn import linear_model");
 linearR = py::getObj(sklearn,"linear_model.LinearRegression")
 linearInst = py::getInstanceFromObj(linearR);
```

### 3.8 py::getInstance

#### Syntax

py::getInstance(module, objName, [args])

**Arguments**

- module: the Python module imported previously. For example, the return value of method `py::importModule` or `py::getObj`.
- objName: a STRING scalar indicating the target object.
- args: the arguments to be passed to the instance. It is an optional argument.

**Details**

Get an instance from a Python module. You can access the attributes and methods of the instance with ".", which returns value of DolphinDB data type if it is convertible, otherwise returns a Python object.

**Examples**

```
linear_model = py::importModule("sklearn.linear_model"); //import submodule linear_model of sklearn
linearInst = py::getInstance(linear_model,"LinearRegression") 
```

**Note:** The method `py::getFunc` obtains the static methods from a module. To call an instance method, please use `py::getInstanceFromObj` or `py::getInstance` to obtain the instance and acccess the method with "`.`". 

### 3.9 py::reloadModule

**Syntax**

py::reloadModule(module)

**Arguments**

- module: the Python module imported previously. For example, the return value of method `py::importModule`.

**Details**

If a module is modified after being imported, please execute `py::reloadModule` instead of `py::importModule` to use the modified module.

**Examples**

```
model = py::importModule("fibo"); //fibo is a module implemented in Section 4.6

model = py::reloadModule(model);  //reload the module if fibo.py is modified 
```

## 4. Examples

### 4.1 Load Plugin

```
loadPlugin("/path/to/plugin/PluginPy.txt");
use py;
```

### 4.2 Data Type Conversion to and from Python

```
x = 1 2 3 4;
y = 2 3 4 5;
d = dict(x, y);
pyDict = py::toPy(d);
Dict = py::fromPy(pyDict);
Dict;
```

### 4.3 Print Default Module Search Path Using Built-In Python Module 

```
sys = py::importModule("sys");
path = py::getObj(sys, "path");
dpath = py::fromPy(path);
dpath;
```

### 4.4 Import NumPy and Call Static Methods

```
np = py::importModule("numpy"); //import numpy
eye = py::getFunc(np, "eye"); //get the eye function of numpy
re = eye(3); //create a diagonal matrix using eye
re;

random = py::getObj(np, "random"); //get the randome submodule of numpy
randint = py::getFunc(random, "randint"); //get the randint function of random
re = randint(0,1000,[2,3]); //execute randint
re;
```

### 4.5 Import sklearn and Call Instance Methods

```
//one way to get the LinearRegression method
linear_model = py::importModule("sklearn.linear_model"); //import the linear_model submodule from sklearn
linearInst = py::getInstance(linear_model,"LinearRegression") 
//the other way
sklearn = py::importModule("sklearn"); //import sklearn
py::cmd("from sklearn import linear_model"); //import the linear_model submodule from sklearn
linearR = py::getObj(sklearn,"linear_model.LinearRegression")
linearInst = py::getInstanceFromObj(linearR);

X = [[0,0],[1,1],[2,2]];
Y = [0,1,2];
linearInst.fit(X, Y); //call the fit function
linearInst.coef_; //output:[0.5,0.5]
linearInst.intercept_; // output: 1.110223E-16 ~ 0
Test = [[3,4],[5,6],[7,8]];
re = linearInst.predict(Test); //call the predict function
re; //output: [3.5, 5.5, 7.5]

datasets = py::importModule("sklearn.datasets");
load_iris = py::getFunc(datasets, "load_iris"); //get the static function load_iris
iris = load_iris(); //call load_iris

datasets = py::importModule("sklearn.datasets");
decomposition = py::importModule("sklearn.decomposition");
PCA = py::getInstance(decomposition, "PCA");
py_pca=PCA.fit_transform(iris['data'].row(0:3)); //train the fir three rows of irir['data']
py_pca.row(0);  //output:[0.334781147691283, -0.011991887788418, 2.926917846106032e-17]
```

Note: Use the `row` function to access the rows of a matrix in DolphinDB. As shown in the above example, `iris['data'].row(0:3)` retrieves the first three rows from `iris['data']`. To retrieve the first three columns, use `iris['data'][0:3]`.

### 4.6 Import Module and Call Static Methods

In this case we have implemented the python module with two static methods: `fib(n)` prints the Fibonacci series from 0 to n; `fib2(n)` returns the Fibonacci series from 0 to n.

We save the module as fibo.py and copy it to the directory where DolphinDB server is located (or to the library path printed by `sys.path`).

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

Then load the plugin and use the module after importing it:

```
loadPlugin("/path/to/plugin/PluginPy.txt"); //load the Py plugin

fibo = py::importModule("fibo");  //import the module fibo
fib = py::getFunc(fibo,"fib");  //get the fib function
fib(10);  //call fib function, print 0 1 1 2 3 5 8
fib2 = py::getFunc(fibo,"fib2"); //get the fib2 function in the module
re = fib2(10);  //call the fib2 function
re;   //output: 0 1 1 2 3 5 8
```

### 4.7 Convert DataFrame to Table with Index Retained

When calling Python functions that return a DataFrame, you can set *convert*=false of the `getFunc` function if you want to keep the index as the first column of the converted table. Use the `fromPy` function to convert the result to a table (set *addIndex*=true).

Implement a function returned pandas.DataFrame. Save the result as demo.py and copy it to the directory where DolphinDB server is located (or to the library path printed by `sys.path`).

```python
import pandas as pd
import numpy as np
def createDF():
    index=pd.Index(['a','b','c'])
    df=pd.DataFrame(np.random.randint(1,10,(3,3)),index=index)
    return df
```

Load the plugin and import the module demo. Use the function to keep the index of the DataFrame as the first column of the result.

```
loadPlugin("/path/to/plugin/PluginPy.txt"); //load the Py plugin

model = py::importModule("demo");
func1 = py::getFunc(model, "createDF", false)
tem = func1()
re =  py::fromPy(tem, true)
```

## 5. Data Conversion

### 5.1 From DolphinDB to Python

**Data Form Conversion**

| DolphinDB Form | Python Form      |
| :------------- | :--------------- |
| vector         | NumPy.array      |
| matrix         | NumPy.array      |
| set            | Set              |
| dictionary     | Dictionary       |
| table          | pandas.DataFrame |

**Data Type Conversion**

| DolphinDB Type | Python Type      |
| :------------- | :--------------- |
| BOOL           | bool             |
| CHAR           | int64            |
| SHORT          | int64            |
| INT            | int64            |
| LONG           | int64            |
| DOUBLE         | float64          |
| FLOAT          | float64          |
| STRING         | String           |
| DATE           | datetime64[D]    |
| MONTH          | datetime64[M]    |
| TIME           | datetime64[ms]   |
| MINUTE         | datetime64[m]    |
| SECOND         | datetime64[s]    |
| DATETIME       | datetime64[s]    |
| TIMESTAMP      | datetime64[ms]   |
| NANOTIME       | datetime64[ns]   |
| NANOTIMESTAMP  | datetime64[ns]   |
| DATEHOUR       | datetime64[s]    |
| vector         | NumPy.array      |
| matrix         | NumPy.array      |
| set            | Set              |
| dictionary     | Dictionary       |
| table          | pandas.DataFrame |

- DolphinDB CHAR types are converted into Python int64 type. 
- As the temporal types in Python pandas are datetime64[ns], all DolphinDB temporal types [are converted into datetime64[ns\] type](https://github.com/pandas-dev/pandas/issues/6741#issuecomment-39026803). MONTH type such as 2012.06M is converted into 2012-06-01 (the first day of the month). TIME, MINUTE, SECOND and NANOTIME types do not include information about date. 1970-01-01 is automatically added during conversion. For example, 13:30m is converted into 1970-01-01 13:30:00.
- NULLs of logical, temporal and numeric types are converted into NaN or NaT; NULLs of string types are converted into empty strings. If a vector contains NULL values, the data type may change. For example, if a vector of Boolean type contains NULL values, NULL will be converted to NaN. As a result, the vector's data type will be converted to float64, and the TRUE and False values will be converted to 1 and 0, respectively.

 

### 5.2 From Python to DolphinDB

**Data Form Conversion**

| Python Form      | DolphinDB Form            |
| :--------------- | :------------------------ |
| Tuple            | vector                    |
| List             | vector                    |
| Dictionary       | dictionary                |
| Set              | set                       |
| NumPy.array      | vector(1d) / matrix(2d)   |
| pandas.DataFrame | table                     |

**Data Type Conversion**

| Python Type    | DolphinDB Type |
| :------------- | :------------- |
| bool           | BOOL           |
| int8           | CHAR           |
| int16          | SHORT          |
| int32          | INT            |
| int64          | LONG           |
| float32        | FLOAT          |
| float64        | DOUBLE         |
| String         | STRING         |
| datetime64[M]  | MONTH          |
| datetime64[D]  | DATE           |
| datetime64[m]  | MINUTE         |
| datetime64[s]  | DATETIME       |
| datetime64[h]  | DATEHOUR       |
| datetime64[ms] | TIMESTAMP      |
| datetime64[us] | NANOTIMESTAMP  |
| datetime64[ns] | NANOTIMESTAMP  |

- The numpy.array will be converted to DolphinDB vector (1d) or matrix (2d) based on its dimension.
- As the only temporal data type in Python pandas is datetime64, all temporal columns of a DataFrame are converted into NANOTIMESTAMP type
- When a pandas.DataFrame is converted to DolphinDB table, if the column name is not supported in DolphinDB, it will be adjusted based on the following rules:
  - If special characters except for letters, digits or underscores are contained in the column names, they are converted to underscores.
  - If the first character is not a letter, "c" is added as the first character of the column name.

 
