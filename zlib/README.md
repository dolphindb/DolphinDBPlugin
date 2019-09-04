# DolphinDB Zlib Plugin

DolphinDB的zlib插件，支持文件到文件的zlib压缩与解压缩

# Build

注意首先需要[构建zlib](https://zlib.net/)，插件静态链接libz.a，在编译libz.a时编译参数需要加上`-fPIC`

```
cd zlib
g++ -DLINUX -std=c++11 -fPIC -c src/ZlibImpl.cpp -I../include -o ZlibImpl.o
g++ -fPIC -shared -o libPluginZlib.so ZlibImpl.o -Wl,-Bstatic -L/path/to/libz.a -lz -Wl,-Bdynamic -lDolphinDB
```

也可以使用CMake编译

```
cd zlib
cmake ..
make
```

编译之后目录下会产生libPluginZlib.so文件

# Interface

```
//loadPlugin
loadPlugin("path/to/DolphinDBPlugin/zlib/PluginZlib.txt");
```

目前支持的函数

- `compressFile`

    - args

        - `inputFileName`, string
        - `level`(optional), range[-1, 9], default is -1(currently equivalent to level 6), 1 gives best speed, 9 gives best compression, 0 gives no compression

    - return

        - inputFileName, add a suffix `.gz`.

    - e.g.
      
      ```
      //loadPlugin
      zlib::compressFile("/home/jccai/data.txt");
      //会将/home/jccai/data.txt压缩为/home/jccai/data.txt.gz
      //注意若输出文件有同名文件，则会被覆盖
      ```
      

- `decompressFile`
  
    - args

        - `inputFileName`, string, should end up with `.gz`

    - return

        - inputFileName, remove the suffix `.gz`.

    - e.g.
      
      ```
      //loadPlugin
      zlib::decompressFile("/home/jccai/data.txt.gz");
      //会将/home/jccai/data.txt.gz解压为/home/jccai/data.txt
      //注意若输出文件有同名文件，则会被覆盖
      ```

- `createZlibInputStream`内部调用，不开放给客户

    - args

        - `inputFileName`, string, should end up with `.gz`

    - return

        - DataInputStreamSP

            - `internalStreamRead`
            - `internalClose`
            - `internalMoveToPosition` is not supported
        
    - e.g.

# Exceptions

If arguments are wrong, throw `IllegalArgumentException`.

If any other errors occur, throw `IOException`.
