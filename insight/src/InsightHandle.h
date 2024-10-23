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

template <typename T>
struct InsightQuoteUtil {
    unordered_set<string> markets_;
    typename TimedWrapperTrait<T>::Type queue_;
    ConstantSP handle_;

    void clear() {
        markets_.clear();
        queue_.clear();
        handle_.clear();
    }

    void startQueue() {
        if (!markets_.empty() && !(handle_.isNull() || handle_->isNull()) && !queue_.isNull() && !queue_->isStarted()) {
            queue_->start();
        }
    }

    void stopQueue() {
        if (!queue_.isNull()) {
            queue_->stop();
        }
    }
};

class InsightHandle : public MessageHandle {
  public:
    InsightHandle(SessionSP session, DictionarySP handles, int seqCheckMode, bool receivedTime, bool outputElapsed,
                  const string &dataVersion);
    ~InsightHandle() override;
    void OnMarketData(const MarketData &record) override;
    void OnLoginSuccess() override;
    void OnLoginFailed(int errorNo, const std::string &message) override;
    void OnNoConnections() override;
    void OnGeneralError(const InsightErrorContext &context) override;
    void OnReconnect() override {}  // TODO
    void OnServiceMessage(const ::com::htsc::mdc::insight::model::MarketDataStream &data_stream) override {}
    void OnSubscribeResponse(const ::com::htsc::mdc::insight::model::MDSubscribeResponse &response) override {}
    void checkColumnsSize(std::vector<ConstantSP> &columns, INSIGHT_DATA_TYPE type);
    void handleSubscribeInfo(const string &marketDataTypes, const string &securityIdSource, const string &securityType);
    void tryStartAfterSubscribe();
    void stopAll();
    TableSP getSchema(const string &type);
    TableSP getStatus();

    template <typename T>
    void setParam(DictionarySP handles, ConstantSP &handle, typename TimedWrapperTrait<T>::Type &queue,
                  const string &typeStr, const string &syntax) {
        ConstantSP member = handles->getMember(typeStr);
        if (!member.isNull() && !member->isNull()) {
            if (member->getForm() == DF_TABLE) {
                if (!((TableSP)member)->isSharedTable() || ((TableSP)member)->getTableType() != REALTIMETBL) {
                    throw IllegalArgumentException(
                        "insight::connect", syntax + " value of '" + typeStr + "' must be stream table or dictionary");
                }
                if (member->columns() != getSchema(typeStr)->rows()) {
                    throw RuntimeException(PLUGIN_INSIGHT_PREFIX + typeStr + " table expect columns " +
                                           std::to_string(getSchema(typeStr)->rows()) + ", actual " +
                                           std::to_string(member->columns()));
                }
            } else {
                throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "handle's values should be tables");
            }
            queue->setTable(member);
            handle = member;
        }
    }

  protected:
    bool receivedTime_;
    bool outputElapsed_;
    string dataVersion_;
    SessionSP session_;
    MarketTypeContainer<> insightTypeContainer_;

    InsightQuoteUtil<MDOrder> orderUtil_;
    InsightQuoteUtil<MDTransaction> transactionUtil_;
    InsightQuoteUtil<MDIndex> indexUtil_;
    InsightQuoteUtil<MDFuture> futureUtil_;
    InsightQuoteUtil<MDFund> fundUtil_;
    InsightQuoteUtil<MDBond> bondUtil_;
    InsightQuoteUtil<MDOption> optionUtil_;
    InsightQuoteUtil<MDStock> stockUtil_;
    InsightQuoteUtil<MDSecurityLending> securityLendingUtil_;

    unordered_set<string> orderTransactionMarket_;
    SmartPointer<ThreadedQueue<InsightOrderTransaction>> orderTransactionQueue_;
    std::unordered_map<int, SmartPointer<ThreadedQueue<InsightOrderTransaction>>> orderTransactionQueueMap_;
    ConstantSP handleOrderTransaction_;
};

#endif
