#pragma once

#include "DolphinDBEverything.h"

#define VISIBILITY __attribute__((visibility("default")))
#define FUNC_VIS __attribute__((unused))

#define PY_FUNC_CALL_LOG PY_LEVEL_LOG(py_log_content)
#include "Util.h"
#include "Types.h"
#include "PythonInstance.h"
#include <iostream>
#include <regex>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace ddb {

class PythonFunctionDef;
class PythonInstance;
class PythonClass;
typedef ObjectPtr<PythonInstance> PyInstanceSP;
typedef ObjectPtr<PythonFunctionDef> PythonFunctionDefSP;

class VISIBILITY PythonFunctionDef : public FunctionDef {
  public:
    PythonFunctionDef(py::object func, const std::string &name)
        : FunctionDef(FUNCTIONDEF_TYPE::USERDEFFUNC, name, 0, 128, true) {
        py::gil_scoped_acquire acquire;
        object_ = std::move(func);
    }
    PythonFunctionDef(const PythonFunctionDef &p) = delete;
    PythonFunctionDef(PythonFunctionDef &&p) = delete;
    PythonFunctionDef &operator=(const PythonFunctionDef &p) = delete;
    PythonFunctionDef &operator=(PythonFunctionDef &&p) = delete;
    ~PythonFunctionDef() {
        py::gil_scoped_acquire acquire;
        object_ = py::object();
    }

    IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const {
        std::ignore = pHeap;
        std::ignore = buffer;
        throw std::runtime_error("unsupport serialize PythonFunctionDef.");
    }
    ConstantSP getInstance() const {throw std::runtime_error("not support getinstance");}
    ConstantSP getValue() const {throw std::runtime_error("not support getvalue");}
    ConstantSP call(Heap* pHeap, vector<ConstantSP>& arguments);
    ConstantSP call(Heap* pHeap, const ConstantSP& a, const ConstantSP& b);
    ConstantSP call(Heap* pHeap, vector<ObjectSP>& arguments);
public:
    py::object object_;
};

class VISIBILITY ReprFunction : public FunctionDef {
public:
    ReprFunction(FUNCTIONDEF_TYPE defType, const string& name, int minParamNum, int maxParamNum, py::handle handle);
    ~ReprFunction();
    IO_ERR serialize(Heap* pHeap, const ByteArrayCodeBufferSP& buffer) const {
        std::ignore = pHeap;
        std::ignore = buffer;
        throw std::runtime_error("unsupport serialize PythonFunctionDef.");
    }
    ConstantSP getInstance() const {throw std::runtime_error("not support getinstance");}
    ConstantSP getValue() const {throw std::runtime_error("not support getvalue");}
    ConstantSP call(Heap* pHeap, vector<ConstantSP>& arguments);
    ConstantSP call(Heap* pHeap, const ConstantSP& a, const ConstantSP& b){
        std::ignore = pHeap;
        std::ignore = a;
        std::ignore = b;
        throw std::runtime_error("not support 2 arguments");
    }
    ConstantSP call(Heap* pHeap, vector<ObjectSP>& arguments);
private:
    py::handle handle_;
};

extern std::atomic<bool> AutoConvert;

} // namespace ddb
