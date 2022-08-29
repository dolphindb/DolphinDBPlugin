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

ConstantSP tcpClientResource;

class TcpClient {
public:
    static TcpClient* getInstance();

    void init() {
        LockGuard<Mutex> _(&mu_);
        if (login_) {
            throw RuntimeException("There can only be one connection at a time");
        }
        init_env();
        // close_cout_log();
        close_file_log();
        open_response_callback();
        // close_compress();
    }

    void registHandle(Heap* heap, DictionarySP handles, int workPoolThreadCount) {
        LockGuard<Mutex> _(&mu_);
        clientInterface_ = ClientFactory::Instance()->CreateClient(true, "./cert");
        if (!clientInterface_) {
		    ClientFactory::Uninstance();
            clientInterface_ = nullptr;
            throw RuntimeException("create client failed!");
	    }
        SessionSP session = heap->currentSession()->copy(true);
        insightHandle_ = new InsightHandle(session, handles);
        clientInterface_->set_handle_pool_thread_count(workPoolThreadCount);
        clientInterface_->RegistHandle(insightHandle_.get());
    }

    void login(const string &ip, int port, const string &user_name, const string &password) {
        LockGuard<Mutex> _(&mu_);
        assert(clientInterface_ != nullptr);
        int result = clientInterface_->LoginByServiceDiscovery(ip, port, user_name, password, false);
        if(result != 0) {
            ClientFactory::Uninstance();
            clientInterface_ = nullptr;
            insightHandle_ = nullptr;
            throw RuntimeException(get_error_code_value(result));
        }
        login_ = true;
    }

    void logout() {
        LockGuard<Mutex> _(&mu_);
        unsubscribe();
        if (login_) {
            ClientFactory::Uninstance();
            clientInterface_ = nullptr;
            insightHandle_ = nullptr;
        }
        
        tcpClientResource = nullptr;
        login_ = false;
        subscribed_ = false;
    }

    void addSubscribeDetail(VectorSP marketDataTypes, const string& securityIdSource, const string& securityType){
        LockGuard<Mutex> _(&mu_);
        if (clientInterface_ == nullptr || insightHandle_ == nullptr) {
            throw RuntimeException("Please connect first");
        }
        assert(sourceType_!= nullptr);
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
    }

    void subscribe(ESubscribeActionType actionType) {
        LockGuard<Mutex> _(&mu_);
        int result = clientInterface_->SubscribeBySourceType(actionType, sourceType_.get());
        if(result != 0) {
            throw RuntimeException(get_error_code_value(result));
        }
        subscribed_ = true;
    }

