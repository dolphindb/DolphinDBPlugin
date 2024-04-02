#include "amdQuote.h"
#include <string>

#include "Concurrent.h"
#include "Exceptions.h"
#include "Types.h"
#include "Util.h"
#include "amdQuoteImp.h"
#include "amdQuoteType.h"
#include "amdSpiImp.h"
#include "ddbplugin/Plugin.h"

using dolphindb::DdbVector;

BackgroundResourceMap<AmdQuote> AMD_HANDLE_MAP(AMDQUOTE_PREFIX, "amdQuote");
Mutex AMD_MUTEX;
static const string AMD_SINGLETON_NAME = "amdQuote_map_key";
static const int ZX_DAILY_TIME = 31200000;
static const unordered_set<string> OPTION_SET = {"DailyIndex", "ReceivedTime", "StartTime", "OutputElapsed", "SecurityCodeToInt"};
const long long AMD_MIN_CONNECT_MEMORY = 250 * 1024 * 1024;

void marketVerify(int market, const string &funcName, const string &syntax) {
    if(market < amd::ama::MarketType::kNone || market > amd::ama::MarketType::kMax) {
        throw IllegalArgumentException(funcName, syntax + "invalid market: " + std::to_string(market) + ".");
    }
}
void closeAmd(Heap *heap, vector<ConstantSP> &arguments) { AMD_HANDLE_MAP.safeRemoveWithoutException(arguments[0]); }

// Check to see if the available memory is sufficient for initialization
long long getRemainingMemory(Heap* heap) {
    FunctionDefSP funcGetMemory = heap->currentSession()->getFunctionDef("getMemoryStat");
    FunctionDefSP funcGetConfig = heap->currentSession()->getFunctionDef("getConfig");
    vector<ConstantSP> emptyArgs{};
    ConstantSP ret = funcGetMemory->call(heap, emptyArgs);
    // long long freeBytes = ((DictionarySP)ret)->getMember("freeBytes")->getLong();
    long long allocatedBytes = ((DictionarySP)ret)->getMember("allocatedBytes")->getLong();
    vector<ConstantSP> configArgs{};
    ConstantSP configName = new String("maxMemSize");
    configArgs.push_back(configName);
    string memRet = funcGetConfig->call(heap, configArgs)->getString();
    long long maxMemSize = std::atoi(memRet.c_str());
    long long maxMemoryPerNode = maxMemSize*1024*1024*1024;
    return maxMemoryPerNode - allocatedBytes;
}

