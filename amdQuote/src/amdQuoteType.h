#ifndef AMD_QUOTE_TYPE_H
#define AMD_QUOTE_TYPE_H

#include "CoreConcept.h"
#include "Logger.h"
#include "Types.h"
#include "Util.h"
#include "ama.h"

static const int ORDER_EXECUTION_OFFSET = 1000;
static const string AMDQUOTE_PREFIX = "[PLUGIN::AMDQUOTE] ";

enum AMDDataType {
    AMD_SNAPSHOT,
    AMD_EXECUTION,
    AMD_ORDER,
    AMD_FUND_SNAPSHOT,
    AMD_FUND_EXECUTION,
    AMD_FUND_ORDER,
    AMD_BOND_SNAPSHOT,
    AMD_BOND_EXECUTION,
    AMD_BOND_ORDER,
    AMD_ORDER_EXECUTION,
    AMD_FUND_ORDER_EXECUTION,
    AMD_BOND_ORDER_EXECUTION,
    AMD_INDEX,
    AMD_ORDER_QUEUE,
    AMD_OPTION_SNAPSHOT,
    AMD_FUTURE_SNAPSHOT,
#ifndef AMD_3_9_6
    AMD_IOPV_SNAPSHOT,
#endif
    AMD_ERROR_DATA_TYPE,
};

enum AMDTableType {
    AMD_SNAPSHOT_SH,
    AMD_EXECUTION_SH,
    AMD_ORDER_SH,
    AMD_INDEX_SH,
    AMD_ORDER_QUEUE_SH,
    AMD_FUND_SNAPSHOT_SH,
    AMD_FUND_EXECUTION_SH,
    AMD_FUND_ORDER_SH,
    AMD_BOND_SNAPSHOT_SH,
    AMD_BOND_EXECUTION_SH,
    AMD_BOND_ORDER_SH,
    AMD_OPTION_SNAPSHOT_SH,
    AMD_FUTURE_SNAPSHOT_SH,
#ifndef AMD_3_9_6
    AMD_IOPV_SNAPSHOT_SH,
#endif

    AMD_SNAPSHOT_SZ,
    AMD_EXECUTION_SZ,
    AMD_ORDER_SZ,
    AMD_INDEX_SZ,
    AMD_ORDER_QUEUE_SZ,
    AMD_FUND_SNAPSHOT_SZ,
    AMD_FUND_EXECUTION_SZ,
    AMD_FUND_ORDER_SZ,
    AMD_BOND_SNAPSHOT_SZ,
    AMD_BOND_EXECUTION_SZ,
    AMD_BOND_ORDER_SZ,
    AMD_OPTION_SNAPSHOT_SZ,
    AMD_FUTURE_SNAPSHOT_SZ,
#ifndef AMD_3_9_6
    AMD_IOPV_SNAPSHOT_SZ,
#endif

    AMD_ORDER_EXECUTION_SH,
    AMD_ORDER_EXECUTION_SZ,
    AMD_FUND_ORDER_EXECUTION_SH,
    AMD_FUND_ORDER_EXECUTION_SZ,
    AMD_BOND_ORDER_EXECUTION_SH,
    AMD_BOND_ORDER_EXECUTION_SZ,
    AMD_ERROR_TABLE_TYPE,
};

// union structure
struct MDOrderExecution {
    bool orderOrExecution;
    long long reachTime;
    union {
        amd::ama::MDTickOrder tickOrder;
        amd::ama::MDTickExecution tickExecution;
    } uni;
};
struct MDBondOrderExecution {
    bool orderOrExecution;
    long long reachTime;
    union {
        amd::ama::MDBondTickOrder tickOrder;
        amd::ama::MDBondTickExecution tickExecution;
    } uni;
};
struct timeMDSnapshot {
    long long reachTime;
    amd::ama::MDSnapshot snapshot;
};
struct timeMDTickOrder {
    long long reachTime;
    amd::ama::MDTickOrder order;
};
struct timeMDTickExecution {
    long long reachTime;
    amd::ama::MDTickExecution execution;
};
struct timeMDBondSnapshot {
    long long reachTime;
    amd::ama::MDBondSnapshot bondSnapshot;
};
struct timeMDBondTickOrder {
    long long reachTime;
    amd::ama::MDBondTickOrder bondOrder;
};
struct timeMDBondTickExecution {
    long long reachTime;
    amd::ama::MDBondTickExecution bondExecution;
};
struct timeMDIndexSnapshot {
    long long reachTime;
    amd::ama::MDIndexSnapshot indexSnapshot;
};
struct timeMDOrderQueue {
    long long reachTime;
    amd::ama::MDOrderQueue orderQueue;
};

