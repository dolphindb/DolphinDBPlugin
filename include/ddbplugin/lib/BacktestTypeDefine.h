#pragma once

#include <string>
#include <vector>

#include "CoreConcept.h"

#define BACKTEST_BUF_SIZE 1024

// data type
#define TICK_MODE 0
#define SNAPSHOT_MODE 1
#define SNAPSHOT_WITH_TRADE_MODE 2
#define MINUTE_MODE 3
#define DAY_MODE 4
#define WIDE_TICK_MODE 5
#define WIDE_TICK_WITH_SNAPSHOT_MODE 6

// normal order direction
#define ORDER_BUY_OPEN 1
#define ORDER_SEL_OPEN 2
#define ORDER_SEL_CLOSE 3
#define ORDER_BUY_CLOSE 4

// margin trading order direction
#define ORDER_BUY_SECU 1
#define ORDER_SELL_SECU 2
#define ORDER_MARGIN_BUY 3
#define ORDER_SECU_LENDING 4
#define ORDER_REPAY_AMOUNT 5
#define ORDER_SELL_SECU_REPAY_AMOUNT 6
#define ORDER_REPAY_SECU 7
#define ORDER_BUY_SECU_REPAY_SECU 8

#define TRADE_BUY_FLAG 1
#define TRADE_SEL_FLAG 2

#define REJECT_CANCEL_STATE (-2)
#define REJECT_STATE (-1)
#define PART_TRADE_STATE 0
#define ALL_TRADE_STATE 1
#define CANCEL_STATE 2
#define COMMIT_STATE 4

namespace ddb {

struct MsgWrapper {
    explicit MsgWrapper(const ConstantSP &obj) : obj_(obj) {
        isTable_ = obj->isTable();
        if (isTable_)
            rows_ = obj->rows();
        else
            rows_ = obj->get(0)->rows();
    }

    inline ConstantSP getColumn(int index) {
        if (index >= columns()) return nullptr;
        if (isTable_)
            return obj_->getColumn(index);
        else
            return obj_->get(index);
    }

    inline INDEX columns() {
        if (isTable_)
            return obj_->columns();
        else
            return obj_->rows();
    }

    inline INDEX rows() { return rows_; }

  private:
    ConstantSP obj_;
    INDEX rows_;
    bool isTable_;
};

enum class UserOrderState { CANCEL_REJECT = -2, REJECT = -1, PART = 0, FINISH = 1, CANCEL = 2, COMMIT = 4 };

enum class BacktestMode {
    Tick_With_Snapshot,
    Tick_Without_Snapshot,
    Snapshot_Mode_1,
    Snapshot_Mode_2,
    Minute_Frequency,
    Day_Frequency
};

struct TradeRecordData {
    long long id_;
    string symbol_;
    int BSFlag_;
    double orderPrice_;
    long long tradeTime_;
    double tradePrice_;
    long long tradeQty_;
    int orderStatus_;
};

}  // namespace ddb
