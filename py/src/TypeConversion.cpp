//
// Created by lin on 2020/9/11.
//

#include "Util.h"
#include "TypeConversion.h"
#include "Exceptions.h"
#include "ScalarImp.h"
#include "Protect.h"
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/numpy.h>
#include <iostream>
namespace py = pybind11;

#if defined(__GNUC__) && __GNUC__ >= 4
#define LIKELY(x) (__builtin_expect((x), 1))
#define UNLIKELY(x) (__builtin_expect((x), 0))
#else
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#endif

struct Preserved {
    // instantiation only once for frequently use

    // modules and methods
    const static py::handle numpy_;         // module
    const static py::handle isnan_;         // func
    const static py::handle datetime64_;    // type, equal to np.datetime64
    const static py::handle pandas_;        // module

    // pandas types (use py::isinstance)
    const static py::handle pdseries_;
    const static py::handle pddataframe_;

    // numpy dtypes (instances of dtypes, use equal)
    const static py::handle nparray_;
    const static py::handle npbool_;
    const static py::handle npint8_;
    const static py::handle npint16_;
    const static py::handle npint32_;
    const static py::handle npint64_;
    const static py::handle npfloat32_;
    const static py::handle npfloat64_;
    const static py::handle npdatetime64M_;
    const static py::handle npdatetime64D_;
    const static py::handle npdatetime64m_;
    const static py::handle npdatetime64s_;
    const static py::handle npdatetime64ms_;
    const static py::handle npdatetime64us_;
    const static py::handle npdatetime64ns_;
    const static py::handle npdatetime64_;    // dtype, equal to np.datetime64().dtype
    const static py::handle npobject_;

    // python types (use py::isinstance)
    const static py::handle pynone_;
    const static py::handle pybool_;
    const static py::handle pyint_;
    const static py::handle pyfloat_;
    const static py::handle pystr_;
    const static py::handle pybytes_;
    const static py::handle pyset_;
    const static py::handle pytuple_;
    const static py::handle pylist_;
    const static py::handle pydict_;

    // null map
    const static uint64_t npnan_ = 9221120237041090560LL;
};

const py::handle Preserved::numpy_ = py::module::import("numpy").inc_ref();
const py::handle Preserved::isnan_ = numpy_.attr("isnan");
const py::handle Preserved::datetime64_ = numpy_.attr("datetime64");
const py::handle Preserved::pandas_ = py::module::import("pandas").inc_ref();
const py::handle Preserved::pddataframe_ = pandas_.attr("DataFrame")().get_type().inc_ref();
const py::handle Preserved::pdseries_ = pandas_.attr("Series")().get_type().inc_ref();
const py::handle Preserved::nparray_ = py::array().get_type().inc_ref();
const py::handle Preserved::npbool_ = py::dtype("bool").inc_ref();
const py::handle Preserved::npint8_ = py::dtype("int8").inc_ref();
const py::handle Preserved::npint16_ = py::dtype("int16").inc_ref();
const py::handle Preserved::npint32_ = py::dtype("int32").inc_ref();
const py::handle Preserved::npint64_ = py::dtype("int64").inc_ref();
const py::handle Preserved::npfloat32_ = py::dtype("float32").inc_ref();
const py::handle Preserved::npfloat64_ = py::dtype("float64").inc_ref();
const py::handle Preserved::npdatetime64M_ = py::dtype("datetime64[M]").inc_ref();
const py::handle Preserved::npdatetime64D_ = py::dtype("datetime64[D]").inc_ref();
const py::handle Preserved::npdatetime64m_ = py::dtype("datetime64[m]").inc_ref();
const py::handle Preserved::npdatetime64s_ = py::dtype("datetime64[s]").inc_ref();
const py::handle Preserved::npdatetime64ms_ = py::dtype("datetime64[ms]").inc_ref();
const py::handle Preserved::npdatetime64us_ = py::dtype("datetime64[us]").inc_ref();
const py::handle Preserved::npdatetime64ns_ = py::dtype("datetime64[ns]").inc_ref();
const py::handle Preserved::npdatetime64_ = py::dtype("datetime64").inc_ref();
const py::handle Preserved::npobject_ = py::dtype("object").inc_ref();
const py::handle Preserved::pynone_ = py::none().get_type().inc_ref();
const py::handle Preserved::pybool_ = py::bool_().get_type().inc_ref();
const py::handle Preserved::pyint_ = py::int_().get_type().inc_ref();
const py::handle Preserved::pyfloat_ = py::float_().get_type().inc_ref();
const py::handle Preserved::pystr_ = py::str().get_type().inc_ref();
const py::handle Preserved::pybytes_ = py::bytes().get_type().inc_ref();
const py::handle Preserved::pyset_ = py::set().get_type().inc_ref();
const py::handle Preserved::pytuple_ = py::tuple().get_type().inc_ref();
const py::handle Preserved::pylist_ = py::list().get_type().inc_ref();
const py::handle Preserved::pydict_ = py::dict().get_type().inc_ref();

template <typename T>
void append(const py::array &pyArray, int size, std::function<void(T *, int)> f) {
    T buf[std::min(1024, size)];
    int i = 0;
    bool isDatetime64 = false;
    bool isInt8 = false;
    bool checked = false;
    for (auto &val : pyArray) {
        if (!checked) {
            isDatetime64 = py::isinstance(val, Preserved::datetime64_);
            auto type = py::getattr(val, "dtype");
            isInt8 = type.equal(Preserved::npint8_);
            checked = true;
        }
        if (isDatetime64) {
            buf[i++] = val.attr("astype")("int64").cast<T>();
        } else if (isInt8) {
            buf[i++] = val.cast<int8_t>();
        } else {
            buf[i++] = val.cast<T>();
        }
        //        obj.attr("astype")("int64")
        if (i == 1024) {
            f(buf, 1024);
            i = 0;
        }
    }
    if (i > 0) { f(buf, i); }
}

static inline void SET_NPNAN(void *p, size_t len = 1) { std::fill((uint64_t *)p, ((uint64_t *)p) + len, 9221120237041090560LL); }

