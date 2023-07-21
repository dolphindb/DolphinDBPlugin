#include <cstddef>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#ifndef _WIN32
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#else
#if _MSC_VER
#define snprintf _snprintf
#endif
#define PRId64 "lld"
#endif

#include "HSNsqSpiImpl.h"

enum nsqType {
    // 0 represents a snapshot.
    // 1 represents trade.
    // 2 represents orders.
    // 3 represents a snapshot of all fields.
    // 4 represents a combination of orders and trades.
    nsqSnapshot,
    nsqTrade,
    nsqOrder,
    nsqSnapshotOfAll,
    nsqOrderTrade
};

void setStatus(Status &stat, const std::string &errMsg)
{
    stat.processedMsgCount++;
    if (!errMsg.empty())
    {
        stat.lastErrMsg = errMsg;
        std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp =
            std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
        auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
        stat.lastFailedTimestamp = Timestamp(tmp.count());
        stat.failedMsgCount += 1;
    }
}

void resetStatus(int id)
{
    SUBSCRIBE_STATUS[id] = 0;

    STATUS[id].processedMsgCount = 0;
    STATUS[id].failedMsgCount = 0;
    STATUS[id].lastErrMsg = "";
    STATUS[id].lastFailedTimestamp = Timestamp(LLONG_MIN);
}

void CHSNsqSpiImpl::OnFrontConnected()
{
    LOG_INFO(NSQ_PREFIX + " " + "nsq: OnFrontConnected\n");
    isConnected = true;
}

void CHSNsqSpiImpl::OnFrontDisconnected(int nResult)
{
    LOG_INFO(NSQ_PREFIX + " " + "OnFrontDisconnected: nResult %d\n", nResult);
    // After the disconnection, it is possible to reconnect.
    // After successfully reconnecting, it is necessary to send a new subscription request to the server.
    LOG_INFO(NSQ_PREFIX + " " + "nsq: OnFrontDisconnected\n");
    isConnected = false;
}

void CHSNsqSpiImpl::OnRspUserLogin(CHSNsqRspUserLoginField *pRspUserLogin, CHSNsqRspInfoField *pRspInfo, int nRequestID,
                                   bool bIsLast)
{
    LOG_INFO(NSQ_PREFIX + " " + "OnRspUserLogin errno=", pRspInfo->ErrorID, " errinfo=", pRspInfo->ErrorMsg);
    if (0 == pRspInfo->ErrorID)
        isLogin = true;
}

// snapshot
void CHSNsqSpiImpl::OnRspSecuDepthMarketDataSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (nRequestID == 10001)
    {
        SUBSCRIBE_STATUS[0] = 1;
    }
    else if (nRequestID == 10002)
    {
        SUBSCRIBE_STATUS[1] = 1;
    }
    if (pRspInfo->ErrorID != 0)
    {
        throw RuntimeException(NSQ_PREFIX + " " + pRspInfo->ErrorMsg);
    }
    LOG_INFO(NSQ_PREFIX + " " + "OnRspSecuDepthMarketDataSubscribe: nRequestID[", nRequestID, "], ErrorID[", pRspInfo->ErrorID,
             "], ErrorMsg: ", pRspInfo->ErrorMsg);
}

// snapshot
void CHSNsqSpiImpl::OnRspSecuDepthMarketDataCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    // After successfully unsubscribing, it is necessary to set the table back
    LOG_INFO(NSQ_PREFIX + " " + "unsubscribe success: nRequestID = ", nRequestID);
    if (nRequestID == 10001)
    {
        resetStatus(0);
    }
    else if (nRequestID == 10002)
    {
        resetStatus(1);
    }
    if (pRspInfo->ErrorID != 0)
    {
        throw RuntimeException(NSQ_PREFIX + " " + pRspInfo->ErrorMsg);
    }
    LOG_INFO(NSQ_PREFIX + " " + "OnRspSecuDepthMarketDataCancel: nRequestID[", nRequestID, "], ErrorID[", pRspInfo->ErrorID,
             "], ErrorMsg: ", pRspInfo->ErrorMsg);
}

vector<ConstantSP> createVectors(int flag, int size)
{
    // 0 represents a snapshot.
    // 1 represents trade.
    // 2 represents orders.
    // 3 represents a snapshot of all fields.
    // 4 represents a combination of orders and trades.
    vector<ConstantSP> columns;
    switch(flag) {
        case nsqSnapshot:
            columns.resize(SNAPSHOT_TYPES.size());
            for (size_t i = 0; i < SNAPSHOT_TYPES.size(); i++)
            {
                columns[i] = Util::createVector(SNAPSHOT_TYPES[i], size, size);
            }
            break;
        case nsqTrade:
            columns.resize(TRADE_TYPES.size());
            for (size_t i = 0; i < TRADE_TYPES.size(); i++)
            {
                columns[i] = Util::createVector(TRADE_TYPES[i], size, size);
            }
            break;
        case nsqOrder:
            columns.resize(ORDER_TYPES.size());
            for (size_t i = 0; i < ORDER_TYPES.size(); i++)
            {
                columns[i] = Util::createVector(ORDER_TYPES[i], size, size);
            }
            break;
        case nsqSnapshotOfAll:
        {
            vector<DATA_TYPE> colTypes = SNAPSHOT_TYPES;
            colTypes.insert(colTypes.end(), ADDED_SNAPSHOT_TYPES.begin(), ADDED_SNAPSHOT_TYPES.end());
            columns.resize(colTypes.size());
            for (size_t i = 0; i < colTypes.size(); i++)
            {
                columns[i] = Util::createVector(colTypes[i], size, size);
            }
        }
            break;
        case nsqOrderTrade:
            columns.resize(TRADE_AND_ORDER_MERGED_TYPES.size());
            for (size_t i = 0; i < TRADE_AND_ORDER_MERGED_TYPES.size(); i++)
            {
                columns[i] = Util::createVector(TRADE_AND_ORDER_MERGED_TYPES[i], size, size);
            }
            break;
        default:
            throw RuntimeException(NSQ_PREFIX + " flag error");
    }
    return columns;
}

