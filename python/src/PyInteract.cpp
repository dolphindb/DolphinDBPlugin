#include "PyInteract.h"

#include "PyInterUtil.h"
#include "PythonInstance.h"

#include "TypeConverter.h"
#include "TypeDefine.h"
#include "TypeHelper.h"

#include "PluginLogger.h"

#include "CoreConcept.h"
#include "Exceptions.h"
#include "ScalarImp.h"
#include "Types.h"
#include "Util.h"

#include <pybind11/embed.h>
#include <pybind11/gil.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>

#include <atomic>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace py = pybind11;

using converter::PyObjs;
using converter::decodeUtf8Text;
using converter::Converter;
using converter::Type;
using converter::CHILD_VECTOR_OPTION;
using converter::ARRAY_OPTION;
using converter::getConstantSP_NULL;
using converter::TableChecker;
using converter::VectorInfo;
using converter::HT_UNK;

std::atomic<bool> AutoConvert{false}; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

class PythonInitial {
  public:
    static PythonInitial &GetInstance() {
        static PythonInitial instance;
        return instance;
    }

  private:
    PythonInitial() {
        py::initialize_interpreter();
        try {
            const py::module_ numpy = py::module_::import("numpy");
            const py::module_ pandas = py::module_::import("pandas");
        } catch (const std::exception& e) {
            throw RuntimeException(std::string("Failed to import required Python modules: ") + e.what());
        }
        PyObjs::Initialize();
        PyEval_SaveThread();
    }
};

ConstantSP initialize(Heap *heap, std::vector<ConstantSP> &arguments) {
    std::ignore = heap;
    std::ignore = arguments;
    PythonInitial::GetInstance();
    return Util::createConstant(DT_VOID);
}

ConstantSP call(Heap *heap, std::vector<ConstantSP> &args) {
    std::ignore = heap;
    auto *instance = dynamic_cast<PythonInstance *>(args[0].get());
    if (instance == nullptr) {
        throw IllegalArgumentException(__func__, "[object] must be a python object.");
    }
    if (args.size() == 1) {
        args.push_back(new String(""));
    }
    std::swap(args[0], args[1]);
    const std::string name = args[0]->getString();
    const py::gil_scoped_acquire acquire;
    PythonFunctionDef method(instance->getPyObject().attr(name.c_str()), name);
    std::vector<ConstantSP> pyArgs(args.begin() + 1, args.end());

    return method.call(heap, pyArgs);
}

ConstantSP attr(Heap *heap, std::vector<ConstantSP> &args) {
    std::ignore = heap;
    auto *instance = dynamic_cast<PythonInstance *>(args[0].get());
    if (instance == nullptr) {
        throw IllegalArgumentException(__func__, "[object] must be a python object.");
    }
    if (!args[1]->isScalar() || args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException(__func__, "[attrName] must be a string scalar.");
    }
    std::string attrName = args[1]->getString();
    const py::gil_scoped_acquire acquire;
    const py::object obj = instance->getPyObject().attr(attrName.c_str());
    return new PythonInstance(obj);
}

ConstantSP repr(Heap *heap, std::vector<ConstantSP> &args) {
    std::ignore = heap;
    auto *instance = dynamic_cast<PythonInstance *>(args[0].get());
    if (instance == nullptr) {
        throw IllegalArgumentException(__func__, "[object] must be a python object.");
    }
    const py::gil_scoped_acquire acquire;
    const py::object obj = instance->getPyObject().attr("__repr__");
    return converter::Converter::toDolphinDB(obj()); 
}

ConstantSP pyNone(Heap *heap, std::vector<ConstantSP> &arguments) {
    std::ignore = heap;
    std::ignore = arguments;

    const py::gil_scoped_acquire acquire;
    return new PythonInstance(py::none());
}

