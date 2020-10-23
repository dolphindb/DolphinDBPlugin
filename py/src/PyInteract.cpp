#include "PyInteract.h"
#include "TypeConversion.h"
#include "Util.h"
#include "Exceptions.h"
#include "ScalarImp.h"
#include <string>

#include <pybind11/embed.h>
#include "PyResource.h"
#include "Protect.h"
//#include "PyInterUtil.h"

namespace py = pybind11;
bool initFlag = false;
static py::scoped_interpreter guard{};

static void pyObjectOnClose(Heap *heap, vector<ConstantSP> &args) {
    ProtectGil proGil;
    PyObject *obj = reinterpret_cast<PyObject *>(args[0]->getLong());
    if (obj != nullptr) {
        Py_DecRef(obj);
        args[0]->setLong(0);
    }
}

static void doNothing(Heap *heap, vector<ConstantSP> &args) {
}

static void init() {
    if (!initFlag) {
        if(!Py_IsInitialized()) {

            Py_Initialize();
        }
        PyEval_InitThreads();
        PyEval_ReleaseThread(PyThreadState_Get());
        initFlag = true;
    }
}

/*
 * run the python command
 * args.size() = 1
 * args[0]:<string> command
 * return <void>
*/
ConstantSP runCommand(Heap* heap, vector<ConstantSP>& arguments) {
    if (arguments[0]->getType() != DT_STRING)
        throw IllegalArgumentException("runCommand", "Input is not a string");
    if (!initFlag) {
        init();
    }
    if (!Py_IsInitialized()) {
        throw RuntimeException("Haven't initialized!");
    }
    string command = arguments[0]->getString();
    ProtectGil proGil;
    PyRun_SimpleStringFlags(command.c_str(), NULL);
    return Util::createConstant(DT_VOID);
}

/*
 * import python module
 * args.size = 1
 * args[0]:<string> name of the module
 * return <resource> ptr_to_module
 */
ConstantSP importModule(Heap* heap, vector<ConstantSP>& arguments) {
    if (arguments[0]->getType() != DT_STRING)
        throw IllegalArgumentException("importModule", "Input is not a string");
    if (!initFlag) {
        init();
    }
    ProtectGil proGil;
    string name = arguments[0]->getString();
    PyObject *pName = PyUnicode_DecodeFSDefault(name.c_str());
    PyObject *pModule = PyImport_Import(pName);
    Py_DECREF(pName);
    if (pModule == nullptr)
        throw RuntimeException("importModule: No such module in your environment");
    //Pack the module and return
    std::unique_ptr<PyObject> pyobj = (std::unique_ptr<PyObject>)pModule;
    FunctionDefSP onClose(Util::createSystemProcedure("importModule onclose()", pyObjectOnClose, 1, 1));
    return Util::createResource((long long)pyobj.release(), "python module", onClose, heap->currentSession());
}

PyObject *getDotNObject(PyObject *obj, string name) {
    size_t s=0, p;
    PyObject *inPyObj, *pyDict, *outPyObj;
    string mname;
    inPyObj = obj;
    ProtectGil proGil;
    while ((p=name.find('.', s))!=string::npos) {
        mname = name.substr(s, p-s);
        pyDict = PyModule_GetDict(inPyObj);
        outPyObj = PyDict_GetItemString(pyDict, mname.c_str());
        if (outPyObj == nullptr)
            return nullptr;
        s = p+1;
        inPyObj = outPyObj;
    }
    mname = name.substr(s);
    pyDict = PyModule_GetDict(inPyObj);
    outPyObj = PyDict_GetItemString(pyDict, mname.c_str());
    return outPyObj;
}

/*
 * Get object from module
 * args.size = 2
 * args[0]: <resource> preloaded module
 * args[1]: <string> name of the object
 * return <resource> ptr_to_object
 */
