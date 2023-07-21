#include "insight_plugin.h"
#include "mdc_client_factory.h"
#include "client_interface.h"
#include "parameter_define.h"
#include "InsightHandle.h"
#include "base_define.h"
#include "ScalarImp.h"
#include "Logger.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
// #include <map>
#include <iostream>
#include <thread>

using namespace std;
using namespace com::htsc::mdc::gateway;
using namespace com::htsc::mdc::insight::model;
using namespace com::htsc::mdc::model;

const unordered_map<string, EMarketDataType> marketDataTypeMap = {
    {"MD_TICK", MD_TICK}, // 快照行情
    {"MD_TRANSACTION", MD_TRANSACTION}, // 逐笔成交
    {"MD_ORDER", MD_ORDER}, // 逐笔委托
};

const unordered_map<string, ESecurityIDSource> securityIdSourceMap = {
    {"XSHE", XSHE},
    {"XSHG", XSHG},
    {"CCFX", CCFX},
    {"CSI", CSI}
};

const unordered_map<string, ESecurityType> securityTypeMap = {
    {"StockType", StockType}, // 股票
    {"IndexType", IndexType}, // 指数
    {"FuturesType", FuturesType}, // 期货
};

class Info {
public:
    string marketType_;
    string securityIdSource_;
    string securityType_;
};

class InfoHash {
public:
    size_t operator() (const Info& info) const {
        return hash<string>{}(info.marketType_) ^ 
               hash<string>{}(info.securityIdSource_) ^
               hash<string>{}(info.securityType_);
    }
};

class InfoEqual {
public:
    bool operator() (const Info& info1, const Info& info2) const {
return info1.marketType_ == info2.marketType_ &&
               info1.securityIdSource_ == info2.securityIdSource_ && 
               info1.securityType_ == info2.securityType_;
    }
};


class Defer {
public:
    Defer(std::function<void()> code): code(code) {}
    ~Defer() { code(); }
private:
    std::function<void()> code;
};

class TcpClient;

typedef SmartPointer<TcpClient> TcpClientSP;

class TcpClient {
public:
    static TcpClientSP getInstance();

    ~TcpClient(){
        LOG_INFO(PLUGIN_INSIGHT_PREFIX, "client has been released. ");
    }