struct timeMDOption {
    long long reachTime;
    amd::ama::MDOptionSnapshot option;
};

struct timeMDFuture {
    long long reachTime;
    amd::ama::MDFutureSnapshot future;
};

#ifndef AMD_3_9_6
struct timeMDIOPV {
    long long reachTime;
    amd::ama::MDIOPVSnapshot IOPV;
};
#endif

class MarketTypeContainer;
void initTypeContainer(MarketTypeContainer &container);
void initSecurityCodeToIntTypeContainer(MarketTypeContainer& container);

void indexReader(vector<ConstantSP> &buffer, timeMDIndexSnapshot &data, bool securityCodeToInt);
void orderQueueReader(vector<ConstantSP> &buffer, timeMDOrderQueue &data, bool securityCodeToInt);
void snapshotReader(vector<ConstantSP> &buffer, timeMDSnapshot &data, bool securityCodeToInt);
void orderReader(vector<ConstantSP> &buffer, timeMDTickOrder &data, bool securityCodeToInt);
void executionReader(vector<ConstantSP> &buffer, timeMDTickExecution &data, bool securityCodeToInt);
void bondSnapshotReader(vector<ConstantSP> &buffer, timeMDBondSnapshot &data, bool securityCodeToInt);
void optionReader(vector<ConstantSP> &buffer, timeMDOption &data, bool securityCodeToInt);
void futureReader(vector<ConstantSP> &buffer, timeMDFuture &data, bool securityCodeToInt);
#ifndef AMD_3_9_6
void IOPVReader(vector<ConstantSP> &buffer, timeMDIOPV &data, bool securityCodeToInt);
#endif

void bondOrderReader(vector<ConstantSP> &buffer, timeMDBondTickOrder &data, bool securityCodeToInt);
void bondExecutionReader(vector<ConstantSP> &buffer, timeMDBondTickExecution &data, bool securityCodeToInt);
void orderExecutionReader(vector<ConstantSP> &buffer, MDOrderExecution &data);
void bondOrderExecutionReader(vector<ConstantSP> &buffer, MDBondOrderExecution &data);

// TODO(ruibinhuang@dolphindb.com): check the real attributes of the table
bool checkSchema(const string &type, TableSP table);
// to cooperate with insight based orderbookSnapshot.
// use convertBSFlag() & convertType() to change amd flag & type to insight flag & type.
// if a flag & type not exist in insight, return origin value.
// details see https://dolphindb1.atlassian.net/browse/DPLG-837
int convertBSFlag(int flag);
long long countTemporalUnit(int days, long long multiplier, long long remainder);
int countDays(int amdDays);
int convertType(int type);
int convertToDate(long long time);
int convertToTime(long long time);
long long convertTime(long long time);
string transMarket(int type);
AMDDataType getAmdDataType(const string &typeStr);
long long countTemporalUnit(int days, long long multiplier, long long remainder);
int countDays(int amdDays);
// to cooperate with insight based orderbookSnapshot.
// use convertBSFlag() & convertType() to change amd flag & type to insight flag & type.
// if a flag & type not exist in insight, return origin value.
// details see https://dolphindb1.atlassian.net/browse/DPLG-837
int convertBSFlag(int flag);
int convertType(int type);

class DailyIndex {
  public:
    DailyIndex() : startTimestamp_(LONG_LONG_MIN) {}
    explicit DailyIndex(long long timestamp) : startTimestamp_(timestamp) {}

    inline int getIndex(int32_t param, long long timestamp) {
        if (startTimestamp_ == LONG_LONG_MIN)
            return INT_MIN;
            // throw RuntimeException(AMDQUOTE_PREFIX + " getIndex failed because DailyIndex was not set");
        const long long dateTimestamp = 24 * 60 * 60 * 1000;
        long long originBase = startTimestamp_ / dateTimestamp;
        long long newBase = timestamp / dateTimestamp;
        if (originBase < newBase) {
            startTimestamp_ = newBase * dateTimestamp;
            LOG_INFO(AMDQUOTE_PREFIX + ": The new DailyIndex with channel_no as " + std::to_string(param) +
                     " will start at " + std::to_string(startTimestamp_));
            indexMap_.clear();
        }
        if (timestamp < startTimestamp_) {
            return INT_MIN;
        }
        if (indexMap_.count(param) != 0) {
            return ++indexMap_[param];
        } else {
            indexMap_[param] = 0;
            return 0;
        }
    }

  private:
    long long startTimestamp_;
    unordered_map<int, int> indexMap_;
};

template <typename T>
int getDailyIndex(DailyIndex &index, T &data, long long timestamp);

#endif  // AMD_QUOTE_TYPE_H
