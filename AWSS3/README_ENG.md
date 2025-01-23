# DolphinDB AWS Plugin

The plugin for Amazon AWS S3.

The DolphinDB AWS plugin has the following versions: [release 200](https://github.com/dolphindb/DolphinDBPlugin/blob/release200/parquet/README.md) and [release130](DolphinDBPlugin/README.md at release130 · dolphindb/DolphinDBPlugin ). Each plugin version corresponds to a DolphinDB server version. You're looking at the plugin documentation for release200. If you use a different DolphinDB server version, please refer to the corresponding version of the plugin documentation.

- [DolphinDB AWS Plugin](#dolphindb-aws-plugin)
  - [1. Compile AWS Plugin](#1-compile-aws-plugin)
  - [2. Load plugin](#2-load-plugin)
    - [2.1 How to load plugin](#21-how-to-load-plugin)
    - [2.2 Set up account](#22-set-up-account)
  - [3. Methods](#3-methods)
    - [3.1 listS3Object](#31-lists3object)
    - [3.2 getS3Object](#32-gets3object)
    - [3.3 readS3Object](#33-reads3object)
    - [3.4 deleteS3Object](#34-deletes3object)
    - [3.5 uploadS3Object](#35-uploads3object)
    - [3.6 listS3Bucket](#36-lists3bucket)
    - [3.7 deleteS3Bucket](#37-deletes3bucket)
    - [3.8 createS3Bucket](#38-creates3bucket)
    - [3.9 loadS3Object](#39-loads3object)


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
loadPlugin("path/to/DolphinDBPlugin/awss3/PluginAWSS3.txt");
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

### 3.1 listS3Object

**Parameters**

* s3account: a DolphinDB dictionary object storing account info including "id" (access key id), "key"(secret access key), and "region"(your aws s3 region).

* bucket: the name of the bucket you want to access.

* prefix: the prefix of the buckets' names.

**Details**

Return a DolphinDB table listing the attributes of all objects under the given bucket.

The attributes listed are as follows:
* index, long
* bucket name, string
* key name, string
* last modified, string, format: ISO_8601
length, long, unit: byte
* ETag, string
* owner, string

**Examples**

```
aws::listS3Object(account,'mys3bucket','test.csv')
```

### 3.2 getS3Object

**Parameters**

* s3account: a DolphinDB dictionary object storing account info including "id" (access key id), "key"(secret access key), and "region"(your aws s3 region).

* bucket: the name of the bucket you want to access.

* key: the name of the object you want to get.

* outputFileName(optional): default is the key name

**Details**

Get an s3 object. Return the file name of the object.

**Examples**

```
aws::getS3Object(account,'mys3bucket','test.csv')
```

### 3.3 readS3Object

**Parameters**


* s3account: a DolphinDB dictionary object storing account info including "id" (access key id), "key"(secret access key), and "region"(your aws s3 region).

* bucket: the name of the bucket you want to access.

* key: the name of the object you want to get.

* offset: the start byte position of the object you want to read

* length: the length of the object from the start byte


**Details**

Get part of an s3 object. Return a DolphinDB vector of char storing part of a s3 object.

**Examples**

```
aws::readS3Object(account,'mys3bucket','test.csv', 0, 100)
```

### 3.4 deleteS3Object

**Parameters**

* s3account:a DolphinDB dictionary object storing account info including "id" (access key id), "key"(secret access key), and "region"(your aws s3 region).

* bucket: the name of the bucket you want to access.

* key: the name of the object you want to get.

**Details**

Delete an s3 object (warning: the deletion cannot be undone).

**Examples**

```
aws::deleteS3Object(account,'mys3bucket','test.csv')
//Warning: irreversible operation 
```

### 3.5 uploadS3Object

**Parameters**

* s3account: a DolphinDB dictionary object storing account info including "id" (access key id), "key"(secret access key), and "region"(your aws s3 region).

* bucket: the name of the bucket you want to access.

* key: the name of the object you want to get.

* inputFileName: the name of the object you want to upload

**Details**

Upload an object to s3.

**Examples**

```
aws::uploadS3Object(account,'mys3bucket','test.csv','/home/test.csv')
```

### 3.6 listS3Bucket

**Parameters**

* s3account: a DolphinDB dictionary object storing account info including "id" (access key id), "key"(secret access key), and "region"(your aws s3 region).

**Details**

Return a table which lists all buckets and their creation dates under the given s3account. The format of the date is ISO_8601.

**Examples**

```
aws::listS3Bucket(account);
```

### 3.7 deleteS3Bucket

**Parameters**
* s3account: a DolphinDB dictionary object storing account info including "id" (access key id), "key"(secret access key), and "region"(your aws s3 region).

* bucket: the name of the bucket you want to access.

**Details**

Delete a given bucket (warning: the deletion cannot be undone).

**Examples**

```
aws::deleteS3Bucket(account,'mys3bucket')
//Warning: irreversible operation
```

### 3.8 createS3Bucket

**Parameters**

* s3account: a DolphinDB dictionary object storing account info including "id" (access key id), "key"(secret access key), and "region"(your aws s3 region).

* bucket: the name of the bucket you want to create.

**Details**

Create a bucket.

**Examples**

```
aws::createS3Bucket(account,'mys3bucket')
```

### 3.9 loadS3Object

**Parameters**

* s3account: an S3 account defined before. It must contain id, key and region.

* bucket: the S3 bucket to be loaded.

* key: a scalar or list of objects to be loaded. The object can be a text file or a ZIP file.

* threadCount: a positive integer indicating the number of threads that can be used to load the objects.

* dbHandle: the database where the imported data will be saved. It can be either a DFS database or an in-memory database.

* tableName: a string indicating the name of the table with the imported data.

* partitionColumns: a string scalar/vector indicating the partitioning column(s). For sequential partition, leave it unspecified; For composite partition, partitionColumns is a string vector.

* delimiter: the table column separator. The default value is ','.

* schema: a table. See the parameter schema of function loadText for the supported parameter.

* skipRows: is an integer between 0 and 1024 indicating the rows in the beginning of the text file to be ignored. The default value is 0.

* transform: is a unary function. The parameter of the function must be a table.

* sortColumns: is a string scalar/vector indicating the columns based on which the table is sorted.

* atomic: is a Boolean value indicating whether to guarantee atomicity when loading a file with the cache engine enabled. If it is set to true, the entire loading process of a file is a transaction; set to false to split the loading process into multiple transactions.

* arrayDelimiter: is a single character indicating the delimiter for columns holding the array vectors in the file. Since the array vectors cannot be recognized automatically, you must use the schema parameter to update the data type of the type column with the corresponding array vector data type before import.

**Details**

Load S3 objects to a table. Return a table object with 3 columns object (STRING), errorCode (INT), and errorInfo (STRING), which indicates the imported files, error codes (0 means no error) and error messages .

The error codes are explained as follows: 
1-Unknown issue. 
2-Failed to parse the file and write it to the table. 
3-Failed to download the file. 
4-Failed to unzip the file. 
5-Cannot find the unzipped file. 
6-An exception is raised and the error message is printed. 
7-Unknown exception is raised.

**Examples**

```
//create an account
account=dict(string,string);
account['id']='XXXXXXXXXXXXXXX';
account['key']='XXXXXXXXXX';
account['region']='us-east';
//load S3 objects
db = database(directory="dfs://rangedb", partitionType=RANGE, partitionScheme=0 51 101)
aws::loadS3Object(account,'dolphindb-test-bucket','t2.zip',4,db,`pt, `ID);
```


