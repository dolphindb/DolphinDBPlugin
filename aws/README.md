# DolphinDB AWS Plugin

DolphinDB的AWS插件，目前支持S3服务，需要链接AWS的动态库。

## 1 插件编译与加载

注意首先需要[构建aws sdk](https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/setup.html)以及构建Zlib。Zlib自身构建时需要指定`-fPIC`参数。

通过CMake进行编译，默认zlib/aws/dolphindb库文件都位于/usr/local/lib下。可以在CMakeLists.txt中指定库文件和头文件的地址。
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

编译之后目录下会产生libPluginAWSS3.so文件。

请注意：Windows平台暂时无法通过MingW编译AWS SDK。

## 2 DolphinDB加载插件

使用AWS插件前需要预先载入并设置好id, key和region。插件的module name为aws。

### 2.1 加载插件
启动DolphinDB实例，执行下述命令加载插件：
```
loadPlugin("path/to/DolphinDBPlugin/awss3/PluginAWSS3.txt");
```
### 2.2 设置账户
```
account=dict(string,string);
account['id']=your_access_key_id;
account['key']=your_secret_access_key;
account['region']=your_region;

//注意，若无法通过验证或SSL出错，可以尝试指定证书
account['caPath']=your_ca_file_path;     //e.g. '/etc/ssl/certs'
account['caFile']=your_ca_file;          //e.g. 'ca-certificates.crt'
account['verifySSL']=verify_or_not;      //e.g. false
```
## 3 接口说明
**listS3Object**

列出S3中指定路径下的所有对象及相关属性。

**参数**

* s3account：账户account对象，至少需包含三个值（id, key 和 region）。
* bucket：字符串，表示访问的桶名称。
* prefix：可选参数，一个字符串，表示访问路径的前缀。

**返回值**

所有匹配对象的属性表，包括：

* index：索引号
* bucket name：桶名
* key name：对象名
* last modified：最近一次修改时间，日期格式为ISO_8601
* length：对象大小，单位为byte
* ETag：标记
* owner：所有者

使用案例

```
aws::listS3Object(account,'mys3bucket','test.csv')
```


**getS3Object** 

获取S3中指定的一个对象

**参数**

* s3account：账户account对象，至少需包含三个值（id, key 和 region）。
* bucket：字符串，表示访问的桶名称。
* key：字符串，表示对象名。
* outputFileName：字符串，表示输出对象的文件名。默认同访问的对象名key。

**返回值**

* 本地输出对象的文件名

使用案例

```
aws::getS3Object(account,'mys3bucket','test.csv')
```

**readS3Object**

获取S3中指定对象的部分内容。

**参数**

* s3account：账户account对象，至少需包含三个值（id, key 和 region）。
* bucket：字符串，表示访问的桶名称。
* key：字符串，表示对象名。
* offset: 偏移量，想要获取的内容的起始位置，单位是byte。
* length: 长度，想要获取的内容的长度，单位是byte。

**返回值**

* 返回由对象指定部分的内容构成的字符向量。

使用案例

```
aws::readS3Object(account,'mys3bucket','test.csv', 0, 100)
```


**deleteS3Object**

删除S3中的指定对象（警告: 删除操作无法撤销）。

**参数**

* s3account：账户account对象，至少需包含三个值（id, key 和 region）。
* bucket：字符串，表示访问的桶名称。
* key：字符串，表示对象名。

**返回值**

* 无

使用案例

```
aws::deleteS3Object(account,'mys3bucket','test.csv')
```

**uploadS3Object**

向S3上传一个对象。

参数

* s3account：账户account对象，至少需包含三个值（id, key 和 region）。
* bucket：字符串，表示访问的桶名称。
* key：字符串，表示对象名。
* inputFileName：字符串，表示准备上传的对象的路径及名称。

返回值

* 无

使用案例

```
aws::uploadS3Object(account,'mys3bucket','test.csv','/home/test.csv')
```


**listS3Bucket**

列出S3指定账户下的所有桶及创建的时间。

**参数**

* s3account：账户account对象，至少需包含三个值（id, key 和 region）。

**返回值**

包含所有桶名字和对应创建时间的表，时间的格式是ISO_8601。

使用案例

```
aws::listS3Bucket(account);
```

**deleteS3Bucket**

删除S3中指定的桶（警告：删除操作无法撤销）。

**参数**

* s3account：账户account对象，至少需包含三个值（id, key 和 region）。
* bucket：字符串，表示删除的桶名称。

**返回值**

* 无

使用案例

```
aws::deleteS3Bucket(account,'mys3bucket')
```

**createS3Bucket**

创建一个桶

**参数**

* s3account：账户account对象，至少需包含三个值（id, key 和 region）。
* bucket：字符串，表示创建的桶名称。

**返回值**

* 无

使用案例

```
aws::createS3Bucket(account,'mys3bucket')
```

**loadS3Object**

加载一批对象到表中

**参数**

* s3account：账户account对象，至少需包含三个值（id, key 和 region）。
* bucket：字符串，表示读取的桶名称。
* key：字符串标量或向量，表示读取对象名或对象名的列表。支持文本文件，或Zip格式的压缩对象。
* threadCount：下载线程数，必须为正整数。
* dbHandle：数据库的句柄，可以是内存数据库或分布式数据库。
* tableName：字符串，表示表的名称。
* partitionColumns：字符串标量或向量，表示分区列。对于顺序分区类型的数据库，partitionColumns 为空字符串""。对于组合分区类型的数据库，partitionColumns 是字符串向量。
* delimiter：各列的分隔符。默认是逗号。
* schema：一个表，用于指定各列的数据类型。具体请参考 [loadText](https://www.dolphindb.cn/cn/help/FunctionsandCommands/FunctionReferences/l/loadText.html) 的 schema 参数。
* skipRows：0到1024之间的整数，表示从文件头开始忽略的行数。它是一个可选参数。默认值为0。
* transform：一元函数，并且该函数接受的参数必须是一个表。
* sortColumns：是字符串标量或向量，用于指定表的排序列。同一个排序列对应的数据在分区内部按顺序存放在一起。
* atomic：是一个布尔值，表示开启 Cache Engine 的情况下，是否保证文件加载的原子性。设置为 true，一个文件的加载过程视为一个完整的事务；设置为 false，加载一个文件的过程分为多个事务进行。
注意：如果要加载的文件超过 Cache Engine 大小，必须设置 atomic = false。否则，一个事务可能卡住（既不能提交，也不能回滚）。
* arrayDelimiter：是数据文件中数组向量列的分隔符。默认是逗号。由于不支持自动识别数组向量，必须同步修改 schema 的 type 列修为数组向量类型。

**返回值**

* 表对象，包含object(STRING), errorCode(INT), errorInfo(STRING)三列，描述解压的每一个文件（object）加载的错误码（errorCode，0表示没有错误）和错误信息（errorInfo）。

错误代码（errorCode）如下：  
1-未知问题  
2-解析文件并写入表中失败  
3-下载文件失败  
4-unzip文件失败  
5-查找解压文件失败  
6-抛出异常，有详细信息  
7-抛出未知异常，没有详细信息

使用案例

```
//创建账号
account=dict(string,string);
account['id']='XXXXXXXXXXXXXXX';
account['key']='XXXXXXXXXX';
account['region']='us-east';
//加载S3对象
db = database(directory="dfs://rangedb", partitionType=RANGE, partitionScheme=0 51 101)
aws::loadS3Object(account, 'dolphindb-test-bucket', 't2.zip', 4, db, `pt, `ID);
```