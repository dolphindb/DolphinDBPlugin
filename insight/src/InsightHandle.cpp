#include "InsightHandle.h"
#include "Logger.h"
#include <iostream>

using namespace std;

class InsertDfsTableRun : public Runnable{
public:
    InsertDfsTableRun(InsightHandle &handle):
        handle_(handle){
    }
    virtual void run(){
        vector<TableSP> tables;
        INDEX insertedRows;
        string errMsg;
        tables.clear();
        vector<ConstantSP> appendValues(1);
        vector<ConstantSP> tableInsertValues(2);
        bool isContinue = true;
        while(isContinue){
            handle_.dfsTables_.blockingPop(tables,10);
            LOG("pop dfs data ",tables.size());
            TableSP firstTable;
            for(auto &one:tables){
                if(one.isNull()){
                    isContinue = false;
                    break;
                }
                if(firstTable.isNull()){
                    firstTable=one;
                }else{
                    appendValues[0]=one;
                    if(firstTable->append(appendValues,insertedRows,errMsg)){
                        LOG_ERR("append table failed.");
                    }
                }
                
            }
            if(firstTable.isNull())
                continue;
            tableInsertValues[0]=handle_.handleStock_;
            tableInsertValues[1]=firstTable;
            LOG("insert dfs data ",firstTable->rows());
            handle_.session_->getFunctionDef("tableInsert")->call(handle_.session_->getHeap().get(), tableInsertValues);
        }
    }
private:
    InsightHandle &handle_;
};