    void registHandleAndLogin(Heap *heap, DictionarySP handles, int workPoolThreadCount, const string &ip,
                              int port, const string &user_name, const string &password) {
        LockGuard<Mutex> _(&mu_);
        try {
            if (login_) { throw RuntimeException("handle must be registered before login. "); }
            init_env();
            Defer df1([&]() {
                if (!login_) {
                    ClientFactory::Uninstance();
                    insightHandle_ = nullptr;
                    clientInterface_ = nullptr;
                }
            });

            // close_cout_log();
            close_file_log();
            open_response_callback();
            // close_compress();
            clientInterface_ = ClientFactory::Instance()->CreateClient(true, "./cert");
            if (!clientInterface_) { throw RuntimeException("create client failed!"); }
            SessionSP session = heap->currentSession()->copy();
            session->setUser(heap->currentSession()->getUser());
            insightHandle_ = new InsightHandle(session, handles);
            clientInterface_->set_handle_pool_thread_count(workPoolThreadCount);
            clientInterface_->RegistHandle(insightHandle_.get());

            int result = clientInterface_->LoginByServiceDiscovery(ip, port, user_name, password, false);
            if (result != 0) { throw RuntimeException(get_error_code_value(result)); }
            login_ = true;
        }catch(exception &e){
            throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "failed to registHandleAndLogin: " + e.what());
        }
    }

    void logout() {
        LockGuard<Mutex> _(&mu_);
        unsubscribe();
        if (login_) {
            try {
                ClientFactory::Uninstance();
            }
            catch(exception & e) { LOG_ERR(PLUGIN_INSIGHT_PREFIX + "failed to logout: " + e.what()); }
            clientInterface_ = nullptr;
            insightHandle_ = nullptr;
            TCPCLIENT = nullptr;
            login_ = false;
            subscribed_ = false;
        }else{
            throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "client has been disconnected. ");
        }
    }

    void subscribe(VectorSP marketDataTypes, const string& securityIdSource, const string& securityType){
        LockGuard<Mutex> _(&mu_);
        try{
            if (clientInterface_ == nullptr) {
                throw RuntimeException("Please connect first");
            }
            sourceType_ = new SubscribeBySourceType();
            SubscribeBySourceTypeDetail *detail = sourceType_->add_subscribebysourcetypedetail();
            SecuritySourceType *security_source_type = new SecuritySourceType();

            auto idsrc = securityIdSourceMap.find(securityIdSource);
            if(idsrc == securityIdSourceMap.end()) {
                throw RuntimeException("unknown security id source");
            }
            security_source_type->set_securityidsource(idsrc->second);

            auto type = securityTypeMap.find(securityType);
            if(type == securityTypeMap.end()) {
                throw RuntimeException("unknown security type");
            }
            security_source_type->set_securitytype(type->second);

            detail->set_allocated_securitysourcetypes(security_source_type);
            for(int i = 0; i < marketDataTypes->size(); ++i) {
                auto mdt = marketDataTypeMap.find(marketDataTypes->get(i)->getString());
                if(mdt == marketDataTypeMap.end()) {
                    throw RuntimeException("unknown market data type");
                }
                detail->add_marketdatatypes(mdt->second);
                
                Info info;
                info.marketType_ = marketDataTypes->get(i)->getString();
                info.securityIdSource_ = securityIdSource;
                info.securityType_ = securityType;
                subscribedInfo_.insert(info);
            }

            int result = clientInterface_->SubscribeBySourceType(ADD, sourceType_.get());
            if(result != 0) {
                throw RuntimeException(get_error_code_value(result));
            }
            subscribed_ = true;
        }catch(exception& e){
            throw RuntimeException(PLUGIN_INSIGHT_PREFIX + e.what());
        }
    }

    void unsubscribe() {
        LockGuard<Mutex> _(&mu_);
        if (subscribed_) {
            if(sourceType_.get() == nullptr)
                throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "failed to unsubscribe: sourece type is null");
            if(clientInterface_ == nullptr)
                throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "failed to unsubscribe: client interface is null");
            ESubscribeActionType actionType = CANCEL;
            try{
                int ret = clientInterface_->SubscribeBySourceType(actionType, sourceType_.get());
                if(ret != 0) {
                    throw RuntimeException(get_error_code_value(ret));
            }
            }catch(exception& e){
                throw RuntimeException(PLUGIN_INSIGHT_PREFIX + e.what());
            }
        }
        subscribed_ = false;
        subscribedInfo_.clear();
    }

    void close() {
        // LockGuard<Mutex> _(&mu);
        // if(loggedIn_ && !subscribed_) {
        //     sourceType_ = new SubscribeBySourceType();
        //     sourceType_->add_subscribebysourcetypedetail();
        //     tcpClientInterface_->SubscribeBySourceType("0.0.0.0", sourceType_.get());
        //     subscribed_ = true;
        // }
        // if(subscribed_) {
        //     ClientFactory::Uninstance();
        //     tcpClientInterface_ = nullptr;
        //     sourceType_ = nullptr;
        //     loggedIn_ = false;
        //     subscribed_ = false;
        //     fini_env();
        // }
        // autoClose_ = true;
        LockGuard<Mutex> _(&mu_);
        logout();
        try{
            fini_env();
        }catch(exception& e){
            throw RuntimeException(PLUGIN_INSIGHT_PREFIX);
        }
    }
    
    unordered_set<Info, InfoHash, InfoEqual> getSubInfo(){
        LockGuard<Mutex> _(&mu_);
        return subscribedInfo_;
    }

    static TcpClientSP TCPCLIENT;
    static Mutex TCPCLIENT_LOCK;

private:
    Mutex mu_;
    SmartPointer<InsightHandle> insightHandle_ = nullptr;
    ClientInterface *clientInterface_ = nullptr;
    SmartPointer<SubscribeBySourceType> sourceType_ = nullptr;
    unordered_set<Info, InfoHash, InfoEqual> subscribedInfo_;
    bool login_ = false;
    bool subscribed_ = false;
};

TcpClientSP TcpClient::TCPCLIENT = nullptr;
Mutex TcpClient::TCPCLIENT_LOCK;


TcpClientSP safeGet(ConstantSP& argument){
    if(argument->getType() != DT_RESOURCE || argument->getString() != "client")
        throw IllegalArgumentException(__FUNCTION__, "client must be a client handle");
    TcpClient *client = reinterpret_cast<TcpClient*>(argument->getLong());
    if(client == nullptr)
        throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "client is close. handle is null.");
    if(TcpClient::TCPCLIENT.isNull())
        throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "client is close. global tcpclient is null.");
    if(client != TcpClient::TCPCLIENT.get())
        throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "client must be a client handle. ");
    return TcpClient::TCPCLIENT;
}

