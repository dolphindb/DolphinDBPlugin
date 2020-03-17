//
// Created by jccai on 19-5-29.
//
#include <gtest/gtest.h>
#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <thread>
#include "depth_order_book.h"
#include "listener.h"
#include "order.h"

#define RAISE EXPECT_TRUE(false)

using namespace std;
using namespace chrono;

using Price = book::Price;
using Quantity = book::Quantity;
using Cost = book::Cost;

using Order = book::Order;
using OrderType = book::OrderType;
using OrderPtr = book::OrderPtr;
using OrderBook = book::OrderBook<OrderPtr>;
using OrderBookPtr = shared_ptr<OrderBook>;
using DepthOrderBook = book::DepthOrderBook<OrderPtr>;
using DepthOrderBookPtr = shared_ptr<DepthOrderBook>;
using BookDepth = book::Depth;

class Callbacks : public book::OrderListener<OrderPtr>,
                  public book::TradeListener<OrderBook>,
                  public book::OrderBookListener<OrderBook>,
                  public book::BboListener<DepthOrderBook>,
                  public book::DepthListener<DepthOrderBook> {
public:
    void on_bbo_change(const DepthOrderBook *book, const BookDepth *depth) override {
        data = "on_bbo_change";
    }
    void on_depth_change(const DepthOrderBook *book, const BookDepth *depth) override {
    }
    void on_order_book_change(const OrderBook *book) override {
    }
    void on_accept(const OrderPtr &order) override {
        order->onAccepted();
    }
    void on_reject(const OrderPtr &order, const char *reason) override {
        order->onRejected(reason);
    }
    void on_fill(const OrderPtr &order, const OrderPtr &matched_order, Quantity fill_qty, Cost fill_cost) override {
        order->onFilled(fill_qty, fill_cost);
        matched_order->onFilled(fill_qty, fill_cost);
    }
    void on_cancel(const OrderPtr &order) override {
        order->onCancelled();
    }
    void on_timeout(const OrderPtr &order) override {
        order->onTimeout();
    }
    void on_cancel_reject(const OrderPtr &order, const char *reason) override {
        order->onCancelRejected(reason);
    }
    void on_replace(const OrderPtr &order, const int64_t &size_delta, Price new_price) override {
        order->onReplaced(size_delta, new_price);
    }
    void on_replace_reject(const OrderPtr &order, const char *reason) override {
        order->onReplaceRejected(reason);
    }
    void on_trade(const OrderBook *book, Quantity qty, Cost cost) override {
    }
    string data;
};

class OrderBookTest : public ::testing::Test {
protected:
    OrderBookTest() {
        book_ = make_shared<DepthOrderBook>("test", 5);
        book_->set_order_book_listener(&cb_);
        book_->set_trade_listener(&cb_);
        book_->set_order_listener(&cb_);
        book_->set_bbo_listener(&cb_);
        book_->set_depth_listener(&cb_);
    }
    ~OrderBookTest() = default;
    OrderPtr makeOrder(map<string, uint64_t> properties);
    uint64_t nextId();

    static uint64_t id;
    const static map<string, uint64_t> limitBid;
    const static map<string, uint64_t> limitAsk;
    const static map<string, uint64_t> marketBid;
    const static map<string, uint64_t> marketAsk;

    DepthOrderBookPtr book_;
    Callbacks cb_;
};

uint64_t OrderBookTest::id = 0;
const map<string, uint64_t> OrderBookTest::limitBid = {
    {"isBuy", 1}, {"quantity", 100}, {"price", 100}, {"threshold", 0}, {"isAON", 0}, {"isIOC", 0}, {"isSL", 0}, {"isTP", 0}, {"isTS", 0}, {"expired", UINT64_MAX},
};
const map<string, uint64_t> OrderBookTest::limitAsk = {
    {"isBuy", 0}, {"quantity", 100}, {"price", 100}, {"threshold", 0}, {"isAON", 0}, {"isIOC", 0}, {"isSL", 0}, {"isTP", 0}, {"isTS", 0}, {"expired", UINT64_MAX},
};
const map<string, uint64_t> OrderBookTest::marketBid = {
    {"isBuy", 1}, {"quantity", 100}, {"price", 0}, {"threshold", 0}, {"isAON", 0}, {"isIOC", 0}, {"isSL", 0}, {"isTP", 0}, {"isTS", 0}, {"expired", UINT64_MAX},
};
const map<string, uint64_t> OrderBookTest::marketAsk = {
    {"isBuy", 0}, {"quantity", 100}, {"price", 0}, {"threshold", 0}, {"isAON", 0}, {"isIOC", 0}, {"isSL", 0}, {"isTP", 0}, {"isTS", 0}, {"expired", UINT64_MAX},
};

