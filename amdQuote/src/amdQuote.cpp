#include "amdQuote.h"
#include "Concurrent.h"
#include "CoreConcept.h"
#include "Exceptions.h"
#include "SmartPointer.h"
#include "Types.h"
#include "amdQuoteType.h"
#include <iostream>
#include <ostream>
#include "amdSpiImp.h"
#include "amdQuoteImp.h"

Mutex AmdQuote::amdMutex_;
AmdQuote* AmdQuote::instance_;

bool receivedTimeFlag = false;
bool dailyIndexFlag = false;
bool outputElapsedFlag = false;
bool isConnected = false;
bool stopTest = true;
int dailyStartTime = INT_MIN;
const int zxDailyTime = 31200000;

unordered_set<string> optionsSet = {"DailyIndex", "ReceivedTime", "StartTime", "OutputElapsed"};
ConstantSP amdConnect(Heap *heap, vector<ConstantSP> &arguments) {
    LockGuard<Mutex> amdLock_(&AmdQuote::amdMutex_);
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

    bool receivedTime = false;
    bool dailyIndex = false;
    bool outputElapsed = false;
    dailyStartTime = INT_MIN;
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
            if(optionsSet.count(str) == 0)
                throw IllegalArgumentException(__FUNCTION__, "key of options must be 'ReceivedTime', 'DailyIndex', 'StartTime', 'OutputElapsed'");
        }

        ConstantSP value = options->getMember("ReceivedTime");
        if(!value.isNull() && !value->isNull()){
            if (value->getType() != DT_BOOL || value->getForm() != DF_SCALAR) {
                throw IllegalArgumentException(__FUNCTION__, "value of 'ReceivedTime' must be boolean");
            }
            receivedTime = value->getBool();
        }

        value = options->getMember("DailyIndex");
        if(!value.isNull() && !value->isNull()){
            if (value->getType() != DT_BOOL || value->getForm() != DF_SCALAR) {
                throw IllegalArgumentException(__FUNCTION__, "value of 'DailyIndex' must be boolean");
            }
            dailyIndex = value->getBool();
        }

        value = options->getMember("StartTime");
        if(!value.isNull() && !value->isNull()){
            if (value->getType() != DT_TIME || value->getForm() != DF_SCALAR) {
                throw IllegalArgumentException(__FUNCTION__, "value of 'StartTime' must be time");
            }
            dailyStartTime = value->getInt();
        }

        value = options->getMember("OutputElapsed");
        if(!value.isNull() && !value->isNull()){
            if (value->getType() != DT_BOOL || value->getForm() != DF_SCALAR) {
                throw IllegalArgumentException(__FUNCTION__, "value of 'OutputElapsed' must be boolean");
            }
            outputElapsed = value->getInt();
        } else {
            outputElapsed = false;      //in case of inherit last connection option.
        }
    }
    if(AmdQuote::instanceValid()) {
        if(receivedTime != receivedTimeFlag || dailyIndex != dailyIndexFlag || outputElapsed != outputElapsedFlag) {
            throw RuntimeException("Option is different from existed AmdQuote connection.");
        }
    } else {
        receivedTimeFlag = receivedTime;
        dailyIndexFlag = dailyIndex;
        outputElapsedFlag = outputElapsed;
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

bool checkDict(string type, ConstantSP dict) {
    // check if the dict's key is int and value is table
    ConstantSP keys = dict->keys();
    std::unordered_map<int, TableSP> tables = {};
    if(keys->size() == 0) {
        throw RuntimeException("The third parameter dict must not be empty");
    }
    for(int index = 0; index < keys->size(); ++index) {
        if(keys->get(index)->getType() != DT_INT) {
            throw RuntimeException("The third parameter dict's key must be int");
        }
        if(keys->get(index)->getInt() < 0) {
            throw RuntimeException("The third parameter dict's key must not less than 0");
        }
        ConstantSP value = dict->getMember(keys->get(index));
        if(value->getForm() != DF_TABLE) {
            throw RuntimeException("The third parameter dict's value should be shared stream table");
        }
        if(!(((TableSP)value)->getTableType() == REALTIMETBL && ((TableSP)value)->isSharedTable())) {
            throw RuntimeException("The third parameter dict's value should be shared stream table");
        }
        if (!checkSchema(type, (TableSP)value)) {
            throw RuntimeException("One of dict value's table schema mismatch");
        }
    }
    //TODO more column quantity & type check
    return true;
}

ConstantSP subscribe(Heap *heap, vector<ConstantSP> &arguments) { // amdHandler type(snapshot execution order) table marketType codeList
    LockGuard<Mutex> amdLock_(&AmdQuote::amdMutex_);
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

    // if (arguments[2]->getForm() != DF_TABLE) {
    //     std::string errMsg = "The third parameter `streamTable` must be a shared stream table.";
    //     throw RuntimeException(errMsg);
    // }

    ConstantSP table = arguments[2];
    if(type == "orderExecution" || type == "fundOrderExecution" || type == "bondOrderExecution") {
        if (!(table->getForm() == DF_DICTIONARY && checkDict(type, table))) {
            std::string errMsg = "The third parameter with type " + type + " must be a dict with int key and table value.";
            throw RuntimeException(errMsg);
        }
    } else {
        if(!(table->getForm() ==DF_TABLE && ((TableSP)table)->getTableType() == REALTIMETBL && ((TableSP)table)->isSharedTable())) {
            std::string errMsg = "The third parameter with type " + type + " must be a shared stream table.";
            throw RuntimeException(errMsg);
        }
    }

    int marketType = 0;
    if (arguments.size() > 3) {
        if (arguments[3]->getForm() != DF_SCALAR || arguments[3]->getType() != DT_INT) {
            throw RuntimeException("fourth argument illegal, should be amd marketType");
        }
        marketType = arguments[3]->getInt();
    }

    std::vector<std::string> codeList;
    if (arguments.size() > 4 && !arguments[4]->isNothing()) {
        if (arguments[4]->getForm() != DF_VECTOR || arguments[4]->getType() != DT_STRING) {
            throw RuntimeException("fifth argument illegal, should be codeList vector");
        }
        for (int i = 0; i < arguments[4]->size(); i++) {
            codeList.push_back(arguments[4]->getString(i));
        }
    }

    FunctionDefSP transform;
    if(arguments.size() > 5){
        if (arguments[5]->getForm() != DF_SCALAR || arguments[5]->getType() != DT_FUNCTIONDEF) {
            throw RuntimeException("sixth argument illegal, transform should be a function");
        }
        transform = arguments[5];
    }

    if ((transform.isNull() || transform->isNull()) && (table->getForm() == DF_TABLE && checkSchema(type, (TableSP)table) == false)) {
        throw RuntimeException("schema mismatch");
    }

    long long dailyIndexStartTimesParam = LONG_LONG_MIN;
    if(dailyIndexFlag){
        if(marketType != 101 && marketType != 102)
            throw RuntimeException("The dailyIndex can be set only when the market is 101 or 102");
        int startTime;
        if(dailyStartTime == INT_MIN)
            startTime = zxDailyTime;
        else
            startTime = dailyStartTime;
        // FIXME time rule should explain
        const long long dateTimestamp = 24 * 60 * 60 * 1000;                // a mod
        long long now = Util::getEpochTime() + 8 * 3600 * 1000;             // add 8 hours
        long long diff = now % dateTimestamp;                               // a day left
        dailyIndexStartTimesParam = now / dateTimestamp * dateTimestamp;    // a day only
        if(diff >= startTime){                                              // 
            dailyIndexStartTimesParam += dateTimestamp;
        }
    }
    LOG_INFO("[PluginAmdQuote]: subscribe " + std::to_string(marketType) + " " + type + " after timestamp " + std::to_string(dailyIndexStartTimesParam));
    if (type == "snapshot") {
        amdQuotePtr->subscribeSnapshot(marketType, (TableSP)table, codeList, transform, receivedTimeFlag, dailyIndexStartTimesParam);
    } else if (type == "execution") {
        amdQuotePtr->subscribeExecution(marketType, (TableSP)table, codeList, transform, receivedTimeFlag, dailyIndexStartTimesParam);
    } else if (type == "order") {
        amdQuotePtr->subscribeOrder(marketType, (TableSP)table, codeList, transform, receivedTimeFlag, dailyIndexStartTimesParam);
    } else if (type == "index") {
        amdQuotePtr->subscribeIndex(marketType, (TableSP)table, codeList, transform, receivedTimeFlag, dailyIndexStartTimesParam);
    } else if (type == "orderQueue") {
        amdQuotePtr->subscribeOrderQueue(marketType, (TableSP)table, codeList, transform, receivedTimeFlag, dailyIndexStartTimesParam);
    } else if (type == "fundSnapshot") {
        amdQuotePtr->subscribeFundSnapshot(marketType, (TableSP)table, codeList, transform, receivedTimeFlag, dailyIndexStartTimesParam);
    } else if (type == "fundExecution") {
        amdQuotePtr->subscribeFundExecution(marketType, (TableSP)table, codeList, transform, receivedTimeFlag, dailyIndexStartTimesParam);
    } else if (type == "fundOrder") {
        amdQuotePtr->subscribeFundOrder(marketType, (TableSP)table, codeList, transform, receivedTimeFlag, dailyIndexStartTimesParam);
    } else if (type == "bondSnapshot") {
        amdQuotePtr->subscribeBondSnapshot(marketType, (TableSP)table, codeList, transform, receivedTimeFlag, dailyIndexStartTimesParam);
    } else if (type == "bondExecution") {
        amdQuotePtr->subscribeBondExecution(marketType, (TableSP)table, codeList, transform, receivedTimeFlag, dailyIndexStartTimesParam);
    } else if (type == "bondOrder") {
        amdQuotePtr->subscribeBondOrder(marketType, (TableSP)table, codeList, transform, receivedTimeFlag, dailyIndexStartTimesParam);

    } else if (type == "orderExecution" || type == "fundOrderExecution" || type == "bondOrderExecution") {
        amdQuotePtr->subscribeOrderExecution(type, marketType, (DictionarySP)table, codeList, transform, receivedTimeFlag, dailyIndexStartTimesParam);

    } else {
        throw RuntimeException("second argument illegal, should be `snapshot`, `execution`, `order`, `index`, `orderQueue`, fundSnapshot, `fundExecution`, `fundOrder, 'orderExecution', 'fundOrderExecution' or 'bondOrderExecution'");
    }

    ConstantSP ret = Util::createConstant(DT_STRING);
    ret->setString("success");

    return ret;
}

ConstantSP unsubscribe(Heap *heap, vector<ConstantSP> &arguments) {
    LockGuard<Mutex> amdLock_(&AmdQuote::amdMutex_);
    AmdQuote* amdQuotePtr = (AmdQuote*)arguments[0]->getLong();
    AmdQuote* instance = AmdQuote::getInstance();
    if (amdQuotePtr == nullptr || instance != amdQuotePtr) {
        throw RuntimeException("first argument illegal, should the obj return by connectAmd");
    }

    std::string amdDataType;
    if (arguments[1]->getForm() != DF_SCALAR || arguments[1]->getType() != DT_STRING) {
        throw RuntimeException("second argument illegal, should be amd dataType, one of `snapshot`, `execution`, `order`, `index`, `orderQueue`, fundSnapshot, `fundExecution`, `fundOrder, 'orderExecution', 'fundOrderExecution', 'bondOrderExecution' or `all`");
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

ConstantSP enableLatencyStatistics(Heap *heap, vector<ConstantSP> &arguments) {
    LockGuard<Mutex> amdLock_(&AmdQuote::amdMutex_);
    if (arguments[0]->getForm() != DF_SCALAR) {
        throw RuntimeException("first argument illegal, should the obj return by connectAmd");
    }
    if (arguments[1]->getForm() != DF_SCALAR || arguments[1]->getType() != DT_BOOL) {
        throw RuntimeException("second argument illegal, should be a bool scalar");
    }
    AmdQuote* amdQuotePtr = (AmdQuote*)arguments[0]->getLong();
    AmdQuote* instance = AmdQuote::getInstance();
    if (amdQuotePtr == nullptr || instance != amdQuotePtr) {
        throw RuntimeException("illegal AmdQuote Handler");
    }
    bool flag = arguments[1]->getBool();
    amdQuotePtr->enableLatencyStatistics(flag);
    return new Void();
}

ConstantSP amdClose(Heap *heap, vector<ConstantSP> &arguments) {
    LockGuard<Mutex> amdLock_(&AmdQuote::amdMutex_);
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
        table = getSnapshotSchema(receivedTimeFlag, dailyIndexFlag, outputElapsedFlag);
    } else if (amdDataType == "execution") {
        table = getExecutionSchema(receivedTimeFlag, dailyIndexFlag, outputElapsedFlag);
    } else if (amdDataType == "order") {
        table = getOrderSchema(receivedTimeFlag, dailyIndexFlag, outputElapsedFlag);
    } else if (amdDataType == "index") {
        table = getIndexSchema(receivedTimeFlag, dailyIndexFlag, outputElapsedFlag);
    } else if (amdDataType == "orderQueue") {
        table = getOrderQueueSchema(receivedTimeFlag, dailyIndexFlag, outputElapsedFlag);
    } else if (amdDataType == "fundSnapshot") {
        table = getSnapshotSchema(receivedTimeFlag, dailyIndexFlag, outputElapsedFlag);
    } else if (amdDataType == "fundExecution") {
        table = getExecutionSchema(receivedTimeFlag, dailyIndexFlag, outputElapsedFlag);
    } else if (amdDataType == "fundOrder") {
        table = getOrderSchema(receivedTimeFlag, dailyIndexFlag, outputElapsedFlag);
    } else if (amdDataType == "bondSnapshot") {
        table = getSnapshotSchema(receivedTimeFlag, dailyIndexFlag, outputElapsedFlag);
    } else if (amdDataType == "bondExecution") {
        table = getExecutionSchema(receivedTimeFlag, dailyIndexFlag, outputElapsedFlag);
    } else if (amdDataType == "bondOrder") {
        table = getOrderSchema(receivedTimeFlag, dailyIndexFlag, outputElapsedFlag);
    } else if (amdDataType == "orderExecution" || amdDataType == "fundOrderExecution" || amdDataType == "bondOrderExecution") {
        table = getOrderExecutionSchema(receivedTimeFlag, dailyIndexFlag, outputElapsedFlag);
    } else {
        throw RuntimeException("first argument illegal, should be one of `snapshot`, `execution`, `order`, `index`, `orderQueue`, fundSnapshot, `fundExecution`, `fundOrder, 'orderExecution', 'fundOrderExecution' or 'bondOrderExecution'");
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
        throw RuntimeException("illegal AmdQuote Handler");
    }

    return instance->getStatus();
}

ConstantSP getCodeList(Heap *heap, vector<ConstantSP> &arguments) {
    AmdQuote* instance = AmdQuote::getInstance();
    if (instance == nullptr) {
        throw RuntimeException("call amdQuote::connect before calling getCodeList");
    }

    amd::ama::CodeTableRecordList list;

    amd::ama::SubCodeTableItem sub[2] ;
    memset(sub, 0, sizeof(sub));
    sub[0].market = amd::ama::MarketType::kSZSE;                   //查询深交所代码表的数据
    strcpy(sub[0].security_code, "\0");                            //查询全部代码代码表的数据

    sub[1].market = amd::ama::MarketType::kSSE;                    //查询上交所代码表的数据
    strcpy(sub[1].security_code, "\0");                            //查询全部代码代码表的数据

    bool ret = amd::ama::IAMDApi::GetCodeTableList(list, sub, 2);
    vector<ConstantSP> cols;
    vector<string> colName = {"securityCode", "marketType", "symbol", "englishName", "securityType", "currency", "varietyCategory", "preClosePrice", "closePrice",
    "underlyingSecurityId", "contractType", "exercisePrice", "expireDate", "highLimited", "lowLimited", "securityStatus", "priceTick", "buyQtyUnit", "sellQtyUnit",
    "marketBuyQtyUnit", "marketSellQtyUnit", "buyQtyLowerLimit", "buyQtyUpperLimit", "sellQtyLowerLimit", "sellQtyUpperLimit", "marketBuyQtyLowerLimit",
    "marketBuyQtyUpperLimit", "marketSellQtyLowerLimit", "marketSellQtyUpperLimit", "listDay", "parValue", "outstandingShare", "publicFloatShareQuantity",
    "contractMultiplier", "regularShare", "interest", "couponRate"};
    if(ret)
    {
        DdbVector<string> security_code(0, list.list_nums);                                                     // 证券代码
        DdbVector<int> market_type(0, list.list_nums);                                                          // 证券市场
        DdbVector<string> symbol(0, list.list_nums);                                                            // 简称
        DdbVector<string> english_name(0, list.list_nums);                                                      // 英文名
        DdbVector<string> security_type(0, list.list_nums);                                                     // 证券子类别
        DdbVector<string>    currency(0, list.list_nums);                                                       // 币种(CNY:人民币,HKD:港币,USD:美元,AUD:澳币,CAD:加币,JPY:日圆,SGD:新加坡币,GBP:英镑,EUR:欧元)
        DdbVector<int> variety_category(0, list.list_nums);                                                     // 证券类别
        DdbVector<long long> pre_close_price(0, list.list_nums);                                                // 昨收价(类型:价格)
        DdbVector<long long> close_price(0, list.list_nums);                                                    // 收盘价(已弃用,固定赋值为0,类型:价格)
        DdbVector<string> underlying_security_id(0, list.list_nums);                                            // 标的代码 (仅期权/权证有效)
        DdbVector<string>    contract_type(0, list.list_nums);                                                  // 合约类别 (仅期权有效)
        DdbVector<long long> exercise_price(0, list.list_nums);                                                 // 行权价(仅期权有效，类型:价格)
        DdbVector<int> expire_date(0, list.list_nums);                                                          // 到期日 (仅期权有效)  
        DdbVector<long long> high_limited(0, list.list_nums);                                                   // 涨停价(类型:价格)
        DdbVector<long long> low_limited(0, list.list_nums);                                                    // 跌停价(类型:价格)
        DdbVector<string> security_status(0, list.list_nums);                                                   // 产品状态标志
        DdbVector<long long> price_tick(0, list.list_nums);                                                     // 最小价格变动单位(类型:价格)
        DdbVector<long long> buy_qty_unit(0, list.list_nums);                                                   // 限价买数量单位(类型:数量)
        DdbVector<long long> sell_qty_unit(0, list.list_nums);                                                  // 限价卖数量单位(类型:数量)
        DdbVector<long long> market_buy_qty_unit(0, list.list_nums);                                            // 市价买数量单位(类型:数量)
        DdbVector<long long> market_sell_qty_unit(0, list.list_nums);                                           // 市价卖数量单位(类型:数量)
        DdbVector<long long> buy_qty_lower_limit(0, list.list_nums);                                            // 限价买数量下限(类型:数量)
        DdbVector<long long> buy_qty_upper_limit(0, list.list_nums);                                            // 限价买数量上限(类型:数量)
        DdbVector<long long> sell_qty_lower_limit(0, list.list_nums);                                           // 限价卖数量下限(类型:数量)
        DdbVector<long long> sell_qty_upper_limit(0, list.list_nums);                                           // 限价卖数量上限(类型:数量)
        DdbVector<long long> market_buy_qty_lower_limit(0, list.list_nums);                                     // 市价买数量下限 (类型:数量)
        DdbVector<long long> market_buy_qty_upper_limit(0, list.list_nums);                                     // 市价买数量上限 (类型:数量)
        DdbVector<long long> market_sell_qty_lower_limit(0, list.list_nums);                                    // 市价卖数量下限 (类型:数量)
        DdbVector<long long> market_sell_qty_upper_limit(0, list.list_nums);                                    // 市价卖数量上限 (类型:数量)
        DdbVector<int> list_day(0, list.list_nums);                                                             // 上市日期
        DdbVector<long long> par_value(0, list.list_nums);                                                      // 面值(类型:价格)
        DdbVector<long long> outstanding_share(0, list.list_nums);                                              // 总发行量(上交所不支持,类型:数量)
        DdbVector<long long> public_float_share_quantity(0, list.list_nums);                                    // 流通股数(上交所不支持,类型:数量)
        DdbVector<long long> contract_multiplier(0, list.list_nums);                                            // 对回购标准券折算率(类型:比例)
        DdbVector<string> regular_share(0, list.list_nums);                                                     // 对应回购标准券(仅深交所)
        DdbVector<long long> interest(0, list.list_nums);                                                       // 应计利息(类型:汇率)
        DdbVector<long long> coupon_rate(0, list.list_nums);                                                    // 票面年利率(类型:比例)

        for(uint32_t i=0; i< list.list_nums; i++)
        {
            /*
                handle list.records
                records 是代码表数据头指针
            */
            //std::cout << amd::ama::Tools::Serialize(list.records[i]) << std::endl;
            security_code.add(list.records[i].security_code);                                                   // 证券代码
            market_type.add(list.records[i].market_type);                                                       // 证券市场
            symbol.add(list.records[i].symbol);                                                                 // 简称
            english_name.add(list.records[i].english_name);                                                     // 英文名
            security_type.add(list.records[i].security_type);                                                   // 证券子类别
            currency.add(list.records[i].currency);                                                             // 币种(CNY:人民币,HKD:港币,USD:美元,AUD:澳币,CAD:加币,JPY:日圆,SGD:新加坡币,GBP:英镑,EUR:欧元)
            variety_category.add(list.records[i].variety_category);                                             // 证券类别
            pre_close_price.add(list.records[i].pre_close_price);                                               // 昨收价(类型:价格)
            close_price.add(list.records[i].close_price);                                                       // 收盘价(已弃用,固定赋值为0,类型:价格)
            underlying_security_id.add(list.records[i].underlying_security_id);                                 // 标的代码 (仅期权/权证有效)
            contract_type.add(list.records[i].contract_type);                                                   // 合约类别 (仅期权有效)
            exercise_price.add(list.records[i].exercise_price);                                                 // 行权价(仅期权有效，类型:价格)
            expire_date.add(countDays(list.records[i].expire_date));                                            // 到期日 (仅期权有效)  
            high_limited.add(list.records[i].high_limited);                                                     // 涨停价(类型:价格)
            low_limited.add(list.records[i].low_limited);                                                       // 跌停价(类型:价格)
            security_status.add(list.records[i].security_status);                                               // 产品状态标志
            price_tick.add(list.records[i].price_tick);                                                         // 最小价格变动单位(类型:价格)
            buy_qty_unit.add(list.records[i].buy_qty_unit);                                                     // 限价买数量单位(类型:数量)
            sell_qty_unit.add(list.records[i].sell_qty_unit);                                                   // 限价卖数量单位(类型:数量)
            market_buy_qty_unit.add(list.records[i].market_buy_qty_unit);                                       // 市价买数量单位(类型:数量)
            market_sell_qty_unit.add(list.records[i].market_sell_qty_unit);                                     // 市价卖数量单位(类型:数量)
            buy_qty_lower_limit.add(list.records[i].buy_qty_lower_limit);                                       // 限价买数量下限(类型:数量)
            buy_qty_upper_limit.add(list.records[i].buy_qty_upper_limit);                                       // 限价买数量上限(类型:数量)
            sell_qty_lower_limit.add(list.records[i].sell_qty_lower_limit);                                     // 限价卖数量下限(类型:数量)
            sell_qty_upper_limit.add(list.records[i].sell_qty_upper_limit);                                     // 限价卖数量上限(类型:数量)
            market_buy_qty_lower_limit.add(list.records[i].market_buy_qty_lower_limit);                         // 市价买数量下限 (类型:数量)
            market_buy_qty_upper_limit.add(list.records[i].market_buy_qty_upper_limit);                         // 市价买数量上限 (类型:数量)
            market_sell_qty_lower_limit.add(list.records[i].market_sell_qty_lower_limit);                       // 市价卖数量下限 (类型:数量)
            market_sell_qty_upper_limit.add(list.records[i].market_sell_qty_upper_limit);                       // 市价卖数量上限 (类型:数量)
            list_day.add(list.records[i].list_day);                                                             // 上市日期
            par_value.add(list.records[i].par_value);                                                           // 面值(类型:价格)
            outstanding_share.add(list.records[i].outstanding_share);                                           // 总发行量(上交所不支持,类型:数量)
            public_float_share_quantity.add(list.records[i].public_float_share_quantity);                       // 流通股数(上交所不支持,类型:数量)
            contract_multiplier.add(list.records[i].contract_multiplier);                                       // 对回购标准券折算率(类型:比例)
            regular_share.add(list.records[i].regular_share);                                                   // 对应回购标准券(仅深交所)
            interest.add(list.records[i].interest);                                                             // 应计利息(类型:汇率)
            coupon_rate.add(list.records[i].coupon_rate);                                                       // 票面年利率(类型:比例)

        }
        cols.push_back(security_code.createVector(DT_STRING));
        cols.push_back(market_type.createVector(DT_INT));
        cols.push_back(symbol.createVector(DT_STRING));
        cols.push_back(english_name.createVector(DT_STRING));
        cols.push_back(security_type.createVector(DT_STRING));
        cols.push_back(currency.createVector(DT_STRING));
        cols.push_back(variety_category.createVector(DT_INT));
        cols.push_back(pre_close_price.createVector(DT_LONG));
        cols.push_back(close_price.createVector(DT_LONG));
        cols.push_back(underlying_security_id.createVector(DT_STRING));
        cols.push_back(contract_type.createVector(DT_STRING));
        cols.push_back(exercise_price.createVector(DT_LONG));
        cols.push_back(expire_date.createVector(DT_DATE));
        cols.push_back(high_limited.createVector(DT_LONG));
        cols.push_back(low_limited.createVector(DT_LONG));
        cols.push_back(security_status.createVector(DT_STRING));
        cols.push_back(price_tick.createVector(DT_LONG));
        cols.push_back(buy_qty_unit.createVector(DT_LONG));
        cols.push_back(sell_qty_unit.createVector(DT_LONG));
        cols.push_back(market_buy_qty_unit.createVector(DT_LONG));
        cols.push_back(market_sell_qty_unit.createVector(DT_LONG));
        cols.push_back(buy_qty_lower_limit.createVector(DT_LONG));
        cols.push_back(buy_qty_upper_limit.createVector(DT_LONG));
        cols.push_back(sell_qty_lower_limit.createVector(DT_LONG));
        cols.push_back(sell_qty_upper_limit.createVector(DT_LONG));
        cols.push_back(market_buy_qty_lower_limit.createVector(DT_LONG));
        cols.push_back(market_buy_qty_upper_limit.createVector(DT_LONG));
        cols.push_back(market_sell_qty_lower_limit.createVector(DT_LONG));
        cols.push_back(market_sell_qty_upper_limit.createVector(DT_LONG));
        cols.push_back(list_day.createVector(DT_INT));
        cols.push_back(par_value.createVector(DT_LONG));
        cols.push_back(outstanding_share.createVector(DT_LONG));
        cols.push_back(public_float_share_quantity.createVector(DT_LONG));
        cols.push_back(contract_multiplier.createVector(DT_LONG));
        cols.push_back(regular_share.createVector(DT_STRING));
        cols.push_back(interest.createVector(DT_LONG));
        cols.push_back(coupon_rate.createVector(DT_LONG));
        if(list.list_nums > 0)
            amd::ama::IAMDApi::FreeMemory(list.records);  //释放代码表内存池数据
    }else{
        throw RuntimeException("GetCodeList failed");
    }
    return Util::createTable(colName, cols);
}

ConstantSP getETFCodeList(Heap *heap, vector<ConstantSP> &arguments) {
    AmdQuote* instance = AmdQuote::getInstance();
    if (instance == nullptr) {
        throw RuntimeException("call amdQuote::connect before calling getETFCodeList");
    }
    amd::ama::ETFCodeTableRecordList list;

    amd::ama::ETFItem etf[2] ;
    memset(etf, 0, sizeof(etf));
    etf[0].market = amd::ama::MarketType::kSZSE;                   //查询深交所ETF代码表的数据
    strcpy(etf[0].security_code, "\0");                            //查询全部代码ETF代码表的数据

    etf[1].market = amd::ama::MarketType::kSSE;                    //查询上交所ETF代码表的数据
    strcpy(etf[1].security_code, "\0");                            //查询全部代码ETF代码表的数据
    
    bool ret = amd::ama::IAMDApi::GetETFCodeTableList(list, etf, 2);
    vector<ConstantSP> cols;
    vector<string> colName = {"securityCode", "creationRedemptionUnit", "maxCashRatio", "publish", "creation", "redemption", "creationRedemptionSwitch", "recordNum", "totalRecordNum", "estimateCashComponent",                                                  //预估现金差额(类型:金额)
    "tradingDay", "preTradingDay", "cashComponent", "navPerCu", "nav", "marketType", "symbol", "fundManagementCompany", "underlyingSecurityId", "underlyingSecurityIdSource", "dividendPerCu",                                                        //红利金额(类型:金额)
    "creationLimit", "redemptionLimit", "creationLimitPerUser", "redemptionLimitPerUser", "netCreationLimit", "netRedemptionLimit", "netCreationLimitPerUser", "netRedemptionLimitPerUser",                                             //单个账户净赎回总额限制,为 0 表示没有限制(仅深圳有效,类型:数量)
    "allCashFlag", "allCashAmount", "allCashPremiumRate", "allCashDiscountRate", "rtgsFlag", "reserved"};
    if(ret)
    {

        DdbVector<string> security_code(0, list.etf_list_nums);                                                 //证券代码
        DdbVector<long long> creation_redemption_unit(0, list.etf_list_nums);                                   //每个篮子对应的ETF份数(类型:数量)
        DdbVector<long long> max_cash_ratio(0, list.etf_list_nums);                                             //最大现金替代比例(类型:比例)
        DdbVector<char> publish(0, list.etf_list_nums);                                                         //是否发布 IOPV,Y=是, N=否
        DdbVector<char> creation(0, list.etf_list_nums);                                                        //是否允许申购,Y=是, N=否(仅深圳有效)
        DdbVector<char> redemption(0, list.etf_list_nums);                                                      //是否允许赎回,Y=是, N=否(仅深圳有效)
        DdbVector<int> creation_redemption_switch(0, list.etf_list_nums);                                       //申购赎回切换(仅上海有效,0 - 不允许申购/赎回, 1 - 申购和赎回皆允许, 2 - 仅允许申购, 3 - 仅允许赎回)
        DdbVector<long long> record_num(0, list.etf_list_nums);                                                 //深市成份证券数目(类型:数量)
        DdbVector<long long> total_record_num(0, list.etf_list_nums);                                           //所有成份证券数量(类型:数量)
        DdbVector<long long> estimate_cash_component(0, list.etf_list_nums);                                    //预估现金差额(类型:金额)
        DdbVector<int> trading_day(0, list.etf_list_nums);                                                      //当前交易日(格式:YYYYMMDD)
        DdbVector<int> pre_trading_day(0, list.etf_list_nums);                                                  //前一交易日(格式:YYYYMMDD)
        DdbVector<long long> cash_component(0, list.etf_list_nums);                                             //前一日现金差额(类型:金额)
        DdbVector<long long> nav_per_cu(0, list.etf_list_nums);                                                 //前一日最小申赎单位净值(类型:价格)
        DdbVector<long long> nav(0, list.etf_list_nums);                                                        //前一日基金份额净值(类型:价格)
        DdbVector<int> market_type(0, list.etf_list_nums);                                                      //证券所属市场(参考 MarketType)
        DdbVector<string> symbol(0, list.etf_list_nums);                                                        //基金名称(仅深圳有效)
        DdbVector<string> fund_management_company(0, list.etf_list_nums);                                       //基金公司名称(仅深圳有效)
        DdbVector<string> underlying_security_id(0, list.etf_list_nums);                                        //拟合指数代码(仅深圳有效)
        DdbVector<string> underlying_security_id_source(0, list.etf_list_nums);                                 //拟合指数代码源(仅深圳有效)
        DdbVector<long long> dividend_per_cu(0, list.etf_list_nums);                                            //红利金额(类型:金额)
        DdbVector<long long> creation_limit(0, list.etf_list_nums);                                             //累计申购总额限制,为 0 表示没有限制(仅深圳有效,类型:数量)
        DdbVector<long long> redemption_limit(0, list.etf_list_nums);                                           //累计赎回总额限制,为 0 表示没有限制(仅深圳有效,类型:数量)
        DdbVector<long long> creation_limit_per_user(0, list.etf_list_nums);                                    //单个账户累计申购总额限制,为 0 表示没有限制(仅深圳有效,类型:数量)
        DdbVector<long long> redemption_limit_per_user(0, list.etf_list_nums);                                  //单个账户累计赎回总额限制,为 0 表示没有限制(仅深圳有效,类型:数量)
        DdbVector<long long> net_creation_limit(0, list.etf_list_nums);                                         //净申购总额限制,为 0 表示没有限制(仅深圳有效,类型:数量)
        DdbVector<long long> net_redemption_limit(0, list.etf_list_nums);                                       //净赎回总额限制,为 0 表示没有限制(仅深圳有效,类型:数量)
        DdbVector<long long> net_creation_limit_per_user(0, list.etf_list_nums);                                //单个账户净申购总额限制,为 0 表示没有限制(仅深圳有效,类型:数量)
        DdbVector<long long> net_redemption_limit_per_user(0, list.etf_list_nums);                              //单个账户净赎回总额限制,为 0 表示没有限制(仅深圳有效,类型:数量)
        DdbVector<char> all_cash_flag(0, list.etf_list_nums);                                                   //是否支持全现金申赎(暂时未启用,取值为空)
        DdbVector<string> all_cash_amount(0, list.etf_list_nums);                                               //全现金替代的总金额(暂时未启用,取值为空)
        DdbVector<string> all_cash_premium_rate(0, list.etf_list_nums);                                         //全现金替代的申购溢价比例(暂时未启用,取值为空)
        DdbVector<string> all_cash_discount_rate(0, list.etf_list_nums);                                        //全现金替代的赎回折价比例(暂时未启用,取值为空)
        DdbVector<char> rtgs_flag(0, list.etf_list_nums);                                                       //是否支持RTGS(暂时未启用,取值为空)
        DdbVector<string> reserved(0, list.etf_list_nums);                                                      //预留字段(暂时未启用,取值为空)
        for(uint32_t i=0; i< list.etf_list_nums; i++)
        {
            /*
                etf_records 是ETF代码表数据头指针
            */
            //std::cout << amd::ama::Tools::Serialize(list.records[i]) << std::endl;
            security_code.add(list.etf_records[i].security_code);
            creation_redemption_unit.add(list.etf_records[i].creation_redemption_unit);
            max_cash_ratio.add(list.etf_records[i].max_cash_ratio);
            publish.add(list.etf_records[i].publish == 'Y' ? true : false);
            creation.add(list.etf_records[i].creation == 'Y' ? true : false);
            redemption.add(list.etf_records[i].redemption == 'Y' ? true : false);
            creation_redemption_switch.add(list.etf_records[i].creation_redemption_switch);
            record_num.add(list.etf_records[i].record_num);
            total_record_num.add(list.etf_records[i].total_record_num);
            estimate_cash_component.add(list.etf_records[i].estimate_cash_component);
            trading_day.add(countDays(list.etf_records[i].trading_day));
            pre_trading_day.add(list.etf_records[i].pre_trading_day);
            cash_component.add(list.etf_records[i].cash_component);
            nav_per_cu.add(list.etf_records[i].nav_per_cu);
            nav.add(list.etf_records[i].nav);
            market_type.add(list.etf_records[i].market_type);
            symbol.add(list.etf_records[i].symbol);
            fund_management_company.add(list.etf_records[i].fund_management_company);
            underlying_security_id.add(list.etf_records[i].underlying_security_id);
            underlying_security_id_source.add(list.etf_records[i].underlying_security_id_source);
            dividend_per_cu.add(list.etf_records[i].dividend_per_cu);
            creation_limit.add(list.etf_records[i].creation_limit);
            redemption_limit.add(list.etf_records[i].redemption_limit);
            creation_limit_per_user.add(list.etf_records[i].creation_limit_per_user);
            redemption_limit_per_user.add(list.etf_records[i].redemption_limit_per_user);
            net_creation_limit.add(list.etf_records[i].net_creation_limit);
            net_redemption_limit.add(list.etf_records[i].net_redemption_limit);
            net_creation_limit_per_user.add(list.etf_records[i].net_creation_limit_per_user);
            net_redemption_limit_per_user.add(list.etf_records[i].net_redemption_limit_per_user);
            all_cash_flag.add(list.etf_records[i].all_cash_flag);
            all_cash_amount.add(list.etf_records[i].all_cash_amount);
            all_cash_premium_rate.add(list.etf_records[i].all_cash_premium_rate);
            all_cash_discount_rate.add(list.etf_records[i].all_cash_discount_rate);
            rtgs_flag.add(list.etf_records[i].rtgs_flag);
            reserved.add(list.etf_records[i].reserved);
        }
        cols.push_back(security_code.createVector(DT_STRING));
        cols.push_back(creation_redemption_unit.createVector(DT_LONG));
        cols.push_back(max_cash_ratio.createVector(DT_LONG));
        cols.push_back(publish.createVector(DT_BOOL));
        cols.push_back(creation.createVector(DT_BOOL));
        cols.push_back(redemption.createVector(DT_BOOL));
        cols.push_back(creation_redemption_switch.createVector(DT_INT));
        cols.push_back(record_num.createVector(DT_LONG));
        cols.push_back(total_record_num.createVector(DT_LONG));
        cols.push_back(estimate_cash_component.createVector(DT_LONG));
        cols.push_back(trading_day.createVector(DT_DATE));
        cols.push_back(pre_trading_day.createVector(DT_INT));
        cols.push_back(cash_component.createVector(DT_LONG));
        cols.push_back(nav_per_cu.createVector(DT_LONG));
        cols.push_back(nav.createVector(DT_LONG));
        cols.push_back(market_type.createVector(DT_INT));
        cols.push_back(symbol.createVector(DT_STRING));
        cols.push_back(fund_management_company.createVector(DT_STRING));
        cols.push_back(underlying_security_id.createVector(DT_STRING));
        cols.push_back(underlying_security_id_source.createVector(DT_STRING));
        cols.push_back(dividend_per_cu.createVector(DT_LONG));
        cols.push_back(creation_limit.createVector(DT_LONG));
        cols.push_back(redemption_limit.createVector(DT_LONG));
        cols.push_back(creation_limit_per_user.createVector(DT_LONG));
        cols.push_back(redemption_limit_per_user.createVector(DT_LONG));
        cols.push_back(net_creation_limit.createVector(DT_LONG));
        cols.push_back(net_redemption_limit.createVector(DT_LONG));
        cols.push_back(net_creation_limit_per_user.createVector(DT_LONG));
        cols.push_back(net_redemption_limit_per_user.createVector(DT_LONG));
        cols.push_back(all_cash_flag.createVector(DT_BOOL));
        cols.push_back(all_cash_amount.createVector(DT_STRING));
        cols.push_back(all_cash_premium_rate.createVector(DT_STRING));
        cols.push_back(all_cash_discount_rate.createVector(DT_STRING));
        cols.push_back(rtgs_flag.createVector(DT_BOOL));
        cols.push_back(reserved.createVector(DT_STRING));
        if(list.etf_list_nums > 0)
            amd::ama::IAMDApi::FreeMemory(list.etf_records);  //释放ETF代码表内存池数据
    }else{
        throw RuntimeException("getETFCodeList failed");
    }
    return Util::createTable(colName, cols);
}
