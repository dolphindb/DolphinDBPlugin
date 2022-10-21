# DolphinDB HTTP Client Plugin

DolphinDB's httpClient plugin enables you to send HTTP requests conveniently.

The DolphinDB HTTPClient plugin has the branches [release 200](https://github.com/dolphindb/DolphinDBPlugin/blob/release200/httpClient/README_EN.md) and [release130](https://github.com/dolphindb/DolphinDBPlugin/blob/release130/httpClient/README_EN.md). Each plugin version corresponds to a DolphinDB server version. You're looking at the plugin documentation for release200. If you use a different DolphinDB server version, please refer to the corresponding branch of the plugin documentation.

## Build

**Prerequisities**: The static library of libcurl, libssl(version 1.0.2), libcrypto(version 1.0.2) and libz are available.

In the make build command, we need to specify the path of CURL_DIR, SSL_DIR and Z_DIR like following command:

```
CURL_DIR=/path/to/curl-7.47.0 SSL_DIR=/path/to/openssl-1.0.2i Z_DIR=/path/to/zlib-1.2.11 make
```

**Note:** If your g++ version is higher than 5.0, you need to add -D_GLIBCXX_USE_CXX11_ABI=0 in makefile's CFLAGS option.

## Load Plugin

If the file libPluginHttpClient.so is built, you can load httpClient Plugin with the following command：

```
loadPlugin("/path/to/PluginHttpClient.txt");
```

# API
HttpClient provides two kinds of method to send HTTP requests. They all return an instance of dictionary.This dictionary contains following keys:
- responseCode: HTTP Status Code
- headers: HTTP Response Header
- text: HTTP Response body
- elapsed: HTTP request elapsed time

## 1. httpClient::get

### Syntax

```
httpClient::get(url, [params = None], [timeout = 0], [nobody = false])
```

### Arguments

- url: a string indicating the url of HTTP request.
- params: a string or dict to send in the query string of HTTP request.
- timeout: a non-negative integer indicating the the maximum time in milliseconds that you allow the HTTP request to take.Default timeout is 0 which means it never times out during transfer.
- nobody: a Boolean value indicating whether the return value will contain the body of HTTP reponse.

### Return Value

A dictionary object.

### Example

```
dic = dict(['theStockCode'], ['sh000001']);
res = httpClient::get("http://www.example.com/getStockImageByCode", dic, 1000, true);
```
This example uses the `get` method to request <http://www.example.com/getStockImageByCode?theStockCode=sh000001>. The timeout is 1 second, and the body of http response will not be returned.

## 2. httpClient::post

### Syntax

```
httpClient::post(url, [data = None], [timeout = 0], [nobody = false])
```

### Arguments

- url: a string indicating the url of HTTP request.
- data: a string or dict to send in the body of HTTP request.
- timeout: a non-negative integer indicating the the maximum time in milliseconds that you allow the HTTP request to take.Default timeout is 0 which means it never times out during transfer.
- nobody: a bool value indicating whether the return value will contain the body of HTTP reponse.

### Return Value

A dictionary object.

### Example

```
dic = dict(['theStockCode'], ['sh000001']);
res = httpClient::post("http://www.examle.com/getStockImageByCode", dic, 1000, false);
```
This example uses post method to request <http://www.example.com/getStockImageByCode>. The timeout is 1 second, and the body of http response will be returned. 
