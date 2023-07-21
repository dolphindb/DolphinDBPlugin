#include "InsightHandle.h"
#include "Logger.h"
#include <iostream>

using namespace std;

InsightHandle::InsightHandle(SessionSP session, DictionarySP handles):
    
    columnNamesStock_(InsightStockTableMeta::COLNAMES),
    columnSizeStock_(columnNamesStock_.size()),

    columnNamesIndex_(InsightIndexTableMeta::COLNAMES),
    columnSizeIndex_(columnNamesIndex_.size()),

    columnNamesFuture_(InsightFuturesTableMeta::COLNAMES),
    columnSizeFuture_(columnNamesFuture_.size()),

    columnNamesTransaction_(InsightTransactionTableMeta::COLNAMES),
    columnSizeTransaction_(columnNamesTransaction_.size()),

    columnNamesOrder_(InsightOrderTableMeta::COLNAMES),
    columnSizeOrder_(columnNamesOrder_.size())
{
    session_ = session;
    handleStock_ = handles->getMember("StockTick");
    if(!handleStock_.isNull() && !handleStock_->isNull()) {
        if (handleStock_->getForm() == DF_TABLE) {
            if (handleStock_->columns() != (INDEX)columnSizeStock_) {
                throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "the schema of tables should be the same");
            }
            lockStock_ = ((TableSP)handleStock_)->getLock();
        } else {
            throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "handle's values should be tables");
        }
    }

    handleIndex_ = handles->getMember("IndexTick");
    if(!handleIndex_.isNull() && !handleIndex_->isNull()) {
        if (handleIndex_->getForm() == DF_TABLE) {
            if (handleIndex_->columns() != (INDEX)columnSizeIndex_) {
                throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "the schema of tables should be the same");
            }
            lockIndex_ = ((TableSP)handleIndex_)->getLock();
        } else {
            throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "handle's values should be tables");
        }
    }

    handleFuture_ = handles->getMember("FuturesTick");
    if(!handleFuture_.isNull() && !handleFuture_->isNull()) {
        if (handleFuture_->getForm() == DF_TABLE) {
            if (handleFuture_->columns() != (INDEX)columnSizeFuture_) {
                throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "the schema of tables should be the same");
            }
            lockFuture_ = ((TableSP)handleFuture_)->getLock();
        } else {
            throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "handle's values should be tables");
        }
    }

    handleTransaction_ = handles->getMember("StockTransaction");
    if(!handleTransaction_.isNull() && !handleTransaction_->isNull()) {
        if (handleTransaction_->getForm() == DF_TABLE) {
            if (handleTransaction_->columns() != (INDEX)columnSizeTransaction_) {
                throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "the schema of tables should be the same");
            }
            lockTransaction_ = ((TableSP)handleTransaction_)->getLock();
        } else {
            throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "handle's values should be tables");
        }
    }
    
    handleOrder_ = handles->getMember("StockOrder");
    if(!handleOrder_.isNull() && !handleOrder_->isNull()) {
        if (handleOrder_->getForm() == DF_TABLE) {
            if (handleOrder_->columns() != (INDEX)columnSizeOrder_) {
                throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "the schema of tables should be the same");
            }
            lockOrder_ = ((TableSP)handleOrder_)->getLock();
        } else {
            throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "handle's values should be tables");
        }
    }
}

InsightHandle::~InsightHandle(){
    
}

