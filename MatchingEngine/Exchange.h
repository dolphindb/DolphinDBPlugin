//
// Created by jccai on 19-5-24.
//

#ifndef MATCHINGENGINE_EXCHANGE_H
#define MATCHINGENGINE_EXCHANGE_H

#include <iostream>
#include "Config.h"
#include "StreamEngine.h"
#include "book/depth_order_book.h"
#include "book/listener.h"
#include "book/order.h"
#include "export.h"

using Price = book::Price;
using Quantity = book::Quantity;
using Cost = book::Cost;

using OrderOperation = book::OrderOperation;
using OrderType = book::OrderType;
using OrderPtr = book::OrderPtr;
using OrderBook = book::OrderBook<OrderPtr>;
using OrderBookPtr = std::shared_ptr<OrderBook>;
using DepthOrderBook = book::DepthOrderBook<OrderPtr>;
using DepthOrderBookPtr = std::shared_ptr<DepthOrderBook>;
using BookDepth = book::Depth;

// for simplicity, Exchange only manages one symbol
class Exchange : public AbstractStreamEngine,
                 public book::OrderListener<OrderPtr>,
                 public book::TradeListener<OrderBook>,
                 public book::OrderBookListener<OrderBook>,
                 public book::BboListener<DepthOrderBook>,
                 public book::DepthListener<DepthOrderBook> {
public:
    explicit Exchange(std::string &&symbol, std::string &&creater, TableSP outputTable, TableSP depthOutputTable);

    ~Exchange() = default;

    static ConstantSP setupGlobalConfig(Heap *heap, vector<ConstantSP> &args);
    static ConstantSP createExchange(Heap *heap, vector<ConstantSP> &args);

    const std::string &symbol() const {
        return symbol_;
    }

    void on_bbo_change(const DepthOrderBook *book, const BookDepth *depth) override {
        // debug() << "bbo_change" << std::endl;
    }

    void on_depth_change(const DepthOrderBook *book, const BookDepth *depth) override {
        const book::DepthLevel *level = depth->bids();
        using namespace std;
        // type level price aggreate_qty order_count
        auto market_price = book->market_price();
        int id = 0;
        for (; level < depth->end(); ++level, ++id) {
            depthOutput_[2]->setLong(id, level->price());
            depthOutput_[3]->setLong(id, level->aggregate_qty());
            depthOutput_[4]->setLong(id, level->order_count());
            depthOutput_[5]->setLong(id, market_price);
        }
        INDEX insertedRows = -1;
        depthOutputTable_->append(depthOutput_, insertedRows, lastErrMsg_);
    }

    void on_order_book_change(const OrderBook *book) override {
        // debug() << "order_book_change" << std::endl;
    }

    void on_accept(const OrderPtr &order) override {
        order->onAccepted();
        // debug() << "order accepted #" << order->order_id() << std::endl;
        outputToBuffer(order);
    }

    // terminal
    void on_reject(const OrderPtr &order, const char *reason) override {
        order->onRejected(reason);
        // debug() << "order rejected #" << order->order_id() << std::endl;
        outputToBuffer(order);
    }

    // maybe terminal
    void on_fill(const OrderPtr &order, const OrderPtr &matched_order, Quantity fill_qty, Cost fill_cost) override {
        order->onFilled(fill_qty, fill_cost);
        matched_order->onFilled(fill_qty, fill_cost);
        // debug() << "order filled #" << order->order_id() << " and #"
        //        << matched_order->order_id() << std::endl;
        outputToBuffer(order);
        outputToBuffer(matched_order);
    }

    // terminal
    void on_timeout(const OrderPtr &order) override {
        order->onTimeout();
        // debug() << "order timed out #" << order->order_id() << std::endl;
        outputToBuffer(order);
    }

    // terminal
    void on_cancel(const OrderPtr &order) override {
        order->onCancelled();
        // debug() << "order cancelled #" << order->order_id() << std::endl;
        outputToBuffer(order);
    }

    void on_cancel_reject(const OrderPtr &order, const char *reason) override {
        order->onCancelRejected(reason);
        // debug() << "order cancel rejected #" << order->order_id() <<
        // std::endl;
        outputToBuffer(order);
    }

    void on_replace(const OrderPtr &order, const int64_t &size_delta, Price new_price) override {
        order->onReplaced(size_delta, new_price);
        // debug() << "order replaced #" << order->order_id() << std::endl;
        outputToBuffer(order);
    }

    void on_replace_reject(const OrderPtr &order, const char *reason) override {
        order->onReplaceRejected(reason);
        // debug() << "order replace rejected #" << order->order_id() <<
        // std::endl;
        outputToBuffer(order);
    }

    void on_trade(const OrderBook *book, Quantity qty, Cost cost) override {
        // debug() << "trade " << qty << " " << book->symbol() << " cost " <<
        // cost
        //        << std::endl;
    }

    // make sure msgAsTable = false
    bool append(vector<ConstantSP> &values, INDEX &insertedRows, string &errMsg) override;

    TableSP generateEngineStatTable() override;
    void initEngineStat() override;
    void updateEngineStat() override;

    void outputToBuffer(const OrderPtr &order) {
        order->flatten(bufferedOutput_, config_.pricePrecision_);
        INDEX insertedRows = -1;
        bufferedOutputTable_->append(bufferedOutput_, insertedRows, lastErrMsg_);
    }

    void outputToOutput() {
        auto newTable = createTableFromModel(bufferedOutputTable_);
        vector<ConstantSP> out{bufferedOutputTable_};
        INDEX insertedRows = -1;
        outputTable_->append(out, insertedRows, lastErrMsg_);
        bufferedOutputTable_ = newTable;
    }

    inline std::ostream &debug() {
        return *logFile_;
    }

private:
    const std::string symbol_;
    static Config config_;
    vector<ConstantSP> bufferedOutput_;
    vector<ConstantSP> depthOutput_;
    TableSP bufferedOutputTable_;
    TableSP outputTable_;
    TableSP depthOutputTable_;
    OrderBookPtr book_;
    uint64_t cumMessages_;
    mutable Mutex mutex_;

    std::ostream *logFile_ = &std::cout;
};

using ExchangeSP = SmartPointer<Exchange>;

#endif    // MATCHINGENGINE_EXCHANGE_H
