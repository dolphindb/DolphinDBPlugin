//======================================================================================================
// parameter_define.h
// 参数定义头文件
// 版本：v1.0
// lujun 
//======================================================================================================

#ifndef HTSC_MD_GATEWAY_CLIENT_PARAMETER_DEFINE_H
#define HTSC_MD_GATEWAY_CLIENT_PARAMETER_DEFINE_H

namespace com {
namespace htsc {
namespace mdc {
namespace gateway {

//=======================================================================================================
//  基础数据定义
//=======================================================================================================
//缓冲区配置文件路径
const int HTSC_INSIGHT_PATH_MAX_LEN = 4096;
//缓冲区长度
const int HTSC_INSIGHT_BUF_LEN = 2048;
//配置文件中key最大长度
const int HTSC_INSIGHT_PROPERTY_KEY_MAX_LEN = 100;
//配置文件中value最大长度
const int HTSC_INSIGHT_PROPERTY_VALUE_MAX_LEN = 1024;
//默认的配置文件名
const char* const HTSC_INSIGHT_PROPERTIES_FILENAME = "htsc-insight-cpp-config.conf";
//配置文件目录系统环境变量名
const char* const HTSC_INSIGHT_ENV_CONFIG_FOLDER = "HTSC_INSIGHT_ENV_CONFIG_FOLDER";

#define HTSC_INSIGHT_COMMENT_LETTER '#'
#define HTSC_INSIGHT_ASSAN_OPERATOR '='

//=======================================================================================================
//  配置名定义
//=======================================================================================================
//连接等待超时时间，单位：秒
const char* const HTSC_INSIGHT_CONFIG_CONNECT_WAIT_TIME = "connect_wait_time";
//每次发送(send)等待超时时间，单位:秒
const char* const HTSC_INSIGHT_CONFIG_SEND_WAIT_TIMEOUT = "send_wait_timeout";
//每次接收(reveive)等待超时时间，单位：秒
const char* const HTSC_INSIGHT_CONFIG_RECV_WAIT_TIMEOUT = "recv_wait_timeout";
//单次接收(reveive)部分数据最大等待时间，单位：秒
const char* const HTSC_INSIGHT_CONFIG_RECV_N_TIMEOUT = "recv_n_timeout";
//服务发现链路等待超时时间，单位：秒
const char* const HTSC_INSIGHT_CONFIG_DISCOVERY_SERVER_WAIT_TIMEOUT = "discovery_server_wait_timeout";
//心跳等待时间，单位:秒
const char* const HTSC_INSIGHT_CONFIG_HEARTBEAT_SLEEP_TIME = "heartbeat_sleep_time";
//服务端心跳失效时间，单位:秒
const char* const HTSC_INSIGHT_CONFIG_SERVER_HEARTBEAT_TIMEOUT = "server_heartbeat_timeout";
//订阅等待应答时间，单位：秒
const char* const HTSC_INSIGHT_CONFIG_SUBSCRIBE_RESPONSE_MESSAGE_WAIT_TIME = "subscribe_response_message_wait_time";
//订阅重试次数
const char* const HTSC_INSIGHT_CONFIG_SUBSCRIBE_TRY_COUNT = "subscribe_try_count";
//MDQuery请求等待应答时间，单位：秒
const char* const HTSC_INSIGHT_CONFIG_MDQUERY_RESPONSE_MESSAGE_WAIT_TIME = "mdquery_response_message_wait_time";
//Playback请求等待应答时间，单位：秒
const char* const HTSC_INSIGHT_CONFIG_PLAYBACK_RESPONSE_MESSAGE_WAIT_TIME = "playback_response_message_wait_time";
//处理线程池线程初始线程数
const char* const HTSC_INSIGHT_CONFIG_THREAD_POOL_INIT_SIZE = "thread_pool_init_size";
//处理线程池最大线程数
const char* const HTSC_INSIGHT_CONFIG_THREAD_POOL_MAX_SIZE = "thread_pool_max_size";
//每次获取的最大消息数
const char* const HTSC_INSIGHT_CONFIG_MAX_FETCH_MESSAGE_COUNT = "max_fetch_message_count";
//收到回放状态为结束的报文等待payload处理完的时间，单位：秒
const char* const HTSC_INSIGHT_CONFIG_PLAYBACK_END_STATUS_WAIT_PAYLOAD_TIME = "playback_end_status_wait_payload_time";
//线程中sleep时间长度，单位：微秒
const char* const HTSC_INSIGHT_CONFIG_THREAD_SLEEP_TIME = "thread_sleep_time";
//流量日志输出间隔数据条数
const char* const HTSC_INSIGHT_CONFIG_TRAFFIC_CHECK_GAP = "traffic_check_gap";
//监测无数据最大容忍时间，单位：毫秒
const char* const HTSC_INSIGHT_CONFIG_NODATA_CHECK_GAP = "nodata_check_gap";
//心跳日志开关
const char* const HTSC_INSIGHT_CONFIG_HEARTBEAT_TRACE = "heartbeat_trace";
//流量日志开关
const char* const HTSC_INSIGHT_CONFIG_GLOBAL_TRACE = "global_trace";
//解压缩线程开关
const char* const HTSC_INSIGHT_CONFIG_COMPRESS_SWITCH = "compress_switch";
//回复控制开关，打开时回复通过回调返回，调用者处理回复消息
const char* const HTSC_INSIGHT_CONFIG_RESPONSE_CALLBACK = "response_callback";
//登录数据网关尝试次数
const char* const HTSC_INSIGHT_CONFIG_LOGIN_TRY_COUNT = "login_try_count";
//登录服务发现网关尝试次数
const char* const HTSC_INSIGHT_CONFIG_LOGIN_DISCOVERY_TRY_COUNT = "login_discovery_try_count";
//OnMarketdata回调函数中数据禁止析构
const char* const HTSC_INSIGHT_CONFIG_FORBID_DESTRUCT = "forbid_destruct";
//接收数据时否出现接收超时就重连
const char* const HTSC_INSIGHT_CONFIG_ETIME_RECONNECT = "etime_reconnect";

//=======================================================================================================
//  默认参数定义
//=======================================================================================================
//连接等待超时时间，单位：秒
const int CONNECT_WAIT_TIME = 30;
//每次发送等待超时时间，单位:秒
const int SEND_WAIT_TIMEOUT = 30;
//每次接收等待超时时间，单位：秒
const int RECV_WAIT_TIMEOUT = 30;
//单次接收(reveive)部分数据最大等待时间，单位：秒
const int RECV_N_TIMEOUT = 3;
//服务发现链路等待超时时间，单位：秒
const int DISCOVERY_SERVER_WAIT_TIMEOUT = 30;
//心跳等待时间，单位:秒
const int HEARTBEAT_SLEEP_TIME = 3;
//服务端心跳失效时间，单位:秒
const int SERVER_HEARTBEAT_TIMEOUT = 60;
//订阅等待应答时间，单位：秒
const int SUBSCRIBE_RESPONSE_MESSAGE_WAIT_TIME = 30;
//订阅重试次数
const int SUBSCRIBE_TRY_COUNT = 10;
//MDQuery请求等待应答时间，单位：秒
const int MDQUERY_RESPONSE_MESSAGE_WAIT_TIME = 30;
//Playback请求等待应答时间，单位：秒
const int PLAYBACK_RESPONSE_MESSAGE_WAIT_TIME = 30;
//处理线程池线程初始线程数
const int THREAD_POOL_INIT_SIZE = 5;
//处理线程池最大线程数
const int THREAD_POOL_MAX_SIZE = 100;
//每次获取的最大消息数
const int MAX_FETCH_MESSAGE_COUNT = 10;
//收到回放状态为结束的报文等待payload处理完的时间，单位：秒
const int PLAYBACK_END_STATUS_WAIT_PAYLOAD_TIME = 3;
//线程中sleep时间长度，单位：微秒
const int THREAD_SLEEP_TIME = 100;//0.1ms
//流量日志输出间隔数据条数
const int TRAFFIC_CHECK_GAP = 10000;
//监测无数据最大容忍时间，单位：毫秒
const int NODATA_CHECK_GAP = 10000;//10s
//心跳日志开关
const int HEARTBEAT_TRACE = 1;
//流量日志开关
const int GLOBAL_TRACE = 1;
//解压缩线程开关
const int COMPRESS_SWITCH = 1;
//回复控制开关，打开时回复通过回调返回，调用者处理回复消息
const int RESPONSE_CALLBACK = 0;
//登录数据网关尝试次数
const int LOGIN_TRY_COUNT = 5;
//登录服务发现网关尝试次数
const int LOGIN_DISCOVERY_TRY_COUNT = 5;
//OnMarketdata回调函数中数据禁止析构
const int FORBID_DESTRUCT = 0;
//接收数据时否出现接收超时就重连
const int ETIME_RECONNECT = 0;

} //gateway
} //mdc
} //htsc
} //com

#endif //HTSC_MD_GATEWAY_CLIENT_PARAMETER_DEFINE_H