Date *getNewDate(int ymd)
{
    int year, month, day;
    year = ymd / 10000;
    month = ymd % 10000 / 100;
    day = ymd % 100;
    return new Date(year, month, day);
}

Time *getNewTime(int time)
{
    int hour, minute, second, ms;
    hour = time / 10000000;
    minute = time % 10000000 / 100000;
    second = time % 100000 / 1000;
    ms = time % 1000;
    return new Time(hour, minute, second, ms);
}

void setMultSnapshotData(vector<ConstantSP> &columns, std::vector<SnapData *> &items)
{

    HSIntVolume Bid1Volume[50];
    HSIntVolume Ask1Volume[50];
    for (size_t r = 0; r < items.size(); r++)
    {
        CHSNsqSecuDepthMarketDataField *pSecuDepthMarketData = items[r]->pSecuDepthMarketData;
        long long receivedTime = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
        HSNum Bid1Count = items[r]->Bid1Count;
        HSNum Ask1Count = items[r]->Ask1Count;
        std::memcpy(Bid1Volume, items[r]->Bid1Volume, sizeof(HSIntVolume) * Bid1Count);
        std::memcpy(Ask1Volume, items[r]->Ask1Volume, sizeof(HSIntVolume) * Ask1Count);
        HSNum MaxBid1Count = items[r]->MaxBid1Count;
        HSNum MaxAsk1Count = items[r]->MaxAsk1Count;
        assert(Bid1Count <= 50);
        assert(Ask1Count <= 50);
        columns[0]->setString(r, pSecuDepthMarketData->ExchangeID);
        columns[1]->setString(r, pSecuDepthMarketData->InstrumentID);
        columns[2]->setDouble(r, pSecuDepthMarketData->LastPrice);
        columns[3]->setDouble(r, pSecuDepthMarketData->PreClosePrice);
        columns[4]->setDouble(r, pSecuDepthMarketData->OpenPrice);
        columns[5]->setDouble(r, pSecuDepthMarketData->HighPrice);
        columns[6]->setDouble(r, pSecuDepthMarketData->LowPrice);
        columns[7]->setDouble(r, pSecuDepthMarketData->ClosePrice);
        columns[8]->setDouble(r, pSecuDepthMarketData->UpperLimitPrice);
        columns[9]->setDouble(r, pSecuDepthMarketData->LowerLimitPrice);
        // TradeDate is DATE, YYYYMMDD
        columns[10]->set(r, getNewDate(pSecuDepthMarketData->TradeDate));
        // UpdateTime is TIME, HHMMSSsss
        columns[11]->set(r, getNewTime(pSecuDepthMarketData->UpdateTime));
        columns[12]->setLong(r, pSecuDepthMarketData->TradeVolume);
        columns[13]->setDouble(r, pSecuDepthMarketData->TradeBalance);
        columns[14]->setDouble(r, pSecuDepthMarketData->AveragePrice);
        columns[15]->setDouble(r, pSecuDepthMarketData->BidPrice[0]);
        columns[16]->setDouble(r, pSecuDepthMarketData->BidPrice[1]);
        columns[17]->setDouble(r, pSecuDepthMarketData->BidPrice[2]);
        columns[18]->setDouble(r, pSecuDepthMarketData->BidPrice[3]);
        columns[19]->setDouble(r, pSecuDepthMarketData->BidPrice[4]);
        columns[20]->setDouble(r, pSecuDepthMarketData->BidPrice[5]);
        columns[21]->setDouble(r, pSecuDepthMarketData->BidPrice[6]);
        columns[22]->setDouble(r, pSecuDepthMarketData->BidPrice[7]);
        columns[23]->setDouble(r, pSecuDepthMarketData->BidPrice[8]);
        columns[24]->setDouble(r, pSecuDepthMarketData->BidPrice[9]);
        columns[25]->setDouble(r, pSecuDepthMarketData->AskPrice[0]);
        columns[26]->setDouble(r, pSecuDepthMarketData->AskPrice[1]);
        columns[27]->setDouble(r, pSecuDepthMarketData->AskPrice[2]);
        columns[28]->setDouble(r, pSecuDepthMarketData->AskPrice[3]);
        columns[29]->setDouble(r, pSecuDepthMarketData->AskPrice[4]);
        columns[30]->setDouble(r, pSecuDepthMarketData->AskPrice[5]);
        columns[31]->setDouble(r, pSecuDepthMarketData->AskPrice[6]);
        columns[32]->setDouble(r, pSecuDepthMarketData->AskPrice[7]);
        columns[33]->setDouble(r, pSecuDepthMarketData->AskPrice[8]);
        columns[34]->setDouble(r, pSecuDepthMarketData->AskPrice[9]);
        columns[35]->setLong(r, pSecuDepthMarketData->BidVolume[0]);
        columns[36]->setLong(r, pSecuDepthMarketData->BidVolume[1]);
        columns[37]->setLong(r, pSecuDepthMarketData->BidVolume[2]);
        columns[38]->setLong(r, pSecuDepthMarketData->BidVolume[3]);
        columns[39]->setLong(r, pSecuDepthMarketData->BidVolume[4]);
        columns[40]->setLong(r, pSecuDepthMarketData->BidVolume[5]);
        columns[41]->setLong(r, pSecuDepthMarketData->BidVolume[6]);
        columns[42]->setLong(r, pSecuDepthMarketData->BidVolume[7]);
        columns[43]->setLong(r, pSecuDepthMarketData->BidVolume[8]);
        columns[44]->setLong(r, pSecuDepthMarketData->BidVolume[9]);
        columns[45]->setLong(r, pSecuDepthMarketData->AskVolume[0]);
        columns[46]->setLong(r, pSecuDepthMarketData->AskVolume[1]);
        columns[47]->setLong(r, pSecuDepthMarketData->AskVolume[2]);
        columns[48]->setLong(r, pSecuDepthMarketData->AskVolume[3]);
        columns[49]->setLong(r, pSecuDepthMarketData->AskVolume[4]);
        columns[50]->setLong(r, pSecuDepthMarketData->AskVolume[5]);
        columns[51]->setLong(r, pSecuDepthMarketData->AskVolume[6]);
        columns[52]->setLong(r, pSecuDepthMarketData->AskVolume[7]);
        columns[53]->setLong(r, pSecuDepthMarketData->AskVolume[8]);
        columns[54]->setLong(r, pSecuDepthMarketData->AskVolume[9]);
        columns[55]->setLong(r, pSecuDepthMarketData->TradesNum);
        columns[56]->setChar(r, pSecuDepthMarketData->InstrumentTradeStatus);
        columns[57]->setLong(r, pSecuDepthMarketData->TotalBidVolume);
        columns[58]->setLong(r, pSecuDepthMarketData->TotalAskVolume);
        columns[59]->setDouble(r, pSecuDepthMarketData->MaBidPrice);
        columns[60]->setDouble(r, pSecuDepthMarketData->MaAskPrice);
        columns[61]->setDouble(r, pSecuDepthMarketData->MaBondBidPrice);
        columns[62]->setDouble(r, pSecuDepthMarketData->MaBondAskPrice);
        columns[63]->setDouble(r, pSecuDepthMarketData->YieldToMaturity);
        columns[64]->setDouble(r, pSecuDepthMarketData->IOPV);
        columns[65]->setInt(r, pSecuDepthMarketData->EtfBuycount);
        columns[66]->setInt(r, pSecuDepthMarketData->EtfSellCount);
        columns[67]->setLong(r, pSecuDepthMarketData->EtfBuyVolume);
        columns[68]->setDouble(r, pSecuDepthMarketData->EtfBuyBalance);
        columns[69]->setLong(r, pSecuDepthMarketData->EtfSellVolume);
        columns[70]->setDouble(r, pSecuDepthMarketData->EtfSellBalance);
        columns[71]->setLong(r, pSecuDepthMarketData->TotalWarrantExecVolume);
        columns[72]->setDouble(r, pSecuDepthMarketData->WarrantLowerPrice);
        columns[73]->setDouble(r, pSecuDepthMarketData->WarrantUpperPrice);
        columns[74]->setInt(r, pSecuDepthMarketData->CancelBuyNum);
        columns[75]->setInt(r, pSecuDepthMarketData->CancelSellNum);
        columns[76]->setLong(r, pSecuDepthMarketData->CancelBuyVolume);
        columns[77]->setLong(r, pSecuDepthMarketData->CancelSellVolume);
        columns[78]->setDouble(r, pSecuDepthMarketData->CancelBuyValue);
        columns[79]->setDouble(r, pSecuDepthMarketData->CancelSellValue);
        columns[80]->setInt(r, pSecuDepthMarketData->TotalBuyNum);
        columns[81]->setInt(r, pSecuDepthMarketData->TotalSellNum);
        columns[82]->setInt(r, pSecuDepthMarketData->DurationAfterBuy);
        columns[83]->setInt(r, pSecuDepthMarketData->DurationAfterSell);
        columns[84]->setInt(r, pSecuDepthMarketData->BidOrdersNum);
        columns[85]->setInt(r, pSecuDepthMarketData->AskOrdersNum);
        columns[86]->setDouble(r, pSecuDepthMarketData->PreIOPV);
        int i = 87;
        if (RECEIVED_TIME_FLAG)
        {
            columns[i++]->setLong(r, receivedTime);
        }
        if (GET_ALL_FIELD_NAMES_FLAG)
        {
            columns[i++]->setInt(r, Bid1Count);
            columns[i++]->setInt(r, MaxBid1Count);
            columns[i++]->setInt(r, Ask1Count);
            columns[i++]->setInt(r, MaxAsk1Count);
            int j;
            for (j = 0; j < 50; j++)
            {
                if (j < Bid1Count)
                {
                    columns[i++]->setLong(r, Bid1Volume[j]);
                }
                else
                {
                    columns[i++]->setLong(r, 0);
                }
            }
            for (j = 0; j < 50; j++)
            {
                if (j < Ask1Count)
                {
                    columns[i++]->setLong(r, Ask1Volume[j]);
                }
                else
                {
                    columns[i++]->setLong(r, 0);
                }
            }
        }
    }
}

