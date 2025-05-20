#include "InsightPlugin.h"

#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include "ddbplugin/CommonInterface.h"
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
#include "ddbplugin/PluginLogger.h"

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
    {"MD_SECURITY_LENDING", MD_SECURITY_LENDING}
};

const unordered_map<string, ESecurityIDSource> SECURITY_ID_SOURCE_MAP = {
    {"XSHE", XSHE}, {"XSHG", XSHG}, {"CCFX", CCFX}, {"CSI", CSI},
    {"XBSE", XBSE}, {"XDCE", XDCE}, {"XSGE", XSGE}, {"NEEQ", NEEQ},
    {"XZCE", XZCE}, {"HKSC", HKSC}, {"HGHQ", HGHQ}, {"CNI", CNI},
    {"HTSM", HTSM}, {"HTIS", HTIS}
    };

const unordered_map<string, ESecurityType> SECURITY_TYPE_MAP = {
    {"StockType", StockType}, {"FundType", FundType},       {"BondType", BondType},
    {"IndexType", IndexType}, {"FuturesType", FuturesType}, {"OptionType", OptionType},
};

static const unordered_set<string> TYPE_SET{"Transaction", "Order",       "StockTransaction", "StockOrder",
                                            "FundTick",    "BondTick",    "OptionTick",       "StockTick",
                                            "IndexTick",   "FuturesTick", "OrderTransaction", "SecurityLending"};

static const unordered_set<string> OPTION_SET = {"ReceivedTime", "OutputElapsed"};
static const unordered_set<string> DATA_VERSION_SET = {"3.2.8", "3.2.11"};

static const string INSIGHT_KEY_NAME = "Insight_handle";
static MarketTypeContainer<> TYPE_CONTAINER;
static dolphindb::BackgroundResourceMap<TcpClient> INSIGHT_HANDLE_MAP(PLUGIN_INSIGHT_PREFIX, "Insight_client");

