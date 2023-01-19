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
#include "plugin_nsq.h"

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
    subscribe_status[id] = 0;

    status[id].processedMsgCount = 0;
    status[id].failedMsgCount = 0;
    status[id].lastErrMsg = "";
    status[id].lastFailedTimestamp = Timestamp(LLONG_MIN);
}

void CHSNsqSpiImpl::OnFrontConnected()
{
    LOG_INFO("nsq: OnFrontConnected\n");
    m_isConnected = true;
}

void CHSNsqSpiImpl::OnFrontDisconnected(int nResult)
{
    LOG_INFO("OnFrontDisconnected: nResult %d\n", nResult);
    // 断线后，可以重新连接
    // 重新连接成功后，需要重新向服务器发起订阅请求
    LOG_INFO("nsq: OnFrontDisconnected\n");
    m_isConnected = false;
    // throw RuntimeException("DisConnected");
}

void CHSNsqSpiImpl::OnRspUserLogin(CHSNsqRspUserLoginField *pRspUserLogin, CHSNsqRspInfoField *pRspInfo, int nRequestID,
                                   bool bIsLast)
{
    LOG_INFO("OnRspUserLogin errno=", pRspInfo->ErrorID, " errinfo=", pRspInfo->ErrorMsg);
    if (0 == pRspInfo->ErrorID)
        m_isLogined = true;
}

// snapshot
void CHSNsqSpiImpl::OnRspSecuDepthMarketDataSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    // LOG_INFO("pRspInfo->ErrorID: %d\n", pRspInfo->ErrorID);
    // LOG_INFO("wjh1: 行情快照请求订阅成功\n");
    if (nRequestID == 10001)
    {
        subscribe_status[0] = 1;
    }
    else if (nRequestID == 10002)
    {
        subscribe_status[1] = 1;
    }
    if (pRspInfo->ErrorID != 0)
    { // 说明有错误
        throw RuntimeException(pRspInfo->ErrorMsg);
    }
    LOG_INFO("OnRspSecuDepthMarketDataSubscribe: nRequestID[", nRequestID, "], ErrorID[", pRspInfo->ErrorID,
             "], ErrorMsg: ", pRspInfo->ErrorMsg);
}

// snapshot
void CHSNsqSpiImpl::OnRspSecuDepthMarketDataCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    // 取消订阅成功后需要将表设置回去
    LOG_INFO("unsuscribe success: nRequestID = ", nRequestID);
    if (nRequestID == 10001)
    {
        resetStatus(0);
    }
    else if (nRequestID == 10002)
    {
        resetStatus(1);
    }
    if (pRspInfo->ErrorID != 0)
    { // 说明有错误
        throw RuntimeException(pRspInfo->ErrorMsg);
    }
    LOG_INFO("OnRspSecuDepthMarketDataCancel: nRequestID[", nRequestID, "], ErrorID[", pRspInfo->ErrorID,
             "], ErrorMsg: ", pRspInfo->ErrorMsg);
}