void setMultTradeData(vector<ConstantSP> &columns, std::vector<CHSNsqSecuTransactionTradeDataField *> &items)
{
    CHSNsqSecuTransactionTradeDataField *pSecuTransactionTradeData = nullptr;
    for (size_t i = 0; i < items.size(); i++)
    {
        pSecuTransactionTradeData = items[i];
        long long receivedTime = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
        if (GET_ALL_FIELD_NAMES_FLAG)
        {
            columns[0]->setString(i, pSecuTransactionTradeData->ExchangeID);
            columns[1]->setString(i, pSecuTransactionTradeData->InstrumentID);
            columns[2]->setInt(i, pSecuTransactionTradeData->TransFlag);
            columns[3]->setLong(i, pSecuTransactionTradeData->SeqNo);
            columns[4]->setInt(i, pSecuTransactionTradeData->ChannelNo);
            columns[5]->set(i, getNewDate(pSecuTransactionTradeData->TradeDate));
            columns[6]->set(i, getNewTime(pSecuTransactionTradeData->TransactTime));
            columns[7]->setInt(i, 2);
            columns[8]->setDouble(i, pSecuTransactionTradeData->TrdPrice);
            columns[9]->setLong(i, pSecuTransactionTradeData->TrdVolume);
            columns[10]->setDouble(i, pSecuTransactionTradeData->TrdMoney);
            columns[11]->setLong(i, pSecuTransactionTradeData->TrdBuyNo);
            columns[12]->setLong(i, pSecuTransactionTradeData->TrdSellNo);
            columns[13]->setChar(i, pSecuTransactionTradeData->TrdBSFlag);
            columns[14]->setLong(i, pSecuTransactionTradeData->BizIndex);
            columns[15]->setChar(i, 0);
            columns[16]->setChar(i, 0);
            columns[17]->setLong(i, 0);
            columns[18]->setLong(i, 0);
            if (RECEIVED_TIME_FLAG)
            {
                columns[19]->setLong(i, receivedTime);
            }
        }
        else
        {
            columns[0]->setString(i, pSecuTransactionTradeData->ExchangeID);
            columns[1]->setString(i, pSecuTransactionTradeData->InstrumentID);
            columns[2]->setInt(i, pSecuTransactionTradeData->TransFlag);
            columns[3]->setLong(i, pSecuTransactionTradeData->SeqNo);
            columns[4]->setInt(i, pSecuTransactionTradeData->ChannelNo);
            columns[5]->set(i, getNewDate(pSecuTransactionTradeData->TradeDate));
            columns[6]->set(i, getNewTime(pSecuTransactionTradeData->TransactTime));
            columns[7]->setDouble(i, pSecuTransactionTradeData->TrdPrice);
            columns[8]->setLong(i, pSecuTransactionTradeData->TrdVolume);
            columns[9]->setDouble(i, pSecuTransactionTradeData->TrdMoney);
            columns[10]->setLong(i, pSecuTransactionTradeData->TrdBuyNo);
            columns[11]->setLong(i, pSecuTransactionTradeData->TrdSellNo);
            columns[12]->setChar(i, pSecuTransactionTradeData->TrdBSFlag);
            columns[13]->setLong(i, pSecuTransactionTradeData->BizIndex);
            if (RECEIVED_TIME_FLAG)
            {
                columns[14]->setLong(i, receivedTime);
            }
        }
    }
}

