#include "TypeException.h"

namespace pybind_dolphindb {

void Init_Module_Exception(py::module &m) {

    py::register_exception<BindWarning>(m, "Warning", PyExc_Exception);

    py::register_exception<BindException>(m, "Error", PyExc_Exception);

    py::register_exception<BindInterfaceException>(m, "InterfaceError", m.attr("Error").ptr());

    py::register_exception<BindDatabaseException>(m, "DatabaseError", m.attr("Error").ptr());

    py::register_exception<BindDataException>(m, "DataError", m.attr("DatabaseError").ptr());

    py::register_exception<BindOperationalException>(m, "OperationalError", m.attr("DatabaseError").ptr());

    py::register_exception<BindIntegrityException>(m, "IntegrityError", m.attr("DatabaseError").ptr());

    py::register_exception<BindInternalException>(m, "InternalError", m.attr("DatabaseError").ptr());

    py::register_exception<BindProgrammingException>(m, "ProgrammingError", m.attr("DatabaseError").ptr());

    py::register_exception<BindNotSupportedException>(m, "NotSupportedError", m.attr("DatabaseError").ptr());

}



} // namespace pybind_dolphindb