TcpClientSP TcpClient::getInstance(){
    if(!TcpClient::TCPCLIENT.isNull())
        throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "tcpclient is registered. ");
    TcpClient::TCPCLIENT = new TcpClient();
    return TcpClient::TCPCLIENT;
}

static void tcpOnClose(Heap *heap, vector<ConstantSP> &arguments){
    // TcpClient *tcpClient = reinterpret_cast<TcpClient*>(arguments[0]->getLong());
    // if(tcpClient != nullptr) {
    //     tcpClient->close();
    // }
}

static void registHandleArgumentValidation(vector<ConstantSP> &arguments, DictionarySP &handles, int& workPoolThreadCount) {
    if(arguments[0]->getForm() != DF_DICTIONARY)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_INSIGHT_PREFIX + "handles must be a dictionary");
    handles = arguments[0];
    {
        VectorSP keys = handles->keys();
        VectorSP values = handles->values();
        for(int i = 0; i < handles->size(); ++i){
            ConstantSP type = keys->get(i);
            if(type->getType() != DT_STRING)
                throw IllegalArgumentException(__FUNCTION__, PLUGIN_INSIGHT_PREFIX + "key of handles must be string");
            std::string str = type->getString();
            if(str != "StockTransaction" && str != "StockOrder" && str != "StockTick" && str != "IndexTick" && str != "FuturesTick")
                throw IllegalArgumentException(__FUNCTION__, PLUGIN_INSIGHT_PREFIX + "key of handles must be one of 'StockTick', 'IndexTick', 'FuturesTick', 'StockTransaction' and 'StockOrder'");
        }
    }

    workPoolThreadCount = 5;
    if(arguments.size() > 5) {
        if(arguments[5]->getCategory() != INTEGRAL || arguments[5]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, PLUGIN_INSIGHT_PREFIX + "workPoolThreadCount must be integral");
        }
        workPoolThreadCount = arguments[5]->getInt();
        if(workPoolThreadCount <= 0 || workPoolThreadCount > 32767) {
            throw IllegalArgumentException(__FUNCTION__, PLUGIN_INSIGHT_PREFIX + "workPoolThreadCount should more than 0 and less than 32767");
        }
    }
}

ConstantSP connectInsight(Heap *heap, vector<ConstantSP> &arguments) {
    LockGuard<Mutex> lock(&TcpClient::TCPCLIENT_LOCK);
    if (!TcpClient::TCPCLIENT.isNull()) {
        FunctionDefSP onClose(Util::createSystemProcedure("tcpClient onClose()", tcpOnClose, 1, 1));
        ConstantSP ret = Util::createResource(reinterpret_cast<long long>(TcpClient::TCPCLIENT.get()), "client", onClose, heap->currentSession());
        return ret;
    }

    DictionarySP handles;
    int workPoolThreadCount;
    registHandleArgumentValidation(arguments, handles, workPoolThreadCount);

    if(arguments[1]->getType() != DT_STRING || arguments[1]->getForm() != DF_SCALAR){
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_INSIGHT_PREFIX + "ip must be a string");
    }
    string ip = arguments[1]->getString();

    if(arguments[2]->getCategory() != INTEGRAL || arguments[2]->getForm() != DF_SCALAR){
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_INSIGHT_PREFIX + "port must be integral");
    }
    int port = arguments[2]->getInt();

    if(arguments[3]->getType() != DT_STRING || arguments[3]->getForm() != DF_SCALAR){
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_INSIGHT_PREFIX + "user must be a string");
    }
    string user = arguments[3]->getString();

    if(arguments[4]->getType() != DT_STRING || arguments[4]->getForm() != DF_SCALAR){
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_INSIGHT_PREFIX + "password must be a string");
    }
    string password = arguments[4]->getString();

    TcpClient::TCPCLIENT = TcpClient::getInstance();
    bool connectSuccess = false;
    Defer defer([&](){if(!connectSuccess) TcpClient::TCPCLIENT.clear();});
    TcpClient::TCPCLIENT->registHandleAndLogin(heap, handles, workPoolThreadCount, ip, port, user, password);
    
    
    FunctionDefSP onClose(Util::createSystemProcedure("tcpClient onClose()", tcpOnClose, 1, 1));
    ConstantSP ret = Util::createResource(reinterpret_cast<long long>(TcpClient::TCPCLIENT.get()), "client", onClose, heap->currentSession());
    connectSuccess = true;
    return ret;
}