ConstantSP getObject(Heap* heap, vector<ConstantSP>& arguments) {
    if (arguments[0]->getType() != DT_RESOURCE
        || (arguments[0]->getString() != "python module" && arguments[0]->getString() != "python object"))
        throw IllegalArgumentException("getObject", "First argument should be a module or an object!");
    if (arguments[1]->getType() != DT_STRING)
        throw IllegalArgumentException("getObject", "Second argument should be a string");
    if (!initFlag) {
        init();
    }
    ProtectGil proGil;
    PyObject *inPyObj = reinterpret_cast<PyObject *>(arguments[0]->getLong());
    PyObject *outPyObj;
    SysProc sysProc;
    if (arguments[0]->getString() == "python module") {
        outPyObj = getDotNObject(inPyObj, arguments[1]->getString());
        sysProc = doNothing;
    }
    else {
        outPyObj = PyObject_GetAttrString(inPyObj, arguments[1]->getString().c_str());
        sysProc = pyObjectOnClose;
    }
    if (outPyObj == nullptr)
        throw RuntimeException("getObject: No such object in the module");
    std::unique_ptr<PyObject> pyobj = (std::unique_ptr<PyObject>)outPyObj;
    FunctionDefSP onClose(Util::createSystemProcedure("getObject onclose()", sysProc, 1, 1));
    return Util::createResource((long long)pyobj.release(), "python object", onClose, heap->currentSession());
}

/*
 * Get function from module or object
 * args.size() = 2
 * args[0]:<resource> ptr to module/object
 * args[1]:<string> name of function
 * return <resource> ptr_to_function
 */
ConstantSP getFunction(Heap* heap, vector<ConstantSP>& arguments) {
    if (arguments[0]->getType() != DT_RESOURCE
        || (arguments[0]->getString() != "python module" && arguments[0]->getString() != "python object"))
        throw IllegalArgumentException("getFunction", "First input should be a python module");
    if (arguments[1]->getType() != DT_STRING)
        throw IllegalArgumentException("getFunction", "Second input should be a string");
    if (!initFlag) {
        init();
    }
    ProtectGil proGil;
    PyObject *inPyObj = reinterpret_cast<PyObject *>(arguments[0]->getLong());
    PyObject *pyDict = PyModule_GetDict(inPyObj);
    PyObject *pyFunc = PyDict_GetItemString(pyDict, arguments[1]->getString().c_str());
    if (pyFunc == nullptr)
        throw RuntimeException("getFunc: No such function in this object");
    std::unique_ptr<PyObject> pyobj = (std::unique_ptr<PyObject>)pyFunc;
    FunctionDefSP onClose(Util::createSystemProcedure("getFunction onclose()", doNothing, 1, 1));
    return Util::createResource((long long)pyobj.release(), "python function", onClose, heap->currentSession());
}

/*
 * convert python object to dolphinDB objects
 * args.size = 1 or 2
 * arguments[0]:<resource> python object
 * arguments[1]: <null> or <string> name of attribute
 * if args.size = 1
 *     must be basic one, convert straightly
 * if args.size = 2:PluginDemo
 *     1st arg is high-level obj
 *     2nd arg is basic obj
 *     extract 2nd from 1st and convert
 */
ConstantSP cvtPy2Dol(Heap* heap, vector<ConstantSP>& arguments) {
    if (arguments[0]->isNull() || arguments[0]->getType() != DT_RESOURCE
        || (arguments[0]->getString() != "python object" && arguments[0]->getString() != "python module"))
        throw IllegalArgumentException("cvtPyObject", "First input should be a python object");
    if (!initFlag) {
        init();
    }
    ProtectGil proGil;
    ConstantSP result;
    PyObject *pyObj = reinterpret_cast<PyObject *>(arguments[0]->getLong());
    if (arguments.size() == 1)
        result = py2dolphin(pyObj);
    else {
        if (arguments[1]->getType() != DT_STRING)
            throw IllegalArgumentException("cvtPyObject", "Second input should be a string");
        PyObject *obj = PyObject_GetAttrString(pyObj, arguments[1]->getString().c_str());
        result = py2dolphin(obj);
    }
    if (result.get() == nullptr)
        throw IllegalArgumentException("cvtPyObject", "The object type is not supported");
    return result;
}