OrderPtr OrderBookTest::makeOrder(map<string, uint64_t> properties) {
    return make_shared<Order>("test", nextId(), (Quantity)properties["quantity"],
                              properties["isBuy"] * book::ORDER_BUY + properties["isAON"] * book::ORDER_AON + properties["isIOC"] * book::ORDER_IOC + properties["isSL"] * book::ORDER_SL +
                                  properties["isTP"] * book::ORDER_TP + properties["isTS"] * book::ORDER_TS + properties["isHidden"] * book::ORDER_HIDDEN,
                              (Price)properties["price"], (Price)properties["threshold"], (uint64_t)properties["expired"]);
}

uint64_t OrderBookTest::nextId() {
    return ++id;
}

/////////////////////////////////////
/// Market Test
TEST_F(OrderBookTest, HasInitialMarketPrice) {
    book_->set_market_price(100);
    auto bid = makeOrder(marketBid);
    auto ask = makeOrder(marketAsk);
    book_->add(bid);
    book_->add(ask);
    EXPECT_EQ(book_->bids().size(), 0);
    EXPECT_EQ(bid->fillCost(), 100 * 100);
    EXPECT_EQ(ask->fillCost(), 100 * 100);
}

/////////////////////////////////////
/// Limit Order Test
TEST_F(OrderBookTest, AddLimitOrderToEmptyBook) {
    auto bid = makeOrder(limitBid);
    book_->add(bid);
    EXPECT_EQ(book_->bids().size(), 1);
    EXPECT_EQ(bid->currentState().state_, Order::State::Accepted);
}

TEST_F(OrderBookTest, LimitOrderTimeout) {
    auto timeout = limitAsk;
    timeout["expired"] = static_cast<uint64_t>((duration_cast<milliseconds>(steady_clock::now().time_since_epoch()) + milliseconds(1000)).count());
    auto ask = makeOrder(timeout);
    book_->add(ask);
    EXPECT_EQ(book_->asks().size(), 1);
    this_thread::sleep_for(milliseconds(1100));
    EXPECT_EQ(book_->asks().size(), 1);    // still in the market due to lazy eval
    EXPECT_EQ(ask->currentState().state_, Order::State::Accepted);
    book_->add(makeOrder(marketBid));    // trigger
    EXPECT_EQ(book_->asks().size(), 0);
    EXPECT_EQ(book_->bids().size(), 1);
    EXPECT_EQ(ask->currentState().state_, Order::State::Timeout);
}

TEST_F(OrderBookTest, PartialMatchedLimitOrder) {
    auto partialconfig = limitAsk;
    partialconfig["quantity"] = 50;
    auto partial = makeOrder(partialconfig);
    book_->add(partial);
    auto bid = makeOrder(limitBid);
    book_->add(bid);
    EXPECT_EQ(book_->bids().size(), 1);
    EXPECT_EQ(book_->bids().size(), book_->orders().size());
    EXPECT_EQ(partial->currentState().state_, Order::Filled);
    EXPECT_EQ(bid->currentState().state_, Order::PartialFilled);
    EXPECT_EQ(bid->quantityOnMarket(), 50);
}

TEST_F(OrderBookTest, FullMatchedLimitOrder) {
    auto bid = makeOrder(limitBid);
    auto ask = makeOrder(limitAsk);
    book_->add(bid);
    book_->add(ask);
    EXPECT_EQ(book_->orders().size(), 0);
    EXPECT_EQ(bid->currentState().state_, Order::Filled);
    EXPECT_EQ(ask->currentState().state_, Order::Filled);
}