ConstantSP subscribe(Heap *heap, vector<ConstantSP> &arguments) {
    LockGuard<Mutex> lock(&TcpClient::TCPCLIENT_LOCK);
    TcpClientSP client = safeGet(arguments[0]);

    if(arguments[1]->getForm() != DF_VECTOR || arguments[1]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_INSIGHT_PREFIX + "marketDataTypes must be a string vector");
    }
    VectorSP marketDataTypes = arguments[1];

    if(arguments[2]->getType() != DT_STRING || arguments[2]->getForm() != DF_SCALAR){
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_INSIGHT_PREFIX + "securityIdSource must be a string");
    }
    string securityIdSource = arguments[2]->getString();

    if(arguments[3]->getType() != DT_STRING || arguments[3]->getForm() != DF_SCALAR){
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_INSIGHT_PREFIX + "securityType must be a string");
    }
    string securityType = arguments[3]->getString();

    client->subscribe(marketDataTypes, securityIdSource, securityType);
    return new Void();
}

ConstantSP unsubscribe(Heap *heap, vector<ConstantSP> &arguments){
    LockGuard<Mutex> lock(&TcpClient::TCPCLIENT_LOCK);
    TcpClientSP tcpClient = safeGet(arguments[0]);

    tcpClient->unsubscribe();

    return new Void();
}

ConstantSP closeInsight(Heap *heap, vector<ConstantSP> &arguments){
    LockGuard<Mutex> lock(&TcpClient::TCPCLIENT_LOCK);
    if(arguments.size() < 1)
        throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "the param size of closeInsight can not be less than 1");
    TcpClientSP tcpClient = safeGet(arguments[0]);
    tcpClient->close();

    return new Void();
}

TableSP getStockSchema() {
    vector<ConstantSP> cols(2);
    cols[0] = Util::createVector(DT_STRING, InsightStockTableMeta::COLNAMES.size());
    cols[1] = Util::createVector(DT_STRING, InsightStockTableMeta::COLNAMES.size());
    for (unsigned int i = 0; i < InsightStockTableMeta::COLNAMES.size(); i++) {
        cols[0]->setString(i, InsightStockTableMeta::COLNAMES[i]);
        cols[1]->setString(i, Util::getDataTypeString(InsightStockTableMeta::COLTYPES[i]));
    }

    std::vector<string> colNames = {"name", "type"};
    TableSP table = Util::createTable(colNames, cols);

    return table;
}

TableSP getFuturesSchema() {
    vector<ConstantSP> cols(2);
    cols[0] = Util::createVector(DT_STRING, InsightFuturesTableMeta::COLNAMES.size());
    cols[1] = Util::createVector(DT_STRING, InsightFuturesTableMeta::COLNAMES.size());
    for (unsigned int i = 0; i < InsightFuturesTableMeta::COLNAMES.size(); i++) {
        cols[0]->setString(i, InsightFuturesTableMeta::COLNAMES[i]);
        cols[1]->setString(i, Util::getDataTypeString(InsightFuturesTableMeta::COLTYPES[i]));
    }

    std::vector<string> colNames = {"name", "type"};
    TableSP table = Util::createTable(colNames, cols);

    return table;
}

TableSP getTransactionSchema() {
    vector<ConstantSP> cols(2);
    cols[0] = Util::createVector(DT_STRING, InsightTransactionTableMeta::COLNAMES.size());
    cols[1] = Util::createVector(DT_STRING, InsightTransactionTableMeta::COLNAMES.size());
    for (unsigned int i = 0; i < InsightTransactionTableMeta::COLNAMES.size(); i++) {
        cols[0]->setString(i, InsightTransactionTableMeta::COLNAMES[i]);
        cols[1]->setString(i, Util::getDataTypeString(InsightTransactionTableMeta::COLTYPES[i]));
    }

    std::vector<string> colNames = {"name", "type"};
    TableSP table = Util::createTable(colNames, cols);

    return table;
}

TableSP getIndexSchema() {
    vector<ConstantSP> cols(2);
    cols[0] = Util::createVector(DT_STRING, InsightIndexTableMeta::COLNAMES.size());
    cols[1] = Util::createVector(DT_STRING, InsightIndexTableMeta::COLTYPES.size());
    for (unsigned int i = 0; i < InsightIndexTableMeta::COLNAMES.size(); i++) {
        cols[0]->setString(i, InsightIndexTableMeta::COLNAMES[i]);
        cols[1]->setString(i, Util::getDataTypeString(InsightIndexTableMeta::COLTYPES[i]));
    }
    std::vector<string> colNames = {"name", "type"};
    TableSP table = Util::createTable(colNames, cols);
    return table;
}