// 如何根据表的Schema来创建 vectors，避免自己手动创建
vector<ConstantSP> createVectors(int flag, int size)
{
    // 0 是snapshot，1是trade，2是orders, 3是所有字段的snapshot， 4是order和trade的结合
    vector<ConstantSP> columns;
    if (flag == 0)
    {
        columns.resize(snapshotTypes.size());
        for (size_t i = 0; i < snapshotTypes.size(); i++)
        {
            columns[i] = Util::createVector(snapshotTypes[i], size, size);
        }
    }
    else if (flag == 1)
    {
        columns.resize(tradeTypes.size());
        for (size_t i = 0; i < tradeTypes.size(); i++)
        {
            columns[i] = Util::createVector(tradeTypes[i], size, size);
        }
    }
    else if (flag == 2)
    {
        columns.resize(ordersTypes.size());
        for (size_t i = 0; i < ordersTypes.size(); i++)
        {
            columns[i] = Util::createVector(ordersTypes[i], size, size);
        }
    }
    else if (flag == 3)
    {
        vector<DATA_TYPE> colTypes = snapshotTypes;
        colTypes.insert(colTypes.end(), addedSnapshotTypes.begin(), addedSnapshotTypes.end());
        columns.resize(colTypes.size());
        for (size_t i = 0; i < colTypes.size(); i++)
        {
            columns[i] = Util::createVector(colTypes[i], size, size);
        }
    }
    else if (flag == 4)
    {
        columns.resize(tradeAndOrdersMergedTypes.size());
        for (size_t i = 0; i < tradeAndOrdersMergedTypes.size(); i++)
        {
            columns[i] = Util::createVector(tradeAndOrdersMergedTypes[i], size, size);
        }
    }
    else
    {
        throw RuntimeException("flag error");
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
    for (int r = 0; r < items.size(); r++)
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
        columns[0]->set(r, new String(pSecuDepthMarketData->ExchangeID));
        columns[1]->set(r, new String(pSecuDepthMarketData->InstrumentID));
        columns[2]->set(r, new Double(pSecuDepthMarketData->LastPrice));
        columns[3]->set(r, new Double(pSecuDepthMarketData->PreClosePrice));
        columns[4]->set(r, new Double(pSecuDepthMarketData->OpenPrice));
        columns[5]->set(r, new Double(pSecuDepthMarketData->HighPrice));
        columns[6]->set(r, new Double(pSecuDepthMarketData->LowPrice));
        columns[7]->set(r, new Double(pSecuDepthMarketData->ClosePrice));
        columns[8]->set(r, new Double(pSecuDepthMarketData->UpperLimitPrice));
        columns[9]->set(r, new Double(pSecuDepthMarketData->LowerLimitPrice));
        // TradeDate 是 DATE类型, YYYYMMDD
        columns[10]->set(r, getNewDate(pSecuDepthMarketData->TradeDate));
        // UpdateTime 是 TIME 类型，HHMMSSsss
        columns[11]->set(r, getNewTime(pSecuDepthMarketData->UpdateTime));
        columns[12]->set(r, new Long(pSecuDepthMarketData->TradeVolume));
        columns[13]->set(r, new Double(pSecuDepthMarketData->TradeBalance));
        columns[14]->set(r, new Double(pSecuDepthMarketData->AveragePrice));
        columns[15]->set(r, new Double(pSecuDepthMarketData->BidPrice[0]));
        columns[16]->set(r, new Double(pSecuDepthMarketData->BidPrice[1]));
        columns[17]->set(r, new Double(pSecuDepthMarketData->BidPrice[2]));
        columns[18]->set(r, new Double(pSecuDepthMarketData->BidPrice[3]));
        columns[19]->set(r, new Double(pSecuDepthMarketData->BidPrice[4]));
        columns[20]->set(r, new Double(pSecuDepthMarketData->BidPrice[5]));
        columns[21]->set(r, new Double(pSecuDepthMarketData->BidPrice[6]));
        columns[22]->set(r, new Double(pSecuDepthMarketData->BidPrice[7]));
        columns[23]->set(r, new Double(pSecuDepthMarketData->BidPrice[8]));
        columns[24]->set(r, new Double(pSecuDepthMarketData->BidPrice[9]));
        columns[25]->set(r, new Double(pSecuDepthMarketData->AskPrice[0]));
        columns[26]->set(r, new Double(pSecuDepthMarketData->AskPrice[1]));
        columns[27]->set(r, new Double(pSecuDepthMarketData->AskPrice[2]));
        columns[28]->set(r, new Double(pSecuDepthMarketData->AskPrice[3]));
        columns[29]->set(r, new Double(pSecuDepthMarketData->AskPrice[4]));
        columns[30]->set(r, new Double(pSecuDepthMarketData->AskPrice[5]));
        columns[31]->set(r, new Double(pSecuDepthMarketData->AskPrice[6]));
        columns[32]->set(r, new Double(pSecuDepthMarketData->AskPrice[7]));
        columns[33]->set(r, new Double(pSecuDepthMarketData->AskPrice[8]));
        columns[34]->set(r, new Double(pSecuDepthMarketData->AskPrice[9]));
        columns[35]->set(r, new Long(pSecuDepthMarketData->BidVolume[0]));
        columns[36]->set(r, new Long(pSecuDepthMarketData->BidVolume[1]));
        columns[37]->set(r, new Long(pSecuDepthMarketData->BidVolume[2]));
        columns[38]->set(r, new Long(pSecuDepthMarketData->BidVolume[3]));
        columns[39]->set(r, new Long(pSecuDepthMarketData->BidVolume[4]));
        columns[40]->set(r, new Long(pSecuDepthMarketData->BidVolume[5]));
        columns[41]->set(r, new Long(pSecuDepthMarketData->BidVolume[6]));
        columns[42]->set(r, new Long(pSecuDepthMarketData->BidVolume[7]));
        columns[43]->set(r, new Long(pSecuDepthMarketData->BidVolume[8]));
        columns[44]->set(r, new Long(pSecuDepthMarketData->BidVolume[9]));
        columns[45]->set(r, new Long(pSecuDepthMarketData->AskVolume[0]));
        columns[46]->set(r, new Long(pSecuDepthMarketData->AskVolume[1]));
        columns[47]->set(r, new Long(pSecuDepthMarketData->AskVolume[2]));
        columns[48]->set(r, new Long(pSecuDepthMarketData->AskVolume[3]));
        columns[49]->set(r, new Long(pSecuDepthMarketData->AskVolume[4]));
        columns[50]->set(r, new Long(pSecuDepthMarketData->AskVolume[5]));
        columns[51]->set(r, new Long(pSecuDepthMarketData->AskVolume[6]));
        columns[52]->set(r, new Long(pSecuDepthMarketData->AskVolume[7]));
        columns[53]->set(r, new Long(pSecuDepthMarketData->AskVolume[8]));
        columns[54]->set(r, new Long(pSecuDepthMarketData->AskVolume[9]));
        columns[55]->set(r, new Long(pSecuDepthMarketData->TradesNum));
        columns[56]->set(r, new Char(pSecuDepthMarketData->InstrumentTradeStatus));
        columns[57]->set(r, new Long(pSecuDepthMarketData->TotalBidVolume));
        columns[58]->set(r, new Long(pSecuDepthMarketData->TotalAskVolume));
        columns[59]->set(r, new Double(pSecuDepthMarketData->MaBidPrice));
        columns[60]->set(r, new Double(pSecuDepthMarketData->MaAskPrice));
        columns[61]->set(r, new Double(pSecuDepthMarketData->MaBondBidPrice));
        columns[62]->set(r, new Double(pSecuDepthMarketData->MaBondAskPrice));
        columns[63]->set(r, new Double(pSecuDepthMarketData->YieldToMaturity));
        columns[64]->set(r, new Double(pSecuDepthMarketData->IOPV));
        columns[65]->set(r, new Int(pSecuDepthMarketData->EtfBuycount));
        columns[66]->set(r, new Int(pSecuDepthMarketData->EtfSellCount));
        columns[67]->set(r, new Long(pSecuDepthMarketData->EtfBuyVolume));
        columns[68]->set(r, new Double(pSecuDepthMarketData->EtfBuyBalance));
        columns[69]->set(r, new Long(pSecuDepthMarketData->EtfSellVolume));
        columns[70]->set(r, new Double(pSecuDepthMarketData->EtfSellBalance));
        columns[71]->set(r, new Long(pSecuDepthMarketData->TotalWarrantExecVolume));
        columns[72]->set(r, new Double(pSecuDepthMarketData->WarrantLowerPrice));
        columns[73]->set(r, new Double(pSecuDepthMarketData->WarrantUpperPrice));
        columns[74]->set(r, new Int(pSecuDepthMarketData->CancelBuyNum));
        columns[75]->set(r, new Int(pSecuDepthMarketData->CancelSellNum));
        columns[76]->set(r, new Long(pSecuDepthMarketData->CancelBuyVolume));
        columns[77]->set(r, new Long(pSecuDepthMarketData->CancelSellVolume));
        columns[78]->set(r, new Double(pSecuDepthMarketData->CancelBuyValue));
        columns[79]->set(r, new Double(pSecuDepthMarketData->CancelSellValue));
        columns[80]->set(r, new Int(pSecuDepthMarketData->TotalBuyNum));
        columns[81]->set(r, new Int(pSecuDepthMarketData->TotalSellNum));
        columns[82]->set(r, new Int(pSecuDepthMarketData->DurationAfterBuy));
        columns[83]->set(r, new Int(pSecuDepthMarketData->DurationAfterSell));
        columns[84]->set(r, new Int(pSecuDepthMarketData->BidOrdersNum));
        columns[85]->set(r, new Int(pSecuDepthMarketData->AskOrdersNum));
        columns[86]->set(r, new Double(pSecuDepthMarketData->PreIOPV));
        int i = 87;
        if (receivedTimeFlag)
        {
            columns[i++]->set(r, new Long(receivedTime));
        }
        if (getAllFieldNamesFlag)
        {
            columns[i++]->set(r, new Int(Bid1Count));
            columns[i++]->set(r, new Int(MaxBid1Count));
            columns[i++]->set(r, new Int(Ask1Count));
            columns[i++]->set(r, new Int(MaxAsk1Count));
            int j;
            for (j = 0; j < 50; j++)
            {
                if (j < Bid1Count)
                {
                    columns[i++]->set(r, new Long(Bid1Volume[j]));
                }
                else
                {
                    columns[i++]->set(r, new Long(0));
                }
            }
            for (j = 0; j < 50; j++)
            {
                if (j < Ask1Count)
                {
                    columns[i++]->set(r, new Long(Ask1Volume[j]));
                }
                else
                {
                    columns[i++]->set(r, new Long(0));
                }
            }
        }
    }
}

