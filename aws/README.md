# DolphinDB AWS Plugin

DolphinDB的AWS插件，目前支持S3服务，需要链接AWS的动态库

# Build

注意首先需要[构建aws sdk](https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/setup.html)以及构建Zlib，Zlib自身构建时需要-fPIC

使用CMake，默认zlib/aws/dolphindb库文件都在/usr/local/lib下，可以在CMakeLists.txt指定库文件和头文件的地址
```
cd aws/s3
cmake .
make
```
或直接
```
cd aws/s3
g++ -DLINUX -std=c++11 -fPIC -c src/AWSS3.cpp -I../../include -o AWSS3.o
g++ -fPIC -shared -o libPluginAWSS3.so AWSS3.o -Wl,-Bstatic -lz -Wl,-Bdynamic -lDolphinDB -laws-cpp-sdk-s3 -laws-cpp-sdk-core
```

编译之后目录下会产生libPluginAWSS3.so文件

× Windows平台暂时无法用MingW编译AWS SDK

# Interface

使用AWS插件前需要预先载入并设置好id、key和region，插件的module name为aws

```
//loadPlugin
loadPlugin("path/to/DolphinDBPlugin/aws/s3/PluginAWSS3.txt");
//set account
account=dict(string,string);
account['id']=your_access_key_id;
account['key']=your_secret_access_key;
account['region']=your_region;
//若无法通过验证或SSL出错，可以尝试手动提供证书
account['caPath']=your_ca_file_path;     //e.g. '/etc/ssl/certs'
account['caFile']=your_ca_file;          //e.g. 'ca-certificates.crt'
account['verifySSL']=verify_or_not;      //e.g. false
```

目前支持的函数

- `listS3Object`
  
    - args

        - `s3account`, dictionary: `id`-access key id, `key`-secret access key, `region`-region.
        - `bucket`, string: name of the bucket you want to access.
        - `prefix`, string: prefix of the objects.
    
    - return

        A table which lists the attribute of all objects under the given bucket and the prefix.
        
        The attributes listed are as follows:
        
        - index, long
        - bucket name, string
        - key name, string
        - last modified, string, format: ISO_8601
        - length, long, unit: byte
        - ETag, string
        - owner, string
    - e.g.

        ```
        //loadPlugin
        //set account
        aws::listS3Object(account,'dolphindb','TAQ2');
        ```
    
- `getS3Object`
  
    - args

        - `s3account`, dictionary: `id`-access key id, `key`-secret access key, `region`-region.
        - `bucket`, string: name of the bucket you want to access.
        - `key`, string: name of the object you want to get.
        - `outputFileName`(optional), string, default: `key`
    
    - return

        - File name of the object
        
    - e.g.

        ```
        //loadPlugin
        //set account
        aws::getS3Object(account,'dolphindb','test.csv');
        ```

- `readS3Object`
  
    - args

        - `s3account`, dictionary: `id`-access key id, `key`-secret access key, `region`-region.
        - `bucket`, string: name of the bucket you want to access.
        - `key`, string: name of the object you want to get.
        - `offset`, long, start byte of the object you want to read
        - `length`, long, length of the object from the start byte
    
    - return

        - Vector of char
        
    - e.g.

        ```
        //loadPlugin
        //set account
        aws::readS3Object(account,'dolphindb','test.csv', 0, 100);
        ```

- `deleteS3Object`

    - args

        - `s3account`, dictionary: `id`-access key id, `key`-secret access key, `region`-region.
        - `bucket`, string: name of the bucket you want to access.
        - `key`, string: name of the object you want to delete.
    
    - return

        - void
        
    - e.g.

        ```
        //loadPlugin
        //set account
        aws::deleteS3Object(account,'dolphindb','test.csv');
        ```
    
- `uploadS3Object`

    - args

        - `s3account`, dictionary: `id`-access key id, `key`-secret access key, `region`-region.
        - `bucket`, string: name of the bucket you want to access.
        - `key`, string: name of the object you want to creat.
        - `inputFileName`, string, name of the object you want to upload.
    
    - return

        - void
        
    - e.g.

        ```
        //loadPlugin
        //set account
        aws::uploadS3Object(account,'dolphindb','test.csv','/home/test.csv');
        ```

- `listS3Bucket`

    - args

        - `s3account`, dictionary: `id`-access key id, `key`-secret access key, `region`-region.
    
    - return

        A table which lists the attribute of all buckets under the given s3account.
        
        The attributes listed are as follows:
        
        - index, long
        - bucket name, string
        - creation date, string, format: ISO_8601

    - e.g.

        ```
        //loadPlugin
        //set account
        aws::listS3Bucket(account);
        ```

- `deleteS3Bucket`

    - args

        - `s3account`, dictionary: `id`-access key id, `key`-secret access key, `region`-region.
        - `bucket`, string: name of the bucket you want to delete.
    
    - return

        - void
        
    - e.g.

        ```
        //loadPlugin
        //set account
        aws::deleteS3Bucket(account,'dolphindb');
        ```

- `createS3Bucket`

    - args

        - `s3account`, dictionary: `id`-access key id, `key`-secret access key, `region`-region.
        - `bucket`, string: name of the bucket you want to create.
    
    - return

        - void
        
    - e.g.

        ```
        //loadPlugin
        //set account
        aws::createS3Bucket(account,'dolphindb');
        ```

- `createS3InputStream`内部调用，不开放给客户

    - args

        - `s3account`, dictionary: `id`-access key id, `key`-secret access key, `region`-region.
        - `bucket`, string: name of the bucket you want to access.
        - `key`, string: name of the object you want to get, if `key` ends up with ".gz", then it will be automatically decompressed by zlib
    
    - return

        - DataInputStreamSP, stream is based on http range header

            - `internalStreamRead`
            - `internalClose`
            - `internalMoveToPosition` is not supported
        
    - e.g.

# Exceptions

If arguments are wrong, throw `IllegalArgumentException`.

If any other errors occur, throw `IOException`.
