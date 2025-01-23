//
// Created by lin on 2020/9/14.
//

#ifndef PLUGINPY_TYPECONVERSION_CPP_H
#define PLUGINPY_TYPECONVERSION_CPP_H
#include "Python.h"
#include "CoreConcept.h"

enum PY_TYPE {PY_NON, PY_NONE, PY_INT, PY_FLOAT, PY_STRING, PY_BOOL, PY_TUPLE, PY_LIST, PY_DICT, PY_SET,
    PY_NP_NDARRAY, PY_NP_MATRIX, PY_NP_BOOL, PY_NP_INT, PY_NP_INT8, PY_NP_INT16, PY_NP_INT32, PY_NP_INT64,
    PY_NP_FLOAT, PY_NP_FLOAT32, PY_NP_FLOAT64,PY_NP_DATETIME, PY_NP_DATETIME_M, PY_NP_DATETIME_D,
    PY_NP_DATETIME_m, PY_NP_DATETIME_s,PY_NP_DATETIME_h, PY_NP_DATETIME_ms, PY_NP_DATETIME_us, PY_NP_DATETIME_ns,
    PY_PD_SERIES, PY_PD_DATAFRAME
};
PyObject* dolphin2py(ConstantSP input, bool dateFrameFlag= false);
extern "C" ConstantSP py2dolphin(PyObject *input, bool free = true, bool addIndex = false);
#endif //PLUGINPY_TYPECONVERSION_CPP_H