    void unsubscribe() {
        LockGuard<Mutex> _(&mu_);
        if (subscribed_) {
            assert(sourceType_.get() != nullptr);
            assert(clientInterface_ != nullptr);

            ESubscribeActionType actionType = CANCEL;
            clientInterface_->SubscribeBySourceType(actionType, sourceType_.get());
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
        fini_env();
    }

    SmartPointer<InsightHandle> insightHandle_ = nullptr;
    unordered_set<Info, InfoHash, InfoEqual> subscribedInfo_;

private:
    Mutex mu_;
    ClientInterface *clientInterface_ = nullptr;
    SmartPointer<SubscribeBySourceType> sourceType_ = nullptr;
    bool login_ = false;
    bool subscribed_ = false;
};

TcpClient* TcpClient::getInstance(){
    static TcpClient tcpClient;
    return &tcpClient;
}

static void tcpOnClose(Heap *heap, vector<ConstantSP> &arguments){
    // TcpClient *tcpClient = reinterpret_cast<TcpClient*>(arguments[0]->getLong());
    // if(tcpClient != nullptr) {
    //     tcpClient->close();
    // }
}

static void registHandleArgumentValidation(vector<ConstantSP> &arguments, DictionarySP &handles, int& workPoolThreadCount) {
    if(arguments[0]->getForm() != DF_DICTIONARY)
        throw IllegalArgumentException(__FUNCTION__, "handles must be a dictionary");
    handles = arguments[0];
    {
        VectorSP keys = handles->keys();
        VectorSP values = handles->values();
        for(int i = 0; i < handles->size(); ++i){
            ConstantSP type = keys->get(i);
            if(type->getType() != DT_STRING)
                throw IllegalArgumentException(__FUNCTION__, "key of handles must be string");
            std::string str = type->getString();
            if(str != "StockTransaction" && str != "StockOrder" && str != "StockTick" && str != "IndexTick" && str != "FuturesTick")
                throw IllegalArgumentException(__FUNCTION__, "key of handles must be one of 'StockTick', 'IndexTick', 'FuturesTick', 'StockTransaction' and 'StockOrder'");
        }
    }

    workPoolThreadCount = 5;
    if(arguments.size() > 5) {
        if(arguments[5]->getCategory() != INTEGRAL) {
            throw IllegalArgumentException(__FUNCTION__, "workPoolThreadCount must be integral");
        }
        workPoolThreadCount = arguments[5]->getInt();
        if(workPoolThreadCount <= 0 || workPoolThreadCount > 32767) {
            throw IllegalArgumentException(__FUNCTION__, "workPoolThreadCount should more than 0 and less than 32767");
        }
    }
}

ConstantSP connectInsight(Heap *heap, vector<ConstantSP> &arguments) {
    if (!tcpClientResource.isNull() && !tcpClientResource->isNull()) {
        return tcpClientResource;
    }

    DictionarySP handles;
    int workPoolThreadCount;
    registHandleArgumentValidation(arguments, handles, workPoolThreadCount);
    TcpClient *tcpClient = TcpClient::getInstance();
    tcpClient->init();
    tcpClient->registHandle(heap, handles, workPoolThreadCount);

    if(arguments[1]->getType() != DT_STRING){
        throw IllegalArgumentException(__FUNCTION__, "ip must be a string");
    }
    string ip = arguments[1]->getString();

    if(arguments[2]->getCategory() != INTEGRAL){
        throw IllegalArgumentException(__FUNCTION__, "port must be integral");
    }
    int port = arguments[2]->getInt();

    if(arguments[3]->getType() != DT_STRING){
        throw IllegalArgumentException(__FUNCTION__, "user must be a string");
    }
    string user = arguments[3]->getString();

    if(arguments[4]->getType() != DT_STRING){
        throw IllegalArgumentException(__FUNCTION__, "password must be a string");
    }
    string password = arguments[4]->getString();

    tcpClient->login(ip, port, user, password);

    FunctionDefSP onClose(Util::createSystemProcedure("tcpClient onClose()", tcpOnClose, 1, 1));
    tcpClientResource = Util::createResource(reinterpret_cast<long long>(tcpClient), "client", onClose, heap->currentSession());
    return tcpClientResource;
}

ConstantSP subscribe(Heap *heap, vector<ConstantSP> &arguments) {
    if(arguments[0]->getType() != DT_RESOURCE || arguments[0]->getString() != "client")
        throw IllegalArgumentException(__FUNCTION__, "client must be a client handle");
    TcpClient *client = reinterpret_cast<TcpClient*>(arguments[0]->getLong());
    if(client == nullptr)
        throw IllegalArgumentException(__FUNCTION__, "Please create client handle first");

    if(arguments[1]->getForm() != DF_VECTOR) {
        throw IllegalArgumentException(__FUNCTION__, "marketDataTypes must be a string vector");
    }
    VectorSP marketDataTypes = arguments[1];

    if(arguments[2]->getType() != DT_STRING){
        throw IllegalArgumentException(__FUNCTION__, "securityIdSource must be a string");
    }
    string securityIdSource = arguments[2]->getString();

    if(arguments[3]->getType() != DT_STRING){
        throw IllegalArgumentException(__FUNCTION__, "securityType must be a string");
    }
    string securityType = arguments[3]->getString();

    client->addSubscribeDetail(marketDataTypes, securityIdSource, securityType);

    ESubscribeActionType actionType = ADD;
    client->subscribe(actionType);

    return new Void();
}

ConstantSP unsubscribe(Heap *heap, vector<ConstantSP> &arguments){
    if(arguments[0]->getType() != DT_RESOURCE || arguments[0]->getString() != "client")
        throw IllegalArgumentException(__FUNCTION__, "tcpClient must be a TcpClient handle");
    TcpClient *tcpClient = reinterpret_cast<TcpClient*>(arguments[0]->getLong());

    tcpClient->unsubscribe();

    return new Void();
}

ConstantSP closeInsight(Heap *heap, vector<ConstantSP> &arguments){
    if(arguments[0]->getType() != DT_RESOURCE || arguments[0]->getString() != "client")
        throw IllegalArgumentException(__FUNCTION__, "tcpClient must be a TcpClient handle");
    TcpClient *tcpClient = reinterpret_cast<TcpClient*>(arguments[0]->getLong());

    tcpClient->close();

    return new Void();
}

ConstantSP getSchema(Heap *heap, vector<ConstantSP> &arguments) {
    if(arguments[0]->getType() != DT_STRING){
        throw IllegalArgumentException(__FUNCTION__, "first argument must be a string");
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
        throw IllegalArgumentException(__FUNCTION__, "unknown schema type");
    }
}

ConstantSP getStatus(Heap *heap, vector<ConstantSP> &arguments) {
    if(arguments[0]->getType() != DT_RESOURCE || arguments[0]->getString() != "client")
        throw IllegalArgumentException(__FUNCTION__, "tcpClient must be a TcpClient handle");
    TcpClient *tcpClient = reinterpret_cast<TcpClient*>(arguments[0]->getLong());

    auto subscribedInfo = tcpClient->subscribedInfo_;
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