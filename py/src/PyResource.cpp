//
// Created by szhang on 2020/8/19.
//

#include "PyResource.h"
#include "Protect.h"

PyResource::~PyResource() noexcept {
    if (!this->isNew)
        return;
    PyObject *obj = reinterpret_cast<PyObject*>(this->getLong());
    ProtectGil proGil;
    if (obj != nullptr) {
        Py_DECREF(obj);
        this->setLong(0);
    }
}

ConstantSP PyResource::createResource(PyObject *obj) const{
    return pyObjectRet(obj, this->session, this->onClose);
}

ConstantSP PyResource::getMember(const ConstantSP& key) const {
    ProtectGil proGil;
    PyObject *pyObj, *pyValue;
    pyObj =  reinterpret_cast<PyObject*>(this->getLong());
    string name = key->getString();
    pyValue = PyObject_GetAttrString(pyObj, name.c_str());
    if (pyValue == nullptr)
        throw IllegalArgumentException("PyResource::getMember", "No such attribute in the instance.");
    return this->createResource(pyValue);
}

ConstantSP PyResource::callMethod(const string &name, Heap* heap, vector<ConstantSP> &arguments) const{
    ProtectGil proGil;
    PyObject *pyClsInst = reinterpret_cast<PyObject*>(this->getLong());
    string methodName = name;
    int argc = arguments.size();
    PyObject *pArgs = PyTuple_New(argc);
    for (int i = 0; i < argc; ++i){
        PyObject *arg;
        if (arguments[i]->getString() != "python object"){
            arg = dolphin2py(arguments[i]);
            if (arg == nullptr){
                throw IllegalArgumentException("PyResource::callMethod", "Invalid argument.");
            }
        } else {
            arg = reinterpret_cast<PyObject *>(arguments[i]->getLong());
            Py_INCREF(arg);
        }
        if (PyTuple_SetItem(pArgs, i, arg) == -1){
            throw IllegalArgumentException("PyResource::callMethod", "Error when creating input args.");
        }
    }
    PyObject *pyFunc = PyObject_GetAttrString(pyClsInst, methodName.c_str());
    if (pyFunc == nullptr)
        throw IllegalArgumentException("PyResource::callMethod",  "The class instance has no method '"+methodName+"'");
    PyObject *res = PyObject_CallObject(pyFunc, pArgs);
    //PyObject *res = PyObject_CallFunctionObjArgs(pyFunc, pArgs);
    Py_DECREF(pArgs);
    if (res == nullptr)
        throw IllegalArgumentException("PyResource::callMethod", "Error when calling method.");
    return this->createResource(res);
}