//======================================================================================================
// log_interface.h
// ��־�ӿڶ���ͷ�ļ�
// �汾��v1.0 20170828
// lujun
//======================================================================================================

#ifndef HTSC_MDC_GATEWAY_CLIENT_LOG_INTERFACE_H
#define HTSC_MDC_GATEWAY_CLIENT_LOG_INTERFACE_H

#include "base_define.h"

NAMESPACE_BEGIN

/**
* �����־�ӿ�
*/
class LIB_EXPORT LogInterface {
public:
	LogInterface() {}
	virtual ~LogInterface() {}

public:
	//��¼��־
	virtual void log(const std::string& log_line) {}
};


NAMESPACE_END

#endif //HTSC_MDC_GATEWAY_CLIENT_LOG_INTERFACE_H