//======================================================================================================
// client_interface.h
// 客户端接口定义头文件
// 版本：v1.0 20170821
// lujun
//======================================================================================================

#ifndef HTSC_MDC_GATEWAY_CLIENT_INTERFACE_H
#define HTSC_MDC_GATEWAY_CLIENT_INTERFACE_H

#include <string>
#include <vector>

#include "MDSubscribe.pb.h"
#include "MDPlayback.pb.h"
#include "MDQuery.pb.h"

#include "base_define.h"
#include "log_interface.h"

NAMESPACE_BEGIN

class MessageHandle;

/**
* gateway客户端接口类
*/
class LIB_EXPORT ClientInterface {
public:
	ClientInterface() {}
	virtual ~ClientInterface() {};

public:
	/**
	* 注册处理对象，必须在login之前设置
	* @param[in] handle 处理对象
	* @param[in] env 环境
	*/
	virtual void RegistHandle(MessageHandle* handle) = 0;

	/**
	* 注册日志回调接口
	*/
	virtual void RegistLogHandle(LogInterface* handle) = 0;

	/**
	* 使用服务发现方式，通过网关获取服务列表进行链接，按照增加的先后顺序连接
	* 本函数禁止多线程并发调用
	* @param[in] ip 服务网关地址
	* @param[in] port 服务网关端口
	* @param[in] user 用户名
	* @param[in] value 登录值（密码或token)
	* @param[in] is_token 是否为token true token false 密码
	* @return 0 成功 < 0 失败，错误信息见错误号
	*/
	virtual int LoginByServiceDiscovery(const std::string& ip, int port,
		const std::string& user, const std::string& value, bool is_token) = 0;

	/**
	* 使用服务发现方式，通过网关获取服务列表进行链接，按照增加的先后顺序连接
	* @param[in] ip 服务网关地址
	* @param[in] port 服务网关端口
	* @param[in] user 用户名
	* @param[in] value 登录值（密码或token)
	* @param[in] password_is_token 是否为token
	* @param[in] backup_list 服务网关备选地址列表
	* @return 0 成功 < 0 失败，错误信息见错误号
	*/
	virtual int LoginByServiceDiscovery(const std::string& ip, int port,
		const std::string& user, const std::string& value, bool password_is_token,
		std::vector<std::string>backup_list) = 0;

	/**
	* 订阅全市场数据，本函数禁止多线程并发调用
	* @input[in] action_type 操作类型
	* @input[in] data_types 数据类型列表，由调用方释放
	* @return 0 成功 < 0 错误号
	*/
	virtual int SubscribeAll(
		::com::htsc::mdc::insight::model::ESubscribeActionType action_type,
		::com::htsc::mdc::insight::model::SubscribeAll* subscribe_all) = 0;

	/**
	* 根据证券类型订阅，本函数禁止多线程并发调用
	* @input[in] action_type 操作类型
	* @input[in] type_details 类型列表，由调用方释放
	* @return 0 成功 < 0 错误号
	*/
	virtual int SubscribeBySourceType(
		::com::htsc::mdc::insight::model::ESubscribeActionType action_type,
		::com::htsc::mdc::insight::model::SubscribeBySourceType* source_type) = 0;

	/**
	* 根据证券编号订阅，本函数禁止多线程并发调用
	* @input[in] action_type 操作类型
	* @input[in] id 订阅id指针，由调用方释放
	* @return 0 成功 < 0 错误号
	*/
	virtual int SubscribeByID(
		::com::htsc::mdc::insight::model::ESubscribeActionType action_type,
		::com::htsc::mdc::insight::model::SubscribeByID* id) = 0;

	/**
	* 订阅数据，本函数禁止多线程并发调用
	* @param[in] sub_request 订阅请求，由调用方释放
	* @return 0 成功 < 0 错误号
	*/
	virtual int Subscribe(::com::htsc::mdc::insight::model::MDSubscribeRequest* request) = 0;

	/**
	* 请求回放，本函数禁止多线程并发调用
	* @param[in] request 已分配好空间的回放请求，由调用方释放
	* @return 0 成功 < 0 错误号
	*/
	virtual int RequestPlayback(::com::htsc::mdc::insight::model::PlaybackRequest* request) = 0;

	/**
	* 请求回放
	* @param[in] idList 回放的证券列表
	* @param[in] startTime 回放的开始时间
	* @param[in] endTime 回放的结束时间
	* @param[in] replayDataType 回放的数据类型
	* @param[in] exrightsType 除复权类型
	* @param[in] sortByMDTime = true 按照MDTime排序, = false 按照RecivedTime排序
	* @return 0 成功 < 0 错误号
	*/
	virtual int RequestPlayback(std::vector<std::string> idList, std::string startTime,
		std::string endTime, ::com::htsc::mdc::insight::model::EMarketDataType replayDataType,
		::com::htsc::mdc::insight::model::EPlaybackExrightsType exrightsType, bool sortByMDTime = true) = 0;

	/**
	* 行情查询请求，本函数禁止多线程并发调用，回复数据通过引用获得，需要关闭response_callback开关
	* @param[in] request 已分配好空间的查询请求，由调用方释放
	* @param[out] responses 内部分配空间，由调用方调用ReleaseMdQueryResponses释放
	* @return 0 成功 < 0 错误号
	*/
	virtual int RequestMDQuery(::com::htsc::mdc::insight::model::MDQueryRequest* request,
		std::vector< ::com::htsc::mdc::insight::model::MDQueryResponse* >*& mdresponses) = 0;

	/**
	* 行情查询请求，本函数禁止多线程并发调用，回复数据通过回调接口获得，需要打开response_callback开关
	* @param[in] request 已分配好空间的查询请求，由调用方释放
	* @return 0 成功 < 0 错误号
	*/
	virtual int RequestMDQuery(::com::htsc::mdc::insight::model::MDQueryRequest* request) = 0;

	/**
	* 释放行情查询结果
	* @param[in] mdresponses 查询结果
	*/
	virtual void ReleaseQueryResult(
		std::vector< ::com::htsc::mdc::insight::model::MDQueryResponse* >*& mdresponses);

	/**
	* 关闭，等待线程退出
	* @param[in] clear_members 是否清理成员，在reconnect时不能清理
	* @param[in] wait_threads_quit 等待线程退出
	*/
	virtual void Close() = 0;

	/**
	* 设置处理线程池线程数，在登录之前调用
	* 当消息的处理逻辑比较复杂，需要更多线程并发处理时，
	* 可以在调用LoginByServiceDiscovery登录服务端之前设置
	* @param[in] count 线程数
	*/
	virtual void set_handle_pool_thread_count(short count) = 0;

	/**
	* 获得处理线程数
	* @return 线程数
	*/
	virtual short handle_pool_thread_count() const = 0;


	virtual void clear_expire_message() = 0;
};

NAMESPACE_END

#endif //HTSC_MD_GATEWAY_CLIENT_INTERFACE_H