void setMultEntrustData(vector<ConstantSP> &columns, vector<CHSNsqSecuTransactionEntrustDataField *> items)
{
    CHSNsqSecuTransactionEntrustDataField *pSecuTransactionEntrustData = nullptr;
    for (size_t i = 0; i < items.size(); i++)
    {
        pSecuTransactionEntrustData = items[i];
        long long receivedTime = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
        if (GET_ALL_FIELD_NAMES_FLAG)
        {
            columns[0]->setString(i, pSecuTransactionEntrustData->ExchangeID);
            columns[1]->setString(i, pSecuTransactionEntrustData->InstrumentID);
            columns[2]->setInt(i, pSecuTransactionEntrustData->TransFlag);
            columns[3]->setLong(i, pSecuTransactionEntrustData->SeqNo);
            columns[4]->setInt(i, pSecuTransactionEntrustData->ChannelNo);
            columns[5]->set(i, getNewDate(pSecuTransactionEntrustData->TradeDate));
            columns[6]->set(i, getNewTime(pSecuTransactionEntrustData->TransactTime));
            columns[7]->setInt(i, 1);
            columns[8]->setDouble(i, pSecuTransactionEntrustData->OrdPrice);
            columns[9]->setLong(i, pSecuTransactionEntrustData->OrdVolume);
            columns[10]->setDouble(i, 0);
            columns[11]->setLong(i, 0);
            columns[12]->setLong(i, 0);
            columns[13]->setChar(i, 0);
            columns[14]->setLong(i, pSecuTransactionEntrustData->BizIndex);
            columns[15]->setChar(i, pSecuTransactionEntrustData->OrdSide);
            columns[16]->setChar(i, pSecuTransactionEntrustData->OrdType);
            columns[17]->setLong(i, pSecuTransactionEntrustData->OrdNo);
            columns[18]->setLong(i, pSecuTransactionEntrustData->TrdVolume);
            if (RECEIVED_TIME_FLAG)
            {
                columns[19]->setLong(i, receivedTime);
            }
        }
        else
        {
            columns[0]->setString(i, pSecuTransactionEntrustData->ExchangeID);
            columns[1]->setString(i, pSecuTransactionEntrustData->InstrumentID);
            columns[2]->setInt(i, pSecuTransactionEntrustData->TransFlag);
            columns[3]->setLong(i, pSecuTransactionEntrustData->SeqNo);
            columns[4]->setInt(i, pSecuTransactionEntrustData->ChannelNo);
            columns[5]->set(i, getNewDate(pSecuTransactionEntrustData->TradeDate));
            columns[6]->set(i, getNewTime(pSecuTransactionEntrustData->TransactTime));
            columns[7]->setDouble(i, pSecuTransactionEntrustData->OrdPrice);
            columns[8]->setLong(i, pSecuTransactionEntrustData->OrdVolume);
            columns[9]->setChar(i, pSecuTransactionEntrustData->OrdSide);
            columns[10]->setChar(i, pSecuTransactionEntrustData->OrdType);
            columns[11]->setLong(i, pSecuTransactionEntrustData->OrdNo);
            columns[12]->setLong(i, pSecuTransactionEntrustData->BizIndex);
            if (columns.size() == 14)
            {
                columns[13]->setLong(i, receivedTime);
            }
        }
    }
}

void CHSNsqSpiImpl::asyncSnapshotHandle(std::vector<SnapData *> &items)
{
    int index = -1;

    try
    {
        if (string(items[0]->pSecuDepthMarketData->ExchangeID) == "1")
        {
            index = 0;
        }
        else if (string(items[0]->pSecuDepthMarketData->ExchangeID) == "2")
        {
            index = 1;
        }
        vector<ConstantSP> columns = createVectors(0, items.size());
        vector<string> colNames = SNAPSHOT_COLUMN_NAMES;
        vector<DATA_TYPE> colTypes = SNAPSHOT_TYPES;
        if (GET_ALL_FIELD_NAMES_FLAG)
        {
            columns = createVectors(3, items.size());
            colNames.insert(colNames.end(), ADDED_SNAPSHOT_COLUMN_NAMES.begin(), ADDED_SNAPSHOT_COLUMN_NAMES.end());
            colTypes.insert(colTypes.end(), ADDED_SNAPSHOT_TYPES.begin(), ADDED_SNAPSHOT_TYPES.end());
        }
        else
        {
            columns = createVectors(0, items.size());
        }
        setMultSnapshotData(columns, items);
        vector<ObjectSP> args = {new String(TABLE_NAMES[index])};
        ConstantSP table = SESSION->getFunctionDef("objByName")->call(SESSION->getHeap().get(), args);
        TableSP tmp_table = Util::createTable(colNames, columns);
        vector<ObjectSP> args2 = {table, tmp_table};
        SESSION->getFunctionDef("append!")->call(SESSION->getHeap().get(), args2);
        setStatus(STATUS[index], "");
    }
    catch (exception &e)
    {
        setStatus(STATUS[index], e.what());
    }
}