class TcpClient {
  public:
    TcpClient(Heap *heap, DictionarySP handles, int workPoolThreadCount, const string &ip, int port,
              const string &user_name, const string &password, int seqCheckMode, bool receivedTime,
              bool outputElapsed, const string &certDirPath, const string &dataVersion, const vector<string> &backupList) {
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
            clientInterface_ = ClientFactory::Instance()->CreateClient(true, certDirPath.c_str());
            if (!clientInterface_) {
                throw RuntimeException("create client failed!");
            }
            SessionSP session = heap->currentSession()->copy();
            session->setUser(heap->currentSession()->getUser());
            insightHandle_ = new InsightHandle(session, handles, seqCheckMode, receivedTime, outputElapsed, dataVersion);
            clientInterface_->set_handle_pool_thread_count(workPoolThreadCount);
            clientInterface_->RegistHandle(insightHandle_.get());

            int result;
            if (backupList.empty()) {
                result = clientInterface_->LoginByServiceDiscovery(ip, port, user_name, password, false);
            } else {
                result = clientInterface_->LoginByServiceDiscovery(ip, port, user_name, password, false, backupList);
            }
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
        PLUGIN_LOG_INFO(PLUGIN_INSIGHT_PREFIX, "client has been released. ");
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
                PLUGIN_LOG_ERR(PLUGIN_INSIGHT_PREFIX + "failed to logout: " + e.what());
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
                        PLUGIN_LOG_ERR(PLUGIN_INSIGHT_PREFIX, "unsubscribe failed due to ", get_error_code_value(ret));
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
            PLUGIN_LOG_WARN(e.what());
        } catch (exception &e) {
            PLUGIN_LOG_WARN(PLUGIN_INSIGHT_PREFIX + e.what());
        } catch (...) {
            PLUGIN_LOG_WARN(PLUGIN_INSIGHT_PREFIX + "error occurs in tcpClient destruction");
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
static void registHandleArgumentValidation(vector<ConstantSP> &arguments, DictionarySP &handles,
                                           int &workPoolThreadCount, const string &usage) {
    if (arguments[0]->getForm() != DF_DICTIONARY)
        throw IllegalArgumentException(__FUNCTION__, usage + "outputDict must be a dictionary");
    handles = arguments[0];
    VectorSP keys = handles->keys();
    VectorSP values = handles->values();
    for (int i = 0; i < handles->size(); ++i) {
        ConstantSP type = keys->get(i);
        if (type->getType() != DT_STRING)
            throw IllegalArgumentException(__FUNCTION__, usage + "key of outputDict must be string");
        std::string str = type->getString();
        if (TYPE_SET.find(str) == TYPE_SET.end()) {
            throw IllegalArgumentException(__FUNCTION__,
                                           usage +
                                               "key of outputDict must be one of 'StockTick', 'IndexTick', "
                                               "'FundTick', 'BondTick', 'OptionTick', "
                                               "'FuturesTick', 'OrderTransaction', 'Transaction' and 'Order'");
        }
    }

    workPoolThreadCount = 5;
    if (arguments.size() > 5 && !arguments[5]->isNull()) {
        if (arguments[5]->getCategory() != INTEGRAL || arguments[5]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, usage + "workerNum must be an integer scalar");
        }
        workPoolThreadCount = arguments[5]->getInt();
        if (workPoolThreadCount <= 0 || workPoolThreadCount > 32767) {
            throw IllegalArgumentException(__FUNCTION__,
                                           usage + "workerNum should be more than 0 and less than 32767");
        }
    }
}
void retrieveOption(ConstantSP option, const string &usage, bool &receivedTime, bool &outputElapsed) {
    if (option->getForm() != DF_DICTIONARY) {
        throw IllegalArgumentException("insight::connect", usage + "option must be a dictionary");
    }
    DictionarySP options = option;
    VectorSP keys = options->keys();
    VectorSP values = options->values();
    for (int i = 0; i < options->size(); ++i) {
        ConstantSP key = keys->get(i);
        if (key->getType() != DT_STRING)
            throw IllegalArgumentException("insight::connect", usage + "key type of option must be string");
        string str = key->getString();
        if (OPTION_SET.count(str) == 0)
            throw IllegalArgumentException("insight::connect",
                                           usage + "key of option must be 'ReceivedTime' or 'OutputElapsed'");
    }
    ConstantSP value = options->getMember("ReceivedTime");
    if (!value->isNull()) {
        if (value->getType() != DT_BOOL || value->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(
                "insight::connect", usage + "value type of key 'ReceivedTime' in option must be boolean scalar");
        }
        receivedTime = value->getBool();
    }
    value = options->getMember("OutputElapsed");
    if (!value->isNull()) {
        if (value->getType() != DT_BOOL || value->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(
                "insight::connect", usage + "value type of key 'OutputElapsed' in option must be boolean scalar");
        }
        outputElapsed = value->getBool();
    }
}

ConstantSP connectInsight(Heap *heap, vector<ConstantSP> &arguments) {
    string usage =
        "insight::connect(outputDict, host, port, username, password, [workerNum=5], [option], "
        "[seqCheckMode=1], [certDirPath], [dataVersion='3.2.8'], [backupList]) ";
    LockGuard<Mutex> lock(&TcpClient::TCP_CLIENT_LOCK);

    DictionarySP handles;
    int workPoolThreadCount;
    registHandleArgumentValidation(arguments, handles, workPoolThreadCount, usage);

    if (arguments[1]->getType() != DT_STRING || arguments[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException("insight::connect", usage + "host must be a string");
    }
    string ip = arguments[1]->getString();

    if (arguments[2]->getCategory() != INTEGRAL || arguments[2]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException("insight::connect", usage + "port must be integral");
    }
    int port = arguments[2]->getInt();

    if (arguments[3]->getType() != DT_STRING || arguments[3]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException("insight::connect", usage + "username must be a string");
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
    int seqCheckMode = 1;
    if (arguments.size() > 7 && !arguments[7]->isNull()) {
        if (arguments[7]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException("insight::connect", usage + "seqCheckMode must be int or string scalar");
        }
        if (arguments[7]->getType() == DT_BOOL || arguments[7]->getCategory() == INTEGRAL) {
            seqCheckMode = arguments[7]->getLong();
            if (seqCheckMode < 0 || seqCheckMode > 2) {
                throw IllegalArgumentException("insight::connect", usage + "if seqCheckMode is int scalar, it must be 0, 1 or 2");
            }
        } else if (arguments[7]->getType() == DT_STRING) {
            string modeStr = arguments[7]->getString();
            if (modeStr == "check") {
                seqCheckMode = 0;
            } else if (modeStr == "ignoreWithLog") {
                seqCheckMode = 1;
            } else if (modeStr == "ignore") {
                seqCheckMode = 2;
            } else {
                throw IllegalArgumentException("insight::connect", usage + "if seqCheckMode is string scalar, it must be 'check', 'ignoreWithLog' or 'ignore'");
            }
        } else {
            throw IllegalArgumentException("insight::connect", usage + "seqCheckMode must be int or string scalar");
        }
    }
    string certDirPath;
    if (arguments.size() > 8 && !arguments[8]->isNull()) {
        if (arguments[8]->getType() != DT_STRING || arguments[8]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException("insight::connect", usage + "certDirPath must be string scalar");
        }
        certDirPath = arguments[8]->getString();
    }
    if (certDirPath == "") {
        string certInServer = Util::EXEC_DIR + "/cert";
        vector<ConstantSP> getConfigArgs{new String("pluginDir")};
        string pluginDir = heap->currentSession()->getFunctionDef("getConfig")->call(heap, getConfigArgs)->getString();

        if(!Util::isAbosultePath(pluginDir)){
            if(Util::existsDir(Util::HOME_DIR + "/" + pluginDir))
                pluginDir = Util::HOME_DIR + "/" + pluginDir;
            else if(Util::existsDir(Util::WORKING_DIR + "/" + pluginDir))
                pluginDir = Util::WORKING_DIR + "/" + pluginDir;
            else if(Util::existsDir(Util::EXEC_DIR + "/" + pluginDir))
                pluginDir = Util::EXEC_DIR + "/" + pluginDir;
            else
                pluginDir = Util::HOME_DIR + "/" + pluginDir;
        }

        string certInPlugins = pluginDir + "/insight/cert";
        if (Util::existsDir(certInServer)) {
            certDirPath = certInServer;
        } else if (Util::existsDir(certInPlugins)) {
            certDirPath = certInPlugins;
        } else {
            throw IllegalArgumentException("insight::connect",
                                           usage + "insight cert directory not found, certDirPath is required.");
        }
    } else {
        if (!Util::existsDir(certDirPath)) {
            throw IllegalArgumentException("insight::connect",
                                           usage + "certDirPath \"" + certDirPath + "\" doesn't exist.");
        }

        string errMsg;
        vector<FileAttributes> files;
        if (Util::getDirectoryContent(certDirPath, files, errMsg)) {
            bool noPerm = true;
            for (auto &file: files) {
                if (file.name.find(".pem") != std::string::npos) {
                    noPerm = false;
                }
            }
            if (noPerm) {
                throw IllegalArgumentException("insight::connect",
                                               usage + "cannot find .pem file in [" + certDirPath + "]");
            }
        } else {
            throw IllegalArgumentException("insight::connect", usage + errMsg);
        }
    }
    string dataVersion = "3.2.8";
    if (arguments.size() > 9 && !arguments[9]->isNull()) {
        if(arguments[9]->getType() != DT_STRING || arguments[9]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException("insight::getSchema", usage + "dataVersion must be a string scalar.");
        }
        dataVersion = arguments[9]->getString();
        if (DATA_VERSION_SET.find(dataVersion) == DATA_VERSION_SET.end()) {
            throw IllegalArgumentException("insight::connect", usage + "dataVersion must be '3.2.8' or '3.2.11'.");
        }
    }
    vector<string> backupList;
    if (arguments.size() > 10 && !arguments[10]->isNull()) {
        if(arguments[10]->getType() != DT_STRING || arguments[10]->getForm() != DF_VECTOR) {
            throw IllegalArgumentException("insight::getSchema", usage + "backupList must be a string vector.");
        }
        for (int i = 0; i < arguments[10]->size(); i++) {
            backupList.emplace_back(arguments[10]->get(i)->getString());
        }
    }

    if (INSIGHT_HANDLE_MAP.size() == 1) {
        PLUGIN_LOG_WARN(PLUGIN_INSIGHT_PREFIX +
                 "insight connect handle already exists, connect function returns the existed handle");
        return INSIGHT_HANDLE_MAP.getHandleByName(INSIGHT_KEY_NAME);
    }

    SmartPointer<TcpClient> client = new TcpClient(heap, handles, workPoolThreadCount, ip, port, user, password,
                                                   seqCheckMode, receivedTime, outputElapsed, certDirPath, dataVersion, backupList);

    FunctionDefSP onClose(Util::createSystemProcedure("tcpClient onClose()", tcpOnClose, 1, 1));
    ConstantSP ret = Util::createResource(reinterpret_cast<long long>(client.get()), "Insight_client", onClose,
                                          heap->currentSession());
    INSIGHT_HANDLE_MAP.safeAdd(ret, client, INSIGHT_KEY_NAME);
    return ret;
}

void subscribe(Heap *heap, vector<ConstantSP> &arguments) {
    string usage = "insight::subscribe(handle, marketDataTypes, securityIDSource, securityType) ";
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
    string usage = "insight::getSchema(dataType, [option], [dataVersion='3.2.8']) ";
    LockGuard<Mutex> lock(&TcpClient::TCP_CLIENT_LOCK);
    if (arguments[0]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "dataType must be a string scalar");
    }
    bool receivedTime = true;
    bool outputElapsed = false;
    if (arguments.size() > 1 && !arguments[1]->isNull()) {
        retrieveOption(arguments[1], usage, receivedTime, outputElapsed);
    }
    string type = arguments[0]->getString();
    if (TYPE_SET.find(type) == TYPE_SET.end()) {
        throw IllegalArgumentException(__FUNCTION__, usage + "unknown schema type " + type);
    }
    string dataVersion = "3.2.8";
    if (arguments.size() > 2 && !arguments[2]->isNull()) {
        if(arguments[2]->getType() != DT_STRING || arguments[2]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException("insight::getSchema", usage + "dataVersion must be a string scalar.");
        }
        dataVersion = arguments[2]->getString();
        if (DATA_VERSION_SET.find(dataVersion) == DATA_VERSION_SET.end()) {
            throw IllegalArgumentException("insight::connect", usage + "dataVersion must be '3.2.8' or '3.2.11'.");
        }
    }
    initTypeContainer(TYPE_CONTAINER, dataVersion);
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