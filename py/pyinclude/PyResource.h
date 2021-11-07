//
// Created by szhang on 2020/8/19.
//

#ifndef PYINTERACT_PYRESOURCE_H
#define PYINTERACT_PYRESOURCE_H

#include "ScalarImp.h"
#include "Python.h"
#include "PyInterUtil.h"

class PyResource : public Resource{
public:
    PyResource(long long handle, const string& desc, const FunctionDefSP& onClose, Session* session, bool isNew = true):Resource(handle, desc, onClose, session){
        this->session = session;
        this->isNew = isNew;
    }
    ConstantSP getMember(const ConstantSP &key) const;
    ConstantSP callMethod(const string& name, Heap* heap, vector<ConstantSP>& args) const;
    ~PyResource();
private:
    Session *session;
    bool isNew;
};



#endif //PYINTERACT_PYRESOURCE_H
