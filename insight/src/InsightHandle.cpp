#include "InsightHandle.h"

#include <iostream>
#include <string>

#include "Exceptions.h"
#include "Logger.h"
#include "Types.h"
#include "ddbplugin/PluginLogger.h"
#include "ddbplugin/PluginLoggerImp.h"

using namespace std;

const static int TIMEOUT = 100;
const static int CAPACITY = 100000;

InsightHandle::InsightHandle(SessionSP session, DictionarySP handles, int seqCheckMode, bool receivedTime,
                             bool outputElapsed, const string &dataVersion)
    : receivedTime_(receivedTime), outputElapsed_(outputElapsed), dataVersion_(dataVersion), session_(session) {
    initTypeContainer(insightTypeContainer_, dataVersion_);

    int flag = 0;
    if (receivedTime_) {
        flag |= MarketOptionFlag::OPT_RECEIVED;
    }
    if (outputElapsed_) {
        flag |= MarketOptionFlag::OPT_ELAPSED;
    }

    // Compatible with previous data type name
    ConstantSP realHandleMember = handles->getMember("Order");
    ConstantSP handleMember = handles->getMember("StockOrder");
    if (realHandleMember->isNull() && !handleMember->isNull()) {
        handles->set("Order", handleMember);
    }
    realHandleMember = handles->getMember("Transaction");
    handleMember = handles->getMember("StockTransaction");
    if (realHandleMember->isNull() && !handleMember->isNull()) {
        handles->set("Transaction", handleMember);
    }

#define NEW_THREADED_QUEUE(type, str, func)                                                                            \
    new ThreadedQueue<TimedWrapper<type>>(session->getHeap().get(), TIMEOUT, CAPACITY, insightTypeContainer_.get(str), \
                                          nullptr, flag, str, PLUGIN_INSIGHT_PREFIX, Util::BUF_SIZE, func)

    if (dataVersion == "3.2.8") {
        orderUtil_.queue_ = NEW_THREADED_QUEUE(MDOrder, "Order", orderReader);
    } else {  // could only be "3.2.11"
        orderUtil_.queue_ = NEW_THREADED_QUEUE(MDOrder, "Order", orderReader_3_2_11);
    }
    transactionUtil_.queue_ = NEW_THREADED_QUEUE(MDTransaction, "Transaction", transactionReader);
    indexUtil_.queue_ = NEW_THREADED_QUEUE(MDIndex, "IndexTick", indexTickReader);
    futureUtil_.queue_ = NEW_THREADED_QUEUE(MDFuture, "FuturesTick", futureTickReader);
    stockUtil_.queue_ = NEW_THREADED_QUEUE(MDStock, "StockTick", stockTickReader);
    fundUtil_.queue_ = NEW_THREADED_QUEUE(MDFund, "FundTick", fundTickReader);
    bondUtil_.queue_ = NEW_THREADED_QUEUE(MDBond, "BondTick", bondTickReader);
    optionUtil_.queue_ = NEW_THREADED_QUEUE(MDOption, "OptionTick", optionTickReader);
    securityLendingUtil_.queue_ = NEW_THREADED_QUEUE(MDSecurityLending, "SecurityLending", securityLendingTickReader);

#undef NEW_THREADED_QUEUE

    orderTransactionQueue_ = new ThreadedQueue<InsightOrderTransaction>(
        session->getHeap().get(), TIMEOUT, CAPACITY, insightTypeContainer_.get("OrderTransaction"), nullptr, flag,
        "OrderTransaction", PLUGIN_INSIGHT_PREFIX, Util::BUF_SIZE, orderTransactionReaderFunction(seqCheckMode));

    string syntax =
        "insight::connect(handles, ip, port, user, password, [workPoolThreadCount=5], [options], "
        "[seqCheckMode=1], [certDirPath], [dataVersion='3.2.8'], [backupList]) ";

    setParam<MDStock>(handles, stockUtil_.handle_, stockUtil_.queue_, "StockTick", syntax);
    setParam<MDFund>(handles, fundUtil_.handle_, fundUtil_.queue_, "FundTick", syntax);
    setParam<MDBond>(handles, bondUtil_.handle_, bondUtil_.queue_, "BondTick", syntax);
    setParam<MDIndex>(handles, indexUtil_.handle_, indexUtil_.queue_, "IndexTick", syntax);
    setParam<MDFuture>(handles, futureUtil_.handle_, futureUtil_.queue_, "FuturesTick", syntax);
    setParam<MDOption>(handles, optionUtil_.handle_, optionUtil_.queue_, "OptionTick", syntax);
    setParam<MDOrder>(handles, orderUtil_.handle_, orderUtil_.queue_, "Order", syntax);
    setParam<MDTransaction>(handles, transactionUtil_.handle_, transactionUtil_.queue_, "Transaction", syntax);
    setParam<MDSecurityLending>(handles, securityLendingUtil_.handle_, securityLendingUtil_.queue_, "SecurityLending",
                                syntax);

    ConstantSP handleOrderTransaction = handles->getMember("OrderTransaction");
    if (!handleOrderTransaction.isNull() && !handleOrderTransaction->isNull()) {
        if (handleOrderTransaction->getForm() == DF_TABLE) {
            if (!((TableSP)handleOrderTransaction)->isSharedTable() ||
                ((TableSP)handleOrderTransaction)->getTableType() != REALTIMETBL) {
                throw IllegalArgumentException(
                    "insight::connect", syntax + " value of 'OrderTransaction' must be stream table or dictionary");
            }
            if (handleOrderTransaction->columns() != getSchema("OrderTransaction")->rows()) {
                throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "OrderTransaction tables expects columns " +
                                       std::to_string(getSchema("OrderTransaction")->rows()) + ", actual " +
                                       std::to_string(handleOrderTransaction->columns()));
            }
            orderTransactionQueue_->setTable(handleOrderTransaction);
            handleOrderTransaction_ = handleOrderTransaction;
        } else if (handleOrderTransaction->getForm() == DF_DICTIONARY) {
            DictionarySP dict = ((DictionarySP)handleOrderTransaction);
            ConstantSP keys = dict->keys();
            for (int i = 0; i < keys->size(); ++i) {
                ConstantSP key = keys->get(i);
                ConstantSP value = dict->getMember(key);
                if (key->getCategory() != INTEGRAL || key->getForm() != DF_SCALAR) {
                    throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "the channel number must be integral scalar");
                }
                if (value->getForm() != DF_TABLE || ((TableSP)value)->getTableType() != REALTIMETBL ||
                    !((TableSP)value)->isSharedTable()) {
                    throw RuntimeException(PLUGIN_INSIGHT_PREFIX +
                                           "the table handles insight data must be shared stream table");
                }
                if (value->columns() != getSchema("OrderTransaction")->rows()) {
                    throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "OrderTransaction tables expects columns " +
                                        std::to_string(getSchema("OrderTransaction")->rows()) + ", actual " +
                                        std::to_string(value->columns()));
                }
                SmartPointer<ThreadedQueue<InsightOrderTransaction>> queue = new ThreadedQueue<InsightOrderTransaction>(
                    session->getHeap().get(), TIMEOUT, CAPACITY, insightTypeContainer_.get("OrderTransaction"), nullptr,
                    flag, "OrderTransaction_" + std::to_string(key->getInt()), PLUGIN_INSIGHT_PREFIX, Util::BUF_SIZE,
                    orderTransactionReaderFunction(seqCheckMode));
                queue->setTable(value);
                orderTransactionQueueMap_[key->getInt()] = queue;
            }
        } else {
            throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "handle's values should be tables");
        }
    }
}