void InsightHandle::OnMarketData(const com::htsc::mdc::insight::model::MarketData &record){
    try{
        long long receiveTime = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
        switch (record.marketdatatype()) {
            case MD_TICK: {
                if (!handleStock_.isNull()&&!handleStock_->isNull() && record.has_mdstock()) {
                    thread_local ColumnsStock columnsStock(columnNamesStock_, columnSizeStock_);
                    setColumnsStock(columnsStock.columns, receiveTime, record.mdstock());
                    handleStockData(columnsStock.columns);
                } else if (!handleIndex_.isNull()&&!handleIndex_->isNull() && record.has_mdindex()) {
                    thread_local ColumnsIndex columnsIndex(columnNamesIndex_, columnSizeIndex_);
                    setColumnsIndex(columnsIndex.columns, receiveTime, record.mdindex());
                    handleIndexData(columnsIndex.columns);
                } else if (!handleFuture_.isNull()&&!handleFuture_->isNull() && record.has_mdfuture()) {
                    thread_local ColumnsFuture columnsFuture(columnNamesFuture_, columnSizeFuture_);
                    setColumnsFuture(columnsFuture.columns, receiveTime, record.mdfuture());
                    handleFutureData(columnsFuture.columns);
                }
                break;
            }

            case MD_TRANSACTION: {
                if(!handleTransaction_.isNull()&&!handleTransaction_->isNull() && record.has_mdtransaction()) {
                    thread_local ColumnsTransaction columnsTransaction(columnNamesTransaction_, columnSizeTransaction_);
                    setColumnsTransaction(columnsTransaction.columns, receiveTime, record.mdtransaction());
                    handleTransactionData(columnsTransaction.columns);
                }
                break;
            }

            case MD_ORDER: {
                if (!handleOrder_.isNull()&&!handleOrder_->isNull() && record.has_mdorder()) {
                    thread_local ColumnsOrder columnsOrder(columnNamesOrder_, columnSizeOrder_);
                    setColumnsOrder(columnsOrder.columns, receiveTime, record.mdorder());
                    handleOrderData(columnsOrder.columns);
                }
                break;
            }
            default: {
                throw RuntimeException(PLUGIN_INSIGHT_PREFIX + "unsupported data type " + std::to_string(record.marketdatatype()));
            }
        }
    } catch (exception &e) {
        LOG_ERR(PLUGIN_INSIGHT_PREFIX + "Faild to received macket data: " + string(e.what()));
    } catch (...) {
        LOG_ERR(PLUGIN_INSIGHT_PREFIX + "Faild to received macket data.");
    }
}

void InsightHandle::OnLoginSuccess(){
    LOG_INFO(PLUGIN_INSIGHT_PREFIX + "Login Success");
}

void InsightHandle::OnLoginFailed(int error_no, const string &message){
    LOG_ERR(PLUGIN_INSIGHT_PREFIX + "Login Failed. Error number: ", error_no, ". Error message: ", message);
}

void InsightHandle::OnNoConnections(){
    LOG_ERR(PLUGIN_INSIGHT_PREFIX + "No Connections");
}

void InsightHandle::OnGeneralError(const com::htsc::mdc::insight::model::InsightErrorContext &context){
    LOG_INFO(PLUGIN_INSIGHT_PREFIX + "PARSE message OnGeneralError: ", context.message());
}


static std::string getInsightTypeString(INSIGHT_DATA_TYPE type){
    switch (type)
    {
    case INSIGHT_DT_FuturesTick:
        return "futures tick";
    case INSIGHT_DT_IndexTick:
        return "index tick";
    case INSIGHT_DT_StockOrder:
        return "stock order";
    case INSIGHT_DT_StockTick:
        return "stock tick";
    case INSIGHT_DT_StockTransaction:
        return "stock transaction";
    default:
        throw RuntimeException(PLUGIN_INSIGHT_PREFIX + " unsupported insight data type " + std::to_string(type));
    }
    return "";
}

void InsightHandle::checkColumnsSize(std::vector<ConstantSP> &columns, INSIGHT_DATA_TYPE type){
    int columnsSize = columns.size();
    std::string msgBegin =PLUGIN_INSIGHT_PREFIX + "the size of ";
    std::string msgEnd = " columns for must be equal than ";
    int targetSize = 0;
    std::string insightType = getInsightTypeString(type);
    switch (type)
    {
    case INSIGHT_DT_FuturesTick:
        targetSize = columnSizeFuture_;
        break;
    case INSIGHT_DT_IndexTick:
        targetSize = columnSizeIndex_;
        break;
    case INSIGHT_DT_StockTick:
        targetSize = columnSizeStock_;
        break;
    case INSIGHT_DT_StockTransaction:
        targetSize = columnSizeTransaction_;
        break;
    case INSIGHT_DT_StockOrder:
        targetSize = columnSizeOrder_;
        break;
    default:
        throw RuntimeException(PLUGIN_INSIGHT_PREFIX + " unsupported insight data type " + std::to_string(type));
        break;
    }
    if(targetSize != columnsSize)
        throw RuntimeException(msgBegin + insightType + std::to_string(targetSize));
 }

void InsightHandle::ColumnsStock::createColumnsStock(std::vector<ConstantSP> &columns, INDEX colNum){
    int columnsSize = InsightStockTableMeta::COLTYPES.size();
    columns.resize(columnsSize);
    for(int i = 0; i < columnsSize; ++i){
        columns[i] = Util::createVector(InsightStockTableMeta::COLTYPES[i], colNum, colNum);
    }
}