PyObject* dolphin2py(ConstantSP input, bool dateFrameFlag) {
    DATA_FORM dForm = input->getForm();
    if (dForm == DF_SCALAR) {
        DATA_TYPE dType = input->getType();
        if (dType == DT_STRING || dType == DT_SYMBOL || dType == DT_BLOB ||
            dType == DT_IP || dType == DT_UUID || dType == DT_INT128) {
            std::string str = input->getString();
            return PyUnicode_DecodeFSDefault(str.c_str());
        }
        else if (dType == DT_FLOAT  || dType == DT_DOUBLE) {
            return PyFloat_FromDouble(input->getDouble());
        }
        else if (dType == DT_CHAR || dType == DT_INT || dType == DT_LONG || dType == DT_SHORT) {
            return PyLong_FromLong(input->getLong());
        }
        else if (dType == DT_BOOL) {
            if (input->isNull()) Py_RETURN_FALSE;
            if (input->getBool()) Py_RETURN_TRUE;
            else Py_RETURN_FALSE;
        }
        else if (dType == DT_VOID) {
            Py_RETURN_NONE;
        }
        else if (dType == DT_DATE || dType == DT_MONTH || dType == DT_TIME ||
                 dType == DT_MINUTE || dType == DT_SECOND || dType == DT_DATETIME ||
                 dType == DT_TIMESTAMP || dType == DT_NANOTIME || dType == DT_NANOTIMESTAMP) {
            if (dType == DT_DATE) {
                py::object ret = Preserved::datetime64_(input->getLong(), "D");
                Py_INCREF(ret.ptr());
                return ret.ptr();
            }
            else if (dType == DT_MONTH) {
                py::object ret = Preserved::datetime64_(input->getLong() - 1970*12LL, "M");
                Py_INCREF(ret.ptr());
                return ret.ptr();
            }
            else if (dType == DT_TIME || dType == DT_TIMESTAMP) {
                py::object ret = Preserved::datetime64_(input->getLong(), "ms");
                Py_INCREF(ret.ptr());
                return ret.ptr();
            }
            else if (dType == DT_MINUTE) {
                py::object ret = Preserved::datetime64_(input->getLong(), "m");
                Py_INCREF(ret.ptr());
                return ret.ptr();
            }
            else if (dType == DT_SECOND || dType == DT_DATETIME) {
                py::object ret = Preserved::datetime64_(input->getLong(), "s");
                Py_INCREF(ret.ptr());
                return ret.ptr();
            }
            else if (dType == DT_NANOTIME || dType == DT_NANOTIMESTAMP) {
                py::object ret = Preserved::datetime64_(input->getLong(), "ns");
                Py_INCREF(ret.ptr());
                return ret.ptr();
            }
        }
    }
    else {
        int size = input->size();
        if (dForm == DF_VECTOR) {
            DATA_TYPE type = input->getType();
            VectorSP ddbVec = input;
            size_t size = ddbVec->size();
            switch (type) {
                case DT_VOID: {
                    py::array pyVec;
                    pyVec.resize({size});
                    Py_INCREF(pyVec.ptr());
                    return pyVec.ptr();
                }
                case DT_BOOL: {
                    py::array pyVec(py::dtype("bool"), {size}, {});
                    ddbVec->getBool(0, size, (char *)pyVec.mutable_data());
                    if(UNLIKELY(ddbVec->getNullFlag())) {
                        if (UNLIKELY(ddbVec->hasNull())) {
                            pyVec = pyVec.attr("astype")("float64");
                            double *p = (double *) pyVec.mutable_data();
                            char buf[1024];
                            int start = 0;
                            int N = size;
                            while (start < N) {
                                int len = std::min(N - start, 1024);
                                ddbVec->getBool(start, len, buf);
                                for (int i = 0; i < len; ++i) {
                                    if(UNLIKELY(buf[i] == INT8_MIN)) {
                                        SET_NPNAN(p + start + i, 1);
                                    }
                                }
                                start += len;
                            }
                        }
                    }
                    Py_INCREF(pyVec.ptr());
                    return pyVec.ptr();
                }
                case DT_CHAR: {
                    py::array pyVec(py::dtype("int8"), {size}, {});
                    ddbVec->getChar(0, size, (char *)pyVec.mutable_data());
                    if(UNLIKELY(ddbVec->getNullFlag())) {
                        if (UNLIKELY(ddbVec->hasNull())) {
                            pyVec = pyVec.attr("astype")("float64");
                            double *p = (double *) pyVec.mutable_data();
                            char buf[1024];
                            int start = 0;
                            int N = size;
                            while (start < N) {
                                int len = std::min(N - start, 1024);
                                ddbVec->getChar(start, len, buf);
                                for (int i = 0; i < len; ++i) {
                                    if(UNLIKELY(buf[i] == INT8_MIN)) {
                                        SET_NPNAN(p + start + i, 1);
                                    }
                                }
                                start += len;
                            }
                        }
                    }
                    Py_INCREF(pyVec.ptr());
                    return pyVec.ptr();
                }
                case DT_SHORT: {
                    py::array pyVec(py::dtype("int16"), {size}, {});
                    ddbVec->getShort(0, size, (short *)pyVec.mutable_data());
                    if(UNLIKELY(ddbVec->getNullFlag())) {
                        if (UNLIKELY(ddbVec->hasNull())) {
                            pyVec = pyVec.attr("astype")("float64");
                            double *p = (double *) pyVec.mutable_data();
                            short buf[1024];
                            int start = 0;
                            int N = size;
                            while (start < N) {
                                int len = std::min(N - start, 1024);
                                ddbVec->getShort(start, len, buf);
                                for (int i = 0; i < len; ++i) {
                                    if(UNLIKELY(buf[i] == INT16_MIN)) {
                                        SET_NPNAN(p + start + i, 1);
                                    }
                                }
                                start += len;
                            }
                        }
                    }
                    Py_INCREF(pyVec.ptr());
                    return pyVec.ptr();
                }
                case DT_INT: {
                    py::array pyVec(py::dtype("int32"), {size}, {});
                    ddbVec->getInt(0, size, (int *)pyVec.mutable_data());
                    if(UNLIKELY(ddbVec->getNullFlag())) {
                        if (UNLIKELY(ddbVec->hasNull())) {
                            pyVec = pyVec.attr("astype")("float64");
                            double *p = (double *) pyVec.mutable_data();
                            int buf[1024];
                            int start = 0;
                            int N = size;
                            while (start < N) {
                                int len = std::min(N - start, 1024);
                                ddbVec->getInt(start, len, buf);
                                for (int i = 0; i < len; ++i) {
                                    if(UNLIKELY(buf[i] == INT32_MIN)) {
                                        SET_NPNAN(p + start + i, 1);
                                    }
                                }
                                start += len;
                            }
                        }
                    }
                    Py_INCREF(pyVec.ptr());
                    return pyVec.ptr();
                }
                case DT_LONG: {
                    py::array pyVec(py::dtype("int64"), {size}, {});
                    ddbVec->getLong(0, size, (long long *)pyVec.mutable_data());
                    if(UNLIKELY(ddbVec->getNullFlag())) {
                        if (UNLIKELY(ddbVec->hasNull())) {
                            pyVec = pyVec.attr("astype")("float64");
                            double *p = (double *) pyVec.mutable_data();
                            long long buf[1024];
                            int start = 0;
                            int N = size;
                            while (start < N) {
                                int len = std::min(N - start, 1024);
                                ddbVec->getLong(start, len, buf);
                                for (int i = 0; i < len; ++i) {
                                    if(UNLIKELY(buf[i] == INT64_MIN)) {
                                        SET_NPNAN(p + start + i, 1);
                                    }
                                }
                                start += len;
                            }
                        }
                    }
                    Py_INCREF(pyVec.ptr());
                    return pyVec.ptr();
                }
                case DT_DATE: {
                    if(dateFrameFlag){
                        py::array pyVec(py::dtype("datetime64[ns]"), {size}, {});
                        ddbVec->getLong(0, size, (long long *)pyVec.mutable_data());
                        long long *p = (long long *)pyVec.mutable_data();
                        if (UNLIKELY(ddbVec->hasNull())) {
                            for (size_t i = 0; i < size; ++i) {
                                if (UNLIKELY(p[i] == INT64_MIN)) { continue; }
                                p[i] *= 86400000000000;
                            }
                        }
                        else {
                            for (size_t i = 0; i < size; ++i) {
                                p[i] *= 86400000000000;
                            }
                        }
                        Py_IncRef(pyVec.ptr());
                        return pyVec.ptr();
                    }
                    else{
                        py::array pyVec(py::dtype("datetime64[D]"), {size}, {});
                        ddbVec->getLong(0, size, (long long *)pyVec.mutable_data());
                        Py_IncRef(pyVec.ptr());
                        return pyVec.ptr();
                    }
                }
                case DT_MONTH: {
                    if(dateFrameFlag) {
                        py::array pyVec(py::dtype("datetime64[M]"), {size}, {});
                        ddbVec->getLong(0, size, (long long *)pyVec.mutable_data());
                        long long *p = (long long *)pyVec.mutable_data();
                        if (UNLIKELY(ddbVec->hasNull())) {
                            for (size_t i = 0; i < size; ++i) {
                                if (UNLIKELY(p[i] == INT64_MIN)) { continue; }
                                else {
                                    if(p[i] < 23640 || p[i] > 27147) {
                                        throw RuntimeException("In dataFrame Month must between 1970.01M and 2262.04M");
                                    }
                                    p[i] -= 23640;
                                }
                            }
                        }
                        else {
                            for (size_t i = 0; i < size; ++i) {
                                if(p[i] < 23640 || p[i] > 27147) {
                                    throw RuntimeException("In dataFrame Month must between 1970.01M and 2262.04M");
                                }
                                p[i] -= 23640;
                            }
                        }
                        Py_INCREF(pyVec.ptr());
                        return pyVec.ptr();
                    } else {
                        py::array pyVec(py::dtype("datetime64[M]"), {size}, {});
                        ddbVec->getLong(0, size, (long long *)pyVec.mutable_data());
                        long long *p = (long long *)pyVec.mutable_data();
                        if (UNLIKELY(ddbVec->hasNull())) {
                            for (size_t i = 0; i < size; ++i) {
                                if (UNLIKELY(p[i] == INT64_MIN)) { continue; }
                                p[i] -= 1970 * 12;
                            }
                        }
                        else {
                            for (size_t i = 0; i < size; ++i) {
                                p[i] -= 1970 * 12;
                            }
                        }
                        Py_INCREF(pyVec.ptr());
                        return pyVec.ptr();
                    }
                }
                case DT_TIME: {
                    if(dateFrameFlag){
                        py::array pyVec(py::dtype("datetime64[ns]"), {size}, {});
                        ddbVec->getLong(0, size, (long long *)pyVec.mutable_data());
                        long long *p = (long long *)pyVec.mutable_data();
                        if (UNLIKELY(ddbVec->hasNull())) {
                            for (size_t i = 0; i < size; ++i) {
                                if (UNLIKELY(p[i] == INT64_MIN)) { continue; }
                                p[i] *= 1000000;
                            }
                        }
                        else {
                            for (size_t i = 0; i < size; ++i) {
                                p[i] *= 1000000;
                            }
                        }
                        Py_IncRef(pyVec.ptr());
                        return pyVec.ptr();
                    }
                    else {
                        py::array pyVec(py::dtype("datetime64[ms]"), {size}, {});
                        ddbVec->getLong(0, size, (long long *)pyVec.mutable_data());
                        Py_IncRef(pyVec.ptr());
                        return pyVec.ptr();
                    }
                }
                case DT_MINUTE: {
                    if(dateFrameFlag) {
                        py::array pyVec(py::dtype("datetime64[ns]"), {size}, {});
                        ddbVec->getLong(0, size, (long long *)pyVec.mutable_data());
                        long long *p = (long long *)pyVec.mutable_data();
                        if (UNLIKELY(ddbVec->hasNull())) {
                            for (size_t i = 0; i < size; ++i) {
                                if (UNLIKELY(p[i] == INT64_MIN)) { continue; }
                                p[i] *= 60000000000;
                            }
                        }
                        else {
                            for (size_t i = 0; i < size; ++i) {
                                p[i] *= 60000000000;
                            }
                        }
                        Py_IncRef(pyVec.ptr());
                        return pyVec.ptr();
                    }
                    else{
                        py::array pyVec(py::dtype("datetime64[m]"), {size}, {});
                        ddbVec->getLong(0, size, (long long *)pyVec.mutable_data());
                        Py_IncRef(pyVec.ptr());
                        return pyVec.ptr();
                    }
                }
                case DT_SECOND: {
                    if(dateFrameFlag) {
                        py::array pyVec(py::dtype("datetime64[ns]"), {size}, {});
                        ddbVec->getLong(0, size, (long long *)pyVec.mutable_data());
                        long long *p = (long long *)pyVec.mutable_data();
                        if (UNLIKELY(ddbVec->hasNull())) {
                            for (size_t i = 0; i < size; ++i) {
                                if (UNLIKELY(p[i] == INT64_MIN)) { continue; }
                                p[i] *= 1000000000;
                            }
                        }
                        else {
                            for (size_t i = 0; i < size; ++i) {
                                p[i] *= 1000000000;
                            }
                        }
                        Py_IncRef(pyVec.ptr());
                        return pyVec.ptr();
                    }
                    else {
                        py::array pyVec(py::dtype("datetime64[s]"), {size}, {});
                        ddbVec->getLong(0, size, (long long *)pyVec.mutable_data());
                        Py_IncRef(pyVec.ptr());
                        return pyVec.ptr();
                    }
                }
                case DT_DATETIME: {
                    if(dateFrameFlag) {
                        py::array pyVec(py::dtype("datetime64[ns]"), {size}, {});
                        ddbVec->getLong(0, size, (long long *)pyVec.mutable_data());
                        long long *p = (long long *)pyVec.mutable_data();
                        if (UNLIKELY(ddbVec->hasNull())) {
                            for (size_t i = 0; i < size; ++i) {
                                if (UNLIKELY(p[i] == INT64_MIN)) { continue; }
                                p[i] *= 1000000000;
                            }
                        }
                        else {
                            for (size_t i = 0; i < size; ++i) {
                                p[i] *= 1000000000;
                            }
                        }
                        Py_IncRef(pyVec.ptr());
                        return pyVec.ptr();
                    }
                    else {
                        py::array pyVec(py::dtype("datetime64[s]"), {size}, {});
                        ddbVec->getLong(0, size, (long long *)pyVec.mutable_data());
                        Py_IncRef(pyVec.ptr());
                        return pyVec.ptr();
                    }
                }
                case DT_TIMESTAMP: {
                    if(dateFrameFlag) {
                        py::array pyVec(py::dtype("datetime64[ns]"), {size}, {});
                        ddbVec->getLong(0, size, (long long *)pyVec.mutable_data());
                        long long *p = (long long *)pyVec.mutable_data();
                        if (UNLIKELY(ddbVec->hasNull())) {
                            for (size_t i = 0; i < size; ++i) {
                                if (UNLIKELY(p[i] == INT64_MIN)) { continue; }
                                p[i] *= 1000000;
                            }
                        }
                        else {
                            for (size_t i = 0; i < size; ++i) {
                                p[i] *= 1000000;
                            }
                        }
                        Py_IncRef(pyVec.ptr());
                        return pyVec.ptr();
                    }
                    else {
                        py::array pyVec(py::dtype("datetime64[ms]"), {size}, {});
                        ddbVec->getLong(0, size, (long long *)pyVec.mutable_data());
                        Py_IncRef(pyVec.ptr());
                        return pyVec.ptr();
                    }
                }
                case DT_NANOTIME: {
                    py::array pyVec(py::dtype("datetime64[ns]"), {size}, {});
                    ddbVec->getLong(0, size, (long long *)pyVec.mutable_data());
                    Py_IncRef(pyVec.ptr());
                    return pyVec.ptr();
                }
                case DT_NANOTIMESTAMP: {
                    py::array pyVec(py::dtype("datetime64[ns]"), {size}, {});
                    ddbVec->getLong(0, size, (long long *)pyVec.mutable_data());
                    Py_IncRef(pyVec.ptr());
                    return pyVec.ptr();
                }
                case DT_FLOAT: {
                    py::array pyVec(py::dtype("float32"), {size}, {});
                    ddbVec->getFloat(0, size, (float *)pyVec.mutable_data());
                    if(UNLIKELY(ddbVec->getNullFlag())) {
                        if (UNLIKELY(ddbVec->hasNull())) {
                            auto p = (float *)pyVec.mutable_data();
                            float buf[1024];
                            int start = 0;
                            int N = size;
                            while (start < N) {
                                int len = std::min(N - start, 1024);
                                ddbVec->getFloat(start, len, buf);
                                for (int i = 0; i < len; ++i) {
                                    if(UNLIKELY(buf[i] == FLT_NMIN)) {
                                       p[i]=NAN;
                                    }
                                }
                                start += len;
                            }
                        }
                    }
                    Py_INCREF(pyVec.ptr());
                    return pyVec.ptr();
                }
                case DT_DOUBLE: {
                    py::array pyVec(py::dtype("float64"), {size}, {});
                    ddbVec->getDouble(0, size, (double *)pyVec.mutable_data());
                    if(UNLIKELY(ddbVec->getNullFlag())) {
                        if (UNLIKELY(ddbVec->hasNull())) {
                            double *p = (double *) pyVec.mutable_data();
                            double buf[1024];
                            int start = 0;
                            int N = size;
                            while (start < N) {
                                int len = std::min(N - start, 1024);
                                ddbVec->getDouble(start, len, buf);
                                for (int i = 0; i < len; ++i) {
                                    if(UNLIKELY(buf[i] == DBL_NMIN)) {
                                        SET_NPNAN(p + start + i, 1);
                                    }
                                }
                                start += len;
                            }
                        }
                    }
                    Py_INCREF(pyVec.ptr());
                    return pyVec.ptr();
                }
                case DT_IP:
                case DT_UUID:
                case DT_INT128: {
                    py::array pyVec(py::dtype("object"), {size}, {});
                    for (size_t i = 0; i < size; ++i) {
                        py::str temp(ddbVec->getString(i));
                        Py_IncRef(temp.ptr());
                        memcpy(pyVec.mutable_data(i), &temp, sizeof(py::object));
                    }
                    Py_IncRef(pyVec.ptr());
                    return pyVec.ptr();
                }
                case DT_SYMBOL: {
                    py::array pyVec(py::dtype("object"), {size}, {});
                    int buf[1024];
                    int start = 0;
                    int N = ddbVec->size();
                    SymbolBaseSP pSymbol = ddbVec->getSymbolBase();
                    size_t cnt = 0;
                    while (start < N) {
                        int len = std::min(N - start, 1024);
                        ddbVec->getInt(start, len, buf);
                        for (int i = 0; i < len; ++i) {
                            py::str temp(pSymbol->getSymbol(buf[i]).c_str(), pSymbol->getSymbol(buf[i]).size());
                            Py_IncRef(temp.ptr());
                            memcpy(pyVec.mutable_data(cnt++), &temp, sizeof(py::object));
                        }
                        start += len;
                    }
                    Py_IncRef(pyVec.ptr());
                    return pyVec.ptr();
                }
                case DT_STRING: {
                    py::array pyVec(py::dtype("object"), {size}, {});
                    auto tem = (DolphinString*)ddbVec->getDataArray();
                    for (size_t i = 0; i < size; ++i) {
                        py::str temp(tem[i].c_str(), tem[i].size());
                        Py_IncRef(temp.ptr());
                        memcpy(pyVec.mutable_data(i), &temp, sizeof(py::object));
                    }
                    Py_IncRef(pyVec.ptr());
                    return pyVec.ptr();
                }
                case DT_ANY: {
                    // handle numpy.array of objects
                    auto l = py::list();
                    for (size_t i = 0; i < size; ++i) { l.append(dolphin2py(ddbVec->get(i))); }
                    py::array pyVec(l);
                    Py_INCREF(pyVec.ptr());
                    return pyVec.ptr();
                }
                default: {
                    throw RuntimeException("type error in Vector!" );
                };
            }
        }
        else if (dForm == DF_SET) {
            PyObject *pSet = PySet_New(NULL);
            VectorSP keys = input->keys();
            if(UNLIKELY(keys->getNullFlag())) {
                for (int i = 0; i < size; ++i) {
                    ConstantSP ele = keys->get(i);
                    if(UNLIKELY(ele->isNull())) {
                        PySet_Add(pSet, Py_None);
                    }
                    else {
                        PyObject* tem = dolphin2py(ele);
                        PySet_Add(pSet, tem);
                        Py_DecRef(tem);
                    }
                }
            }
            else {
                for (int i = 0; i < size; ++i) {
                    ConstantSP ele = keys->get(i);
                    PyObject* tem = dolphin2py(ele);
                    PySet_Add(pSet, tem);
                    Py_DecRef(tem);
                    //PySet_Add(pSet, dolphin2py(ele));
                }
            }
            return pSet;
        }
        else if (dForm == DF_DICTIONARY) {
            PyObject *pDict = PyDict_New();
            VectorSP keys = input->keys();
            for (int i = 0; i < size; ++i) {
                ConstantSP key = keys->getItem(i);
                ConstantSP val = input->getMember(key);
                PyObject* tkey, *tval;
                tkey = dolphin2py(key);
                tval = dolphin2py(val);
                //int success = PyDict_SetItem(pDict, dolphin2py(key), dolphin2py(val));
                int success = PyDict_SetItem(pDict, tkey, tval);
                Py_DecRef(tkey);
                Py_DecRef(tval);
                if (success == -1) throw RuntimeException("dolphin2py: Error when creating python dict.");
            }
            return pDict;
        }
        else if (dForm == DF_MATRIX) {
            ConstantSP ddbMat = input;
            size_t rows = ddbMat->rows();
            size_t cols = ddbMat->columns();
            // FIXME: currently only support numerical matrix
            if (ddbMat->getCategory() == MIXED) { throw std::runtime_error("currently only support single typed matrix"); }
            ddbMat->setForm(DF_VECTOR);
            PyObject *tem = dolphin2py(ddbMat);
            py::array pyMat = py::handle(tem).cast<py::array>();
            Py_DecRef(tem);
            ddbMat->setForm(DF_MATRIX);
            pyMat.resize({cols, rows});
            pyMat = pyMat.attr("transpose")();
            Py_IncRef(pyMat.ptr());
            return pyMat.ptr();
        }
        else if (dForm == DF_TABLE) {
            size_t size = input->size();
            size_t columnSize = input->columns();
            TableSP ddbTbl = input;
            py::object index = py::globals()["__builtins__"].attr("range")(size);
            using namespace pybind11::literals;
            PyObject *first = dolphin2py(input->getColumn(0), true);
            py::array pyf = py::handle(first).cast<py::array>();
            auto colName = py::list();
            colName.append(py::str(ddbTbl->getColumnName(0)));
            py::object dataframe = Preserved::pandas_.attr("DataFrame")(pyf, "index"_a = index, "columns"_a =colName);
            Py_DecRef(first);
            for (size_t i = 1; i < columnSize; ++i) {
                PyObject *tem = dolphin2py(input->getColumn(i), true);
                dataframe[ddbTbl->getColumnName(i).data()] = tem;
                Py_DecRef(tem);
            }
            Py_INCREF(dataframe.ptr());
            return dataframe.ptr();
        }
        else if (dForm == DF_PAIR) {
            VectorSP ddbPair = input;
            PyObject* pyPair = PyList_New(size);
            for(int i = 0; i < size; ++i) {
                PyList_SetItem(pyPair, i, dolphin2py(ddbPair->get(i)));
            }
            return pyPair;
        }
    }
    return nullptr;
}

