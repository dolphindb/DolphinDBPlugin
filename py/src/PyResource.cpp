//
// Created by szhang on 2020/8/19.
//

#include "PyResource.h"
#include "Protect.h"
#include <pybind11/embed.h>

namespace py = pybind11;

static void pyObjectOnClose(Heap *heap, vector<ConstantSP> &args) {
    ProtectGil proGil;
    PyObject *obj = reinterpret_cast<PyObject *>(args[0]->getLong());
    if (obj != nullptr) {
        Py_DecRef(obj);
        args[0]->setLong(0);
    }
}

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

ConstantSP PyResource::getMember(const ConstantSP& key) const {
    ProtectGil proGil;
    PyObject *pyObj, *pyValue;
    pyObj =  reinterpret_cast<PyObject*>(this->getLong());
    string name = key->getString();
    pyValue = PyObject_GetAttrString(pyObj, name.c_str());
    if (pyValue == nullptr)
        throw IllegalArgumentException("PyResource::getMember", "No such attribute in the instance.");
    FunctionDefSP onClose(Util::createSystemProcedure("getInstanceByName onclose()", pyObjectOnClose, 1, 1));
    return pyObjectRet(pyValue, this->session, onClose);
}

ConstantSP PyResource::callMethod(const string &name, Heap* heap, vector<ConstantSP> &arguments) const{
    ProtectGil proGil;
    PyObject *pyClsInst = reinterpret_cast<PyObject*>(this->getLong());
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
    PyObject *pyFunc = PyObject_GetAttrString(pyClsInst, name.c_str());
    if (pyFunc == nullptr)
        throw IllegalArgumentException("PyResource::callMethod",  "The class instance has no method '"+ name +"'");
    PyObject *res;
    res = PyObject_CallObject(pyFunc, pArgs);
    Py_DECREF(pArgs);
    if (res == nullptr){
        PyObject *exc, *val, *tb;
        PyErr_Fetch(&exc, &val, &tb);
        if(exc){
            py::str errMsg = py::handle(val).cast<py::str>();
            string msg = py::handle(errMsg).cast<std::string>();
            throw RuntimeException("Error when call function "+ name +": "+ msg);
        }
        throw RuntimeException("Error when call function "+ name);
    }
    FunctionDefSP onClose(Util::createSystemProcedure("pyObject onclose()", pyObjectOnClose, 1, 1));
    return pyObjectRet(res, this->session, onClose);
}