InsightHandle::~InsightHandle() {
    orderTransactionMarket_.clear();
    orderTransactionQueue_ = nullptr;
    orderTransactionQueueMap_.clear();
    handleOrderTransaction_ = nullptr;

    orderUtil_.clear();
    transactionUtil_.clear();
    indexUtil_.clear();
    futureUtil_.clear();
    stockUtil_.clear();
    fundUtil_.clear();
    bondUtil_.clear();
    optionUtil_.clear();
    securityLendingUtil_.clear();
}

void InsightHandle::tryStartAfterSubscribe() {
    orderUtil_.startQueue();
    transactionUtil_.startQueue();
    indexUtil_.startQueue();
    fundUtil_.startQueue();
    futureUtil_.startQueue();
    bondUtil_.startQueue();
    stockUtil_.startQueue();
    optionUtil_.startQueue();
    securityLendingUtil_.startQueue();

    if (!orderTransactionMarket_.empty() && !(handleOrderTransaction_.isNull() || handleOrderTransaction_->isNull()) &&
        !orderTransactionQueue_.isNull() && !orderTransactionQueue_->isStarted()) {
        orderTransactionQueue_->start();
    }
    if (!orderTransactionMarket_.empty()) {
        for (auto &pair : orderTransactionQueueMap_) {
            if (!pair.second->isStarted()) {
                pair.second->start();
            }
        }
    }
}

