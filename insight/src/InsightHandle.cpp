#include "InsightHandle.h"

#include <iostream>
#include <string>

#include "Exceptions.h"
#include "Logger.h"
#include "Types.h"

using namespace std;

const static int TIMEOUT = 100;
const static int CAPACITY = 100000;
InsightHandle::InsightHandle(SessionSP session, DictionarySP handles, bool ignoreApplSeq, bool receivedTime,
                             bool outputElapsed)
    : receivedTime_(receivedTime), outputElapsed_(outputElapsed), session_(session) {
    initTypeContainer(insightTypeContainer_);

    int flag = 0;
    if (receivedTime_) {
        flag |= MarketOptionFlag::OPT_RECEIVED;
    }
    if (outputElapsed_) {
        flag |= MarketOptionFlag::OPT_ELAPSED;
    }
    orderQueue_ =
        new ThreadedQueue<InsightOrder>(session->getHeap().get(), TIMEOUT, CAPACITY, insightTypeContainer_.get("Order"),
                                        nullptr, flag, "Order", PLUGIN_INSIGHT_PREFIX, Util::BUF_SIZE, orderReader);
    transactionQueue_ = new ThreadedQueue<InsightTransaction>(
        session->getHeap().get(), TIMEOUT, CAPACITY, insightTypeContainer_.get("Transaction"), nullptr, flag,
        "Transaction", PLUGIN_INSIGHT_PREFIX, Util::BUF_SIZE, transactionReader);
    orderTransactionQueue_ = new ThreadedQueue<InsightOrderTransaction>(
        session->getHeap().get(), TIMEOUT, CAPACITY, insightTypeContainer_.get("OrderTransaction"), nullptr, flag,
        "OrderTransaction", PLUGIN_INSIGHT_PREFIX, Util::BUF_SIZE, orderTransactionReaderFunction(ignoreApplSeq));
    indexTickQueue_ = new ThreadedQueue<InsightIndexTick>(
        session->getHeap().get(), TIMEOUT, CAPACITY, insightTypeContainer_.get("IndexTick"), nullptr, flag, "IndexTick",
        PLUGIN_INSIGHT_PREFIX, Util::BUF_SIZE, indexTickReader);
    futureTickQueue_ = new ThreadedQueue<InsightFutureTick>(
        session->getHeap().get(), TIMEOUT, CAPACITY, insightTypeContainer_.get("FuturesTick"), nullptr, flag,
        "FuturesTick", PLUGIN_INSIGHT_PREFIX, Util::BUF_SIZE, futureTickReader);
    stockTickQueue_ = new ThreadedQueue<InsightStockTick>(
        session->getHeap().get(), TIMEOUT, CAPACITY, insightTypeContainer_.get("StockTick"), nullptr, flag, "StockTick",
        PLUGIN_INSIGHT_PREFIX, Util::BUF_SIZE, stockTickReader);

    string errMsg =
        "insight::connect(handles, ip, port, user, password, [workPoolThreadCount=5], [options], "
        "[ignoreApplSeq=false])";
    ConstantSP handleStock = handles->getMember("StockTick");
    if (!handleStock.isNull() && !handleStock->isNull()) {
        if (handleStock->getForm() == DF_TABLE) {
            if(!((TableSP)handleStock)->isSharedTable() || ((TableSP)handleStock)->getTableType() != REALTIMETBL) {
                throw IllegalArgumentException("insight::connect", errMsg + " value of 'StockTick' must be stream table or dictionary");
            }
            if (handleStock->columns() != getSchema("StockTick")->rows()) {
                throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "the schema of tables should be the same expect " +
                                       std::to_string(handleStock->columns()) + " actual " +
                                       std::to_string(getSchema("StockTick")->rows()));
            }
        } else {
            throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "handle's values should be tables");
        }
        stockTickQueue_->setTable(handleStock);
        handleStock_ = handleStock;
    }

    ConstantSP handleIndex = handles->getMember("IndexTick");
    if (!handleIndex.isNull() && !handleIndex->isNull()) {
        if (handleIndex->getForm() == DF_TABLE) {
            if(!((TableSP)handleIndex)->isSharedTable() || ((TableSP)handleIndex)->getTableType() != REALTIMETBL) {
                throw IllegalArgumentException("insight::connect", errMsg + " value of 'IndexTick' must be stream table or dictionary");
            }
            if (handleIndex->columns() != getSchema("IndexTick")->rows()) {
                throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "the schema of tables should be the same");
            }
        } else {
            throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "handle's values should be tables");
        }
        indexTickQueue_->setTable(handleIndex);
        handleIndex_ = handleIndex;
    }

    ConstantSP handleFuture = handles->getMember("FuturesTick");
    if (!handleFuture.isNull() && !handleFuture->isNull()) {
        if (handleFuture->getForm() == DF_TABLE) {
            if(!((TableSP)handleFuture)->isSharedTable() || ((TableSP)handleFuture)->getTableType() != REALTIMETBL) {
                throw IllegalArgumentException("insight::connect", errMsg + " value of 'FuturesTick' must be stream table or dictionary");
            }
            if (handleFuture->columns() != getSchema("FuturesTick")->rows()) {
                throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "the schema of tables should be the same");
            }
        } else {
            throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "handle's values should be tables");
        }
        futureTickQueue_->setTable(handleFuture);
        handleFuture_ = handleFuture;
    }

    ConstantSP handleTransaction = handles->getMember("Transaction");
    if (handleTransaction.isNull() || handleTransaction->isNull()) {
        handleTransaction = handles->getMember("StockTransaction");
    }
    if (!handleTransaction.isNull() && !handleTransaction->isNull()) {
        if (handleTransaction->getForm() == DF_TABLE) {
            if(!((TableSP)handleTransaction)->isSharedTable() || ((TableSP)handleTransaction)->getTableType() != REALTIMETBL) {
                throw IllegalArgumentException("insight::connect", errMsg + " value of 'Transaction' must be stream table or dictionary");
            }
            if (handleTransaction->columns() != getSchema("Transaction")->rows()) {
                throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "the schema of tables should be the same");
            }
        } else {
            throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "handle's values should be tables");
        }
        transactionQueue_->setTable(handleTransaction);
        handleTransaction_ = handleTransaction;
    }

    ConstantSP handleOrder = handles->getMember("Order");
    if (handleOrder.isNull() || handleOrder->isNull()) {
        handleOrder = handles->getMember("StockOrder");
    }
    if (!handleOrder.isNull() && !handleOrder->isNull()) {
        if (handleOrder->getForm() == DF_TABLE) {
            if(!((TableSP)handleOrder)->isSharedTable() || ((TableSP)handleOrder)->getTableType() != REALTIMETBL) {
                throw IllegalArgumentException("insight::connect", errMsg + " value of 'Order' must be stream table or dictionary");
            }
            if (handleOrder->columns() != getSchema("Order")->rows()) {
                throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "the schema of tables should be the same");
            }
        } else {
            throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "handle's values should be tables");
        }
        orderQueue_->setTable(handleOrder);
        handleOrder_ = handleOrder;
    }

    ConstantSP handleOrderTransaction = handles->getMember("OrderTransaction");
    if (!handleOrderTransaction.isNull() && !handleOrderTransaction->isNull()) {
        if (handleOrderTransaction->getForm() == DF_TABLE) {
            if(!((TableSP)handleOrderTransaction)->isSharedTable() || ((TableSP)handleOrderTransaction)->getTableType() != REALTIMETBL) {
                throw IllegalArgumentException("insight::connect", errMsg + " value of 'OrderTransaction' must be stream table or dictionary");
            }
            if (handleOrderTransaction->columns() != getSchema("OrderTransaction")->rows()) {
                throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "the schema of tables should be the same");
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
                SmartPointer<ThreadedQueue<InsightOrderTransaction>> queue = new ThreadedQueue<InsightOrderTransaction>(
                    session->getHeap().get(), TIMEOUT, CAPACITY, insightTypeContainer_.get("OrderTransaction"), nullptr,
                    flag, "OrderTransaction_" + std::to_string(key->getInt()), PLUGIN_INSIGHT_PREFIX, Util::BUF_SIZE,
                    orderTransactionReaderFunction(ignoreApplSeq));
                queue->setTable(value);
                orderTransactionQueueMap_[key->getInt()] = queue;
            }
        } else {
            throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "handle's values should be tables");
        }
    }
}

