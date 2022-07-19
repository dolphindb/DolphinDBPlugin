# DolphinDB HTTP Client Plugin

使用该插件可以便捷地进行HTTP请求或发送邮件。

## 1. 安装构建

## 1.1 预编译安装

在DolphinDBPlugin/httpClient/bin/linux64目录下有预先编译的插件文件，在DolphinDB中执行以下命令导入httpClient插件：

```
loadPlugin("<PluginDir>/httpClient/bin/linux64/PluginHttpClient.txt");
```
> 本插件使用了libcurl。如果访问https网站时出现"curl return: error setting certificate verify locations:   CAfile: /etc/ssl/certs/ca-certificates.crt   CApath: none"，这是curl默认寻找https根证书的位置，此时需要下载curl的https根证书：
```
wget https://curl.haxx.se/ca/cacert.pem
```
> 然后复制到上面的目录下的相应文件。

### 1.2 自行编译

需要先构建 libcurl、libssl（版本为1.0.2）、libcrypto（版本为1.0.2）和libz的静态链接库。

#### openssl-1.0.2u编译

```
wget https://www.openssl.org/source/old/1.0.2/openssl-1.0.2u.tar.gz
tar -xzf openssl-1.0.2u.tar.gz
cd openssl-1.0.2u
./config shared --prefix=/tmp/ssl -fPIC
make
make install
```

### curl7.47.0编译

```
wget https://github.com/curl/curl/releases/download/curl-7_47_0/curl-7.47.0.tar.gz
tar -zxf curl-7.47.0.tar.gz
cd curl-7.47.0
CFLAGS="-fPIC" ./configure --prefix=/tmp/curl --without-nss --with-ssl=/tmp/ssl --with-ca-bundle=/etc/ssl/certs/ca-certificates.crt //指定前面安装的openssl的所在位置，否则不支持https协议。并且指定了https的默认证书位置。
make
make install
```

### zlib 1.2.11编译

```
wget http://www.zlib.net/zlib-1.2.11.tar.gz
tar -zxf zlib-1.2.11.tar.gz
cd zlib-1.2.11
CFLAGS="-fPIC" ./configure  --prefix=/tmp/zlib 
make 
make install
```

### 插件编译

在使用`make`构建时，需要指定 CURL_DIR和SSL_DIR（假定 libssl.a 和 libcrypto.a 在同一个目录下）。例如：

```
CURL_DIR=/tmp/curl SSL_DIR=/tmp/ssl Z_DIR=/tmp/zlib make
```

会在当前目录下编译出插件库 libPluginHttpClient.so。



## 2 使用

### 2.1 httpGet

发送HTTP GET请求。

语法：

httpClient::httpGet(url, [params], [timeout], [headers])

参数：
* url: 请求的URL字符串。
* params: 一个字符串或一个键和值都是string的字典。http协议Get方法请求的会把参数放在url的后面。
  假如url为 www.dolphindb.cn，
    * 如果params为一个字符串（例如，"example"），则发出的完整http报文的请求头中的url为 "www.dolphindb.cn?example"。
    * 如果params为一个字典（例如，两个键值对"name"->"zmx"和"id"->"111"），则发出的完整http报文的请求头中的url为 "www.dolphindb.cn?id=111&name=zmx"。
* timeout: 超时时间，单位为毫秒。
* headers: 一个字符串或一个键和值都是string的字典，填写http请求头部。如果headers为一个字典（两个键值对"groupName"->"dolphindb"和"groupId"->"11"），
则发出的完整http报文添加请求头"groupId:11"和"groupName:dolphindb"。如果只是一个字符串，则必须是"xx:xx"格式，会添加一个http请求头。

返回一个dictionary，包括以下键：
- responseCode: 请求返回的响应码
- headers: 请求返回的头部
- text: 请求返回的内容文本
- elapsed: 请求经过的时间

例子：
```
loadPlugin('/home/zmx/worker/DolphinDBPlugin/httpClient/PluginHttpClient.txt');
param=dict(string,string);
header=dict(string,string);
param['name']='zmx';
param['id']='111';
header['groupName']='dolphindb';
header['groupId']='11';
//Please set up your own httpServer ex.(python -m SimpleHTTPServer 8900)
url = "localhost:8900";
res = httpClient::httpGet(url,param,1000,header);
```

### 2.2 httpPost

发送HTTP POST请求。

语法：

httpClient::httpPost(url, [params], [timeout], [headers])

参数：
* url: 请求的URL字符串。
* params: 一个字符串或一个key是string的字典。http协议Post方法请求的会把参数放在http请求正文中。
    * 如果params为一个字符串（例如，"example"），则发出的完整http报文的请求正文l为"example"。
    * 如果params为一个字典（例如，两个键值对"name"->"zmx"和"id"->"111"），则发出的完整http报文的请求正文为 "id=111&name=zmx"。
* timeout: 超时时间，单位为毫秒。
* headers: 一个字符串或一个键和值都是string的字典，填写http请求头部。如果headers为一个字典（两个键值对"groupName"->"dolphindb"和"groupId"->"11"），
则发出的完整http报文添加请求头"groupId:11"和"groupName:dolphindb"。如果只是一个字符串，则必须是"xx:xx"格式，会添加一个http请求头。

返回一个dictionary，键包括：
- responseCode: 请求返回的响应码
- headers: 请求返回的头部
- text: 请求返回的内容文本
- elapsed: 请求经过的时间