TableSP getOrderSchema() {
    vector<ConstantSP> cols(2);
    cols[0] = Util::createVector(DT_STRING, InsightOrderTableMeta::COLNAMES.size());
    cols[1] = Util::createVector(DT_STRING, InsightOrderTableMeta::COLNAMES.size());
    for (unsigned int i = 0; i < InsightOrderTableMeta::COLNAMES.size(); i++) {
        cols[0]->setString(i, InsightOrderTableMeta::COLNAMES[i]);
        cols[1]->setString(i, Util::getDataTypeString(InsightOrderTableMeta::COLTYPES[i]));
    }

    std::vector<string> colNames = {"name", "type"};
    TableSP table = Util::createTable(colNames, cols);

    return table;
}

ConstantSP getSchema(Heap *heap, vector<ConstantSP> &arguments) {
    LockGuard<Mutex> lock(&TcpClient::TCPCLIENT_LOCK);
    if(arguments[0]->getType() != DT_STRING){
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_INSIGHT_PREFIX + "first argument must be a string");
    }
    string type = arguments[0]->getString();
    if (type == "StockTick") {
        return getStockSchema();
    } else if (type == "IndexTick") {
        return getIndexSchema();
    } else if (type == "FuturesTick") {
        return getFuturesSchema();
    } else if (type == "StockTransaction") {
        return getTransactionSchema();
    } else if (type == "StockOrder") {
        return getOrderSchema();
    } else {
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_INSIGHT_PREFIX + "unknown schema type");
    }
}

ConstantSP getStatus(Heap *heap, vector<ConstantSP> &arguments) {
    LockGuard<Mutex> lock(&TcpClient::TCPCLIENT_LOCK);
    TcpClientSP tcpClient = safeGet(arguments[0]);

    auto subscribedInfo = tcpClient->getSubInfo();
    INDEX size = (INDEX)subscribedInfo.size();

    vector<DATA_TYPE> types{DT_STRING, DT_STRING, DT_STRING};
    vector<string> names{"marketType", "securityIdSource", "securityType"};
    std::vector<ConstantSP> cols;
    for (size_t i = 0; i < types.size(); i++) {
        cols.push_back(Util::createVector(types[i], size, size));
    }
    
    int i = 0;
    for (auto& info : subscribedInfo) {
        cols[0]->set(i, new String(info.marketType_));
        cols[1]->set(i, new String(info.securityIdSource_));
        cols[2]->set(i, new String(info.securityType_));
        i++;
    }

    return Util::createTable(names, cols);
}

std::vector<string> InsightIndexTableMeta::COLNAMES = {
            "MDDate", "MDTime", "HTSCSecurityID", "LastPx", "HighPx",
            "LowPx", "TotalVolumeTrade", "TotalValueTrade", "TradingPhaseCode", "ReceivedTime",
        };
std::vector<DATA_TYPE> InsightIndexTableMeta::COLTYPES = {
            DT_DATE, DT_TIME, DT_SYMBOL, DT_LONG, DT_LONG,
            DT_LONG, DT_LONG, DT_LONG, DT_SYMBOL, DT_NANOTIMESTAMP
    };

std::vector<string> InsightStockTableMeta::COLNAMES = {
            "HTSCSecurityID", "MDDate", "MDTime","securityIDSource", "PreClosePx", "TotalVolumeTrade", "TotalValueTrade", "LastPx", "OpenPx", "HighPx",

            "LowPx", "DiffPx1", "TotalBuyQty", "TotalSellQty", "WeightedAvgBuyPx", "WeightedAvgSellPx", "BuyPrice1", "BuyPrice2", "BuyPrice3", "BuyPrice4",

            "BuyPrice5", "BuyPrice6", "BuyPrice7", "BuyPrice8", "BuyPrice9", "BuyPrice10", "BuyOrderQty1", "BuyOrderQty2", "BuyOrderQty3", "BuyOrderQty4",

            "BuyOrderQty5", "BuyOrderQty6", "BuyOrderQty7", "BuyOrderQty8", "BuyOrderQty9", "BuyOrderQty10", "SellPrice1", "SellPrice2", "SellPrice3", "SellPrice4",

            "SellPrice5", "SellPrice6", "SellPrice7", "SellPrice8", "SellPrice9", "SellPrice10", "SellOrderQty1", "SellOrderQty2", "SellOrderQty3", "SellOrderQty4",

            "SellOrderQty5", "SellOrderQty6", "SellOrderQty7", "SellOrderQty8", "SellOrderQty9", "SellOrderQty10", "BuyOrder1", "BuyOrder2", "BuyOrder3", "BuyOrder4",

            "BuyOrder5", "BuyOrder6", "BuyOrder7", "BuyOrder8", "BuyOrder9", "BuyOrder10", "SellOrder1", "SellOrder2", "SellOrder3", "SellOrder4",

            "SellOrder5", "SellOrder6", "SellOrder7", "SellOrder8", "SellOrder9", "SellOrder10", "BuyNumOrders1", "BuyNumOrders2", "BuyNumOrders3", "BuyNumOrders4",

            "BuyNumOrders5", "BuyNumOrders6", "BuyNumOrders7", "BuyNumOrders8", "BuyNumOrders9", "BuyNumOrders10", "SellNumOrders1", "SellNumOrders2", "SellNumOrders3", "SellNumOrders4",

            "SellNumOrders5", "SellNumOrders6", "SellNumOrders7", "SellNumOrders8", "SellNumOrders9", "SellNumOrders10", "TradingPhaseCode", "ReceivedTime",
        };

