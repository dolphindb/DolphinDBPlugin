# DolphinDB HTTP Client Plugin

使用该插件可以便捷地进行HTTP请求,邮件发送。

## 构建

需要首先构建`libcurl`, `libssl`(版本为1.0.2), `libcrypto`(版本为1.0.2)和libz的静态链接库。

在使用`make`构建时，需要指定`CURL_DIR`, `SSL_DIR`和`Z_DIR`（假定`libssl.a`和`libcrypto.a`在同一个目录下）。例如：

```
CURL_DIR=/home/zmx/curl-7.47.0 SSL_DIR=/home/zmx/openssl-1.0.2i Z_DIR=/home/zmx/zlib-1.2.11 make
```

## 使用

编译生成`libPluginHttpClient.so`之后，通过以下脚本加载插件：

```
loadPlugin("/path/to/PluginHttpClient.txt");
```

### httpClient::httpGet(url,[params],[timeout],[headers])

发送HTTP GET请求。
参数
* url：为请求的URL字符串。
* params：为一个字符串或一个键和值都是string的字典。http协议Get方法请求的会把参数放在url的后面。
假如url为 www.dolphindb.cn，
1. 如果params为一个字符串("example"),则发出的完整http报文的请求头中的url为 "www.dolphindb.cn?example"。
2. 如果params为一个字典(两个键值对"name"->"zmx"和"id"->"111"),则发出的完整http报文的请求头中的url为 "www.dolphindb.cn?id=111&name=zmx"。
* timeout：为超时时间，单位为毫秒。
* headers：为一个字符串或一个键和值都是string的字典,填写http请求头部。 如果headers为一个字典(两个键值对"groupName"->"dolphindb"和"groupId"->"11"),
则发出的完整http报文添加请求头"groupId:11"和"groupName:dolphindb"。如果只是一个字符串，则必须是"xx:xx"格式，会添加一个http请求头。

返回一个dictionary，键包括：
- `responseCode`: 请求返回的响应码。
- `headers`: 请求返回的头部。
- `text`: 请求返回的内容文本。
- `elapsed`: 请求经过的时间。

例子
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
### httpClient::httpPost(url,[params],[timeout],[headers])

发送HTTP POST请求。

参数
* url：为请求的URL字符串。
* params：为一个字符串或一个key是string的字典。http协议Post方法请求的会把参数放在http请求正文中。
1. 如果params为一个字符串("example"),则发出的完整http报文的请求正文l为"example"。
2. 如果params为一个字典(两个键值对"name"->"zmx"和"id"->"111"),则发出的完整http报文的请求正文为 "id=111&name=zmx"。
* timeout：为超时时间，单位为毫秒。
* headers：为一个字符串或一个键和值都是string的字典,填写http请求头部。 如果headers为一个字典(两个键值对"groupName"->"dolphindb"和"groupId"->"11"),
则发出的完整http报文添加请求头"groupId:11"和"groupName:dolphindb"。如果只是一个字符串，则必须是"xx:xx"格式，会添加一个http请求头。

返回一个dictionary，键包括：
- `responseCode`: 请求返回的响应码。
- `headers`: 请求返回的头部。
- `text`: 请求返回的内容文本。
- `elapsed`: 请求经过的时间。
例子
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
### httpClient::sendEmail(userId,pwd,recipient,subject,body)
发送邮件。
- 使用本插件的邮件发送函数通常需要在邮件服务商上面设置开启smtp协议，还有需要获取邮箱授权码，参数pwd为邮箱授权码的字符串。
- 如果成功发送邮件，返回的字典res，res['responseCode']==250。

参数
* userId：发送者邮箱账号。
* pwd：发送者邮箱密码(授权码)。
* recipient：目标邮箱账号的一个字符串或一个字符串集合。
* subject：邮件主题的字符串。
* body：为邮件正文的字符串。

返回一个dictionary，键包括：
- `userId`: 发送者邮箱字符串。
- `recipient`: 接受者邮箱的集合的字符串。
- `responseCode`: 请求返回的响应码。
- `headers`: 请求返回的头部。
- `text`: 请求返回的内容文本。
- `elapsed`: 请求经过的时间。

例子
```
res=httpClient::sendEmail('MailFrom@xxx.com','xxxxx','Maildestination@xxx.com','This is a subject','It is a text');
```
```
emailTo='Maildestination@xxx.com''Maildestination2@xxx.com''Maildestination3@xxx.com';
res=httpClient::sendEmail('MailFrom@xxx.com','xxxxx',emailTo,'This is a subject','It is a text');
```