/*
 * convert dolphinDB objects to python basic objects
 * args.size = 1
 * args[0]: <ConstantSP> ddb object
 */
ConstantSP cvtDol2Py(Heap* heap, vector<ConstantSP>& arguments) {
    if (!initFlag) {
        init();
    }
    ProtectGil proGil;
    PyObject* res;
    res = dolphin2py(arguments[0]);
    if (res == nullptr)
        throw IllegalArgumentException("cvtDol2Py", "The converting is not supported.");
    std::unique_ptr<PyObject> pyobj = (std::unique_ptr<PyObject>)res;
    //FunctionDefSP onClose(Util::createSystemProcedure("cvtDol2Py onclose()", doNothing, 1, 1));
    FunctionDefSP onClose(Util::createSystemProcedure("cvtDol2Py onclose()", pyObjectOnClose, 1, 1));
    return Util::createResource((long long)pyobj.release(), "python object", onClose, heap->currentSession());
}

/*
 * call a python function and return python object
 * args.size >= 1
 * args[0]: <python function> ptr to the function
 * args[1]~[n]: <python object> ptr to input python objects
 * return: <python object> ptr to output python object
 */
ConstantSP callFunction(Heap* heap, vector<ConstantSP>& arguments) {
    if (arguments[0]->getString() != "python function")
        throw IllegalArgumentException("callFunction", "The first arg should be a python function.");
    if (!initFlag) {
        init();
    }
    ProtectGil proGil;
    int argc = arguments.size();
    int argi = 1;
    PyObject *pArgs = PyTuple_New(argc - argi);
    for (int i = argi; i < argc; ++i) {
        PyObject *arg;
        if (arguments[i]->getString() != "python object") {
            arg = dolphin2py(arguments[i]);
            if (arg == nullptr){
                throw IllegalArgumentException("callFunction", "Invalid argument.");
            }
        } else {
            arg = reinterpret_cast<PyObject *>(arguments[i]->getLong());
            Py_INCREF(arg);
        }
        if (PyTuple_SetItem(pArgs, i-argi, arg) == -1){
            throw RuntimeException("callFunction: Error when creating input args.");
        }
    }
    PyObject *pFunc = reinterpret_cast<PyObject *>(arguments[0]->getLong());
    PyObject *res = PyObject_CallObject(pFunc, pArgs);
    Py_DECREF(pArgs);
    if (res == nullptr)
        throw IllegalArgumentException("callFunction", "Error when calling python function.");
    FunctionDefSP onClose(Util::createSystemProcedure("callFunction onclose()", doNothing, 1, 1));
    return pyObjectRet(res, heap->currentSession(), onClose);
}

/*
 * get instance of python class
 * args.size = 1
 * args[0]: <resource> preloaded python class object
 * return: <resource> ptr_to_instance
 */
ConstantSP getInstance(Heap* heap, vector<ConstantSP>& arguments) {
    if (arguments.size() != 1)
        throw IllegalArgumentException("getInstance", "Please input 1 variable.");
    if (!initFlag) {
        init();
    }
    ProtectGil proGil;
    PyObject *pyClass = reinterpret_cast<PyObject *>(arguments[0]->getLong());
    PyObject *pInstance;
    pInstance = PyObject_CallObject(pyClass, nullptr);
    if (pInstance == nullptr) {
        throw RuntimeException("getInstance: Get class instance failed");
    }
    SysProc sysProc = doNothing;
    sysProc = pyObjectOnClose;
    std::unique_ptr<PyObject> pyobj = (std::unique_ptr<PyObject>)pInstance;
    FunctionDefSP onClose(Util::createSystemProcedure("getInstance onclose()", sysProc, 1, 1));
    return Util::createResource((long long)pyobj.release(), "python class instance", onClose, heap->currentSession());
}

/*
 * get instance of python class
 * args.size >= 1
 * args[0]: <resource> preloaded python class object
 * args[1]~[n]: <python object> ptr to instance args
 * return: <resource> ptr_to_instance
 */