static PY_TYPE getPyTypeEx(py::handle obj) {
    if (py::isinstance(obj,  Preserved::numpy_.attr("ndarray"))) {
        if (py::isinstance(obj, Preserved::numpy_.attr("matrix"))) {
            return PY_NP_MATRIX;
        }
        else {
            return PY_NP_NDARRAY;
       }
    }
    if (py::isinstance(obj, Preserved::pddataframe_)){
        return PY_PD_DATAFRAME;
    }
    if (py::isinstance(obj, Preserved::pdseries_)) {
        return PY_PD_SERIES;
    }
    else if(py::hasattr(obj, "dtype")) {
        py::object type = py::getattr(obj, "dtype");
        if (type.equal(Preserved::npint8_) || type.equal(Preserved::npint16_) || type.equal(Preserved::npint32_) || type.equal(Preserved::npint64_))
            return PY_NP_INT;
        else if (type.equal(Preserved::npfloat32_) || type.equal(Preserved::npfloat64_))
            return PY_FLOAT;
        else if (type.equal(Preserved::npbool_))
            return PY_BOOL;
        else if (type.equal(Preserved::npdatetime64_))
            return PY_NP_DATETIME;
        else if (type.equal(Preserved::npdatetime64M_))
            return PY_NP_DATETIME_M;
        else if (type.equal(Preserved::npdatetime64D_))
            return PY_NP_DATETIME_D;
        else if (type.equal(Preserved::npdatetime64m_))
            return PY_NP_DATETIME_m;
        else if (type.equal(Preserved::npdatetime64s_))
            return PY_NP_DATETIME_s;
        else if (type.equal(Preserved::npdatetime64ms_))
            return PY_NP_DATETIME_ms;
        else if (type.equal(Preserved::npdatetime64us_))
            return PY_NP_DATETIME_us;
        else if (type.equal(Preserved::npdatetime64ns_))
            return PY_NP_DATETIME_ns;
    }
    return PY_NON;
}