void InsightHandle::stopAll() {
    orderUtil_.stopQueue();
    transactionUtil_.stopQueue();
    indexUtil_.stopQueue();
    fundUtil_.stopQueue();
    futureUtil_.stopQueue();
    bondUtil_.stopQueue();
    stockUtil_.stopQueue();
    optionUtil_.stopQueue();
    securityLendingUtil_.stopQueue();

    if (!orderTransactionQueue_.isNull()) {
        orderTransactionQueue_->stop();
    }
    for (auto &pair : orderTransactionQueueMap_) {
        pair.second->stop();
    }
}

void InsightHandle::OnMarketData(const com::htsc::mdc::insight::model::MarketData &record) {
    try {
        long long receiveTime = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
        PLUGIN_LOG(PLUGIN_INSIGHT_PREFIX, "<OnMarketData> receive ", EMarketDataType_Name(record.marketdatatype()),
            " data, timestamp: ", receiveTime, ", transaction: ", record.has_mdtransaction(),
            ", order: ", record.has_mdorder(), ", stock: ", record.has_mdstock(), ", index: ", record.has_mdindex(),
            ", future: ", record.has_mdfuture(), ", fund: ", record.has_mdfund(), ", bond: ", record.has_mdbond(),
            ", option: ", record.has_mdoption());
        switch (record.marketdatatype()) {
            case MD_TICK: {
                if (record.has_mdstock() && LIKELY(!stockUtil_.queue_.isNull() && stockUtil_.queue_->isStarted())) {
                    stockUtil_.queue_->push(TimedWrapper<MDStock>{record.mdstock(), receiveTime});
                } else if (record.has_mdindex() &&
                           LIKELY(!indexUtil_.queue_.isNull() && indexUtil_.queue_->isStarted())) {
                    indexUtil_.queue_->push(TimedWrapper<MDIndex>{record.mdindex(), receiveTime});
                } else if (record.has_mdfuture() &&
                           LIKELY(!futureUtil_.queue_.isNull() && futureUtil_.queue_->isStarted())) {
                    futureUtil_.queue_->push(TimedWrapper<MDFuture>{record.mdfuture(), receiveTime});
                } else if (record.has_mdfund() && LIKELY(!fundUtil_.queue_.isNull() && fundUtil_.queue_->isStarted())) {
                    fundUtil_.queue_->push(TimedWrapper<MDFund>{record.mdfund(), receiveTime});
                } else if (record.has_mdbond() && LIKELY(!bondUtil_.queue_.isNull() && bondUtil_.queue_->isStarted())) {
                    bondUtil_.queue_->push(TimedWrapper<MDBond>{record.mdbond(), receiveTime});
                } else if (record.has_mdoption() &&
                           LIKELY(!optionUtil_.queue_.isNull() && optionUtil_.queue_->isStarted())) {
                    optionUtil_.queue_->push(TimedWrapper<MDOption>{record.mdoption(), receiveTime});
                }
                break;
            }
            case MD_TRANSACTION: {
                if (record.has_mdtransaction()) {
                    int channelno = record.mdtransaction().channelno();
                    if (LIKELY(!transactionUtil_.queue_.isNull() && transactionUtil_.queue_->isStarted())) {
                        transactionUtil_.queue_->push(TimedWrapper<MDTransaction>{record.mdtransaction(), receiveTime});
                    }
                    if (LIKELY(!orderTransactionQueue_.isNull() && orderTransactionQueue_->isStarted())) {
                        orderTransactionQueue_->push(InsightOrderTransaction{.orderOrTransaction = false,
                                                                             .reachTime = receiveTime,
                                                                             .order = MDOrder(),
                                                                             .transaction = record.mdtransaction()});
                    }
                    if (orderTransactionQueueMap_.find(channelno) != orderTransactionQueueMap_.end()) {
                        if (LIKELY(orderTransactionQueueMap_[channelno]->isStarted())) {
                            orderTransactionQueueMap_[channelno]->push(
                                InsightOrderTransaction{.orderOrTransaction = false,
                                                        .reachTime = receiveTime,
                                                        .order = MDOrder(),
                                                        .transaction = record.mdtransaction()});
                        }
                    }
                }
                break;
            }

            case MD_ORDER: {
                if (record.has_mdorder()) {
                    int channelno = record.mdorder().channelno();
                    if (LIKELY(!orderUtil_.queue_.isNull() && orderUtil_.queue_->isStarted())) {
                        orderUtil_.queue_->push(TimedWrapper<MDOrder>{record.mdorder(), receiveTime});
                    }
                    if (LIKELY(!orderTransactionQueue_.isNull() && orderTransactionQueue_->isStarted())) {
                        orderTransactionQueue_->push(InsightOrderTransaction{
                            .orderOrTransaction = true, .reachTime = receiveTime, .order = record.mdorder()});
                    }
                    if (orderTransactionQueueMap_.find(channelno) != orderTransactionQueueMap_.end()) {
                        if (LIKELY(orderTransactionQueueMap_[channelno]->isStarted())) {
                            orderTransactionQueueMap_[channelno]->push(InsightOrderTransaction{
                                .orderOrTransaction = true, .reachTime = receiveTime, .order = record.mdorder()});
                        }
                    }
                }
                break;
            }

            case MD_SECURITY_LENDING: {
                if (record.has_mdsecuritylending()) {
                    if (LIKELY(!securityLendingUtil_.queue_.isNull() && securityLendingUtil_.queue_->isStarted())) {
                        securityLendingUtil_.queue_->push(
                            TimedWrapper<MDSecurityLending>{record.mdsecuritylending(), receiveTime});
                    }
                }
                break;
            }
            default: {
                throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "unsupported data type " +
                                       std::to_string(record.marketdatatype()));
            }
        }
    } catch (exception &e) {
        PLUGIN_LOG_ERR(PLUGIN_INSIGHT_PREFIX + "Failed to received market data: " + string(e.what()));
    } catch (...) {
        PLUGIN_LOG_ERR(PLUGIN_INSIGHT_PREFIX + "Failed to received market data.");
    }
}