InsightHandle::InsightHandle(SessionSP session, DictionarySP handles):
    columnNamesStock{
        "HTSCSecurityID",
        "MDDate",
        "MDTime",
        "securityIDSource",
        "PreClosePx",
        "TotalVolumeTrade",
        "TotalValueTrade",
        "LastPx",
        "OpenPx",
        "HighPx",
        "LowPx",
        "DiffPx1",
        "TotalBuyQty",
        "TotalSellQty",
        "WeightedAvgBuyPx",
        "WeightedAvgSellPx",
        "BuyPrice1",
        "BuyPrice2",
        "BuyPrice3",
        "BuyPrice4",
        "BuyPrice5",
        "BuyPrice6",
        "BuyPrice7",
        "BuyPrice8",
        "BuyPrice9",
        "BuyPrice10",
        "BuyOrderQty1",
        "BuyOrderQty2",
        "BuyOrderQty3",
        "BuyOrderQty4",
        "BuyOrderQty5",
        "BuyOrderQty6",
        "BuyOrderQty7",
        "BuyOrderQty8",
        "BuyOrderQty9",
        "BuyOrderQty10",
        "SellPrice1",
        "SellPrice2",
        "SellPrice3",
        "SellPrice4",
        "SellPrice5",
        "SellPrice6",
        "SellPrice7",
        "SellPrice8",
        "SellPrice9",
        "SellPrice10",
        "SellOrderQty1",
        "SellOrderQty2",
        "SellOrderQty3",
        "SellOrderQty4",
        "SellOrderQty5",
        "SellOrderQty6",
        "SellOrderQty7",
        "SellOrderQty8",
        "SellOrderQty9",
        "SellOrderQty10",
        "BuyOrder1",
        "BuyOrder2",
        "BuyOrder3",
        "BuyOrder4",
        "BuyOrder5",
        "BuyOrder6",
        "BuyOrder7",
        "BuyOrder8",
        "BuyOrder9",
        "BuyOrder10",
        "SellOrder1",
        "SellOrder2",
        "SellOrder3",
        "SellOrder4",
        "SellOrder5",
        "SellOrder6",
        "SellOrder7",
        "SellOrder8",
        "SellOrder9",
        "SellOrder10",
        "BuyNumOrders1",
        "BuyNumOrders2",
        "BuyNumOrders3",
        "BuyNumOrders4",
        "BuyNumOrders5",
        "BuyNumOrders6",
        "BuyNumOrders7",
        "BuyNumOrders8",
        "BuyNumOrders9",
        "BuyNumOrders10",
        "SellNumOrders1",
        "SellNumOrders2",
        "SellNumOrders3",
        "SellNumOrders4",
        "SellNumOrders5",
        "SellNumOrders6",
        "SellNumOrders7",
        "SellNumOrders8",
        "SellNumOrders9",
        "SellNumOrders10",
        "TradingPhaseCode",
        "UpdateTime1",
    },
    columnSizeStock(columnNamesStock.size()),

    columnNamesIndex{
        "MDDate",
        "MDTime",
        "HTSCSecurityID",
        "LastPx",
        "HighPx",
        "LowPx",
        "TotalVolumeTrade",
        "TotalValueTrade",
        "TradingPhaseCode",
        "UpdateTime1",
    },
    columnSizeIndex(columnNamesIndex.size()),

    columnNamesFuture{
        "HTSCSecurityID",
        "MDDate",
        "MDTime",
        "securityIDSource",
        "PreClosePx",
        "TotalVolumeTrade",
        "TotalValueTrade",
        "LastPx",
        "OpenPx",
        "HighPx",
        "LowPx",
        "PreOpenInterestPx",
        "PreSettlePrice",
        "OpenInterest",
        "BuyPrice1",
        "BuyPrice2",
        "BuyPrice3",
        "BuyPrice4",
        "BuyPrice5",
        "BuyOrderQty1",
        "BuyOrderQty2",
        "BuyOrderQty3",
        "BuyOrderQty4",
        "BuyOrderQty5",
        "SellPrice1",
        "SellPrice2",
        "SellPrice3",
        "SellPrice4",
        "SellPrice5",
        "SellOrderQty1",
        "SellOrderQty2",
        "SellOrderQty3",
        "SellOrderQty4",
        "SellOrderQty5",
        "TradingPhaseCode",
        "UpdateTime1",
    },
    columnSizeFuture(columnNamesFuture.size()),

    columnNamesTransaction{
        "HTSCSecurityID",
        "MDDate",
        "MDTime",
        "securityIDSource",
        "TradeIndex",
        "TradeBuyNo",
        "TradeSellNo",
        "TradeBSFlag",
        "TradePrice",
        "TradeQty",
        "TradeMoney",
        "ApplSeqNum",
        "UpdateTime1",
    },
    columnSizeTransaction(columnNamesTransaction.size()),

    columnNamesOrder{
        "HTSCSecurityID",
        "MDDate",
        "MDTime",
        "securityIDSource",
        "securityType",
        "OrderIndex",
        "SourceType",
        "OrderType",
        "OrderPrice",
        "OrderQty",
        "OrderBSFlag",
        "BuyNo",
        "SellNo",
        "ApplSeqNum",
        "ChannelNo",
        "UpdateTime1",
    },
    columnSizeOrder(columnNamesOrder.size()),
    session_(session)
{
    handleStock_ = handles->getMember("StockType");
    if(!handleStock_.isNull() && !handleStock_->isNull()) {
        if (handleStock_->getForm() == DF_TABLE) {
            if (handleStock_->columns() != (INDEX)columnSizeStock) {
                throw RuntimeException("the schema of tables should be the same");
            }
            lockStock_ = ((TableSP)handleStock_)->getLock();
        } else {
            throw RuntimeException("handle's values should be tables");
        }
    }

    handleIndex = handles->getMember("IndexType");
    if(!handleIndex.isNull() && !handleIndex->isNull()) {
        if (handleIndex->getForm() == DF_TABLE) {
            if (handleIndex->columns() != (INDEX)columnSizeIndex) {
                throw RuntimeException("the schema of tables should be the same");
            }
            lockIndex = ((TableSP)handleIndex)->getLock();
        } else {
            throw RuntimeException("handle's values should be tables");
        }
    }

    handleFuture = handles->getMember("FuturesType");
    if(!handleFuture.isNull() && !handleFuture->isNull()) {
        if (handleFuture->getForm() == DF_TABLE) {
            if (handleFuture->columns() != (INDEX)columnSizeFuture) {
                throw RuntimeException("the schema of tables should be the same");
            }
            lockFuture = ((TableSP)handleFuture)->getLock();
        } else {
            throw RuntimeException("handle's values should be tables");
        }
    }

    handleTransaction = handles->getMember("StockTypeTransaction");
    if(!handleTransaction.isNull() && !handleTransaction->isNull()) {
        if (handleTransaction->getForm() == DF_TABLE) {
            if (handleTransaction->columns() != (INDEX)columnSizeTransaction) {
                throw RuntimeException("the schema of tables should be the same");
            }
            lockTransaction = ((TableSP)handleTransaction)->getLock();
        } else {
            throw RuntimeException("handle's values should be tables");
        }
    }
    
    handleOrder = handles->getMember("StockTypeOrder");
    if(!handleOrder.isNull() && !handleOrder->isNull()) {
        if (handleOrder->getForm() == DF_TABLE) {
            if (handleOrder->columns() != (INDEX)columnSizeOrder) {
                throw RuntimeException("the schema of tables should be the same");
            }
            lockOrder = ((TableSP)handleOrder)->getLock();
        } else {
            throw RuntimeException("handle's values should be tables");
        }
    }
    dfsThread_ = new Thread(new InsertDfsTableRun(*this));
    dfsThread_->start();
}