void InsightHandle::setColumnsStock(std::vector<ConstantSP> &columns, long long receiveTime, const com::htsc::mdc::insight::model::MDStock &record, int index){
    checkColumnsSize(columns, INSIGHT_DT_StockTick);
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
        columns[16+i]->setLong(index, record.buypricequeue(i));
    for(int i = record.buypricequeue_size(); i < 10; i++)
        columns[16+i]->setLong(index, DT_LONG);

    for(int i = 0; i < record.buyorderqtyqueue_size() && i < 10; i++)
        columns[26+i]->setLong(index, record.buyorderqtyqueue(i));
    for(int i = record.buyorderqtyqueue_size(); i < 10; i++)
        columns[26+i]->setLong(index, DT_LONG);

    for(int i = 0; i < record.sellpricequeue_size() && i < 10; i++)
        columns[36+i]->setLong(index, record.sellpricequeue(i));
    for(int i = record.sellpricequeue_size(); i < 10; i++)
        columns[36+i]->setLong(index, DT_LONG);

    for(int i = 0; i < record.sellorderqtyqueue_size() && i < 10; i++)
        columns[46+i]->setLong(index, record.sellorderqtyqueue(i));
    for(int i = record.sellorderqtyqueue_size(); i < 10; i++)
        columns[46+i]->setLong(index, LONG_LONG_MIN);

    for(int i = 0; i < record.buyorderqueue_size() && i < 10; i++)
        columns[56+i]->setLong(index, record.buyorderqueue(i));
    for(int i = record.buyorderqueue_size(); i < 10; i++)
        columns[56+i]->setLong(index, LONG_LONG_MIN);

    for(int i = 0; i < record.sellorderqueue_size() && i < 10; i++)
        columns[66+i]->setLong(index, record.sellorderqueue(i));
    for(int i = record.sellorderqueue_size(); i < 10; i++)
        columns[66+i]->setLong(index, LONG_LONG_MIN);

    for(int i = 0; i < record.buynumordersqueue_size() && i < 10; i++)
        columns[76+i]->setLong(index, record.buynumordersqueue(i));
    for(int i = record.buynumordersqueue_size(); i < 10; i++)
        columns[76+i]->setLong(index, LONG_LONG_MIN);

    for(int i = 0; i < record.sellnumordersqueue_size() && i < 10; i++)
        columns[86+i]->setLong(index, record.sellnumordersqueue(i));
    for(int i = record.sellnumordersqueue_size(); i < 10; i++)
        columns[86+i]->setLong(index, LONG_LONG_MIN);

    columns[96]->setString(index, record.tradingphasecode());

    columns[97]->setLong(index, receiveTime);
}

void InsightHandle::handleStockData(vector<ConstantSP> &columns){
    checkColumnsSize(columns, INSIGHT_DT_StockTick);
    //assert(columns.size() == columnSizeStock);
    // INDEX insertedRows;
    // string errMsg;
    LockGuard<Mutex> _(lockStock_);
    TableSP data = Util::createTable(columnNamesStock_, columns);
    vector<ConstantSP> args = {handleStock_, data};
    session_->getFunctionDef("tableInsert")->call(session_->getHeap().get(), args);
}

void InsightHandle::ColumnsIndex::createColumnsIndex(std::vector<ConstantSP> &columns, INDEX colNum){
    int columnsSize = InsightIndexTableMeta::COLTYPES.size();
    columns.resize(columnsSize);
    for(int i = 0; i < columnsSize; ++i){
        columns[i] = Util::createVector(InsightIndexTableMeta::COLTYPES[i], colNum, colNum);
    }
}

void InsightHandle::setColumnsIndex(std::vector<ConstantSP> &columns, long long receiveTime, const com::htsc::mdc::insight::model::MDIndex &record, int index){
    checkColumnsSize(columns, INSIGHT_DT_IndexTick);
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
    checkColumnsSize(columns, INSIGHT_DT_IndexTick);
    // INDEX insertedRows;
    // string errMsg;
    LockGuard<Mutex> _(lockIndex_);
    TableSP data = Util::createTable(columnNamesIndex_, columns);
    vector<ConstantSP> args = {handleIndex_, data};
    session_->getFunctionDef("tableInsert")->call(session_->getHeap().get(), args);
}