void setMultTradeData(vector<ConstantSP> &columns, std::vector<CHSNsqSecuTransactionTradeDataField *> &items)
{
    CHSNsqSecuTransactionTradeDataField *pSecuTransactionTradeData = nullptr;
    for (int i = 0; i < items.size(); i++)
    {
        pSecuTransactionTradeData = items[i];
        long long receivedTime = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
        if (getAllFieldNamesFlag)
        {
            columns[0]->set(i, new String(pSecuTransactionTradeData->ExchangeID));
            columns[1]->set(i, new String(pSecuTransactionTradeData->InstrumentID));
            columns[2]->set(i, new Int(pSecuTransactionTradeData->TransFlag));
            columns[3]->set(i, new Long(pSecuTransactionTradeData->SeqNo));
            columns[4]->set(i, new Int(pSecuTransactionTradeData->ChannelNo));
            columns[5]->set(i, getNewDate(pSecuTransactionTradeData->TradeDate));
            columns[6]->set(i, getNewTime(pSecuTransactionTradeData->TransactTime));
            columns[7]->set(i, new Int(2));
            columns[8]->set(i, new Double(pSecuTransactionTradeData->TrdPrice));
            columns[9]->set(i, new Long(pSecuTransactionTradeData->TrdVolume));
            columns[10]->set(i, new Double(pSecuTransactionTradeData->TrdMoney));
            columns[11]->set(i, new Long(pSecuTransactionTradeData->TrdBuyNo));
            columns[12]->set(i, new Long(pSecuTransactionTradeData->TrdSellNo));
            columns[13]->set(i, new Char(pSecuTransactionTradeData->TrdBSFlag));
            columns[14]->set(i, new Long(pSecuTransactionTradeData->BizIndex));
            columns[15]->set(i, new Char(0));
            columns[16]->set(i, new Char(0));
            columns[17]->set(i, new Long(0));
            columns[18]->set(i, new Long(0));
            if (receivedTimeFlag)
            {
                columns[19]->set(i, new Long(receivedTime));
            }
        }
        else
        {
            columns[0]->set(i, new String(pSecuTransactionTradeData->ExchangeID));
            columns[1]->set(i, new String(pSecuTransactionTradeData->InstrumentID));
            columns[2]->set(i, new Int(pSecuTransactionTradeData->TransFlag));
            columns[3]->set(i, new Long(pSecuTransactionTradeData->SeqNo));
            columns[4]->set(i, new Int(pSecuTransactionTradeData->ChannelNo));
            columns[5]->set(i, getNewDate(pSecuTransactionTradeData->TradeDate));
            columns[6]->set(i, getNewTime(pSecuTransactionTradeData->TransactTime));
            columns[7]->set(i, new Double(pSecuTransactionTradeData->TrdPrice));
            columns[8]->set(i, new Long(pSecuTransactionTradeData->TrdVolume));
            columns[9]->set(i, new Double(pSecuTransactionTradeData->TrdMoney));
            columns[10]->set(i, new Long(pSecuTransactionTradeData->TrdBuyNo));
            columns[11]->set(i, new Long(pSecuTransactionTradeData->TrdSellNo));
            columns[12]->set(i, new Char(pSecuTransactionTradeData->TrdBSFlag));
            columns[13]->set(i, new Long(pSecuTransactionTradeData->BizIndex));
            if (receivedTimeFlag)
            {
                columns[14]->set(i, new Long(receivedTime));
            }
        }
    }
}

