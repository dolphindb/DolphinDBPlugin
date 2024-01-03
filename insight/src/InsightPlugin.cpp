#include "InsightPlugin.h"

#include <exception>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "Exceptions.h"
#include "InsightHandle.h"
#include "Logger.h"
#include "ScalarImp.h"
#include "Types.h"
#include "base_define.h"
#include "client_interface.h"
#include "ddbplugin/Plugin.h"
#include "mdc_client_factory.h"
#include "parameter_define.h"
#include <iostream>
#include <thread>

using namespace std;
using namespace com::htsc::mdc::gateway;
using namespace com::htsc::mdc::insight::model;
using namespace com::htsc::mdc::model;

class TcpClient;
typedef SmartPointer<TcpClient> TcpClientSP;

const unordered_map<string, EMarketDataType> MARKET_DATA_TYPE_MAP = {
    {"MD_TICK", MD_TICK},
    {"MD_TRANSACTION", MD_TRANSACTION},
    {"MD_ORDER", MD_ORDER},
};

const unordered_map<string, ESecurityIDSource> SECURITY_ID_SOURCE_MAP = {
    {"XSHE", XSHE}, {"XSHG", XSHG}, {"CCFX", CCFX}, {"CSI", CSI}};

const unordered_map<string, ESecurityType> SECURITY_TYPE_MAP = {
    {"StockType", StockType}, {"FundType", FundType},       {"BondType", BondType},
    {"IndexType", IndexType}, {"FuturesType", FuturesType},
};

static const unordered_set<string> OPTION_SET = {"ReceivedTime", "OutputElapsed"};
static const string INSIGHT_KEY_NAME = "Insight_handle";
static MarketTypeContainer TYPE_CONTAINER;
static bool TYPE_CONTAINER_INIT_FLAG = false;
static dolphindb::BackgroundResourceMap<TcpClient> INSIGHT_HANDLE_MAP(PLUGIN_INSIGHT_PREFIX, "Insight_client");

class TcpClient {
  public:
    TcpClient(Heap *heap, DictionarySP handles, int workPoolThreadCount, const string &ip, int port,
              const string &user_name, const string &password, bool ignoreApplSeq, bool receivedTime,
              bool outputElapsed) {
        LockGuard<Mutex> _(&mutex_);
        try {
            if (login_) {
                throw RuntimeException("handle must be registered before login. ");
            }
            init_env();
            dolphindb::PluginDefer df1([&]() {
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
            if (!clientInterface_) {
                throw RuntimeException("create client failed!");
            }
            SessionSP session = heap->currentSession()->copy();
            session->setUser(heap->currentSession()->getUser());
            insightHandle_ = new InsightHandle(session, handles, ignoreApplSeq, receivedTime, outputElapsed);
            clientInterface_->set_handle_pool_thread_count(workPoolThreadCount);
            clientInterface_->RegistHandle(insightHandle_.get());

            int result = clientInterface_->LoginByServiceDiscovery(ip, port, user_name, password, false);
            if (result != 0) {
                throw RuntimeException(get_error_code_value(result));
            }
            login_ = true;
        } catch (exception &e) {
            throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "failed to registHandleAndLogin: " + e.what());
        }
    }
    ~TcpClient() {
        close();
        LOG_INFO(PLUGIN_INSIGHT_PREFIX, "client has been released. ");
    }
    TableSP getSchema(const string &type) { return insightHandle_->getSchema(type); }
    TableSP getStatus() { return insightHandle_->getStatus(); }

    void logout() {
        LockGuard<Mutex> _(&mutex_);
        unsubscribe();
        if (login_) {
            try {
                ClientFactory::Uninstance();
            } catch (exception &e) {
                LOG_ERR(PLUGIN_INSIGHT_PREFIX + "failed to logout: " + e.what());
            }
            clientInterface_ = nullptr;
            insightHandle_ = nullptr;
            login_ = false;
            subscribed_ = false;
        } else {
            throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "client has been disconnected. ");
        }
    }

