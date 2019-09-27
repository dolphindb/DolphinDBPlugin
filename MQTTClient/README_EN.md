# MQTT Client Plugin

## 1. Build(tested on Linux 64bit gcc version 5.4.0)

1. This plugin has been successfully compiled with gcc (version 5.4.0) on 64-bits Linux operating system.
2. Before compiling, install [CMake](https://cmake.org/).
3. Create a 'build' directory. Enter it and run `cmake` and `make` to generate `libPluginMQTTClient.so`。

```
mkdir build
cd build
cmake ..
make
```

## 2. Load Plugin

Use the function `loadPlugin` to load MQTT client plugin.

```
loadPlugin("/YOUR_PATH/MQTTClient/PluginMQTTClient.txt"); 
```

## 3. Publish

**Syntax**

```
MQTTClient::mqttClientPub(host, port, topic, message,messageLen, QoS)
```

**Arguments**

- `host` is a string indicating the IP address of MQTT server/broker.

- `port` is an integer indicating the port of MQTT server/broker.

- `topic` is a string indicating the topic.

- `message` is the message.

- `messageLen` is an integer indicating the length of message. It is optional.

- `Qos` is an integer indicating the quality of service. It is optional. The default value is 0.

**Details**

Publish a message to MQTT server/broker.

**Example**

```
try {      
   MQTTClient::mqttClientPub("test.mosquitto.org",1883,"dolphindb/topic1","welcome",7,1)
}
catch(ex)
{
   print ex
};

```

## 4. Subscribe

### 4.1 Subscribe

**Syntax**

```
MQTTClient::mqttClientSub(host, port, topic, handler)
```

**Arguments**

- `host` is a string indicating the IP address of MQTT server/broker.

- `port` is an integer indicating the port of MQTT server/broker.

- `topic` is a string indicating the topic.

- `handler` is a function. It is used to process the subscribed data.

**Details**

Subscribe a MQTT server/broker. It returns a handler.

**Example**

```
t1 = table( 1000:0, `time`topic`msg, [TIMESTAMP, STRING, STRING])
def callback(mutable t,d1,d2) {
	insert into t values(now(), d1, d2)

}
callback_1 = callback{t1}

try {      
    handle=MQTTClient::mqttClientSub("test.mosquitto.org",1883,"dolphindb/#",callback_1)
}
catch(ex)
{
    print ex
};   

```

### 4.2 Unsubscribe

**Syntax**

```
MQTTClient::mqttClientStopSub(handle)
```

**Arguments**

- `handler` is the handler of subsctiption.

**Example**

```
MQTTClient::mqttClientStopSub(9)
```

## 5. Example

```
loadPlugin("/home/qianxj/MQTTClient/PluginMQTTClient.txt"); 

//subscribe and insert the messages into the table

sensorInfoTable = table( 10000:0,`deviceID`send_time`recv_time`cpu_used`mem_used`temperature ,[STRING, TIMESTAMP,TIMESTAMP, DOUBLE, DOUBLE,DOUBLE])
def callback(mutable t, d1,d2) {
	try {
	      val=split(d2,",")
	      insert into t values(val[0], timestamp(val[1]),now(), double(val[2]),double(val[3]),double(val[4])) 
    }
    catch(ex){
	   print ex
    };  

}
callback_1 = callback{sensorInfoTable}

try {      
    MQTTClient::mqttClientSub("127.0.0.1",1883,"dolphindb/#",callback_1)
}
catch(ex){
	print ex
};   

//publish

def simulatePub(deviceIds,interval,count){
	for(i in 0:count){
		pubstr=deviceIds+ "," + string(now())+"," + string(rand(100.0,1)[0] )+"," + string(rand(100.0,1)[0] )+"," + string(rand(50..80,1) [0])

		try {      
			MQTTClient::mqttClientPub("127.0.0.1",1883,"dolphindb/cpu_mem_temp",pubstr,,0)
		}
		catch(ex){
			print ex;
		}; 
		    		
		sleep(interval-1)	
	}
	
}

submitJob("submit_pub1", "submit_p1", simulatePub{"sensor1001", 10, 1000})
submitJob("submit_pub2", "submit_p2", simulatePub{"sensor1002", 10, 1000})

```
