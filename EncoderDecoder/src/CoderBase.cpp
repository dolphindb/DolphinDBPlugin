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

bool CoderImplClass::hasMethod(const string& name) const {
    if (name == "parseAndHandle" || name == "append" || name == "parse") {
        return true;
    }
    return false;
}

ConstantSP parseAndHandle(Heap* heap, vector<ConstantSP>& args) {
    if (UNLIKELY(args.size() != 2)) {
        throw IllegalArgumentException("Coder::parseAndHandle(item)",
            "The function [parseAndHandle] expects 2 argument(s), but the actual number of arguments is: "
            + std::to_string(args.size()));
    }
    if (args[0]->getString() != "coder instance") {
        throw  IllegalArgumentException("Coder::parseAndHandle(item)", "only a coder could parseAndHandle, but actually: " + args[0]->getString() + ".");
    }
    ((CoderImpl *)(args[0].get()))->appendTable(args[1]);
    return new Void();
}

FunctionDefSP CoderImplClass::getMethod(const string& name) const {
    if (name == "parse") {
        return funcWrapper_;
    } else if (name == "parseAndHandle" || name == "append") {
        FunctionDefSP partParser = Util::createSystemFunction(name, parseAndHandle, 2, 32, false);
        return partParser;
    }
    throw RuntimeException(ENCODERDECODER_PREFIX + " EncoderDecoder class doesn't have method " + name);
}