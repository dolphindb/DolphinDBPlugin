#include "CoderBase.h"

#include "DolphinDBEverything.h"
#include "Exceptions.h"
#include "Types.h"
#include <exception>
#include <iostream>

using namespace google::protobuf;

using namespace ddb;
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
    std::ignore = values;
    std::ignore = insertedRows;
    std::ignore = errMsg;
    return false;
}

bool CoderImplClass::hasMethod(const string& name) const {
    if (name == "parseAndHandle" || name == "append" || name == "parse") {
        return true;
    }
    return false;
}

bool EncoderClass::hasMethod(const string& name) const {
    return name == "serialize" ? true : false;
}

ConstantSP parseAndHandle(Heap* heap, vector<ConstantSP>& args) {
    std::ignore = heap;
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

ConstantSP protobufSerialize(Heap* heap, vector<ConstantSP>& args) {
    std::ignore = heap;
    if (args[0]->getString() != "encoder instance") {
        throw  IllegalArgumentException("encoder::serialize", "only a encoder could serialize, but actually: " + args[0]->getString() + ".");
    }
    return reinterpret_cast<EncoderInstance*>(args[0].get())->protobufSerialize(args[1]);
}

ConstantSP EncoderInstance::protobufSerialize(ConstantSP obj) {
    if(obj->getForm() != DF_TABLE) {
        throw RuntimeException(ENCODERDECODER_PREFIX + "protobuf encode only support serialize table");
    }
    TableSP t = obj;

    INDEX rowNum = t->rows();
    std::vector<std::shared_ptr<Message>> messages;
    for(INDEX i = 0; i < rowNum; ++i) {
        std::shared_ptr<Message> msg(protoType_->New());
        messages.push_back(msg);
    }

    const auto* reflection = protoType_->GetReflection();
    const auto *messageDesc = protoType_->GetDescriptor();

    int fieldNum = protoType_->GetDescriptor()->field_count();
    for(int i = 0; i < fieldNum; ++i) {
        const FieldDescriptor* discriptor = messageDesc->field(i);
        if(!t->contain(discriptor->name())) {
            continue;
        }
        VectorSP col = t->getColumn(discriptor->name());

        auto protoType = discriptor->type();
        switch (protoType) {
            case FieldDescriptor::TYPE_DOUBLE:
                for(int j = 0; j < rowNum; ++j) {
                    reflection->SetDouble(messages[j].get(), discriptor, col->getDouble(j));
                }
                break;
            case FieldDescriptor::TYPE_FLOAT:
                for(int j = 0; j < rowNum; ++j) {
                    reflection->SetFloat(messages[j].get(), discriptor, col->getFloat(j));
                }
                break;
            case FieldDescriptor::TYPE_INT64:
                for(int j = 0; j < rowNum; ++j) {
                    reflection->SetInt64(messages[j].get(), discriptor, col->getLong(j));
                }
                break;
            case FieldDescriptor::TYPE_UINT64:
                for(int j = 0; j < rowNum; ++j) {
                    long long value = col->getLong(j);
                    if(value < 0) {
                        throw RuntimeException(ENCODERDECODER_PREFIX + "the uint64 proto type cannot represend negative numbers: column " + discriptor->name()  + ", row " + std::to_string(j + 1));
                    }
                    reflection->SetUInt64(messages[j].get(), discriptor, value);
                }
                break;
            case FieldDescriptor::TYPE_INT32:
                for(int j = 0; j < rowNum; ++j) {
                    reflection->SetInt32(messages[j].get(), discriptor, col->getInt(j));
                }
                break;
            case FieldDescriptor::TYPE_BOOL:
                for(int j = 0; j < rowNum; ++j) {
                    bool value = false;
                    if(!col->isNull(j)) {
                        value = col->getBool(j);
                    }
                    reflection->SetBool(messages[j].get(), discriptor, value);
                }
                break;
            case FieldDescriptor::TYPE_STRING:
                for(int j = 0; j < rowNum; ++j) {
                    reflection->SetString(messages[j].get(), discriptor, col->getString(j));
                }
                break;
            case FieldDescriptor::TYPE_UINT32:
                for(int j = 0; j < rowNum; ++j) {
                    int value = col->getInt(j);
                    if(value < 0) {
                        throw RuntimeException(ENCODERDECODER_PREFIX + "the uint32 proto type cannot represend negative numbers: column " + discriptor->name()  + ", row " + std::to_string(j + 1));
                    }
                    reflection->SetUInt32(messages[j].get(), discriptor, value);
                }
                break;
            default:
                throw RuntimeException(ENCODERDECODER_PREFIX + "protubuf encoder not support this proto type " + discriptor->type_name());
        }
    }

    std::vector<std::string> results(rowNum);
    for(int i = 0; i < rowNum; ++i) {
        if (!messages[i]->SerializeToString(&results[i])) {
            throw RuntimeException(ENCODERDECODER_PREFIX + "serialize row " + std::to_string(i) + " fail");
        }
    }
    VectorSP ret = Util::createVector(DT_BLOB, 0, rowNum);
    ret->appendString(results.data(), rowNum);
    return ret;
}

FunctionDefSP EncoderClass::getMethod(const string& name) const {
    if (name == "serialize") {
        FunctionDefSP func = Util::createSystemFunction(name, protobufSerialize, 2, 2, false);
        return func;
    }
    throw RuntimeException(ENCODERDECODER_PREFIX + " EncoderClass class doesn't have method " + name);
}

void EncoderInstance::initialize(const std::string& filePath, const std::string& protoName) {
    protoType_ = getMessageFromProtoFile(filePath, protoName, pool, factory);
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