void InsightHandle::OnMarketData(const com::htsc::mdc::insight::model::MarketData &record) {
    try {
        long long receiveTime = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
        switch (record.marketdatatype()) {
            case MD_TICK: {
                if (record.has_mdstock() && LIKELY(!stockTickQueue_.isNull() && stockTickQueue_->isStarted())) {
                    stockTickQueue_->push(InsightStockTick{receiveTime, record.mdstock()});
                } else if (record.has_mdindex() && LIKELY(!indexTickQueue_.isNull() && indexTickQueue_->isStarted())) {
                    indexTickQueue_->push(InsightIndexTick{receiveTime, record.mdindex()});
                } else if (record.has_mdfuture() && LIKELY(!futureTickQueue_.isNull() && futureTickQueue_->isStarted())) {
                    futureTickQueue_->push(InsightFutureTick{receiveTime, record.mdfuture()});
                }
                break;
            }

            case MD_TRANSACTION: {
                if (record.has_mdtransaction()) {
                    int channelno = record.mdtransaction().channelno();
                    if (LIKELY(!transactionQueue_.isNull() && transactionQueue_->isStarted())) {
                        transactionQueue_->push(InsightTransaction{receiveTime, record.mdtransaction()});
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
                    if (LIKELY(!orderQueue_.isNull() && orderQueue_->isStarted())) {
                        orderQueue_->push(InsightOrder{receiveTime, record.mdorder()});
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
            default: {
                throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "unsupported data type " +
                                       std::to_string(record.marketdatatype()));
            }
        }
    } catch (exception &e) {
        LOG_ERR(PLUGIN_INSIGHT_PREFIX + "Failed to received market data: " + string(e.what()));
    } catch (...) {
        LOG_ERR(PLUGIN_INSIGHT_PREFIX + "Failed to received market data.");
    }
}

void InsightHandle::OnLoginSuccess() { LOG_INFO(PLUGIN_INSIGHT_PREFIX + "Login Success"); }

void InsightHandle::OnLoginFailed(int errorNo, const string &message) {
    LOG_ERR(PLUGIN_INSIGHT_PREFIX + "Login Failed. Error number: ", errorNo, ". Error message: ", message);
}

void InsightHandle::OnNoConnections() { LOG_ERR(PLUGIN_INSIGHT_PREFIX + "No Connections"); }

void InsightHandle::OnGeneralError(const com::htsc::mdc::insight::model::InsightErrorContext &context) {
    LOG_INFO(PLUGIN_INSIGHT_PREFIX + "PARSE message OnGeneralError: ", context.message());
}