ConstantSP pyInt(Heap *heap, std::vector<ConstantSP> &arguments) {
    std::ignore = heap;
    const auto &value = arguments[0];
    if (!value->isScalar()) {
        throw IllegalArgumentException("pyInt", "[value] must be a scalar.");
    }

    const py::gil_scoped_acquire acquire;
    return new PythonInstance(py::int_(value->getLong()));
}

ConstantSP pyFloat(Heap *heap, std::vector<ConstantSP> &arguments) {
    std::ignore = heap;
    const auto &value = arguments[0];
    if (!value->isScalar()) {
        throw IllegalArgumentException(__func__, "[value] must be a scalar.");
    }

    const py::gil_scoped_acquire acquire;
    return new PythonInstance(py::float_(value->getDouble()));
}

ConstantSP pyBool(Heap *heap, std::vector<ConstantSP> &arguments) {
    std::ignore = heap;
    const auto &value = arguments[0];
    if (!value->isScalar()) {
        throw IllegalArgumentException(__func__, "[value] must be a scalar.");
    }

    const py::gil_scoped_acquire acquire;
    return new PythonInstance(py::bool_(value->getBool() != 0));
}

ConstantSP pyStr(Heap *heap, std::vector<ConstantSP> &arguments) {
    std::ignore = heap;
    const auto &value = arguments[0];
    const std::string str = value->getString();

    const py::gil_scoped_acquire acquire;
    PyObject *tmp = decodeUtf8Text(str.data(), static_cast<int>(str.size()));
    return new PythonInstance(py::reinterpret_steal<py::object>(tmp));
}

ConstantSP pyBytes(Heap *heap, std::vector<ConstantSP> &arguments) {
    std::ignore = heap;
    const auto &value = arguments[0];
    const std::string str = value->getString();

    const py::gil_scoped_acquire acquire;
    return new PythonInstance(py::bytes(str.data(), str.size()));
}

ConstantSP toPy(Heap *heap, std::vector<ConstantSP> &arguments) {
    std::ignore = heap;
    const auto &data = arguments[0];
    if (data->isNull()) {
        throw IllegalArgumentException(__func__, "Cannot convert null value to python object.");
    }

    const py::gil_scoped_acquire acquire;
    const py::object res = Converter::toPython(data);
    return new PythonInstance(res);
}

ConstantSP fromPy(Heap *heap, std::vector<ConstantSP> &arguments) {
    std::ignore = heap;
    const auto &data = arguments[0];
    if (data->isNull()) {
        throw IllegalArgumentException(__func__, "Input is not python object");
    }

    auto *instance = dynamic_cast<PythonInstance *>(data.get());
    if (instance == nullptr) {
        throw IllegalArgumentException(__func__, "Input is not python object");
    }
    const py::gil_scoped_acquire acquire;

    return Converter::toDolphinDB(instance->getPyObject());
}

ConstantSP toNumpy(Heap *heap, std::vector<ConstantSP> &arguments) {
    std::ignore = heap;
    const auto &data = arguments[0];
    const auto form = data->getForm();
    if (form != DF_VECTOR && form != DF_MATRIX) {
        throw IllegalArgumentException(__func__, "[data] must be a vector or matrix.");
    }

    const py::gil_scoped_acquire acquire;
    py::object res;
    if (form == DF_VECTOR) {
        res = Converter::toNumpy_Vector(data);
    } else {
        res = Converter::toNumpy_Matrix(data);
    }
    return new PythonInstance(res);
}

ConstantSP fromNumpy(Heap *heap, std::vector<ConstantSP> &args) {
    std::ignore = heap;
    auto *instance = dynamic_cast<PythonInstance *>(args[0].get());
    if (instance == nullptr) {
        throw IllegalArgumentException(__func__, "Input is not python object");
    }
    const py::gil_scoped_acquire acquire;

    auto obj = instance->getPyObject();
    const Type type{HT_UNK, EXPARAM_DEFAULT};
    if (CHECK_INS(obj, np_array_)) {
        return Converter::toDolphinDB_VectorOrMatrix_fromNDArray(
            py::reinterpret_borrow<py::array>(obj), type, CHILD_VECTOR_OPTION::ANY_VECTOR, ARRAY_OPTION::AO_UNSPEC);
    }
    throw IllegalArgumentException(__func__, "[obj] cannot be converted to DolphinDB data: unknown python type.");

    return getConstantSP_NULL();
}

