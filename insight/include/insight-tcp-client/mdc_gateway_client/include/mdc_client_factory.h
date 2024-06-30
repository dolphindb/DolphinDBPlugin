//======================================================================================================
// mdc_client_factory.h
// �ͻ������ɹ���ͷ�ļ�
// �汾��v1.0 ����discovery service ssl����
// lujun 
//======================================================================================================

#ifndef HTSC_MDC_GATEWAY_CLIENT_CLIENT_FACTORY_H
#define HTSC_MDC_GATEWAY_CLIENT_CLIENT_FACTORY_H

#include "client_interface.h"
#include "base_define.h"

NAMESPACE_BEGIN

/**
* gateway�ͻ��˽ӿڴ���������
*/
class LIB_EXPORT ClientFactory {
public:
	virtual ~ClientFactory();

	//����
	static ClientFactory* Instance();
	static void Uninstance();

private:
	DISALLOW_COPY_AND_ASSIGN(ClientFactory);

private:
	ClientFactory();
	//��ʼ�����ã�������ȡĬ��ֵ�ʹ������ļ��и���
	void InitConfig();

private:
	static ClientFactory* factory_;
	static ClientInterface* client_;

public:
	/**
	* �����ͻ��ˣ�ȫ��Ψһ
	* @param[in] discovery_service_using_ssl �Ƿ�ʹ�ü���
	* @param[in] folder ֤�����ڵ�Ŀ¼�����¾߱�֤�飨FacadeClientCert.pem����˽Կ��FacadeClientKeyPkcs8.pem��
	* @return �ͻ���ָ��
	*/
	ClientInterface* CreateClient(bool discovery_service_using_ssl = false, const char* folder = "./cert");

	/**
	* ��ȡ��������
	* @param[in] key ��
	* @return ����ֵ >= 0 ��ȷ < 0 �����
	*/
	int GetIntPropertyValue(const char* key);

	/**
	* ������������
	* @param[in] key ��
	* @param[in] value ֵ
	* @return ����ֵ < 0 ����� >=0 ��ȷֵ
	*/
	int SetIntPropertyValue(const char* key, int value);

	/**
	* ���ü����������Կ
	*/
	void SetEncodeKey(const unsigned char* key);

private:
	bool using_ssl_;
};

NAMESPACE_END

#endif //HTSC_MDC_GATEWAY_CLIENT_CLIENT_FACTORY_H