例子：
```
loadPlugin('/home/zmx/worker/DolphinDBPlugin/httpClient/PluginHttpClient.txt');
param=dict(string,string);
header=dict(string,string);
param['name']='zmx';
param['id']='111';
header['groupName']='dolphindb';
header['groupId']='11';
//Please set up your own httpServer ex.(python -m SimpleHTTPServer 8900)
url = "localhost:8900";
res = httpClient::httpPost(url,param,1000,header);
```

### 2.2 httpCreateSubJob

创建一个循环请求httpGet的请求后台任务。

语法：

httpClient::httpCreateSubJob(url, handle, parser, [paserInterval], [cycles], [cookieFile], [param], [timeout], [headers])

参数：
* url: 请求的URL字符串。类型是STRING类型的向量或者是常量。
* handler: 一个函数或表，用于处理从http请求正文中接收的消息。
* parser: 一个函数，用于对http请求正文解析。
* paserInterval: parser函数每次解析http正文的字节数。若不指定则默认为每10240字节用parser解析收到的请求报文。
* cycles: 循环次数。若不指定则为无限循环。
* cookieFile: 保存http协议Session连接中的cookie的文件名。类型为string类型。使用这个文件中的字符串初始化一个http Session的cookie，在发送的httpGet请求添加一个请求头"Cookie"。如果收到的http响应中有响应头"Set-Cookie"，会把cookie的值写入到这个文件中。如果为空，默认在当前DolphinDB server的home目录下httpClientCookie文件夹下。
* params: 一个字符串或一个key是string的字典。http协议Post方法请求的会把参数放在http请求正文中。
    * 如果params为一个字符串（例如，"example"），则发出的完整http报文的请求正文l为"example"。
    * 如果params为一个字典（例如，两个键值对"name"->"zmx"和"id"->"111"），则发出的完整http报文的请求正文为 "id=111&name=zmx"。
* timeout: 超时时间，单位为毫秒。
* headers: 一个字符串或一个键和值都是string的字典，填写http请求头部。如果headers为一个字典（两个键值对"groupName"->"dolphindb"和"groupId"->"11"），
则发出的完整http报文添加请求头"groupId:11"和"groupName:dolphindb"。如果只是一个字符串，则必须是"xx:xx"格式，会添加一个http请求头。

例子：
```
url="127.0.0.1:8900/chunk_file"
st = streamTable(1000000:0,`tag`ts`data,[SYMBOL,TIMESTAMP,INT])
enableTableShareAndPersistence(table=st, tableName=`sc, asynWrite=true, compress=true, cacheSize=100000)
job=httpClient::httpCreateSubJob(url, st, <parser>, 2560,  , "/home/zmx/httpJobCookie", )
```

### 2.4 httpGetJobStat

查询所有订阅信息。

语法：

httpClient::httpGetJobStat()

返回一个表，包含如下字段：
* subscriptionId：订阅标志符。
* user：建立订阅的的会话用户。
* url：请求的url。
* cookie：http session会话的cookie信息。
* cycles_completed：已完成的循环次数。
* createTimestamp： 订阅建立时间。
* readByte: 读取到的http正文的字节数。
* dataNumber：成功使用parser解析出来的数据行数。

例子：
```
httpClient::httpGetJobStat()
```

### 2.5 httpCancelSubJob

取消一个http后台任务。

语法：

httpClient::httpCancelSubJob(subscription)

参数：
* subscription: - 'subscription'是`httpCreateSubJob`函数返回的值或`httpGetJobStat`返回的订阅标识符。

例子：
```
httpClient::httpCancelSubJob(subscription)
```

### 2.5 sendEmail

发送邮件。
- 本插件使用smtp邮件传输协议，所以邮件服务器必须支持smtp协议和开启smtp端口，如果邮件服务商没有默认开启smtp端口，则需要开启该账号邮箱的smtp服务。还需要注意该邮件服务商是否提供邮箱授权码的功能，如果有，此时的参数pwd为邮箱授权码而非邮箱密码。
- 如果成功发送邮件，返回字典res，res['responseCode']==250。

语法：

httpClient::sendEmail(userId,pwd,recipient,subject,body)

参数：
* userId: 发送者邮箱账号。
* pwd: 发送者邮箱密码。
* recipient: 目标邮箱账号的一个字符串或一个字符串集合。
* subject: 邮件主题的字符串。
* body: 邮件正文的字符串。

返回一个dictionary，键包括：
- userId: 发送者邮箱账号
- recipient: 接受者邮箱的集合的字符串
- responseCode: 请求返回的响应码
- headers: 请求返回的头部
- text: 请求返回的内容文本
- elapsed: 请求经过的时间

例子：
```
res=httpClient::sendEmail('MailFrom@xxx.com','xxxxx','Maildestination@xxx.com','This is a subject','It is a text');
```
```
recipient='Maildestination@xxx.com''Maildestination2@xxx.com''Maildestination3@xxx.com';
res=httpClient::sendEmail('MailFrom@xxx.com','xxxxx',recipient,'This is a subject','It is a text');
```

### 2.6 emailSmtpConfig

配置邮件服务器。

语法：

httpClient::emailSmtpConfig(emailName,host,post)

参数：
* emailName: 邮箱名称，格式为邮箱'@'字符后的字符串。类型为字符串。如果是qq邮箱，则是"qq.com"。如果是yeah邮箱，则是"yeah.net"。
* host: 邮箱stmp服务器的地址。类型为字符串。
* port: 邮箱服务器端口。类型为INT。如果为空，默认为25。

例子：
```
emailName="qq.com";
host="smtp.qq.com";
port=25;
httpClient::emailSmtpConfig(emailName,host,port);
```
