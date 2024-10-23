# DolphinDB OPCUA Plugin

OPC UA is a data exchange standard for safe, reliable, manufacturer-independent, and platform-independent industrial communication. It enables secure data exchange between hardware platforms from different vendors and across operating systems. The DolphinDB OPCUA plugin enables data transfer between DolphinDB and OPC UA servers.

- [DolphinDB OPCUA Plugin](#dolphindb-opcua-plugin)
  - [Install with `installPlugin`](#install-with-installplugin)
    - [Installation Steps:](#installation-steps)
  - [Method References](#method-references)
    - [getOpcServerList](#getopcserverlist)
    - [getOpcEndPointList](#getopcendpointlist)
    - [connect](#connect)
    - [browseNode](#browsenode)
    - [readNode](#readnode)
    - [writeTag](#writetag)
    - [subscribe](#subscribe)
    - [getSubscriberStat](#getsubscriberstat)
    - [unsubscribe](#unsubscribe)
    - [close](#close)
  - [Appendix: Manual Installation](#appendix-manual-installation)
    - [Download Precompiled Binaries](#download-precompiled-binaries)
    - [Compile from Source](#compile-from-source)
    - [Use cmake to build libPluginOPCUA](#use-cmake-to-build-libpluginopcua)

## Install with `installPlugin`

Required server version: DolphinDB 1.30.22/2.00.10 or higher

### Installation Steps:

(1) Use `listRemotePlugins` to check plugin information in the plugin repository

```
login("admin", "123456")
listRemotePlugins(, "http://plugins.dolphindb.com/plugins/")
```

(2) Invoke `installPlugin` for plugin installation

```
installPlugin("opcua")
```

It returns <path_to_OPCUA_plugin>/PluginOPCUA.txt.

## Method References

Before using the plugin methods, the plugin must be loaded with `loadPlugin("/path/to/PluginOPCUA/PluginOPCUA.txt")`. Note that for Windows, you must specify the absolute path during plugin loading, and use "\\" or "/" instead of "\" in the path.

The server URL *opc.tcp://118.24.36.220:62547/DataAccessServer* is an online service endpoint that can be used to test the plugin's connection, reading/writing nodes, subscribing, and other functions.

The [prosys opc-ua-simulation-server](https://downloads.prosysopc.com/opc-ua-simulation-server-downloads.php) provides a local server. By following the [user manual](https://downloads.prosysopc.com/opcua/apps/JavaServer/dist/4.0.2-108/Prosys_OPC_UA_Simulation_Server_UserManual.pdf), you can specify endpoints, encryption policies, user tokens, manage certificates, etc. It can be used to test the plugin's encrypted communication functions.

### getOpcServerList

**Syntax**

```
opcua::getOpcServerList(serverUrl)
```

**Parameters**

- **serverUrl** is a string indicating the server URL. For example opc.tcp://127.0.0.1:53530/OPCUA/SimulationServer/.

**Details**

Get a list of OPC servers. It returns a table with five columns:

- ServerUri: The unique identifier for the server.
- ServerName: The server name.
- ProductUri: The unique identifier for the product.
- Type: The server type.
- DiscoveryUrl: URL(s) for the discovery endpoints. Separate multiple URLs with semicolon ";".

**Examples**

```
opcua::getOpcServerList(serverUrl);
```

### getOpcEndPointList

**Syntax**

```
opcua::getOpcEndPointList(serverUrl)
```

**Parameters**

- **serverUrl** is a string indicating the server URL. For example opc.tcp://127.0.0.1:53530/OPCUA/SimulationServer/.

**Details**

Get a list of OPC server endpoints. It returns a table with five columns:

- endpointUrl: The endpoint URL.
- transportProfileUri: The unique identifier of the transport configuration.
- securityMode: The security mode.
- securityPolicyUri: The unique identifier of security policy.
- securityLevel: The security level.

**Examples**

```
opcua::getOpcEndPointList(serverUrl);
```

### connect

**Syntax**

```
opcua::connect(endPointUrl,clientUri,[userName],[userPassword],[securityMode],[securityPolicy],[certificatePath],[privateKeyPath])
```

**Parameters**

- **endPointUrl** is a string specifying the endpoint URL to which the client attempted to connect.
- **clientUri** is a string representing the unique identifier of client. If certificate is specified, the uri must be consistent.
- **userName** is a string representing the username. It can be skipped if server is not set.
- **userPassword** is a string representing the user password.
- **securityMode** is a string indicating the security mode for connection. It must be "None" (default), "Sign", or "SignAndEncrypt".
- **securityPolicy** is a string representing the security algorithm for connection. It must be "None" (default), "Basic256", "Basic128Rsa15", or "Basic256Sha256". If using "Basic256", "Basic128Rsa15", or "Basic256Sha256" encryption, *certificate* and *privateKey* must also be specified. The certificate and privateKey under ./open62541/cert/ directory can be used, or custom certificate and key can be used (tools of [open62541](https://github.com/open62541/open62541/tree/master/tools/certs) can be used to generate certificates).
- **certificatePath** is a string specifying the certificate path. It can be omitted if encrypted communication is not used.
- **privateKeyPath** is a string specifying the private key path. It can be omitted if encrypted communication is not used.

**Details**

Connect to an OPC server. The method returns a connection that can be explicitly closed by calling the close function. If encrypted communication is enabled, a trusted certificate must be specified on the server. If using encryption with the Prosys local simulation server, a user needs to be added in the Users interface with a username and password. For example, for username "user1" and password "123456", trust `open62541server@localhost` in the Certificates interface.

**Examples**

```
connection=opcua::connect(serverUrl,"myClient");
connection=opcua::connect(serverUrl,"myClient","user1","123456");
connection=opcua::connect(serverUrl,"urn:opcua.client.application","user1","123456","SignAndEncrypt","Basic128Rsa15","./open62541/cert/client_cert.der","./open62541/cert/client_key.der");
```

### browseNode

**Syntax**

```
opcua::browseNode(connection)
```

**Parameters**

- **connection** is the return value of method connect.

**Details**

Return a table of all nodes, with 2 columns nodeNamespace and nodeIdString.

**Examples**

```
connection=opcua::connect(serverUrl,"myClient");
opcua::browseNode(connection);
```

### readNode

**Syntax**

```
opcua::readNode(connection, nodeNamespace, nodeIdString, [table])
```

**Parameters**

- **connection** is the return value of method connect.
- **nodeNamespace** is an INT scalar or vector indicating the node namespace(s).
- **nodeIdString** is a STRING scalar or array indicating the node ID(s).
- **table** stores the accessed node values. If a table is specified, all values are inserted into the table. If multiple tables are specified, it must be a tuple of tables and its size must match the number of nodes. The value of each node is inserted into the corresponding table. If no table is specified, the method returns a table that stores the node values.

**Details**

Read the node value synchronously. An OPC connection must be established first before using the method. The result returned for each node includes four values:

- node id: The node ID, i.e., nodeNamespace and nodeIdString concatenated by ":".
- value: The node value.
- timestamp: The source timestamp in local time.
- status: The status of the node value.

**Examples**

```
t1 = table(200:0,`nodeID`value`timestamp`status, [SYMBOL, INT, TIMESTAMP, SYMBOL])
opcua::readNode(conn, 3, "Counter",t1)
opcua::readNode(conn, 3, ["Counter","Expression","Random","Sawtooth"],t1)
t2 = table(200:0,`nodeID`value`timestamp`status`nodeID`value`sourceTimestamp`status,[SYMBOL, INT, TIMESTAMP, SYMBOL，SYMBOL, INT, TIMESTAMP, SYMBOL])
opcua::readNode(conn, 1, ["test1","test4"],t2)
t3 = table(200:0,`nodeID`value`timestamp`status, [SYMBOL, INT, TIMESTAMP, SYMBOL])
t4 = table(200:0,`nodeID`value`timestamp`status, [SYMBOL, INT, TIMESTAMP, SYMBOL])
t5 = table(200:0,`nodeID`value`timestamp`status, [SYMBOL, INT, TIMESTAMP, SYMBOL])
opc::readNode(conn, 1, ["test1","test4", "test9"],[t3,t4,t5]) 
```

### writeTag

**Syntax**

```
opc::writeTag(connection, nodeNamespace, nodeIdString, value)
```

**Parameters**

- **connection** is the return value of method connect.
- **nodeNamespace** is an INT scalar or array indicating the node namespace(s).
- **nodeIdString** is a string scalar or array indicating the node ID(s).
- **value** is the value(s) to write to the node(s).

**Details**

Write value(s) to one or multiple nodes synchronously. An exception will be thrown if the write type is incorrect.

**Examples**

```
opcua::writeNode(conn,1,"testwrite.test1",1)
opcua::writeNode(conn,1,["testwrite.test5","testwrite.test6"],[33,11])
opcua::writeNode(conn,1,"testwrite.test2",[1,2,3,4]) //one-dimensional array
m = matrix([[1,2,3],[1,2,3]])
opcua::writeNode(conn,1,"testwrite.test3",m) //two-dimensional array
```

### subscribe

**Syntax**

```
opcua::subscribe(connection, nodeNamespace, nodeIdString, handler)
```

**Parameters**

- **connection** is the return value of method connect.
- **nodeNamespace** is an INT scalar or vector indicating the node namespace(s).
- **nodeIdString** is a STRING scalar or vector indicating the node ID(s).
- **handler** is the callback function or table or will be invoked when value changes.

**Details**

Subscribe to the changes of node values. Note that currently one subscription uses one connection exclusively, i.e., if subscribe is called on a connection, that connection cannot be used for `readNode`, `writeNode`, etc.

**Examples**

```
t1 = table(200:0,`nodeID`value`timestamp`status, [SYMBOL, INT, TIMESTAMP, SYMBOL])
conn1=opcua::connect(serverUrl，"myClient")
opcua::subscribe(conn1,1,"test.subscribe",t1)
t2 = table(200:0,`nodeID`value`timestamp`status, [STRING, INT, TIMESTAMP, STRING])
conn2=opcua::connect(serverUrl，"myClient")
opcua::subscribe(conn2, 3, ["Counter","Expression","Random","Sawtooth"],t2)
t3 = table(200:0,`nodeID`value`timestamp`status, [SYMBOL, BOOL, TIMESTAMP, SYMBOL])
def callback1(mutable t, d) {
	t.append!(d)
}
conn3=opcua::connect(serverUrl，"myClient")
opcua::subscribe(conn3,2, "testsubscribe",callback1{t3})
```

### getSubscriberStat

**Syntax**

```
opcua::getSubscriberStat()
```

**Details**

Get status of all current subscriptions, including:

- subscriptionId: The subscription ID.
- user: The user who created the subscription.
- endPointUrl: The endpoint URL of the connection.
- clientUri: The client identifier of the connection.
- nodeID: All subscribed nodes, represented as "nodeNamespace:nodeIdString" for each node and separated by a semicolon ';'.
- createTimestamp: The time when the subscription was created.
- receivedPackets: Number of packets received for the subscription.
- errorMsg: Latest error message when processing messages.

**Examples**

```
opcua::getSubscriberStat()
```

### unsubscribe

**Syntax**

```
opcua::unsubscribe(subscription)
```

**Parameters**

- **subscription** is the return value of method `connect` or the subscription identifier returned by `getSubscriberStat`.

**Details**

Cancel the client's subscription.

**Examples**

```
opcua::unsubscribe(subscription)
```

### close

**Syntax**

```
opcua::close(connection)
```

**Parameters**

- **connection** is the return value of method connect.

**Details**

Close the connection to the OPC server.

**Examples**

```
opcua::close(connection)
```

## Appendix: Manual Installation

In addition to installing the plugin with function `installPlugin`, you can also install through precompiled binaries or compile from source. These files can be accessed from our [GitHub repository](https://github.com/dolphindb/DolphinDBPlugin/tree/master) by switching to the appropriate version branch.

### Download Precompiled Binaries

You can download the pre-built binaries libPluginOPCUA.dll or libPluginOPCUA.so from the bin directory.

On Linux, set the required dynamic library path for the plugin runtime:

```
export LD_LIBRARY_PATH=/path/to/PluginOPCUA:$LD_LIBRARY_PATH
```

For Windows, you must specify the absolute path during loading, and use "\\" or "/" instead of "\" in the path.

### Compile from Source

You need to first compile the mbedtls static library and the open62541 dynamic library. Steps are:

**On Windows:**

- Install mbedtls

  (1) Download the latest mbedtls project from GitHub:

  ```
  git clone https://github.com/ARMmbed/mbedtls.git
  ```

  (2) Compile to static library using CMake:

  ```
  cd mbedtls
  mkdir build
  cd build
  cmake .. -G "MinGW Makefiles" -DENABLE_PROGRAMS=OFF
  make
  ```

  The compiled static libraries are under *mbedtls/build/library*, named *libmbedcrypto.a*, *libmbedx509.a*, and *libmbedtls.a*.

- Install open62541

  (1) Download the 1.0 version of open62541 project from GitHub:

  ```
  git clone https://github.com/open62541/open62541.git
  git submodule update --init --recursive
  cd open62541
  git checkout 1.0
  ```

  (2) Compile to dynamic library using CMake:

  ```
  cd mbedtls
  mkdir build
  cd build
  cmake .. -G "MinGW Makefiles" -DENABLE_PROGRAMS=OFF
  make
  ```

  Note: The paths for -DMBEDTLS_INCLUDE_DIRS and -DMBEDTLS_LIBRARIES must be replaced with actual path.

**On Linux Ubuntu:**

- Install mbedtls

  ```
  sudo apt-get install libmbedtls-dev
  ```

- Install open62541 same as on Windows, no need to specify -DMBEDTLS_INCLUDE_DIRS and -DMBEDTLS_LIBRARIES.

**On Linux Centos:**

- Install mbedtls:

  ```
  yum install mbedtls-devel
  ```

- Install open62541 same as on Windows, no need to specify -DMBEDTLS_INCLUDE_DIRS and -DMBEDTLS_LIBRARIES.

### Use cmake to build libPluginOPCUA

- (Skip this step on Linux) Copy libmbedcrypto.a, libmbedx509.a, libmbedtls.a from mbedtls/build/library to ./lib directory. Copy mbedtls and psa folders under mbedtls/include to ./include directory.

- Copy .dll files or all .so files from open62541/build/bin to ./lib directory. Copy folders under open62541/build/src_generated/, open62541/include/, open62541/plugins/include/ to ./include directory, and folders under open62541/arch/ to ./include/open62541 directory.

- Use cmake to build libPluginOPCUA. `-G` is not required on Linux.

  ```
  mkdir build
  cd build
  cmake .. -G "MinGW Makefiles" -DLIBDOLPHINDB="path_to_libdolphindb"
  make
  ```

  Note: The path for -DLIBDOLPHINDB must be replaced with actual path.

- Copy libopen62541.dll or `libopen62541.so` to the build directory.