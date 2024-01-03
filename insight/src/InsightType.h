#ifndef INSIGHT_TYPE_H
#define INSIGHT_TYPE_H

#include <string>
#include <vector>

#include "Concurrent.h"
#include "CoreConcept.h"
#include "ESecurityIDSource.pb.h"
#include "InsightPlugin.h"
#include "MDOrder.pb.h"
#include "MDTransaction.pb.h"
#include "ScalarImp.h"
#include "Util.h"
#include "base_define.h"
#include "client_interface.h"
#include "mdc_client_factory.h"
#include "message_handle.h"

using namespace std;
using namespace com::htsc::mdc::gateway;
using namespace com::htsc::mdc::insight::model;
using namespace com::htsc::mdc::model;

static string PLUGIN_INSIGHT_PREFIX = "[PLUGIN::INSIGHT]: ";

enum INSIGHT_DATA_TYPE {
    INSIGHT_DT_VOID,
    INSIGHT_DT_StockTick,
    INSIGHT_DT_IndexTick,
    INSIGHT_DT_FuturesTick,
    INSIGHT_DT_StockTransaction,
    INSIGHT_DT_StockOrder
};

struct InsightOrderTransaction {
    bool orderOrTransaction;
    long long reachTime;
    MDOrder order;
    MDTransaction transaction;
};

struct InsightOrder {
    long long reachTime;
    MDOrder order;
};

struct InsightTransaction {
    long long reachTime;
    MDTransaction transaction;
};

struct InsightIndexTick {
    long long reachTime;
    MDIndex indexTick;
};

struct InsightFutureTick {
    long long reachTime;
    MDFuture futureTick;
};

struct InsightStockTick {
    long long reachTime;
    MDStock stockTick;
};

class MarketTypeContainer;
void initTypeContainer(MarketTypeContainer &container);

void orderReader(vector<ConstantSP> &buffer, InsightOrder &data);
void transactionReader(vector<ConstantSP> &buffer, InsightTransaction &data);
void orderTransactionReader(vector<ConstantSP> &buffer, InsightOrderTransaction &data, bool ignoreApplSeq,
                            std::unordered_map<int, int64_t> &lastSeqNum, std::unordered_map<int, int64_t> &tradeSeqNum,
                            std::unordered_map<int, int64_t> &orderSeqNum);

class orderTransactionReaderFunction {
  public:
    orderTransactionReaderFunction(bool ignoreApplSeq) : ignoreApplSeq_(ignoreApplSeq) {}
    void operator()(vector<ConstantSP> &buffer, InsightOrderTransaction &data) {
        orderTransactionReader(buffer, data, ignoreApplSeq_, lastSeqNum_, tradeSeqNum_, orderSeqNum_);
    }

  private:
    bool ignoreApplSeq_ = false;
    std::unordered_map<int, int64_t> lastSeqNum_;
    std::unordered_map<int, int64_t> tradeSeqNum_;
    std::unordered_map<int, int64_t> orderSeqNum_;
};

void indexTickReader(vector<ConstantSP> &buffer, InsightIndexTick &data);
void futureTickReader(vector<ConstantSP> &buffer, InsightFutureTick &data);
void stockTickReader(vector<ConstantSP> &buffer, InsightStockTick &data);

#endif