void CHSNsqSpiImpl::OnRtnSecuDepthMarketData(CHSNsqSecuDepthMarketDataField *pSecuDepthMarketData,
                                             HSIntVolume Bid1Volume[], HSNum Bid1Count, HSNum MaxBid1Count,
                                             HSIntVolume Ask1Volume[], HSNum Ask1Count, HSNum MaxAsk1Count)
{
    try
    {
        SnapData *val = new SnapData();
        val->pSecuDepthMarketData = new CHSNsqSecuDepthMarketDataField();
        std::memcpy(val->pSecuDepthMarketData, pSecuDepthMarketData, sizeof(CHSNsqSecuDepthMarketDataField));
        val->Bid1Volume = new HSIntVolume[50];
        val->Ask1Volume = new HSIntVolume[50];
        std::memcpy(val->Ask1Volume, Ask1Volume, sizeof(HSIntVolume) * Ask1Count);
        std::memcpy(val->Bid1Volume, Bid1Volume, sizeof(HSIntVolume) * Bid1Count);
        val->Bid1Count = Bid1Count;
        val->MaxBid1Count = MaxBid1Count;
        val->Ask1Count = Ask1Count;
        val->MaxAsk1Count = MaxAsk1Count;
        if (string(pSecuDepthMarketData->ExchangeID) == "1")
        {
            snapshotBlockingQueue1_->blockingPush(val);
        }
        else if (string(pSecuDepthMarketData->ExchangeID) == "2")
        {
            snapshotBlockingQueue2_->blockingPush(val);
        }
        else
        {
            throw RuntimeException(NSQ_PREFIX + " error ExchangeID");
        }
    }
    catch (const std::exception &e)
    {
        LOG_ERR(NSQ_PREFIX + " " + "[PluginNsq] OnRtnSecuDepthMarketData function has problem because ", e.what(), " or can not malloc");
    }
}

void CHSNsqSpiImpl::OnRtnSecuATPMarketData(CHSNsqSecuATPMarketDataField *pSecuDepthMarketData, HSIntVolume Bid1Volume[],
                                           HSNum Bid1Count, HSNum MaxBid1Count, HSIntVolume Ask1Volume[],
                                           HSNum Ask1Count, HSNum MaxAsk1Count)
{
    // LOG_INFO("OnRtnSecuATPMarketData: ExchangeID %s, InstrumentID %s, TradeDate %d, "
    //        "UpdateTime %d, PreClosePrice %lf, ClosePrice %lf, InstrumentTradeStatus "
    //        "%c, TradeVolume  %" PRId64 ",\n"
    //        "\tBidPrice1 %lf, BidVolume1 %" PRId64 ", Bid1Volume[0] %" PRId64 ", Bid1Count %d, MaxBid1Count %d\n"
    //        "\tAskPrice1 %lf, AskVolume1 %" PRId64 ", Ask1Volume[0] %" PRId64 ", Ask1Count %d, MaxAsk1Count %d\n",
    //        pSecuDepthMarketData->ExchangeID, pSecuDepthMarketData->InstrumentID, pSecuDepthMarketData->TradeDate,
    //        pSecuDepthMarketData->UpdateTime, pSecuDepthMarketData->PreClosePrice, pSecuDepthMarketData->ClosePrice,
    //        pSecuDepthMarketData->InstrumentTradeStatus, pSecuDepthMarketData->TradeVolume,

    //        pSecuDepthMarketData->BidPrice1, pSecuDepthMarketData->BidVolume1, Bid1Volume[0], Bid1Count, MaxBid1Count,
    //        pSecuDepthMarketData->AskPrice1, pSecuDepthMarketData->AskVolume1, Ask1Volume[0], Ask1Count,
    //        MaxAsk1Count);
}

// trade and order mix together, they are all uTransaction，one of type1，one of type2
void CHSNsqSpiImpl::OnRspSecuTransactionSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (nRequestID == 10003)
    {
        SUBSCRIBE_STATUS[2] = 1;
    }
    else if (nRequestID == 10004)
    {
        SUBSCRIBE_STATUS[3] = 1;
    }
    else if (nRequestID == 10005)
    {
        SUBSCRIBE_STATUS[4] = 1;
    }
    else if (nRequestID == 10006)
    {
        SUBSCRIBE_STATUS[5] = 1;
    }
    if (pRspInfo->ErrorID != 0)
    {
        throw RuntimeException(NSQ_PREFIX + " " + pRspInfo->ErrorMsg);
    }
    LOG_INFO(NSQ_PREFIX + " " + "OnRspSecuTransactionSubscribe: nRequestID[", nRequestID, "], ErrorID[", pRspInfo->ErrorID,
             "], ErrorMsg: ", pRspInfo->ErrorMsg);
}

// trade
void CHSNsqSpiImpl::OnRspSecuTransactionCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (nRequestID == 10003)
    {
        resetStatus(2);
    }
    else if (nRequestID == 10004)
    {
        resetStatus(3);
    }
    else if (nRequestID == 10005)
    {
        resetStatus(4);
    }
    else if (nRequestID == 10006)
    {
        resetStatus(5);
    }
    if (pRspInfo->ErrorID != 0)
    {
        throw RuntimeException(NSQ_PREFIX + " " + pRspInfo->ErrorMsg);
    }
    LOG_INFO(NSQ_PREFIX + " " + "OnRspSecuTransactionCancel: nRequestID[", nRequestID, "], ErrorID[", pRspInfo->ErrorID,
             "], ErrorMsg: ", pRspInfo->ErrorMsg);
}

