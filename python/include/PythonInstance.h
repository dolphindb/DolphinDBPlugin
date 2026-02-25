#pragma once

#include "DolphinDBEverything.h"
#include "ScalarImp.h"
#include "DolphinClass.h"
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <pybind11/embed.h>
#define VISIBILITY __attribute__((visibility("default")))

namespace py = pybind11;

namespace ddb {
class PythonClass;
class PythonInstance;
typedef ObjectPtr<PythonClass> PythonClassSP;
typedef ObjectPtr<PythonInstance> PythonInstanceSP;
typedef ObjectPtr<DolphinClass> DolphinClassSP;

class VISIBILITY PythonClass : public DolphinClass {
public:
    PythonClass(const string &instName);
    PythonClass(const string& qualifier, const string &instName);
    ~PythonClass() = default;

    bool hasMethod(const std::string &name) const override {
        if (name == "repr" || name == "str") {
            return true;
        }
        return false;
    }
    virtual ConstantSP getValue() const override;
    void setName(const std::string& name){ name_ = name; }
    static OOClassSP createPyClass(const string& qualifier, const string& name) {
        return new PythonClass(qualifier, name);
    }
    static PythonClassSP global_class;
};

class VISIBILITY PythonInstance : public DolphinInstance {
  public:
    PythonInstance(py::object obj, const PythonClassSP &ddbClass)
        : DolphinInstance(ddbClass), object_(std::move(obj)) {}
    ~PythonInstance() {
        const py::gil_scoped_acquire acquire;
        object_ = py::object();
    }

    virtual string getString() const override { 
        py::gil_scoped_acquire acquire;
        py::function obj = py::reinterpret_borrow<py::function>(object_.attr("__str__"));
        return py::str(obj()); 
    }

    bool hasMethod(const std::string &name) const override {
        if (name == "repr") {
            return true;
        }
        return false;
    }
    virtual string getScript() const override { return getString(); }
    py::object getPyObject() const { return object_; };
    virtual ConstantSP getValue() const override;
    virtual ConstantSP getMember(const string& key) const override;
    virtual FunctionDefSP getMethod(const string& name) const override;
    virtual FunctionDefSP getOperator(const string& name) const override;

private:
    py::object object_;
};

} // namespace ddb