void setMultEntrustData(vector<ConstantSP> &columns, vector<CHSNsqSecuTransactionEntrustDataField *> items)
{
    CHSNsqSecuTransactionEntrustDataField *pSecuTransactionEntrustData = nullptr;
    for (int i = 0; i < items.size(); i++)
    {
        pSecuTransactionEntrustData = items[i];
        long long receivedTime = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
        if (getAllFieldNamesFlag)
        {
            columns[0]->set(i, new String(pSecuTransactionEntrustData->ExchangeID));
            columns[1]->set(i, new String(pSecuTransactionEntrustData->InstrumentID));
            columns[2]->set(i, new Int(pSecuTransactionEntrustData->TransFlag));
            columns[3]->set(i, new Long(pSecuTransactionEntrustData->SeqNo));
            columns[4]->set(i, new Int(pSecuTransactionEntrustData->ChannelNo));
            columns[5]->set(i, getNewDate(pSecuTransactionEntrustData->TradeDate));
            columns[6]->set(i, getNewTime(pSecuTransactionEntrustData->TransactTime));
            columns[7]->set(i, new Int(1));
            columns[8]->set(i, new Double(pSecuTransactionEntrustData->OrdPrice));
            columns[9]->set(i, new Long(pSecuTransactionEntrustData->OrdVolume));
            columns[10]->set(i, new Double(0));
            columns[11]->set(i, new Long(0));
            columns[12]->set(i, new Long(0));
            columns[13]->set(i, new Char(0));
            columns[14]->set(i, new Long(pSecuTransactionEntrustData->BizIndex));
            columns[15]->set(i, new Char(pSecuTransactionEntrustData->OrdSide));
            columns[16]->set(i, new Char(pSecuTransactionEntrustData->OrdType));
            columns[17]->set(i, new Long(pSecuTransactionEntrustData->OrdNo));
            columns[18]->set(i, new Long(pSecuTransactionEntrustData->TrdVolume));
            if (receivedTimeFlag)
            {
                columns[19]->set(i, new Long(receivedTime));
            }
        }
        else
        {
            columns[0]->set(i, new String(pSecuTransactionEntrustData->ExchangeID));
            columns[1]->set(i, new String(pSecuTransactionEntrustData->InstrumentID));
            columns[2]->set(i, new Int(pSecuTransactionEntrustData->TransFlag));
            columns[3]->set(i, new Long(pSecuTransactionEntrustData->SeqNo));
            columns[4]->set(i, new Int(pSecuTransactionEntrustData->ChannelNo));
            columns[5]->set(i, getNewDate(pSecuTransactionEntrustData->TradeDate));
            columns[6]->set(i, getNewTime(pSecuTransactionEntrustData->TransactTime));
            columns[7]->set(i, new Double(pSecuTransactionEntrustData->OrdPrice));
            columns[8]->set(i, new Long(pSecuTransactionEntrustData->OrdVolume));
            columns[9]->set(i, new Char(pSecuTransactionEntrustData->OrdSide));
            columns[10]->set(i, new Char(pSecuTransactionEntrustData->OrdType));
            columns[11]->set(i, new Long(pSecuTransactionEntrustData->OrdNo));
            columns[12]->set(i, new Long(pSecuTransactionEntrustData->BizIndex));
            if (columns.size() == 14)
            {
                columns[13]->set(i, new Long(receivedTime));
            }
        }
    }
}

