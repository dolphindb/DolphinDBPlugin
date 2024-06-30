//======================================================================================================
// error_define.h
// 错误号定义头文件
// 版本：v1.0 增加错误含义注释
// lujun
//======================================================================================================

#ifndef HTSC_MD_GATEWAY_CLIENT_ERROR_DEFINE_H
#define HTSC_MD_GATEWAY_CLIENT_ERROR_DEFINE_H

namespace com {
namespace htsc {
namespace mdc {
namespace gateway {

//============================================================
//错误号定义
//============================================================
const int CONNECT_TO_SERVER_FAILED = -1000;                 //连接服务器失败，网络不通
const int SERVER_REJECT_LOGIN = -1001;                      //服务器拒绝登录
const int INVALID_MESSAGE_HANDLE = -1002;                   //无效的消息处理对象，需要在登录前设置
const int STREAM_SEND_ERROR = -1003;                        //链路故障，发送报文失败
const int STREAM_RECV_ERROR = -1004;                        //链路故障，接收报文失败
const int LOGIN_WAIT_TIMEOUT = -1005;                       //登录超时
const int INVALID_LOGIN_RESPONSE_MESSAGE = -1006;           //无效的登录应答消息，内部错误
const int LOGIN_ALL_SERVERS_FAILED = -1007;                 //所有服务器登录失败
const int SERVICE_DISCOVERY_RESPONSE_INVALID = -1008;       //无效的服务发现应答消息
const int SERVICE_DISCOVERY_RESPONSE_FAILURE = -1009;       //服务发现应答结果为：失败
const int VALID_SERVER_NOT_EXIST = -1010;                   //不存在可登录的服务器
const int STREAM_INVALID = -1011;                           //链接无效，正在重连
const int SEND_WAIT_TIMEOUT_ERROR = -1012;                  //请求发送超时
const int RECEIVE_WAIT_TIMEOUT_ERROR = -1013;               //接收超时

const int INVALID_INPUT_PORT = -1020;                       //无效的输入端口号，输入检查时报告
const int INVALID_INPUT_USER = -1021;                       //无效的输入用户名，输入检查检测时报告
const int INVALID_INPUT_IP = -1022;                         //无效的ip，输入检查检测时报告
const int INVALID_CLIENT = -1023;                           //无效的客户端

const int START_MAINTAIN_THREAD_FAILED = -2100;             //启动链接对象维护变成失败
const int START_HEARTBEAT_THREAD_FAILED = -2101;            //启动心跳线程失败
const int START_MESSAGE_THREAD_FAILED = -2102;              //启动消息处理线程失败
const int START_HANDLE_THREAD_FAILED = -2103;               //启动handle处理线程失败
const int STOP_LOGIN_FOR_QUIT = -2104;                      //客户端退出，停止登录
const int ACQUIRE_QUIT_MUTEX_FAILED = -2105;                //请求锁失败，内部错误
const int GET_MESSAGE_FROM_QUEUE_TIMEOUT = -2106;           //获取消息超时
const int SUBSCRIBE_RESPONSE_REJECT = -2107;                //订阅应答结果为拒绝
const int SUBSCRIBE_RESPONSE_HEADER_ID_NOT_EQUAL = -2108;   //订阅应答消息头不一致，错位的订阅应答，丢弃
const int INVALID_EMDC_MESSAGE_TYPE = -2109;                //无效的EMDC消息类型
const int INVALID_PACKAGE_SIZE = -2110;                     //无效的包长度
const int NULL_MESSAGE_POINTER = -2111;                     //消息指针为NULL，内部异常
const int INVALID_INSIGHT_MESSAGE_HEADER = -2112;           //无效的消息头
const int INVALID_INSIGHT_MESSAGE_BODY = -2113;             //无效的消息体 
const int SERIALIZE_MESSAGE_HEADER_TO_ARRAY_FAILED = -2114; //序列化消息头失败
const int SERIALIZE_MESSAGE_BODY_TO_ARRAY_FAILED = -2115;   //序列化消息体失败
const int PARSE_MESSAGE_HEADER_FROM_ARRAY_FAILED = -2116;   //从数组中解析消息头失败
const int PARSE_MESSAGE_BODY_FROM_ARRAY_FAILED = -2117;     //从数组中解析消息体失败
const int INVALID_INSIGHT_MESSAGE_BUF = -2118;              //无效的消息缓冲
const int OUT_OF_MEMORY = -2120;                            //内存不足
const int INVALID_SUBSCRIBE_INPUT = -2121;                  //无效的订阅输入
const int SUBSCRIBE_FAILED = -2122;                         //订阅失败
const int CLIENT_IS_ALREADY_LOGIN = -2300;                  //已登录不能重复登录
const int WAIT_SUBSCRIBE_RESPONSE_TIMEOUT = -2301;          //等待订阅应答超时
const int INVALID_MDQUERY_RESPONSE_MESSAGE = -2302;         //无效的查询应答消息
const int WAIT_PLAYBACK_RESPONSE_TIMEOUT = -2303;           //等待回放应答超时
const int WAIT_MDQUERY_RESPONSE_TIMEOUT = -2304;            //等待查询应答失败
const int PLAYBACK_RESPONSE_REJECT = -2305;                 //回放应答结果为：拒绝
const int MDQUERY_RESPONSE_ERROR = -2306;                   //查询应答错误
const int MDQUERY_RESPONSE_FAILURE = -2307;                 //查询结果为失败
const int ACQUIRE_SUBSCRIBE_MUTEX_FAILED = -2308;           //请求订阅锁失败，内部错误
const int INVALID_SUBSCRIBE_MESSAGE_BODY = -2309;           //订阅消息体无效

const int INIT_SSL_CONTEXT_FAILED = -2400;                  //初始化SSL环境错误
const int SSL_VERIFY_PRIVATE_KEY_FAILED = -2401;            //SSL验证key失败
const int PLAYBACK_CONTROL_RESPONSE_REJECT = -2402;         //回放控制应答结果为：拒绝
const int QUERY_RESPONSE_CITATION_REJECT = -2403;           //选择回调函数处理查询回复，却使用引用返回回复
const int QUERY_RESPONSE_CALLBACK_REJECT = -2404;           //未选择回调函数处理查询回复，却未使用应用返回回复

const int UNKNOWN_PROPERTY_NAME = -2501;                    //未知的参数名

} //gateway
} //mdc
} //htsc
} //com
#endif //HTSC_MD_GATEWAY_CLIENT_ERROR_DEFINE_H