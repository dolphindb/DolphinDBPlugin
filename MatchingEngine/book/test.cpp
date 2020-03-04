//
// Created by jccai on 19-5-29.
//
#include <iostream>
#include <memory>
#include <map>
#include <thread>
#include <chrono>
#include <gtest/gtest.h>
#include "order.h"
#include "depth_order_book.h"
#include "listener.h"

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

class Callbacks :
  public book::OrderListener<OrderPtr>,
  public book::TradeListener<OrderBook>,
  public book::OrderBookListener<OrderBook>,
  public book::BboListener<DepthOrderBook>,
  public book::DepthListener<DepthOrderBook> {
 public:
  void on_bbo_change(
    const DepthOrderBook *book,
    const BookDepth *depth) override
  {
    data = "on_bbo_change";
  }
  void on_depth_change(
    const DepthOrderBook *book,
    const BookDepth *depth) override
  {

  }
  void on_order_book_change(const OrderBook *book) override
  {

  }
  void on_accept(const OrderPtr &order) override
  {
    order->onAccepted();
  }
  void on_reject(const OrderPtr &order, const char *reason) override
  {
    order->onRejected(reason);
  }
  void on_fill(
    const OrderPtr &order,
    const OrderPtr &matched_order,
    Quantity fill_qty,
    Cost fill_cost) override
  {
    order->onFilled(fill_qty, fill_cost);
    matched_order->onFilled(fill_qty, fill_cost);
  }
  void on_cancel(const OrderPtr &order) override
  {
    order->onCancelled();
  }
  void on_timeout(const OrderPtr &order) override
  {
    order->onTimeout();
  }
  void on_cancel_reject(const OrderPtr &order, const char *reason) override
  {
    order->onCancelRejected(reason);
  }
  void on_replace(
    const OrderPtr &order,
    const int64_t &size_delta,
    Price new_price) override
  {
    order->onReplaced(size_delta, new_price);
  }
  void on_replace_reject(const OrderPtr &order, const char *reason) override
  {
    order->onReplaceRejected(reason);
  }
  void on_trade(const OrderBook *book, Quantity qty, Cost cost) override
  {

  }
  string data;
};