InsightHandle::~InsightHandle(){
    dfsTables_.push(nullptr);
    dfsThread_->join();
}

void InsightHandle::OnMarketData(const com::htsc::mdc::insight::model::MarketData &record){
    long long receiveTime = Util::getEpochTime();
    switch (record.marketdatatype()) {
        case MD_TICK: {
            if (!handleStock_->isNull() && record.has_mdstock()) {
                thread_local ColumnsStock columnsStock(columnNamesStock, columnSizeStock);
                setColumnsStock(columnsStock.columns, receiveTime, record.mdstock());
                handleStockData(columnsStock.columns);
            } else if(!handleIndex->isNull() && record.has_mdindex()) {
                thread_local ColumnsIndex columnsIndex(columnNamesIndex, columnSizeIndex);
                setColumnsIndex(columnsIndex.columns, receiveTime, record.mdindex());
                handleIndexData(columnsIndex.columns);
            } else if(!handleFuture->isNull() && record.has_mdfuture()) {
                thread_local ColumnsFuture columnsFuture(columnNamesFuture, columnSizeFuture);
                setColumnsFuture(columnsFuture.columns, receiveTime, record.mdfuture());
                handleFutureData(columnsFuture.columns);
            }
            break;
        }

        case MD_TRANSACTION: {
            if(!handleTransaction->isNull() && record.has_mdtransaction()) {
                thread_local ColumnsTransaction columnsTransaction(columnNamesTransaction, columnSizeTransaction);
                setColumnsTransaction(columnsTransaction.columns, receiveTime, record.mdtransaction());
                handleTransactionData(columnsTransaction.columns);
            }
            break;
        }

        case MD_ORDER: {
            if (!handleOrder->isNull() && record.has_mdorder()) {
                thread_local ColumnsOrder columnsOrder(columnNamesOrder, columnSizeOrder);
                setColumnsOrder(columnsOrder.columns, receiveTime, record.mdorder());
                handleOrderData(columnsOrder.columns);
            }
            break;
        }
        default: {
            break;
        }
    }
}

void InsightHandle::OnLoginSuccess(){
    LOG_INFO("[PluginInsight] Login Success");
}

void InsightHandle::OnLoginFailed(int error_no, const string &message){
    LOG_ERR("[PluginInsight] Login Failed. Error number: ", error_no, ". Error message: ", message);
}

void InsightHandle::OnNoConnections(){
    LOG_ERR("[PluginInsight] No Connections");
}

void InsightHandle::OnGeneralError(const com::htsc::mdc::insight::model::InsightErrorContext &context){
    LOG_INFO("[PluginInsight] PARSE message OnGeneralError: ", context.message());
}

void InsightHandle::ColumnsStock::createColumnsStock(std::vector<ConstantSP> &columns, INDEX colNum){
    columns[0] = Util::createVector(DT_SYMBOL, colNum, colNum);
    columns[1] = Util::createVector(DT_DATE, colNum, colNum);
    columns[2] = Util::createVector(DT_TIME, colNum, colNum);
    columns[3] = Util::createVector(DT_SYMBOL, colNum, colNum);

    columns[4] = Util::createVector(DT_LONG, colNum, colNum);
    columns[5] = Util::createVector(DT_LONG, colNum, colNum);
    columns[6] = Util::createVector(DT_LONG, colNum, colNum);
    columns[7] = Util::createVector(DT_LONG, colNum, colNum);
    columns[8] = Util::createVector(DT_LONG, colNum, colNum);
    columns[9] = Util::createVector(DT_LONG, colNum, colNum);
    columns[10] = Util::createVector(DT_LONG, colNum, colNum);
    columns[11] = Util::createVector(DT_LONG, colNum, colNum);
    columns[12] = Util::createVector(DT_LONG, colNum, colNum);
    columns[13] = Util::createVector(DT_LONG, colNum, colNum);
    columns[14] = Util::createVector(DT_LONG, colNum, colNum);
    columns[15] = Util::createVector(DT_LONG, colNum, colNum);

    for(int i = 0; i < 10 * 8; i++)
        columns[16+i] = Util::createVector(DT_LONG, colNum, colNum);

    columns[96] = Util::createVector(DT_SYMBOL, colNum, colNum);
    columns[97] = Util::createVector(DT_TIMESTAMP, colNum, colNum);
}

