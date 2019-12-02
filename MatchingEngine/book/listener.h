//
// Created by jccai on 19-5-29.
//

#ifndef MATCHINGENGINE_BOOK_LISTENER_H_
#define MATCHINGENGINE_BOOK_LISTENER_H_

namespace book {

/// @brief generic listener of top-of-book events
template <class OrderBook>
class BboListener {
public:
    /// @brief callback for top of book change
    virtual void on_bbo_change(const OrderBook *book, const typename OrderBook::DepthTracker *depth) = 0;
};

/// @brief listener of depth events.  Implement to build an aggregate depth
/// feed.
template <class OrderBook>
class DepthListener {
public:
    /// @brief callback for change in tracked aggregated depth
    virtual void on_depth_change(const OrderBook *book, const typename OrderBook::DepthTracker *depth) = 0;
};

/// @brief generic listener of order book events
template <class OrderBook>
class OrderBookListener {
public:
    /// @brief callback for change anywhere in order book
    virtual void on_order_book_change(const OrderBook *book) = 0;
};
/// @brief generic listener of order events.  Implement to build a full order book feed.
//    Used by common version of OrderBook::process_callback().
template <typename OrderPtr>
class OrderListener {
public:
    /// @brief callback for an order accept
    virtual void on_accept(const OrderPtr &order) = 0;

    /// @brief callback for an order reject
    virtual void on_reject(const OrderPtr &order, const char *reason) = 0;

    /// @brief callback for an order fill
    /// @param order the inbound order
    /// @param matched_order the matched order
    /// @param fill_qty the quantity of this fill
    /// @param fill_cost the cost of this fill (qty * price)
    virtual void on_fill(const OrderPtr &order, const OrderPtr &matched_order, Quantity fill_qty, Cost fill_cost) = 0;

    /// @brief callback for an order cancellation
    virtual void on_cancel(const OrderPtr &order) = 0;

    /// @brief callback for an order timeout
    virtual void on_timeout(const OrderPtr &order) = 0;

    /// @brief callback for an order cancel rejection
    virtual void on_cancel_reject(const OrderPtr &order, const char *reason) = 0;

    /// @brief callback for an order replace
    /// @param order the replaced order
    /// @param size_delta the change to order quantity
    /// @param new_price the updated order price
    virtual void on_replace(const OrderPtr &order, const int64_t &size_delta, Price new_price) = 0;

    /// @brief callback for an order replace rejection
    virtual void on_replace_reject(const OrderPtr &order, const char *reason) = 0;
};

/// @brief listener of trade events.   Implement to build a trade feed.
template <class OrderBook>
class TradeListener {
public:
    /// @brief callback for a trade
    /// @param book the order book of the fill (not defined whether this is before
    ///      or after fill)
    /// @param qty the quantity of this fill
    /// @param cost the cost of this fill (qty * price)
    virtual void on_trade(const OrderBook *book, Quantity qty, Cost cost) = 0;
};

}    // namespace book

#endif    // MATCHINGENGINE_BOOK_LISTENER_H_