TEST_F(OrderBookTest, CancelOnMarketLimitOrder) {
    auto bidconfig = limitBid;
    bidconfig["price"] = 50;
    auto bid0 = makeOrder(bidconfig);
    auto bid1 = makeOrder(limitBid);
    auto bid2 = makeOrder(limitBid);
    bidconfig["price"] = 150;
    auto bid3 = makeOrder(bidconfig);
    book_->add(bid0);
    book_->add(bid1);
    book_->add(bid2);
    book_->add(bid3);
    EXPECT_EQ(book_->bids().size(), 4);
    EXPECT_EQ(book_->bids().size(), book_->orders().size());
    book_->cancel(bid1->order_id());
    EXPECT_EQ(book_->bids().size(), 3);
    EXPECT_EQ(book_->bids().size(), book_->orders().size());
    EXPECT_EQ(bid0->currentState().state_, Order::Accepted);
    EXPECT_EQ(bid1->currentState().state_, Order::Cancelled);
    EXPECT_EQ(bid2->currentState().state_, Order::Accepted);
    EXPECT_EQ(bid3->currentState().state_, Order::Accepted);
}

TEST_F(OrderBookTest, CancelCanceledLimitOrder) {
    auto bid = makeOrder(limitBid);
    book_->add(bid);
    book_->cancel(bid->order_id());
    EXPECT_EQ(bid->currentState().state_, Order::Cancelled);
    book_->cancel(bid->order_id());
    EXPECT_EQ(book_->bids().size(), 0);
    EXPECT_EQ(book_->bids().size(), book_->orders().size());
    EXPECT_EQ(bid->currentState().state_, Order::Cancelled);
}

TEST_F(OrderBookTest, CancelFilledLimitOrder) {
    auto bid = makeOrder(limitBid);
    book_->add(bid);
    auto ask = makeOrder(limitAsk);
    book_->add(ask);
    EXPECT_EQ(book_->bids().size(), 0);
    EXPECT_EQ(book_->bids().size(), book_->orders().size());
    EXPECT_EQ(bid->currentState().state_, Order::Filled);
    book_->cancel(bid->order_id());
    EXPECT_EQ(bid->currentState().state_, Order::Filled);    // nothing happens
}

TEST_F(OrderBookTest, ModifyOnMarketLimitOrder) {
    auto bid = makeOrder(limitBid);
    book_->add(bid);
    EXPECT_EQ(bid->currentState().state_, Order::Accepted);
    auto config = limitBid;
    config["quantity"] = 50;
    auto newbid = makeOrder(config);
    newbid->setId(bid->order_id());
    book_->replace(newbid);
    EXPECT_EQ(bid->currentState().state_, Order::Cancelled);
    EXPECT_EQ(newbid->currentState().state_, Order::Accepted);
}

TEST_F(OrderBookTest, ModifyCanceledLimitOrder) {
    auto bid = makeOrder(limitBid);
    book_->add(bid);
    EXPECT_EQ(bid->currentState().state_, Order::Accepted);
    book_->cancel(bid->order_id());
    EXPECT_EQ(bid->currentState().state_, Order::Cancelled);
    auto config = limitBid;
    config["quantity"] = 50;
    auto newbid = makeOrder(config);
    newbid->setId(bid->order_id());
    book_->replace(newbid);
    EXPECT_EQ(bid->currentState().state_, Order::Cancelled);
    EXPECT_EQ(newbid->currentState().state_, Order::ModifyRejected);
}

TEST_F(OrderBookTest, ModifyLimitToMarketOrder) {
    auto askconfig = limitAsk;
    askconfig["price"] = 110;
    auto ask = makeOrder(askconfig);
    auto bid = makeOrder(limitBid);
    book_->add(ask);
    book_->add(bid);
    EXPECT_EQ(bid->currentState().state_, Order::Accepted);
    EXPECT_EQ(book_->asks().size() + book_->bids().size(), book_->orders().size());
    EXPECT_EQ(book_->asks().size(), book_->bids().size());
    auto bidconfig = limitBid;
    bidconfig["price"] = 0;
    auto newbid = makeOrder(bidconfig);
    newbid->setId(bid->order_id());
    book_->replace(newbid);
    EXPECT_EQ(bid->currentState().state_, Order::Cancelled);
    EXPECT_EQ(newbid->currentState().state_, Order::Filled);
    EXPECT_EQ(book_->orders().size(), 0);
    EXPECT_EQ(ask->fillCost(), 100 * 110);
}