void CHSNsqSpiImpl::asyncTradeHandle(std::vector<CHSNsqSecuTransactionTradeDataField *> &items)
{
    int index = -1;

    try
    {
        if (string(items[0]->ExchangeID) == "1")
        {
            index = 2;
        }
        else if (string(items[0]->ExchangeID) == "2")
        {
            index = 3;
        }

        vector<ConstantSP> columns;
        vector<string> colNames;
        vector<DATA_TYPE> colTypes;
        if (GET_ALL_FIELD_NAMES_FLAG)
        {
            columns = createVectors(4, items.size());
            colNames = TRADE_AND_ORDER_MERGED_COLUMN_NAMES;
            colTypes = TRADE_AND_ORDER_MERGED_TYPES;
        }
        else
        {
            columns = createVectors(1, items.size());
            colNames = TRADE_COLUMN_NAMES;
            colTypes = TRADE_TYPES;
        }

        setMultTradeData(columns, items);

        vector<ObjectSP> args = {new String(TABLE_NAMES[index])};
        ConstantSP table = SESSION->getFunctionDef("objByName")->call(SESSION->getHeap().get(), args);
        TableSP tmp_table = Util::createTable(colNames, columns);
        vector<ObjectSP> args2 = {table, tmp_table};
        SESSION->getFunctionDef("append!")->call(SESSION->getHeap().get(), args2);

        setStatus(STATUS[index], "");
    }
    catch (exception &e)
    {
        setStatus(STATUS[index], e.what());
    }
}

void CHSNsqSpiImpl::OnRtnSecuTransactionTradeData(CHSNsqSecuTransactionTradeDataField *pSecuTransactionTradeData)
{
    try
    {
        CHSNsqSecuTransactionTradeDataField *val = new CHSNsqSecuTransactionTradeDataField();
        std::memcpy(val, pSecuTransactionTradeData, sizeof(CHSNsqSecuTransactionTradeDataField));
        if (string(pSecuTransactionTradeData->ExchangeID) == "1")
        {
            tradeBlockingQueue1_->blockingPush(val);
        }
        else if (string(pSecuTransactionTradeData->ExchangeID) == "2")
        {
            tradeBlockingQueue2_->blockingPush(val);
        }
        else
        {
            throw RuntimeException(NSQ_PREFIX + " error ExchangeID");
        }
    }
    catch (const std::exception &e)
    {
        LOG_ERR(NSQ_PREFIX + " OnRtnSecuTransactionTradeData function has problem because ", e.what(), " or can not malloc");
    }
}

void CHSNsqSpiImpl::asyncEntrustHandle(std::vector<CHSNsqSecuTransactionEntrustDataField *> &items)
{
    int index = -1;

    try
    {
        if (string(items[0]->ExchangeID) == "1")
        {
            index = 4;
        }
        else if (string(items[0]->ExchangeID) == "2")
        {
            index = 5;
        }
        else
        {
            throw RuntimeException(NSQ_PREFIX + " " + "error ExchangeID");
        }

        vector<ConstantSP> columns;
        vector<string> colNames;
        vector<DATA_TYPE> colTypes;
        if (GET_ALL_FIELD_NAMES_FLAG)
        {
            columns = createVectors(4, items.size());
            colNames = TRADE_AND_ORDER_MERGED_COLUMN_NAMES;
            colTypes = TRADE_AND_ORDER_MERGED_TYPES;
        }
        else
        {
            columns = createVectors(2, items.size());
            colNames = ORDER_COLUMN_NAMES;
            colTypes = ORDER_TYPES;
        }
        setMultEntrustData(columns, items);

        vector<ObjectSP> args = {new String(TABLE_NAMES[index])};
        ConstantSP table = SESSION->getFunctionDef("objByName")->call(SESSION->getHeap().get(), args);
        TableSP tmp_table = Util::createTable(colNames, columns);
        vector<ObjectSP> args2 = {table, tmp_table};
        SESSION->getFunctionDef("append!")->call(SESSION->getHeap().get(), args2);

        setStatus(STATUS[index], "");
    }
    catch (exception &e)
    {
        setStatus(STATUS[index], e.what());
    }
}

void CHSNsqSpiImpl::OnRtnSecuTransactionEntrustData(
    CHSNsqSecuTransactionEntrustDataField *pSecuTransactionEntrustData)
{
    try
    {
        CHSNsqSecuTransactionEntrustDataField *val = new CHSNsqSecuTransactionEntrustDataField();
        std::memcpy(val, pSecuTransactionEntrustData, sizeof(CHSNsqSecuTransactionEntrustDataField));

        if (string(pSecuTransactionEntrustData->ExchangeID) == "1")
        {
            entrustBlockingQueue1_->blockingPush(val);
        }
        else if (string(pSecuTransactionEntrustData->ExchangeID) == "2")
        {
            entrustBlockingQueue2_->blockingPush(val);
        }
        else
        {
            throw RuntimeException(NSQ_PREFIX + " " + "error ExchangeID");
        }
    }
    catch (const std::exception &e)
    {
        LOG_ERR(NSQ_PREFIX + " OnRtnSecuTransactionEntrustData function has problem because ", e.what(), " or can not malloc");
    }
}

void CHSNsqSpiImpl::OnRtnSecuATPTransactionTradeData(CHSNsqSecuTransactionTradeDataField *pSecuTransactionTradeData) {}