ConstantSP toPandas(Heap *heap, std::vector<ConstantSP> &arguments) {
    std::ignore = heap;
    const auto &data = arguments[0];
    const auto form = data->getForm();
    if (form != DF_VECTOR && form != DF_TABLE) {
        throw IllegalArgumentException(__func__, "[data] must be a vector or table.");
    }

    const py::gil_scoped_acquire acquire;
    py::object res;
    if (form == DF_VECTOR) {
        res = Converter::toPandas_Vector(data);
    } else {
        res = Converter::toPandas_Table(data);
    }
    return new PythonInstance(res);
}

ConstantSP fromPandas(Heap *heap, std::vector<ConstantSP> &args) {
    std::ignore = heap;
    const auto &data = args[0];
    if (data->isNull()) {
        throw IllegalArgumentException(__func__, "Input is not python object");
    }

    auto *instance = dynamic_cast<PythonInstance *>(data.get());
    if (instance == nullptr) {
        throw IllegalArgumentException(__func__, "Input is not python object");
    }
    const py::gil_scoped_acquire acquire;

    auto obj = instance->getPyObject();
    const Type type{HT_UNK, EXPARAM_DEFAULT};
    if (CHECK_INS(obj, pd_dataframe_)) {
        return Converter::toDolphinDB_Table_fromDataFrame(obj, TableChecker(py::dict()));
    }
    if (CHECK_INS(obj, pd_series_)) {
        return Converter::toDolphinDB_Vector_fromSeriesOrIndex(obj, type, CHILD_VECTOR_OPTION::ANY_VECTOR,
                                                               VectorInfo());
    }
    if (CHECK_INS(obj, pd_index_)) {
        return Converter::toDolphinDB_Vector_fromSeriesOrIndex(obj, type, CHILD_VECTOR_OPTION::ANY_VECTOR,
                                                               VectorInfo());
    }
    throw IllegalArgumentException(__func__, "[obj] cannot be converted to DolphinDB data: unknown python type.");

    return getConstantSP_NULL();
}

/*
 * import python module
 * args.size = 1
 * args[0]:<string> name of the module
 * return PythonInstance
 */
ConstantSP importModule(Heap *heap, std::vector<ConstantSP> &arguments) {
    std::ignore = heap;
    if (arguments[0]->getType() != DT_STRING) {
        throw IllegalArgumentException("importModule", "Input is not a string");
    }

    const std::string name = arguments[0]->getString();
    const py::gil_scoped_acquire acquire;
    py::object mod;

    try {
        mod = py::module_::import(name.c_str());
    } catch (const py::error_already_set &e) {
        throw RuntimeException(std::string("Failed to import module ") + name + ": " + e.what());
        return new Void();
    }

    LOG_INFO("Imported ", name);
    return new PythonInstance(mod);
}

template <typename T>
ConstantSP pyContainer(const ConstantSP &data)
{
    if (!data->isArray() && !data->isTuple()) {
        throw IllegalArgumentException(__func__, "[data] must be a vector or tuple.");
    }

    const py::gil_scoped_acquire acquire;
    size_t size = data->size();
    py::list res = py::list(size);
    for (size_t i = 0; i < size; ++i) {
        auto *instance = dynamic_cast<PythonInstance *>(data->get(i).get());
        if (instance == nullptr) {
            res[i] = Converter::toPython(data->get(i));
        } else {
            res[i] = instance->getPyObject();
        }
    }
    return new PythonInstance(T(res));
}