void CHSNsqSpiImpl::asyncSnapshotHandle(std::vector<SnapData *> &items)
{
    INDEX insertedRows;
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
        vector<string> colNames = snapshotColumnNames;
        vector<DATA_TYPE> colTypes = snapshotTypes;
        if (getAllFieldNamesFlag)
        {
            columns = createVectors(3, items.size());
            colNames.insert(colNames.end(), addedSnapshotColumnNames.begin(), addedSnapshotColumnNames.end());
            colTypes.insert(colTypes.end(), addedSnapshotTypes.begin(), addedSnapshotTypes.end());
        }
        else
        {
            columns = createVectors(0, items.size());
        }
        setMultSnapshotData(columns, items);
        vector<ObjectSP> args = {new String(tablenames[index])};
        ConstantSP table = session->getFunctionDef("objByName")->call(session->getHeap().get(), args);
        TableSP tmp_table = Util::createTable(colNames, columns);
        vector<ObjectSP> args2 = {table, tmp_table};
        session->getFunctionDef("append!")->call(session->getHeap().get(), args2);
        setStatus(status[index], "");
    }
    catch (exception &e)
    {
        setStatus(status[index], e.what());
    }
}

// 回调函数，先创建一行vector，再设置值，然后将数据添加到tsp里，用mutex锁住
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
        // recsum++;
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
            throw RuntimeException("error ExchangeID");
        }
    }
    catch (const std::exception &e)
    {
        LOG_ERR("[PluginNsq] OnRtnSecuDepthMarketData function has problem because ", e.what(), " or can not malloc");
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
    //        pSecuDepthMarketData->UpdateTime, pSecuD·epthMarketData->PreClosePrice, pSecuDepthMarketData->ClosePrice,
    //        pSecuDepthMarketData->InstrumentTradeStatus, pSecuDepthMarketData->TradeVolume,

    //        pSecuDepthMarketData->BidPrice1, pSecuDepthMarketData->BidVolume1, Bid1Volume[0], Bid1Count, MaxBid1Count,
    //        pSecuDepthMarketData->AskPrice1, pSecuDepthMarketData->AskVolume1, Ask1Volume[0], Ask1Count,
    //        MaxAsk1Count);
}

