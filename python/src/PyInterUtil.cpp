#include "PyInterUtil.h"
#include "Util.h"
#include "converter.h"
#include "PyInterUtil.h"

ConstantSP PythonFunctionDef::call(Heap* pHeap, vector<ConstantSP>& arguments) {
    std::ignore = pHeap;
    py::gil_scoped_acquire acquire;
    std::vector<py::object> args;
    args.reserve(arguments.size());

    for (size_t i = 1; i < arguments.size(); i++) {
        if (dynamic_cast<PythonInstance*>(arguments[i].get()) != nullptr) {
            PythonInstanceSP csp(arguments[i]);
            auto obj = csp->getPyObject();
            args.push_back(obj);
        } else {
            if (!AutoConvert) {
                throw IllegalArgumentException(
                    "Invalid argument type", "When autoConvert is set to false, all arguments must be Python objects.");
            }
            py::object pyinstance = converter::Converter::toPython(arguments[i]);
            args.push_back(pyinstance);
        }
    }
    py::object res = object_(*py::args(py::cast(args)));

    if (getName() == "repr" || getName() == "str") {
        return converter::Converter::toDolphinDB(res);
    }
    return new PythonInstance(res);
}

ConstantSP PythonFunctionDef::call(Heap* pHeap, const ConstantSP& a, const ConstantSP& b) {
    std::vector<ConstantSP> args;
    args.push_back(a);
    args.push_back(b);
    return call(pHeap, args);
}

ConstantSP PythonFunctionDef::call(Heap* pHeap,vector<ObjectSP>& arguments) {
    py::gil_scoped_acquire acquire;

    std::vector<py::object> args;
    args.reserve(arguments.size());

    for (size_t i = 1; i < arguments.size(); i++) {
        if(dynamic_cast<PythonInstance*>(arguments[i].get()) != nullptr || arguments[1]->isVariable()){
            ConstantSP csp(arguments[i]->getValue(pHeap));
            PythonInstanceSP pp = (PythonInstanceSP)(csp);
            auto obj = pp->getPyObject();
            args.push_back(obj); 
        }else{
            if (!AutoConvert) {
                throw IllegalArgumentException(
                    "Invalid argument type", "When autoConvert is set to false, all arguments must be Python objects.");
            }
            py::object pyinstance = converter::Converter::toPython(arguments[i]);
            args.push_back(pyinstance); 
        }
    }

    py::object res = object_(*py::args(py::cast(args)));

    return new PythonInstance(res);
}
