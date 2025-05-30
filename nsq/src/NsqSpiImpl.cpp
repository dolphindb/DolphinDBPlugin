//
// Created by htxu on 11/20/2023.
//

#include "NsqSpiImpl.h"
#include "NsqConnection.h"
#include "ddbplugin/PluginLogger.h"
#include "ddbplugin/PluginLoggerImp.h"


#define SAFE_EXECUTE(...)                               \
    try {                                               \
        __VA_ARGS__                                     \
    } catch (std::exception &e) {                       \
        PLUGIN_LOG_ERR(e.what());                              \
    } catch (...) {                                     \
        PLUGIN_LOG_ERR("An error occurred in ", __FUNCTION__); \
    }

void CHSNsqSpiImpl::OnFrontConnected() {

    try {
        NsqConnection::connectionNotifyL();
        NsqConnection::getInstance()->login(username_, password_);
    } catch (std::exception &e) {
        PLUGIN_LOG_ERR(e.what());
    } catch (...) {
        PLUGIN_LOG_ERR("An error occurred in ", __FUNCTION__);
    }
}

void CHSNsqSpiImpl::OnRspUserLogin(CHSNsqRspUserLoginField *pRspUserLogin, CHSNsqRspInfoField *pRspInfo, int nRequestID,
                                   bool bIsLast) {
    SAFE_EXECUTE(
        if (pRspInfo->ErrorID != 0) {
            // optimization: log
        } else {
            NsqConnection::loginNotifyL();
        }
    )
}

void CHSNsqSpiImpl::OnFrontDisconnected(int nResult) {
    PLUGIN_LOG_WARN(NSQ_PREFIX, __FUNCTION__, " nsq disconnect");
}

void CHSNsqSpiImpl::OnRspSecuDepthMarketDataSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {

    SAFE_EXECUTE(
        if (pRspInfo->ErrorID != 0) {
            throw RuntimeException(NSQ_PREFIX + "subscribe failed."); // optimization: add message from demo
        }
    )
}

void CHSNsqSpiImpl::OnRspSecuDepthMarketDataCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {

    SAFE_EXECUTE(
        if (pRspInfo->ErrorID != 0) {
            throw RuntimeException(NSQ_PREFIX + "unsubscribe failed.");
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

        string marketType;
        if (strcmp(pSecuDepthMarketData->ExchangeID, HS_EI_SSE) == 0) {
            marketType = nsqUtil::SH;
        } else if (strcmp(pSecuDepthMarketData->ExchangeID, HS_EI_SZSE) == 0) {
            marketType = nsqUtil::SZ;
        }
        queues_->pushData(data, marketType);
    )
}

void CHSNsqSpiImpl::OnRspSecuTransactionSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {

    SAFE_EXECUTE(
        if (pRspInfo->ErrorID != 0) {
            throw RuntimeException(NSQ_PREFIX + "subscribe failed.");
        }
    )
}

void CHSNsqSpiImpl::OnRspSecuTransactionCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {

    SAFE_EXECUTE(
        if (pRspInfo->ErrorID != 0) {
            throw RuntimeException(NSQ_PREFIX + "unsubscribe failed.");
        }
    )
}

void CHSNsqSpiImpl::OnRtnSecuTransactionTradeData(CHSNsqSecuTransactionTradeDataField *pSecuTransactionTradeData) {

    SAFE_EXECUTE(
        string marketType;
        if (strcmp(pSecuTransactionTradeData->ExchangeID, HS_EI_SSE) == 0) {
            marketType = nsqUtil::SH;
        } else if (strcmp(pSecuTransactionTradeData->ExchangeID, HS_EI_SZSE) == 0) {
            marketType = nsqUtil::SZ;
        }
        queues_->pushData(pSecuTransactionTradeData, marketType);
    )
}

void CHSNsqSpiImpl::OnRtnSecuTransactionEntrustData(CHSNsqSecuTransactionEntrustDataField *pSecuTransactionEntrustData) {

    SAFE_EXECUTE(
        string marketType;
        if (strcmp(pSecuTransactionEntrustData->ExchangeID, HS_EI_SSE) == 0) {
            marketType = nsqUtil::SH;
        } else if (strcmp(pSecuTransactionEntrustData->ExchangeID, HS_EI_SZSE) == 0) {
            marketType = nsqUtil::SZ;
        }
        queues_->pushData(pSecuTransactionEntrustData, marketType);
    )
}