void CHSNsqSpiImpl::OnRspQrySecuInstruments(CHSNsqSecuInstrumentStaticInfoField *pSecuInstrumentStaticInfo,
                                            CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    char sz_buf[1024];
    int buf_len;
    if (0 == iAllInstrCount)
    {
        LOG_INFO(NSQ_PREFIX + " " + "RspQrySecuInstruments Successed !");
    }

    buf_len = snprintf(sz_buf, 1024,
                       "[OnRspQrySecuInstruments]: ExchangeID %s, InstrumentID %s, "
                       "InstrumentName %s, SecurityType %c, PreClosePrice %lf, UpperLimitPrice "
                       "%lf, LowerLimitPrice %lf, PriceTick %lf, BuyVolumeUnit %d, "
                       "SellVolumeUnit %d, TradeDate %d, bIsLast %d\n",
                       pSecuInstrumentStaticInfo->ExchangeID, pSecuInstrumentStaticInfo->InstrumentID,
                       pSecuInstrumentStaticInfo->InstrumentName, pSecuInstrumentStaticInfo->SecurityType,
                       pSecuInstrumentStaticInfo->PreClosePrice, pSecuInstrumentStaticInfo->UpperLimitPrice,
                       pSecuInstrumentStaticInfo->LowerLimitPrice, pSecuInstrumentStaticInfo->PriceTick,
                       pSecuInstrumentStaticInfo->BuyVolumeUnit, pSecuInstrumentStaticInfo->SellVolumeUnit,
                       pSecuInstrumentStaticInfo->TradeDate, bIsLast);

#ifdef ENABLE_IPC_TIMESTAMP_DEBUG
    LOG_INFO("%s", sz_buf);
#endif

    if (strcmp(pSecuInstrumentStaticInfo->ExchangeID, HS_EI_SZSE) == 0)
    {
        if (fSzStockInit == NULL)
        {
            fSzStockInit = fopen("sz_stock_code.txt", "w+");
            if (fSzStockInit == NULL)
            {
                LOG_INFO(NSQ_PREFIX + " " + "fopen sz_stock_code.txt fail: %s\n", strerror(errno));
            }
        }
        if (fSzStockInit)
        {
            fwrite(sz_buf, buf_len, 1, fSzStockInit);
            if (bIsLast)
            {
                fclose(fSzStockInit);
                fSzStockInit = NULL;
            }
        }
    }
    else
    {
        if (fShStockInit == NULL)
        {
            fShStockInit = fopen("sh_stock_code.txt", "w+");
            if (fShStockInit == NULL)
            {
                LOG_INFO(NSQ_PREFIX + " " + "fopen sh_stock_code.txt fail: %s\n", strerror(errno));
            }
        }
        if (fShStockInit)
        {
            fwrite(sz_buf, buf_len, 1, fShStockInit);
            if (bIsLast)
            {
                fclose(fShStockInit);
                fShStockInit = NULL;
            }
        }
    }

    iAllInstrCount++;
    if (bIsLast)
    {
        isAllInstrReady = true;
    }
}

void CHSNsqSpiImpl::OnRspOptDepthMarketDataSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    LOG_INFO(NSQ_PREFIX + " " + "OnRspOptDepthMarketDataSubscribe: nRequestID[", nRequestID, "], ErrorID[", pRspInfo->ErrorID,
             "], ErrorMsg: ", pRspInfo->ErrorMsg);
}

void CHSNsqSpiImpl::OnRspOptDepthMarketDataCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    LOG_INFO(NSQ_PREFIX + " " + "OnRspOptDepthMarketDataCancel: nRequestID[", nRequestID, "], ErrorID[", pRspInfo->ErrorID,
             "], ErrorMsg: ", pRspInfo->ErrorMsg);
}

void CHSNsqSpiImpl::OnRtnOptDepthMarketData(CHSNsqOptDepthMarketDataField *pOptDepthMarketData)
{
    CHSNsqOptDepthMarketDataField *p = pOptDepthMarketData;

    LOG_INFO(NSQ_PREFIX + " " + "%s,%lf,%lf,%lf,%lf,%lf,%lf,%" PRId64 ",%" PRId64 ",%lf,%lf,%lf,%lf,%lf,%lf,%" PRId64 ",%" PRId64
             ",%lf,%lf,%" PRId64 ",%c,%c,%lf,%" PRId64 ",%d,%" PRId64 "\n",
             p->InstrumentID, p->LastPrice, p->PreClosePrice, p->OpenPrice, p->HighPrice, p->LowPrice, p->ClosePrice,
             p->PreOpenInterest, p->OpenInterest, p->PreSettlementPrice, p->SettlementPrice, p->UpperLimitPrice,
             p->LowerLimitPrice, p->PreDelta, p->CurDelta, (uint64)p->TradeDate * 1000000000 + p->UpdateTime,
             p->TradeVolume, p->TradeBalance, p->AveragePrice, p->TradesNum, p->InstrumentTradeStatus,
             p->OpenRestriction[0], p->AuctionPrice, p->AuctionVolume, p->LastEnquiryTime, p->LeaveQty);
    int i;
    for (i = 0; i < 10; i++)
    {
        LOG_INFO(NSQ_PREFIX + " " + "bid[%d]:%lf,%" PRId64 "\n", i, p->BidPrice[i], p->BidVolume[i]);
    }
    for (i = 0; i < 10; i++)
    {
        LOG_INFO(NSQ_PREFIX + " " + "ask[%d]:%lf,%" PRId64 "\n", i, p->AskPrice[i], p->AskVolume[i]);
    }
}