// trade 和 orders混在了一起。。。他们都是 uTransaction，一个是type1，一个是type2
void CHSNsqSpiImpl::OnRspSecuTransactionSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (nRequestID == 10003)
    {
        subscribe_status[2] = 1;
    }
    else if (nRequestID == 10004)
    {
        subscribe_status[3] = 1;
    }
    else if (nRequestID == 10005)
    {
        subscribe_status[4] = 1;
    }
    else if (nRequestID == 10006)
    {
        subscribe_status[5] = 1;
    }
    if (pRspInfo->ErrorID != 0)
    { // 说明有错误
        throw RuntimeException(pRspInfo->ErrorMsg);
    }
    LOG_INFO("OnRspSecuTransactionSubscribe: nRequestID[", nRequestID, "], ErrorID[", pRspInfo->ErrorID,
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
    { // 说明有错误
        throw RuntimeException(pRspInfo->ErrorMsg);
    }
    LOG_INFO("OnRspSecuTransactionCancel: nRequestID[", nRequestID, "], ErrorID[", pRspInfo->ErrorID,
             "], ErrorMsg: ", pRspInfo->ErrorMsg);
}

void CHSNsqSpiImpl::asyncTradeHandle(std::vector<CHSNsqSecuTransactionTradeDataField *> &items)
{
    INDEX insertedRows;
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
        if (getAllFieldNamesFlag)
        {
            columns = createVectors(4, items.size());
            colNames = tradeAndOrdersMergedColumnNames;
            colTypes = tradeAndOrdersMergedTypes;
        }
        else
        {
            columns = createVectors(1, items.size());
            colNames = tradeColumnNames;
            colTypes = tradeTypes;
        }

        setMultTradeData(columns, items);

        vector<ObjectSP> args = {new String(tablenames[index])};
        ConstantSP table = session->getFunctionDef("objByName")->call(session->getHeap().get(), args);
        TableSP tmp_table = Util::createTable(colNames, columns);
        vector<ObjectSP> args2 = {table, tmp_table};
        session->getFunctionDef("append!")->call(session->getHeap().get(), args2);

        setStatus(status[index], "");
    }
    catch (exception &e)
    {
        setStatus(status[index], e.what());
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
            throw RuntimeException("error ExchangeID");
        }
    }
    catch (const std::exception &e)
    {
        LOG_ERR("[PluginNsq] OnRtnSecuTransactionTradeData function has problem because ", e.what(), " or can not malloc");
    }
}