ConstantSP createObject(Heap* heap, vector<ConstantSP>& arguments) {
    if (!initFlag) {
        init();
    }
    ProtectGil proGil;
    PyObject *pInstance;
    bool isNew;
    if (arguments[0]->getString() == "python class instance"){
        pInstance = reinterpret_cast<PyObject *>(arguments[0]->getLong());
        isNew = false;
    } else {
        int argi = 1, argc = arguments.size();
        PyObject *pArgs = PyTuple_New(argc - argi);
        for (int i = argi; i < argc; ++i){
            PyObject *arg;
            if (arguments[i]->getString() != "python object"){
                arg = dolphin2py(arguments[i]);
                if (arg == nullptr){
                    throw IllegalArgumentException("getInstanceByName", "Invalid argument.");
                }
            } else {
                arg = reinterpret_cast<PyObject *>(arguments[i]->getLong());
                Py_INCREF(arg);
            }
            if (PyTuple_SetItem(pArgs, i-argi, arg) == -1){
                throw RuntimeException("getInstanceByName: Error when creating input args.");
            }
        }
        PyObject *pyClass = reinterpret_cast<PyObject *>(arguments[0]->getLong());
        pInstance = PyObject_CallObject(pyClass, pArgs);
        isNew = true;
        Py_DECREF(pArgs);
    }
    if (pInstance == nullptr) {
        throw IllegalArgumentException("createObject", "Get class instance failed");
    }
    std::unique_ptr<PyObject> pyobj = (std::unique_ptr<PyObject>)pInstance;
    FunctionDefSP onClose(Util::createSystemProcedure("createObject onclose()", doNothing, 1, 1));
    Constant* pyRes = new PyResource((long long)pyobj.release(), "python resource instance", onClose, heap->currentSession(), isNew);
    pyRes->setOOInstance(true);
    return pyRes;
}

/*
 * call python class method
 * args.size >= 2
 * args[0]: <resource> python class or resource instance
 * args[1]: <string> method name
 * args[2]~[n]: <python object> ptr to method args
 * return: <python object> ptr to output python object
 */
ConstantSP callMethod(Heap *heap, vector<ConstantSP>& arguments) {
    if (arguments[0]->getType() != DT_RESOURCE || (arguments[0]->getString() != "python class instance" && arguments[0]->getString() != "python resource instance"))
        throw IllegalArgumentException("callMethod", "First argument should be a python class instance!");
    if (arguments[1]->getType() != DT_STRING)
        throw IllegalArgumentException("callMethod", "Second argument should be a string!");
    if (!initFlag) {
        init();
    }
    ProtectGil proGil;
    string methodName = arguments[1]->getString();
    int argi = 2, argc = arguments.size();
    PyObject *pyClsInst = reinterpret_cast<PyObject *>(arguments[0]->getLong());
    PyObject *pArgs = PyTuple_New(argc - argi);
    for (int i = argi; i < argc; ++i){
        PyObject *arg;
        if (arguments[i]->getString() != "python object"){
            arg = dolphin2py(arguments[i]);
            if (arg == nullptr){
                throw IllegalArgumentException("callMethod", "Invalid argument.");
            }
        } else {
            arg = reinterpret_cast<PyObject *>(arguments[i]->getLong());
            Py_INCREF(arg);
        }
        if (PyTuple_SetItem(pArgs, i-argi, arg) == -1){
            throw RuntimeException("callMethod: Error when creating input args.");
        }
    }
    PyObject *pyFunc = PyObject_GetAttrString(pyClsInst, methodName.c_str());
    if (pyFunc == nullptr)
        throw RuntimeException("callMethod: The class object has no method '"+methodName+"'");
    PyObject *res = PyObject_CallObject(pyFunc, pArgs);
    Py_DECREF(pArgs);
    if (res == nullptr)
        throw RuntimeException("callMethod: Error when calling method.");
    FunctionDefSP onClose(Util::createSystemProcedure("callMethod onclose()", pyObjectOnClose, 1, 1));
    return pyObjectRet(res, heap->currentSession(), onClose);
}