ConstantSP amdConnect(Heap *heap, vector<ConstantSP> &arguments) {
    string usage("amdQuote::connect(username, password, ips, ports, options) ");
    LockGuard<Mutex> amdLock_(&AMD_MUTEX);
    if (arguments[0]->getForm() != DF_SCALAR || arguments[0]->getType() != DT_STRING) {
        throw IllegalArgumentException("amdQuote::connect", usage + "username should be STRING SCALAR");
    }
    string username = arguments[0]->getString();
    if (arguments[1]->getForm() != DF_SCALAR || arguments[1]->getType() != DT_STRING) {
        throw IllegalArgumentException("amdQuote::connect", usage + "password should be STRING SCALAR");
    }
    string password = arguments[1]->getString();
    if (arguments[2]->getForm() != DF_VECTOR || arguments[2]->getType() != DT_STRING) {
        throw IllegalArgumentException("amdQuote::connect", usage + "ips should be STRING VECTOR");
    }
    if (arguments[3]->getForm() != DF_VECTOR || arguments[3]->getCategory() != INTEGRAL) {
        throw IllegalArgumentException("amdQuote::connect", usage + "ports should be INTEGRAL VECTOR");
    }
    if (arguments[2]->size() != arguments[3]->size()) {
        throw IllegalArgumentException("amdQuote::connect", usage + "ips and ports should be the same length");
    }

    std::vector<string> ips;
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
    int dailyStartTime = ZX_DAILY_TIME;
    bool securityCodeToInt = false;
    if (arguments.size() > 4) {
        if (arguments[4]->getForm() != DF_DICTIONARY) {
            throw IllegalArgumentException("amdQuote::connect", usage + "options must be a DICTIONARY");
        }
        DictionarySP options = arguments[4];
        VectorSP keys = options->keys();
        VectorSP values = options->values();
        for (int i = 0; i < options->size(); ++i) {
            ConstantSP key = keys->get(i);
            if (key->getType() != DT_STRING)
                throw IllegalArgumentException("amdQuote::connect", usage + "key type of options must be STRING");
            string str = key->getString();
            if (OPTION_SET.count(str) == 0)
                throw IllegalArgumentException(
                    "amdQuote::connect",
                    usage + "key of options must be 'ReceivedTime', 'DailyIndex', 'StartTime' or 'OutputElapsed'");
        }

        ConstantSP value = options->getMember("ReceivedTime");
        if (!value->isNull()) {
            if (value->getType() != DT_BOOL || value->getForm() != DF_SCALAR) {
                throw IllegalArgumentException(
                    "amdQuote::connect", usage + "value type of key 'ReceivedTime' in options must be BOOLEAN SCALAR");
            }
            receivedTime = value->getBool();
        }
        value = options->getMember("DailyIndex");
        if (!value->isNull()) {
            if (value->getType() != DT_BOOL || value->getForm() != DF_SCALAR) {
                throw IllegalArgumentException(
                    "amdQuote::connect", usage + "value type of key 'DailyIndex' in options must be BOOLEAN SCALAR");
            }
            dailyIndex = value->getBool();
        }
        value = options->getMember("StartTime");
        if (!value->isNull()) {
            if (value->getType() != DT_TIME || value->getForm() != DF_SCALAR) {
                throw IllegalArgumentException("amdQuote::connect",
                                               usage + "value type of key 'StartTime' in options must be TIME SCALAR");
            }
            dailyStartTime = value->getInt();
        }
        value = options->getMember("OutputElapsed");
        if (!value->isNull()) {
            if (value->getType() != DT_BOOL || value->getForm() != DF_SCALAR) {
                throw IllegalArgumentException(
                    "amdQuote::connect", usage + "value type of key 'OutputElapsed' in options must be TIME SCALAR");
            }
            outputElapsed = value->getInt();
        }
        value = options->getMember("SecurityCodeToInt");
        if (!value->isNull()) {
            if (value->getType() != DT_BOOL || value->getForm() != DF_SCALAR) {
                throw IllegalArgumentException(__FUNCTION__, "value of 'SecurityCodeToInt' must be boolean");
            }
            securityCodeToInt = value->getBool();
        }
    }
    if (AMD_HANDLE_MAP.size() > 0) {
        SmartPointer<AmdQuote> handle = AMD_HANDLE_MAP.safeGetByName(AMD_SINGLETON_NAME);
        if (!handle->checkIfSame(ips, ports, receivedTime, dailyIndex, outputElapsed, dailyStartTime)) {
            throw IllegalArgumentException("amdQuote::connect",
                                           usage + "Option is different from existed AmdQuote connection.");
        }
        return AMD_HANDLE_MAP.getHandleByName(AMD_SINGLETON_NAME);
    }

    long long bytesAvailable = getRemainingMemory(heap);
    LOG_INFO(AMDQUOTE_PREFIX + "connecting when system remains available memory [" + std::to_string(bytesAvailable) + "]");
    if (bytesAvailable < AMD_MIN_CONNECT_MEMORY) {
        throw RuntimeException(AMDQUOTE_PREFIX + "The remaining memory [" + std::to_string(bytesAvailable) +
            " bytes] is not enough to initialize the amdQuote connection");
    }

    SessionSP session = heap->currentSession()->copy();
    session->setUser(heap->currentSession()->getUser());
    SmartPointer<AmdQuote> amdQuoteHandler =
        new AmdQuote(username, password, ips, ports, session, receivedTime, dailyIndex, outputElapsed, dailyStartTime, securityCodeToInt);
    string handlerName("amdQuote");
    FunctionDefSP onClose(Util::createSystemProcedure("amdQuote onClose()", closeAmd, 1, 1));
    ConstantSP resource =
        Util::createResource((long long)amdQuoteHandler.get(), handlerName, onClose, heap->currentSession());

    AMD_HANDLE_MAP.safeAdd(resource, amdQuoteHandler, AMD_SINGLETON_NAME);
    return resource;
}

bool checkDict(string type, ConstantSP dict) {
    // check if the dict's key is int and value is table
    ConstantSP keys = dict->keys();
    unordered_map<int, TableSP> tables;
    if (keys->size() == 0) {
        throw RuntimeException(AMDQUOTE_PREFIX + "The third parameter dict must not be empty");
    }
    for (int index = 0; index < keys->size(); ++index) {
        if (keys->get(index)->getType() != DT_INT) {
            throw RuntimeException(AMDQUOTE_PREFIX + "The third parameter dict's key must be int");
        }
        if (keys->get(index)->getInt() < 0) {
            throw RuntimeException(AMDQUOTE_PREFIX + "The third parameter dict's key must not less than 0");
        }
        ConstantSP value = dict->getMember(keys->get(index));
        if (value->getForm() != DF_TABLE) {
            throw RuntimeException(AMDQUOTE_PREFIX +
                                   "The third parameter dict's value should be shared stream table or IPC table or stream engine");
        }
        TABLE_TYPE tblType = ((TableSP)value)->getTableType();
        if(!(tblType == REALTIMETBL && ((TableSP)value)->isSharedTable()) && tblType != IPCTBL && tblType != STREAMENGINE) {
            throw RuntimeException(AMDQUOTE_PREFIX +
                                    "The third parameter dict's value should be shared stream table, IPC table or stream engine");
        }
        // TODO more column quantity & type check
        // if (!checkSchema(type, (TableSP)value)) {
        //     throw RuntimeException(AMDQUOTE_PREFIX + "One of dict value's table schema mismatch");
        // }
    }
    return true;
}

