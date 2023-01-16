# DolphinDB AWS Plugin

The plugin for Amazon AWS S3. It now only supports Linux OS.

### 1) How to load plugin
start a DolphinDB instance, then execute the following command:

```
loadPlugin("path/to/DolphinDBPlugin/awss3/awss3.txt");
```

### 2)set up account

```
account=dict(string,string);
account['id']=your_access_key_id;
account['key']=your_secret_access_key;
account['region']=your_region;

//if your s3 bucket cannot beconnected successfully，you may try to set up your certificate manually as follows:
account['caPath']=your_ca_file_path;     //e.g. '/etc/ssl/certs'
account['caFile']=your_ca_file;          //e.g. 'ca-certificates.crt'
account['verifySSL']=verify_or_not;      //e.g. false
```



### 3) Supported functions

**listS3Object**

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

args

* s3account: a DolphinDB dictionary object storing account info including "id" (access key id), "key"(secret access key), and "region"(your aws s3 region).

return

A table which lists the attribute of all buckets under the given s3account.

The attributes listed are as follows:
* index, long
* bucket name, string
* creation date, string, format: ISO_8601

e.g.

```
aws::listS3Bucket(account);
```


**deleteS3Bucket**

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

args

* s3account: a DolphinDB dictionary object storing account info including "id" (access key id), "key"(secret access key), and "region"(your aws s3 region).
* bucket: the name of the bucket you want to create.

return
* no return

e.g.
```
aws::createS3Bucket(account,'mys3bucket')
```

**loadS3Object**

Load S3 objects to a table


Parameters

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

Return

* a table object with 3 columns object (STRING), errorCode (INT), and errorInfo (STRING), which indicates the imported files, error codes (0 means no error) and error messages .

The error codes are explained as follows:
1-Unknown issue.
2-Failed to parse the file and write it to the table.
3-Failed to download the file.
4-Failed to unzip the file.
5-Cannot find the unzipped file.
6-An exception is raised and the error message is printed.
7-Unknown exception is raised.

Example

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

