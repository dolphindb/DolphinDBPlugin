//
// Created by szhang on 2020/8/19.
//

#ifndef PYINTERACT_PYRESOURCE_H
#define PYINTERACT_PYRESOURCE_H

#include "DolphinDBEverything.h"
#include "ScalarImp.h"
#include "Python.h"
#include "PyInterUtil.h"
#include "DolphinClass.h"

class PyResource;
typedef SmartPointer<PyResource> PyResourceSP;

class PyResource : public DolphinClass {
public:
    PyResource(PyObject* handle, const string& desc, const FunctionDefSP& onClose, Session* session, bool isNew = true) : DolphinClass("py", desc){
        std::ignore = onClose;
        this->session_ = session;
        this->isNew_ = isNew;
        this->handle_ = handle;
    }

    ConstantSP getMember(const ConstantSP &key) const override;
    ConstantSP getMember(const string &key) const override{
        return getMember(new String(key));
    }
    virtual ~PyResource() noexcept;
	virtual FunctionDefSP getMethod(const string& name) const override;
	virtual bool hasMethod(const string& name) const override;
    ConstantSP callMethod(const string& name, Heap* heap, vector<ConstantSP>& args) const;
    string getString() const {return "PyResource class "; }
    ConstantSP getValue() const override{ return new Bool(true);} 
    static ConstantSP callMethod(Heap* heap, vector<ConstantSP> &args); 
private:
    Session *session_;
    bool isNew_;
public:
    PyObject* handle_ = nullptr;
};

class PyInstance : public OOInstance{
public:
    PyInstance(const OOClassSP& ooClass, SYSOBJ_TYPE type) : OOInstance(ooClass, type){}
    virtual ConstantSP getMember(const string& key) const override{
         return getClass()->getMember(key);
    }
    virtual ConstantSP getValue() const override {
        return new PyInstance(getClass(), getSysObjType());
    }

    virtual ~PyInstance(){}
};


#endif //PYINTERACT_PYRESOURCE_H
