//======================================================================================================
// base_define.h
// 导出定义、日志函数等常用函数定义头文件
// 版本：v1.0  
// lujun
//======================================================================================================

#ifndef HTSC_MD_GATEWAY_CLIENT_BASE_DEFINE_H
#define HTSC_MD_GATEWAY_CLIENT_BASE_DEFINE_H

#include <string>
#include <stdint.h>

#ifdef USE_STATIC_LIB
#define LIB_EXPORT
#elif defined(WIN32) || defined(WIN64) || defined(_WINDOWS)
#ifdef MD_GATEWAY_CLIENT_EXPORT_FLAG
#define LIB_EXPORT __declspec(dllexport)
#else
#define LIB_EXPORT __declspec(dllimport)
#endif
#else
#define LIB_EXPORT
#endif

#define NAMESPACE_BEGIN \
namespace com { \
namespace htsc {  \
namespace mdc { \
namespace gateway {

#define NAMESPACE_END \
} \
} \
} \
} 

//禁用拷贝构造函数和赋值操作
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&);                \
    TypeName& operator=(const TypeName&)

//定义单个对象及对象数组释放宏
#define DELETE_P(buf) if (buf) { delete buf; buf = NULL;}
#define DELETE_P_ARRAY(buf) if (buf) { delete[] buf; buf = NULL;}

NAMESPACE_BEGIN
//messageClassification消息分类 
const int MESSAGE_CLASSIFICATION_SYSTEM = 1001;	   //系统消息分类	心跳消息，登录消息，通用警告消息
const int MESSAGE_CLASSIFICATION_REALTIME = 2001;	   //实时行情消息分类	实时行情相关的消息
const int MESSAGE_CLASSIFICATION_PLAYBACK = 2002;	   //回放任务消息分类	回放相关的消息
const int MESSAGE_CLASSIFICATION_QUERY = 2003;	   //查询消息分类	查询项的消息

//queryType说明
const int QUERY_TYPE_BASE_INFORMATION = 1011;        //查询证券基础信息	包含到当日为止行情中心对应类型的所以基础信息，不需要特别的QueryParam
const int QUERY_TYPE_LATEST_BASE_INFORMATION = 1012; //查询最新证券基础信息	仅包含当日有的证券基础信息，不需要特别的QueryParam
const int QUERY_TYPE_ETF_BASE_INFORMATION = 1013;    //查询ETF的基础信息
const int QUERY_TYPE_STATUS = 1021;	                 //查询证券最新状态	不需要特别的QueryParam
const int QUERY_TYPE_SUBSCRIBE_ANSWER = 1031;        //订阅的返回值

//appType说明
const int APP_TYPE_INSIGHT = 101;	//insight自有用户
const int APP_TYPE_XTRADER = 201;	//XTrader系统

/**
* 环境初始化、关闭函数
* 对于MFC用户(非main函数入口)，需要在入口起始和结尾处调用InitEnv和FiniEnv，用于初始化ACE环境
*/
LIB_EXPORT void init_env();
LIB_EXPORT void fini_env();

/**
* 打开关闭g_trace日志
*/
LIB_EXPORT void open_trace();
LIB_EXPORT void close_trace();
LIB_EXPORT bool is_trace();

/**
* 打开关闭g_heartbeat_trace日志
*/
LIB_EXPORT void open_heartbeat_trace();
LIB_EXPORT void close_heartbeat_trace();
LIB_EXPORT bool is_heartbeat_trace();

/**
* 根据是否为压缩数据选择线程
*/
LIB_EXPORT void open_compress();
LIB_EXPORT void close_compress();
LIB_EXPORT bool is_compress();

/**
* 打开关闭g_response_callback开关
* 打开g_response_callback开关后，需要自行处理response消息
*/
LIB_EXPORT void open_response_callback();
LIB_EXPORT void close_response_callback();
LIB_EXPORT bool is_response_callback();

/**
* 服务网关地址自动选择开关，open:根据服务端配置选择，close:根据客户端配置选择
*/
LIB_EXPORT void open_node_auto();
LIB_EXPORT void close_node_auto();
LIB_EXPORT bool is_node_auto();

