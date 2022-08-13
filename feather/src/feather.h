#ifndef FEATHER_PLUGIN_H
#define FEATHER_PLUGIN_H

#include <CoreConcept.h>
#include <Exceptions.h>
#include <FlatHashmap.h>
#include <ScalarImp.h>
#include <SysIO.h>
#include <Util.h>
#include <Types.h>

#include "arrow/type.h"
#include "arrow/type_fwd.h"


extern "C" ConstantSP loadFeather(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP saveFeather(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP schemaFeather(Heap *heap, vector<ConstantSP> &arguments);

DATA_TYPE convertArrowToDolphinDB(std::shared_ptr<arrow::DataType> type){
    switch(type->id()) {
    case arrow::Type::type::NA:
        return DT_VOID;
    /// Boolean as 1 bit, LSB bit-packed ordering
    case arrow::Type::type::BOOL:
        return DT_BOOL;
    /// Unsigned 8-bit little-endian integer
    case arrow::Type::type::UINT8:
        return DT_SHORT;
    /// Signed 8-bit little-endian integer
    case arrow::Type::type::INT8:
        return DT_CHAR;
    /// Unsigned 16-bit little-endian integer
    case arrow::Type::type::UINT16:
        return DT_INT;
    /// Signed 16-bit little-endian integer
    case arrow::Type::type::INT16:
        return DT_SHORT;
    /// Unsigned 32-bit little-endian integer
    case arrow::Type::type::UINT32:
        return DT_LONG;
    /// Signed 32-bit little-endian integer
    case arrow::Type::type::INT32:
        return DT_INT;
    /// Unsigned 64-bit little-endian integer
    case arrow::Type::type::UINT64:
        return DT_LONG;
    /// Signed 64-bit little-endian integer
    case arrow::Type::type::INT64:
        return DT_LONG;
    /// 2-byte floating point value
    case arrow::Type::type::HALF_FLOAT:
        break;
    /// 4-byte floating point value
    case arrow::Type::type::FLOAT:
        return DT_FLOAT;
    /// 8-byte floating point value
    case arrow::Type::type::DOUBLE:
        return DT_DOUBLE;
    /// UTF8 variable-length string as List<Char>
    case arrow::Type::type::STRING:
        return DT_STRING;
    /// Variable-length bytes (no guarantee of UTF8-ness)
    case arrow::Type::type::BINARY:
        break;
    /// Fixed-size binary. Each value occupies the same number of bytes
    case arrow::Type::type::FIXED_SIZE_BINARY:
        break;
    /// int32_t days since the UNIX epoch
    case arrow::Type::type::DATE32:
        return DT_DATE;
    /// int64_t milliseconds since the UNIX epoch
    case arrow::Type::type::DATE64:
        return DT_TIMESTAMP;
    /// Exact timestamp encoded with int64 since UNIX epoch
    /// Default unit millisecond
    case arrow::Type::type::TIMESTAMP:
    {
        auto timeType = std::static_pointer_cast<arrow::TimeType>(type);
        if(timeType->unit() == arrow::TimeUnit::type::MILLI) {
            return DT_TIMESTAMP;
        } else if (timeType->unit() == arrow::TimeUnit::type::NANO) {
            return DT_NANOTIMESTAMP;
        } else {
            //TODO
        }
        break;
    }
    /// Time as signed 32-bit integer, representing either seconds or
    /// milliseconds since midnight
    case arrow::Type::type::TIME32:
    {
        auto timeType = std::static_pointer_cast<arrow::TimeType>(type);
        if(timeType->unit() == arrow::TimeUnit::type::MILLI) {
            return DT_TIME;
        } else if(timeType->unit() == arrow::TimeUnit::type::SECOND) {
            return DT_SECOND;
        } else{
            //TODO
        }
        break;
    }

    /// Time as signed 64-bit integer, representing either microseconds or
    /// nanoseconds since midnight
    case arrow::Type::type::TIME64:
    {
        auto timeType = std::static_pointer_cast<arrow::TimeType>(type);
        if(timeType->unit() == arrow::TimeUnit::type::NANO) {
            return DT_NANOTIME;
        } else {
            //TODO
        }
        break;
    }
        
    /// YEAR_MONTH interval in SQL style
    case arrow::Type::type::INTERVAL_MONTHS:
        break;
    /// DAY_TIME interval in SQL style
    case arrow::Type::type::INTERVAL_DAY_TIME:
        break;
    /// Precision- and scale-based decimal type with 128 bits.
    case arrow::Type::type::DECIMAL128:
        break;
    /// Precision- and scale-based decimal type with 256 bits.
    case arrow::Type::type::DECIMAL256:
        break;
    /// A list of some logical data type
    case arrow::Type::type::LIST:
        break;
    /// Struct of logical types
    case arrow::Type::type::STRUCT:
        break;
    /// Sparse unions of logical types
    case arrow::Type::type::SPARSE_UNION:
        break;
    /// Dense unions of logical types
    case arrow::Type::type::DENSE_UNION:
        break;
    /// Dictionary-encoded type, also called "categorical" or "factor"
    /// in other programming languages. Holds the dictionary value
    /// type but not the dictionary itself, which is part of the
    /// ArrayData struct
    case arrow::Type::type::DICTIONARY:
        break;
    /// Map, a repeated struct logical type
    case arrow::Type::type::MAP:
        break;
    /// Custom data type, implemented by user
    case arrow::Type::type::EXTENSION:
        break;
    /// Fixed size list of some logical type
    case arrow::Type::type::FIXED_SIZE_LIST:
        break;
    /// Measure of elapsed time in either seconds, milliseconds, microseconds
    /// or nanoseconds.
    case arrow::Type::type::DURATION:
        break;
    /// Like STRING, but with 64-bit offsets
    case arrow::Type::type::LARGE_STRING:
        break;
    /// Like BINARY, but with 64-bit offsets
    case arrow::Type::type::LARGE_BINARY:
        break;
    /// Like LIST, but with 64-bit offsets
    case arrow::Type::type::LARGE_LIST:
        break;
    /// Calendar interval type with three fields.
    case arrow::Type::type::INTERVAL_MONTH_DAY_NANO:
        break;
    // Leave this at the end
    case arrow::Type::type::MAX_ID:
        break;
    default:
        //TODO
        return DT_VOID;
    };
    return DT_VOID;
}


std::shared_ptr<arrow::DataType> convertDolphinDBToArrow(DATA_TYPE type){
    switch(type) {
    case DT_VOID:
        return arrow::null();
    case DT_BOOL:
        return arrow::boolean();
    case DT_CHAR:
        return arrow::int8();
    case DT_SHORT:
        return arrow::int16();
    case DT_INT:
        return arrow::int32();
    case DT_LONG:
        return arrow::int64();
    case DT_DATE:
        return arrow::date32();
    case DT_MONTH:
        break;
    case DT_TIME:
        return arrow::time32(arrow::TimeUnit::MILLI);
    case DT_MINUTE:
        break;
    case DT_SECOND:
        return arrow::time32(arrow::TimeUnit::SECOND);
    case DT_DATETIME:
        break;
    case DT_TIMESTAMP:
        return arrow::timestamp(arrow::TimeUnit::MILLI);
    case DT_NANOTIME:
        return arrow::time64(arrow::TimeUnit::NANO);
    case DT_NANOTIMESTAMP:
        return arrow::timestamp(arrow::TimeUnit::NANO);
    case DT_FLOAT:
        return arrow::float32();
    case DT_DOUBLE:
        return arrow::float64();
    case DT_STRING:
        return arrow::utf8();
    case DT_SYMBOL:
        return arrow::utf8();
    case DT_UUID:
        break;
    case DT_INT128:
        break;
    case DT_BLOB:
        break;
    default:
        return arrow::null();
    };
    //TODO
    return arrow::null();
}

#endif