/////////////////////////////////////
/// Market Order Test
TEST_F(OrderBookTest, AddMarketOrderToEmptyBook) {
    auto bid = makeOrder(marketBid);
    book_->add(bid);
    EXPECT_EQ(book_->bids().size(), 1);
    EXPECT_EQ(bid->currentState().state_, Order::State::Accepted);
}

TEST_F(OrderBookTest, MarketOrderTimeout) {
    auto timeout = marketAsk;
    timeout["expired"] = static_cast<uint64_t>((duration_cast<milliseconds>(steady_clock::now().time_since_epoch()) + milliseconds(1000)).count());
    auto ask = makeOrder(timeout);
    book_->add(ask);
    EXPECT_EQ(book_->asks().size(), 1);
    this_thread::sleep_for(milliseconds(1100));
    EXPECT_EQ(book_->asks().size(), 1);    // still in the market due to lazy eval
    EXPECT_EQ(ask->currentState().state_, Order::State::Accepted);
    book_->add(makeOrder(limitBid));    // trigger
    EXPECT_EQ(book_->asks().size(), 0);
    EXPECT_EQ(book_->bids().size(), 1);
    EXPECT_EQ(ask->currentState().state_, Order::State::Timeout);
}

TEST_F(OrderBookTest, ModifyMarketToLimitOrder) {
    auto askconfig = marketAsk;
    askconfig["quantity"] = 200;
    auto ask = makeOrder(askconfig);
    book_->add(ask);
    EXPECT_EQ(ask->currentState().state_, Order::State::Accepted);
    auto bidconfig = limitBid;
    auto bid = makeOrder(bidconfig);
    book_->add(bid);
    EXPECT_EQ(ask->currentState().state_, Order::State::PartialFilled);
    askconfig["price"] = 100;
    auto newask = makeOrder(askconfig);
    newask->setId(ask->order_id());
    book_->replace(newask);
    EXPECT_EQ(ask->currentState().state_, Order::State::Cancelled);
    EXPECT_EQ(book_->orders().size(), 1);
    EXPECT_EQ(newask->currentState().state_, Order::State::Accepted);
}

TEST_F(OrderBookTest, PartialMatchedMarketOrder) {
    auto askconfig = marketAsk;
    askconfig["quantity"] = 200;
    auto ask = makeOrder(askconfig);
    book_->add(ask);
    EXPECT_EQ(ask->currentState().state_, Order::State::Accepted);
    auto bidconfig = limitBid;
    auto bid = makeOrder(bidconfig);
    book_->add(bid);
    EXPECT_EQ(ask->currentState().state_, Order::State::PartialFilled);
}

TEST_F(OrderBookTest, FullMatchedMarketOrder) {
    RAISE;
}

TEST_F(OrderBookTest, ModifyOnMarketMarketOrder) {
    RAISE;
}

/////////////////////////////////////
/// ImmediateOrCancel Order Test
TEST_F(OrderBookTest, ImmediateOrCancelTimeout) {
    RAISE;
}

TEST_F(OrderBookTest, PartialMatchedImmediateOrCancel) {
    book_->add(makeOrder(limitBid));
    auto ioc = limitAsk;
    ioc["isIOC"] = 1;
    ioc["quantity"] = 200;
    auto ask = makeOrder(ioc);
    book_->add(ask);
    EXPECT_EQ(book_->asks().size(), 0);
    EXPECT_EQ(ask->fillCost(), 100 * 100);
    EXPECT_EQ(ask->quantityFilled(), 100);
    EXPECT_EQ(ask->quantityOnMarket(), 0);
}

TEST_F(OrderBookTest, FullMatchedImmediateOrCancel) {
    book_->add(makeOrder(limitBid));
    EXPECT_EQ(book_->bids().size(), 1);
    auto ioc = limitAsk;
    ioc["isIOC"] = 1;
    auto ask = makeOrder(ioc);
    book_->add(ask);
    EXPECT_EQ(book_->asks().size(), 0);
    EXPECT_EQ(ask->fillCost(), 100 * 100);
}

TEST_F(OrderBookTest, CanceledImmediateOrCancel) {
    auto ioc = limitAsk;
    ioc["isIOC"] = 1;
    auto ask = makeOrder(ioc);
    book_->add(ask);
    EXPECT_EQ(book_->asks().size(), 0);
    EXPECT_EQ(ask->fillCost(), 0);
    EXPECT_EQ(ask->history().back().state_, Order::State::Cancelled);
}

