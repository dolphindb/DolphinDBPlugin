# DolphinDB HTTP Client Plugin

使用DolphinDB的http请求插件可以便捷地进行HTTP请求。

## 构建

需要首先构建libcurl, libssl(版本为1.0.2), libcrypto(版本为1.0.2)和libz的静态链接库。

在使用`make`构建时，需要指定CURL_DIR, SSL_DIR和Z_DIR（假定libssl.a和libcrypto.a在同一个目录下）。例如：

```
CURL_DIR=/path/to/curl-7.47.0 SSL_DIR=/path/to/openssl-1.0.2i Z_DIR=/path/to/zlib-1.2.11 make
```
**请注意:** 在编译时若使用g++5.0以上的版本，需要在编译时增加 -D_GLIBCXX_USE_CXX11_ABI=0 选项。

## 插件加载

编译生成 libPluginHttpClient.so 之后，通过以下脚本加载插件：

```
loadPlugin("/path/to/PluginHttpClient.txt");
```

# API
HttpClient插件提供了两种方法发送HTTP请求。它们的返回值相同，均是一个字典对象。该字典对象包含以下的键值：

- responseCode: 请求返回的响应码。
- headers: 请求返回的头部。
- text: 请求返回的内容文本。
- elapsed: 请求经过的时间。


## 1. httpClient::get

### 语法

```
httpClient::get(url, [params = None], [timeout = 0], [nobody = false])
```

### 参数

- url: 一个字符串，表示请求的URL参数。
- params: 一个字符串或字典，表示请求的URL参数。
- timeout: 一个非负整数，表示超时时间，单位为毫秒。默认值为0代表着请求永不超时。
- nobody: 一个布尔值，表示是否返回http请求体。

### 返回值

返回一个字典对象。

### 例子

```
dic = dict(['theStockCode'], ['sh000001']);
res = httpClient::get("http://www.example.com/getStockImageByCode", dic, 1000, true);
```
该例子将用get方法请求<http://www.example.com/getStockImageByCode?theStockCode=sh000001>, 超时时间为1秒，不返回请求体。

## 2. httpClient::post

### 语法

```
httpClient::post(url, [data = None], [timeout = 0], [nobody = false])
```

### 参数

- url: 一个字符串，表示请求的URL参数。
- data: 一个字符串或字典，表示HTTP请求中body部分包含的数据。
- timeout: 一个非负整数，表示超时时间，单位为毫秒。默认值为0代表着请求永不超时。
- nobody: 一个布尔值，表示是否返回http请求体。

### 返回值

返回一个字典对象。
### 例子

```
dic = dict(['theStockCode'], ['sh000001']);
res = httpClient::post("http://www.examle.com/getStockImageByCode", dic, 1000, false);
```
该例子将用post方法请求<http://www.example.com/getStockImageByCode>, 超时时间为1秒，返回请求体。