static PY_TYPE getPyType(PyObject* obj) {
    if (obj == Py_None) return PY_NONE;
    else if (PyObject_IsInstance(obj, (PyObject*)&PyLong_Type)) return PY_INT;
    else if (PyObject_IsInstance(obj, (PyObject*)&PyFloat_Type)) return PY_FLOAT;
    else if (PyObject_IsInstance(obj, (PyObject*)&PyBool_Type)) return PY_BOOL;
    else if (PyObject_IsInstance(obj, (PyObject*)&PyList_Type)) return PY_LIST;
    else if (PyObject_IsInstance(obj, (PyObject*)&PyTuple_Type)) return PY_TUPLE;
    else if (PyObject_IsInstance(obj, (PyObject*)&PyDict_Type)) return PY_DICT;
    else if (PyObject_IsInstance(obj, (PyObject*)&PySet_Type)) return PY_SET;
    else if (PyObject_IsInstance(obj, (PyObject*)&PyUnicode_Type) || PyObject_IsInstance(obj, (PyObject*)&PyBytes_Type))
        return PY_STRING;
    else return getPyTypeEx(obj);
}

static DATA_TYPE numpyToDolphinDBType(py::array array) {
    py::dtype type = array.dtype();
    if (type.equal(Preserved::npbool_))
        return DT_BOOL;
    else if (type.equal(Preserved::npint8_))
        return DT_CHAR;
    else if (type.equal(Preserved::npint16_))
        return DT_SHORT;
    else if (type.equal(Preserved::npint32_))
        return DT_INT;
    else if (type.equal(Preserved::npint64_))
        return DT_LONG;
    else if (type.equal(Preserved::npfloat32_))
        return DT_FLOAT;
    else if (type.equal(Preserved::npfloat64_))
        return DT_DOUBLE;
    else if (type.equal(Preserved::npdatetime64M_))
        return DT_MONTH;
    else if (type.equal(Preserved::npdatetime64D_))
        return DT_DATE;
    else if (type.equal(Preserved::npdatetime64m_))
        return DT_MINUTE;
    else if (type.equal(Preserved::npdatetime64s_))
        return DT_DATETIME;
    else if (type.equal(Preserved::npdatetime64ms_))
        return DT_TIMESTAMP;
    else if (type.equal(Preserved::npdatetime64us_))
        return DT_NANOTIMESTAMP;
    else if (type.equal(Preserved::npdatetime64ns_))
        return DT_NANOTIMESTAMP;
    else if (type.equal(Preserved::npdatetime64_))    // np.array of null datetime64
        return DT_NANOTIMESTAMP;
    else if (type.equal(Preserved::npobject_))
        return DT_ANY;
    else
        return DT_ANY;
}