void InsightHandle::OnLoginSuccess() { PLUGIN_LOG_INFO(PLUGIN_INSIGHT_PREFIX, "Login Success"); }

void InsightHandle::OnLoginFailed(int errorNo, const string &message) {
    PLUGIN_LOG_ERR(PLUGIN_INSIGHT_PREFIX, "Login Failed. Error number: ", errorNo, ". Error message: ", message);
}

void InsightHandle::OnNoConnections() { PLUGIN_LOG_ERR(PLUGIN_INSIGHT_PREFIX, "No Connections"); }

void InsightHandle::OnGeneralError(const com::htsc::mdc::insight::model::InsightErrorContext &context) {
    LOG_INFO(PLUGIN_INSIGHT_PREFIX + "PARSE message OnGeneralError: ", context.message());
}

void InsightHandle::handleSubscribeInfo(const string &marketDataTypes, const string &securityIdSource,
                                        const string &securityType) {
    if (marketDataTypes == "MD_ORDER") {
        orderUtil_.markets_.insert("(" + securityIdSource + ", " + securityType + ")");
    } else if (marketDataTypes == "MD_TRANSACTION") {
        transactionUtil_.markets_.insert("(" + securityIdSource + ", " + securityType + ")");
    } else if (marketDataTypes == "MD_ORDER_TRANSACTION") {
        orderTransactionMarket_.insert("(" + securityIdSource + ", " + securityType + ")");
    } else if (marketDataTypes == "MD_TICK" && securityType == "StockType") {
        stockUtil_.markets_.insert("(" + securityIdSource + ", " + securityType + ")");
    } else if (securityType == "IndexType") {
        indexUtil_.markets_.insert("(" + securityIdSource + ", " + securityType + ")");
    } else if (securityType == "FuturesType") {
        futureUtil_.markets_.insert("(" + securityIdSource + ", " + securityType + ")");
    } else if (marketDataTypes == "MD_TICK" && securityType == "FundType") {
        fundUtil_.markets_.insert("(" + securityIdSource + ", " + securityType + ")");
    } else if (marketDataTypes == "MD_TICK" && securityType == "BondType") {
        bondUtil_.markets_.insert("(" + securityIdSource + ", " + securityType + ")");
    } else if (marketDataTypes == "MD_TICK" && securityType == "OptionType") {
        optionUtil_.markets_.insert("(" + securityIdSource + ", " + securityType + ")");
    } else if (marketDataTypes == "MD_SECURITY_LENDING") {
        securityLendingUtil_.markets_.insert("(" + securityIdSource + ", " + securityType + ")");
    }
    // TODO
}

