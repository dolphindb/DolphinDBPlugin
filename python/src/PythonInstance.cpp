#include "PythonInstance.h"

#include "PyInterUtil.h"

#include "CoreConcept.h"
#include "DolphinClass.h"
#include "Exceptions.h"
#include "ScalarImp.h"
#include "SmartPointer.h"

#include <pybind11/cast.h>
#include <pybind11/gil.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>

#include <string>
#include <vector>

namespace ddb {

using PythonClassSP = ObjectPtr<PythonClass>;
using PythonInstanceSP = ObjectPtr<PythonInstance>;

PythonClass::PythonClass(const string &instName) : DolphinClass("PythonPlugin", instName) {}
PythonClass::PythonClass(const string &qualifier, const string &instName) : DolphinClass(qualifier, instName) {}

ConstantSP PythonClass::getValue() const {
    const py::gil_scoped_acquire acquire;
    return new PythonClass(name_);
}

ConstantSP PythonInstance::getValue() const {
    const py::gil_scoped_acquire acquire;
    return new PythonInstance(object_, PythonClass::global_class);
}

ConstantSP PythonInstance::getMember(const string &key) const {
    const py::gil_scoped_acquire acquire;

    try {
        const py::object obj = object_.attr(key.c_str());
        return new PythonInstance(obj, PythonClass::global_class);
    } catch (const py::error_already_set& e) {
        throw RuntimeException(std::string("error accessing attribute: ") + e.what());
    }
    return new Void();
}

FunctionDefSP PythonInstance::getMethod(const string &name) const {
    const py::gil_scoped_acquire acquire;

    py::object method;

    // DolphinDB doc says the parser executes x.str() when you try to run "print(x)" in script.
    // But print(x) works even if we don't provide the function.

    if (name == "repr") {
        // DolphinDB parser executes x.repr() when you try to run "x" in script.
        method = object_.attr("__repr__");
    } else if (!name.empty()) {
        method = object_.attr(name.c_str());
    } else {
        method = object_;
    }

    PythonFunctionDefSP pyfunction = new PythonFunctionDef(method, name);
    return pyfunction;
}

FunctionDefSP PythonInstance::getOperator(const string &name) const {
    return getMethod(name);
}

} // namespace ddb