class OrderBookTest : public ::testing::Test {
 protected:
  OrderBookTest()
  {
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
  {"isBuy", 1},
  {"quantity", 100},
  {"price", 100},
  {"threshold", 0},
  {"isAON", 0},
  {"isIOC", 0},
  {"isSL", 0},
  {"isTP", 0},
  {"isTS", 0},
  {"expired", UINT64_MAX},
};
const map<string, uint64_t> OrderBookTest::limitAsk = {
  {"isBuy", 0},
  {"quantity", 100},
  {"price", 100},
  {"threshold", 0},
  {"isAON", 0},
  {"isIOC", 0},
  {"isSL", 0},
  {"isTP", 0},
  {"isTS", 0},
  {"expired", UINT64_MAX},
};
const map<string, uint64_t> OrderBookTest::marketBid = {
  {"isBuy", 1},
  {"quantity", 100},
  {"price", 0},
  {"threshold", 0},
  {"isAON", 0},
  {"isIOC", 0},
  {"isSL", 0},
  {"isTP", 0},
  {"isTS", 0},
  {"expired", UINT64_MAX},
};
const map<string, uint64_t> OrderBookTest::marketAsk = {
  {"isBuy", 0},
  {"quantity", 100},
  {"price", 0},
  {"threshold", 0},
  {"isAON", 0},
  {"isIOC", 0},
  {"isSL", 0},
  {"isTP", 0},
  {"isTS", 0},
  {"expired", UINT64_MAX},
};

OrderPtr OrderBookTest::makeOrder(map<string, uint64_t> properties)
{

  return make_shared<Order>(
    0, // dummy account uid
    "test",
    nextId(),
    (Quantity) properties["quantity"],
    properties["isBuy"] * book::ORDER_BUY +
    properties["isAON"] * book::ORDER_AON +
    properties["isIOC"] * book::ORDER_IOC +
    properties["isSL"] * book::ORDER_SL +
    properties["isTP"] * book::ORDER_TP +
    properties["isTS"] * book::ORDER_TS,
    (Price) properties["price"],
    (Price) properties["threshold"],
    (uint64_t) properties["expired"]);
}

uint64_t OrderBookTest::nextId()
{
  return ++id;
}

/////////////////////////////////////
/// Market Test
TEST_F(OrderBookTest, NoMarketToMarketTradeWithoutMarketPrice)
{
  auto bid = makeOrder(marketBid);
  auto ask = makeOrder(marketAsk);
  book_->add(bid);
  book_->add(ask);
  EXPECT_EQ(book_->orders().size(), 2);
  EXPECT_EQ(book_->bids().size(), 1);
  EXPECT_EQ(book_->asks().size(), 1);
}

TEST_F(OrderBookTest, HasInitialMarketPrice)
{
  book_->set_market_price(100);
  auto bid = makeOrder(marketBid);
  auto ask = makeOrder(marketAsk);
  book_->add(bid);
  book_->add(ask);
  EXPECT_EQ(book_->orders().size(), 0);
  EXPECT_EQ(book_->bids().size(), 0);
  EXPECT_EQ(bid->fillCost(), 100 * 100);
  EXPECT_EQ(ask->fillCost(), 100 * 100);
}

/////////////////////////////////////
/// Limit Order Test
TEST_F(OrderBookTest, AddLimitOrderToEmptyBook)
{
  auto bid = makeOrder(limitBid);
  book_->add(bid);
  EXPECT_EQ(book_->bids().size(), 1);
  EXPECT_EQ(book_->orders().size(), 1);
  EXPECT_EQ(bid->currentState().state_, Order::Accepted);
}

TEST_F(OrderBookTest, AddTimedOutLimitOrder)
{
  auto timeout = limitAsk;
  timeout["expired"] = static_cast<uint64_t>((
    duration_cast<milliseconds>(
      steady_clock::now().time_since_epoch()).count()) - 1000);
  auto ask = makeOrder(timeout);
  auto ret = book_->add(ask);
  EXPECT_EQ(book_->orders().size(), 0);
  EXPECT_EQ(ret, false);
  EXPECT_EQ(book_->asks().size(), 0);
  EXPECT_EQ(ask->currentState().state_, Order::Timeout);
}

TEST_F(OrderBookTest, LimitOrderTimeout)
{
  auto timeout = limitAsk;
  timeout["expired"] = static_cast<uint64_t>((
    duration_cast<milliseconds>(
      steady_clock::now().time_since_epoch()) + milliseconds(50)).count());
  auto ask = makeOrder(timeout);
  book_->add(ask);
  EXPECT_EQ(book_->asks().size(), 1);
  EXPECT_EQ(book_->orders().size(), 1);
  this_thread::sleep_for(milliseconds(100));
  EXPECT_EQ(book_->asks().size(), 1); // still in the market due to lazy eval
  EXPECT_EQ(book_->orders().size(), 1);
  EXPECT_EQ(ask->currentState().state_, Order::Accepted);
  book_->add(makeOrder(marketBid)); // trigger
  EXPECT_EQ(book_->asks().size(), 0);
  EXPECT_EQ(book_->bids().size(), 1);
  EXPECT_EQ(book_->orders().size(), 1);
  EXPECT_EQ(ask->currentState().state_, Order::Timeout);
}

TEST_F(OrderBookTest, PartialMatchedLimitOrder)
{
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

TEST_F(OrderBookTest, FullMatchedLimitOrder)
{
  auto bid = makeOrder(limitBid);
  auto ask = makeOrder(limitAsk);
  book_->add(bid);
  book_->add(ask);
  EXPECT_EQ(book_->orders().size(), 0);
  EXPECT_EQ(bid->currentState().state_, Order::Filled);
  EXPECT_EQ(ask->currentState().state_, Order::Filled);
}

TEST_F(OrderBookTest, CancelOnMarketLimitOrder)
{
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
  EXPECT_EQ(book_->orders().size(), 4);
  EXPECT_EQ(book_->bids().size(), 4);
  EXPECT_EQ(book_->bids().size(), book_->orders().size());
  book_->cancel(bid1->order_id());
  EXPECT_EQ(book_->orders().size(), 3);
  EXPECT_EQ(book_->bids().size(), 3);
  EXPECT_EQ(book_->bids().size(), book_->orders().size());
  EXPECT_EQ(bid0->currentState().state_, Order::Accepted);
  EXPECT_EQ(bid1->currentState().state_, Order::Cancelled);
  EXPECT_EQ(bid2->currentState().state_, Order::Accepted);
  EXPECT_EQ(bid3->currentState().state_, Order::Accepted);
}

TEST_F(OrderBookTest, CancelCanceledLimitOrder)
{
  auto bid = makeOrder(limitBid);
  book_->add(bid);
  EXPECT_EQ(book_->orders().size(), 1);
  book_->cancel(bid->order_id());
  EXPECT_EQ(book_->orders().size(), 0);
  EXPECT_EQ(bid->currentState().state_, Order::Cancelled);
  book_->cancel(bid->order_id());
  EXPECT_EQ(book_->bids().size(), 0);
  EXPECT_EQ(book_->bids().size(), book_->orders().size());
  EXPECT_EQ(bid->currentState().state_, Order::Cancelled);
}

TEST_F(OrderBookTest, CancelFilledLimitOrder)
{
  auto bid = makeOrder(limitBid);
  book_->add(bid);
  auto ask = makeOrder(limitAsk);
  book_->add(ask);
  EXPECT_EQ(book_->bids().size(), 0);
  EXPECT_EQ(book_->bids().size(), book_->orders().size());
  EXPECT_EQ(bid->currentState().state_, Order::Filled);
  book_->cancel(bid->order_id());
  EXPECT_EQ(bid->currentState().state_, Order::Filled); // nothing happens
}

TEST_F(OrderBookTest, ModifyOnMarketLimitOrder)
{
  auto bid = makeOrder(limitBid);
  book_->add(bid);
  EXPECT_EQ(book_->orders().size(), 1);
  EXPECT_EQ(bid->currentState().state_, Order::Accepted);
  auto config = limitBid;
  config["quantity"] = 50;
  auto newbid = makeOrder(config);
  newbid->setId(bid->order_id());
  book_->replace(newbid);
  EXPECT_EQ(book_->orders().size(), 1);
  EXPECT_EQ(bid->currentState().state_, Order::Cancelled);
  EXPECT_EQ(newbid->currentState().state_, Order::Accepted);
}

TEST_F(OrderBookTest, ModifyCanceledLimitOrder)
{
  auto bid = makeOrder(limitBid);
  book_->add(bid);
  EXPECT_EQ(bid->currentState().state_, Order::Accepted);
  book_->cancel(bid->order_id());
  EXPECT_EQ(bid->currentState().state_, Order::Cancelled);
  EXPECT_EQ(book_->orders().size(), 0);
  auto config = limitBid;
  config["quantity"] = 50;
  auto newbid = makeOrder(config);
  newbid->setId(bid->order_id());
  book_->replace(newbid);
  EXPECT_EQ(book_->orders().size(), 0);
  EXPECT_EQ(bid->currentState().state_, Order::Cancelled);
  EXPECT_EQ(newbid->currentState().state_, Order::ModifyRejected);
}

TEST_F(OrderBookTest, ModifyLimitToMarketOrder)
{
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
  EXPECT_EQ(ask->fillCost(), 100*110);
}

/////////////////////////////////////
/// Market Order Test
TEST_F(OrderBookTest, AddMarketOrderToEmptyBook)
{
  auto bid = makeOrder(marketBid);
  book_->add(bid);
  EXPECT_EQ(book_->bids().size(), 1);
  EXPECT_EQ(bid->currentState().state_, Order::Accepted);
}

TEST_F(OrderBookTest, MarketOrderTimeout)
{
  auto timeout = marketAsk;
  timeout["expired"] = static_cast<uint64_t>((
    duration_cast<milliseconds>(
      steady_clock::now().time_since_epoch()) + milliseconds(50)).count());
  auto ask = makeOrder(timeout);
  book_->add(ask);
  EXPECT_EQ(book_->asks().size(), 1);
  this_thread::sleep_for(milliseconds(100));
  EXPECT_EQ(book_->asks().size(), 1); // still in the market due to lazy eval
  EXPECT_EQ(ask->currentState().state_, Order::Accepted);
  book_->add(makeOrder(limitBid)); // trigger
  EXPECT_EQ(book_->asks().size(), 0);
  EXPECT_EQ(book_->bids().size(), 1);
  EXPECT_EQ(ask->currentState().state_, Order::Timeout);
}

TEST_F(OrderBookTest, ModifyMarketToLimitOrder)
{
  auto askconfig = marketAsk;
  askconfig["quantity"] = 200;
  auto ask = makeOrder(askconfig);
  book_->add(ask);
  EXPECT_EQ(ask->currentState().state_, Order::Accepted);
  auto bidconfig = limitBid;
  auto bid = makeOrder(bidconfig);
  book_->add(bid);
  EXPECT_EQ(ask->currentState().state_, Order::PartialFilled);
  askconfig["price"] = 100;
  auto newask = makeOrder(askconfig);
  newask->setId(ask->order_id());
  book_->replace(newask);
  EXPECT_EQ(ask->currentState().state_, Order::Cancelled);
  EXPECT_EQ(book_->orders().size(), 1);
  EXPECT_EQ(newask->currentState().state_, Order::Accepted);
}

TEST_F(OrderBookTest, PartialMatchedMarketOrder)
{
  auto askconfig = marketAsk;
  askconfig["quantity"] = 200;
  auto ask = makeOrder(askconfig);
  book_->add(ask);
  EXPECT_EQ(ask->currentState().state_, Order::Accepted);
  auto bidconfig = limitBid;
  auto bid = makeOrder(bidconfig);
  book_->add(bid);
  EXPECT_EQ(ask->currentState().state_, Order::PartialFilled);
}

TEST_F(OrderBookTest, ModifyOnMarketMarketOrder)
{
  auto bid = makeOrder(marketBid);
  book_->add(bid);
  EXPECT_EQ(book_->orders().size(), 1);
  EXPECT_EQ(bid->currentState().state_, Order::Accepted);
  auto config = marketBid;
  config["quantity"] = 50;
  auto newbid = makeOrder(config);
  newbid->setId(bid->order_id());
  book_->replace(newbid);
  EXPECT_EQ(book_->orders().size(), 1);
  EXPECT_EQ(bid->currentState().state_, Order::Cancelled);
  EXPECT_EQ(newbid->currentState().state_, Order::Accepted);
}

/////////////////////////////////////
/// ImmediateOrCancel Order Test
TEST_F(OrderBookTest, ImmediateOrCancelTimeout)
{
  auto timeout = limitAsk;
  timeout["expired"] = static_cast<uint64_t>((
    duration_cast<milliseconds>(
      steady_clock::now().time_since_epoch()) + milliseconds(50)).count());
  timeout["isIOC"] = 1;
  auto ask = makeOrder(timeout);
  book_->add(ask);
  EXPECT_EQ(book_->asks().size(), 0);
  EXPECT_EQ(ask->currentState().state_, Order::Cancelled);
  this_thread::sleep_for(milliseconds(100));
  EXPECT_EQ(book_->asks().size(), 0);
  EXPECT_EQ(ask->currentState().state_, Order::Cancelled);
}

TEST_F(OrderBookTest, PartialMatchedImmediateOrCancel)
{
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

TEST_F(OrderBookTest, FullMatchedImmediateOrCancel)
{
  book_->add(makeOrder(limitBid));
  EXPECT_EQ(book_->bids().size(), 1);
  auto ioc = limitAsk;
  ioc["isIOC"] = 1;
  auto ask = makeOrder(ioc);
  book_->add(ask);
  EXPECT_EQ(book_->asks().size(), 0);
  EXPECT_EQ(ask->fillCost(), 100 * 100);
}

TEST_F(OrderBookTest, CanceledImmediateOrCancel)
{
  auto ioc = limitAsk;
  ioc["isIOC"] = 1;
  auto ask = makeOrder(ioc);
  book_->add(ask);
  EXPECT_EQ(book_->asks().size(), 0);
  EXPECT_EQ(ask->fillCost(), 0);
  EXPECT_EQ(ask->history().back().state_, Order::Cancelled);
}

/////////////////////////////////////
/// AllOrNone Order Test
TEST_F(OrderBookTest, AllOrNoneTimeout)
{
  auto timeout = limitAsk;
  timeout["expired"] = static_cast<uint64_t>((
    duration_cast<milliseconds>(
      steady_clock::now().time_since_epoch()) + milliseconds(50)).count());
  timeout["isAON"] = 1;
  auto ask = makeOrder(timeout);
  book_->add(ask);
  EXPECT_EQ(book_->asks().size(), 1);
  EXPECT_EQ(ask->currentState().state_, Order::Accepted);
  this_thread::sleep_for(milliseconds(100));
  EXPECT_EQ(ask->currentState().state_, Order::Accepted);
  auto bid = makeOrder(marketBid);
  auto ret = book_->add(bid);
  EXPECT_EQ(ret, false);
  EXPECT_EQ(bid->currentState().state_, Order::Accepted);
  EXPECT_EQ(ask->currentState().state_, Order::Timeout);
  EXPECT_EQ(book_->asks().size(), 0);
  EXPECT_EQ(book_->orders().size(), 1);
  EXPECT_EQ(book_->bids().size(), 1);
}

TEST_F(OrderBookTest, PartialMatchedAllOrNone)
{
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
  EXPECT_EQ(bid->currentState().state_, Order::Accepted);
}

TEST_F(OrderBookTest, FullMatchedAllOrNone)
{
  auto aon = limitAsk;
  aon["isAON"] = 1;
  auto ask = makeOrder(aon);
  book_->add(ask);
  EXPECT_EQ(book_->asks().size(), 1);
  book_->add(makeOrder(marketBid));
  EXPECT_EQ(book_->asks().size(), 0);
  EXPECT_EQ(ask->fillCost(), 100 * 100);
}

TEST_F(OrderBookTest, NotMatchedAllOrNone)
{
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

TEST_F(OrderBookTest, MultipleAllOrNone)
{
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
TEST_F(OrderBookTest, FullMatchedAONIOC)
{
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

/////////////////////////////////////
/// Stop Loss Order Test
TEST_F(OrderBookTest, StoppedStopLossOrderTimeout)
{
  auto config = limitAsk;
  config["isSL"] = 1;
  config["threshold"] = 110;
  config["expired"] = static_cast<uint64_t>((
    duration_cast<milliseconds>(
      steady_clock::now().time_since_epoch()) + milliseconds(50)).count());
  book_->price_change_test(120);
  auto asksl = makeOrder(config);
  book_->add(asksl);
  EXPECT_EQ(asksl->currentState().state_, Order::Accepted);
  EXPECT_EQ(book_->orders().size(), 1);
  EXPECT_EQ(book_->asks().size(), 0); // is stopped
  this_thread::sleep_for(milliseconds(100));
  EXPECT_EQ(book_->orders().size(), 1);
  EXPECT_EQ(book_->asks().size(), 0); // not triggered
  book_->price_change_test(80);
  EXPECT_EQ(book_->orders().size(), 0);
  EXPECT_EQ(book_->asks().size(), 0); // triggered but timed out when price go down
  EXPECT_EQ(asksl->currentState().state_, Order::Timeout);
}

TEST_F(OrderBookTest, StoppedStopLossAskOrder)
{
  auto config = limitAsk;
  config["isSL"] = 1;
  config["threshold"] = 110;
  book_->price_change_test(120);
  auto asksl = makeOrder(config);
  book_->add(asksl);
  EXPECT_EQ(asksl->currentState().state_, Order::Accepted);
  EXPECT_EQ(book_->orders().size(), 1);
  EXPECT_EQ(book_->asks().size(), 0); // is stopped
  book_->price_change_test(130);
  EXPECT_EQ(book_->asks().size(), 0); // not triggered when price go up
  book_->price_change_test(80);
  EXPECT_EQ(book_->asks().size(), 1); // triggered when price go down
  EXPECT_EQ(asksl->currentState().state_, Order::Accepted);
}

TEST_F(OrderBookTest, StoppedStopLossBidOrder)
{
  auto config = limitBid;
  config["isSL"] = 1;
  config["threshold"] = 110;
  book_->price_change_test(90);
  auto bidsl = makeOrder(config);
  book_->add(bidsl);
  EXPECT_EQ(bidsl->currentState().state_, Order::Accepted);
  EXPECT_EQ(book_->orders().size(), 1);
  EXPECT_EQ(book_->bids().size(), 0); // is stopped
  book_->price_change_test(80);
  EXPECT_EQ(book_->bids().size(), 0); // not triggered when price go down
  book_->price_change_test(130);
  EXPECT_EQ(book_->bids().size(), 1); // triggered when price go up
  EXPECT_EQ(bidsl->currentState().state_, Order::Accepted);
}

TEST_F(OrderBookTest, NotStoppedStopLossAskOrder)
{
  auto config = limitAsk;
  config["isSL"] = 1;
  config["threshold"] = 110;
  book_->price_change_test(80);
  auto asksl = makeOrder(config);
  book_->add(asksl);
  EXPECT_EQ(asksl->currentState().state_, Order::Accepted);
  EXPECT_EQ(book_->orders().size(), 1);
  EXPECT_EQ(book_->asks().size(), 1); // is not stopped
  book_->price_change_test(130);
  EXPECT_EQ(book_->asks().size(), 1); // nothing changed
  book_->price_change_test(80);
  EXPECT_EQ(book_->asks().size(), 1); // nothing changed
}

TEST_F(OrderBookTest, NotStoppedStopLossBidOrder)
{
  auto config = limitBid;
  config["isSL"] = 1;
  config["threshold"] = 110;
  book_->price_change_test(130);
  auto bidsl = makeOrder(config);
  book_->add(bidsl);
  EXPECT_EQ(bidsl->currentState().state_, Order::Accepted);
  EXPECT_EQ(book_->orders().size(), 1);
  EXPECT_EQ(book_->bids().size(), 1); // is not stopped
  book_->price_change_test(80);
  EXPECT_EQ(book_->bids().size(), 1); // nothing changed
  book_->price_change_test(130);
  EXPECT_EQ(book_->bids().size(), 1); // nothing changed
}

/////////////////////////////////////
/// Trailing Stop Loss Order Test
TEST_F(OrderBookTest, TrailingStopLossOrderTimeout)
{
  auto config = limitAsk;
  config["isTS"] = 1;
  config["threshold"] = 5;
  config["expired"] = static_cast<uint64_t>((
    duration_cast<milliseconds>(
      steady_clock::now().time_since_epoch()) + milliseconds(50)).count());
  book_->price_change_test(120);
  auto asksl = makeOrder(config);
  book_->add(asksl);
  EXPECT_EQ(asksl->currentState().state_, Order::Accepted);
  EXPECT_EQ(book_->orders().size(), 1);
  EXPECT_EQ(book_->asks().size(), 0); // is stopped
  this_thread::sleep_for(milliseconds(100));
  EXPECT_EQ(book_->orders().size(), 1);
  EXPECT_EQ(book_->asks().size(), 0); // not triggered
  book_->price_change_test(80);
  EXPECT_EQ(book_->orders().size(), 0);
  EXPECT_EQ(book_->asks().size(), 0); // triggered but timed out when price go down
  EXPECT_EQ(asksl->currentState().state_, Order::Timeout);
}

TEST_F(OrderBookTest, TrailingStopLossAskOrder)
{
  auto config = limitAsk;
  config["isTS"] = 1;
  config["threshold"] = 5;
  book_->price_change_test(100);
  auto trail = makeOrder(config);
  book_->add(trail);
  EXPECT_EQ(trail->currentState().state_, Order::Accepted);
  EXPECT_EQ(book_->asks().size(), 0);
  EXPECT_EQ(book_->orders().size(), 1);
  book_->price_change_test(97); // 100-97<5, not triggered
  EXPECT_EQ(book_->asks().size(), 0);
  EXPECT_EQ(book_->orders().size(), 1);
  book_->price_change_test(108); // price go up, not triggered
  EXPECT_EQ(book_->asks().size(), 0);
  EXPECT_EQ(book_->orders().size(), 1);
  book_->price_change_test(100); // 108-100>5, triggered
  EXPECT_EQ(book_->asks().size(), 1);
  EXPECT_EQ(book_->orders().size(), 1);
}

TEST_F(OrderBookTest, TrailingStopLossBidOrder)
{
  auto config = limitBid;
  config["isTS"] = 1;
  config["threshold"] = 5;
  book_->price_change_test(100);
  auto trail = makeOrder(config);
  book_->add(trail);
  EXPECT_EQ(trail->currentState().state_, Order::Accepted);
  EXPECT_EQ(book_->bids().size(), 0);
  EXPECT_EQ(book_->orders().size(), 1);
  book_->price_change_test(102); // 102-100<5, not triggered
  EXPECT_EQ(book_->bids().size(), 0);
  EXPECT_EQ(book_->orders().size(), 1);
  book_->price_change_test(90); // price go down, not triggered
  EXPECT_EQ(book_->bids().size(), 0);
  EXPECT_EQ(book_->orders().size(), 1);
  book_->price_change_test(96); // 96-90>5, triggered
  EXPECT_EQ(book_->bids().size(), 1);
  EXPECT_EQ(book_->orders().size(), 1);
}

/////////////////////////////////////
/// Take Profit Order Test
TEST_F(OrderBookTest, StoppedTakeProfitOrderTimeout)
{
  auto config = limitAsk;
  config["isTP"] = 1;
  config["threshold"] = 110;
  config["expired"] = static_cast<uint64_t>((
    duration_cast<milliseconds>(
      steady_clock::now().time_since_epoch()) + milliseconds(50)).count());
  book_->price_change_test(80);
  auto asksl = makeOrder(config);
  book_->add(asksl);
  EXPECT_EQ(asksl->currentState().state_, Order::Accepted);
  EXPECT_EQ(book_->orders().size(), 1);
  EXPECT_EQ(book_->asks().size(), 0); // is stopped
  this_thread::sleep_for(milliseconds(100));
  EXPECT_EQ(book_->orders().size(), 1);
  EXPECT_EQ(book_->asks().size(), 0); // not triggered
  book_->price_change_test(120);
  EXPECT_EQ(book_->orders().size(), 0);
  EXPECT_EQ(book_->asks().size(), 0); // triggered but timed out when price go down
  EXPECT_EQ(asksl->currentState().state_, Order::Timeout);
}

TEST_F(OrderBookTest, StoppedTakeProfitAskOrder)
{
  auto config = limitAsk;
  config["isTP"] = 1;
  config["threshold"] = 110;
  book_->price_change_test(90);
  auto asktp = makeOrder(config);
  book_->add(asktp);
  EXPECT_EQ(asktp->currentState().state_, Order::Accepted);
  EXPECT_EQ(book_->orders().size(), 1);
  EXPECT_EQ(book_->asks().size(), 0); // is stopped
  book_->price_change_test(80);
  EXPECT_EQ(book_->asks().size(), 0); // not triggered when price go down
  book_->price_change_test(130);
  EXPECT_EQ(book_->asks().size(), 1); // triggered when price go up
}

TEST_F(OrderBookTest, StoppedTakeProfitBidOrder)
{
  auto config = limitBid;
  config["isTP"] = 1;
  config["threshold"] = 90;
  book_->price_change_test(110);
  auto bidtp = makeOrder(config);
  book_->add(bidtp);
  EXPECT_EQ(bidtp->currentState().state_, Order::Accepted);
  EXPECT_EQ(book_->orders().size(), 1);
  EXPECT_EQ(book_->bids().size(), 0); // is stopped
  book_->price_change_test(130);
  EXPECT_EQ(book_->bids().size(), 0); // not triggered when price go up
  book_->price_change_test(80);
  EXPECT_EQ(book_->bids().size(), 1); // triggered when price go down
}

TEST_F(OrderBookTest, NotStoppedTakeProfitAskOrder)
{
  auto config = limitAsk;
  config["isTP"] = 1;
  config["threshold"] = 110;
  book_->price_change_test(130);
  auto asksl = makeOrder(config);
  book_->add(asksl);
  EXPECT_EQ(asksl->currentState().state_, Order::Accepted);
  EXPECT_EQ(book_->orders().size(), 1);
  EXPECT_EQ(book_->asks().size(), 1); // is not stopped
  book_->price_change_test(80);
  EXPECT_EQ(book_->asks().size(), 1); // nothing changed
  book_->price_change_test(130);
  EXPECT_EQ(book_->asks().size(), 1); // nothing changed
}

TEST_F(OrderBookTest, NotStoppedTakeProfitBidOrder)
{
  auto config = limitBid;
  config["isTP"] = 1;
  config["threshold"] = 110;
  book_->price_change_test(80);
  auto bidsl = makeOrder(config);
  book_->add(bidsl);
  EXPECT_EQ(bidsl->currentState().state_, Order::Accepted);
  EXPECT_EQ(book_->orders().size(), 1);
  EXPECT_EQ(book_->bids().size(), 1); // is not stopped
  book_->price_change_test(130);
  EXPECT_EQ(book_->bids().size(), 1); // nothing changed
  book_->price_change_test(80);
  EXPECT_EQ(book_->bids().size(), 1); // nothing changed
}

/////////////////////////////////////
/// Level Test
TEST_F(OrderBookTest, Level)
{
  auto &bidlevel1 = book_->depth().bids()[0];
  EXPECT_EQ(bidlevel1.aggregate_qty(), 0);
  EXPECT_EQ(bidlevel1.order_count(), 0);
  EXPECT_EQ(bidlevel1.price(), 0); // initial price = 0
  auto config = limitBid;
  auto bid = makeOrder(config);
  book_->add(bid);
  EXPECT_EQ(bid->currentState().state_, Order::Accepted);
  EXPECT_EQ(bidlevel1.aggregate_qty(), 100);
  EXPECT_EQ(bidlevel1.order_count(), 1);
  EXPECT_EQ(bidlevel1.price(), 100);
  EXPECT_FALSE(bidlevel1.is_excess());
  auto yabid = makeOrder(config);
  book_->add(yabid);
  EXPECT_EQ(yabid->currentState().state_, Order::Accepted);
  EXPECT_EQ(bidlevel1.aggregate_qty(), 200);
  EXPECT_EQ(bidlevel1.order_count(), 2);
  EXPECT_EQ(bidlevel1.price(), 100);
  EXPECT_FALSE(bidlevel1.is_excess());
}

TEST_F(OrderBookTest, LevelChange)
{
  auto &bidlevel1 = book_->depth().bids()[0];
  auto &bidlevel2 = book_->depth().bids()[1];
  auto config = limitBid;
  auto bid1 = makeOrder(config);
  config["price"] = 99;
  auto bid2 = makeOrder(config);
  book_->add(bid1);
  EXPECT_EQ(bidlevel1.price(), 100);
  EXPECT_EQ(bidlevel1.aggregate_qty(), 100);
  EXPECT_EQ(bidlevel2.price(), 0);
  EXPECT_EQ(bidlevel2.aggregate_qty(), 0);
  book_->add(bid2);
  EXPECT_EQ(bidlevel1.price(), 100);
  EXPECT_EQ(bidlevel1.aggregate_qty(), 100);
  EXPECT_EQ(bidlevel2.price(), 99);
  EXPECT_EQ(bidlevel2.aggregate_qty(), 100);
}

TEST_F(OrderBookTest, ExcessLevel)
{
  auto &bidlevel1 = book_->depth().bids()[0];
  auto &bidlevel2 = book_->depth().bids()[1];
  auto &bidlevel3 = book_->depth().bids()[2];
  auto &bidlevel4 = book_->depth().bids()[3];
  auto &bidlevel5 = book_->depth().bids()[4];
  auto config = limitBid;
  config["price"] = 101;
  auto bid1 = makeOrder(config);
  config["price"] = 102;
  auto bid2 = makeOrder(config);
  config["price"] = 103;
  auto bid3 = makeOrder(config);
  config["price"] = 104;
  auto bid4 = makeOrder(config);
  config["price"] = 105;
  auto bid5 = makeOrder(config);
  config["price"] = 106;
  auto bid6 = makeOrder(config);
  book_->add(bid1);
  book_->add(bid2);
  book_->add(bid3);
  book_->add(bid4);
  book_->add(bid5);
  EXPECT_EQ(bidlevel1.price(), 105);
  EXPECT_EQ(bidlevel1.aggregate_qty(), 100);
  EXPECT_EQ(bidlevel2.price(), 104);
  EXPECT_EQ(bidlevel2.aggregate_qty(), 100);
  EXPECT_EQ(bidlevel3.price(), 103);
  EXPECT_EQ(bidlevel3.aggregate_qty(), 100);
  EXPECT_EQ(bidlevel4.price(), 102);
  EXPECT_EQ(bidlevel4.aggregate_qty(), 100);
  EXPECT_EQ(bidlevel5.price(), 101);
  EXPECT_EQ(bidlevel5.aggregate_qty(), 100);
  EXPECT_FALSE(bidlevel5.is_excess());
  book_->add(bid6);
  EXPECT_EQ(bidlevel1.price(), 106);
  EXPECT_EQ(bidlevel1.aggregate_qty(), 100);
  EXPECT_EQ(bidlevel2.price(), 105);
  EXPECT_EQ(bidlevel2.aggregate_qty(), 100);
  EXPECT_EQ(bidlevel3.price(), 104);
  EXPECT_EQ(bidlevel3.aggregate_qty(), 100);
  EXPECT_EQ(bidlevel4.price(), 103);
  EXPECT_EQ(bidlevel4.aggregate_qty(), 100);
  EXPECT_EQ(bidlevel5.price(), 102);
  EXPECT_EQ(bidlevel5.aggregate_qty(), 100);
  EXPECT_FALSE(bidlevel5.is_excess());
  auto config2 = marketAsk;
  auto ask1 = makeOrder(config2);
  book_->add(ask1);
  EXPECT_EQ(bidlevel1.price(), 105);
  EXPECT_EQ(bidlevel1.aggregate_qty(), 100);
  EXPECT_EQ(bidlevel2.price(), 104);
  EXPECT_EQ(bidlevel2.aggregate_qty(), 100);
  EXPECT_EQ(bidlevel3.price(), 103);
  EXPECT_EQ(bidlevel3.aggregate_qty(), 100);
  EXPECT_EQ(bidlevel4.price(), 102);
  EXPECT_EQ(bidlevel4.aggregate_qty(), 100);
  EXPECT_EQ(bidlevel5.price(), 101);
  EXPECT_EQ(bidlevel5.aggregate_qty(), 100);
  EXPECT_FALSE(bidlevel5.is_excess());
}