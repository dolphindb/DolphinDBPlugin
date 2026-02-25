#include "CoderBase.h"

#include "DolphinDBEverything.h"
#include "Exceptions.h"
#include "Types.h"
#include <exception>
#include <iostream>

using namespace google::protobuf;

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

ConstantSP EncoderImpl::protobufSerialize(ConstantSP obj) const {
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
        const FieldDescriptor* descriptor = messageDesc->field(i);
        std::string descriptorName = std::string(descriptor->name());
        if(!t->contain(descriptorName)) {
            continue;
        }
        VectorSP col = t->getColumn(descriptorName);

        auto protoType = descriptor->type();
        switch (protoType) {
            case FieldDescriptor::TYPE_DOUBLE:
                for(int j = 0; j < rowNum; ++j) {
                    reflection->SetDouble(messages[j].get(), descriptor, col->getDouble(j));
                }
                break;
            case FieldDescriptor::TYPE_FLOAT:
                for(int j = 0; j < rowNum; ++j) {
                    reflection->SetFloat(messages[j].get(), descriptor, col->getFloat(j));
                }
                break;
            case FieldDescriptor::TYPE_INT64:
                for(int j = 0; j < rowNum; ++j) {
                    reflection->SetInt64(messages[j].get(), descriptor, col->getLong(j));
                }
                break;
            case FieldDescriptor::TYPE_UINT64:
                for(int j = 0; j < rowNum; ++j) {
                    long long value = col->getLong(j);
                    if(value < 0) {
                        throw RuntimeException(ENCODERDECODER_PREFIX + "the uint64 proto type cannot represend negative numbers: column " + descriptorName  + ", row " + std::to_string(j + 1));
                    }
                    reflection->SetUInt64(messages[j].get(), descriptor, value);
                }
                break;
            case FieldDescriptor::TYPE_INT32:
                for(int j = 0; j < rowNum; ++j) {
                    reflection->SetInt32(messages[j].get(), descriptor, col->getInt(j));
                }
                break;
            case FieldDescriptor::TYPE_BOOL:
                for(int j = 0; j < rowNum; ++j) {
                    bool value = false;
                    if(!col->isNull(j)) {
                        value = col->getBool(j);
                    }
                    reflection->SetBool(messages[j].get(), descriptor, value);
                }
                break;
            case FieldDescriptor::TYPE_STRING:
                for(int j = 0; j < rowNum; ++j) {
                    reflection->SetString(messages[j].get(), descriptor, col->getString(j));
                }
                break;
            case FieldDescriptor::TYPE_UINT32:
                for(int j = 0; j < rowNum; ++j) {
                    int value = col->getInt(j);
                    if(value < 0) {
                        throw RuntimeException(ENCODERDECODER_PREFIX + "the uint32 proto type cannot represend negative numbers: column " + descriptorName  + ", row " + std::to_string(j + 1));
                    }
                    reflection->SetUInt32(messages[j].get(), descriptor, value);
                }
                break;
            default:
                throw RuntimeException(ENCODERDECODER_PREFIX + "protubuf encoder not support this proto type " + std::string(descriptor->type_name()));
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

ConstantSP EncoderImpl::callMethod(const string& name, Heap* heap, vector<ConstantSP>& args) const {
    if(name == "serialize") {
        if(args.size() != 1){
             throw IllegalArgumentException("encoder::serialize(obj)", "The function [serialize] expects 1 argument(s), but the actual number of arguments is: " + std::to_string(args.size()));
        }
        return protobufSerialize(args[0]);
    }
    throw IllegalArgumentException("Encoder::" + name, "function [" + name + "] is unsupported on decoder");
}

void EncoderImpl::initialize(const std::string& filePath, const std::string& protoName) {
    protoType_ = getMessageFromProtoFile(filePath, protoName, pool, factory);
}