void CHSNsqSpiImpl::asyncEntrustHandle(std::vector<CHSNsqSecuTransactionEntrustDataField *> &items)
{
    INDEX insertedRows;
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
            throw RuntimeException("error ExchangeID");
        }

        vector<ConstantSP> columns;
        vector<string> colNames;
        vector<DATA_TYPE> colTypes;
        if (getAllFieldNamesFlag)
        {
            columns = createVectors(4, items.size());
            colNames = tradeAndOrdersMergedColumnNames;
            colTypes = tradeAndOrdersMergedTypes;
        }
        else
        {
            columns = createVectors(2, items.size());
            colNames = ordersColumnNames;
            colTypes = ordersTypes;
        }
        setMultEntrustData(columns, items);

        vector<ObjectSP> args = {new String(tablenames[index])};
        ConstantSP table = session->getFunctionDef("objByName")->call(session->getHeap().get(), args);
        TableSP tmp_table = Util::createTable(colNames, columns);
        vector<ObjectSP> args2 = {table, tmp_table};
        session->getFunctionDef("append!")->call(session->getHeap().get(), args2);

        setStatus(status[index], "");
    }
    catch (exception &e)
    {
        setStatus(status[index], e.what());
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
            throw RuntimeException("error ExchangeID");
        }
    }
    catch (const std::exception &e)
    {
        LOG_ERR("[PluginNsq] OnRtnSecuTransactionEntrustData function has problem because ", e.what(), " or can not malloc");
    }
}

void CHSNsqSpiImpl::OnRtnSecuATPTransactionTradeData(CHSNsqSecuTransactionTradeDataField *pSecuTransactionTradeData) {}

void CHSNsqSpiImpl::OnRspQrySecuInstruments(CHSNsqSecuInstrumentStaticInfoField *pSecuInstrumentStaticInfo,
                                            CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    char sz_buf[1024];
    int buf_len;
    if (0 == m_iAllInstrCount)
    {
        LOG_INFO("RspQrySecuInstruments Successed !");
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
        if (f_sz_stock_init == NULL)
        {
            f_sz_stock_init = fopen("sz_stock_code.txt", "w+");
            if (f_sz_stock_init == NULL)
            {
                LOG_INFO("fopen sz_stock_code.txt fail: %s\n", strerror(errno));
            }
        }
        if (f_sz_stock_init)
        {
            fwrite(sz_buf, buf_len, 1, f_sz_stock_init);
            if (bIsLast)
            {
                fclose(f_sz_stock_init);
                f_sz_stock_init = NULL;
            }
        }
    }
    else
    {
        if (f_sh_stock_init == NULL)
        {
            f_sh_stock_init = fopen("sh_stock_code.txt", "w+");
            if (f_sh_stock_init == NULL)
            {
                LOG_INFO("fopen sh_stock_code.txt fail: %s\n", strerror(errno));
            }
        }
        if (f_sh_stock_init)
        {
            fwrite(sz_buf, buf_len, 1, f_sh_stock_init);
            if (bIsLast)
            {
                fclose(f_sh_stock_init);
                f_sh_stock_init = NULL;
            }
        }
    }

    m_iAllInstrCount++;
    if (bIsLast)
    {
        m_isAllInstrReady = true;
    }
}

/// Description: 期权订阅-行情应答
void CHSNsqSpiImpl::OnRspOptDepthMarketDataSubscribe(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    LOG_INFO("OnRspOptDepthMarketDataSubscribe: nRequestID[", nRequestID, "], ErrorID[", pRspInfo->ErrorID,
             "], ErrorMsg: ", pRspInfo->ErrorMsg);
}

/// Description: 期权订阅取消-行情应答
void CHSNsqSpiImpl::OnRspOptDepthMarketDataCancel(CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    LOG_INFO("OnRspOptDepthMarketDataCancel: nRequestID[", nRequestID, "], ErrorID[", pRspInfo->ErrorID,
             "], ErrorMsg: ", pRspInfo->ErrorMsg);
}

