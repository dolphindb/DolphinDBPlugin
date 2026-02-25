#ifndef CONVERTER_TYPEEXCEPTION_H_
#define CONVERTER_TYPEEXCEPTION_H_

#ifndef PYTHON_SWORDFISH
#include "DolphinDBEverything.h"
#endif
#include <pybind11/pybind11.h>
#include <Exceptions.h>

namespace py = pybind11;

namespace pybind_dolphindb {

/**
 *  (BindBaseException, py::Exception) -> BindWarning (Warning)
 *  (BindBaseException, RuntimeException, py::Exception) -> BindException (Error)
 *   |
 *   |--- BindInterfaceException (InterfaceError)
 *   |--- BindDatabaseException (DatabaseError)
 *      |--- BindDataException (DataError)(ConversionError)
 *      |--- BindOperationalException (OperatorError)
 *      |--- BindIntegrityException (IntegrityError)
 *      |--- BindInternalException (InternalError)
 *      |--- BindProgrammingException (ProgrammingError)
 *      |--- BindNotSupportedException (NotSupportedError)
 * 
 */


class BindBaseException {};


class BindWarning : public BindBaseException, public RuntimeException {
public:
    BindWarning(const std::string &errMsg) : RuntimeException(errMsg) {}
};

class BindException : public BindBaseException, public RuntimeException {
public:
    BindException(const std::string &errMsg) : RuntimeException(errMsg) {}
};

class BindInterfaceException : public BindException {
public:
    BindInterfaceException(const std::string &errMsg) : BindException(errMsg) {}
};

class BindDatabaseException : public BindException {
public:
    BindDatabaseException(const std::string &errMsg) : BindException(errMsg) {}
};

class BindDataException : public BindDatabaseException {
public:
    BindDataException(const std::string &errMsg) : BindDatabaseException(errMsg) {}
};

typedef BindDataException ConversionException;

class BindOperationalException : public BindDatabaseException {
public:
    BindOperationalException(const std::string &errMsg) : BindDatabaseException(errMsg) {}
};

class BindIntegrityException : public BindDatabaseException {
public:
    BindIntegrityException(const std::string &errMsg) : BindDatabaseException(errMsg) {}
};

class BindInternalException : public BindDatabaseException {
public:
    BindInternalException(const std::string &errMsg) : BindDatabaseException(errMsg) {}
};

class BindProgrammingException : public BindDatabaseException {
public:
    BindProgrammingException(const std::string &errMsg) : BindDatabaseException(errMsg) {}
};

class BindNotSupportedException : public BindDatabaseException {
public:
    BindNotSupportedException(const std::string &errMsg) : BindDatabaseException(errMsg) {}
};


void Init_Module_Exception(py::module &m);

} // namespace pybind_dolphindb

#endif // CONVERTER_TYPEEXCEPTION_H_