ConstantSP subscribe(Heap *heap, vector<ConstantSP> &arguments) {
    // subscribe may change the threadedQueue map inside the amdSpi
    string usage("amdQuote::subscribe(handle, type, outputTable, marketType, codeList, transform) ");
    LockGuard<Mutex> amdLock_(&AMD_MUTEX);
    if (arguments[0]->getType() != DT_RESOURCE || arguments[0]->getString() != "amdQuote") {
        throw IllegalArgumentException("amdQuote::subscribe", usage + "handle should be an amdQuote connection handle");
    }
    SmartPointer<AmdQuote> amdQuotePtr = AMD_HANDLE_MAP.safeGet(arguments[0]);

    if (arguments[1]->getForm() != DF_SCALAR && arguments[1]->getType() != DT_STRING) {
        throw IllegalArgumentException("amdQuote::subscribe", usage + "type should be STRING SCALAR");
    }
    string type = arguments[1]->getString();

    ConstantSP table = arguments[2];
    if (type == "orderExecution" || type == "bondOrderExecution") {
        if (!(table->getForm() == DF_DICTIONARY && checkDict(type, table))) {
            string errMsg = "type [" + type + "] must pass a dict with int key and table value into the third argument";
            throw IllegalArgumentException("amdQuote::subscribe", usage + errMsg);
        }
    } else {
        getAmdDataType(type); // use to check if type is legal, if illegal would throw exception
        if (!(table->getForm() == DF_TABLE && ((TableSP)table)->getTableType() == REALTIMETBL &&
              ((TableSP)table)->isSharedTable())) {
            if (!(table->getForm() == DF_TABLE && ((TableSP)table)->getTableType() == IPCTBL)) {
                string errMsg =
                    "The third parameter with type [" + type + "] must be a shared stream table or IPC table.";
                throw IllegalArgumentException("amdQuote::subscribe", usage + errMsg);
            }
        }
    }

    int marketType = 0;
    if (arguments.size() > 3) {
        if (arguments[3]->isNull() || arguments[3]->getForm() != DF_SCALAR || arguments[3]->getCategory() != INTEGRAL) {
            throw IllegalArgumentException("amdQuote::subscribe", usage + "market should be INTEGRAL SCALAR");
        }
        marketType = arguments[3]->getInt();
        marketVerify(marketType, "amdQuote::unsubscribe", usage);
    }
    if(marketType == 0) {throw IllegalArgumentException("amdQuote::subscribe", usage + "doesn't support market kNone(0).");}

    std::vector<string> codeList;
    if (arguments.size() > 4 && !arguments[4]->isNull()) {
        if (arguments[4]->getForm() != DF_VECTOR || arguments[4]->getType() != DT_STRING) {
            throw IllegalArgumentException("amdQuote::subscribe", usage + "codeList should be STRING VECTOR");
        }
        for (int i = 0; i < arguments[4]->size(); i++) {
            codeList.push_back(arguments[4]->getString(i));
        }
    }

    FunctionDefSP transform;
    if (arguments.size() > 5 && !arguments[5]->isNull()) {
        if (arguments[5]->getForm() != DF_SCALAR || arguments[5]->getType() != DT_FUNCTIONDEF) {
            throw IllegalArgumentException("amdQuote::subscribe", usage + "transform should be FUNCTIONDEF SCALAR");
        }
        transform = arguments[5];
    }

    // TODO check schema
    // if ((transform.isNull() || transform->isNull()) &&
    //     (table->getForm() == DF_TABLE && checkSchema(type, (TableSP)table) == false)) {
    //     throw RuntimeException(AMDQUOTE_PREFIX + "schema mismatch");
    // }

    long long dailyIndexStartTimesParam = LONG_LONG_MIN;
    long long currentTime = Util::toLocalTimestamp(Util::getEpochTime());

    amdQuotePtr->subscribe(heap, type, marketType, codeList, table, transform, currentTime);

    LOG_INFO(AMDQUOTE_PREFIX + "subscribe " + std::to_string(marketType) + " " + type + " after timestamp " +
             std::to_string(Util::toLocalTimestamp(Util::getEpochTime())));

    ConstantSP ret = Util::createConstant(DT_STRING);
    ret->setString("success");
    return ret;
}

