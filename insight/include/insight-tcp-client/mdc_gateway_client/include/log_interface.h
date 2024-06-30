//======================================================================================================
// log_interface.h
// 日志接口定义头文件
// 版本：v1.0 20170828
// lujun
//======================================================================================================

#ifndef HTSC_MDC_GATEWAY_CLIENT_LOG_INTERFACE_H
#define HTSC_MDC_GATEWAY_CLIENT_LOG_INTERFACE_H

#include "base_define.h"

NAMESPACE_BEGIN

/**
* 输出日志接口
*/
class LIB_EXPORT LogInterface {
public:
	LogInterface() {}
	virtual ~LogInterface() {}

public:
	//记录日志
	virtual void log(const std::string& log_line) {}
};


NAMESPACE_END

#endif //HTSC_MDC_GATEWAY_CLIENT_LOG_INTERFACE_H