/////////////////////////////////////
/// AllOrNone Order Test
TEST_F(OrderBookTest, PartialMatchedAllOrNone) {
    auto aonconfig = limitAsk;
    aonconfig["isAON"] = 1;
    auto aonask = makeOrder(aonconfig);
    auto config = limitBid;
    config["quantity"] = 50;
    auto bid = makeOrder(config);
    book_->add(aonask);
    EXPECT_EQ(book_->asks().size(), 1);
    book_->add(bid);
    EXPECT_EQ(book_->asks().size(), book_->bids().size());
    EXPECT_EQ(bid->currentState().state_, Order::State::Accepted);
}

TEST_F(OrderBookTest, FullMatchedAllOrNone) {
    auto aon = limitAsk;
    aon["isAON"] = 1;
    auto ask = makeOrder(aon);
    book_->add(ask);
    EXPECT_EQ(book_->asks().size(), 1);
    book_->add(makeOrder(marketBid));
    EXPECT_EQ(book_->asks().size(), 0);
    EXPECT_EQ(ask->fillCost(), 100 * 100);
}

TEST_F(OrderBookTest, NotMatchedAllOrNone) {
    auto aon = limitAsk;
    aon["isAON"] = 1;
    auto ask = makeOrder(aon);
    book_->add(ask);
    EXPECT_EQ(book_->asks().size(), 1);
    auto partial = marketBid;
    partial["quantity"] = 50;
    book_->add(makeOrder(partial));
    EXPECT_EQ(ask->fillCost(), 0);
    EXPECT_EQ(book_->asks().size(), book_->bids().size());
}

TEST_F(OrderBookTest, MultipleAllOrNone) {
    auto ask = limitAsk;
    ask["quantity"] = 40;
    auto ask1 = makeOrder(ask);
    auto ask2 = makeOrder(ask);
    ask["isAON"] = 1;
    auto askaon = makeOrder(ask);
    auto bid = limitBid;
    bid["isAON"] = 1;
    auto bidaon = makeOrder(bid);
    book_->add(ask1);
    book_->add(ask2);
    book_->add(askaon);
    EXPECT_EQ(book_->asks().size(), 3);
    book_->add(bidaon);
    EXPECT_EQ(book_->asks().size(), 1);
    EXPECT_EQ(book_->orders().size(), 1);
    EXPECT_EQ(bidaon->currentState().state_, Order::Filled);
    EXPECT_EQ(askaon->currentState().state_, Order::Filled);
}

/////////////////////////////////////
/// ImmediateOrCancel AllOrNone Order Test
TEST_F(OrderBookTest, FullMatchedAONIOC) {
    auto aonioc = limitAsk;
    aonioc["isAON"] = 1;
    aonioc["isIOC"] = 1;
    auto ask = makeOrder(aonioc);
    book_->add(ask);
    EXPECT_EQ(book_->orders().size(), 0);
    EXPECT_EQ(ask->currentState().state_, Order::Cancelled);
    auto ask2 = makeOrder(aonioc);
    book_->add(makeOrder(limitBid));
    book_->add(ask2);
    EXPECT_EQ(book_->orders().size(), 0);
    EXPECT_EQ(book_->bids().size(), 0);
    EXPECT_EQ(book_->asks().size(), 0);
    EXPECT_EQ(ask2->currentState().state_, Order::Filled);
}

// HIDDEN Test
TEST_F(OrderBookTest, Hidden) {
    auto hidden_ask = limitAsk;
    hidden_ask["isHidden"] = 1;
    auto ask = makeOrder(hidden_ask);
    book_->add(ask);
    EXPECT_EQ(book_->orders().size(), 1);
    EXPECT_EQ(book_->depth().changed(), false);
    EXPECT_EQ(ask->currentState().state_, Order::Accepted);

    auto hidden_bid = limitBid;
    hidden_bid["isHidden"] = 1;
    auto bid = makeOrder(hidden_bid);
    book_->add(bid);
    EXPECT_EQ(book_->orders().size(), 0);
    EXPECT_EQ(book_->depth().changed(), false);
    EXPECT_EQ(bid->currentState().state_, Order::Filled);
}