void CHSNsqSpiImpl::OnRspQryOptInstruments(CHSNsqOptInstrumentStaticInfoField *pOptInstrumentStaticInfo,
                                           CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    char sz_buf[1024];
    int buf_len;
    CHSNsqOptInstrumentStaticInfoField *p = pOptInstrumentStaticInfo;
    if (0 == iAllInstrCount)
    {
        LOG_INFO(NSQ_PREFIX + " " + "RspQryOptInstruments Successed !");
    }

    buf_len = snprintf(sz_buf, 1024,
                       "ExchangeID[%s], InstrumentID[%s], InstrumentTradeID[%s], "
                       "InstrumentName[%s]",
                       p->ExchangeID, p->InstrumentID, p->InstrumentTradeID, p->InstrumentName);
    buf_len += snprintf(sz_buf + buf_len, 1024 - buf_len,
                        "SecurityType[%c], UnderlyingInstrID[%s], "
                        "OptionsType[%c], ExerciseStyle[%c]",
                        p->SecurityType, p->UnderlyingInstrID, p->OptionsType, p->ExerciseStyle);
    buf_len += snprintf(sz_buf + buf_len, 1024 - buf_len,
                        "ContractMultiplierUnit[%" PRId64 "], ExercisePrice[%lf], StartDate[%d], EndDate[%d]",
                        p->ContractMultiplierUnit, p->ExercisePrice, p->StartDate, p->EndDate);
    buf_len += snprintf(sz_buf + buf_len, 1024 - buf_len,
                        "ExerciseDate[%d], DeliveryDate[%d], "
                        "ExpireDate[%d],TotalLongPosition[%" PRId64 "]",
                        p->ExerciseDate, p->DeliveryDate, p->ExpireDate, p->TotalLongPosition);
    buf_len +=
        snprintf(sz_buf + buf_len, 1024 - buf_len,
                 "PreClosePrice[%lf], PreSettlPrice[%lf], UnderlyingClosePrice[%lf], "
                 "UpperLimitPrice[%lf], LowerLimitPrice[%lf]",
                 p->PreClosePrice, p->PreSettlPrice, p->UnderlyingClosePrice, p->UpperLimitPrice, p->LowerLimitPrice);
    buf_len += snprintf(sz_buf + buf_len, 1024 - buf_len,
                        "MarginUnit[%lf], MarginRatioParam1[%lf], "
                        "MarginRatioParam2[%lf], VolumeMultiple[%" PRId64 "]",
                        p->MarginUnit, p->MarginRatioParam1, p->MarginRatioParam2, p->VolumeMultiple);
    buf_len += snprintf(sz_buf + buf_len, 1024 - buf_len,
                        "MinLimitOrderVolume[%" PRId64 "], MaxLimitOrderVolume[%" PRId64
                        "], MinMarketOrderVolume[%" PRId64 "]",
                        p->MinLimitOrderVolume, p->MaxLimitOrderVolume, p->MinMarketOrderVolume);
    buf_len +=
        snprintf(sz_buf + buf_len, 1024 - buf_len, "MaxMarketOrderVolume[%" PRId64 "], PriceTick[%lf], TradeDate[%d]\n",
                 p->MaxMarketOrderVolume, p->PriceTick, p->TradeDate);

#ifdef ENABLE_IPC_TIMESTAMP_DEBUG
    LOG_INFO("%s", sz_buf);
#endif
    if (strcmp(p->ExchangeID, HS_EI_SZSE) == 0)
    {
        if (fSzOptInit == NULL)
        {
            fSzOptInit = fopen("sz_opt_code.txt", "w+");
            if (fSzOptInit == NULL)
            {
                LOG_INFO(NSQ_PREFIX + " " + "fopen sz_opt_code.txt fail: %s\n", strerror(errno));
            }
        }
        if (fSzOptInit)
        {
            fwrite(sz_buf, buf_len, 1, fSzOptInit);
            if (bIsLast)
            {
                fclose(fSzOptInit);
                fSzOptInit = NULL;
            }
        }
    }
    else
    {
        if (fShOptInit == NULL)
        {
            fShOptInit = fopen("sh_opt_code.txt", "w+");
            if (fShOptInit == NULL)
            {
                LOG_INFO(NSQ_PREFIX + " " + "fopen sh_opt_code.txt fail: %s\n", strerror(errno));
            }
        }
        if (fShOptInit)
        {
            fwrite(sz_buf, buf_len, 1, fShOptInit);
            if (bIsLast)
            {
                fclose(fShOptInit);
                fShOptInit = NULL;
            }
        }
    }

    iAllInstrCount++;
    if (bIsLast)
    {
        isAllInstrReady = true;
    }
}

void CHSNsqSpiImpl::OnRspQryOptDepthMarketData(CHSNsqOptDepthMarketDataField *pOptDepthMarketData,
                                               CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {}

void CHSNsqSpiImpl::OnRspSecuTransactionData(CHSNsqSecuTransactionDataField *pSecuTransactionData,
                                             CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    LOG_INFO(NSQ_PREFIX + " " + "[OnRspSecuTransactionData]: CHSNsqRspInfoField ErrorID %d ErrorMsg %s "
             "bIsLast %d\n",
             pRspInfo->ErrorID, pRspInfo->ErrorMsg, bIsLast);
    if (pRspInfo->ErrorID == 0)
    {
        CHSNsqSecuTransactionDataField *p = pSecuTransactionData;
        if (p->TransType == HS_TRANS_Trade)
        {
            LOG_INFO(NSQ_PREFIX + " " + "[OnRspSecuTransactionData]: ExchangeID %s, InstrumentID %s, "
                     "TransFlag %d, SeqNo %" PRId64 ", ChannelNo %d,"
                     " TradeDate %d, TransactTime %d, TrdPrice %lf, TrdVolume %" PRId64 ", TrdMoney %lf,"
                     " TrdBuyNo %" PRId64 ", TrdSellNo %" PRId64 ", TrdBSFlag %c, BizIndex %" PRId64 "\n",
                     p->TradeData.ExchangeID, p->TradeData.InstrumentID, p->TradeData.TransFlag, p->TradeData.SeqNo,
                     p->TradeData.ChannelNo, p->TradeData.TradeDate, p->TradeData.TransactTime, p->TradeData.TrdPrice,
                     p->TradeData.TrdVolume, p->TradeData.TrdMoney, p->TradeData.TrdBuyNo, p->TradeData.TrdSellNo,
                     p->TradeData.TrdBSFlag, p->TradeData.BizIndex);
        }
        else if (p->TransType == HS_TRANS_Entrust)
        {
            LOG_INFO(NSQ_PREFIX + " " + "[OnRspSecuTransactionData]: ExchangeID %s, InstrumentID %s, "
                     "TransFlag %d, SeqNo %" PRId64 ", ChannelNo %d,"
                     " TradeDate %d, TransactTime %d, OrdPrice %lf, OrdVolume %" PRId64 ", OrdSide %c,"
                     " OrdType %c, OrdNo %" PRId64 ", BizIndex %" PRId64 "\n",
                     p->EntrustData.ExchangeID, p->EntrustData.InstrumentID, p->EntrustData.TransFlag,
                     p->EntrustData.SeqNo, p->EntrustData.ChannelNo, p->EntrustData.TradeDate,
                     p->EntrustData.TransactTime, p->EntrustData.OrdPrice, p->EntrustData.OrdVolume,
                     p->EntrustData.OrdSide, p->EntrustData.OrdType, p->EntrustData.OrdNo, p->EntrustData.BizIndex);
        }
        else
        {
            LOG_INFO(NSQ_PREFIX + " " + "error TransType:%c\n", p->TransType);
        }
    }
}

void CHSNsqSpiImpl::OnRspSecuTransactionDataTimeout(int nRequestID)
{
    LOG_INFO(NSQ_PREFIX + " " + "OnRspSecuTransactionDataTimeout nRequestID=%d\n", nRequestID);
}
