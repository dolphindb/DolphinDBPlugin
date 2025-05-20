//
// Created by szhang on 2020/8/19.
//

#include "DolphinDBEverything.h"
#include "PyResource.h"
#include "Protect.h"
#include <pybind11/embed.h>
#include "ddbplugin/PluginLoggerImp.h"

namespace py = pybind11;

static void pyObjectOnClose(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    ProtectGil proGil;
    PyObject *obj = reinterpret_cast<PyObject *>(args[0]->getLong());
    if (obj != nullptr) {
        Py_DecRef(obj);
        args[0]->setLong(0);
    }
}

PyResource::~PyResource() noexcept {
    if (!this->isNew_)
        return;
    PyObject *obj = reinterpret_cast<PyObject*>(handle_);
    ProtectGil proGil;
    if (obj != nullptr) {
        Py_DECREF(obj);
        this->setLong(0);
    }
}

ConstantSP PyResource::getMember(const ConstantSP& key) const {
    ProtectGil proGil;
    PyObject *pyValue;
    string name = key->getString();
    pyValue = PyObject_GetAttrString(handle_, name.c_str()); 
    if (pyValue == nullptr)
        throw IllegalArgumentException("PyResource::getMember", "No such attribute in the instance.");
    FunctionDefSP onClose(Util::createSystemProcedure("getInstanceByName onclose()", pyObjectOnClose, 1, 1));
    return pyObjectRet(pyValue, this->session_, onClose);
}

bool PyResource::hasMethod(const string& name) const {
    ProtectGil proGil;
    PyObject *pyFunc = PyObject_GetAttrString(handle_, name.c_str());
    if (pyFunc == nullptr)
        throw IllegalArgumentException("PyResource::callMethod",  "The class instance has no method '"+ name +"'");
    return true;
}

ConstantSP PyResource::callMethod(const string &name, Heap* heap, vector<ConstantSP> &arguments) const{
    std::ignore = heap;
    ProtectGil proGil;
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
    PyObject *pyFunc = PyObject_GetAttrString(handle_, name.c_str());
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
    return pyObjectRet(res, this->session_, onClose);
}   

ConstantSP PyResource::callMethod(Heap* heap, vector<ConstantSP> &args){
    if (args.size() < 2){
        throw IllegalArgumentException("PyResource::callMethod", "The number of arguments should be at least 1.");
    }
    string name = args[0]->getString();
    OOInstanceSP handle = args[1];
    vector<ConstantSP> arguments;
    for (size_t i = 2; i < args.size(); ++i){
        arguments.push_back(args[i]);
    }
    PyResourceSP classPy = handle->getClass();
    ConstantSP ret = classPy->callMethod(name, heap, arguments);
    return ret;
}

FunctionDefSP PyResource::getMethod(const string& name) const{
    hasMethod(name);
    vector<ConstantSP> args;
    args.push_back(new String(name));
    FunctionDefSP callMethodFunc(Util::createSystemFunction("PyResource::callMethod", PyResource::callMethod, 0, 102, false));
    return Util::createPartialFunction(callMethodFunc, args);
}   