void InsightHandle::setColumnsStock(std::vector<ConstantSP> &columns, long long receiveTime, const com::htsc::mdc::insight::model::MDStock &record, int index){
    columns[0]->setString(index, record.htscsecurityid());
    columns[1]->setInt(index, countDays(record.mddate()));
    columns[2]->setInt(index, realTime(record.mdtime()));
    columns[3]->setString(index, com::htsc::mdc::model::ESecurityIDSource_Name(record.securityidsource()));

    columns[4]->setLong(index, record.preclosepx());
    columns[5]->setLong(index, record.totalvolumetrade());
    columns[6]->setLong(index, record.totalvaluetrade());
    columns[7]->setLong(index, record.lastpx());
    columns[8]->setLong(index, record.openpx());
    columns[9]->setLong(index, record.highpx());
    columns[10]->setLong(index, record.lowpx());
    columns[11]->setLong(index, record.diffpx1());
    columns[12]->setLong(index, record.totalbuyqty());
    columns[13]->setLong(index, record.totalsellqty());
    columns[14]->setLong(index, record.weightedavgbuypx());
    columns[15]->setLong(index, record.weightedavgsellpx());

    for(int i = 0; i < record.buypricequeue_size() && i < 10; i++)
        columns[16+i]->set(index, new Long(record.buypricequeue(i)));
    for(int i = record.buypricequeue_size(); i < 10; i++)
        columns[16+i]->set(index, Util::createNullConstant(DT_LONG));

    for(int i = 0; i < record.buyorderqtyqueue_size() && i < 10; i++)
        columns[26+i]->set(index, new Long(record.buyorderqtyqueue(i)));
    for(int i = record.buyorderqtyqueue_size(); i < 10; i++)
        columns[26+i]->set(index, Util::createNullConstant(DT_LONG));

    for(int i = 0; i < record.sellpricequeue_size() && i < 10; i++)
        columns[36+i]->set(index, new Long(record.sellpricequeue(i)));
    for(int i = record.sellpricequeue_size(); i < 10; i++)
        columns[36+i]->set(index, Util::createNullConstant(DT_LONG));

    for(int i = 0; i < record.sellorderqtyqueue_size() && i < 10; i++)
        columns[46+i]->set(index, new Long(record.sellorderqtyqueue(i)));
    for(int i = record.sellorderqtyqueue_size(); i < 10; i++)
        columns[46+i]->set(index, Util::createNullConstant(DT_LONG));

    for(int i = 0; i < record.buyorderqueue_size() && i < 10; i++)
        columns[56+i]->set(index, new Long(record.buyorderqueue(i)));
    for(int i = record.buyorderqueue_size(); i < 10; i++)
        columns[56+i]->set(index, Util::createNullConstant(DT_LONG));

    for(int i = 0; i < record.sellorderqueue_size() && i < 10; i++)
        columns[66+i]->set(index, new Long(record.sellorderqueue(i)));
    for(int i = record.sellorderqueue_size(); i < 10; i++)
        columns[66+i]->set(index, Util::createNullConstant(DT_LONG));

    for(int i = 0; i < record.buynumordersqueue_size() && i < 10; i++)
        columns[76+i]->set(index, new Long(record.buynumordersqueue(i)));
    for(int i = record.buynumordersqueue_size(); i < 10; i++)
        columns[76+i]->set(index, Util::createNullConstant(DT_LONG));

    for(int i = 0; i < record.sellnumordersqueue_size() && i < 10; i++)
        columns[86+i]->set(index, new Long(record.sellnumordersqueue(i)));
    for(int i = record.sellnumordersqueue_size(); i < 10; i++)
        columns[86+i]->set(index, Util::createNullConstant(DT_LONG));

    columns[96]->setString(index, record.tradingphasecode());

    columns[97]->setLong(index, receiveTime);
}