/*
 * get attribute of python class instance
 * args.size = 2
 * args[0]: <resource> python class instance
 * args[1]: <string> attribute name
 * return: <python object> ptr_to_object
 */
ConstantSP getAttr(Heap *heap, vector<ConstantSP>& arguments) {
    if (arguments[0]->getType() != DT_RESOURCE || arguments[0]->getString() != "python class instance")
        throw IllegalArgumentException("getAttr", "First argument should be a python class instance!");
    if (arguments[1]->getType() != DT_STRING)
        throw IllegalArgumentException("getObject", "Second argument should be a string");
    if (!initFlag) {
        init();
    }
    ProtectGil proGil;
    PyObject *pyClsInst, *attr;
    pyClsInst = reinterpret_cast<PyObject *>(arguments[0]->getLong());
    string attrName = arguments[1]->getString();
    attr = PyObject_GetAttrString(pyClsInst, attrName.c_str());
    if (attr == nullptr)
        throw RuntimeException("getAttr: No such attribute in the instance.");
    SysProc sysProc = pyObjectOnClose;
    FunctionDefSP onClose(Util::createSystemProcedure("getAttr onclose()", sysProc, 1, 1));
    return pyObjectRet(attr, heap->currentSession(), onClose);
}

FunctionDefSP getFunctionDol(Heap* heap, vector<ConstantSP>& arguments) {
    FunctionDefSP callFunc = Util::createSystemFunction("callFunction", callFunction, 1, 32, false);
    ConstantSP pyFunc = getFunction(heap, arguments);
    vector<ConstantSP> args = {pyFunc};
    return Util::createPartialFunction(callFunc, args);
}

ConstantSP getInstanceByName(Heap* heap, vector<ConstantSP>& arguments) {
    if (arguments[0]->getType() != DT_RESOURCE
        || (arguments[0]->getString() != "python module" && arguments[0]->getString() != "python object"))
        throw IllegalArgumentException("getInstanceByName", "First argument should be a module or an object!");
    if (arguments[1]->getType() != DT_STRING)
        throw IllegalArgumentException("getInstanceByName", "Second argument should be a string");
    if (!initFlag) {
        init();
    }
    ProtectGil proGil;
    PyObject *inPyObj = reinterpret_cast<PyObject *>(arguments[0]->getLong());
    PyObject *outPyObj;
    if (arguments[0]->getString() == "python module") {
        outPyObj = getDotNObject(inPyObj, arguments[1]->getString());
    }
    else {
        outPyObj = PyObject_GetAttrString(inPyObj, arguments[1]->getString().c_str());
    }
    if (outPyObj == nullptr)
        throw IllegalArgumentException("getInstanceByName", "No such object in the module");

    int argi = 2, argc = arguments.size();
    PyObject *pArgs = PyTuple_New(argc - argi);
    for (int i = argi; i < argc; ++i){
        PyObject *arg;
        if (arguments[i]->getString() != "python object"){
            arg = dolphin2py(arguments[i]);
            if (arg == nullptr){
                throw IllegalArgumentException("getInstanceByName", "Invalid argument.");
            }
        } else {
            arg = reinterpret_cast<PyObject *>(arguments[i]->getLong());
            Py_INCREF(arg);
        }
        if (PyTuple_SetItem(pArgs, i-argi, arg) == -1){
            throw RuntimeException("getInstanceByName: Error when creating input args.");
        }
    }
    PyObject *pInstance;
    pInstance = PyObject_CallObject(outPyObj, pArgs);
    Py_DECREF(pArgs);
    if (pInstance == nullptr) {
        throw RuntimeException("getInstanceByName: Get class instance failed");
    }
    std::unique_ptr<PyObject> pyobj = (std::unique_ptr<PyObject>)pInstance;
    FunctionDefSP onClose(Util::createSystemProcedure("getInstanceByName onclose()", doNothing, 1, 1));
    Constant* pyRes = new PyResource((long long)pyobj.release(), "python resource instance", onClose, heap->currentSession(), true);
    pyRes->setOOInstance(true);
    return pyRes;
}