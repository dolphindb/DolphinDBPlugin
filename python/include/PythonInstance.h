#pragma once

#include "DolphinDBEverything.h"
#include "ScalarImp.h"
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <pybind11/embed.h>
#define VISIBILITY __attribute__((visibility("default")))

namespace py = pybind11;

class VISIBILITY PythonInstance : public String {
  public:
    PythonInstance(py::object obj) : object_(std::move(obj)) {}
    ~PythonInstance() {
        const py::gil_scoped_acquire acquire;
        object_ = py::object();
    }

	DATA_TYPE getRawType() const override { return DT_RESOURCE; };
	ConstantSP getInstance() const override { throw std::logic_error("Python object does not support getInstance."); };

    virtual string getString() const override { 
        py::gil_scoped_acquire acquire;
        py::function obj = py::reinterpret_borrow<py::function>(object_.attr("__str__"));
        return py::str(obj()); 
    }

    virtual string getScript() const override { return getString(); }
    py::object getPyObject() const { return object_; };
    virtual ConstantSP getValue() const override;
    virtual ConstantSP getMember(const string& key) const;
    virtual FunctionDefSP getMethod(const string& name) const;
    virtual FunctionDefSP getOperator(const string& name) const;

private:
    py::object object_;
};

typedef ObjectPtr<PythonInstance> PythonInstanceSP;
