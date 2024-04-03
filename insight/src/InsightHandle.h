#ifndef INSIGHT_HANDLE_H
#define INSIGHT_HANDLE_H

#include <climits>
#include <string>
#include <vector>

#include "Concurrent.h"
#include "CoreConcept.h"
#include "InsightPlugin.h"
#include "InsightType.h"
#include "ScalarImp.h"
#include "Types.h"
#include "Util.h"
#include "base_define.h"
#include "client_interface.h"
#include "ddbplugin/ThreadedQueue.h"
#include "mdc_client_factory.h"
#include "message_handle.h"

using namespace com::htsc::mdc::gateway;
using namespace com::htsc::mdc::model;
using namespace com::htsc::mdc::insight::model;

class InsightHandle : public MessageHandle {
  public:
    InsightHandle(SessionSP session, DictionarySP handles, bool ignoreApplSeq, bool receivedTime, bool outputElapsed);
    virtual ~InsightHandle() {
        orderMarket_.clear();
        transactionMarket_.clear();
        orderTransactionMarket_.clear();
        indexTickMarket_.clear();
        futureTickMarket_.clear();
        stockTickMarket_.clear();
        orderQueue_ = nullptr;
        transactionQueue_ = nullptr;
        orderTransactionQueue_ = nullptr;
        orderTransactionQueueMap_.clear();
        indexTickQueue_ = nullptr;
        futureTickQueue_ = nullptr;
        stockTickQueue_ = nullptr;

        handleIndex_ = nullptr;
        handleStock_ = nullptr;
        handleFuture_ = nullptr;
        handleTransaction_ = nullptr;
        handleOrder_ = nullptr;
        handleOrderTransaction_ = nullptr;
    }
    void OnMarketData(const MarketData &record) override;
    void OnLoginSuccess() override;
    void OnLoginFailed(int errorNo, const std::string &message) override;
    void OnNoConnections() override;
    void OnGeneralError(const InsightErrorContext &context) override;

    void checkColumnsSize(std::vector<ConstantSP> &columns, INSIGHT_DATA_TYPE type);