void InsightHandle::handleStockData(vector<ConstantSP> &columns){
    assert(columns.size() == columnSizeStock);
    // INDEX insertedRows;
    // string errMsg;
    TableSP data = Util::createTable(columnNamesStock, columns);
    if (((TableSP)handleStock_)->getTableType() == REALTIMETBL) {
        LockGuard<Mutex> _(lockStock_);
        vector<ConstantSP> args = {handleStock_, data};
        HeapSP heap = session_->getHeap();
        session_->getFunctionDef("tableInsert")->call(heap.get(), args);
    } else {
        LOG("push dfs data ", dfsTables_.size());
        dfsTables_.push(data);
    }
}

void InsightHandle::ColumnsIndex::createColumnsIndex(std::vector<ConstantSP> &columns, INDEX colNum){
    columns[0] = Util::createVector(DT_DATE, colNum, colNum);
    columns[1] = Util::createVector(DT_TIME, colNum, colNum);
    columns[2] = Util::createVector(DT_SYMBOL, colNum, colNum);
    columns[3] = Util::createVector(DT_LONG, colNum, colNum);
    columns[4] = Util::createVector(DT_LONG, colNum, colNum);
    columns[5] = Util::createVector(DT_LONG, colNum, colNum);
    columns[6] = Util::createVector(DT_LONG, colNum, colNum);
    columns[7] = Util::createVector(DT_LONG, colNum, colNum);
    columns[8] = Util::createVector(DT_SYMBOL, colNum, colNum);
    columns[9] = Util::createVector(DT_TIMESTAMP, colNum, colNum);
}

void InsightHandle::setColumnsIndex(std::vector<ConstantSP> &columns, long long receiveTime, const com::htsc::mdc::insight::model::MDIndex &record, int index){
    columns[0]->setInt(index, countDays(record.mddate()));
    columns[1]->setInt(index, realTime(record.mdtime()));
    columns[2]->setString(index, record.htscsecurityid());
    columns[3]->setLong(index, record.lastpx());
    columns[4]->setLong(index, record.highpx());
    columns[5]->setLong(index, record.lowpx());
    columns[6]->setLong(index, record.totalvolumetrade());
    columns[7]->setLong(index, record.totalvaluetrade());
    columns[8]->setString(index, record.tradingphasecode());
    columns[9]->setLong(index, receiveTime);
}

void InsightHandle::handleIndexData(vector<ConstantSP> &columns){
    assert(columns.size() == columnSizeIndex);
    // INDEX insertedRows;
    // string errMsg;
    LockGuard<Mutex> _(lockIndex);
    TableSP data = Util::createTable(columnNamesIndex, columns);
    vector<ConstantSP> args = {handleIndex, data};
    session_->getFunctionDef("tableInsert")->call(session_->getHeap().get(), args);
}

void InsightHandle::ColumnsFuture::createColumnsFuture(std::vector<ConstantSP> &columns, INDEX colNum){
    columns[0] = Util::createVector(DT_SYMBOL, colNum, colNum);
    columns[1] = Util::createVector(DT_DATE, colNum, colNum);
    columns[2] = Util::createVector(DT_TIME, colNum, colNum);
    columns[3] = Util::createVector(DT_SYMBOL, colNum, colNum);

    columns[4] = Util::createVector(DT_LONG, colNum, colNum);
    columns[5] = Util::createVector(DT_LONG, colNum, colNum);
    columns[6] = Util::createVector(DT_LONG, colNum, colNum);
    columns[7] = Util::createVector(DT_LONG, colNum, colNum);
    columns[8] = Util::createVector(DT_LONG, colNum, colNum);
    columns[9] = Util::createVector(DT_LONG, colNum, colNum);
    columns[10] = Util::createVector(DT_LONG, colNum, colNum);
    columns[11] = Util::createVector(DT_LONG, colNum, colNum);
    columns[12] = Util::createVector(DT_LONG, colNum, colNum);
    columns[13] = Util::createVector(DT_LONG, colNum, colNum);

    // BuyPrice, BuyOrderQty, SellPrice, SellOrderQty
    for(int i = 0; i < 5 * 4; i++)
        columns[14+i] = Util::createVector(DT_LONG, colNum, colNum);

    columns[34] = Util::createVector(DT_SYMBOL, colNum, colNum);
    columns[35] = Util::createVector(DT_TIMESTAMP, colNum, colNum);
}

