//
// Created by jccai on 19-5-27.
//

#ifndef MATCHINGENGINE_BOOK_ORDER_H_
#define MATCHINGENGINE_BOOK_ORDER_H_

#include <chrono>
#include <memory>
#include <sstream>
#include <vector>
#ifndef TEST
#include <CoreConcept.h>
#endif
#include "types.h"

namespace book {

// SL for stop loss, TP for take profit, TS for trailing stop
//
// ...0    0     0     0     0     0     0     0
//               |     |     |     |     |     |
//             isTS  isTP  isSL  isIOC  isAON  isBuy

const uint32_t ORDER_SEL = 0;
const uint32_t ORDER_BUY = 1;
const uint32_t ORDER_AON = 2;
const uint32_t ORDER_IOC = 4;
const uint32_t ORDER_SL = 8;
const uint32_t ORDER_TP = 16;
const uint32_t ORDER_TS = 32;
const uint32_t ORDER_HIDDEN = 64;

enum OrderType {
    Limit,
    Market,
    StopLossLimit,
    StopLossMarket,
    TrailingStopLossMarket,
    TakeProfitLimit,
    TakeProfitMarket,
};

enum OrderOperation {
    ADD,
    MODIFY,
    CANCEL,
};

class Order {
public:
    enum State {
        Submitted,
        Rejected,    // Terminal state
        Accepted,
        ModifyRequested,
        ModifyRejected,
        Modified,
        PartialFilled,
        Filled,     // Terminal State
        Timeout,    // Terminal State
        CancelRequested,
        CancelRejected,
        Cancelled,    // Terminal state
        Unknown
    };

    static const std::string &getStateName(State state) {
        static const std::string names[] = {"Submitted", "Rejected", "Accepted",        "ModifyRequested", "ModifyRejected", "Modified", "PartialFilled",
                                            "Filled",    "Timeout",  "CancelRequested", "CancelRejected",  "Cancelled",      "Unknown"};
        if (state >= Submitted && state <= Unknown)
            return names[state];
        else
            return names[11];
    }

    struct StateChange {
        State state_;
        std::string description_;

        StateChange() : state_(Unknown) {
        }

        StateChange(State state, const std::string &description = "") : state_(state), description_(description) {
        }

        std::string print() const {
            return "{" + getStateName(state_) + ": " + description_ + "}";
        }
    };

    using History = std::vector<StateChange>;

public:
    // TODO: simplify the construction, use bit fields for isBuy/isAON/isIOC/etc
    explicit Order(std::string &&symbol, uint64_t id, Quantity quantity, uint32_t condition, Price price, Price thresholdPrice = 0, uint64_t expiredTime = 0)
        : symbol_(std::move(symbol)),
          id_(id),
          quantity_(quantity),
          condition_(condition),
          price_(price),
          thresholdPrice_(thresholdPrice),
          expiredTime_(expiredTime),
          quantityFilled_(0),
          quantityOnMarket_(0),
          fillCost_(0),
          history_() {
    }

    uint64_t order_id() const {
        return id_;
    }

    const std::string &symbol() const {
        return symbol_;
    }

    bool is_limit() const {
        return price_;
    }

    bool is_buy() const {
        return condition_ & ORDER_BUY;
    }

    bool all_or_none() const {
        return condition_ & ORDER_AON;
    }

    bool immediate_or_cancel() const {
        return condition_ & ORDER_IOC;
    }

    bool is_hidden() const {
        return condition_ & ORDER_HIDDEN;
    }

    bool stop_loss() const {
        return condition_ & ORDER_SL;
    }

    bool trailing_stop() const {
        return condition_ & ORDER_TS;
    }

    bool take_profit() const {
        return condition_ & ORDER_TP;
    }

    Price price() const {
        return price_;
    }

    Price threshold_price() const {
        return thresholdPrice_;
    }

    Quantity order_qty() const {
        return quantity_;
    }

    uint64_t expired_time() const {
        return expiredTime_;
    }

    bool expired() const {
        // FIXME: use Util::getEpochTime ?
        return expiredTime_ != 0 && expiredTime_ < static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
    }

    Quantity quantityFilled() const {
        return quantityFilled_;
    }

    Quantity quantityOnMarket() const {
        return quantityOnMarket_;
    }

    Cost fillCost() const {
        return fillCost_;
    }

    const History &history() const {
        return history_;
    }

    const StateChange &currentState() const {
        return history_.back();
    }