TEST_F(OrderBookTest, Hidden_PartialFill) {
    auto hidden_ask = limitAsk;
    hidden_ask["isHidden"] = 1;
    hidden_ask["price"] = 102;
    auto ask = makeOrder(hidden_ask);
    book_->add(ask);
    EXPECT_EQ(book_->orders().size(), 1);
    EXPECT_EQ(book_->depth().changed(), false);
    EXPECT_EQ(ask->currentState().state_, Order::Accepted);

    auto hidden_bid = limitBid;
    hidden_bid["isHidden"] = 1;
    hidden_bid["price"] = 103;
    auto bid = makeOrder(hidden_bid);
    book_->add(bid);
    EXPECT_EQ(book_->orders().size(), 0);
    EXPECT_EQ(book_->depth().changed(), false);
    EXPECT_EQ(bid->currentState().state_, Order::Filled);
}

TEST_F(OrderBookTest, HiddenIOC) {
    auto hidden_ask = limitAsk;
    hidden_ask["isHidden"] = 1;
    hidden_ask["isIOC"] = 1;
    auto ask = makeOrder(hidden_ask);
    book_->add(ask);
    EXPECT_EQ(book_->orders().size(), 0);
    EXPECT_EQ(book_->depth().changed(), false);
    EXPECT_EQ(ask->currentState().state_, Order::Cancelled);

    book_->add(makeOrder(limitAsk));
    auto hidden_bid = limitBid;
    hidden_bid["isHidden"] = 1;
    hidden_bid["isIOC"] = 1;
    hidden_bid["price"] = 99;
    auto bid = makeOrder(hidden_bid);
    book_->add(bid);
    EXPECT_EQ(book_->orders().size(), 1);
    EXPECT_EQ(book_->depth().changed(), false);
    EXPECT_EQ(bid->currentState().state_, Order::Cancelled);
}

TEST_F(OrderBookTest, HiddenIOC_q) {
    auto hidden_ask = limitAsk;
    hidden_ask["isHidden"] = 1;
    hidden_ask["isIOC"] = 1;
    auto ask = makeOrder(hidden_ask);
    book_->add(ask);
    EXPECT_EQ(book_->orders().size(), 0);
    EXPECT_EQ(book_->depth().changed(), false);
    EXPECT_EQ(ask->currentState().state_, Order::Cancelled);

    book_->add(makeOrder(limitAsk));
    auto hidden_bid = limitBid;
    hidden_bid["isHidden"] = 1;
    hidden_bid["isIOC"] = 1;
    hidden_bid["quantity"] = 99;
    auto bid = makeOrder(hidden_bid);
    book_->add(bid);
    EXPECT_EQ(book_->orders().size(), 0);
    EXPECT_EQ(book_->depth().changed(), false);
    EXPECT_EQ(bid->currentState().state_, Order::Cancelled);
}

TEST_F(OrderBookTest, HiddenAON) {
    auto hidden_ask = limitAsk;
    hidden_ask["isHidden"] = 1;
    hidden_ask["isAON"] = 1;
    auto ask = makeOrder(hidden_ask);
    book_->add(ask);
    EXPECT_EQ(book_->orders().size(), 1);
    EXPECT_EQ(book_->depth().changed(), false);
    EXPECT_EQ(ask->currentState().state_, Order::Accepted);

    auto hidden_bid = limitBid;
    hidden_bid["isHidden"] = 1;
    hidden_bid["isAON"] = 1;
    auto bid = makeOrder(hidden_bid);
    book_->add(bid);
    EXPECT_EQ(book_->orders().size(), 0);
    EXPECT_EQ(book_->depth().changed(), false);
    EXPECT_EQ(bid->currentState().state_, Order::Filled);
}

/////////////////////////////////////
/// Stop Loss Order Test
TEST_F(OrderBookTest, StoppedStopLossOrder) {
    RAISE;
}

TEST_F(OrderBookTest, NotStoppedStopLossOrder) {
    RAISE;
}

TEST_F(OrderBookTest, TriggerStopLossOrder) {
    RAISE;
}

/////////////////////////////////////
/// Take Profit Order Test
TEST_F(OrderBookTest, StoppedTakeProfitOrder) {
    RAISE;
}

TEST_F(OrderBookTest, NotStoppedTakeProfitOrder) {
    RAISE;
}

TEST_F(OrderBookTest, TriggerTakeProfitOrder) {
    RAISE;
}