ConstantSP pyTuple(Heap *heap, std::vector<ConstantSP> &args) {
    std::ignore = heap;
    return pyContainer<py::tuple>(args[0]);
}

ConstantSP pyList(Heap *heap, std::vector<ConstantSP> &args) {
    std::ignore = heap;
    return pyContainer<py::list>(args[0]);
}

ConstantSP pySet(Heap *heap, std::vector<ConstantSP> &args) {
    std::ignore = heap;
    const auto &data = args[0];
    if (!data->isSet() && !data->isArray() && !data->isTuple()) {
        throw IllegalArgumentException(__func__, "[data] must be a set or vector or tuple.");
    }
    if (!data->isSet()) {
        return pyContainer<py::set>(data);
    }
    return pyContainer<py::set>(data->keys());
}

ConstantSP pyDict(Heap *heap, std::vector<ConstantSP> &args) {
    std::ignore = heap;
    if (args.size() == 1) {
        const auto &data = args[0];
        if (!data->isDictionary() && !data->isTable()) {
            throw IllegalArgumentException(__func__, "[data] must be a dictionary or table.");
        }

        const py::gil_scoped_acquire acquire;
        py::object res;
        if (data->isDictionary()) {
            res = Converter::toPyDict_Dictionary(data);
        } else {
            res = Converter::toPyDict_Table(data);
        }
        return new PythonInstance(res);
    }

    const auto &keys = args[0];
    const auto &values = args[1];
    if (!keys->isArray() || !values->isArray() || keys->size() != values->size()) {
        throw IllegalArgumentException(__func__, "[keys] and [values] must be vectors of the same size.");
    }

    const py::gil_scoped_acquire acquire;
    const py::dict py_dict;
    const auto size = keys->size();
    for (int i = 0; i < size; ++i) {
        py_dict[Converter::toPython_Scalar(keys->get(i))] = Converter::toPython(values->get(i));
    }
    return new PythonInstance(py_dict);
}

ConstantSP pythonType(Heap *heap, std::vector<ConstantSP> &arguments) {
    std::ignore = heap;

    auto *instance = dynamic_cast<PythonInstance *>(arguments[0].get());
    if (instance == nullptr) {
        throw IllegalArgumentException("pythonType", "Input is not python object");
    }
    const py::gil_scoped_acquire acquire;

    auto obj = instance->getPyObject();
    const py::object type = obj.attr("__class__");

    const py::str type_name = py::str(type);

    const auto res = type_name.cast<std::string>();
    return new String(res);
}

ConstantSP pythonDir(Heap *heap, std::vector<ConstantSP> &arguments) {
    std::ignore = heap;

    auto *instance = dynamic_cast<PythonInstance *>(arguments[0].get());
    if (instance == nullptr) {
        throw IllegalArgumentException("pythonDir", "Input is not python object");
    }
    const py::gil_scoped_acquire acquire;

    const py::object result = py::module_::import("builtins").attr("dir")(instance->getPyObject());

    return Converter::toDolphinDB(result);
}

ConstantSP setGlobalConfig(Heap *heap, std::vector<ConstantSP> &arguments) {
    std::ignore = heap;
    if (!arguments[0]->isScalar() || arguments[0]->getType() != DT_STRING) {
        throw IllegalArgumentException("py::setGlobalConfig", "The first argument must be a string.");
    }
    const py::gil_scoped_acquire acquire;

    const auto key = arguments[0]->getString();
    const auto &value = arguments[1];
    if (key == "autoConvert") {
        if (value->getType() != DT_BOOL || value->getForm() != DF_SCALAR || value->isNull()) {
            throw IllegalArgumentException(__func__,
                                           "value of key 'autoConvert' must be boolean scalar and cannot be NULL.");
        }
        AutoConvert = (value->getBool() == 1);
    } else {
        throw IllegalArgumentException(
            __func__, "key must be 'autoConvert'.");
    }

    return new Void();
}
