#pragma once

#include <string>

enum class nsq_data {
    DepthMarket,
    TransactionTrade,
    TransactionEntrust,
    orderbook
};

// enum for HS_EI_*
// e.g. HS_EI_SSE -> SSE
enum class nsq_market {
    SSE,
    SZSE,
    BJSE,
    SWI,
    TZASE,
    CZCE,
    DCE,
    SHFE,
    CFFEX,
    INE,
    GFEX,
    SSEHK,
    SZSEHK,
    unknown
};

enum class nsq_version {
    origin,
    v220105,
};

inline std::string to_string(nsq_data data) {
    switch (data) {
        case nsq_data::DepthMarket: return "snapshot";
        case nsq_data::TransactionTrade: return "trade";
        case nsq_data::TransactionEntrust: return "orders";
        case nsq_data::orderbook: return "orderTrade";
        default: return "unknown";
    }
}

inline std::string to_string(nsq_market market) {
    switch (market) {
        // for compatibility with nsq demo, use "sh" instead of "SSE"
        case nsq_market::SSE: return "sh";
        case nsq_market::SZSE: return "sz";
        case nsq_market::BJSE: return "bj";
        case nsq_market::SWI: return "swi";
        case nsq_market::TZASE: return "neeq";
        case nsq_market::CZCE: return "czce";
        case nsq_market::DCE: return "dce";
        case nsq_market::SHFE: return "shfe";
        case nsq_market::CFFEX: return "cffex";
        case nsq_market::INE: return "ine";
        case nsq_market::GFEX: return "cfex";
        case nsq_market::SSEHK: return "SSEHK";
        case nsq_market::SZSEHK: return "SZSEHK";
        default: return "unknown";
    }
}

const auto NSQ_PREFIX = "[PLUGIN::NSQ] ";
