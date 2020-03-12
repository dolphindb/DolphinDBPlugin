# DolphinDB AWS S3插件

AWS S3插件目前仅支持Linux操作系统

### 1) 加载插件
启动DolphinDB实例，执行下述命令: 

```
loadPlugin("path/to/DolphinDBPlugin/aws/PluginAWS.txt");
```

### 2) 设置账户

```
account=dict(string,string);
account['id']=your_access_key_id;
account['key']=your_secret_access_key;
account['region']=your_region;

//注意，若无法连接成功，可以尝试指定证书
account['caPath']=your_ca_file_path;     //例如'/etc/ssl/certs'
account['caFile']=your_ca_file;          //例如'ca-certificates.crt'
account['verifySSL']=verify_or_not;      //例如false
```

### 3) 支持的功能

**listS3Object**

列出S3中指定路径下的所有对象及相关属性

参数

* s3account: 参考前述的设置账户account对象，至少要包含id，key，region

* bucket: 访问的桶名称

* prefix: 访问的路径前缀

返回值

所有匹配的对象的属性表，所列出的属性包括

* index: long, 索引号
* bucket name: string, 桶名
* key name: string, 对象名
* last modified: string, 最近一次修改时间，日期格式为ISO_8601
* length: long, 对象大小，单位为byte
* ETag: string, 标记
* owner: string, 所有者

使用案例

```
aws::listS3Object(account,'mys3bucket','test.csv')
```


**getS3Object** 

获取S3中指定的一个对象

参数

* s3account: 参考前述的设置账户account对象，至少要包含id，key，region
* bucket: 访问的桶名称
* key: 对象名
* outputFileName: 输出对象的文件名，默认为访问的对象名key

返回值

* 本地输出对象的文件名

使用案例

```
aws::getS3Object(account,'mys3bucket','test.csv')
```


**readS3Object**

获取S3中指定对象的一部分

参数

* s3account: 参考前述的设置账户account对象，至少要包含id，key，region
* bucket: 访问的桶名称
* key: 对象名
* offset: 偏移量，想要获取的部分的起始位置，单位是byte
* length: 大小，想要获取的部分的大小，单位是byte

返回值

* 指定对象的一部分构成的字符向量

使用案例

```
aws::readS3Object(account,'mys3bucket','test.csv', 0, 100)
```


**deleteS3Object**

删除S3中的指定对象（警告: 删除操作无法撤销）

参数

* s3account: 参考前述的设置账户account对象，至少要包含id，key，region
* bucket: 访问的桶名称
* key: 对象名

返回值

* 无

使用案例

```
aws::deleteS3Object(account,'mys3bucket','test.csv')
```

**uploadS3Object**

向S3上传一个对象

参数

* s3account: 参考前述的设置账户account对象，至少要包含id，key，region
* bucket: 访问的桶名称
* key: 对象名
* inputFileName: 想要上传的对象的路径及名称

返回值

* 无

使用案例

```
aws::uploadS3Object(account,'mys3bucket','test.csv','/home/test.csv')
```


**listS3Bucket**

列出S3指定账户下的所有桶及创建的时间

参数

* s3account: 参考前述的设置账户account对象，至少要包含id，key，region

返回值

包含所有桶名字和对应创建时间的表，时间的格式是ISO_8601

使用案例

```
aws::listS3Bucket(account);
```

**deleteS3Bucket**

删除S3中指定的桶（警告: 删除操作无法撤销）

参数

* s3account: 参考前述的设置账户account对象，至少要包含id，key，region
* bucket: 删除的桶名称

返回值

* 无

使用案例

```
aws::deleteS3Bucket(account,'mys3bucket')
```

**createS3Bucket**

创建一个桶

参数

* s3account: 参考前述的设置账户account对象，至少要包含id，key，region
* bucket: 创建的桶名称

返回值

* 无

使用案例

```
aws::createS3Bucket(account,'mys3bucket')
```