    void subscribe(VectorSP marketDataTypes, const string &securityIdSource, const string &securityType) {
        LockGuard<Mutex> _(&mutex_);
        try {
            if (clientInterface_ == nullptr) {
                throw RuntimeException("Please connect first");
            }
            SmartPointer<SubscribeBySourceType> sourceType = new SubscribeBySourceType();
            SubscribeBySourceTypeDetail *detail = sourceType->add_subscribebysourcetypedetail();
            SecuritySourceType *security_source_type = new SecuritySourceType();

            auto idSrc = SECURITY_ID_SOURCE_MAP.find(securityIdSource);
            if (idSrc == SECURITY_ID_SOURCE_MAP.end()) {
                throw RuntimeException("unknown security id source");
            }
            security_source_type->set_securityidsource(idSrc->second);

            auto type = SECURITY_TYPE_MAP.find(securityType);
            if (type == SECURITY_TYPE_MAP.end()) {
                throw RuntimeException("unknown security type");
            }
            security_source_type->set_securitytype(type->second);

            detail->set_allocated_securitysourcetypes(security_source_type);
            for (int i = 0; i < marketDataTypes->size(); ++i) {
                string key = marketDataTypes->get(i)->getString();
                auto mdt = MARKET_DATA_TYPE_MAP.find(key);
                if (mdt == MARKET_DATA_TYPE_MAP.end()) {
                    // HACK special treatment for orderTrade
                    if (key == "MD_ORDER_TRANSACTION") {
                        detail->add_marketdatatypes(MD_TRANSACTION);
                        detail->add_marketdatatypes(MD_ORDER);
                        insightHandle_->handleSubscribeInfo(key, securityIdSource, securityType);
                        continue;
                    }
                    throw RuntimeException("unknown market data type");
                }
                detail->add_marketdatatypes(mdt->second);

                insightHandle_->handleSubscribeInfo(key, securityIdSource, securityType);
            }

            int result = clientInterface_->SubscribeBySourceType(ADD, sourceType.get());
            if (result != 0) {
                throw RuntimeException(get_error_code_value(result));
            }
            sourceTypeVec_.push_back(sourceType);
            subscribed_ = true;
            insightHandle_->tryStartAfterSubscribe();
        } catch (exception &e) {
            throw RuntimeException(PLUGIN_INSIGHT_PREFIX + e.what());
        }
    }

    void unsubscribe() {
        LockGuard<Mutex> _(&mutex_);
        if (subscribed_) {
            if (sourceTypeVec_.empty())
                throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "failed to unsubscribe: source type is null");
            if (clientInterface_ == nullptr)
                throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "failed to unsubscribe: client interface is null");
            ESubscribeActionType actionType = CANCEL;
            try {
                for (auto &sourceType : sourceTypeVec_) {
                    int ret = clientInterface_->SubscribeBySourceType(actionType, sourceType.get());
                    if (ret != 0) {
                        LOG_ERR(PLUGIN_INSIGHT_PREFIX, "unsubscribe failed due to ", get_error_code_value(ret));
                    }
                }
            } catch (exception &e) {
                throw RuntimeException(PLUGIN_INSIGHT_PREFIX + e.what());
            }
        }

        if (!insightHandle_.isNull()) {
            insightHandle_->stopAll();
        }
        subscribed_ = false;
    }

    void close() {
        try {
            LockGuard<Mutex> _(&mutex_);
            logout();
            fini_env();
        } catch (RuntimeException &e) {
            LOG_WARN(e.what());
        } catch (exception &e) {
            LOG_WARN(PLUGIN_INSIGHT_PREFIX + e.what());
        } catch (...) {
            LOG_WARN(PLUGIN_INSIGHT_PREFIX + "error occurs in tcpClient destruction");
        }
    }

    static Mutex TCP_CLIENT_LOCK;

  private:
    Mutex mutex_;
    SmartPointer<InsightHandle> insightHandle_ = nullptr;
    ClientInterface *clientInterface_ = nullptr;
    vector<SmartPointer<SubscribeBySourceType>> sourceTypeVec_;
    bool login_ = false;
    bool subscribed_ = false;
};

Mutex TcpClient::TCP_CLIENT_LOCK;

static void tcpOnClose(Heap *heap, vector<ConstantSP> &arguments) {
    INSIGHT_HANDLE_MAP.safeRemoveWithoutException(arguments[0]);
}
static const unordered_set<string> TYPE_SET{"Transaction", "Order",     "StockTransaction", "StockOrder",
                                            "StockTick",   "IndexTick", "FuturesTick",      "OrderTransaction"};
