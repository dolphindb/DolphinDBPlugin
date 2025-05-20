//
// Created by szhang on 2020/8/19.
//

#ifndef PYINTERACT_PYINTERUTIL_H
#define PYINTERACT_PYINTERUTIL_H

#include "DolphinDBEverything.h"
#include "TypeConversion.h"
#include "Protect.h"
using namespace ddb;

static ConstantSP pyObjectRet(PyObject *obj, Session *session = nullptr, FunctionDefSP onClose= nullptr, bool cvt=true) {
    ConstantSP csp = nullptr;
    ProtectGil pgil;
    if (cvt){
        csp = py2dolphin(obj);
    }
    if (csp.get() == nullptr){
        if (session == nullptr){
            throw IllegalArgumentException("pyObjectRet", "Unknown python data type.");
        }
        std::unique_ptr<PyObject> pyobj = (std::unique_ptr<PyObject>)obj;
        return Util::createResource((long long)pyobj.release(), "python objectRet", onClose, session);
    }
    Py_DECREF(obj);
    return csp;
}

#endif //PYINTERACT_PYINTERUTIL_H