ConstantSP py2dolphin(PyObject *input, bool free) {
    PY_TYPE pType = getPyType(input);
    if (pType == PY_STRING) {
        const char *str;
        PyArg_Parse(input, "s", &str);
        return new String(str);
    }
    else if (pType == PY_FLOAT) {
        auto result = py::handle(input).cast<double>();
        long long r;
        memcpy(&r, &result, sizeof(r));
        if (r == 9221120237041090560LL) { result = DBL_NMIN; }
        return new Double(result);
    }
    else if (pType == PY_INT) {
        long long l = py::handle(input).cast<long long>();
        return new Long(l);
    }
    else if (pType == PY_BOOL) {
        bool b;
        PyArg_Parse(input, "p", &b);
        return new Bool(b);
    }
    else if (pType == PY_NONE) {
        return Util::createConstant(DT_VOID);
    }
    else if (pType == PY_NP_INT) {
        long long l = py::handle(input).cast<long long>();
        return new Long(l);
    }
    else if (pType == PY_NP_FLOAT) {
        auto result = py::handle(input).cast<double>();
        long long r;
        memcpy(&r, &result, sizeof(r));
        if (r == 9221120237041090560LL) { result = DBL_NMIN; }
        return new Double(result);
    }
    else if (pType == PY_TUPLE) {
        py::tuple tuple = py::handle(input).cast<py::tuple>();
        size_t size = tuple.size();
        vector<ConstantSP> _ddbVec;
        DATA_TYPE type = DT_VOID;
        DATA_FORM form = DF_SCALAR;
        int types = 0;
        int forms = 1;
        for (size_t i = 0; i < size; ++i) {
            _ddbVec.push_back(py2dolphin(tuple[i].ptr(), false));
            if (_ddbVec.back()->isNull()) { continue; }
            DATA_TYPE tmpType = _ddbVec.back()->getType();
            DATA_FORM tmpForm = _ddbVec.back()->getForm();
            if (tmpType != type) {
                types++;
                type = tmpType;
            }
            if (tmpForm != form) { forms++; }
        }
        if (types >= 2 || forms >= 2) {
            type = DT_ANY;
        } else if (types == 0) {
            throw RuntimeException("can not create all None vector");
        }
        VectorSP ddbVec = Util::createVector(type, 0, size);
        for (size_t i = 0; i < size; ++i) { ddbVec->append(_ddbVec[i]); }
        return ddbVec;
    }
    else if (pType == PY_LIST) {
        py::list list = py::handle(input).cast<py::list>();
        size_t size = list.size();
        vector<ConstantSP> _ddbVec;
        DATA_TYPE type = DT_VOID;
        DATA_FORM form = DF_SCALAR;
        int types = 0;
        int forms = 1;
        for (size_t i = 0; i < size; ++i) {
            _ddbVec.push_back(py2dolphin(list[i].ptr(), false));
            if (_ddbVec.back()->isNull()) { continue; }
            DATA_TYPE tmpType = _ddbVec.back()->getType();
            DATA_FORM tmpForm = _ddbVec.back()->getForm();
            if (tmpType != type) {
                types++;
                type = tmpType;
            }
            if (tmpForm != form) { forms++; }
        }
        if (types >= 2 || forms >= 2) {
            type = DT_ANY;
        } else if (types == 0) {
            throw RuntimeException("can not create all None vector");
        }
        VectorSP ddbVec = Util::createVector(type, 0, size);
        for (size_t i = 0; i < size; ++i) { ddbVec->append(_ddbVec[i]); }
        return ddbVec;
    }
    else if (pType == PY_DICT) {
        py::dict pyDict = py::handle(input).cast<py::dict>();
        size_t size = pyDict.size();
        vector<ConstantSP> _ddbKeyVec;
        vector<ConstantSP> _ddbValVec;
        DATA_TYPE keyType = DT_VOID;
        DATA_TYPE valType = DT_VOID;
        DATA_FORM keyForm = DF_SCALAR;
        DATA_FORM valForm = DF_SCALAR;
        int keyTypes = 0;
        int valTypes = 0;
        int keyForms = 1;
        int valForms = 1;
        for (auto it = pyDict.begin(); it != pyDict.end(); ++it) {
            _ddbKeyVec.push_back(py2dolphin(py::reinterpret_borrow<py::object>(it->first).ptr(), false));
            _ddbValVec.push_back(py2dolphin(py::reinterpret_borrow<py::object>(it->second).ptr(), false));
            if (_ddbKeyVec.back()->isNull() || _ddbValVec.back()->isNull()) { continue; }
            DATA_TYPE tmpKeyType = _ddbKeyVec.back()->getType();
            DATA_TYPE tmpValType = _ddbValVec.back()->getType();
            DATA_FORM tmpKeyForm = _ddbKeyVec.back()->getForm();
            DATA_FORM tmpValForm = _ddbValVec.back()->getForm();
            if (tmpKeyType != keyType) {
                keyTypes++;
                keyType = tmpKeyType;
            }
            if (tmpValType != valType) {
                valTypes++;
                valType = tmpValType;
            }
            if (tmpKeyForm != keyForm) { keyForms++; }
            if (tmpValForm != valForm) { valForms++; }
        }
        if (keyTypes >= 2 || keyType == DT_BOOL || keyForms >= 2) { throw RuntimeException("the key type can not be BOOL or ANY"); }
        if (valTypes >= 2 || valForms >= 2) {
            valType = DT_ANY;
        } else if (keyTypes == 0 || valTypes == 0) {
            throw RuntimeException("can not create all None vector in dictionary");
        }
        VectorSP ddbKeyVec = Util::createVector(keyType, 0, size);
        VectorSP ddbValVec = Util::createVector(valType, 0, size);
        for (size_t i = 0; i < size; ++i) {
            ddbKeyVec->append(_ddbKeyVec[i]);
            ddbValVec->append(_ddbValVec[i]);
        }
        DictionarySP ddbDict = Util::createDictionary(keyType, nullptr, valType, nullptr);
        ddbDict->set(ddbKeyVec, ddbValVec);
        return ddbDict;
    }
    else if (pType == PY_SET) {
        py::set pySet = py::handle(input).cast<py::set>();
        vector<ConstantSP> _ddbSet;
        DATA_TYPE type = DT_VOID;
        DATA_FORM form = DF_SCALAR;
        int types = 0;
        int forms = 1;
        for (auto it = pySet.begin(); it != pySet.end(); ++it) {
            _ddbSet.push_back(py2dolphin(py::reinterpret_borrow<py::object>(*it).ptr(), false));
            if (_ddbSet.back()->isNull()) { continue; }
            DATA_TYPE tmpType = _ddbSet.back()->getType();
            DATA_FORM tmpForm = _ddbSet.back()->getForm();
            if (tmpType != type) {
                types++;
                type = tmpType;
            }
            if (tmpForm != form) { forms++; }
        }
        if (types >= 2 || forms >= 2) {
            throw RuntimeException("set in DolphinDB doesn't support multiple types");
        } else if (types == 0) {
            throw RuntimeException("can not create all None set");
        }
        SetSP ddbSet = Util::createSet(type, nullptr, 0);
        for (auto &v : _ddbSet) { ddbSet->append(v); }
        return ddbSet;
    }
    else if (pType == PY_NP_MATRIX) {
        py::array pyarray = py::handle(input).cast<py::array>();
        int totalNum = pyarray.size();
        if (totalNum == 0) throw RuntimeException("py2dolphin: Empty matrix is not supported.");
        return py2dolphin(pyarray.base().ptr(), false);
    }
    else if (pType == PY_NP_NDARRAY || pType == PY_PD_SERIES) {
        py::array pyVec = py::handle(input).cast<py::array>();
        DATA_TYPE type = numpyToDolphinDBType(pyVec);
        if (UNLIKELY(pyVec.ndim() > 2)) { throw RuntimeException("numpy.ndarray with dimension > 2 is not supported"); }
        if (pyVec.ndim() == 1) {
            size_t size = pyVec.size();
            VectorSP ddbVec;
            ddbVec = Util::createVector(type, 0, size);
            switch (type) {
                case DT_BOOL: {
                    append<bool>(pyVec, size, [&](bool *buf, int size) { ddbVec->appendBool((char *) buf, size); });
                    return ddbVec;
                }
                case DT_CHAR: {
                    append<char>(pyVec, size, [&](char *buf, int size) { ddbVec->appendChar(buf, size); });
                    return ddbVec;
                }
                case DT_SHORT: {
                    //append<short>(pyVec, size, [&](short *buf, int size) { ddbVec->appendShort(buf, size); });
                    pyVec = pyVec.attr("astype")("int16");
                    ddbVec->appendShort((short*)pyVec.data(),size);
                    return ddbVec;
                }
                case DT_INT: {
                    //append<int>(pyVec, size, [&](int *buf, int size) { ddbVec->appendInt(buf, size); });
                    pyVec = pyVec.attr("astype")("int32");
                    ddbVec->appendInt((int*)pyVec.data(),size);
                    return ddbVec;
                }
                case DT_MONTH: {
                    append<long long>(pyVec, size, [&](long long *buf, int size) {
                        for (int i = 0; i < size; ++i) {
                            if(buf[i]!=INT64_MIN){
                                buf[i] += 23640;
                            }
                        }
                        ddbVec->appendLong(buf, size);
                    });
                    return ddbVec;
                }
                case DT_DATE:
                case DT_TIME:
                case DT_MINUTE:
                case DT_SECOND:
                case DT_DATETIME:
                case DT_TIMESTAMP:
                case DT_NANOTIME:
                case DT_NANOTIMESTAMP: {
                    pyVec = pyVec.attr("astype")("int64");
                    ddbVec->appendLong((long long*)pyVec.data(), size);
                    return ddbVec;
                }
                case DT_LONG: {
                    //append<long long>(pyVec, size, [&](long long *buf, int size) { ddbVec->appendLong(buf, size); });
                    ddbVec->appendLong((long long*)pyVec.data(), size);
                    return ddbVec;
                }
                case DT_FLOAT: {
                    //append<float>(pyVec, size, [&](float *buf, int size) { ddbVec->appendFloat(buf, size); });
                    pyVec = pyVec.attr("astype")("float32");
                    ddbVec->appendFloat((float *)pyVec.data(),size);
                    float buf[1024];
                    int start = 0;
                    int n = size;
                    bool containNull = false;
                    while (start < n) {
                        int len = std::min(n - start, 1024);
                        float * p = ddbVec->getFloatBuffer(start, len, buf);
                        bool changed = false;
                        for (int i = 0; i < len; ++i) {
                            if(std::isnan(p[i])) {
                                p[i] = FLT_NMIN;
                                changed = true;
                            }
                        }
                        if(changed) {
                            ddbVec->setFloat(start, len, p);
                            containNull = true;
                        }
                        start += len;
                    }
                    ddbVec->setNullFlag(containNull);
                    return ddbVec;
                }
                case DT_DOUBLE: {
                    // special handle for np.nan value as type(np.nan)=float
                    //append<double>(pyVec, size, [&](double *buf, int size) { ddbVec->appendDouble(buf, size); });
                    pyVec = pyVec.attr("astype")("float64");
                    ddbVec->appendDouble((double *)pyVec.data(),size);
                    double buf[1024];
                    int start = 0;
                    int n = size;
                    bool containNull = false;
                    while (start < n) {
                        int len = std::min(n - start, 1024);
                        double * p =(double*) ddbVec->getDoubleBuffer(start, len, buf);
                        long long* lp = (long long*)p;
                        bool changed = false;
                        for (int i = 0; i < len; ++i) {
                            if(lp[i] == 9221120237041090560LL) {
                                p[i] = DBL_NMIN;
                                changed = true;
                            }
                        }
                        if(changed) {
                            ddbVec->setDouble(start, len, p);
                            containNull = true;
                        }
                        start += len;
                    }
                    ddbVec->setNullFlag(containNull);
                    return ddbVec;
                }
                case DT_SYMBOL:
                case DT_STRING:
                case DT_ANY: {
                    // extra check (determine string vector or any vector)
                    type = DT_STRING;
                    bool isAny = false;
                    for (auto it = pyVec.begin(); it != pyVec.end(); ++it) {
                        if (!py::isinstance(*it, py::str().get_type())) {
                            //                                type = DT_ANY;
                            isAny = true;
                            break;
                        }
                    }
                    ddbVec = Util::createVector(type, 0, size);
                    if (isAny) {
                        for (auto it = pyVec.begin(); it != pyVec.end(); ++it) {
                            ConstantSP item = py2dolphin(py::reinterpret_borrow<py::object>(*it).ptr());
                            ddbVec->append(new String(item->getString()));
                        }
                    } else {
                        vector<std::string> strs;
                        for (auto it = pyVec.begin(); it != pyVec.end(); ++it) {
                            strs.emplace_back(py::reinterpret_borrow<py::str>(*it).cast<std::string>());
                        }
                        ddbVec->appendString(strs.data(), strs.size());
                    }
                    return ddbVec;
                }
                default: {
                    throw RuntimeException("type error in numpy!");
                }
            }
        }
        else {
            size_t rows = pyVec.shape(0);
            size_t cols = pyVec.shape(1);
            pyVec = Preserved::numpy_.attr("array")(pyVec);
//                pyVec = Preserved::numpy_.attr("array")(pyVec.attr("transpose")()).attr("flatten")();
            pyVec = pyVec.attr("transpose")().attr("reshape")(pyVec.size());
            ConstantSP ddbVec = py2dolphin(pyVec.ptr());
            // FIXME: consider batch?
            ConstantSP ddbMat = Util::createMatrix(type, cols, rows, cols);
            for (size_t i = 0; i < cols; ++i) {
                for (size_t j = 0; j < rows; ++j) { ddbMat->set(i, j, ddbVec->get(i * rows + j)); }
            }
            return ddbMat;
            //py::object pytup = py::handle(input).cast<py::tuple>();
            //return py2dolphin(pytup.ptr(), false);
        }
    }
    else if (pType == PY_PD_DATAFRAME) {
        py::object dataframe = py::handle(input).cast<py::object>();
        py::object pyLabel = dataframe.attr("columns");
        //py::dict typeIndicators = py::getattr(dataframe, "__DolphinDB_Type__", py::dict());
        size_t columnSize = pyLabel.attr("size").cast<size_t>();
        vector<std::string> columnNames;
        columnNames.reserve(columnSize);
        static py::object stringType = py::eval("str");
        for (auto it = pyLabel.begin(); it != pyLabel.end(); ++it) {
            if (!py::isinstance(*it, stringType)) {
                throw RuntimeException("DolphinDB only support string as column names, and each of them must be a valid variable name.");
            }
            auto cur = it->cast<string>();
            if (!Util::isVariableCandidate(cur)) {
                throw RuntimeException("'" + cur + "' is not a valid variable name, thus can not be used as a column name in DolphinDB.");
            }
            columnNames.emplace_back(cur);
        }
        vector<ConstantSP> columns;
        columns.reserve(columnSize);
        for (size_t i = 0; i < columnSize; ++i) {
            columns.emplace_back(py2dolphin(dataframe[columnNames[i].data()].ptr() ,false));
            //columns.emplace_back(py2dolphin(py::array(dataframe[columnNames[i].data()]).ptr(), false));
        }
        TableSP ddbTbl = Util::createTable(columnNames, columns);
        return ddbTbl;
    }
    else if (pType == PY_NP_DATETIME) {
        auto re = py::handle(input).attr("astype")("float64").cast<double>();
        if (re == 9221120237041090560LL) { return new Void; }
        return new DateTime(re);
    }
    else if (pType == PY_NP_DATETIME_D) {
        auto re = py::handle(input).attr("astype")("float64").cast<double>();
        if (re == 9221120237041090560LL) { return new Void; }
        return new Date(re);
    }
    else if (pType == PY_NP_DATETIME_M) {
        auto re = py::handle(input).attr("astype")("float64").cast<double>();
        if (re == 9221120237041090560LL) { return new Void; }
        return new Month(1970, re + 1);
    }
    else if (pType == PY_NP_DATETIME_m) {
        auto re = py::handle(input).attr("astype")("float64").cast<double>();
        if (re == 9221120237041090560LL) { return new Void; }
        return new Minute(re);
    }
    else if (pType == PY_NP_DATETIME_s) {
        auto re = py::handle(input).attr("astype")("float64").cast<double>();
        if (re == 9221120237041090560LL) { return new Void; }
        return new DateTime(re);
    }
    else if (pType == PY_NP_DATETIME_ms) {
        auto re = py::handle(input).attr("astype")("float64").cast<double>();
        if (re == 9221120237041090560LL) { return new Void; }
        return new Timestamp(re);
    }
    else if (pType == PY_NP_DATETIME_us) {
        auto re = py::handle(input).attr("astype")("float64").cast<double>();
        if (re == 9221120237041090560LL) { return new Void; }
        return new NanoTimestamp(re * 1000ll);
    }
    else if (pType == PY_NP_DATETIME_ns) {
        auto re = py::handle(input).attr("astype")("float64").cast<double>();
        if (re == 9221120237041090560LL) { return new Void; }
        return new NanoTimestamp(re);
    }
    else
        return nullptr;
}
