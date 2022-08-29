#include "amdQuote.h"
#include "amdQuoteType.h"
#include "ScalarImp.h"

std::mutex AmdQuote::amdMutex_;
AmdQuote* AmdQuote::instance_;

bool receivedTimeFlag = false;
bool isConnected = false;

ConstantSP amdConnect(Heap *heap, vector<ConstantSP> &arguments) {
    // if (arguments[0]->getForm() != DF_PAIR) {
    //     throw RuntimeException("first argument illegal, should be stream table size");
    // }
    // ConstantSP tableSize  = arguments[0];

    if (arguments[0]->getForm() != DF_SCALAR || arguments[0]->getType() != DT_STRING) {
        throw RuntimeException("first argument illegal, should be amd username");
    }
    std::string username = arguments[0]->getString();

    if (arguments[1]->getForm() != DF_SCALAR || arguments[1]->getType() != DT_STRING) {
        throw RuntimeException("second argument illegal, should be amd password");
    }
    std::string password = arguments[1]->getString();

    if (arguments[2]->getForm() != DF_VECTOR || arguments[2]->getType() != DT_STRING) {
        throw RuntimeException("third argument illegal, should be a vector of amd ips list.");
    }

    if (arguments[3]->getForm() != DF_VECTOR || arguments[3]->getType() != DT_INT) {
        throw RuntimeException("fourth argument illegal, should be a vector of amd ports list.");
    }

    if (arguments[2]->size() != arguments[3]->size()) {
        throw RuntimeException("third or fourth argument illegal, ips nums not equal to ports nums");
    }
    std::vector<std::string> ips;
    std::vector<int> ports;

    for (int i = 0; i < arguments[2]->size(); i++) {
        ips.push_back(arguments[2]->getString(i));
    }
    
    for (int i = 0; i < arguments[3]->size(); i++) {
        ports.push_back(arguments[3]->getInt(i));
    }

    if (arguments.size() > 4) {
        if (arguments[4]->getForm() != DF_DICTIONARY) {
            throw IllegalArgumentException(__FUNCTION__, "options must be a dictionary"); 
        }

        DictionarySP options = arguments[4];
        VectorSP keys = options->keys();
        VectorSP values = options->values();
        for(int i = 0; i < options->size(); ++i) {
            ConstantSP key = keys->get(i);
            if(key->getType() != DT_STRING)
                throw IllegalArgumentException(__FUNCTION__, "key of options must be string");
            std::string str = key->getString();
            if(str != "ReceivedTime")
                throw IllegalArgumentException(__FUNCTION__, "key of options must be 'ReceivedTime'");
        }

        ConstantSP value = options->getMember("ReceivedTime");
        if (value->getType() != DT_BOOL) {
            throw IllegalArgumentException(__FUNCTION__, "value of 'ReceivedTime' must be boolean");
        }
        receivedTimeFlag = value->getBool();
    }

    SessionSP session = heap->currentSession()->copy(true);
    auto amdQuoteHandler = AmdQuote::getInstance(username, password, ips, ports, session);
    std::string handlerName("amdQuote");
    FunctionDefSP onClose(Util::createSystemProcedure("amdQuote onClose()", closeAmd, 1, 1));
    auto resource = Util::createResource((long long)amdQuoteHandler, handlerName, onClose, heap->currentSession());

    isConnected = true;
    return resource;
}

void closeAmd(Heap *heap, vector<ConstantSP> &arguments) {
    // need release by hand
    /*
    AmdQuote* amdQuotePtr = (AmdQuote*)arguments[0]->getLong();
    if (amdQuotePtr != nullptr) {
        delete amdQuotePtr;
    }*/
}

ConstantSP subscribe(Heap *heap, vector<ConstantSP> &arguments) { // amdHandler type(snapshot execution order) table marketType codeList

    if (arguments[0]->getForm() != DF_SCALAR) {
        throw RuntimeException("first argument illegal, should the obj return by connectAmd");
    }
    AmdQuote* amdQuotePtr = (AmdQuote*)arguments[0]->getLong();
    AmdQuote* instance = AmdQuote::getInstance();
    if (amdQuotePtr == nullptr || instance != amdQuotePtr) {
        throw RuntimeException("first argument illegal, should the obj return by connectAmd");
    }

    if (arguments[1]->getForm() != DF_SCALAR && arguments[1]->getType() != DT_STRING) {
        throw RuntimeException("second argument illegal, should be string");
    }
    std::string type = arguments[1]->getString();

    TableSP table = arguments[2];
    if (table->getTableType() != REALTIMETBL || !table->isSharedTable()) {
        std::string errMsg = "The third parameter `streamTable` must be a shared stream table.";
        throw RuntimeException(errMsg);
    }
    
    if (checkSchema(type, table) == false) {
        throw RuntimeException("schema mismatch");
    }

    int marketType = 0;
    if (arguments.size() > 3) {
        if (arguments[3]->getForm() != DF_SCALAR || arguments[3]->getType() != DT_INT) {
            throw RuntimeException("fourth argument illegal, should be amd marketType");
        }
        marketType = arguments[3]->getInt();
    }

    std::vector<std::string> codeList;
    if (arguments.size() > 4) {
        if (arguments[4]->getForm() != DF_VECTOR || arguments[4]->getType() != DT_STRING) {
            throw RuntimeException("fifth argument illegal, should be codeList vector");
        }
        for (int i = 0; i < arguments[4]->size(); i++) {
            codeList.push_back(arguments[4]->getString(i));
        }
    }
    if (type == "snapshot") {
        amdQuotePtr->subscribeSnapshot(marketType, table, codeList);
    } else if (type == "execution") {
        amdQuotePtr->subscribeExecution(marketType, table, codeList);
    } else if (type == "order") {
        amdQuotePtr->subscribeOrder(marketType, table, codeList);
    } else {
        throw RuntimeException("second argument illegal, should be `snapshot`, `execution` or `order`");
    }

    ConstantSP ret = Util::createConstant(DT_STRING);
    ret->setString("success");
    return ret;
}

