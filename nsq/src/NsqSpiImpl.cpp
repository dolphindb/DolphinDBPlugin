//
// Created by htxu on 11/20/2023.
//

#include "NsqSpiImpl.h"
#include <tuple>
#include "NsqConnection.h"
#include "ddb_nsq.h"
#include "ddbplugin/PluginLogger.h"


std::shared_ptr<CHSNsqSpiImpl> g_spi; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

namespace {

inline nsq_market from_exchange_id(const char* exchange_id) {
    if (strcmp(exchange_id, HS_EI_SSE) == 0) {
        return nsq_market::SSE;
    }
    if (strcmp(exchange_id, HS_EI_SZSE) == 0) {
        return nsq_market::SZSE;
    }
    if (strcmp(exchange_id, HS_EI_BJSE) == 0) {
        return nsq_market::BJSE;
    }
    if (strcmp(exchange_id, HS_EI_SWI) == 0) {
        return nsq_market::SWI;
    }
    if (strcmp(exchange_id, HS_EI_TZASE) == 0) {
        return nsq_market::TZASE;
    }
    if (strcmp(exchange_id, HS_EI_CZCE) == 0) {
        return nsq_market::CZCE;
    }
    if (strcmp(exchange_id, HS_EI_DCE) == 0) {
        return nsq_market::DCE;
    }
    if (strcmp(exchange_id, HS_EI_SHFE) == 0) {
        return nsq_market::SHFE;
    }
    if (strcmp(exchange_id, HS_EI_CFFEX) == 0) {
        return nsq_market::CFFEX;
    }
    if (strcmp(exchange_id, HS_EI_INE) == 0) {
        return nsq_market::INE;
    }
    if (strcmp(exchange_id, HS_EI_GFEX) == 0) {
        return nsq_market::GFEX;
    }
    if (strcmp(exchange_id, HS_EI_SSEHK) == 0) {
        return nsq_market::SSEHK;
    }
    if (strcmp(exchange_id, HS_EI_SZSEHK) == 0) {
        return nsq_market::SZSEHK;
    }
    return nsq_market::unknown;
}

} // namespace

#define SAFE_EXECUTE(...)                               \
    try {                                               \
        __VA_ARGS__                                     \
    } catch (std::exception &e) {                       \
        LOG_ERR(e.what());                              \
    } catch (...) {                                     \
        LOG_ERR("An error occurred in ", __FUNCTION__); \
    }

void CHSNsqSpiImpl::OnFrontConnected() {

    try {
        conn_->connectionNotifyL();
        conn_->login(username_, password_);
    } catch (std::exception &e) {
        LOG_ERR(e.what());
    } catch (...) {
        LOG_ERR("An error occurred in ", __FUNCTION__);
    }
}

void CHSNsqSpiImpl::OnRspUserLogin(CHSNsqRspUserLoginField *pRspUserLogin, CHSNsqRspInfoField *pRspInfo, int nRequestID,
                                   bool bIsLast) {
    std::ignore = pRspUserLogin;
    std::ignore = nRequestID;
    std::ignore = bIsLast;
    SAFE_EXECUTE(
        if (pRspInfo->ErrorID != 0) {
            // optimization: log
        } else {
            conn_->loginNotifyL();
        }
    )
}

void CHSNsqSpiImpl::OnFrontDisconnected(int nResult) {
    std::ignore = nResult;
    LOG_WARN(NSQ_PREFIX, __FUNCTION__, " nsq disconnect");
}

void CHSNsqSpiImpl::OnRspSecuDepthMarketDataSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    std::ignore = nRequestID;
    std::ignore = bIsLast;
    SAFE_EXECUTE(
        if (pRspInfo->ErrorID != 0) {
            throw RuntimeException("subscribe failed: " + string(pRspInfo->ErrorMsg)); // optimization: add message from demo
        }
    )
}

void CHSNsqSpiImpl::OnRspSecuDepthMarketDataCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    std::ignore = nRequestID;
    std::ignore = bIsLast;
    SAFE_EXECUTE(
        if (pRspInfo->ErrorID != 0) {
            throw RuntimeException("unsubscribe failed: " + string(pRspInfo->ErrorMsg)); // optimization: add message from demo
        }
    )
}

void CHSNsqSpiImpl::OnRtnSecuDepthMarketData(CHSNsqSecuDepthMarketDataField *pSecuDepthMarketData,
                                             HSIntVolume *Bid1Volume, HSNum Bid1Count, HSNum MaxBid1Count,
                                             HSIntVolume *Ask1Volume, HSNum Ask1Count, HSNum MaxAsk1Count) {

    SAFE_EXECUTE(
        nsqUtil::SnapshotDataStruct data{
                Util::toLocalNanoTimestamp(Util::getNanoEpochTime()),
                *pSecuDepthMarketData,
                vector<HSIntVolume>(Bid1Count),
                Bid1Count,
                MaxBid1Count,
                vector<HSIntVolume>(Ask1Count),
                Ask1Count,
                MaxAsk1Count
        };

        std::memcpy(data.Bid1Volume.data(), Bid1Volume, sizeof(HSIntVolume) * Bid1Count);
        std::memcpy(data.Ask1Volume.data(), Ask1Volume, sizeof(HSIntVolume) * Ask1Count);

        auto exchange = &pSecuDepthMarketData->ExchangeID[0];
        nsq_market market = from_exchange_id(exchange);
        if (market == nsq_market::unknown) {
            LOG_WARN("Unknown ExchangeID: ", exchange);
            return;
        }
        queues_->pushData(data, market);
    )
}

void CHSNsqSpiImpl::OnRspSecuTransactionSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    std::ignore = nRequestID;
    std::ignore = bIsLast;
    SAFE_EXECUTE(
        if (pRspInfo->ErrorID != 0) {
            throw RuntimeException("subscribe failed: " + string(pRspInfo->ErrorMsg)); // optimization: add message from demo
        }
    )
}

void CHSNsqSpiImpl::OnRspSecuTransactionCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    std::ignore = nRequestID;
    std::ignore = bIsLast;
    SAFE_EXECUTE(
        if (pRspInfo->ErrorID != 0) {
            throw RuntimeException("unsubscribe failed: " + string(pRspInfo->ErrorMsg)); // optimization: add message from demo
        }
    )
}

void CHSNsqSpiImpl::OnRtnSecuTransactionTradeData(CHSNsqSecuTransactionTradeDataField *pSecuTransactionTradeData) {

    SAFE_EXECUTE(
        auto exchange = &pSecuTransactionTradeData->ExchangeID[0];
        nsq_market market = from_exchange_id(exchange);
        if (market == nsq_market::unknown) {
            LOG_WARN("Unknown ExchangeID: ", exchange);
            return;
        }
        queues_->pushData(pSecuTransactionTradeData, market);
    )
}

void CHSNsqSpiImpl::OnRtnSecuTransactionEntrustData(CHSNsqSecuTransactionEntrustDataField *pSecuTransactionEntrustData) {

    SAFE_EXECUTE(
        auto exchange = &pSecuTransactionEntrustData->ExchangeID[0];
        nsq_market market = from_exchange_id(exchange);
        if (market == nsq_market::unknown) {
            LOG_WARN("Unknown ExchangeID: ", exchange);
            return;
        }
        queues_->pushData(pSecuTransactionEntrustData, market);
    )
}
