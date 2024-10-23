//
// Created by ynwang on 2019/5/22.
//

#ifndef DOLPHINDBPLUGIN_OPCEXCEPTION_H
#define DOLPHINDBPLUGIN_OPCEXCEPTION_H

#endif //DOLPHINDBPLUGIN_OPCEXCEPTION_H
#include <string>
#include <exception>
using namespace std;
class OPCException: public std::exception
{
public:
    OPCException(const string& errMsg): errMsg_(errMsg){}
    virtual const char* what() const throw(){
        return errMsg_.c_str();
    }
    virtual ~OPCException() throw(){}
    const std::string errMsg_;
};