std::vector<DATA_TYPE> InsightStockTableMeta::COLTYPES = {
            DT_SYMBOL, DT_DATE, DT_TIME, DT_SYMBOL, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_SYMBOL, DT_NANOTIMESTAMP,
        };

std::vector<string> InsightFuturesTableMeta::COLNAMES = {
            "HTSCSecurityID", "MDDate", "MDTime", "securityIDSource", "PreClosePx", "TotalVolumeTrade", "TotalValueTrade", "LastPx", "OpenPx", "HighPx", 
            "LowPx", "PreOpenInterestPx", "PreSettlePrice", "OpenInterest", "BuyPrice1", "BuyPrice2", "BuyPrice3", "BuyPrice4", "BuyPrice5", "BuyOrderQty1", 
            "BuyOrderQty2", "BuyOrderQty3", "BuyOrderQty4", "BuyOrderQty5", "SellPrice1", "SellPrice2", "SellPrice3", "SellPrice4", "SellPrice5", "SellOrderQty1", 
            "SellOrderQty2", "SellOrderQty3", "SellOrderQty4", "SellOrderQty5", "TradingPhaseCode", "ReceivedTime",
        };

std::vector<DATA_TYPE> InsightFuturesTableMeta::COLTYPES = {
            DT_SYMBOL, DT_DATE, DT_TIME, DT_SYMBOL, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG,   DT_LONG, DT_LONG, DT_LONG,   DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG,   DT_LONG, DT_LONG, DT_LONG,   DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
            DT_LONG,   DT_LONG, DT_LONG, DT_LONG,   DT_SYMBOL, DT_NANOTIMESTAMP,
        };

std::vector<string> InsightTransactionTableMeta::COLNAMES = {
            "HTSCSecurityID", "MDDate",      "MDTime",      "securityIDSource", "TradeIndex",
            "TradeBuyNo",     "TradeSellNo", "TradeBSFlag", "TradePrice",       "TradeQty",
            "TradeMoney",     "ApplSeqNum",  "ReceivedTime",
        };
    
std::vector<DATA_TYPE> InsightTransactionTableMeta::COLTYPES = {
            DT_SYMBOL, DT_DATE, DT_TIME, DT_SYMBOL, DT_LONG, 
            DT_LONG,   DT_LONG, DT_INT,  DT_LONG,   DT_LONG,
            DT_LONG,   DT_LONG, DT_NANOTIMESTAMP
        };

std::vector<string> InsightOrderTableMeta::COLNAMES = {
            "HTSCSecurityID", "MDDate",     "MDTime",    "securityIDSource", "securityType",
            "OrderIndex",     "SourceType", "OrderType", "OrderPrice",       "OrderQty",
            "OrderBSFlag",    "BuyNo",      "SellNo",    "ApplSeqNum",       "ChannelNo",
            "ReceivedTime",
        };

std::vector<DATA_TYPE> InsightOrderTableMeta::COLTYPES = {
            DT_SYMBOL, DT_DATE, DT_TIME, DT_SYMBOL, DT_SYMBOL,
            DT_LONG,   DT_INT,  DT_INT,  DT_LONG,   DT_LONG,
            DT_INT,    DT_LONG, DT_LONG, DT_LONG,   DT_INT,
            DT_NANOTIMESTAMP,
        };