//======================================================================================================
// client_interface.h
// �ͻ��˽ӿڶ���ͷ�ļ�
// �汾��v1.0 20170821
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
* gateway�ͻ��˽ӿ���
*/
class LIB_EXPORT ClientInterface {
public:
	ClientInterface() {}
	virtual ~ClientInterface() {};

public:
	/**
	* ע�ᴦ����󣬱�����login֮ǰ����
	* @param[in] handle �������
	* @param[in] env ����
	*/
	virtual void RegistHandle(MessageHandle* handle) = 0;

	/**
	* ע����־�ص��ӿ�
	*/
	virtual void RegistLogHandle(LogInterface* handle) = 0;

	/**
	* ʹ�÷����ַ�ʽ��ͨ�����ػ�ȡ�����б�������ӣ��������ӵ��Ⱥ�˳������
	* ��������ֹ���̲߳�������
	* @param[in] ip �������ص�ַ
	* @param[in] port �������ض˿�
	* @param[in] user �û���
	* @param[in] value ��¼ֵ�������token)
	* @param[in] is_token �Ƿ�Ϊtoken true token false ����
	* @return 0 �ɹ� < 0 ʧ�ܣ�������Ϣ�������
	*/
	virtual int LoginByServiceDiscovery(const std::string& ip, int port,
		const std::string& user, const std::string& value, bool is_token) = 0;

	/**
	* ʹ�÷����ַ�ʽ��ͨ�����ػ�ȡ�����б�������ӣ��������ӵ��Ⱥ�˳������
	* @param[in] ip �������ص�ַ
	* @param[in] port �������ض˿�
	* @param[in] user �û���
	* @param[in] value ��¼ֵ�������token)
	* @param[in] password_is_token �Ƿ�Ϊtoken
	* @param[in] backup_list �������ر�ѡ��ַ�б�
	* @return 0 �ɹ� < 0 ʧ�ܣ�������Ϣ�������
	*/
	virtual int LoginByServiceDiscovery(const std::string& ip, int port,
		const std::string& user, const std::string& value, bool password_is_token,
		std::vector<std::string>backup_list) = 0;

	/**
	* ����ȫ�г����ݣ���������ֹ���̲߳�������
	* @input[in] action_type ��������
	* @input[in] data_types ���������б��ɵ��÷��ͷ�
	* @return 0 �ɹ� < 0 �����
	*/
	virtual int SubscribeAll(
		::com::htsc::mdc::insight::model::ESubscribeActionType action_type,
		::com::htsc::mdc::insight::model::SubscribeAll* subscribe_all) = 0;

	/**
	* ����֤ȯ���Ͷ��ģ���������ֹ���̲߳�������
	* @input[in] action_type ��������
	* @input[in] type_details �����б��ɵ��÷��ͷ�
	* @return 0 �ɹ� < 0 �����
	*/
	virtual int SubscribeBySourceType(
		::com::htsc::mdc::insight::model::ESubscribeActionType action_type,
		::com::htsc::mdc::insight::model::SubscribeBySourceType* source_type) = 0;

	/**
	* ����֤ȯ��Ŷ��ģ���������ֹ���̲߳�������
	* @input[in] action_type ��������
	* @input[in] id ����idָ�룬�ɵ��÷��ͷ�
	* @return 0 �ɹ� < 0 �����
	*/
	virtual int SubscribeByID(
		::com::htsc::mdc::insight::model::ESubscribeActionType action_type,
		::com::htsc::mdc::insight::model::SubscribeByID* id) = 0;

	/**
	* �������ݣ���������ֹ���̲߳�������
	* @param[in] sub_request ���������ɵ��÷��ͷ�
	* @return 0 �ɹ� < 0 �����
	*/
	virtual int Subscribe(::com::htsc::mdc::insight::model::MDSubscribeRequest* request) = 0;

	/**
	* ����طţ���������ֹ���̲߳�������
	* @param[in] request �ѷ���ÿռ�Ļط������ɵ��÷��ͷ�
	* @return 0 �ɹ� < 0 �����
	*/
	virtual int RequestPlayback(::com::htsc::mdc::insight::model::PlaybackRequest* request) = 0;

	/**
	* ����ط�
	* @param[in] idList �طŵ�֤ȯ�б�
	* @param[in] startTime �طŵĿ�ʼʱ��
	* @param[in] endTime �طŵĽ���ʱ��
	* @param[in] replayDataType �طŵ���������
	* @param[in] exrightsType ����Ȩ����
	* @param[in] sortByMDTime = true ����MDTime����, = false ����RecivedTime����
	* @return 0 �ɹ� < 0 �����
	*/
	virtual int RequestPlayback(std::vector<std::string> idList, std::string startTime,
		std::string endTime, ::com::htsc::mdc::insight::model::EMarketDataType replayDataType,
		::com::htsc::mdc::insight::model::EPlaybackExrightsType exrightsType, bool sortByMDTime = true) = 0;

	/**
	* �����ѯ���󣬱�������ֹ���̲߳������ã��ظ�����ͨ�����û�ã���Ҫ�ر�response_callback����
	* @param[in] request �ѷ���ÿռ�Ĳ�ѯ�����ɵ��÷��ͷ�
	* @param[out] responses �ڲ�����ռ䣬�ɵ��÷�����ReleaseMdQueryResponses�ͷ�
	* @return 0 �ɹ� < 0 �����
	*/
	virtual int RequestMDQuery(::com::htsc::mdc::insight::model::MDQueryRequest* request,
		std::vector< ::com::htsc::mdc::insight::model::MDQueryResponse* >*& mdresponses) = 0;

	/**
	* �����ѯ���󣬱�������ֹ���̲߳������ã��ظ�����ͨ���ص��ӿڻ�ã���Ҫ��response_callback����
	* @param[in] request �ѷ���ÿռ�Ĳ�ѯ�����ɵ��÷��ͷ�
	* @return 0 �ɹ� < 0 �����
	*/
	virtual int RequestMDQuery(::com::htsc::mdc::insight::model::MDQueryRequest* request) = 0;

	/**
	* �ͷ������ѯ���
	* @param[in] mdresponses ��ѯ���
	*/
	virtual void ReleaseQueryResult(
		std::vector< ::com::htsc::mdc::insight::model::MDQueryResponse* >*& mdresponses);

	/**
	* �رգ��ȴ��߳��˳�
	* @param[in] clear_members �Ƿ������Ա����reconnectʱ��������
	* @param[in] wait_threads_quit �ȴ��߳��˳�
	*/
	virtual void Close() = 0;

	/**
	* ���ô����̳߳��߳������ڵ�¼֮ǰ����
	* ����Ϣ�Ĵ����߼��Ƚϸ��ӣ���Ҫ�����̲߳�������ʱ��
	* �����ڵ���LoginByServiceDiscovery��¼�����֮ǰ����
	* @param[in] count �߳���
	*/
	virtual void set_handle_pool_thread_count(short count) = 0;

	/**
	* ��ô����߳���
	* @return �߳���
	*/
	virtual short handle_pool_thread_count() const = 0;


	virtual void clear_expire_message() = 0;
};

NAMESPACE_END

#endif //HTSC_MD_GATEWAY_CLIENT_INTERFACE_H