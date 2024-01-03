# DolphinDB HTTP Client Plugin

DolphinDB's httpClient plugin enables you to send HTTP requests and emails conveniently.

The DolphinDB httpClient plugin has different branches, such as release200 and release130. Each branch corresponds to a DolphinDB server version. Please make sure you are in the correct branch of the plugin documentation.

- [DolphinDB HTTP Client Plugin](#dolphindb-http-client-plugin)
  - [1. Build](#1-build)
    - [1.1 Load Plugin](#11-load-plugin)
    - [1.2 (Optional) Manually Build Plugin](#12-optional-manually-build-plugin)
  - [Methods](#methods)
    - [2.1 httpGet](#21-httpget)
    - [2.2 httpPost](#22-httppost)
    - [2.3 emailSmtpConfig](#23-emailsmtpconfig)
    - [2.8 sendEmail](#28-sendemail)


## 1. Build

### 1.1 Load Plugin

Download the precompiled plugin file under *DolphinDBPlugin/httpClient/bin/linux64*. Load the plugin with the following command in DolphinDB：

```
loadPlugin("/path/to/PluginHttpClient.txt");
```

> This plugin uses the libcurl library. If the message “*curl return: error setting certificate verify locations: CAfile: /etc/ssl/certs/ca-certificates.crt CApath: none*“ is displayed when visiting an HTTPS website, it’s because cURL expects the certificate to be at the default path. Please download the root certificate for cURL using this command:

```
wget https://curl.haxx.se/ca/cacert.pem
```

> Then place the downloaded file under the default directory as mentioned in the error message.

### 1.2 (Optional) Manually Build Plugin

**Prerequisites**: The static libraries of libcurl, libssl(version 1.0.1 on ARM; version 1.0.2 on x86), libcrypto(version 1.0.1 on ARM; version 1.0.2 on x86) and libz are available.

#### OpenSSL

version 1.0.1 for ARM architecture:

```
wget https://www.openssl.org/source/old/1.0.1/openssl-1.0.1u.tar.gz
tar -xzf openssl-1.0.2u.tar.gz
cd openssl-1.0.1u
./config shared --prefix=/tmp/ssl -fPIC
make
make install
```

version 1.0.2 for x86 architecture:

```
wget https://www.openssl.org/source/old/1.0.2/openssl-1.0.2u.tar.gz
tar -xzf openssl-1.0.2u.tar.gz
cd openssl-1.0.2u
./config shared --prefix=/tmp/ssl -fPIC
make
make install
```

#### cURL 7.47.0

```
wget https://github.com/curl/curl/releases/download/curl-7_47_0/curl-7.47.0.tar.gz
tar -zxf curl-7.47.0.tar.gz
cd curl-7.47.0
CFLAGS="-fPIC" ./configure --prefix=/tmp/curl --without-nss --with-ssl=/tmp/ssl --with-ca-bundle=/etc/ssl/certs/ca-certificates.crt //specify the openssl path for https support. also specify the path to the default https certificate
make
make install
```

#### zlib 1.2.11

```
wget http://www.zlib.net/zlib-1.2.11.tar.gz
tar -zxf zlib-1.2.11.tar.gz
cd zlib-1.2.11
CFLAGS="-fPIC" ./configure  --prefix=/tmp/zlib 
make 
make install
```

#### Build the Plugin

In the make build script, we need to specify the path of CURL_DIR and SSL_DIR and Z_DIR. For example:

```
mkdir build
cd build
cmake -DCURL_DIR=/tmp/curl -DSSL_DIR=/tmp/ssl -DZ_DIR=/tmp/zlib  ..
make -j
```

The plugin library *libPluginHttpClient.so* will be generated under the current directory.

## Methods

### 2.1 httpGet

Send HTTP GET request.

**Syntax**

```
httpClient::httpGet(url, [params], [timeout], [headers])
```

**Arguments**

- *url*: a string indicating the URL of HTTP request.
- *params*: a string or dict (whose keys and values are strings) to send in the HTTP request. For example, suppose *url* is specified as “www.dolphindb.com“:
  - if *params* is a string (e.g., “example“), the URL of the HTTP request would be “www.dolphindb.com?example“;
  -  if  *params*  is a dictionary (e.g., with 2 key-value pairs “name“->”ddb” and “id“->”111”), the URL of the HTTP request would be “www.dolphindb.com?id=111&name=ddb“.
- *timeout*: a non-negative integer indicating the the maximum time in milliseconds that an HTTP request can take. Default timeout is 0 which means it never times out during transfer.
- *headers*: a string or dict (whose keys and values are strings) indicating the headers of the HTTP request. 
  - If it’s a dictionary (e.g., with 2 key-value pairs “groupName“->”dolphindb” and “groupId“->”11”), two headers “groupId:11“ and “groupName:dolphindb“ will be added to the HTTP request ;
  - If it’s a string, it must have the pattern `<key>:<value>` and it will be added as a header to the request. 

**Return** 

A dictionary object containing following keys:

- *responseCode*: HTTP status code
- *headers*: HTTP response header
- *text*: HTTP response body
- *elapsed*: HTTP request elapsed time

**Example**

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

Send HTTP POST request.

**Syntax**

httpClient::httpPost(url, [params], [timeout], [headers])

**Arguments**

- *url*: a string indicating the URL of HTTP request.
- *params*: a string or dict (whose keys and values are strings) to send in the HTTP request. For example, suppose *url* is specified as “DolphinDB - High Performance Time-series Database “:
  - if *params* is a string (e.g., “example“), the URL of the HTTP request would be “www.dolphindb.com?example“;
  -  if  *params*  is a dictionary (e.g., with 2 key-value pairs “name“->”ddb” and “id“->”111”), the URL of the HTTP request would be “www.dolphindb.com?id=111&name=ddb“.
- *timeout*: a non-negative integer indicating the the maximum time in milliseconds that you allow the HTTP request to take. Default timeout is 0 which means it never times out during transfer.
- *headers*: a string or dict (whose keys and values are strings) indicating the headers of the HTTP request. 
  - If it’s a dictionary (e.g., with 2 key-value pairs “groupName“->”dolphindb” and “groupId“->”11”), two headers “groupId:11“ and “groupName:dolphindb“ will be added to the HTTP request ;
  - If it’s a string, it must have the pattern `<key>:<value>` and it will be added as a header to the request. 

**Return** 

A dictionary object containing following keys:

- *responseCode*: HTTP status code
- *headers*: HTTP response header
- *text*: HTTP response body
- *elapsed*: HTTP request elapsed time

**Example**

```
loadPlugin('/home/zmx/worker/DolphinDBPlugin/httpClient/PluginHttpClient.txt');
param=dict(string,string);
header=dict(string,string);
param['name']='zmx';
param['id']='111';
header['groupName']='dolphindb';
header['groupId']='11';
//Please set up your own httpServer e.g. (python -m SimpleHTTPServer 8900)
url = "localhost:8900";
res = httpClient::httpPost(url,param,1000,header);
```

### 2.3 emailSmtpConfig

Configure the email server. 

Note: To send email using the httpClient plugin, the SMTP service on your mail account must be enabled.

**Syntax**

httpClient::emailSmtpConfig(emailName,host,post)

**Arguments**

- *emailName*: a string indicating the domain name in your email address. For example, `"gmail.com"` or `"yahoo.com"`.
- *host*: a string indicating your SMTP server address.
- *port*: an integer (INT) indicating the port number of your email server. If this parameter is unspecified, it takes the default value of 25.

**Examples**

```
emailName="example.com";
host="smtp.example.com";
port=25;
httpClient::emailSmtpConfig(emailName,host,port);
```

### 2.8 sendEmail

Send an email.

- This plugin uses the SMTP protocol for sending email, so the mail server must support SMTP and have the SMTP port open. If the email service provider does not enable SMTP by default, you will need to enable SMTP service for that email account. Also check if the provider offers authorization codes for accounts. If so, use the authorization code for the *pwd* parameter instead of the account password.
- If the email is sent successfully, a dictionary "res" will be returned with `res['responseCode']==250`.

**Syntax**

httpClient::sendEmail(userId,pwd,recipient,subject,body)

**Arguments**

- user: a string indicating the email address for sending the email.
- pwd: a string or a vector of strings. If an SMTP password is provided, specify *pwd* as the SMTP password. Otherwise, it is the password to your mailbox.
- recipient: a string indicating the mail address(es) to send the mail to. 
- subject: a string indicating the subject of the email.  
- body: a string indicating the subject of the email.  

**Return**

Return a dictionary “res“ containing the following fields: *userId* (email address of the sender), *recepient*, *responseCode*, *headers*, *text*, *elapsed* (the elapsed time since the request was sent).

If the email is sent successfully, `res['responseCode']==250` is returned.

**Example**

Send mail to one recipient:

```
res=httpClient::sendEmail('MailFrom@example.com','xxxxx','MailTo@example.com','This is a subject','It is a text');
```

Send to multiple recipients:

```
recipients='recepient1@example.com''recepient2@example.com''recepient3@example.com';
res=httpClient::sendEmail('MailFrom@example.com','xxxxx',recipients,'This is a subject','It is a text');
```