static void registHandleArgumentValidation(vector<ConstantSP> &arguments, DictionarySP &handles,
                                           int &workPoolThreadCount, const string &usage) {
    if (arguments[0]->getForm() != DF_DICTIONARY)
        throw IllegalArgumentException(__FUNCTION__, usage + "handles must be a dictionary");
    handles = arguments[0];
    VectorSP keys = handles->keys();
    VectorSP values = handles->values();
    for (int i = 0; i < handles->size(); ++i) {
        ConstantSP type = keys->get(i);
        if (type->getType() != DT_STRING)
            throw IllegalArgumentException(__FUNCTION__, usage + "key of handles must be string");
        std::string str = type->getString();
        if (TYPE_SET.find(str) == TYPE_SET.end()) {
            throw IllegalArgumentException(__FUNCTION__,
                                           usage +
                                               "key of handles must be one of 'StockTick', 'IndexTick', "
                                               "'FuturesTick', 'OrderTransaction', 'Transaction' and 'Order'");
        }
    }

    workPoolThreadCount = 5;
    if (arguments.size() > 5 && !arguments[5]->isNull()) {
        if (arguments[5]->getCategory() != INTEGRAL || arguments[5]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, usage + "workPoolThreadCount must be integral");
        }
        workPoolThreadCount = arguments[5]->getInt();
        if (workPoolThreadCount <= 0 || workPoolThreadCount > 32767) {
            throw IllegalArgumentException(__FUNCTION__,
                                           usage + "workPoolThreadCount should more than 0 and less than 32767");
        }
    }
}
void retrieveOption(ConstantSP option, const string &usage, bool &receivedTime, bool &outputElapsed) {
    if (option->getForm() != DF_DICTIONARY) {
        throw IllegalArgumentException("insight::connect", usage + "options must be a dictionary");
    }
    DictionarySP options = option;
    VectorSP keys = options->keys();
    VectorSP values = options->values();
    for (int i = 0; i < options->size(); ++i) {
        ConstantSP key = keys->get(i);
        if (key->getType() != DT_STRING)
            throw IllegalArgumentException("insight::connect", usage + "key type of options must be string");
        string str = key->getString();
        if (OPTION_SET.count(str) == 0)
            throw IllegalArgumentException("insight::connect",
                                           usage + "key of options must be 'ReceivedTime' or 'OutputElapsed'");
    }
    ConstantSP value = options->getMember("ReceivedTime");
    if (!value.isNull() && !value->isNull()) {
        if (value->getType() != DT_BOOL || value->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(
                "insight::connect", usage + "value type of key 'ReceivedTime' in options must be boolean scalar");
        }
        receivedTime = value->getBool();
    }
    value = options->getMember("OutputElapsed");
    if (!value.isNull() && !value->isNull()) {
        if (value->getType() != DT_BOOL || value->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(
                "insight::connect", usage + "value type of key 'OutputElapsed' in options must be boolean scalar");
        }
        outputElapsed = value->getBool();
    }
}

ConstantSP connectInsight(Heap *heap, vector<ConstantSP> &arguments) {
    string usage =
        "insight::connect(handles, ip, port, user, password, [workPoolThreadCount=5], [options], "
        "[ignoreApplSeq=false])";
    LockGuard<Mutex> lock(&TcpClient::TCP_CLIENT_LOCK);

    DictionarySP handles;
    int workPoolThreadCount;
    registHandleArgumentValidation(arguments, handles, workPoolThreadCount, usage);

    if (arguments[1]->getType() != DT_STRING || arguments[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException("insight::connect", usage + "ip must be a string");
    }
    string ip = arguments[1]->getString();

    if (arguments[2]->getCategory() != INTEGRAL || arguments[2]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException("insight::connect", usage + "port must be integral");
    }
    int port = arguments[2]->getInt();

    if (arguments[3]->getType() != DT_STRING || arguments[3]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException("insight::connect", usage + "user must be a string");
    }
    string user = arguments[3]->getString();

    if (arguments[4]->getType() != DT_STRING || arguments[4]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException("insight::connect", usage + "password must be a string");
    }
    string password = arguments[4]->getString();
    bool receivedTime = true;
    bool outputElapsed = false;
    if (arguments.size() > 6 && !arguments[6]->isNull()) {
        retrieveOption(arguments[6], usage, receivedTime, outputElapsed);
    }
    bool ignoreApplSeq = false;
    if (arguments.size() > 7 && !arguments[7]->isNull()) {
        if (arguments[7]->getType() != DT_BOOL || arguments[7]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException("insight::connect", usage + "ignoreApplSeq must be boolean scalar");
        }
        ignoreApplSeq = arguments[7]->getBool();
    }
    if (INSIGHT_HANDLE_MAP.size() == 1) {
        LOG_WARN(PLUGIN_INSIGHT_PREFIX +
                 "insight connect handle already exists, connect function returns the existed handle");
        return INSIGHT_HANDLE_MAP.getHandleByName(INSIGHT_KEY_NAME);
    }

    SmartPointer<TcpClient> client = new TcpClient(heap, handles, workPoolThreadCount, ip, port, user, password,
                                                   ignoreApplSeq, receivedTime, outputElapsed);

    FunctionDefSP onClose(Util::createSystemProcedure("tcpClient onClose()", tcpOnClose, 1, 1));
    ConstantSP ret = Util::createResource(reinterpret_cast<long long>(client.get()), "Insight_client", onClose,
                                          heap->currentSession());
    INSIGHT_HANDLE_MAP.safeAdd(ret, client, INSIGHT_KEY_NAME);
    return ret;
}

void subscribe(Heap *heap, vector<ConstantSP> &arguments) {
    string usage = "insight::subscribe(tcpClient, marketDataTypes, securityIDSource, securityType)";
    LockGuard<Mutex> lock(&TcpClient::TCP_CLIENT_LOCK);
    TcpClientSP client = INSIGHT_HANDLE_MAP.safeGet(arguments[0]);

    if (arguments[1]->getForm() != DF_VECTOR || arguments[1]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "marketDataTypes must be a string vector");
    }
    VectorSP marketDataTypes = arguments[1];

    if (arguments[2]->getType() != DT_STRING || arguments[2]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "securityIdSource must be a string");
    }
    string securityIdSource = arguments[2]->getString();

    if (arguments[3]->getType() != DT_STRING || arguments[3]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "securityType must be a string");
    }
    string securityType = arguments[3]->getString();

    client->subscribe(marketDataTypes, securityIdSource, securityType);
}

