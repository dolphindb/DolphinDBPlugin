#include "EncoderDecoder.h"

#include <Concurrent.h>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <vector>

#include "ddbplugin/CommonInterface.h"
#include "CoderBase.h"
#include "Types.h"
#include "ddbplugin/pluginVersion.h"
#include "jsonUtil.h"
#include "protobufUtil.h"

static void doNothing(Heap *heap, vector<ConstantSP> &args) {}

void truncWorkerNum(long long &workerNum) {
    unsigned int sysThreadNum = std::thread::hardware_concurrency();
    workerNum = (sysThreadNum != 0 && sysThreadNum < workerNum) ? sysThreadNum : workerNum;
}

ConstantSP getProtobufSchema(Heap *heap, vector<ConstantSP> &arguments) {
    const string usage = "Usage: extractProtobufSchema(schemaPath, [toArrayVector], [messageName]). ";
    if (arguments[0]->getType() != DT_STRING || arguments[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException("extractProtobufSchema", usage + "schemaPath must be a string scalar. ");
    }
    string schemaPath = arguments[0]->getString();

    if (!Util::exists(schemaPath)) {
        throw IllegalArgumentException("extractProtobufSchema", usage + "file [" + schemaPath + "] does not exist. ");
    }

    bool needArrayVector = false;
    if (arguments.size() > 1 && !arguments[1]->isNull()) {
        if (arguments[1]->getForm() != DF_SCALAR || arguments[1]->getType() != DT_BOOL) {
            throw IllegalArgumentException("extractProtobufSchema", usage + "toArrayVector must be a bool scalar. ");
        }
        needArrayVector = static_cast<bool>(arguments[1]->getBool());
    }
    string messageName = "";
    if (arguments.size() > 2 && !arguments[2]->isNull()) {
        if (arguments[2]->getForm() != DF_SCALAR || arguments[2]->getType() != DT_STRING) {
            throw IllegalArgumentException("extractProtobufSchema", usage + "messageName must be a string scalar. ");
        }
        messageName = arguments[2]->getString();
    }

    unordered_set<string> ignoredColumn;

    // if (arguments.size() > 2) {
    //     if (arguments[2]->getForm() != DF_VECTOR || arguments[2]->getType() != DT_STRING) {
    //         throw IllegalArgumentException("extractProtobufSchema", usage + "ignoredColumn must be a string vector.
    //         ");
    //     }
    //     VectorSP column = arguments[];
    //     for(INDEX i = 0; i < column->size(); ++i) {
    //         ignoredColumn.insert(column->getString(i));
    //     }
    // }

    return protobufSchema(schemaPath, needArrayVector, ignoredColumn, messageName);
}

namespace OperatorImp {
    ConstantSP getStreamingStat(Heap* heap, vector<ConstantSP>& arguments);
}

ConstantSP createProtobufDecoder(Heap *heap, vector<ConstantSP> &arguments) {
    const auto usage = string(
        "Usage: protobufDecoder(schemaPath, [handler], [workerNum=1], [batchSize=0], [throttle=1.0], [toArrayVector=false], "
        "[schema], [messageName], [useNullAsDefault=false]). ");
    ConstantSP schemaPath, handler, dummyTable;
    long long workerNum = 1;
    long long batchSize = 0;
    long long throttle = 1000;
    ConstantSP schema = nullptr;

    if (arguments[0]->getType() != DT_STRING || arguments[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException("protobufDecoder", usage + "schemaPath must be a string scalar. ");
    }
    schemaPath = arguments[0];
    if (!Util::exists(schemaPath->getString())) {
        throw IllegalArgumentException("protobufDecoder",
                                       usage + "file [" + schemaPath->getString() + "] does not exist. ");
    }

    if (arguments.size() >= 2 && !arguments[1]->isNull()) {
        if (arguments[1]->getForm() != DF_TABLE && arguments[1]->getType() != DT_FUNCTIONDEF &&
            arguments[1]->getType() != DT_RESOURCE) {
            throw IllegalArgumentException("protobufDecoder",
                                           usage + "handler must be a table or a function or another decoder. ");
        }
        handler = arguments[1];
        vector<string> dummyColName = {"pbString"};
        vector<DATA_TYPE> dummyColType = {DT_STRING};
        dummyTable = Util::createTable(dummyColName, dummyColType, 0, 1);
    }
    if (arguments.size() >= 3 && !arguments[2]->isNull()) {
        if (arguments[2]->getCategory() != INTEGRAL || arguments[2]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException("protobufDecoder", usage + "workerNum must be a integer scalar. ");
        }
        workerNum = arguments[2]->getLong();
        if (workerNum < 1) {
            throw IllegalArgumentException("protobufDecoder", usage + "workerNum must be a positive integer. ");
        }
        truncWorkerNum(workerNum);
    }
    if (arguments.size() >= 4 && !arguments[3]->isNull()) {
        if (arguments[3]->getCategory() != INTEGRAL || arguments[3]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException("protobufDecoder", usage + "batchSize must be a integer scalar. ");
        }
        batchSize = arguments[3]->getInt();
    }
    if (arguments.size() >= 5 && !arguments[4]->isNull()) {
        DATA_CATEGORY category = arguments[4]->getCategory();
        if((category == INTEGRAL || category == FLOATING) && arguments[4]->getForm() == DF_SCALAR) {
            throttle = arguments[4]->getDouble() * 1000;
        } else {
            throw IllegalArgumentException("protobufDecoder", usage + "throttle must be a float scalar. ");
        }
    }

    string name = "ProtobufDecoder";
    SysFunc func = &parseProtobuf;

    ConstantSP needArrayTableArgs = new Bool(false);
    ConstantSP tableArgs = new String("");
    ConstantSP protoNameArgs = new String("");
    ConstantSP useZeroArgs = new Bool(true);

    if (arguments.size() >= 6 && !arguments[5]->isNull()) {
        if (arguments[5]->getType() != DT_BOOL || arguments[5]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException("protobufDecoder", usage + "needArrayVector must be a boolean scalar. ");
        }
        needArrayTableArgs = arguments[5];
    }

    if (arguments.size() >= 7 && !arguments[6]->isNull()) {
        if (arguments[6]->getForm() == DF_TABLE) {
            schema = arguments[6];
            string errMsg =
                "schema must be a table with at least two columns. First column is name, second column is typeString.";
            if (schema->columns() < 2) {
                throw IllegalArgumentException("protobufDecoder", usage + errMsg);
            }
            if (((TableSP)schema)->getColumnName(0) != "name" || ((TableSP)schema)->getColumnName(1) != "typeString") {
                throw IllegalArgumentException("protobufDecoder", usage + errMsg);
            }
            if (((TableSP)schema)->getColumnType(0) != DT_STRING || ((TableSP)schema)->getColumnType(1) != DT_STRING) {
                throw IllegalArgumentException("protobufDecoder", usage + "first two schema columns' type must be string.");
            }
            // TODO? maybe no need. add restriction, if handler is not table, schema must be passed.
            if (!handler.isNull() && handler->getForm() == DF_TABLE) {
                if (handler->columns() != schema->getColumn(0)->size()) {
                    throw IllegalArgumentException("protobufDecoder",
                                                   usage + "handler table  is not compatible with schema. ");
                }
                VectorSP name = schema->getColumn(0);
                VectorSP type = schema->getColumn(1);
                for (INDEX i = 0; i < name->size(); ++i) {
                    if (((TableSP)handler)->getColumnName(i) != ((VectorSP)name)->getString(i)) {
                        throw IllegalArgumentException("protobufDecoder",
                                                       usage + "incompatible name between handler and schema");
                    }
                    if (Util::getDataTypeString(((TableSP)handler)->getColumnType(i)) !=
                        ((VectorSP)type)->getString(i)) {
                        throw IllegalArgumentException("protobufDecoder",
                                                       usage + "incompatible type between handler and schema");
                    }
                }
            }
            tableArgs = arguments[6];
        } else {
            throw IllegalArgumentException("protobufDecoder", usage + "schema must be a table with at least two columns. ");
        }
    }
    if (arguments.size() >= 8 && !arguments[7]->isNull()) {
        if (arguments[7]->getType() != DT_STRING || arguments[7]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException("protobufDecoder", usage + "messageName must be a string scalar. ");
        }
        protoNameArgs = arguments[7];
    }
    if (arguments.size() >= 9 && !arguments[8]->isNull()) {
        if (arguments[8]->getType() != DT_BOOL || arguments[8]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException("protobufDecoder", usage + "useNullAsDefault must be a boolean scalar. ");
        }
        bool input = arguments[8]->getBool();
        useZeroArgs->setBool(!input);
    }

    vector<ConstantSP> args{schemaPath, needArrayTableArgs, tableArgs, protoNameArgs, useZeroArgs};
    FunctionDefSP partParser = Util::createSystemFunction(name, func, 6, 6, false);
    FunctionDefSP parser = Util::createPartialFunction(partParser, args);
    FunctionDefSP onClose(Util::createSystemProcedure("createProtobufDecoder onClose()", doNothing, 1, 1));

    ConstantSP decoder;
    if (arguments.size() == 1 || arguments[1]->isNull()) {
        decoder = new CoderImpl(heap->currentSession(), parser);
    } else {
        decoder = new CoderImpl(heap->currentSession(), parser, handler, dummyTable, workerNum, batchSize, throttle);
    }
    decoder->setOOInstance(true);
    return decoder;
}

ConstantSP createJsonDecoder(Heap* heap, vector<ConstantSP>& arguments)
{
    const auto usage = string("Usage: jsonDecoder(colNames, colTypes, [handler], [workerNum=1], [batchSize=0], [throttle=1.0], [isMultiJson=false]). ");
    // const auto usage = string("Usage: jsonDecoder(colNames, colTypes, [handler], [workerNum=1], [batchSize=0], [throttle=1.0], [isMultiJson=false], [isNested=false]). ");

    ConstantSP colNames, colTypes, handler, dummyTable;
    long long workerNum = 1;
    long long batchSize = 0;
    long long throttle = 1000;

    if (arguments[0]->getForm() != DF_VECTOR || arguments[0]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "colNames must be a string vector. ");
    }
    if (arguments[1]->getForm() != DF_VECTOR || arguments[1]->getType() != DT_INT) {
        throw IllegalArgumentException(__FUNCTION__, usage + "colTypes must be a int vector. ");
    }
    colNames = arguments[0];
    colTypes = arguments[1];

    vector<string> dummyColName = { "jsonString" };
    vector<DATA_TYPE> dummyColType = { DT_STRING };
    dummyTable = Util::createTable(dummyColName, dummyColType, 0, 1);

    if (arguments.size() >= 3 && !arguments[2]->isNull()) {
        if (arguments[2]->getForm() != DF_TABLE && arguments[2]->getType() != DT_FUNCTIONDEF && arguments[2]->getType() != DT_RESOURCE) {
            throw IllegalArgumentException(__FUNCTION__, usage + "handler must be a table or a function or another decoder. ");
        }
        handler = arguments[2];
    }
    if (arguments.size() >= 4 && !arguments[3]->isNull()) {
        if (arguments[3]->getCategory() == INTEGRAL && arguments[3]->getForm() == DF_SCALAR) {
            workerNum = arguments[3]->getLong();
        } else {
            throw IllegalArgumentException(__FUNCTION__, usage + "workerNum must be a integer scalar. ");
        }
        if (workerNum < 1) {
            throw IllegalArgumentException(__FUNCTION__, usage + "workerNum must be a positive integer. ");
        }
        truncWorkerNum(workerNum);
    }
    if (arguments.size() >= 5 && !arguments[4]->isNull()) {
        if (arguments[4]->getCategory() == INTEGRAL && arguments[4]->getForm() == DF_SCALAR) {
            batchSize = arguments[4]->getLong();
        } else {
            throw IllegalArgumentException(__FUNCTION__, usage + "batchSize must be a integer. ");
        }
    }
    if (arguments.size() >= 6 && !arguments[5]->isNull()) {
        DATA_CATEGORY category = arguments[5]->getCategory();
        if ((category == FLOATING || category == INTEGRAL) && arguments[5]->getForm() == DF_SCALAR) {
            throttle = arguments[5]->getDouble() * 1000;
        } else {
            throw IllegalArgumentException(__FUNCTION__, usage + "throttle must be a float number. ");
        }
    }

    bool isMultiJsons = false;
    if (arguments.size() >= 7 && !arguments[6]->isNull()) {
        if(arguments[6]->getType() == DT_BOOL && arguments[6]->getForm() == DF_SCALAR) {
            isMultiJsons = arguments[6]->getBool();
        } else {
            throw IllegalArgumentException(__FUNCTION__, usage + " isMulti must be a boolean scalar. ");
        }
    }
    ConstantSP isMulti = new Bool(isMultiJsons);

    bool isNested = false;
    // if (arguments.size() >= 8 && !arguments[7]->isNull()) {
    //     if (arguments[7]->getType() == DT_BOOL && arguments[7]->getForm() == DF_SCALAR) {
    //         isNested = arguments[7]->getBool();
    //     } else {
    //         throw IllegalArgumentException(__FUNCTION__, usage + " isNested must be a boolean scalar. ");
    //     }
    // }

    string name = "jsonDecoder";
    SysFunc func;
    if (isNested) {
        func = &parseNestedJson;
    } else {
        func = &parseJson;
    }
    FunctionDefSP partParser = Util::createSystemFunction(name, func, 4, 4, false);
    FunctionDefSP parser = Util::createPartialFunction(partParser, { colNames, colTypes, isMulti });
    FunctionDefSP onClose(Util::createSystemProcedure("createJsonDecoder onClose()", doNothing, 1, 1));

    ConstantSP decoderRes;
    if (arguments.size() == 2 || arguments[2]->isNull()) {
        decoderRes = new CoderImpl(heap->currentSession(), parser);
    } else {
        decoderRes = new CoderImpl(heap->currentSession(), parser, handler, dummyTable, workerNum, batchSize, throttle);
    }

    decoderRes->setOOInstance(true);
    return decoderRes;
}
