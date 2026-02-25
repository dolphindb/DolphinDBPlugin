#ifndef CONVERTER_TYPEDEFINE_H_
#define CONVERTER_TYPEDEFINE_H_

#ifndef PYTHON_SWORDFISH
#include "DolphinDBEverything.h"
#endif
#include "CoreConcept.h"
#include "ScalarImp.h"
#include "Util.h"
#ifdef PYTHON_SWORDFISH
#include "Swordfish.h"
#endif

#include <pybind11/pybind11.h>

namespace pybind_dolphindb {

enum STORAGE_ENGINE_TYPE {
    OLAP = 0,
    OLTP = 1,
    IOT = 2,
    IMOLTP = 3,
    PKEY = 4,
    IOTDB = 5,
};

}   // pybind_dolphindb


namespace converter {

using namespace pybind_dolphindb;

enum HELPER_TYPE {
    HT_VOID, HT_BOOL, HT_CHAR, HT_SHORT, HT_INT,
    HT_LONG, HT_DATE, HT_MONTH, HT_TIME, HT_MINUTE,
    HT_SECOND, HT_DATETIME, HT_TIMESTAMP, HT_NANOTIME, HT_NANOTIMESTAMP,
    HT_FLOAT, HT_DOUBLE, HT_SYMBOL, HT_STRING, HT_UUID,
    HT_FUNCTIONDEF, HT_HANDLE, HT_CODE, HT_DATASOURCE, HT_RESOURCE,
    HT_ANY, HT_COMPRESS, HT_DICTIONARY, HT_DATEHOUR, HT_DATEMINUTE,
    HT_IP, HT_INT128, HT_BLOB, HT_DECIMAL, HT_COMPLEX,
    HT_POINT, HT_DURATION, HT_DECIMAL32, HT_DECIMAL64, HT_DECIMAL128,
    HT_OBJECT, HT_EXTENSION, HT_UNK, HT_COUNT
};

enum HELPER_CATEGORY {
    HC_NOTHING,
    HC_LOGICAL,
    HC_INTEGRAL,
    HC_FLOATING,
    HC_TEMPORAL,
    HC_LITERAL,
    HC_SYSTEM,
    HC_MIXED,
    HC_BINARY,
    HC_COMPLEX,
    HC_ARRAY,
    HC_DENARY,
    HC_PYTYPE,
};

typedef std::pair<HELPER_TYPE, int> Type;

#define EXPARAM_DEFAULT      INT_MIN
#define EXPARM_VALUE(v)     (v == EXPARAM_DEFAULT ? 0 : v)

#define MAX_NULL_PRI        999

enum FUNCTION_TYPE { INNER_FUNC, OPTR_FUNC, OPTR2_FUNC, SYS_FUNC, UDF_FUNC, PARTIAL_FUNC };

}   // converter



#endif  // CONVERTER_TYPEDEFINE_H_