void InsightHandle::ColumnsFuture::createColumnsFuture(std::vector<ConstantSP> &columns, INDEX colNum){
    int columnsSize = InsightFuturesTableMeta::COLTYPES.size();
    columns.resize(columnsSize);
    for(int i = 0; i < columnsSize; ++i){
        columns[i] = Util::createVector(InsightFuturesTableMeta::COLTYPES[i], colNum, colNum);
    }
}

void InsightHandle::setColumnsFuture(std::vector<ConstantSP> &columns, long long receiveTime, const com::htsc::mdc::insight::model::MDFuture &record, int index){
    checkColumnsSize(columns, INSIGHT_DT_FuturesTick);
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
        columns[14+i]->setLong(index, record.buypricequeue(0+i));
    for(int i = record.buypricequeue_size(); i < 5; i++)
        columns[14+i]->setLong(index, LONG_LONG_MIN);

    for(int i = 0; i < record.buyorderqtyqueue_size() && i < 5; i++)
        columns[19+i]->setLong(index, record.buyorderqtyqueue(0+i));
    for(int i = record.buyorderqtyqueue_size(); i < 5; i++)
        columns[19+i]->setLong(index, LONG_LONG_MIN);

    for(int i = 0; i < record.sellpricequeue_size() && i < 5; i++)
        columns[24+i]->setLong(index, record.sellpricequeue(0+i));
    for(int i = record.sellpricequeue_size(); i < 5; i++)
        columns[24+i]->setLong(index, LONG_LONG_MIN);

    for(int i = 0; i < record.sellorderqtyqueue_size() && i < 5; i++)
        columns[29+i]->setLong(index, record.sellorderqtyqueue(0+i));
    for(int i = record.sellorderqtyqueue_size(); i < 5; i++)
        columns[29+i]->setLong(index, LONG_LONG_MIN);

    columns[34]->setString(index, record.tradingphasecode());

    columns[35]->setLong(index, receiveTime);
}

void InsightHandle::handleFutureData(vector<ConstantSP> &columns){
    checkColumnsSize(columns, INSIGHT_DT_FuturesTick);
    //assert(columns.size() == columnSizeFuture);
    LockGuard<Mutex> _(lockFuture_);
    TableSP data = Util::createTable(columnNamesFuture_, columns);
    vector<ConstantSP> args = {handleFuture_, data};
    session_->getFunctionDef("tableInsert")->call(session_->getHeap().get(), args);
}

void InsightHandle::ColumnsTransaction::createColumnsTransaction(std::vector<ConstantSP> &columns, INDEX colNum){
    int columnsSize = InsightTransactionTableMeta::COLTYPES.size();
    columns.resize(columnsSize);
    for(int i = 0; i < columnsSize; ++i){
        columns[i] = Util::createVector(InsightTransactionTableMeta::COLTYPES[i], colNum, colNum);
    }
}

void InsightHandle::setColumnsTransaction(std::vector<ConstantSP> &columns, long long receiveTime, const com::htsc::mdc::insight::model::MDTransaction &record, int index){
    checkColumnsSize(columns, INSIGHT_DT_StockTransaction);
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
    checkColumnsSize(columns, INSIGHT_DT_StockTransaction);
    //assert(columns.size() == columnSizeTransaction);
    // INDEX insertedRows;
    // string errMsg;
    LockGuard<Mutex> _(lockTransaction_);
    TableSP data = Util::createTable(columnNamesTransaction_, columns);
    vector<ConstantSP> args = {handleTransaction_, data};
    session_->getFunctionDef("tableInsert")->call(session_->getHeap().get(), args);
}

void InsightHandle::ColumnsOrder::createColumnsOrder(std::vector<ConstantSP> &columns, INDEX colNum){
    int columnsSize = InsightOrderTableMeta::COLTYPES.size();
    columns.resize(columnsSize);
    for(int i = 0; i < columnsSize; ++i){
        columns[i] = Util::createVector(InsightOrderTableMeta::COLTYPES[i], colNum, colNum);
    }
}

void InsightHandle::setColumnsOrder(std::vector<ConstantSP> &columns, long long receiveTime, const com::htsc::mdc::insight::model::MDOrder &record, int index){
    checkColumnsSize(columns, INSIGHT_DT_StockOrder);
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
    checkColumnsSize(columns, INSIGHT_DT_StockOrder);
    LockGuard<Mutex> _(lockOrder_);
    TableSP data = Util::createTable(columnNamesOrder_, columns);
    vector<ConstantSP> args = {handleOrder_, data};
    session_->getFunctionDef("tableInsert")->call(session_->getHeap().get(), args);
}