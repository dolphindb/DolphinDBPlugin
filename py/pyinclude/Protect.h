//
// Created by lin on 2020/9/5.
//

#ifndef PLUGINPY_PROTECT_H
#define PLUGINPY_PROTECT_H
#include "Python.h"

class ProtectGil{
public:
    ProtectGil() {
        gstate_ = PyGILState_Ensure();
    }
    ~ProtectGil() {
        PyGILState_Release(gstate_);
    }
private:
    PyGILState_STATE gstate_;
};

#endif //PLUGINPY_PROTECT_H
