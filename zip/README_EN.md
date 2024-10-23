This plugin enables you to unzip ZIP files in DolphinDB.

The DolphinDB zip plugin has different branches, such as release200 and release130. Each branch corresponds to a DolphinDB server version. Please make sure you are in the correct branch of the plugin documentation.

- [1. Compile and Install Plugin](#1-compile-and-install-plugin)
  - [1.1 Install Precompiled Plugin](#11-install-precompiled-plugin)
  - [1.2 (Optional) Manually Compile Plugin with CMake](#12-optional-manually-compile-plugin-with-cmake)
  - [1.3 Load Plugin](#13-load-plugin)
- [2. Methods](#2-methods)
  - [zip::unzip](#zipunzip)

## 1. Compile and Install Plugin

### 1.1 Install Precompiled Plugin

**Linux**

Download the precompiled plugin files under the `DolphinDBPlugin/zip/bin/linux64` directory (make sure to switch to the correct branch). Save the files under /DolphinDB/server/plugins/zip in your local system.

### 1.2 (Optional) Manually Compile Plugin with CMake

Install CMake

```
sudo apt install cmake
```

Compile plugin

```
mkdir build
cd build
cp /path_to_dolphindb/libDolphinDB.so ../lib
cmake ..
make -j
```

After compilation, the file *libPluginZip.so* is generated under the build directory.

### 1.3 Load Plugin

Load the plugin in DolphinDB with the following command:

```
loadPlugin("/path_to_pluginZip/PluginZip.txt");
```

## 2. Methods

### zip::unzip

**Syntax**

zip::unzip(zipFileName, outputDir, callback)

**Arguments**

- zipFileName: a string indicating the absolute path to the ZIP file.  
- outputDir: a string indicating the absolute path the file will be extracted to. If it is unspecified or specified as ““, the ZIP file will be extracted to its current directory. Note that existing files with the same name under the directory will be replaced.
- callback (optional): a callback function for handling the unzipped file. It only takes one argument of STRING type. 

**Details**

Extract files from a ZIP file. Return a string vector indicating the file paths to the extracted files. Optionally, specify a callback function to process the extracted files. The callback is executed each time a member file is extracted for optimal performance.

**Examples**

```
filenames = zip::unzip("/path_to_zipFile/test.zip", "/path_to_output/", func);

print(filenames)
["/path_to_output/test.csv"]
```



