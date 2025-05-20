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
#include "ddbplugin/ThreadedQueue.h"
#include "mdc_client_factory.h"
#include "message_handle.h"

using namespace std;
using namespace com::htsc::mdc::gateway;
using namespace com::htsc::mdc::insight::model;
using namespace com::htsc::mdc::model;

static string PLUGIN_INSIGHT_PREFIX = "[PLUGIN::INSIGHT]: ";

using namespace ddb;

using namespace ThreadedQueueUtil;

enum INSIGHT_DATA_TYPE {
    INSIGHT_DT_VOID,
    INSIGHT_DT_StockTick,
    INSIGHT_DT_FundTick,
    INSIGHT_DT_BondTick,
    INSIGHT_DT_OptionTick,
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

void initTypeContainer(MarketTypeContainer<> &container, const string &dataVersion);

void orderReader(vector<ConstantSP> &buffer, TimedWrapper<MDOrder> &data);
void orderReader_3_2_11(vector<ConstantSP> &buffer, TimedWrapper<MDOrder> &data);
void transactionReader(vector<ConstantSP> &buffer, TimedWrapper<MDTransaction> &data);
void orderTransactionReader(vector<ConstantSP> &buffer, InsightOrderTransaction &data, int seqCheckMode,
                            std::unordered_map<int, long long> &szLastSeqNum,
                            std::unordered_map<int, long long> &shLastSeqNum);

class orderTransactionReaderFunction {
  public:
    orderTransactionReaderFunction(int seqCheckMode) : seqCheckMode_(seqCheckMode) {}
    void operator()(vector<ConstantSP> &buffer, InsightOrderTransaction &data) {
        orderTransactionReader(buffer, data, seqCheckMode_, shLastSeqNum_, szLastSeqNum_);
    }

  private:
    int seqCheckMode_;
    std::unordered_map<int, long long> shLastSeqNum_;
    std::unordered_map<int, long long> szLastSeqNum_;
};

void indexTickReader(vector<ConstantSP> &buffer, TimedWrapper<MDIndex> &data);
void futureTickReader(vector<ConstantSP> &buffer, TimedWrapper<MDFuture> &data);
void stockTickReader(vector<ConstantSP> &buffer, TimedWrapper<MDStock> &data);

void fundTickReader(vector<ConstantSP> &buffer, TimedWrapper<MDFund> &data);
void bondTickReader(vector<ConstantSP> &buffer, TimedWrapper<MDBond> &data);
void optionTickReader(vector<ConstantSP> &buffer, TimedWrapper<MDOption> &data);

void securityLendingTickReader(vector<ConstantSP> &buffer, TimedWrapper<MDSecurityLending> &data);

#endif