/// Description: 主推-期权行情
void CHSNsqSpiImpl::OnRtnOptDepthMarketData(CHSNsqOptDepthMarketDataField *pOptDepthMarketData)
{
    CHSNsqOptDepthMarketDataField *p = pOptDepthMarketData;

    LOG_INFO("%s,%lf,%lf,%lf,%lf,%lf,%lf,%" PRId64 ",%" PRId64 ",%lf,%lf,%lf,%lf,%lf,%lf,%" PRId64 ",%" PRId64
             ",%lf,%lf,%" PRId64 ",%c,%c,%lf,%" PRId64 ",%d,%" PRId64 "\n",
             p->InstrumentID, p->LastPrice, p->PreClosePrice, p->OpenPrice, p->HighPrice, p->LowPrice, p->ClosePrice,
             p->PreOpenInterest, p->OpenInterest, p->PreSettlementPrice, p->SettlementPrice, p->UpperLimitPrice,
             p->LowerLimitPrice, p->PreDelta, p->CurDelta, (uint64)p->TradeDate * 1000000000 + p->UpdateTime,
             p->TradeVolume, p->TradeBalance, p->AveragePrice, p->TradesNum, p->InstrumentTradeStatus,
             p->OpenRestriction[0], p->AuctionPrice, p->AuctionVolume, p->LastEnquiryTime, p->LeaveQty);
    int i;
    for (i = 0; i < 10; i++)
    {
        LOG_INFO("bid[%d]:%lf,%" PRId64 "\n", i, p->BidPrice[i], p->BidVolume[i]);
    }
    for (i = 0; i < 10; i++)
    {
        LOG_INFO("ask[%d]:%lf,%" PRId64 "\n", i, p->AskPrice[i], p->AskVolume[i]);
    }
}

/// Description: 获取当前交易日合约应答
void CHSNsqSpiImpl::OnRspQryOptInstruments(CHSNsqOptInstrumentStaticInfoField *pOptInstrumentStaticInfo,
                                           CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    char sz_buf[1024];
    int buf_len;
    CHSNsqOptInstrumentStaticInfoField *p = pOptInstrumentStaticInfo;
    if (0 == m_iAllInstrCount)
    {
        LOG_INFO("RspQryOptInstruments Successed !");
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
        if (f_sz_opt_init == NULL)
        {
            f_sz_opt_init = fopen("sz_opt_code.txt", "w+");
            if (f_sz_opt_init == NULL)
            {
                LOG_INFO("fopen sz_opt_code.txt fail: %s\n", strerror(errno));
            }
        }
        if (f_sz_opt_init)
        {
            fwrite(sz_buf, buf_len, 1, f_sz_opt_init);
            if (bIsLast)
            {
                fclose(f_sz_opt_init);
                f_sz_opt_init = NULL;
            }
        }
    }
    else
    {
        if (f_sh_opt_init == NULL)
        {
            f_sh_opt_init = fopen("sh_opt_code.txt", "w+");
            if (f_sh_opt_init == NULL)
            {
                LOG_INFO("fopen sh_opt_code.txt fail: %s\n", strerror(errno));
            }
        }
        if (f_sh_opt_init)
        {
            fwrite(sz_buf, buf_len, 1, f_sh_opt_init);
            if (bIsLast)
            {
                fclose(f_sh_opt_init);
                f_sh_opt_init = NULL;
            }
        }
    }

    m_iAllInstrCount++;
    if (bIsLast)
    {
        m_isAllInstrReady = true;
    }
}

/// Description: 获取合约的最新快照信息应答
void CHSNsqSpiImpl::OnRspQryOptDepthMarketData(CHSNsqOptDepthMarketDataField *pOptDepthMarketData,
                                               CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {}

void CHSNsqSpiImpl::OnRspSecuTransactionData(CHSNsqSecuTransactionDataField *pSecuTransactionData,
                                             CHSNsqRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    LOG_INFO("[OnRspSecuTransactionData]: CHSNsqRspInfoField ErrorID %d ErrorMsg %s "
             "bIsLast %d\n",
             pRspInfo->ErrorID, pRspInfo->ErrorMsg, bIsLast);
    if (pRspInfo->ErrorID == 0)
    {
        CHSNsqSecuTransactionDataField *p = pSecuTransactionData;
        if (p->TransType == HS_TRANS_Trade)
        {
            LOG_INFO("[OnRspSecuTransactionData]: ExchangeID %s, InstrumentID %s, "
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
            LOG_INFO("[OnRspSecuTransactionData]: ExchangeID %s, InstrumentID %s, "
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
            LOG_INFO("error TransType:%c\n", p->TransType);
        }
    }
}

void CHSNsqSpiImpl::OnRspSecuTransactionDataTimeout(int nRequestID)
{
    LOG_INFO("OnRspSecuTransactionDataTimeout nRequestID=%d\n", nRequestID);
}
