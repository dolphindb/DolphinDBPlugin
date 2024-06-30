//======================================================================================================
// mdc_client_factory.h
// 客户端生成工厂头文件
// 版本：v1.0 增加discovery service ssl加密
// lujun 
//======================================================================================================

#ifndef HTSC_MDC_GATEWAY_CLIENT_CLIENT_FACTORY_H
#define HTSC_MDC_GATEWAY_CLIENT_CLIENT_FACTORY_H

#include "client_interface.h"
#include "base_define.h"

NAMESPACE_BEGIN

/**
* gateway客户端接口创建工厂类
*/
class LIB_EXPORT ClientFactory {
public:
	virtual ~ClientFactory();

	//单例
	static ClientFactory* Instance();
	static void Uninstance();

private:
	DISALLOW_COPY_AND_ASSIGN(ClientFactory);

private:
	ClientFactory();
	//初始化配置，包括读取默认值和从配置文件中更新
	void InitConfig();

private:
	static ClientFactory* factory_;
	static ClientInterface* client_;

public:
	/**
	* 创建客户端，全局唯一
	* @param[in] discovery_service_using_ssl 是否使用加密
	* @param[in] folder 证书所在的目录，其下具备证书（FacadeClientCert.pem）和私钥（FacadeClientKeyPkcs8.pem）
	* @return 客户端指针
	*/
	ClientInterface* CreateClient(bool discovery_service_using_ssl = false, const char* folder = "./cert");

	/**
	* 获取整形属性
	* @param[in] key 健
	* @return 返回值 >= 0 正确 < 0 错误号
	*/
	int GetIntPropertyValue(const char* key);

	/**
	* 设置整形属性
	* @param[in] key 健
	* @param[in] value 值
	* @return 返回值 < 0 错误号 >=0 正确值
	*/
	int SetIntPropertyValue(const char* key, int value);

	/**
	* 设置加密密码的秘钥
	*/
	void SetEncodeKey(const unsigned char* key);

private:
	bool using_ssl_;
};

NAMESPACE_END

#endif //HTSC_MDC_GATEWAY_CLIENT_CLIENT_FACTORY_H