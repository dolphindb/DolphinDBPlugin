//======================================================================================================
// message_handle.h
// 消息处理handle头文件
// panhao 2019-01-16
// 版本：
//    v0.1实现handle定义类 
//    v1.1.2 补充OnLoginSuccess、OnLoginFailed、OnReconnect回调函数，用于处理登录和重连时的通知
//    v2.1.0 补充OnPlaybackResponse、OnPlaybackControlResponse
//======================================================================================================

#ifndef HTSC_MD_GATEWAY_MESSAGE_HANDLE_H
#define HTSC_MD_GATEWAY_MESSAGE_HANDLE_H


#include "base_define.h"
#include "MDSubscribe.pb.h"
#include "MessageHeader.pb.h"
#include "MessageBody.pb.h"

NAMESPACE_BEGIN

/**
* 消息处理定义类
* 请根据需求实现下列虚函数，默认方式是不做任何处理
* 要求支持多线程并发，不要使用对象成员
*/
class LIB_EXPORT MessageHandle {
public:
	MessageHandle();
	virtual ~MessageHandle();

public:
	/**
	* 处理InsightMessage
	* @param[in] header 消息头
	* @param[in] body 消息体
	*/
	void ProcessMessage(
		const com::htsc::mdc::insight::model::MessageHeader* header,
		const com::htsc::mdc::insight::model::MessageBody* body);

	//=============================================================================================
	// 以下为业务逻辑处理回调函数
	//=============================================================================================

	/**
	* 处理MDSecurityRecord
	* @param[in] MarketData 数据体
	*/
	virtual void OnMarketData(const com::htsc::mdc::insight::model::MarketData& data);

	/**
	* 处理回放消息
	* @param[in] PlaybackPayload 回放数据
	*/
	virtual void OnPlaybackPayload(const com::htsc::mdc::insight::model::PlaybackPayload& payload);

	/**
	* 处理回放状态消息
	* @param[in] PlaybackStatus 回放状态
	*/
	virtual void OnPlaybackStatus(const com::htsc::mdc::insight::model::PlaybackStatus& status);

	/**
	* 处理回放请求返回结果
	* @param[in] PlaybackResponse 回放请求结果
	*/
	virtual void OnPlaybackResponse(const com::htsc::mdc::insight::model::PlaybackResponse& response);

	/**
	* 处理回放控制请求返回结果
	* @param[in] PlaybackControlResponse 回放控制请求结果
	*/
	virtual void OnPlaybackControlResponse(const com::htsc::mdc::insight::model::PlaybackControlResponse& control_response);

	/**
	* 处理证券最新状态返回结果，订阅后返回
	* @param[in] MarketDataStream 状态数据
	*/
	virtual void OnServiceMessage(const ::com::htsc::mdc::insight::model::MarketDataStream& data_stream);

	/**
	* 处理订阅请求返回结果
	* @param[in] MDSubscribeResponse 订阅请求结果
	*/
	virtual void OnSubscribeResponse(const ::com::htsc::mdc::insight::model::MDSubscribeResponse& response);

	/**
	* 处理查询请求返回结果
	* @param[in] MDQueryResponse 查询请求返回结果
	*/
	virtual void OnQueryResponse(const ::com::htsc::mdc::insight::model::MDQueryResponse& response);

	/**
	* 处理告警消息
	* 为避免影响后续处理逻辑，请不要在此处进行复杂的业务逻辑
	* @param[in] context 错误内容
	*/
	virtual void OnGeneralError(const com::htsc::mdc::insight::model::InsightErrorContext& context);

	//=============================================================================================
	// 以下为状态信息处理回调函数
	// 主要为影响业务逻辑的重连提供入口
	//=============================================================================================
	/**
	* 登录成功回调，在重连成功时会触发
	* 本函数在登录成功后回调，为避免影响后续处理逻辑，请不要在此处进行复杂的业务逻辑
	* @param[in] server_name 服务
	*/
	virtual void OnLoginSuccess();

	/**
	* 登录失败，在重连时会多次触发
	* 本函数在登录失败后回调，为避免影响后续处理逻辑，请不要在此处进行复杂的业务逻辑
	* 常见的处理方式是记录下登录错误信息，检查登录验证信息正确性后，在外部关闭客户端并进行重连
	* @param[in] error_no 错误号
	* @param[in] message 错误消息
	*/
	virtual void OnLoginFailed(int error_no, const std::string& message);


	/**
	* 在重连失败时回调，表示所有服务器都无法连接
	* 仅通知，不要在此处对Mdc客户端对象的进行处理
	* 建议的使用方式：在进行回放操作时记录OnReconnect的发生时间，如果在回访操作进行时，则本次回放失败。
	*/
	virtual void OnNoConnections();

	/**
	* 重连成功时，为避免影响后续处理逻辑，请不要在此处进行复杂的业务逻辑处理
	* 建议的使用方式：在进行回放操作时记录OnReconnect发生时间，如果此时正在回放，则回放应失败（回放不支持续传）。
	*/
	virtual void OnReconnect();

};

NAMESPACE_END

#endif //HTSC_MD_GATEWAY_MESSAGE_HANDLE_H