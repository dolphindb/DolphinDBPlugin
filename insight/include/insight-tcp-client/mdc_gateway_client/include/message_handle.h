//======================================================================================================
// message_handle.h
// ��Ϣ����handleͷ�ļ�
// panhao 2019-01-16
// �汾��
//    v0.1ʵ��handle������ 
//    v1.1.2 ����OnLoginSuccess��OnLoginFailed��OnReconnect�ص����������ڴ����¼������ʱ��֪ͨ
//    v2.1.0 ����OnPlaybackResponse��OnPlaybackControlResponse
//======================================================================================================

#ifndef HTSC_MD_GATEWAY_MESSAGE_HANDLE_H
#define HTSC_MD_GATEWAY_MESSAGE_HANDLE_H


#include "base_define.h"
#include "MDSubscribe.pb.h"
#include "MessageHeader.pb.h"
#include "MessageBody.pb.h"

NAMESPACE_BEGIN

/**
* ��Ϣ��������
* ���������ʵ�������麯����Ĭ�Ϸ�ʽ�ǲ����κδ���
* Ҫ��֧�ֶ��̲߳�������Ҫʹ�ö����Ա
*/
class LIB_EXPORT MessageHandle {
public:
	MessageHandle();
	virtual ~MessageHandle();

public:
	/**
	* ����InsightMessage
	* @param[in] header ��Ϣͷ
	* @param[in] body ��Ϣ��
	*/
	void ProcessMessage(
		const com::htsc::mdc::insight::model::MessageHeader* header,
		const com::htsc::mdc::insight::model::MessageBody* body);

	//=============================================================================================
	// ����Ϊҵ���߼�����ص�����
	//=============================================================================================

	/**
	* ����MDSecurityRecord
	* @param[in] MarketData ������
	*/
	virtual void OnMarketData(const com::htsc::mdc::insight::model::MarketData& data);

	/**
	* ����ط���Ϣ
	* @param[in] PlaybackPayload �ط�����
	*/
	virtual void OnPlaybackPayload(const com::htsc::mdc::insight::model::PlaybackPayload& payload);

	/**
	* ����ط�״̬��Ϣ
	* @param[in] PlaybackStatus �ط�״̬
	*/
	virtual void OnPlaybackStatus(const com::htsc::mdc::insight::model::PlaybackStatus& status);

	/**
	* ����ط����󷵻ؽ��
	* @param[in] PlaybackResponse �ط�������
	*/
	virtual void OnPlaybackResponse(const com::htsc::mdc::insight::model::PlaybackResponse& response);

	/**
	* ����طſ������󷵻ؽ��
	* @param[in] PlaybackControlResponse �طſ���������
	*/
	virtual void OnPlaybackControlResponse(const com::htsc::mdc::insight::model::PlaybackControlResponse& control_response);

	/**
	* ����֤ȯ����״̬���ؽ�������ĺ󷵻�
	* @param[in] MarketDataStream ״̬����
	*/
	virtual void OnServiceMessage(const ::com::htsc::mdc::insight::model::MarketDataStream& data_stream);

	/**
	* ���������󷵻ؽ��
	* @param[in] MDSubscribeResponse ����������
	*/
	virtual void OnSubscribeResponse(const ::com::htsc::mdc::insight::model::MDSubscribeResponse& response);

	/**
	* �����ѯ���󷵻ؽ��
	* @param[in] MDQueryResponse ��ѯ���󷵻ؽ��
	*/
	virtual void OnQueryResponse(const ::com::htsc::mdc::insight::model::MDQueryResponse& response);

	/**
	* ����澯��Ϣ
	* Ϊ����Ӱ����������߼����벻Ҫ�ڴ˴����и��ӵ�ҵ���߼�
	* @param[in] context ��������
	*/
	virtual void OnGeneralError(const com::htsc::mdc::insight::model::InsightErrorContext& context);

	//=============================================================================================
	// ����Ϊ״̬��Ϣ����ص�����
	// ��ҪΪӰ��ҵ���߼��������ṩ���
	//=============================================================================================
	/**
	* ��¼�ɹ��ص����������ɹ�ʱ�ᴥ��
	* �������ڵ�¼�ɹ���ص���Ϊ����Ӱ����������߼����벻Ҫ�ڴ˴����и��ӵ�ҵ���߼�
	* @param[in] server_name ����
	*/
	virtual void OnLoginSuccess();

	/**
	* ��¼ʧ�ܣ�������ʱ���δ���
	* �������ڵ�¼ʧ�ܺ�ص���Ϊ����Ӱ����������߼����벻Ҫ�ڴ˴����и��ӵ�ҵ���߼�
	* �����Ĵ���ʽ�Ǽ�¼�µ�¼������Ϣ������¼��֤��Ϣ��ȷ�Ժ����ⲿ�رտͻ��˲���������
	* @param[in] error_no �����
	* @param[in] message ������Ϣ
	*/
	virtual void OnLoginFailed(int error_no, const std::string& message);


	/**
	* ������ʧ��ʱ�ص�����ʾ���з��������޷�����
	* ��֪ͨ����Ҫ�ڴ˴���Mdc�ͻ��˶���Ľ��д���
	* �����ʹ�÷�ʽ���ڽ��лطŲ���ʱ��¼OnReconnect�ķ���ʱ�䣬����ڻطò�������ʱ���򱾴λط�ʧ�ܡ�
	*/
	virtual void OnNoConnections();

	/**
	* �����ɹ�ʱ��Ϊ����Ӱ����������߼����벻Ҫ�ڴ˴����и��ӵ�ҵ���߼�����
	* �����ʹ�÷�ʽ���ڽ��лطŲ���ʱ��¼OnReconnect����ʱ�䣬�����ʱ���ڻطţ���ط�Ӧʧ�ܣ��طŲ�֧����������
	*/
	virtual void OnReconnect();

};

NAMESPACE_END

#endif //HTSC_MD_GATEWAY_MESSAGE_HANDLE_H