    // Events
    void onSubmitted() {
        std::stringstream msg;
        msg << (is_buy() ? "BUY " : "SELL ") << quantity_ << " " << symbol_ << " @" << (price() == 0 ? "MKT" : std::to_string(price_));
        history_.emplace_back(Submitted, std::move(msg.str()));
    }

    void onAccepted() {
        quantityOnMarket_ = quantity_;
        history_.emplace_back(Accepted);
    }

    void onRejected(const char *reason) {
        history_.emplace_back(Rejected, reason);
    }

    void onFilled(Quantity fill_qty, Cost fill_cost) {
        quantityOnMarket_ -= fill_qty;
        quantityFilled_ += fill_qty;
        fillCost_ += fill_cost;
        std::stringstream msg;
        msg << fill_qty << " for " << fill_cost;
        if (quantityOnMarket_ == 0) {
            history_.emplace_back(Filled, std::move(msg.str()));
        } else {
            history_.emplace_back(PartialFilled, std::move(msg.str()));
        }
    }

    void onTimeout() {
        quantityOnMarket_ = 0;
        history_.emplace_back(Timeout);
    }

    void onCancelRequested() {
        history_.emplace_back(CancelRequested);
    }

    void onCancelled() {
        quantityOnMarket_ = 0;
        history_.emplace_back(Cancelled);
    }

    void onCancelRejected(const char *reason) {
        history_.emplace_back(CancelRejected, reason);
    }

    void onReplaceRequested(int64_t size_delta, Price new_price) {
        std::stringstream msg;
        if (size_delta != book::SIZE_UNCHANGED)
            msg << "Quantity change: " << size_delta << " ";
        if (new_price != book::PRICE_UNCHANGED)
            msg << "New Price " << new_price;
        history_.emplace_back(ModifyRequested, std::move(msg.str()));
    }

    void onReplaced(int64_t size_delta, Price new_price) {
        std::stringstream msg;
        if (size_delta != book::SIZE_UNCHANGED) {
            quantity_ += size_delta;
            quantityOnMarket_ += size_delta;
            msg << "Quantity change: " << size_delta << " ";
        }
        if (new_price != book::PRICE_UNCHANGED) {
            price_ = new_price;
            msg << "New Price " << new_price;
        }
        history_.emplace_back(Modified, std::move(msg.str()));
    }

    void onReplaceRejected(const char *reason) {
        history_.emplace_back(ModifyRejected, reason);
    }

#ifndef TEST
    void flatten(std::vector<ConstantSP> &row, uint32_t precision) {
        row[0]->setString(symbol_);
        row[1]->setLong(id_);
        row[2]->setString(getStateName(currentState().state_));
        row[3]->setBool(condition_);
        row[4]->setLong(quantity_);
        row[5]->setLong(quantityFilled_);
        row[6]->setDouble(1.0 * fillCost_ / precision);
    }
#else
    void setId(uint64_t id) {
        id_ = id;
    }
#endif

private:
    std::string symbol_;
    uint64_t id_;
    Quantity quantity_;
    uint32_t condition_;
    Price price_;             // 0: Market Order
    Price thresholdPrice_;    // 0: not holding order (used by stop/take/trailing)
    uint64_t expiredTime_;    // 0: always valid

    Quantity quantityFilled_;
    Quantity quantityOnMarket_;
    Cost fillCost_;
    History history_;
};

using OrderPtr = std::shared_ptr<Order>;

inline std::ostream &operator<<(std::ostream &out, const Order &order) {
    out << "[#" << order.order_id();
    out << ' ' << (order.is_buy() ? "BUY" : "SELL");
    out << ' ' << order.order_qty();
    out << ' ' << order.symbol();
    if (order.price() == 0)
        out << " MKT";
    else
        out << " $" << order.price();
    out << (order.all_or_none() ? " AON" : "") << (order.immediate_or_cancel() ? " IOC" : "");
    auto onMarket = order.quantityOnMarket();
    if (onMarket != 0)
        out << " Open: " << onMarket;
    auto filled = order.quantityFilled();
    if (filled != 0)
        out << " Filled: " << filled;
    auto cost = order.fillCost();
    if (cost != 0)
        out << " Cost: " << cost;
    for (auto &event : order.history())
        out << "\n\t" << event.print();
    out << ']';
    return out;
}

}    // namespace book

#endif    // MATCHINGENGINE_BOOK_ORDER_H_