void InsightHandle::setColumnsFuture(std::vector<ConstantSP> &columns, long long receiveTime, const com::htsc::mdc::insight::model::MDFuture &record, int index){
    columns[0]->setString(index, record.htscsecurityid());
    columns[1]->setInt(index, countDays(record.mddate()));
    columns[2]->setInt(index, realTime(record.mdtime()));
    columns[3]->setString(index, com::htsc::mdc::model::ESecurityIDSource_Name(record.securityidsource()));

    columns[4]->setLong(index, record.preclosepx());
    columns[5]->setLong(index, record.totalvolumetrade());
    columns[6]->setLong(index, record.totalvaluetrade());
    columns[7]->setLong(index, record.lastpx());
    columns[8]->setLong(index, record.openpx());
    columns[9]->setLong(index, record.highpx());
    columns[10]->setLong(index, record.lowpx());
    columns[11]->setLong(index, record.preopeninterest());
    columns[12]->setLong(index, record.presettleprice());
    columns[13]->setLong(index, record.openinterest());

    for(int i = 0; i < record.buypricequeue_size() && i < 5; i++)
        columns[14+i]->set(index, new Long(record.buypricequeue(0+i)));
    for(int i = record.buypricequeue_size(); i < 5; i++)
        columns[14+i]->set(index, Util::createNullConstant(DT_LONG));

    for(int i = 0; i < record.buyorderqtyqueue_size() && i < 5; i++)
        columns[19+i]->set(index, new Long(record.buyorderqtyqueue(0+i)));
    for(int i = record.buyorderqtyqueue_size(); i < 5; i++)
        columns[19+i]->set(index, Util::createNullConstant(DT_LONG));

    for(int i = 0; i < record.sellpricequeue_size() && i < 5; i++)
        columns[24+i]->set(index, new Long(record.sellpricequeue(0+i)));
    for(int i = record.sellpricequeue_size(); i < 5; i++)
        columns[24+i]->set(index, Util::createNullConstant(DT_LONG));

    for(int i = 0; i < record.sellorderqtyqueue_size() && i < 5; i++)
        columns[29+i]->set(index, new Long(record.sellorderqtyqueue(0+i)));
    for(int i = record.sellorderqtyqueue_size(); i < 5; i++)
        columns[29+i]->set(index, Util::createNullConstant(DT_LONG));

    columns[34]->setString(index, record.tradingphasecode());
    columns[35]->setLong(index, receiveTime);
}

void InsightHandle::handleFutureData(vector<ConstantSP> &columns){
    assert(columns.size() == columnSizeFuture);
    LockGuard<Mutex> _(lockFuture);
    TableSP data = Util::createTable(columnNamesFuture, columns);
    vector<ConstantSP> args = {handleFuture, data};
    session_->getFunctionDef("tableInsert")->call(session_->getHeap().get(), args);
}

void InsightHandle::ColumnsTransaction::createColumnsTransaction(std::vector<ConstantSP> &columns, INDEX colNum){
    columns[0] = Util::createVector(DT_SYMBOL, colNum, colNum);
    columns[1] = Util::createVector(DT_DATE, colNum, colNum);
    columns[2] = Util::createVector(DT_TIME, colNum, colNum);
    columns[3] = Util::createVector(DT_SYMBOL, colNum, colNum);

    columns[4] = Util::createVector(DT_LONG, colNum, colNum);
    columns[5] = Util::createVector(DT_LONG, colNum, colNum);
    columns[6] = Util::createVector(DT_LONG, colNum, colNum);
    columns[7] = Util::createVector(DT_INT, colNum, colNum);
    columns[8] = Util::createVector(DT_LONG, colNum, colNum);
    columns[9] = Util::createVector(DT_LONG, colNum, colNum);
    columns[10] = Util::createVector(DT_LONG, colNum, colNum);
    columns[11] = Util::createVector(DT_LONG, colNum, colNum);

    columns[12] = Util::createVector(DT_TIMESTAMP, colNum, colNum);
}

