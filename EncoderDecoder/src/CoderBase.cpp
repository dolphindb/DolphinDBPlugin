#include "CoderBase.h"
#include "Exceptions.h"
#include "Types.h"
#include <exception>
#include <iostream>


CoderImpl::~CoderImpl(){}

void CoderImpl::appendTable(ConstantSP items) const
{
    if (handler_->isNull()) {
        throw RuntimeException("To use method parseAndHandle, the handler of the coder object must be specified.");
    }
    string errMsg = ENCODERDECODER_PREFIX + " code.parseAndHandle() input items should be one of: " +
        string("table with only one STRING column table, STRING vector, STRING scalar.");

    switch(items->getForm()) {
        case DF_TABLE:
            if(UNLIKELY(items->columns() > 1 || items->columns() == 0)) {
                throw RuntimeException(errMsg);
            }
            batchProc_->add(items);
            break;
        case DF_VECTOR:
        {
            DATA_TYPE type = items->getType();
            if(UNLIKELY(type != DT_STRING)) {
                throw RuntimeException(errMsg);
            }
            batchProc_->add(items);
            break;
        }
        case DF_SCALAR:
        {
            DATA_TYPE type = items->getType();
            if(UNLIKELY(type != DT_STRING)) {
                throw RuntimeException(errMsg);
            }
            VectorSP input = Util::createVector(DT_STRING, 0);
            input->append(items);
            batchProc_->add(input);
            break;
        }
        default:
            throw RuntimeException(errMsg);
    }
}

bool CoderImpl::append(vector<ConstantSP>& values, INDEX& insertedRows, string& errMsg)
{
    return false;
}

ConstantSP CoderImpl::callMethod(const string& name, Heap* heap, vector<ConstantSP>& args) const
{
    if (name == "parseAndHandle" || name == "append") {
        if (UNLIKELY(args.size() > 1 || args.size() == 0)) {
            throw IllegalArgumentException("Coder::parseAndHandle(item)",
                "The function [parseAndHandle] expects 1 argument(s), but the actual number of arguments is: "
                + std::to_string(args.size()));
        }
        appendTable(args[0]);
        return new Void();
    } else if (name == "parse") {
        if (UNLIKELY(args.size() > 1 || args.size() == 0)) {
            throw IllegalArgumentException("Coder::parse(item)",
                "The function [parse] expects 1 argument(s), but the actual number of arguments is: "
                + std::to_string(args.size()));
        }
        return func_->call(session_->getHeap().get(), args);
    } else {
        throw IllegalArgumentException("Coder::" + name, "function [" + name + "] is unsupported on decoder");
    }
    return new Void();
}