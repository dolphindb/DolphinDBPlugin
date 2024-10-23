# DolphinDB mat Plugin

The DolphinDB mat plugin has the branches [release200](https://github.com/dolphindb/DolphinDBPlugin/blob/release200/mat/README_EN.md) and [release130](https://github.com/dolphindb/DolphinDBPlugin/blob/release130/mat/README_EN.md). Each plugin version corresponds to a DolphinDB server version. You're looking at the plugin documentation for release200. If you use a different DolphinDB server version, please refer to the corresponding branch of the plugin documentation.

The DolphinDB mat plugin can be used to read data from a .mat file to DolphinDB or the other way around. The data types are automatically converted during the writing.

- [DolphinDB mat Plugin](#dolphindb-mat-plugin)
  - [1. Install the Plugin](#1-install-the-plugin)
    - [1.1 Download Precompiled Binaries](#11-download-precompiled-binaries)
    - [1.2 Build a Plugin](#12-build-a-plugin)
  - [2. Methods](#2-methods)
    - [2.1 mat::extractMatSchema](#21-matextractmatschema)
    - [2.2 mat::loadMat](#22-matloadmat)
    - [2.3 mat::convertToDatetime](#23-matconverttodatetime)
    - [2.4 mat::writeMat](#24-matwritemat)
  - [3. Data Type Conversion](#3-data-type-conversion)
    - [3.1 INT Type](#31-int-type)
    - [3.2 Floating-point](#32-floating-point)
    - [3.3 String](#33-string)

## 1. Install the Plugin

### 1.1 Download Precompiled Binaries

- Prerequisites

Download the MATLAB Runtime for your operating system: [Mathworks website](http://www.mathworks.com/products/compiler/mcr/).

Download and install MATLAB Compiler Runtime R2016a:

```
wget https://ssd.mathworks.com/supportfiles/downloads/R2016a/deployment_files/R2016a/installers/glnxa64/MCR_R2016a_glnxa64_installer.zip
unzip MCR_R2016a_glnxa64_installer.zip -d matlabFile
cd matlabFile
./install -mode silent -agreeToLicense  yes  -destinationFolder  /home/Matlab
```

Please make sure the file *libDolphinDB.so* is under the GCC search path before compilation. You can configure with the following command:

```
cd <DolphinDBServerDir>
export LD_LIBRARY_PATH=/home/Matlab/v901/bin/glnxa64:$LD_LIBRARY_PATH
```

Then load the plugin in DolphinDB:

```
./dolphindb
login(`admin,`123456)
loadPlugin("<PluginDir>/mat/build/PluginMat.txt");
```

### 1.2 Build a Plugin

You can compile the mat plugin with CMake and G++ on Linux.

#### Compile on Linux with CMake:

Install CMake:

```
sudo apt-get install cmake
```

Build a project:

```
cd <PluginDir>/mat
mkdir build
cd build
cmake -DmatlabRoot=/home/Matlab/v901/ ../   
make
```

**Note:** Please make sure the file *libDolphinDB.so* is under the GCC search path before compilation. You can add the plugin path to the library search path `LD_LIBRARY_PATH` or copy it to the build directory.

After successful compilation, libPluginMat.so and PluginMat.txt are generated under the directory.

## 2. Methods

### 2.1 mat::extractMatSchema

**Syntax**

mat::extractMatSchema(file)

**Parameters**

- file: A STRING scalar, indicating the absolute path where the .mat file is located.  

**Details**

Generate the schema of the data set in the .mat file. It contains 2 columns: column name and data type.

**Example**

```
schema=mat::extractMatSchema("<FileDir>/simple.mat");
```

### 2.2 mat::loadMat

**Syntax**

mat::loadMat(file, [schema])

**Parameters**

- file: A STRING scalar, indicating the absolute path where the .mat file is located.  
- schema: a table containing the names and data types of columns. You can modify the data type as appropriate.

**Details**

Return a dictionary. The key is a string of variable names; The value is the corresponding matrix or vector. When converting a character array, it returns a STRING vector.

**Examples**

```
schema=mat::extractMatSchema("<FileDir>/simple.mat");
ret=mat::loadMat("<FileDir>/simple.mat",schema);
```

### 2.3 mat::convertToDatetime

**Syntax**

mat::convertToDatetime(data)

**Parameters**

- data: The variable to be converted. It can be a scalar, vector, or matrix of double type.

**Details**

Convert the temporal variables in the .mat file to DolphinDB DATETIME values.

**Examples**

```
schema=mat::extractMatSchema("<FileDir>/simple.mat");
ret=loadMat("<FileDir>/simple.mat",schema);
//t_num in simple.nat is a temporal variable of double type
ret=mat::convertToDatetime(ret[`t_num]);
```

### 2.4 mat::writeMat

**Syntax**

mat::writeMat(file, varName, data)

**Parameters**

- file: a STRING scalar indicating the file name
- varName: a STRING scalar indicating the corresponding variable name after being written. 
- data: a matrix to which the data is written. It is of bool, char, short, int, long, float, or double type.

**Details**

Write a matrix to a .mat file.

**Examples**

```
data = matrix([1, 1, 1], [2, 2, 2]).float()
mat::writeMat("var.mat", "var1", data)
```

 

## 3. Data Type Conversion

### 3.1 INT Type

| MATLAB type | DolphinDB type |
| :---------- | :------------- |
| int8        | CHAR           |
| uint8       | SHORT          |
| int16       | SHORT          |
| uint16      | INT            |
| int32       | INT            |
| uint32      | LONG           |
| int64       | LONG           |
| uint64      | not supported  |

- All data types in DolphinDB are signed types. To avoid data type overflow, unsigned types are converted to higher-level signed types. For example, unsigned CHAR is converted to signed SHORT, unsigned SHORT is converted to signed INT, etc. 64-bit unsigned types cannot be converted.
- Unsigned long long types are not supported in DolphinDB. If the data type in a .mat file is bigint unsigned, you can specify *schema* of method `loadMat` as DOUBLE or FLOAT.
- The smallest value of integral types in DolphinDB is NULL, including -128 (CHAR), -32,768 (SHORT), -2,147,483,648 (INT) and -9,223,372,036,854,775,808 (LONG).
- NaN and Inf values in MATLAB are converted to DolphinDB NULL values.

### 3.2 Floating-point

| MATLAB type | DolphinDB type |
| :---------- | :------------- |
| single      | FLOAT          |
| double      | DOUBLE         |

### 3.3 String

| MATLAB type     | DolphinDB type |
| :-------------- | :------------- |
| character array | STRING         |

 