ConstantSP unsubscribe(Heap *heap, vector<ConstantSP> &arguments) {
    // unsubscribe may change the threadedQueue map inside the amdSpi
    string usage("amdQuote::unsubscribe(handle, type, marketType, codeList) ");
    LockGuard<Mutex> amdLock_(&AMD_MUTEX);
    SmartPointer<AmdQuote> amdQuotePtr = AMD_HANDLE_MAP.safeGet(arguments[0]);
    string amdDataType;
    if (arguments[1]->getForm() != DF_SCALAR || arguments[1]->getType() != DT_STRING) {
        throw IllegalArgumentException("amdQuote::unsubscribe", usage + "type must be STRING SCALAR");
    }
    amdDataType = arguments[1]->getString();
    if (amdDataType != "all" && arguments.size() <= 2) {
        throw IllegalArgumentException("amdQuote::unsubscribe",
                                       usage + "data type is not all but market and codeList are missed");
    }
    int marketType = 0;
    if (arguments.size() > 2) {
        if (arguments[2]->getForm() != DF_SCALAR || arguments[2]->getCategory() != INTEGRAL) {
            throw IllegalArgumentException("amdQuote::unsubscribe", usage + "market must be INTEGRAL SCALAR");
        }
        marketType = arguments[2]->getInt();
        marketVerify(marketType, "amdQuote::unsubscribe", usage);
    }
    if(marketType == 0) {throw IllegalArgumentException("amdQuote::unsubscribe", usage + "doesn't support market kNone(0).");}
    std::vector<string> codeList;
    if (arguments.size() > 3) {
        if (arguments[3]->getForm() != DF_VECTOR || arguments[3]->getType() != DT_STRING) {
            throw IllegalArgumentException("amdQuote::unsubscribe", usage + "codeList must be STRING VECTOR");
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
    // close disable the use of amdSpi
    LockGuard<Mutex> amdLock_(&AMD_MUTEX);
    AMD_HANDLE_MAP.safeRemove(arguments[0]);
    return new String("success");
}

ConstantSP getSchema(Heap *heap, vector<ConstantSP> &arguments) {
    string usage("amdQuote::getSchema(type) ");
    if (AMD_HANDLE_MAP.size() != 1) {
        throw RuntimeException(AMDQUOTE_PREFIX + " please call the connect function first");
    }
    if (arguments[0]->getForm() != DF_SCALAR || arguments[0]->getType() != DT_STRING) {
        throw IllegalArgumentException("amdQuote::getSchema", usage + "type should be STRING SCALAR");
    }
    string amdDataType = arguments[0]->getString();
    SmartPointer<AmdQuote> amdQuotePtr = AMD_HANDLE_MAP.safeGetByName(AMD_SINGLETON_NAME);
    TableSP ret = amdQuotePtr->getSchema(amdDataType);
    vector<string> names{"name", "type", "typeInt"};
    vector<ConstantSP> cols{ret->getColumn(0), ret->getColumn(1), ret->getColumn(2)};
    return Util::createTable(names, cols);
}

ConstantSP getStatus(Heap *heap, vector<ConstantSP> &arguments) {
    // getStatus need stable threadedQueue maps in amdSpi
    string usage("amdQuote::getStatus(handle) ");
    LockGuard<Mutex> amdLock_(&AMD_MUTEX);
    if (arguments[0]->getType() != DT_RESOURCE || arguments[0]->getString() != "amdQuote") {
        throw IllegalArgumentException("amdQuote::getStatus", usage + "handle should be an amdQuote connection handle");
    }
    SmartPointer<AmdQuote> amdQuotePtr = AMD_HANDLE_MAP.safeGet(arguments[0]);
    return amdQuotePtr->getStatus();
}

ConstantSP getHandle(Heap *heap, vector<ConstantSP> &arguments) {
    LockGuard<Mutex> amdLock_(&AMD_MUTEX);
    if (AMD_HANDLE_MAP.size() == 0) {
        throw RuntimeException(AMDQUOTE_PREFIX + "no existed amdQuote connection handle, please connect() first.");
    }
    return AMD_HANDLE_MAP.getHandleByName(AMD_SINGLETON_NAME);
}

#ifndef AMD_3_9_6
ConstantSP getCodeList(Heap *heap, vector<ConstantSP> &arguments) {
    string usage("amdQuote::getCodeList([market]) ");
    LockGuard<Mutex> amdLock_(&AMD_MUTEX);
    if (AMD_HANDLE_MAP.size() == 0) {
        throw RuntimeException(AMDQUOTE_PREFIX + "call amdQuote::connect before calling getCodeList");
    }
    SmartPointer<AmdQuote> amdQuotePtr = AMD_HANDLE_MAP.safeGetByName(AMD_SINGLETON_NAME);

    vector<int> marketList;
    if(arguments.size() == 1) {
        if(arguments[0]->getCategory() != INTEGRAL || arguments[0]->getForm() != DF_VECTOR) {
            throw IllegalArgumentException("amdQuote::getCodeList", usage + "markets must be integer vector.");
        }
        for(int i = 0; i < arguments[0]->size(); ++i) {
            long long market = arguments[0]->getLong(i);
            marketVerify(market, "amdQuote::getCodeList", usage);
            marketList.emplace_back(market);
        }
    } else {
        marketList = {amd::ama::MarketType::kSZSE, amd::ama::MarketType::kSSE};
    }

    amd::ama::CodeTableRecordList list;
    amd::ama::SubCodeTableItem* sub = new amd::ama::SubCodeTableItem[marketList.size()];
    PluginDefer defer([=](){
        delete[] sub;
    });
    for (auto i = 0u; i <marketList.size(); ++i) {
        sub[i].market = marketList[i];
        strcpy(sub[i].security_code, "\0");
    }

    bool ret;
    try {
        ret = amd::ama::IAMDApi::GetCodeTableList(list, sub, marketList.size());
    } catch (std::exception &exception) {
        throw RuntimeException(string(AMDQUOTE_PREFIX + "") + exception.what());
    }

    vector<ConstantSP> cols;
    vector<string> colName = {"securityCode",
                              "marketType",
                              "symbol",
                              "englishName",
                              "securityType",
                              "currency",
                              "varietyCategory",
                              "preClosePrice",
                              "closePrice",
                              "underlyingSecurityId",
                              "contractType",
                              "exercisePrice",
                              "expireDate",
                              "highLimited",
                              "lowLimited",
                              "securityStatus",
                              "priceTick",
                              "buyQtyUnit",
                              "sellQtyUnit",
                              "marketBuyQtyUnit",
                              "marketSellQtyUnit",
                              "buyQtyLowerLimit",
                              "buyQtyUpperLimit",
                              "sellQtyLowerLimit",
                              "sellQtyUpperLimit",
                              "marketBuyQtyLowerLimit",
                              "marketBuyQtyUpperLimit",
                              "marketSellQtyLowerLimit",
                              "marketSellQtyUpperLimit",
                              "listDay",
                              "parValue",
                              "outstandingShare",
                              "publicFloatShareQuantity",
                              "contractMultiplier",
                              "regularShare",
                              "interest",
                              "couponRate"};
    if (ret) {
        DdbVector<string> security_code(0, list.list_nums);  // 证券代码
        DdbVector<int> market_type(0, list.list_nums);       // 证券市场
        DdbVector<string> symbol(0, list.list_nums);         // 简称
        DdbVector<string> english_name(0, list.list_nums);   // 英文名
        DdbVector<string> security_type(0, list.list_nums);  // 证券子类别
        DdbVector<string> currency(
            0,
            list.list_nums);  // 币种(CNY:人民币,HKD:港币,USD:美元,AUD:澳币,CAD:加币,JPY:日圆,SGD:新加坡币,GBP:英镑,EUR:欧元)
        DdbVector<int> variety_category(0, list.list_nums);       // 证券类别
        DdbVector<long long> pre_close_price(0, list.list_nums);  // 昨收价(类型:价格)
        DdbVector<long long> close_price(0, list.list_nums);      // 收盘价(已弃用,固定赋值为0,类型:价格)
        DdbVector<string> underlying_security_id(0, list.list_nums);   // 标的代码 (仅期权/权证有效)
        DdbVector<string> contract_type(0, list.list_nums);            // 合约类别 (仅期权有效)
        DdbVector<long long> exercise_price(0, list.list_nums);        // 行权价(仅期权有效，类型:价格)
        DdbVector<int> expire_date(0, list.list_nums);                 // 到期日 (仅期权有效)
        DdbVector<long long> high_limited(0, list.list_nums);          // 涨停价(类型:价格)
        DdbVector<long long> low_limited(0, list.list_nums);           // 跌停价(类型:价格)
        DdbVector<string> security_status(0, list.list_nums);          // 产品状态标志
        DdbVector<long long> price_tick(0, list.list_nums);            // 最小价格变动单位(类型:价格)
        DdbVector<long long> buy_qty_unit(0, list.list_nums);          // 限价买数量单位(类型:数量)
        DdbVector<long long> sell_qty_unit(0, list.list_nums);         // 限价卖数量单位(类型:数量)
        DdbVector<long long> market_buy_qty_unit(0, list.list_nums);   // 市价买数量单位(类型:数量)
        DdbVector<long long> market_sell_qty_unit(0, list.list_nums);  // 市价卖数量单位(类型:数量)
        DdbVector<long long> buy_qty_lower_limit(0, list.list_nums);   // 限价买数量下限(类型:数量)
        DdbVector<long long> buy_qty_upper_limit(0, list.list_nums);   // 限价买数量上限(类型:数量)
        DdbVector<long long> sell_qty_lower_limit(0, list.list_nums);  // 限价卖数量下限(类型:数量)
        DdbVector<long long> sell_qty_upper_limit(0, list.list_nums);  // 限价卖数量上限(类型:数量)
        DdbVector<long long> market_buy_qty_lower_limit(0, list.list_nums);   // 市价买数量下限 (类型:数量)
        DdbVector<long long> market_buy_qty_upper_limit(0, list.list_nums);   // 市价买数量上限 (类型:数量)
        DdbVector<long long> market_sell_qty_lower_limit(0, list.list_nums);  // 市价卖数量下限 (类型:数量)
        DdbVector<long long> market_sell_qty_upper_limit(0, list.list_nums);  // 市价卖数量上限 (类型:数量)
        DdbVector<int> list_day(0, list.list_nums);                           // 上市日期
        DdbVector<long long> par_value(0, list.list_nums);                    // 面值(类型:价格)
        DdbVector<long long> outstanding_share(0, list.list_nums);  // 总发行量(上交所不支持,类型:数量)
        DdbVector<long long> public_float_share_quantity(0, list.list_nums);  // 流通股数(上交所不支持,类型:数量)
        DdbVector<long long> contract_multiplier(0, list.list_nums);  // 对回购标准券折算率(类型:比例)
        DdbVector<string> regular_share(0, list.list_nums);           // 对应回购标准券(仅深交所)
        DdbVector<long long> interest(0, list.list_nums);             // 应计利息(类型:汇率)
        DdbVector<long long> coupon_rate(0, list.list_nums);          // 票面年利率(类型:比例)

        for (uint32_t i = 0; i < list.list_nums; i++) {
            /*
                handle list.records
                records 是代码表数据头指针
            */
            // std::cout << amd::ama::Tools::Serialize(list.records[i]) << std::endl;
            security_code.add(list.records[i].security_code);  // 证券代码
            market_type.add(list.records[i].market_type);      // 证券市场
            symbol.add(list.records[i].symbol);                // 简称
            english_name.add(list.records[i].english_name);    // 英文名
            security_type.add(list.records[i].security_type);  // 证券子类别
            currency.add(
                list.records[i]
                    .currency);  // 币种(CNY:人民币,HKD:港币,USD:美元,AUD:澳币,CAD:加币,JPY:日圆,SGD:新加坡币,GBP:英镑,EUR:欧元)
            variety_category.add(list.records[i].variety_category);  // 证券类别
            pre_close_price.add(list.records[i].pre_close_price);    // 昨收价(类型:价格)
            close_price.add(list.records[i].close_price);  // 收盘价(已弃用,固定赋值为0,类型:价格)
            underlying_security_id.add(list.records[i].underlying_security_id);  // 标的代码 (仅期权/权证有效)
            contract_type.add(list.records[i].contract_type);                    // 合约类别 (仅期权有效)
            exercise_price.add(list.records[i].exercise_price);            // 行权价(仅期权有效，类型:价格)
            expire_date.add(countDays(list.records[i].expire_date));       // 到期日 (仅期权有效)
            high_limited.add(list.records[i].high_limited);                // 涨停价(类型:价格)
            low_limited.add(list.records[i].low_limited);                  // 跌停价(类型:价格)
            security_status.add(list.records[i].security_status);          // 产品状态标志
            price_tick.add(list.records[i].price_tick);                    // 最小价格变动单位(类型:价格)
            buy_qty_unit.add(list.records[i].buy_qty_unit);                // 限价买数量单位(类型:数量)
            sell_qty_unit.add(list.records[i].sell_qty_unit);              // 限价卖数量单位(类型:数量)
            market_buy_qty_unit.add(list.records[i].market_buy_qty_unit);  // 市价买数量单位(类型:数量)
            market_sell_qty_unit.add(list.records[i].market_sell_qty_unit);  // 市价卖数量单位(类型:数量)
            buy_qty_lower_limit.add(list.records[i].buy_qty_lower_limit);    // 限价买数量下限(类型:数量)
            buy_qty_upper_limit.add(list.records[i].buy_qty_upper_limit);    // 限价买数量上限(类型:数量)
            sell_qty_lower_limit.add(list.records[i].sell_qty_lower_limit);  // 限价卖数量下限(类型:数量)
            sell_qty_upper_limit.add(list.records[i].sell_qty_upper_limit);  // 限价卖数量上限(类型:数量)
            market_buy_qty_lower_limit.add(list.records[i].market_buy_qty_lower_limit);  // 市价买数量下限 (类型:数量)
            market_buy_qty_upper_limit.add(list.records[i].market_buy_qty_upper_limit);  // 市价买数量上限 (类型:数量)
            market_sell_qty_lower_limit.add(list.records[i].market_sell_qty_lower_limit);  // 市价卖数量下限 (类型:数量)
            market_sell_qty_upper_limit.add(list.records[i].market_sell_qty_upper_limit);  // 市价卖数量上限 (类型:数量)
            list_day.add(list.records[i].list_day);                                        // 上市日期
            par_value.add(list.records[i].par_value);                                      // 面值(类型:价格)
            outstanding_share.add(list.records[i].outstanding_share);  // 总发行量(上交所不支持,类型:数量)
            public_float_share_quantity.add(
                list.records[i].public_float_share_quantity);  // 流通股数(上交所不支持,类型:数量)
            contract_multiplier.add(list.records[i].contract_multiplier);  // 对回购标准券折算率(类型:比例)
            regular_share.add(list.records[i].regular_share);              // 对应回购标准券(仅深交所)
            interest.add(list.records[i].interest);                        // 应计利息(类型:汇率)
            coupon_rate.add(list.records[i].coupon_rate);                  // 票面年利率(类型:比例)
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
        if (list.list_nums > 0) amd::ama::IAMDApi::FreeMemory(list.records);  //释放代码表内存池数据
    } else {
        throw RuntimeException(AMDQUOTE_PREFIX + "GetCodeList failed");
    }
    return Util::createTable(colName, cols);
}

ConstantSP getETFCodeList(Heap *heap, vector<ConstantSP> &arguments) {
    string usage("amdQuote::getETFCodeList([market]) ");
    LockGuard<Mutex> amdLock_(&AMD_MUTEX);
    if (AMD_HANDLE_MAP.size() == 0) {
        throw RuntimeException(AMDQUOTE_PREFIX + "call amdQuote::connect before calling getETFCodeList");
    }
    SmartPointer<AmdQuote> amdQuotePtr = AMD_HANDLE_MAP.safeGetByName(AMD_SINGLETON_NAME);

    vector<int> marketList;
    if(arguments.size() == 1) {
        if(arguments[0]->getCategory() != INTEGRAL || arguments[0]->getForm() != DF_VECTOR) {
            throw IllegalArgumentException("amdQuote::getETFCodeList", usage + "markets must be integer vector.");
        }
        for(int i = 0; i < arguments[0]->size(); ++i) {
            long long market = arguments[0]->getLong(i);
            marketVerify(market, "amdQuote::getETFCodeList", usage);
            marketList.emplace_back(market);
        }
    } else {
        marketList = {amd::ama::MarketType::kSZSE, amd::ama::MarketType::kSSE};
    }

    amd::ama::ETFCodeTableRecordList list;
    amd::ama::ETFItem * etf = new amd::ama::ETFItem[marketList.size()];
    PluginDefer defer([=](){
        delete[] etf;
    });
    for (auto i = 0u; i <marketList.size(); ++i) {
        etf[i].market = marketList[i];
        strcpy(etf[i].security_code, "\0");
    }

    bool ret;
    try {
        ret = amd::ama::IAMDApi::GetETFCodeTableList(list, etf, marketList.size());
    } catch (std::exception &exception) {
        throw RuntimeException(string(AMDQUOTE_PREFIX + "") + exception.what());
    }

    vector<ConstantSP> cols;
    vector<string> colName = {
        "securityCode",
        "creationRedemptionUnit",
        "maxCashRatio",
        "publish",
        "creation",
        "redemption",
        "creationRedemptionSwitch",
        "recordNum",
        "totalRecordNum",
        "estimateCashComponent",  //预估现金差额(类型:金额)
        "tradingDay",
        "preTradingDay",
        "cashComponent",
        "navPerCu",
        "nav",
        "marketType",
        "symbol",
        "fundManagementCompany",
        "underlyingSecurityId",
        "underlyingSecurityIdSource",
        "dividendPerCu",  //红利金额(类型:金额)
        "creationLimit",
        "redemptionLimit",
        "creationLimitPerUser",
        "redemptionLimitPerUser",
        "netCreationLimit",
        "netRedemptionLimit",
        "netCreationLimitPerUser",
        "netRedemptionLimitPerUser",  //单个账户净赎回总额限制,为 0 表示没有限制(仅深圳有效,类型:数量)
        "allCashFlag",
        "allCashAmount",
        "allCashPremiumRate",
        "allCashDiscountRate",
        "rtgsFlag",
        "reserved"};
    if (ret) {
        DdbVector<string> security_code(0, list.etf_list_nums);                //证券代码
        DdbVector<long long> creation_redemption_unit(0, list.etf_list_nums);  //每个篮子对应的ETF份数(类型:数量)
        DdbVector<long long> max_cash_ratio(0, list.etf_list_nums);            //最大现金替代比例(类型:比例)
        DdbVector<char> publish(0, list.etf_list_nums);                        //是否发布 IOPV,Y=是, N=否
        DdbVector<char> creation(0, list.etf_list_nums);    //是否允许申购,Y=是, N=否(仅深圳有效)
        DdbVector<char> redemption(0, list.etf_list_nums);  //是否允许赎回,Y=是, N=否(仅深圳有效)
        DdbVector<int> creation_redemption_switch(
            0, list.etf_list_nums);  //申购赎回切换(仅上海有效,0 - 不允许申购/赎回, 1 - 申购和赎回皆允许, 2 -
                                     //仅允许申购, 3 - 仅允许赎回)
        DdbVector<long long> record_num(0, list.etf_list_nums);               //深市成份证券数目(类型:数量)
        DdbVector<long long> total_record_num(0, list.etf_list_nums);         //所有成份证券数量(类型:数量)
        DdbVector<long long> estimate_cash_component(0, list.etf_list_nums);  //预估现金差额(类型:金额)
        DdbVector<int> trading_day(0, list.etf_list_nums);                    //当前交易日(格式:YYYYMMDD)
        DdbVector<int> pre_trading_day(0, list.etf_list_nums);                //前一交易日(格式:YYYYMMDD)
        DdbVector<long long> cash_component(0, list.etf_list_nums);           //前一日现金差额(类型:金额)
        DdbVector<long long> nav_per_cu(0, list.etf_list_nums);  //前一日最小申赎单位净值(类型:价格)
        DdbVector<long long> nav(0, list.etf_list_nums);         //前一日基金份额净值(类型:价格)
        DdbVector<int> market_type(0, list.etf_list_nums);       //证券所属市场(参考 MarketType)
        DdbVector<string> symbol(0, list.etf_list_nums);         //基金名称(仅深圳有效)
        DdbVector<string> fund_management_company(0, list.etf_list_nums);        //基金公司名称(仅深圳有效)
        DdbVector<string> underlying_security_id(0, list.etf_list_nums);         //拟合指数代码(仅深圳有效)
        DdbVector<string> underlying_security_id_source(0, list.etf_list_nums);  //拟合指数代码源(仅深圳有效)
        DdbVector<long long> dividend_per_cu(0, list.etf_list_nums);             //红利金额(类型:金额)
        DdbVector<long long> creation_limit(
            0, list.etf_list_nums);  //累计申购总额限制,为 0 表示没有限制(仅深圳有效,类型:数量)
        DdbVector<long long> redemption_limit(
            0, list.etf_list_nums);  //累计赎回总额限制,为 0 表示没有限制(仅深圳有效,类型:数量)
        DdbVector<long long> creation_limit_per_user(
            0, list.etf_list_nums);  //单个账户累计申购总额限制,为 0 表示没有限制(仅深圳有效,类型:数量)
        DdbVector<long long> redemption_limit_per_user(
            0, list.etf_list_nums);  //单个账户累计赎回总额限制,为 0 表示没有限制(仅深圳有效,类型:数量)
        DdbVector<long long> net_creation_limit(
            0, list.etf_list_nums);  //净申购总额限制,为 0 表示没有限制(仅深圳有效,类型:数量)
        DdbVector<long long> net_redemption_limit(
            0, list.etf_list_nums);  //净赎回总额限制,为 0 表示没有限制(仅深圳有效,类型:数量)
        DdbVector<long long> net_creation_limit_per_user(
            0, list.etf_list_nums);  //单个账户净申购总额限制,为 0 表示没有限制(仅深圳有效,类型:数量)
        DdbVector<long long> net_redemption_limit_per_user(
            0, list.etf_list_nums);  //单个账户净赎回总额限制,为 0 表示没有限制(仅深圳有效,类型:数量)
        DdbVector<char> all_cash_flag(0, list.etf_list_nums);  //是否支持全现金申赎(暂时未启用,取值为空)
        DdbVector<string> all_cash_amount(0, list.etf_list_nums);  //全现金替代的总金额(暂时未启用,取值为空)
        DdbVector<string> all_cash_premium_rate(0, list.etf_list_nums);  //全现金替代的申购溢价比例(暂时未启用,取值为空)
        DdbVector<string> all_cash_discount_rate(0,
                                                 list.etf_list_nums);  //全现金替代的赎回折价比例(暂时未启用,取值为空)
        DdbVector<char> rtgs_flag(0, list.etf_list_nums);   //是否支持RTGS(暂时未启用,取值为空)
        DdbVector<string> reserved(0, list.etf_list_nums);  //预留字段(暂时未启用,取值为空)
        for (uint32_t i = 0; i < list.etf_list_nums; i++) {
            /*
                etf_records 是ETF代码表数据头指针
            */
            // std::cout << amd::ama::Tools::Serialize(list.records[i]) << std::endl;
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
        if (list.etf_list_nums > 0) amd::ama::IAMDApi::FreeMemory(list.etf_records);  //释放ETF代码表内存池数据
    } else {
        throw RuntimeException(AMDQUOTE_PREFIX + "getETFCodeList failed");
    }
    return Util::createTable(colName, cols);
}
#endif

ConstantSP setErrorLog(Heap *heap, vector<ConstantSP> &arguments) {
    const string usage = "amdQuote::setLogError(flag)";
    if (arguments[0]->getType() != DT_BOOL || arguments[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "flag must be a BOOL SCALAR.");
    }
    ERROR_LOG.store(arguments[0]->getBool());
    return new Void();
}