ConstantSP unsubscribe(Heap *heap, vector<ConstantSP> &arguments) {
    AmdQuote* amdQuotePtr = (AmdQuote*)arguments[0]->getLong();
    AmdQuote* instance = AmdQuote::getInstance();
    if (amdQuotePtr == nullptr || instance != amdQuotePtr) {
        throw RuntimeException("first argument illegal, should the obj return by connectAmd");
    }

    std::string amdDataType;
    if (arguments[1]->getForm() != DF_SCALAR || arguments[1]->getType() != DT_STRING) {
        throw RuntimeException("second argument illegal, should be amd dataType, one of snapshot, execution, order, all");
    }
    amdDataType = arguments[1]->getString();

    if (amdDataType != "all" && arguments.size() <= 2) {
        throw RuntimeException("argument illegal, data type is not all but market and codes missed");
    }

    int marketType = 0;
    if (arguments.size() > 2) {
        if (arguments[2]->getForm() != DF_SCALAR || arguments[2]->getType() != DT_INT) {
            throw RuntimeException("third argument illegal, should be amd marketType");
        }
        marketType = arguments[2]->getInt();
    }

    std::vector<std::string> codeList;
    if (arguments.size() > 3) {
        if (arguments[3]->getForm() != DF_VECTOR || arguments[3]->getType() != DT_STRING) {
            throw RuntimeException("fourth argument illegal, should be codeList vector");
        }

        for (int i = 0; i < arguments[3]->size(); i++) {
            codeList.push_back(arguments[3]->getString(i));
        }
    }

    amdQuotePtr->unsubscribe(amdDataType, marketType, codeList);
    ConstantSP ret = Util::createConstant(DT_STRING);
    ret->setString("success");
    return ret;
}

ConstantSP amdClose(Heap *heap, vector<ConstantSP> &arguments) {
    if (arguments[0]->getForm() != DF_SCALAR) {
        throw RuntimeException("first argument illegal, should the obj return by connectAmd");
    }
    AmdQuote* amdQuotePtr = (AmdQuote*)arguments[0]->getLong();
    AmdQuote* instance = AmdQuote::getInstance();
    if (amdQuotePtr == nullptr || instance != amdQuotePtr) {
        throw RuntimeException("release Amd err, illegal AmdQuote Handler"); 
    }
    AmdQuote::deleteInstance();
    
    isConnected = false;
    receivedTimeFlag = false;

    ConstantSP ret = Util::createConstant(DT_STRING);
    ret->setString("release success");
    return ret;
}

ConstantSP getSchema(Heap *heap, vector<ConstantSP> &arguments) { // type
    if (isConnected == false) {
        throw RuntimeException("call the connect function first");
    }
    if (arguments[0]->getForm() != DF_SCALAR || arguments[0]->getType() != DT_STRING) {
        throw RuntimeException("first argument illegal, should be string");
    }
    std::string amdDataType = arguments[0]->getString();
    ConstantSP table;
    if (amdDataType == "snapshot") {
        table = getSnapshotSchema(receivedTimeFlag);
    } else if (amdDataType == "execution") {
        table = getExecutionSchema(receivedTimeFlag);
    } else if (amdDataType == "order") {
        table = getOrderSchema(receivedTimeFlag);
    } else {
        throw RuntimeException("first argument illegal, should be one of `snapshot`, `execution` or `order`");
    }

    return table;
}

ConstantSP getStatus(Heap *heap, vector<ConstantSP> &arguments) {
    if (arguments[0]->getForm() != DF_SCALAR) {
        throw RuntimeException("first argument illegal, should the obj return by connectAmd");
    }
    AmdQuote* amdQuotePtr = (AmdQuote*)arguments[0]->getLong();
    AmdQuote* instance = AmdQuote::getInstance();
    if (amdQuotePtr == nullptr || instance != amdQuotePtr) {
        throw RuntimeException("release Amd err, illegal AmdQuote Handler");
    }

    return instance->getStatus();
}

ConstantSP enableLatencyStatistics(Heap *heap, vector<ConstantSP> &arguments) {
    if (arguments[0]->getForm() != DF_SCALAR) {
        throw RuntimeException("first argument illegal, should the obj return by connectAmd");
    }
    AmdQuote* amdQuotePtr = (AmdQuote*)arguments[0]->getLong();
    AmdQuote* instance = AmdQuote::getInstance();
    if (amdQuotePtr == nullptr || instance != amdQuotePtr) {
        throw RuntimeException("release Amd err, illegal AmdQuote Handler");
    }

    instance->enableLatencyStatistics();

    ConstantSP ret = Util::createConstant(DT_STRING);
    ret->setString("release success");
    return ret;
}