void InsightHandle::setColumnsTransaction(std::vector<ConstantSP> &columns, long long receiveTime, const com::htsc::mdc::insight::model::MDTransaction &record, int index){
    columns[0]->setString(index, record.htscsecurityid());
    columns[1]->setInt(index, countDays(record.mddate()));
    columns[2]->setInt(index, realTime(record.mdtime()));
    columns[3]->setString(index, com::htsc::mdc::model::ESecurityIDSource_Name(record.securityidsource()));

    columns[4]->setLong(index, record.tradeindex());
    columns[5]->setLong(index, record.tradebuyno());
    columns[6]->setLong(index, record.tradesellno());
    columns[7]->setInt(index, record.tradebsflag());
    columns[8]->setLong(index, record.tradeprice());
    columns[9]->setLong(index, record.tradeqty());
    columns[10]->setLong(index, record.trademoney());
    columns[11]->setLong(index, record.applseqnum());

    columns[12]->setLong(index, receiveTime);
}

void InsightHandle::handleTransactionData(vector<ConstantSP> &columns){
    assert(columns.size() == columnSizeTransaction);
    // INDEX insertedRows;
    // string errMsg;
    LockGuard<Mutex> _(lockTransaction);
    TableSP data = Util::createTable(columnNamesTransaction, columns);
    vector<ConstantSP> args = {handleTransaction, data};
    HeapSP heap = session_->getHeap();
    session_->getFunctionDef("tableInsert")->call(heap.get(), args);
}

void InsightHandle::ColumnsOrder::createColumnsOrder(std::vector<ConstantSP> &columns, INDEX colNum){
    columns[0] = Util::createVector(DT_SYMBOL, colNum, colNum);
    columns[1] = Util::createVector(DT_DATE, colNum, colNum);
    columns[2] = Util::createVector(DT_TIME, colNum, colNum);
    columns[3] = Util::createVector(DT_SYMBOL, colNum, colNum);
    columns[4] = Util::createVector(DT_SYMBOL, colNum, colNum);

    columns[5] = Util::createVector(DT_LONG, colNum, colNum);
    columns[6] = Util::createVector(DT_INT, colNum, colNum);
    columns[7] = Util::createVector(DT_INT, colNum, colNum);
    columns[8] = Util::createVector(DT_LONG, colNum, colNum);
    columns[9] = Util::createVector(DT_LONG, colNum, colNum);

    columns[10] = Util::createVector(DT_INT, colNum, colNum);
    columns[11] = Util::createVector(DT_LONG, colNum, colNum);
    columns[12] = Util::createVector(DT_LONG, colNum, colNum);
    columns[13] = Util::createVector(DT_LONG, colNum, colNum);
    columns[14] = Util::createVector(DT_INT, colNum, colNum);

    columns[15] = Util::createVector(DT_TIMESTAMP, colNum, colNum);
}

void InsightHandle::setColumnsOrder(std::vector<ConstantSP> &columns, long long receiveTime, const com::htsc::mdc::insight::model::MDOrder &record, int index){
    columns[0]->setString(index, record.htscsecurityid());
    columns[1]->setInt(index, countDays(record.mddate()));
    columns[2]->setInt(index, realTime(record.mdtime()));
    columns[3]->setString(index, com::htsc::mdc::model::ESecurityIDSource_Name(record.securityidsource()));
    columns[4]->setString(index, com::htsc::mdc::model::ESecurityType_Name(record.securitytype()));

    columns[5]->setLong(index, record.orderindex());
    columns[6]->setInt(index, 0);
    columns[7]->setInt(index, record.ordertype());
    columns[8]->setLong(index, record.orderprice());
    columns[9]->setLong(index, record.orderqty());

    columns[10]->setInt(index, record.orderbsflag());
    columns[11]->setLong(index, record.orderno());
    columns[12]->setLong(index, record.orderno());
    columns[13]->setLong(index, record.applseqnum());
    columns[14]->setInt(index, record.channelno());

    columns[15]->setLong(index, receiveTime);
}

void InsightHandle::handleOrderData(vector<ConstantSP> &columns){
    LockGuard<Mutex> _(lockOrder);
    TableSP data = Util::createTable(columnNamesOrder, columns);
    vector<ConstantSP> args = {handleOrder, data};
    session_->getFunctionDef("tableInsert")->call(session_->getHeap().get(), args);
}