TableSP InsightHandle::getSchema(const string &type) {
    int flag = 0;
    if (receivedTime_) {
        flag |= MarketOptionFlag::OPT_RECEIVED;
    }
    if (outputElapsed_) {
        flag |= MarketOptionFlag::OPT_ELAPSED;
    }
    return insightTypeContainer_.getSchema(type, flag);
}

string vecToString(const unordered_set<string> &input) {
    if (input.empty()) {
        return "";
    }
    string ret;
    // ret += "(";
    for (auto &str : input) {
        ret += str;
        ret += ", ";
    }
    ret.pop_back();
    ret.pop_back();
    // ret += ")";
    return ret;
}

TableSP InsightHandle::getStatus() {
    vector<string> colNames{"topicType",
                            "channelNo",
                            START_TIME_STR,
                            END_TIME_STR,
                            FIRST_MSG_TIME_STR,
                            LAST_MSG_TIME_STR,
                            PROCESSED_MSG_COUNT_STR,
                            FAILED_MSG_COUNT_STR,
                            LAST_ERR_MSG_STR,
                            LAST_FAILED_TIMESTAMP_STR,
                            "subscribeInfo"};
    INDEX size = 10 + orderTransactionQueueMap_.size();
    VectorSP topicTypeDdbVec = Util::createVector(DT_STRING, 0, size);
    VectorSP channelNoDdbVec = Util::createVector(DT_INT, 0, size);
    VectorSP marketDdbVec = Util::createVector(DT_STRING, 0, size);
    VectorSP startTimeDdbVec = Util::createVector(DT_NANOTIMESTAMP, 0, size);
    VectorSP endTimeDdbVec = Util::createVector(DT_NANOTIMESTAMP, 0, size);
    VectorSP firstMsgTimeDdbVec = Util::createVector(DT_NANOTIMESTAMP, 0, size);
    VectorSP lastMsgTimeDdbVec = Util::createVector(DT_NANOTIMESTAMP, 0, size);
    VectorSP processedMsgCountDdbVec = Util::createVector(DT_LONG, 0, size);
    VectorSP lastErrMsgDdbVec = Util::createVector(DT_STRING, 0, size);
    VectorSP failedMsgCountDdbVec = Util::createVector(DT_LONG, 0, size);
    VectorSP lastFailedTimestampDdbVec = Util::createVector(DT_NANOTIMESTAMP, 0, size);

    vector<string> topicVec{"Order",     "Transaction", "OrderTransaction", "IndexTick",  "FuturesTick",
                            "StockTick", "FundTick",    "BondTick",         "OptionTick", "SecurityLending"};
    for (unsigned int i = 0; i < orderTransactionQueueMap_.size(); ++i) {
        topicVec.push_back("OrderTransaction");
    }
    vector<int> channelNoVec{INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN};
    for (auto &pair : orderTransactionQueueMap_) {
        channelNoVec.push_back(pair.first);
    }

    vector<string> marketVec{
        vecToString(orderUtil_.markets_),     vecToString(transactionUtil_.markets_),
        vecToString(orderTransactionMarket_), vecToString(indexUtil_.markets_),
        vecToString(futureUtil_.markets_),    vecToString(stockUtil_.markets_),
        vecToString(fundUtil_.markets_),      vecToString(bondUtil_.markets_),
        vecToString(optionUtil_.markets_),    vecToString(securityLendingUtil_.markets_),
    };
    for (unsigned int i = 0; i < orderTransactionQueueMap_.size(); ++i) {
        marketVec.push_back(vecToString(orderTransactionMarket_));
    }
    topicTypeDdbVec->appendString(topicVec.data(), size);
    channelNoDdbVec->appendInt(channelNoVec.data(), size);
    marketDdbVec->appendString(marketVec.data(), size);
#define ADD_STATISTIC(queue)                                                   \
    {                                                                          \
        MarketStatus stats = queue->getStatusConst();                          \
        startTimeDdbVec->appendLong(&stats.startTime_, 1);                     \
        endTimeDdbVec->appendLong(&stats.endTime_, 1);                         \
        firstMsgTimeDdbVec->appendLong(&stats.firstMsgTime_, 1);               \
        lastMsgTimeDdbVec->appendLong(&stats.lastMsgTime_, 1);                 \
        processedMsgCountDdbVec->appendLong(&stats.processedMsgCount_, 1);     \
        lastErrMsgDdbVec->appendString(&stats.lastErrMsg_, 1);                 \
        failedMsgCountDdbVec->appendLong(&stats.failedMsgCount_, 1);           \
        lastFailedTimestampDdbVec->appendLong(&stats.lastFailedTimestamp_, 1); \
    }
    ADD_STATISTIC(orderUtil_.queue_)
    ADD_STATISTIC(transactionUtil_.queue_)
    ADD_STATISTIC(orderTransactionQueue_)
    ADD_STATISTIC(indexUtil_.queue_)
    ADD_STATISTIC(futureUtil_.queue_)
    ADD_STATISTIC(stockUtil_.queue_)
    ADD_STATISTIC(fundUtil_.queue_)
    ADD_STATISTIC(bondUtil_.queue_)
    ADD_STATISTIC(optionUtil_.queue_)
    ADD_STATISTIC(securityLendingUtil_.queue_)
    for (auto &pair : orderTransactionQueueMap_) {
        ADD_STATISTIC(pair.second);
    }
#undef ADD_STATISTIC

    vector<ConstantSP> cols{
        topicTypeDdbVec,   channelNoDdbVec,         startTimeDdbVec,      endTimeDdbVec,    firstMsgTimeDdbVec,
        lastMsgTimeDdbVec, processedMsgCountDdbVec, failedMsgCountDdbVec, lastErrMsgDdbVec, lastFailedTimestampDdbVec,
        marketDdbVec};
    return Util::createTable(colNames, cols);
}