    void tryStartAfterSubscribe() {
        if (!orderMarket_.empty() && !(handleOrder_.isNull() || handleOrder_->isNull()) && !orderQueue_.isNull() && !orderQueue_->isStarted()) {
            orderQueue_->start();
        }
        if (!transactionMarket_.empty() && !(handleTransaction_.isNull() || handleTransaction_->isNull()) &&
            !transactionQueue_.isNull() && !transactionQueue_->isStarted()) {
            transactionQueue_->start();
        }
        if (!orderTransactionMarket_.empty() &&
            !(handleOrderTransaction_.isNull() || handleOrderTransaction_->isNull()) &&
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
        if (!indexTickMarket_.empty() && !(handleIndex_.isNull() || handleIndex_->isNull()) &&
            !orderTransactionQueue_.isNull() && !indexTickQueue_->isStarted()) {
            indexTickQueue_->start();
        }
        if (!futureTickMarket_.empty() && !(handleFuture_.isNull() || handleFuture_->isNull()) &&
            !futureTickQueue_.isNull() && !futureTickQueue_->isStarted()) {
            futureTickQueue_->start();
        }
        if (!stockTickMarket_.empty() && !(handleStock_.isNull() || handleStock_->isNull()) &&
            !stockTickQueue_.isNull() && !stockTickQueue_->isStarted()) {
            stockTickQueue_->start();
        }
    }
    void stopAll() {
        if(!orderQueue_.isNull()) { orderQueue_->stop(); }
        if(!transactionQueue_.isNull()) { transactionQueue_->stop(); }
        if(!orderTransactionQueue_.isNull()) { orderTransactionQueue_->stop(); }
        for (auto &pair : orderTransactionQueueMap_) {
            pair.second->stop();
        }
        if(!indexTickQueue_.isNull()) { indexTickQueue_->stop(); }
        if(!futureTickQueue_.isNull()) { futureTickQueue_->stop(); }
        if(!stockTickQueue_.isNull()) { stockTickQueue_->stop(); }
    }
    void handleSubscribeInfo(const string &marketDataTypes, const string &securityIdSource,
                             const string &securityType) {
        if (marketDataTypes == "MD_ORDER") {
            orderMarket_.insert("(" + securityIdSource + ", " + securityType + ")");
        } else if (marketDataTypes == "MD_TRANSACTION") {
            transactionMarket_.insert("(" + securityIdSource + ", " + securityType + ")");
        } else if (marketDataTypes == "MD_ORDER_TRANSACTION") {
            orderTransactionMarket_.insert("(" + securityIdSource + ", " + securityType + ")");
        } else if (marketDataTypes == "MD_TICK" && securityType == "StockType") {
            stockTickMarket_.insert("(" + securityIdSource + ", " + securityType + ")");
        } else if (securityType == "IndexType") {
            indexTickMarket_.insert("(" + securityIdSource + ", " + securityType + ")");
        } else if (securityType == "FuturesType") {
            futureTickMarket_.insert("(" + securityIdSource + ", " + securityType + ")");
        }
    }

    TableSP getSchema(const string &type) {
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
    TableSP getStatus() {
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
        INDEX size = 6 + orderTransactionQueueMap_.size();
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

        vector<string> topicVec{"Order", "Transaction", "OrderTransaction", "IndexTick", "FuturesTick", "StockTick"};
        for (unsigned int i = 0; i < orderTransactionQueueMap_.size(); ++i) {
            topicVec.push_back("OrderTransaction");
        }
        vector<int> channelNoVec{INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN};
        for (auto &pair : orderTransactionQueueMap_) {
            channelNoVec.push_back(pair.first);
        }

        vector<string> marketVec{
            vecToString(orderMarket_),     vecToString(transactionMarket_), vecToString(orderTransactionMarket_),
            vecToString(indexTickMarket_), vecToString(futureTickMarket_),  vecToString(stockTickMarket_),
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
        ADD_STATISTIC(orderQueue_)
        ADD_STATISTIC(transactionQueue_)
        ADD_STATISTIC(orderTransactionQueue_)
        ADD_STATISTIC(indexTickQueue_)
        ADD_STATISTIC(futureTickQueue_)
        ADD_STATISTIC(stockTickQueue_)
        for (auto &pair : orderTransactionQueueMap_) {
            ADD_STATISTIC(pair.second);
        }
#undef ADD_STATISTIC

        vector<ConstantSP> cols{topicTypeDdbVec,
                                channelNoDdbVec,
                                startTimeDdbVec,
                                endTimeDdbVec,
                                firstMsgTimeDdbVec,
                                lastMsgTimeDdbVec,
                                processedMsgCountDdbVec,
                                failedMsgCountDdbVec,
                                lastErrMsgDdbVec,
                                lastFailedTimestampDdbVec,
                                marketDdbVec};
        return Util::createTable(colNames, cols);
    }

  protected:
    bool receivedTime_;
    bool outputElapsed_;
    SessionSP session_;
    unordered_set<string> orderMarket_;
    SmartPointer<ThreadedQueue<InsightOrder>> orderQueue_;
    unordered_set<string> transactionMarket_;
    SmartPointer<ThreadedQueue<InsightTransaction>> transactionQueue_;
    unordered_set<string> orderTransactionMarket_;
    SmartPointer<ThreadedQueue<InsightOrderTransaction>> orderTransactionQueue_;
    std::unordered_map<int, SmartPointer<ThreadedQueue<InsightOrderTransaction>>> orderTransactionQueueMap_;
    unordered_set<string> indexTickMarket_;
    SmartPointer<ThreadedQueue<InsightIndexTick>> indexTickQueue_;
    unordered_set<string> futureTickMarket_;
    SmartPointer<ThreadedQueue<InsightFutureTick>> futureTickQueue_;
    unordered_set<string> stockTickMarket_;
    SmartPointer<ThreadedQueue<InsightStockTick>> stockTickQueue_;
    MarketTypeContainer insightTypeContainer_;

    ConstantSP handleIndex_;
    ConstantSP handleStock_;
    ConstantSP handleFuture_;
    ConstantSP handleTransaction_;
    ConstantSP handleOrder_;
    ConstantSP handleOrderTransaction_;
};

#endif
