#ifndef CONVERTER_TYPEDEFINE_H_
#define CONVERTER_TYPEDEFINE_H_

#ifndef PYTHON_SWORDFISH
#include "DolphinDBEverything.h"
#endif
#include "OperatorImp.h"
#include "CoreConcept.h"
#include "ScalarImp.h"
#include "Util.h"
#ifdef PYTHON_SWORDFISH
#include "Swordfish.h"
#endif

#include <pybind11/pybind11.h>


namespace pybind_dolphindb {

using int128 = ddb::int128;

using ddb::INDEX;
using ddb::ARRAY_TYPE_BASE;

using ddb::FLT_NMIN;
using ddb::DBL_NMIN;

using ddb::Constant;
using ddb::ConstantSP;

using ddb::Util;

using ddb::DATA_TYPE;
using ddb::DATA_TYPE::DT_VOID;
using ddb::DATA_TYPE::DT_BOOL;
using ddb::DATA_TYPE::DT_CHAR;
using ddb::DATA_TYPE::DT_SHORT;
using ddb::DATA_TYPE::DT_INT;
using ddb::DATA_TYPE::DT_LONG;
using ddb::DATA_TYPE::DT_DATE;
using ddb::DATA_TYPE::DT_MONTH;
using ddb::DATA_TYPE::DT_TIME;
using ddb::DATA_TYPE::DT_MINUTE;
using ddb::DATA_TYPE::DT_SECOND;
using ddb::DATA_TYPE::DT_DATETIME;
using ddb::DATA_TYPE::DT_TIMESTAMP;
using ddb::DATA_TYPE::DT_NANOTIME;
using ddb::DATA_TYPE::DT_NANOTIMESTAMP;
using ddb::DATA_TYPE::DT_FLOAT;
using ddb::DATA_TYPE::DT_DOUBLE;
using ddb::DATA_TYPE::DT_SYMBOL;
using ddb::DATA_TYPE::DT_STRING;
using ddb::DATA_TYPE::DT_UUID;
using ddb::DATA_TYPE::DT_FUNCTIONDEF;
using ddb::DATA_TYPE::DT_HANDLE;
using ddb::DATA_TYPE::DT_CODE;
using ddb::DATA_TYPE::DT_DATASOURCE;
using ddb::DATA_TYPE::DT_RESOURCE;
using ddb::DATA_TYPE::DT_ANY;
using ddb::DATA_TYPE::DT_COMPRESS;
using ddb::DATA_TYPE::DT_DICTIONARY;
using ddb::DATA_TYPE::DT_DATEHOUR;
using ddb::DATA_TYPE::DT_DATEMINUTE;
using ddb::DATA_TYPE::DT_IP;
using ddb::DATA_TYPE::DT_INT128;
using ddb::DATA_TYPE::DT_BLOB;
using ddb::DATA_TYPE::DT_DECIMAL;
using ddb::DATA_TYPE::DT_COMPLEX;
using ddb::DATA_TYPE::DT_POINT;
using ddb::DATA_TYPE::DT_DURATION;
using ddb::DATA_TYPE::DT_DECIMAL32;
using ddb::DATA_TYPE::DT_DECIMAL64;
using ddb::DATA_TYPE::DT_DECIMAL128;
using ddb::DATA_TYPE::DT_OBJECT;
using ddb::DATA_TYPE::DT_IOTANY;

using ddb::DATA_FORM;
using ddb::DATA_FORM::DF_SCALAR;
using ddb::DATA_FORM::DF_VECTOR;
using ddb::DATA_FORM::DF_PAIR;
using ddb::DATA_FORM::DF_MATRIX;
using ddb::DATA_FORM::DF_SET;
using ddb::DATA_FORM::DF_DICTIONARY;
using ddb::DATA_FORM::DF_TABLE;
using ddb::DATA_FORM::DF_CHART;
using ddb::DATA_FORM::DF_CHUNK;
using ddb::DATA_FORM::DF_SYSOBJ;
using ddb::DATA_FORM::DF_TENSOR;
using ddb::DATA_FORM::MAX_DATA_FORMS;

using ddb::DATA_CATEGORY;
using ddb::DATA_CATEGORY::NOTHING;
using ddb::DATA_CATEGORY::LOGICAL;
using ddb::DATA_CATEGORY::INTEGRAL;
using ddb::DATA_CATEGORY::FLOATING;
using ddb::DATA_CATEGORY::TEMPORAL;
using ddb::DATA_CATEGORY::LITERAL;
using ddb::DATA_CATEGORY::SYSTEM;
using ddb::DATA_CATEGORY::MIXED;
using ddb::DATA_CATEGORY::BINARY;
using ddb::DATA_CATEGORY::COMPLEX;
using ddb::DATA_CATEGORY::ARRAY;
using ddb::DATA_CATEGORY::DENARY;

using ddb::DBENGINE_TYPE;

using ddb::OBJECT_TYPE;

using ddb::PARTITION_TYPE;
using ddb::PARTITION_TYPE::SEQ;
using ddb::PARTITION_TYPE::VALUE;
using ddb::PARTITION_TYPE::RANGE;
using ddb::PARTITION_TYPE::LIST;
using ddb::PARTITION_TYPE::HIER;
using ddb::PARTITION_TYPE::HASH;

using ddb::TABLE_TYPE;
using ddb::TABLE_TYPE::BASICTBL;
using ddb::TABLE_TYPE::REALTIMETBL;
using ddb::TABLE_TYPE::SNAPTBL;
using ddb::TABLE_TYPE::FILETBL;
using ddb::TABLE_TYPE::CHUNKTBL;
using ddb::TABLE_TYPE::JOINTBL;
using ddb::TABLE_TYPE::SEGTBL;
using ddb::TABLE_TYPE::ALIASTBL;
using ddb::TABLE_TYPE::COMPRESSTBL;
using ddb::TABLE_TYPE::LOGROWTBL;
using ddb::TABLE_TYPE::MVCCTBL;
using ddb::TABLE_TYPE::WIDETBL;
using ddb::TABLE_TYPE::DIMTBL;
using ddb::TABLE_TYPE::SNAPDIMTBL;
using ddb::TABLE_TYPE::CUSTOMIZEDTBL;
using ddb::TABLE_TYPE::CACHEDTBL;
using ddb::TABLE_TYPE::RESPOOLTBL;
using ddb::TABLE_TYPE::SUBTBL;
using ddb::TABLE_TYPE::IOTABLET;
using ddb::TABLE_TYPE::STREAMENGINE;
using ddb::TABLE_TYPE::IPCTBL;
using ddb::TABLE_TYPE::SEG_PERSISTENT_TBL;
using ddb::TABLE_TYPE::SQLTBL;
using ddb::TABLE_TYPE::MULTIJOINTBL;
using ddb::TABLE_TYPE::UNIVERSAL_TBL_JOIN;
using ddb::TABLE_TYPE::IMOLTPTABLET;
using ddb::TABLE_TYPE::TABLEJOINERTBL;
using ddb::TABLE_TYPE::PKTABLET;
using ddb::TABLE_TYPE::MAX_TABLE_TYPES;

using ddb::Void;
using ddb::Bool;
using ddb::Char;
using ddb::Short;
using ddb::Int;
using ddb::Long;
using ddb::Float;
using ddb::Double;
using ddb::String;
using ddb::Date;
using ddb::Month;
using ddb::NanoTime;
using ddb::Time;
using ddb::Second;
using ddb::Minute;
using ddb::NanoTimestamp;
using ddb::Timestamp;
using ddb::DateTime;
using ddb::DateHour;
using ddb::Decimal32;
using ddb::Decimal64;
using ddb::Decimal128;
using ddb::Int128;
using ddb::Uuid;
using ddb::IPAddr;
using ddb::EnumInt;
using ddb::FunctionDef;
using ddb::FunctionDefSP;
using ddb::Duration;
using ddb::Point;
using ddb::MetaCode;
using ddb::Decimal;

using ddb::Vector;
using ddb::VectorSP;

using ddb::Set;
using ddb::SetSP;

using ddb::Dictionary;
using ddb::DictionarySP;

using ddb::Table;
using ddb::TableSP;

using ddb::SmartPointer;
using ddb::ObjectPtr;

using ddb::Object;
using ddb::ObjectSP;

// using ::FastSymbolVector;
using ddb::SymbolBaseSP;

using ddb::Session;
using ddb::SessionSP;

using ddb::Param;
using ddb::ParamSP;

#ifdef PYTHON_SWORDFISH
using ddb::DolphinDBLib;
using ddb::CaseWhen;
#endif

using ddb::Heap;
using ddb::LockGuard;
using ddb::Mutex;
using ddb::Expression;
using ddb::ColumnDesc;

using ddb::Guid;
using ddb::DolphinString;

using ddb::OptrFunc;
using ddb::SysFunc;
using ddb::SysProc;

using ddb::log_inst;
using ddb::severity_type;

namespace OperatorImp = ddb::OperatorImp;

typedef SmartPointer<ddb::MetaCode> MetaCodeSP;

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