/**
* 判断是否禁止析构
*/
LIB_EXPORT bool is_forbid_destruct();


/**
* 判断是否出现接收超时就重连
*/
LIB_EXPORT bool is_etime_reconnect();

LIB_EXPORT void open_bind_port(uint32_t start_port, uint32_t end_port);
LIB_EXPORT void close_bind_port();
LIB_EXPORT bool is_bind_port();
LIB_EXPORT uint32_t get_start_port();
LIB_EXPORT uint32_t get_end_port();

/**
* 日志级别定义
*/
typedef enum mdc_log_severity {
	MDC_LOG_SEVERITY_DEBUG,
	MDC_LOG_SEVERITY_WARNING,
	MDC_LOG_SEVERITY_ERROR
} mdc_log_severity;

//打开、关闭日志到文件，日志文件存放在命令运行目录，格式为：pid_进程id.log
LIB_EXPORT void open_file_log();
LIB_EXPORT void close_file_log();

//打开、关闭日志到标准输出
LIB_EXPORT void open_cout_log();
LIB_EXPORT void close_cout_log();

/**
* 打印日志，默认打印到日志和标准输出中
* 如果需要控制，使用随后的open/close接口
* 使用方式：与printf类似，仅支持%s、%d等常用类型
*/
LIB_EXPORT void debug_print(const char* format, ...);
LIB_EXPORT void warning_print(const char* format, ...);
LIB_EXPORT void error_print(const char* format, ...);

/**
* 获取错误号含义
* @param[in] code 错误号
* @return 错误含义
*/
LIB_EXPORT std::string get_error_code_value(int code);

/**
* 获取动态库版本，最长15个字符
*/
LIB_EXPORT const char* get_dll_version();

/**
* 设置动态库版本
*/
LIB_EXPORT void set_dll_version(const char* version);

/**
* 获取taskid，规则为：节点名+pid+当前时间（精确到微秒）+自定义内容后缀
* 如：hostname_13323_20170728T1500.0132_suffix
* @param[in] suffix 后缀
* @return 任务id字符串
*/
LIB_EXPORT std::string get_task_id(const std::string& suffix = "");

/**
* 将utf8编码的字符串转换为gb2312的字符串
* @param[in] str utf8编码的字符串
* @return gb2312的字符串
*/
LIB_EXPORT std::string utf8_to_gb2312(const std::string& str);
/**
* 将gb2312编码的buf转换为utf8的字符串
* @param[in] str gb2312编码的字符串
* @return utf8的字符串
*/
LIB_EXPORT std::string gb2312_to_utf8(const std::string& str);

//属性获取及设置，必须调用init_env初始化
LIB_EXPORT int get_int_property_value(const char* key);
LIB_EXPORT int set_int_property_value(const char* key, int value);

/**
* AES加密，块大小必须为128位（16字节）
* 如果不是，则要补齐，密钥长度可以选择128位、192位、256位。
* @param[in] key 密钥
* @param[in] data 数据
*/
LIB_EXPORT std::string encode(const std::string& key, const std::string& data);

/**
* AES解密，密钥长度可以选择128位、192位、256位。
* @param[in] key 密钥
* @param[in] encode_data 数据
*/
LIB_EXPORT std::string decode(const std::string& key, const std::string& encode_data);

/**
* 删除地址映射表中的某个地址
* @param[in] original_ip 待删除的原地址
*/
LIB_EXPORT void delete_ip_map(std::string original_ip);

/**
* 新增地址映射表中的映射逻辑
* @param[in] original_ip 原地址
* @param[in] mapped_ip   映射地址
*/
LIB_EXPORT void add_ip_map(std::string original_ip, std::string mapped_ip);

/**
* 删除端口映射表中的某个端口
* @param[in] original_port 待删除的原端口
*/
LIB_EXPORT void delete_port_map(int original_port);

/**
* 新增端口映射表中的映射逻辑
* @param[in] original_port 原端口
* @param[in] mapped_port   映射端口
*/
LIB_EXPORT void add_port_map(int original_port, int mapped_port);

NAMESPACE_END

#endif //HTSC_MD_GATEWAY_CLIENT_BASE_DEFINE_H