void unsubscribe(Heap *heap, vector<ConstantSP> &arguments) {
    LockGuard<Mutex> lock(&TcpClient::TCP_CLIENT_LOCK);
    TcpClientSP tcpClient = INSIGHT_HANDLE_MAP.safeGet(arguments[0]);
    tcpClient->unsubscribe();
}

void closeInsight(Heap *heap, vector<ConstantSP> &arguments) {
    LockGuard<Mutex> lock(&TcpClient::TCP_CLIENT_LOCK);
    INSIGHT_HANDLE_MAP.safeRemove(arguments[0]);
}

ConstantSP getSchema(Heap *heap, vector<ConstantSP> &arguments) {
    string usage = "insight::getSchema(type, [options])";
    if (!TYPE_CONTAINER_INIT_FLAG) {
        initTypeContainer(TYPE_CONTAINER);
        TYPE_CONTAINER_INIT_FLAG = true;
    }
    LockGuard<Mutex> lock(&TcpClient::TCP_CLIENT_LOCK);
    if (arguments[0]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "type must be a string");
    }
    bool receivedTime = true;
    bool outputElapsed = false;
    if (arguments.size() > 1) {
        retrieveOption(arguments[1], usage, receivedTime, outputElapsed);
    }
    string type = arguments[0]->getString();
    if (TYPE_SET.find(type) == TYPE_SET.end()) {
        throw IllegalArgumentException(__FUNCTION__, usage + "unknown schema type " + type);
    }
    string queryType;
    if (type == "StockTransaction" || type == "Transaction") {
        queryType = "Transaction";
    } else if (type == "StockOrder" || type == "order") {
        queryType = "Order";
    } else {
        queryType = type;
    }

    int flag = 0;
    if (receivedTime) {
        flag |= MarketOptionFlag::OPT_RECEIVED;
    }
    if (outputElapsed) {
        flag |= MarketOptionFlag::OPT_ELAPSED;
    }
    TableSP info = TYPE_CONTAINER.getSchema(queryType, flag);
    info->setColumnName(1, "type");
    return info;
}

ConstantSP getStatus(Heap *heap, vector<ConstantSP> &arguments) {
    LockGuard<Mutex> lock(&TcpClient::TCP_CLIENT_LOCK);
    TcpClientSP tcpClient = INSIGHT_HANDLE_MAP.safeGet(arguments[0]);
    return tcpClient->getStatus();
}

ConstantSP getHandle(Heap *heap, vector<ConstantSP> &arguments) {
    LockGuard<Mutex> lock(&TcpClient::TCP_CLIENT_LOCK);
    if (INSIGHT_HANDLE_MAP.size() == 0) {
        throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "Please connect first, there is no connected handle");
    }
    return INSIGHT_HANDLE_MAP.getHandleByName(INSIGHT_KEY_NAME);
}