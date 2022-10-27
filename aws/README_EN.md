# DolphinDB AWS Plugin

The plugin for Amazon AWS S3.

The DolphinDB AWS plugin has the following versions: [release 200](https://github.com/dolphindb/DolphinDBPlugin/blob/release200/parquet/README.md) and [release130](DolphinDBPlugin/README.md at release130 · dolphindb/DolphinDBPlugin ). Each plugin version corresponds to a DolphinDB server version. You're looking at the plugin documentation for release200. If you use a different DolphinDB server version, please refer to the corresponding version of the plugin documentation.

## 1. Compile AWS Plugin

Construct [AWS SDK](https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/welcome.html) and Zlib first, and specify the `-fPIC` parameter for Zlib construction.

Compile with CMake. The folder `zlib/aws/dolphindb` are located in library `/usr/local/lib` by default. You can specify the directory to the library and header files in CMakeLists.txt.

```
cd aws/s3
cmake .
make
```

Or

```
cd aws/s3
g++ -DLINUX -std=c++11 -fPIC -c src/AWSS3.cpp -I../../include -o AWSS3.o
g++ -fPIC -shared -o libPluginAWSS3.so AWSS3.o -Wl,-Bstatic -lz -Wl,-Bdynamic -lDolphinDB -laws-cpp-sdk-s3 -laws-cpp-sdk-core
```
After compiling, a file named `libPluginAWSS3.so` is generated.

Note: Currently, AWS SDK cannot be compiled through MinGW on Windows.

## 2. Load plugin

Before using the AWS plugin, you need to preload and set the id, key and region. The module name of the plugin is aws.

### 2.1 How to load plugin

Start a DolphinDB server, then execute the following command:

```
loadPlugin("path/to/DolphinDBPlugin/awss3/awss3.txt");
```

### 2.2 Set up account

```
account=dict(string,string);
account['id']=your_access_key_id;
account['key']=your_secret_access_key;
account['region']=your_region;

//if your s3 bucket cannot be connected successfully，you may try to set up your certificate manually as follows:
account['caPath']=your_ca_file_path;     //e.g. '/etc/ssl/certs'
account['caFile']=your_ca_file;          //e.g. 'ca-certificates.crt'
account['verifySSL']=verify_or_not;      //e.g. false
```



## 3. Methods

**listS3Object**

List the attributes of all objects under the given bucket.

args

* s3account: a DolphinDB dictionary object storing account info including "id" (access key id), "key"(secret access key), and "region"(your aws s3 region).

* bucket: the name of the bucket you want to access.

* prefix: the prefix of the buckets' names.


return

* a DolphinDB table listing the attributes of all objects under the given bucket.

The attributes listed are as follows:


* index, long
* bucket name, string
* key name, string
* last modified, string, format: ISO_8601
length, long, unit: byte
* ETag, string
* owner, string



e.g.

```
aws::listS3Object(account,'mys3bucket','test.csv')
```



**getS3Object**

Get an s3 object.

args


* s3account: a DolphinDB dictionary object storing account info including "id" (access key id), "key"(secret access key), and "region"(your aws s3 region).


* bucket: the name of the bucket you want to access.

* key: the name of the object you want to get.

* outputFileName(optional): default is the key name


return

* the file name of the object


e.g.

```
aws::getS3Object(account,'mys3bucket','test.csv')
```




**readS3Object**

Get part of an s3 object.

args


* s3account: a DolphinDB dictionary object storing account info including "id" (access key id), "key"(secret access key), and "region"(your aws s3 region).

* bucket: the name of the bucket you want to access.

* key: the name of the object you want to get.

* offset: the start byte position of the object you want to read

* length: the length of the object from the start byte


return

* a DolphinDB vector of char storing part of a s3 object



e.g.

```
aws::readS3Object(account,'mys3bucket','test.csv', 0, 100)
```


**deleteS3Object**

Delete an s3 object (warning: the deletion cannot be undone).

args

* s3account:a DolphinDB dictionary object storing account info including "id" (access key id), "key"(secret access key), and "region"(your aws s3 region).

* bucket: the name of the bucket you want to access.

* key: the name of the object you want to get.

return

* no return

e.g.

```
aws::deleteS3Object(account,'mys3bucket','test.csv')
//Warning: irreversible operation 
```

**uploadS3Object**

Upload an object to s3.

args

* s3account: a DolphinDB dictionary object storing account info including "id" (access key id), "key"(secret access key), and "region"(your aws s3 region).

* bucket: the name of the bucket you want to access.

* key: the name of the object you want to get.

* inputFileName: the name of the object you want to upload

return

* no return

e.g.

```
aws::uploadS3Object(account,'mys3bucket','test.csv','/home/test.csv')
```

**listS3Bucket**

List all buckets and their creation dates under the given s3 account.

args

* s3account: a DolphinDB dictionary object storing account info including "id" (access key id), "key"(secret access key), and "region"(your aws s3 region).

return

A table which lists all buckets and their creation dates under the given s3account. The format of the date is ISO_8601.

e.g.

```
aws::listS3Bucket(account);
```


**deleteS3Bucket**

Delete a given bucket (warning: the deletion cannot be undone).

args
* s3account: a DolphinDB dictionary object storing account info including "id" (access key id), "key"(secret access key), and "region"(your aws s3 region).
* bucket: the name of the bucket you want to access.

return
* no return

e.g.
```
aws::deleteS3Bucket(account,'mys3bucket')
//Warning: irreversible operation
```

**createS3Bucket**

Create a bucket.

args

* s3account: a DolphinDB dictionary object storing account info including "id" (access key id), "key"(secret access key), and "region"(your aws s3 region).
* bucket: the name of the bucket you want to create.

return
* no return

e.g.
```
aws::createS3Bucket(account,'mys3bucket')
```
