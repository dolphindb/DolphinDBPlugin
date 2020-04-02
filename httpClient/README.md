# DolphinDB HTTP Client Plugin

使用该插件可以便捷地进行HTTP请求。

## 构建

需要首先构建`libcurl`, `libssl`(版本为1.0.2), `libcrypto`(版本为1.0.2)和libz的静态链接库。

在使用`make`构建时，需要指定`CURL_DIR`, `SSL_DIR`和`Z_DIR`（假定`libssl.a`和`libcrypto.a`在同一个目录下）。例如：

```
CURL_DIR=/home/ywang/curl-7.47.0 SSL_DIR=/home/ywang/openssl-1.0.2i Z_DIR=/home/ywang/zlib-1.2.11 make
```

## 使用

编译生成`libPluginHttpClient.so`之后，通过以下脚本加载插件：

```
loadPlugin("/path/to/PluginHttpClient.txt");
```

### httpClient::httpGet(url,[params],[timeout])

发送HTTP GET请求。

参数`url`为请求的URL字符串。参数`params`为一个字符串或一个dictionary。参数`timeout`为超时时间，单位为毫秒。

返回一个dictionary，键包括：
- `responseCode`: 请求返回的响应码。
- `headers`: 请求返回的头部。
- `text`: 请求返回的内容文本。
- `elapsed`: 请求经过的时间。

### httpClient::httpPost(url,[params],[timeout])

发送HTTP POST请求。

参数和返回值和`httpGet`相同。