
#include "amdSpiImp.h"
#include "Types.h"
#include "amdQuoteImp.h"
#include "amdQuoteType.h"
#include <iostream>

using dolphindb::Executor;
using dolphindb::DdbVector;


void AMDSpiImp::startOrderExecution(string type) {
    if(type == "orderExecution") {
        *orderExecutionStopFlag_ = false;
        orderExecutionFlag_ = true;
        for(auto it = orderExecutionData_.begin(); it != orderExecutionData_.end(); ++it) {
            int channel = it->first;
            orderExecutionBoundQueue_[it->first] = new OrderExecutionQueue(
                CAPACITY, ObjectSizer<MDOrderExecution>(), ObjectUrgency<MDOrderExecution>());
                
            SmartPointer<Executor> executor = new Executor(
                std::bind(&blockHandling<MDOrderExecution>,
                        orderExecutionBoundQueue_[it->first], 
                        [this, channel](vector<MDOrderExecution>& data) {
                            OnMDorderExecutionHelper(channel, data.data(), data.size(), true);},
                        orderExecutionStopFlag_, "[PluginAmdQuote]:", type
            ));
            orderExecutionThread_[it->first] = new Thread(executor);
            orderExecutionThread_[it->first]->start();
        }
    } else if(type == "fundOrderExecution") {
        *fundOrderExecutionStopFlag_ = false;
        fundOrderExecutionFlag_ = true;
        for(auto it = fundOrderExecutionData_.begin(); it != fundOrderExecutionData_.end(); ++it) {
            int channel = it->first;
            fundOrderExecutionBoundQueue_[it->first] = new OrderExecutionQueue(
                CAPACITY, ObjectSizer<MDOrderExecution>(), ObjectUrgency<MDOrderExecution>());

            SmartPointer<Executor> executor = new Executor(
                std::bind(&blockHandling<MDOrderExecution>,
                        fundOrderExecutionBoundQueue_[it->first],
                        [this, channel](vector<MDOrderExecution>& data) {
                            OnMDorderExecutionHelper(channel, data.data(), data.size(), false);},
                        fundOrderExecutionStopFlag_, "[PluginAmdQuote]:", type
            ));
            fundOrderExecutionThread_[it->first] = new Thread(executor);
            fundOrderExecutionThread_[it->first]->start();
        }
    } else if(type == "bondOrderExecution") {
        *bondOrderExecutionStopFlag_ = false;
        bondOrderExecutionFlag_ = true;
        for(auto it = bondOrderExecutionData_.begin(); it != bondOrderExecutionData_.end(); ++it) {
            int channel = it->first;
            bondOrderExecutionBoundQueue_[it->first] = new BondOrderExecutionQueue(
                CAPACITY, ObjectSizer<MDBondOrderExecution>(), ObjectUrgency<MDBondOrderExecution>());

            SmartPointer<Executor> executor = new Executor(
                std::bind(&blockHandling<MDBondOrderExecution>,
                        bondOrderExecutionBoundQueue_[it->first], 
                        [this, channel](vector<MDBondOrderExecution>& data) {
                            OnMDBondOrderExecutionHelper(channel, data.data(), data.size());},
                        bondOrderExecutionStopFlag_, "[PluginAmdQuote]:", type
            ));
            bondOrderExecutionThread_[it->first] = new Thread(executor);
            bondOrderExecutionThread_[it->first]->start();
        }
    }

}

void AMDSpiImp::clearOrderExecution(string type) {
    // stop threads
    if(type == "orderExecution") {
        if(*orderExecutionStopFlag_ == false) {
            *orderExecutionStopFlag_ = true;
            orderExecutionFlag_ = false;
            for(auto it = orderExecutionThread_.begin(); it != orderExecutionThread_.end(); ++it) {
                orderExecutionThread_[it->first]->join();
            }
            orderExecutionData_.clear();
            orderExecutionThread_.clear();
            orderExecutionBoundQueue_.clear();
        }
    } else if(type == "fundOrderExecution") {
        if (*fundOrderExecutionStopFlag_ == false) {
            *fundOrderExecutionStopFlag_ = true;
            fundOrderExecutionFlag_ = false;
            for(auto it = fundOrderExecutionThread_.begin(); it != fundOrderExecutionThread_.end(); ++it) {
                fundOrderExecutionThread_[it->first]->join();
            }
            fundOrderExecutionData_.clear();
            fundOrderExecutionThread_.clear();
            fundOrderExecutionBoundQueue_.clear();
        }
    } else if(type == "bondOrderExecution") {
        if(*bondOrderExecutionStopFlag_ == false) {
            *bondOrderExecutionStopFlag_ = true;
            bondOrderExecutionFlag_ = false;
            for(auto it = bondOrderExecutionThread_.begin(); it != bondOrderExecutionThread_.end(); ++it) {
                bondOrderExecutionThread_[it->first]->join();
            }
            bondOrderExecutionData_.clear();
            bondOrderExecutionThread_.clear();
            bondOrderExecutionBoundQueue_.clear();
        }
    }
}

inline string AMDSpiImp::transMarket(int type) {
    if(type == 101) {
        return "XSHG";
    } else if(type == 102) {
        return "XSHE";
    }
    return "";
}

// use stockOrFund to distinguish stock or fund 
void AMDSpiImp::OnMDorderExecutionHelper(int channel, MDOrderExecution* ticks, uint32_t cnt, bool stockOrFund) {
    try{
        if(cnt == 0)return;
        long long startTime = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
        // std::lock_guard<std::mutex> amdLock_(amdMutex_);
        if(stockOrFund) {
            if(!orderExecutionFlag_) {
                return;
            }
            TableSP stockInsertedTable = orderExecutionData_[channel];
            FunctionDefSP stockTransform = orderExecutionTransform_;
            int marketType;
            uint8_t varietyCategory;
            AMDDataType datatype;
            // stock ddbVector
            DdbVector<string> col0(0, cnt);
            DdbVector<int> col1(0, cnt);
            DdbVector<int> col2(0, cnt);
            DdbVector<string> col3(0, cnt);
            DdbVector<string> col4(0, cnt);
            DdbVector<int> col5(0, cnt);
            DdbVector<int> col6(0, cnt);
            DdbVector<int> col7(0, cnt);
            DdbVector<long long> col8(0, cnt);
            DdbVector<long long> col9(0, cnt);
            DdbVector<int> col10(0, cnt);
            DdbVector<long long> col11(0, cnt);
            DdbVector<long long> col12(0, cnt);
            DdbVector<long long> col13(0, cnt);
            DdbVector<int> col14(0, cnt);
            DdbVector<long long> col15(0, cnt);
            DdbVector<long long> col16(0, cnt);

            for (uint32_t i = 0; i < cnt; ++i) {
                if(ticks[i].orderOrExecution) {
                    marketType = ticks[i].uni.tickOrder.market_type;
                } else {
                    marketType = ticks[i].uni.tickExecution.market_type;
                }
                if (ticks[i].orderOrExecution) {
                    varietyCategory = ticks[i].uni.tickOrder.variety_category;
                } else {
                    varietyCategory = ticks[i].uni.tickExecution.variety_category;
                }
                
                if(varietyCategory == 1) {
                    datatype = AMD_ORDER_EXECUTION;
                } else if(varietyCategory == 2) {
                    datatype = AMD_FUND_ORDER_EXECUTION;
                } else {
                    return;
                }
                if(datatype == AMD_ORDER_EXECUTION) {
                    if(ticks[i].orderOrExecution) {
                        if(marketType == 101) {
                            col0.add(ticks[i].uni.tickOrder.security_code + string(".SH"));
                        } else if (marketType == 102) {
                            col0.add(ticks[i].uni.tickOrder.security_code + string(".SZ"));
                        }
                        
                        col1.add(convertToDate(ticks[i].uni.tickOrder.order_time));
                        col2.add(convertToTime(ticks[i].uni.tickOrder.order_time));
                        col3.add(transMarket(ticks[i].uni.tickOrder.market_type));
                        col4.add("StockType");
                        //  make sure dailyIndexFlagTotal always true
                        int dailyIndex = INT_MIN;
                        if(!getDailyIndex(dailyIndex, 
                                            dailyIndex_,
                                            28,             //change sizeof to constant
                                            ticks[i].uni.tickOrder.market_type,
                                            datatype,
                                            ticks[i].uni.tickOrder.channel_no,
                                            convertTime(ticks[i].uni.tickOrder.order_time))){
                            LOG_ERR("[PluginAmdQuote]: getDailyIndex failed. ");
                            return;
                        }
                        col5.add(dailyIndex);
                        col6.add(0);
                        col7.add(ticks[i].uni.tickOrder.order_type);
                        col8.add(ticks[i].uni.tickOrder.order_price);
                        col9.add(ticks[i].uni.tickOrder.order_volume);
                        col10.add(ticks[i].uni.tickOrder.side);
                        col11.add(ticks[i].uni.tickOrder.orig_order_no);
                        col12.add(ticks[i].uni.tickOrder.orig_order_no);
                        col13.add(ticks[i].uni.tickOrder.appl_seq_num);
                        col14.add(ticks[i].uni.tickOrder.channel_no);
                        // make sure receivedTimeFlag always true
                        col15.add(ticks[i].reachTime);
                    } else {
                        if(marketType == 101) {
                            col0.add(ticks[i].uni.tickExecution.security_code + string(".SH"));
                        } else if (marketType == 102) {
                            col0.add(ticks[i].uni.tickExecution.security_code + string(".SZ"));
                        }
                        col1.add(convertToDate(ticks[i].uni.tickExecution.exec_time));
                        col2.add(convertToTime(ticks[i].uni.tickExecution.exec_time));
                        col3.add(transMarket(ticks[i].uni.tickExecution.market_type));
                        col4.add("StockType");
                        //  make sure dailyIndexFlagTotal always true
                        int dailyIndex = INT_MIN;
                        if(!getDailyIndex(dailyIndex,
                                            dailyIndex_,
                                            28,                 //change sizeof to constant
                                            ticks[i].uni.tickExecution.market_type,
                                            datatype,
                                            ticks[i].uni.tickExecution.channel_no,
                                            convertTime(ticks[i].uni.tickExecution.exec_time))){
                            LOG_ERR("[PluginAmdQuote]: getDailyIndex failed. ");
                            return;
                        }
                        col5.add(dailyIndex);
                        col6.add(0);
                        col7.add(ticks[i].uni.tickExecution.exec_type);
                        col8.add(ticks[i].uni.tickExecution.exec_price);
                        col9.add(ticks[i].uni.tickExecution.exec_volume);
                        col10.add(ticks[i].uni.tickExecution.side);
                        col11.add(ticks[i].uni.tickExecution.bid_appl_seq_num);
                        col12.add(ticks[i].uni.tickExecution.offer_appl_seq_num);
                        col13.add(ticks[i].uni.tickExecution.appl_seq_num);
                        col14.add(ticks[i].uni.tickExecution.channel_no);
                        // make sure receivedTimeFlag always true
                        col15.add(ticks[i].reachTime);
                    }
                }
            }

            vector<ConstantSP> cols;
            cols.push_back(col0.createVector(DT_STRING));
            cols.push_back(col1.createVector(DT_DATE));
            cols.push_back(col2.createVector(DT_TIME));
            cols.push_back(col3.createVector(DT_STRING));
            cols.push_back(col4.createVector(DT_STRING));
            cols.push_back(col5.createVector(DT_INT));
            cols.push_back(col6.createVector(DT_INT));
            cols.push_back(col7.createVector(DT_INT));
            cols.push_back(col8.createVector(DT_LONG));
            cols.push_back(col9.createVector(DT_LONG));
            cols.push_back(col10.createVector(DT_INT));
            cols.push_back(col11.createVector(DT_LONG));
            cols.push_back(col12.createVector(DT_LONG));
            cols.push_back(col13.createVector(DT_LONG));
            cols.push_back(col14.createVector(DT_INT));
            // make sure receivedTimeFlag always true
            cols.push_back(col15.createVector(DT_NANOTIMESTAMP));

            vector<long long> bufVec;
            bufVec.resize(cols[15]->size());
            auto col15Buffer = cols[15]->getLongConst(0, cols[15]->size(), bufVec.data());

            vector<string> colNames = orderExecutionTableMeta_.colNames_;
            if(outputElapsedFlag_) {
                long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                for(int i = 0; i < col15.size(); ++i) {
                    col16.add(time - col15Buffer[i]);
                }
                cols.push_back(col16.createVector(DT_NANOTIME));
                colNames.push_back("perPenetrationTime");
            }
            TableSP data = Util::createTable(colNames, cols);

            if(orderExecutionFlag_ && !(stockInsertedTable.isNull())) {
                vector<ConstantSP> args = {data};
                try{
                    if(!stockTransform.isNull())
                        data = stockTransform->call(session_->getHeap().get(), args);
                }catch(exception &e){
                    throw RuntimeException("call transform error " + string(e.what()));
                }
                if(stockInsertedTable.isNull())
                    throw RuntimeException("stock insertedTable is null");
                if(stockInsertedTable ->columns() != data->columns()) {
                    throw RuntimeException("The number of columns of the table to insert must be the same as that of the original table");
                }
                args = {stockInsertedTable, data};
                LockGuard<Mutex> _(stockInsertedTable->getLock());
                vector<ConstantSP> colData(data->columns());
                for(INDEX i = 0; i < data->columns(); ++i) {
                    colData[i] = data->getColumn(i);
                }
                INDEX rows;
                string errMsg;
                stockInsertedTable->append(colData, rows, errMsg);
                if(errMsg != "") {
                    LOG_ERR("[PluginAmdQuote]: OnMDTickOrderExecution channel " + std::to_string(channel) + " append failed, " + errMsg);
                    return;
                }
                if (latencyFlag_) {
                    long long diff = Util::toLocalNanoTimestamp(Util::getNanoEpochTime() - startTime);
                    latencyLog(6, startTime, cnt, diff);
                }
            }
        } else {
            if(!fundOrderExecutionFlag_) {
                return;
            }
            TableSP fundInsertedTable = fundOrderExecutionData_[channel];
            FunctionDefSP fundTransform = fundOrderExecutionTransform_;
            int marketType;
            uint8_t varietyCategory;
            AMDDataType datatype;
            //fund ddbVector
            DdbVector<string> fcol0(0, cnt);
            DdbVector<int> fcol1(0, cnt);
            DdbVector<int> fcol2(0, cnt);
            DdbVector<string> fcol3(0, cnt);
            DdbVector<string> fcol4(0, cnt);
            DdbVector<int> fcol5(0, cnt);
            DdbVector<int> fcol6(0, cnt);
            DdbVector<int> fcol7(0, cnt);
            DdbVector<long long> fcol8(0, cnt);
            DdbVector<long long> fcol9(0, cnt);
            DdbVector<int> fcol10(0, cnt);
            DdbVector<long long> fcol11(0, cnt);
            DdbVector<long long> fcol12(0, cnt);
            DdbVector<long long> fcol13(0, cnt);
            DdbVector<int> fcol14(0, cnt);
            DdbVector<long long> fcol15(0, cnt);
            DdbVector<long long> fcol16(0, cnt);

            for (uint32_t i = 0; i < cnt; ++i) {
                if(ticks[i].orderOrExecution) {
                    marketType = ticks[i].uni.tickOrder.market_type;
                } else {
                    marketType = ticks[i].uni.tickExecution.market_type;
                }
                if (ticks[i].orderOrExecution) {
                    varietyCategory = ticks[i].uni.tickOrder.variety_category;
                } else {
                    varietyCategory = ticks[i].uni.tickExecution.variety_category;
                }
                if(varietyCategory == 1) {
                    datatype = AMD_ORDER_EXECUTION;
                } else if(varietyCategory == 2) {
                    datatype = AMD_FUND_ORDER_EXECUTION;
                } else {
                    return;
                }
                if(datatype == AMD_FUND_ORDER_EXECUTION) {
                    if(ticks[i].orderOrExecution) {
                        if(marketType == 101) {
                            fcol0.add(ticks[i].uni.tickOrder.security_code + string(".SH"));
                        } else if (marketType == 102) {
                            fcol0.add(ticks[i].uni.tickOrder.security_code + string(".SZ"));
                        }
                        
                        fcol1.add(convertToDate(ticks[i].uni.tickOrder.order_time));
                        fcol2.add(convertToTime(ticks[i].uni.tickOrder.order_time));
                        fcol3.add(transMarket(ticks[i].uni.tickOrder.market_type));
                        fcol4.add("FundType");
                        //  make sure dailyIndexFlagTotal always true
                        int dailyIndex = INT_MIN;
                        if(!getDailyIndex(dailyIndex, 
                                            dailyIndex_,
                                            28,             //change sizeof to constant
                                            ticks[i].uni.tickOrder.market_type,
                                            datatype,
                                            ticks[i].uni.tickOrder.channel_no,
                                            convertTime(ticks[i].uni.tickOrder.order_time))){
                            LOG_ERR("[PluginAmdQuote]: getDailyIndex failed. ");
                            return;
                        }
                        fcol5.add(dailyIndex);
                        fcol6.add(0);
                        fcol7.add(ticks[i].uni.tickOrder.order_type);
                        fcol8.add(ticks[i].uni.tickOrder.order_price);
                        fcol9.add(ticks[i].uni.tickOrder.order_volume);
                        fcol10.add(ticks[i].uni.tickOrder.side);
                        fcol11.add(ticks[i].uni.tickOrder.orig_order_no);
                        fcol12.add(ticks[i].uni.tickOrder.orig_order_no);
                        fcol13.add(ticks[i].uni.tickOrder.appl_seq_num);
                        fcol14.add(ticks[i].uni.tickOrder.channel_no);
                        // make sure receivedTimeFlag always true
                        fcol15.add(ticks[i].reachTime);
                    } else {
                        if(marketType == 101) {
                            fcol0.add(ticks[i].uni.tickExecution.security_code + string(".SH"));
                        } else if (marketType == 102) {
                            fcol0.add(ticks[i].uni.tickExecution.security_code + string(".SZ"));
                        }
                        fcol1.add(convertToDate(ticks[i].uni.tickExecution.exec_time));
                        fcol2.add(convertToTime(ticks[i].uni.tickExecution.exec_time));
                        fcol3.add(transMarket(ticks[i].uni.tickExecution.market_type));
                        fcol4.add("FundType");
                        //  make sure dailyIndexFlagTotal always true
                        int dailyIndex = INT_MIN;
                        if(!getDailyIndex(dailyIndex,
                                            dailyIndex_,
                                            28,                 //change sizeof to constant
                                            ticks[i].uni.tickExecution.market_type,
                                            datatype,
                                            ticks[i].uni.tickExecution.channel_no,
                                            convertTime(ticks[i].uni.tickExecution.exec_time))){
                            LOG_ERR("[PluginAmdQuote]: getDailyIndex failed. ");
                            return;
                        }
                        fcol5.add(dailyIndex);
                        fcol6.add(0);
                        fcol7.add(ticks[i].uni.tickExecution.exec_type);
                        fcol8.add(ticks[i].uni.tickExecution.exec_price);
                        fcol9.add(ticks[i].uni.tickExecution.exec_volume);
                        fcol10.add(ticks[i].uni.tickExecution.side);
                        fcol11.add(ticks[i].uni.tickExecution.bid_appl_seq_num);
                        fcol12.add(ticks[i].uni.tickExecution.offer_appl_seq_num);
                        fcol13.add(ticks[i].uni.tickExecution.appl_seq_num);
                        fcol14.add(ticks[i].uni.tickExecution.channel_no);
                        // make sure receivedTimeFlag always true
                        fcol15.add(ticks[i].reachTime);
                    }
                }
            }
            vector<ConstantSP> fcols;
            fcols.push_back(fcol0.createVector(DT_STRING));
            fcols.push_back(fcol1.createVector(DT_DATE));
            fcols.push_back(fcol2.createVector(DT_TIME));
            fcols.push_back(fcol3.createVector(DT_STRING));
            fcols.push_back(fcol4.createVector(DT_STRING));
            fcols.push_back(fcol5.createVector(DT_INT));
            fcols.push_back(fcol6.createVector(DT_INT));
            fcols.push_back(fcol7.createVector(DT_INT));
            fcols.push_back(fcol8.createVector(DT_LONG));
            fcols.push_back(fcol9.createVector(DT_LONG));
            fcols.push_back(fcol10.createVector(DT_INT));
            fcols.push_back(fcol11.createVector(DT_LONG));
            fcols.push_back(fcol12.createVector(DT_LONG));
            fcols.push_back(fcol13.createVector(DT_LONG));
            fcols.push_back(fcol14.createVector(DT_INT));
            // make sure receivedTimeFlag always true
            fcols.push_back(fcol15.createVector(DT_NANOTIMESTAMP));

            vector<long long> bufVec;
            bufVec.resize(fcols[15]->size());
            auto col15Buffer = fcols[15]->getLongConst(0, fcols[15]->size(), bufVec.data());

            vector<string> fcolNames = orderExecutionTableMeta_.colNames_;
            if(outputElapsedFlag_) {
                long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                for(int i = 0; i < fcol15.size(); ++i) {
                    fcol16.add(time - col15Buffer[i]);
                }
                fcols.push_back(fcol16.createVector(DT_NANOTIME));
                fcolNames.push_back("perPenetrationTime");
            }

            TableSP fdata = Util::createTable(fcolNames, fcols);
            if (fundOrderExecutionFlag_ && !(fundInsertedTable.isNull())){
                vector<ConstantSP> fargs = {fdata};
                try{
                    if(!fundTransform.isNull())
                        fdata = fundTransform->call(session_->getHeap().get(), fargs);
                }catch(exception &e){
                    throw RuntimeException("call transform error " + string(e.what()));
                }

                if(fundInsertedTable.isNull())
                    throw RuntimeException("fund insertedTable is null");
                if(fundInsertedTable ->columns() != fdata->columns()) {
                    throw RuntimeException("The number of columns of the table to insert must be the same as that of the original table");
                }

                fargs = {fundInsertedTable, fdata};
                LockGuard<Mutex> _f(fundInsertedTable->getLock());

                vector<ConstantSP> fcolData(fdata->columns());
                for(INDEX i = 0; i < fdata->columns(); ++i) {
                    fcolData[i] = fdata->getColumn(i);
                }
                INDEX frows;
                string ferrMsg;
                fundInsertedTable->append(fcolData, frows, ferrMsg);
                if(ferrMsg != "") {
                    LOG_ERR("[PluginAmdQuote]: OnMDTickFundOrderExecution channel " + std::to_string(channel) + " append failed, " + ferrMsg);
                    return;
                }
                if (latencyFlag_) { 
                    long long diff = Util::toLocalNanoTimestamp(Util::getNanoEpochTime() - startTime);
                    latencyLog(7, startTime, cnt, diff);
                }
            }
        }
    }
    catch(exception &e){
        LOG_ERR("[PluginAmdQuote]: OnMDTickOrderExecution channel " + std::to_string(channel) + " failed, ", e.what());
    }
}
void AMDSpiImp::OnMDBondOrderExecutionHelper(int channel, MDBondOrderExecution* ticks, uint32_t cnt) {
    try{
        if(cnt == 0)return;
        long long startTime = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());

        TableSP insertedTable;
        FunctionDefSP transform;
        AMDDataType datatype;
        uint8_t varietyCategory;

        int marketType;
        if(ticks[0].orderOrExecution) {
            marketType = ticks[0].uni.tickOrder.market_type;
            varietyCategory = ticks[0].uni.tickOrder.variety_category;
        } else {
            marketType = ticks[0].uni.tickExecution.market_type;
            varietyCategory = ticks[0].uni.tickExecution.variety_category;
        }

        if (varietyCategory == 3) { // 债券
            insertedTable = bondOrderExecutionData_[channel];
            transform = bondOrderExecutionTransform_;
            datatype = AMD_BOND_ORDER_EXECUTION;
        } else {
            return ;
        }

        if(!bondOrderExecutionFlag_) {
            return;
        }

        AMDTableType tableTyeTotal = getAmdTableType(datatype, marketType);
        if(tableTyeTotal == AMD_ERROR_TABLE_TYPE){
            LOG_ERR("[PluginAmdQuote]: error amd table type AMD_ERROR_TABLE_TYPE");
            return;
        }

        DdbVector<string> col0(0, cnt);
        DdbVector<int> col1(0, cnt);
        DdbVector<int> col2(0, cnt);
        DdbVector<string> col3(0, cnt);
        DdbVector<string> col4(0, cnt);
        DdbVector<int> col5(0, cnt);
        DdbVector<int> col6(0, cnt);
        DdbVector<int> col7(0, cnt);
        DdbVector<long long> col8(0, cnt);
        DdbVector<long long> col9(0, cnt);
        DdbVector<int> col10(0, cnt);
        DdbVector<long long> col11(0, cnt);
        DdbVector<long long> col12(0, cnt);
        DdbVector<long long> col13(0, cnt);
        DdbVector<int> col14(0, cnt);
        DdbVector<long long> col15(0, cnt);
        DdbVector<long long> col16(0, cnt);

        for (uint32_t i = 0; i < cnt; ++i) {
            if(ticks[i].orderOrExecution) {
                if(marketType == 101) {
                    col0.add(ticks[i].uni.tickOrder.security_code + string(".SH"));
                } else if (marketType == 102) {
                    col0.add(ticks[i].uni.tickOrder.security_code + string(".SZ"));
                }
                col1.add(convertToDate(ticks[i].uni.tickOrder.order_time));
                col2.add(convertToTime(ticks[i].uni.tickOrder.order_time));
                col3.add(transMarket(ticks[i].uni.tickOrder.market_type));
                col4.add("BondType");
                
                //  make sure dailyIndexFlagTotal always true
                int dailyIndex = INT_MIN;
                if(!getDailyIndex(dailyIndex,
                                    dailyIndex_,
                                    28,             //change sizeof to constant
                                    ticks[i].uni.tickOrder.market_type,
                                    datatype,
                                    ticks[i].uni.tickOrder.channel_no,
                                    convertTime(ticks[i].uni.tickOrder.order_time))){
                    LOG_ERR("[PluginAmdQuote]: getDailyIndex failed. ");
                    return;
                }
                col5.add(dailyIndex);
                col6.add(0);
                col7.add(ticks[i].uni.tickOrder.order_type);
                col8.add(ticks[i].uni.tickOrder.order_price);
                col9.add(ticks[i].uni.tickOrder.order_volume);
                col10.add(ticks[i].uni.tickOrder.side);
                col11.add(ticks[i].uni.tickOrder.orig_order_no);
                col12.add(ticks[i].uni.tickOrder.orig_order_no);
                col13.add(ticks[i].uni.tickOrder.appl_seq_num);
                col14.add(ticks[i].uni.tickOrder.channel_no);
                // make sure receivedTimeFlag always true
                col15.add(ticks[i].reachTime);
            } else {
                if(marketType == 101) {
                    col0.add(ticks[i].uni.tickExecution.security_code + string(".SH"));
                } else if (marketType == 102) {
                    col0.add(ticks[i].uni.tickExecution.security_code + string(".SZ"));
                }
                col1.add(convertToDate(ticks[i].uni.tickExecution.exec_time));
                col2.add(convertToTime(ticks[i].uni.tickExecution.exec_time));
                col3.add(transMarket(ticks[i].uni.tickExecution.market_type));
                col4.add("BondType");
                //  make sure dailyIndexFlagTotal always true
                int dailyIndex = INT_MIN;
                if(!getDailyIndex(dailyIndex,
                                    dailyIndex_,
                                    28,             //change sizeof to constant
                                    ticks[i].uni.tickExecution.market_type,
                                    datatype,
                                    ticks[i].uni.tickExecution.channel_no,
                                    convertTime(ticks[i].uni.tickExecution.exec_time))){
                    LOG_ERR("[PluginAmdQuote]: getDailyIndex failed. ");
                    return;
                }
                col5.add(dailyIndex);
                // }
                col6.add(0);
                col7.add(ticks[i].uni.tickExecution.exec_type);
                col8.add(ticks[i].uni.tickExecution.exec_price);
                col9.add(ticks[i].uni.tickExecution.exec_volume);
                col10.add(ticks[i].uni.tickExecution.side);
                col11.add(ticks[i].uni.tickExecution.bid_appl_seq_num);
                col12.add(ticks[i].uni.tickExecution.offer_appl_seq_num);
                col13.add(ticks[i].uni.tickExecution.appl_seq_num);
                col14.add(ticks[i].uni.tickExecution.channel_no);
                // make sure receivedTimeFlag always true
                col15.add(ticks[i].reachTime);
            }
        }

        vector<ConstantSP> cols;
        cols.push_back(col0.createVector(DT_STRING));
        cols.push_back(col1.createVector(DT_DATE));
        cols.push_back(col2.createVector(DT_TIME));
        cols.push_back(col3.createVector(DT_STRING));
        cols.push_back(col4.createVector(DT_STRING));
        cols.push_back(col5.createVector(DT_INT));
        cols.push_back(col6.createVector(DT_INT));
        cols.push_back(col7.createVector(DT_INT));
        cols.push_back(col8.createVector(DT_LONG));
        cols.push_back(col9.createVector(DT_LONG));
        cols.push_back(col10.createVector(DT_INT));
        cols.push_back(col11.createVector(DT_LONG));
        cols.push_back(col12.createVector(DT_LONG));
        cols.push_back(col13.createVector(DT_LONG));
        cols.push_back(col14.createVector(DT_INT));
        cols.push_back(col15.createVector(DT_NANOTIMESTAMP));

        vector<long long> bufVec;
        bufVec.resize(cols[15]->size());
        auto col15Buffer = cols[15]->getLongConst(0, cols[15]->size(), bufVec.data());

        vector<string> colNames = orderExecutionTableMeta_.colNames_;
        if(outputElapsedFlag_) {
            long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
            for(int i = 0; i < col15.size(); ++i) {
                col16.add(time - col15Buffer[i]);
            }
            cols.push_back(col16.createVector(DT_NANOTIME));
            colNames.push_back("perPenetrationTime");
        }
        TableSP data = Util::createTable(colNames, cols);

        vector<ConstantSP> args = {data};
        try{
            if(!transform.isNull())
                data = transform->call(session_->getHeap().get(), args);
        }catch(exception &e){
            throw RuntimeException("call transform error " + string(e.what()));
        }

        if(insertedTable.isNull())
            throw RuntimeException("insertedTable is null");
        if(insertedTable ->columns() != data->columns()) {
            throw RuntimeException("The number of columns of the table to insert must be the same as that of the original table");
        }
        args = {insertedTable, data};
        LockGuard<Mutex> _(insertedTable->getLock());
        INDEX rows;
        string errMsg;
        vector<ConstantSP> colData(data->columns());
        for(INDEX i = 0; i < data->columns(); ++i) {
            colData[i] = data->getColumn(i);
        }
        insertedTable->append(colData, rows, errMsg);
        if(errMsg != "") {
            LOG_ERR("[PluginAmdQuote]: OnMDTickBondOrderExecution channel " + std::to_string(channel) + " append failed, " + errMsg);
            return;
        }

        if (latencyFlag_) { 
            long long diff = Util::toLocalNanoTimestamp(Util::getNanoEpochTime() - startTime);
            latencyLog(8, startTime, cnt, diff);
        }
    }
    catch(exception &e){
        LOG_ERR("[PluginAmdQuote]: OnMDTickBondOrderExecution channel " + std::to_string(channel) + " failed, ", e.what());
    }
}

// 接受并处理快照数据（包含基金和股票数据）
void AMDSpiImp::OnMDSnapshotHelper(timeMDSnapshot* snapshot, uint32_t cnt) {
    try{
        if(cnt == 0)return;
        long long startTime = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
        // std::lock_guard<std::mutex> amdLock_(amdMutex_);

        // 基金和股票数据插入到不同的表
        TableSP insertedTable = snapshotData_;
        FunctionDefSP transform = snapshotTransform_;
        TableSP fundInsertedTable = fundSnapshotData_;
        FunctionDefSP fundTransform = fundSnapshotTransform_;

        uint8_t varietyCategory;
        AMDDataType datatype;
        int market_type;

        vector<long long> reachTimeVec;
        if(outputElapsedFlag_) {
            reachTimeVec.reserve(cnt);
        }
        vector<long long> fundReachTimeVec;
        if(outputElapsedFlag_) {
            fundReachTimeVec.reserve(cnt);
        }

        DdbVector<int> col0(0, cnt);
        DdbVector<string> col1(0, cnt);
        DdbVector<long long> col2(0, cnt);
        DdbVector<string> col3(0, cnt);
        DdbVector<long long> col4(0, cnt);
        DdbVector<long long> col5(0, cnt);
        DdbVector<long long> col6(0, cnt);
        DdbVector<long long> col7(0, cnt);
        DdbVector<long long> col8(0, cnt);
        DdbVector<long long> col9(0, cnt);

        DdbVector<long long> col10(0, cnt);
        DdbVector<long long> col11(0, cnt);
        DdbVector<long long> col12(0, cnt);
        DdbVector<long long> col13(0, cnt);
        DdbVector<long long> col14(0, cnt);
        DdbVector<long long> col15(0, cnt);
        DdbVector<long long> col16(0, cnt);
        DdbVector<long long> col17(0, cnt);
        DdbVector<long long> col18(0, cnt);
        DdbVector<long long> col19(0, cnt);

        DdbVector<long long> col20(0, cnt);
        DdbVector<long long> col21(0, cnt);
        DdbVector<long long> col22(0, cnt);
        DdbVector<long long> col23(0, cnt);
        DdbVector<long long> col24(0, cnt);
        DdbVector<long long> col25(0, cnt);
        DdbVector<long long> col26(0, cnt);
        DdbVector<long long> col27(0, cnt);
        DdbVector<long long> col28(0, cnt);
        DdbVector<long long> col29(0, cnt);

        DdbVector<long long> col30(0, cnt);
        DdbVector<long long> col31(0, cnt);
        DdbVector<long long> col32(0, cnt);
        DdbVector<long long> col33(0, cnt);
        DdbVector<long long> col34(0, cnt);
        DdbVector<long long> col35(0, cnt);
        DdbVector<long long> col36(0, cnt);
        DdbVector<long long> col37(0, cnt);
        DdbVector<long long> col38(0, cnt);
        DdbVector<long long> col39(0, cnt);
        
        DdbVector<long long> col40(0, cnt);
        DdbVector<long long> col41(0, cnt);
        DdbVector<long long> col42(0, cnt);
        DdbVector<long long> col43(0, cnt);
        DdbVector<long long> col44(0, cnt);
        DdbVector<long long> col45(0, cnt);
        DdbVector<long long> col46(0, cnt);
        DdbVector<long long> col47(0, cnt);
        DdbVector<long long> col48(0, cnt);
        DdbVector<long long> col49(0, cnt);

        DdbVector<long long> col50(0, cnt);
        DdbVector<long long> col51(0, cnt);
        DdbVector<long long> col52(0, cnt);
        DdbVector<long long> col53(0, cnt);
        DdbVector<long long> col54(0, cnt);
        DdbVector<long long> col55(0, cnt);
        DdbVector<long long> col56(0, cnt);
        DdbVector<long long> col57(0, cnt);
        DdbVector<long long> col58(0, cnt);
        DdbVector<long long> col59(0, cnt);

        DdbVector<long long> col60(0, cnt);
        DdbVector<long long> col61(0, cnt);
        DdbVector<long long> col62(0, cnt);
        DdbVector<long long> col63(0, cnt);
        DdbVector<long long> col64(0, cnt);
        DdbVector<int> col65(0, cnt);
        DdbVector<string> col66(0, cnt);
        DdbVector<string> col67(0, cnt);
        DdbVector<long long> col68(0, cnt);
        DdbVector<long long> col69(0, cnt);

        DdbVector<long long> col70(0, cnt);
        DdbVector<long long> col71(0, cnt);
        DdbVector<long long> col72(0, cnt);
        DdbVector<long long> col73(0, cnt);
        DdbVector<long long> col74(0, cnt);
        DdbVector<long long> col75(0, cnt);
        DdbVector<long long> col76(0, cnt);
        DdbVector<long long> col77(0, cnt);
        DdbVector<long long> col78(0, cnt);
        DdbVector<long long> col79(0, cnt);

        DdbVector<long long> col80(0, cnt);
        DdbVector<long long> col81(0, cnt);
        DdbVector<long long> col82(0, cnt);
        DdbVector<long long> col83(0, cnt);
        DdbVector<long long> col84(0, cnt);
        DdbVector<long long> col85(0, cnt);
        DdbVector<long long> col86(0, cnt);
        DdbVector<long long> col87(0, cnt);
        DdbVector<int> col88(0, cnt);
        DdbVector<int> col89(0, cnt);

        DdbVector<int> col90(0, cnt);
        DdbVector<int> col91(0, cnt);
        DdbVector<long long> col92(0, cnt);
        DdbVector<char> col93(0, cnt);
        DdbVector<long long> col94(0, cnt);
        DdbVector<int> col95(0, cnt);
        DdbVector<long long> col96(0, cnt);

        DdbVector<int> fcol0(0, cnt);
        DdbVector<string> fcol1(0, cnt);
        DdbVector<long long> fcol2(0, cnt);
        DdbVector<string> fcol3(0, cnt);
        DdbVector<long long> fcol4(0, cnt);
        DdbVector<long long> fcol5(0, cnt);
        DdbVector<long long> fcol6(0, cnt);
        DdbVector<long long> fcol7(0, cnt);
        DdbVector<long long> fcol8(0, cnt);
        DdbVector<long long> fcol9(0, cnt);

        DdbVector<long long> fcol10(0, cnt);
        DdbVector<long long> fcol11(0, cnt);
        DdbVector<long long> fcol12(0, cnt);
        DdbVector<long long> fcol13(0, cnt);
        DdbVector<long long> fcol14(0, cnt);
        DdbVector<long long> fcol15(0, cnt);
        DdbVector<long long> fcol16(0, cnt);
        DdbVector<long long> fcol17(0, cnt);
        DdbVector<long long> fcol18(0, cnt);
        DdbVector<long long> fcol19(0, cnt);

        DdbVector<long long> fcol20(0, cnt);
        DdbVector<long long> fcol21(0, cnt);
        DdbVector<long long> fcol22(0, cnt);
        DdbVector<long long> fcol23(0, cnt);
        DdbVector<long long> fcol24(0, cnt);
        DdbVector<long long> fcol25(0, cnt);
        DdbVector<long long> fcol26(0, cnt);
        DdbVector<long long> fcol27(0, cnt);
        DdbVector<long long> fcol28(0, cnt);
        DdbVector<long long> fcol29(0, cnt);

        DdbVector<long long> fcol30(0, cnt);
        DdbVector<long long> fcol31(0, cnt);
        DdbVector<long long> fcol32(0, cnt);
        DdbVector<long long> fcol33(0, cnt);
        DdbVector<long long> fcol34(0, cnt);
        DdbVector<long long> fcol35(0, cnt);
        DdbVector<long long> fcol36(0, cnt);
        DdbVector<long long> fcol37(0, cnt);
        DdbVector<long long> fcol38(0, cnt);
        DdbVector<long long> fcol39(0, cnt);
        
        DdbVector<long long> fcol40(0, cnt);
        DdbVector<long long> fcol41(0, cnt);
        DdbVector<long long> fcol42(0, cnt);
        DdbVector<long long> fcol43(0, cnt);
        DdbVector<long long> fcol44(0, cnt);
        DdbVector<long long> fcol45(0, cnt);
        DdbVector<long long> fcol46(0, cnt);
        DdbVector<long long> fcol47(0, cnt);
        DdbVector<long long> fcol48(0, cnt);
        DdbVector<long long> fcol49(0, cnt);

        DdbVector<long long> fcol50(0, cnt);
        DdbVector<long long> fcol51(0, cnt);
        DdbVector<long long> fcol52(0, cnt);
        DdbVector<long long> fcol53(0, cnt);
        DdbVector<long long> fcol54(0, cnt);
        DdbVector<long long> fcol55(0, cnt);
        DdbVector<long long> fcol56(0, cnt);
        DdbVector<long long> fcol57(0, cnt);
        DdbVector<long long> fcol58(0, cnt);
        DdbVector<long long> fcol59(0, cnt);

        DdbVector<long long> fcol60(0, cnt);
        DdbVector<long long> fcol61(0, cnt);
        DdbVector<long long> fcol62(0, cnt);
        DdbVector<long long> fcol63(0, cnt);
        DdbVector<long long> fcol64(0, cnt);
        DdbVector<int> fcol65(0, cnt);
        DdbVector<string> fcol66(0, cnt);
        DdbVector<string> fcol67(0, cnt);
        DdbVector<long long> fcol68(0, cnt);
        DdbVector<long long> fcol69(0, cnt);

        DdbVector<long long> fcol70(0, cnt);
        DdbVector<long long> fcol71(0, cnt);
        DdbVector<long long> fcol72(0, cnt);
        DdbVector<long long> fcol73(0, cnt);
        DdbVector<long long> fcol74(0, cnt);
        DdbVector<long long> fcol75(0, cnt);
        DdbVector<long long> fcol76(0, cnt);
        DdbVector<long long> fcol77(0, cnt);
        DdbVector<long long> fcol78(0, cnt);
        DdbVector<long long> fcol79(0, cnt);

        DdbVector<long long> fcol80(0, cnt);
        DdbVector<long long> fcol81(0, cnt);
        DdbVector<long long> fcol82(0, cnt);
        DdbVector<long long> fcol83(0, cnt);
        DdbVector<long long> fcol84(0, cnt);
        DdbVector<long long> fcol85(0, cnt);
        DdbVector<long long> fcol86(0, cnt);
        DdbVector<long long> fcol87(0, cnt);
        DdbVector<int> fcol88(0, cnt);
        DdbVector<int> fcol89(0, cnt);

        DdbVector<int> fcol90(0, cnt);
        DdbVector<int> fcol91(0, cnt);
        DdbVector<long long> fcol92(0, cnt);
        DdbVector<char> fcol93(0, cnt);
        DdbVector<long long> fcol94(0, cnt);
        DdbVector<int> fcol95(0, cnt);
        DdbVector<long long> fcol96(0, cnt);

        for (uint32_t i = 0; i < cnt; ++i) {
            varietyCategory = snapshot[i].snapshot.variety_category;
            if (varietyCategory == 1) {
                datatype = AMD_SNAPSHOT;
                if(!snapshotFlag_) {
                    continue;
                }
            } else if(varietyCategory == 2){
                datatype = AMD_FUND_SNAPSHOT;
                if(!fundSnapshotFlag_) {
                    continue;
                }
            } else {
                continue;
            }
            market_type = snapshot[i].snapshot.market_type;
            AMDTableType tableTyeTotal = getAmdTableType(datatype, market_type);
            
            if(tableTyeTotal == AMD_ERROR_TABLE_TYPE){
                LOG_ERR("[PluginAmdQuote]: error amd table type AMD_ERROR_TABLE_TYPE");
                return;
            }

            if(varietyCategory == 1) {
                col0.add(snapshot[i].snapshot.market_type);
                col1.add(snapshot[i].snapshot.security_code);
                col2.add(convertTime(snapshot[i].snapshot.orig_time));
                col3.add(snapshot[i].snapshot.trading_phase_code);
                col4.add(snapshot[i].snapshot.pre_close_price);
                col5.add(snapshot[i].snapshot.open_price);
                col6.add(snapshot[i].snapshot.high_price);
                col7.add(snapshot[i].snapshot.low_price);
                col8.add(snapshot[i].snapshot.last_price);
                col9.add(snapshot[i].snapshot.close_price);

                int bidPriceIndex = 0;
                col10.add(snapshot[i].snapshot.bid_price[bidPriceIndex++]);
                col11.add(snapshot[i].snapshot.bid_price[bidPriceIndex++]);
                col12.add(snapshot[i].snapshot.bid_price[bidPriceIndex++]);
                col13.add(snapshot[i].snapshot.bid_price[bidPriceIndex++]);
                col14.add(snapshot[i].snapshot.bid_price[bidPriceIndex++]);
                col15.add(snapshot[i].snapshot.bid_price[bidPriceIndex++]);
                col16.add(snapshot[i].snapshot.bid_price[bidPriceIndex++]);
                col17.add(snapshot[i].snapshot.bid_price[bidPriceIndex++]);
                col18.add(snapshot[i].snapshot.bid_price[bidPriceIndex++]);
                col19.add(snapshot[i].snapshot.bid_price[bidPriceIndex++]);

                int bidVolumeIndex = 0;
                col20.add(snapshot[i].snapshot.bid_volume[bidVolumeIndex++]);
                col21.add(snapshot[i].snapshot.bid_volume[bidVolumeIndex++]);
                col22.add(snapshot[i].snapshot.bid_volume[bidVolumeIndex++]);
                col23.add(snapshot[i].snapshot.bid_volume[bidVolumeIndex++]);
                col24.add(snapshot[i].snapshot.bid_volume[bidVolumeIndex++]);
                col25.add(snapshot[i].snapshot.bid_volume[bidVolumeIndex++]);
                col26.add(snapshot[i].snapshot.bid_volume[bidVolumeIndex++]);
                col27.add(snapshot[i].snapshot.bid_volume[bidVolumeIndex++]);
                col28.add(snapshot[i].snapshot.bid_volume[bidVolumeIndex++]);
                col29.add(snapshot[i].snapshot.bid_volume[bidVolumeIndex++]);

                int offerPriceIndex = 0;
                col30.add(snapshot[i].snapshot.offer_price[offerPriceIndex++]);
                col31.add(snapshot[i].snapshot.offer_price[offerPriceIndex++]);
                col32.add(snapshot[i].snapshot.offer_price[offerPriceIndex++]);
                col33.add(snapshot[i].snapshot.offer_price[offerPriceIndex++]);
                col34.add(snapshot[i].snapshot.offer_price[offerPriceIndex++]);
                col35.add(snapshot[i].snapshot.offer_price[offerPriceIndex++]);
                col36.add(snapshot[i].snapshot.offer_price[offerPriceIndex++]);
                col37.add(snapshot[i].snapshot.offer_price[offerPriceIndex++]);
                col38.add(snapshot[i].snapshot.offer_price[offerPriceIndex++]);
                col39.add(snapshot[i].snapshot.offer_price[offerPriceIndex++]);

                int offerVolumeIndex = 0;
                col40.add(snapshot[i].snapshot.offer_volume[offerVolumeIndex++]);
                col41.add(snapshot[i].snapshot.offer_volume[offerVolumeIndex++]);
                col42.add(snapshot[i].snapshot.offer_volume[offerVolumeIndex++]);
                col43.add(snapshot[i].snapshot.offer_volume[offerVolumeIndex++]);
                col44.add(snapshot[i].snapshot.offer_volume[offerVolumeIndex++]);
                col45.add(snapshot[i].snapshot.offer_volume[offerVolumeIndex++]);
                col46.add(snapshot[i].snapshot.offer_volume[offerVolumeIndex++]);
                col47.add(snapshot[i].snapshot.offer_volume[offerVolumeIndex++]);
                col48.add(snapshot[i].snapshot.offer_volume[offerVolumeIndex++]);
                col49.add(snapshot[i].snapshot.offer_volume[offerVolumeIndex++]);

                col50.add(snapshot[i].snapshot.num_trades);
                col51.add(snapshot[i].snapshot.total_volume_trade);
                col52.add(snapshot[i].snapshot.total_value_trade);
                col53.add(snapshot[i].snapshot.total_bid_volume);
                col54.add(snapshot[i].snapshot.total_offer_volume);
                col55.add(snapshot[i].snapshot.weighted_avg_bid_price);
                col56.add(snapshot[i].snapshot.weighted_avg_offer_price);
                col57.add(snapshot[i].snapshot.IOPV);
                col58.add(snapshot[i].snapshot.yield_to_maturity);
                col59.add(snapshot[i].snapshot.high_limited);
                col60.add(snapshot[i].snapshot.low_limited);
                col61.add(snapshot[i].snapshot.price_earning_ratio1);
                col62.add(snapshot[i].snapshot.price_earning_ratio2);
                col63.add(snapshot[i].snapshot.change1);
                col64.add(snapshot[i].snapshot.change2);
                col65.add(snapshot[i].snapshot.channel_no);
                col66.add(snapshot[i].snapshot.md_stream_id);
                col67.add(snapshot[i].snapshot.instrument_status);
                col68.add(snapshot[i].snapshot.pre_close_iopv);
                col69.add(snapshot[i].snapshot.alt_weighted_avg_bid_price);
                col70.add(snapshot[i].snapshot.alt_weighted_avg_offer_price);
                col71.add(snapshot[i].snapshot.etf_buy_number);
                col72.add(snapshot[i].snapshot.etf_buy_amount);
                col73.add(snapshot[i].snapshot.etf_buy_money);
                col74.add(snapshot[i].snapshot.etf_sell_number);
                col75.add(snapshot[i].snapshot.etf_sell_amount);
                col76.add(snapshot[i].snapshot.etf_sell_money);
                col77.add(snapshot[i].snapshot.total_warrant_exec_volume);
                col78.add(snapshot[i].snapshot.war_lower_price);
                col79.add(snapshot[i].snapshot.war_upper_price);
                col80.add(snapshot[i].snapshot.withdraw_buy_number);
                col81.add(snapshot[i].snapshot.withdraw_buy_amount);
                col82.add(snapshot[i].snapshot.withdraw_buy_money);
                col83.add(snapshot[i].snapshot.withdraw_sell_number);
                col84.add(snapshot[i].snapshot.withdraw_sell_amount);
                col85.add(snapshot[i].snapshot.withdraw_sell_money);
                col86.add(snapshot[i].snapshot.total_bid_number);
                col87.add(snapshot[i].snapshot.total_offer_number);
                col88.add(snapshot[i].snapshot.bid_trade_max_duration);
                col89.add(snapshot[i].snapshot.offer_trade_max_duration);
                col90.add(snapshot[i].snapshot.num_bid_orders);
                col91.add(snapshot[i].snapshot.num_offer_orders);
                col92.add(snapshot[i].snapshot.last_trade_time);
                col93.add(snapshot[i].snapshot.variety_category);
                if(dailyIndexFlag_){
                    int dailyIndex = INT_MIN;
                    if(!getDailyIndex(dailyIndex, dailyIndex_, sizeof(dailyIndex_), snapshot[i].snapshot.market_type, datatype, snapshot[i].snapshot.channel_no, convertTime(snapshot[i].snapshot.orig_time))){
                        LOG_ERR("[PluginAmdQuote]: getDailyIndex failed. ");
                        return;
                    }
                    col95.add(dailyIndex);
                }
                if (receivedTimeFlag_) {
                    col94.add(snapshot[i].reachTime);
                }
                reachTimeVec.push_back(snapshot[i].reachTime);

            } else if(varietyCategory == 2) {
                fcol0.add(snapshot[i].snapshot.market_type);
                fcol1.add(snapshot[i].snapshot.security_code);
                fcol2.add(convertTime(snapshot[i].snapshot.orig_time));
                fcol3.add(snapshot[i].snapshot.trading_phase_code);
                fcol4.add(snapshot[i].snapshot.pre_close_price);
                fcol5.add(snapshot[i].snapshot.open_price);
                fcol6.add(snapshot[i].snapshot.high_price);
                fcol7.add(snapshot[i].snapshot.low_price);
                fcol8.add(snapshot[i].snapshot.last_price);
                fcol9.add(snapshot[i].snapshot.close_price);

                int bidPriceIndex = 0;
                fcol10.add(snapshot[i].snapshot.bid_price[bidPriceIndex++]);
                fcol11.add(snapshot[i].snapshot.bid_price[bidPriceIndex++]);
                fcol12.add(snapshot[i].snapshot.bid_price[bidPriceIndex++]);
                fcol13.add(snapshot[i].snapshot.bid_price[bidPriceIndex++]);
                fcol14.add(snapshot[i].snapshot.bid_price[bidPriceIndex++]);
                fcol15.add(snapshot[i].snapshot.bid_price[bidPriceIndex++]);
                fcol16.add(snapshot[i].snapshot.bid_price[bidPriceIndex++]);
                fcol17.add(snapshot[i].snapshot.bid_price[bidPriceIndex++]);
                fcol18.add(snapshot[i].snapshot.bid_price[bidPriceIndex++]);
                fcol19.add(snapshot[i].snapshot.bid_price[bidPriceIndex++]);

                int bidVolumeIndex = 0;
                fcol20.add(snapshot[i].snapshot.bid_volume[bidVolumeIndex++]);
                fcol21.add(snapshot[i].snapshot.bid_volume[bidVolumeIndex++]);
                fcol22.add(snapshot[i].snapshot.bid_volume[bidVolumeIndex++]);
                fcol23.add(snapshot[i].snapshot.bid_volume[bidVolumeIndex++]);
                fcol24.add(snapshot[i].snapshot.bid_volume[bidVolumeIndex++]);
                fcol25.add(snapshot[i].snapshot.bid_volume[bidVolumeIndex++]);
                fcol26.add(snapshot[i].snapshot.bid_volume[bidVolumeIndex++]);
                fcol27.add(snapshot[i].snapshot.bid_volume[bidVolumeIndex++]);
                fcol28.add(snapshot[i].snapshot.bid_volume[bidVolumeIndex++]);
                fcol29.add(snapshot[i].snapshot.bid_volume[bidVolumeIndex++]);

                int offerPriceIndex = 0;
                fcol30.add(snapshot[i].snapshot.offer_price[offerPriceIndex++]);
                fcol31.add(snapshot[i].snapshot.offer_price[offerPriceIndex++]);
                fcol32.add(snapshot[i].snapshot.offer_price[offerPriceIndex++]);
                fcol33.add(snapshot[i].snapshot.offer_price[offerPriceIndex++]);
                fcol34.add(snapshot[i].snapshot.offer_price[offerPriceIndex++]);
                fcol35.add(snapshot[i].snapshot.offer_price[offerPriceIndex++]);
                fcol36.add(snapshot[i].snapshot.offer_price[offerPriceIndex++]);
                fcol37.add(snapshot[i].snapshot.offer_price[offerPriceIndex++]);
                fcol38.add(snapshot[i].snapshot.offer_price[offerPriceIndex++]);
                fcol39.add(snapshot[i].snapshot.offer_price[offerPriceIndex++]);

                int offerVolumeIndex = 0;
                fcol40.add(snapshot[i].snapshot.offer_volume[offerVolumeIndex++]);
                fcol41.add(snapshot[i].snapshot.offer_volume[offerVolumeIndex++]);
                fcol42.add(snapshot[i].snapshot.offer_volume[offerVolumeIndex++]);
                fcol43.add(snapshot[i].snapshot.offer_volume[offerVolumeIndex++]);
                fcol44.add(snapshot[i].snapshot.offer_volume[offerVolumeIndex++]);
                fcol45.add(snapshot[i].snapshot.offer_volume[offerVolumeIndex++]);
                fcol46.add(snapshot[i].snapshot.offer_volume[offerVolumeIndex++]);
                fcol47.add(snapshot[i].snapshot.offer_volume[offerVolumeIndex++]);
                fcol48.add(snapshot[i].snapshot.offer_volume[offerVolumeIndex++]);
                fcol49.add(snapshot[i].snapshot.offer_volume[offerVolumeIndex++]);

                fcol50.add(snapshot[i].snapshot.num_trades);
                fcol51.add(snapshot[i].snapshot.total_volume_trade);
                fcol52.add(snapshot[i].snapshot.total_value_trade);
                fcol53.add(snapshot[i].snapshot.total_bid_volume);
                fcol54.add(snapshot[i].snapshot.total_offer_volume);
                fcol55.add(snapshot[i].snapshot.weighted_avg_bid_price);
                fcol56.add(snapshot[i].snapshot.weighted_avg_offer_price);
                fcol57.add(snapshot[i].snapshot.IOPV);
                fcol58.add(snapshot[i].snapshot.yield_to_maturity);
                fcol59.add(snapshot[i].snapshot.high_limited);
                fcol60.add(snapshot[i].snapshot.low_limited);
                fcol61.add(snapshot[i].snapshot.price_earning_ratio1);
                fcol62.add(snapshot[i].snapshot.price_earning_ratio2);
                fcol63.add(snapshot[i].snapshot.change1);
                fcol64.add(snapshot[i].snapshot.change2);
                fcol65.add(snapshot[i].snapshot.channel_no);
                fcol66.add(snapshot[i].snapshot.md_stream_id);
                fcol67.add(snapshot[i].snapshot.instrument_status);
                fcol68.add(snapshot[i].snapshot.pre_close_iopv);
                fcol69.add(snapshot[i].snapshot.alt_weighted_avg_bid_price);
                fcol70.add(snapshot[i].snapshot.alt_weighted_avg_offer_price);
                fcol71.add(snapshot[i].snapshot.etf_buy_number);
                fcol72.add(snapshot[i].snapshot.etf_buy_amount);
                fcol73.add(snapshot[i].snapshot.etf_buy_money);
                fcol74.add(snapshot[i].snapshot.etf_sell_number);
                fcol75.add(snapshot[i].snapshot.etf_sell_amount);
                fcol76.add(snapshot[i].snapshot.etf_sell_money);
                fcol77.add(snapshot[i].snapshot.total_warrant_exec_volume);
                fcol78.add(snapshot[i].snapshot.war_lower_price);
                fcol79.add(snapshot[i].snapshot.war_upper_price);
                fcol80.add(snapshot[i].snapshot.withdraw_buy_number);
                fcol81.add(snapshot[i].snapshot.withdraw_buy_amount);
                fcol82.add(snapshot[i].snapshot.withdraw_buy_money);
                fcol83.add(snapshot[i].snapshot.withdraw_sell_number);
                fcol84.add(snapshot[i].snapshot.withdraw_sell_amount);
                fcol85.add(snapshot[i].snapshot.withdraw_sell_money);
                fcol86.add(snapshot[i].snapshot.total_bid_number);
                fcol87.add(snapshot[i].snapshot.total_offer_number);
                fcol88.add(snapshot[i].snapshot.bid_trade_max_duration);
                fcol89.add(snapshot[i].snapshot.offer_trade_max_duration);
                fcol90.add(snapshot[i].snapshot.num_bid_orders);
                fcol91.add(snapshot[i].snapshot.num_offer_orders);
                fcol92.add(snapshot[i].snapshot.last_trade_time);
                fcol93.add(snapshot[i].snapshot.variety_category);
                if(dailyIndexFlag_){
                    int dailyIndex = INT_MIN;
                    if(!getDailyIndex(dailyIndex, dailyIndex_, sizeof(dailyIndex_), snapshot[i].snapshot.market_type, datatype, snapshot[i].snapshot.channel_no, convertTime(snapshot[i].snapshot.orig_time))){
                        LOG_ERR("[PluginAmdQuote]: getDailyIndex failed. ");
                        return;
                    }
                    fcol95.add(dailyIndex);
                }
                if (receivedTimeFlag_) {
                    fcol94.add(snapshot[i].reachTime);
                }
                fundReachTimeVec.push_back(snapshot[i].reachTime);
            }
        }
        if(snapshotFlag_) {
            vector<ConstantSP> cols;
            cols.push_back(col0.createVector(DT_INT));
            cols.push_back(col1.createVector(DT_SYMBOL));
            cols.push_back(col2.createVector(DT_TIMESTAMP));
            cols.push_back(col3.createVector(DT_STRING));
            cols.push_back(col4.createVector(DT_LONG));
            cols.push_back(col5.createVector(DT_LONG));
            cols.push_back(col6.createVector(DT_LONG));
            cols.push_back(col7.createVector(DT_LONG));
            cols.push_back(col8.createVector(DT_LONG));
            cols.push_back(col9.createVector(DT_LONG));

            cols.push_back(col10.createVector(DT_LONG));
            cols.push_back(col11.createVector(DT_LONG));
            cols.push_back(col12.createVector(DT_LONG));
            cols.push_back(col13.createVector(DT_LONG));
            cols.push_back(col14.createVector(DT_LONG));
            cols.push_back(col15.createVector(DT_LONG));
            cols.push_back(col16.createVector(DT_LONG));
            cols.push_back(col17.createVector(DT_LONG));
            cols.push_back(col18.createVector(DT_LONG));
            cols.push_back(col19.createVector(DT_LONG));

            cols.push_back(col20.createVector(DT_LONG));
            cols.push_back(col21.createVector(DT_LONG));
            cols.push_back(col22.createVector(DT_LONG));
            cols.push_back(col23.createVector(DT_LONG));
            cols.push_back(col24.createVector(DT_LONG));
            cols.push_back(col25.createVector(DT_LONG));
            cols.push_back(col26.createVector(DT_LONG));
            cols.push_back(col27.createVector(DT_LONG));
            cols.push_back(col28.createVector(DT_LONG));
            cols.push_back(col29.createVector(DT_LONG));

            cols.push_back(col30.createVector(DT_LONG));
            cols.push_back(col31.createVector(DT_LONG));
            cols.push_back(col32.createVector(DT_LONG));
            cols.push_back(col33.createVector(DT_LONG));
            cols.push_back(col34.createVector(DT_LONG));
            cols.push_back(col35.createVector(DT_LONG));
            cols.push_back(col36.createVector(DT_LONG));
            cols.push_back(col37.createVector(DT_LONG));
            cols.push_back(col38.createVector(DT_LONG));
            cols.push_back(col39.createVector(DT_LONG));

            cols.push_back(col40.createVector(DT_LONG));
            cols.push_back(col41.createVector(DT_LONG));
            cols.push_back(col42.createVector(DT_LONG));
            cols.push_back(col43.createVector(DT_LONG));
            cols.push_back(col44.createVector(DT_LONG));
            cols.push_back(col45.createVector(DT_LONG));
            cols.push_back(col46.createVector(DT_LONG));
            cols.push_back(col47.createVector(DT_LONG));
            cols.push_back(col48.createVector(DT_LONG));
            cols.push_back(col49.createVector(DT_LONG));

            cols.push_back(col50.createVector(DT_LONG));
            cols.push_back(col51.createVector(DT_LONG));
            cols.push_back(col52.createVector(DT_LONG));
            cols.push_back(col53.createVector(DT_LONG));
            cols.push_back(col54.createVector(DT_LONG));
            cols.push_back(col55.createVector(DT_LONG));
            cols.push_back(col56.createVector(DT_LONG));
            cols.push_back(col57.createVector(DT_LONG));
            cols.push_back(col58.createVector(DT_LONG));
            cols.push_back(col59.createVector(DT_LONG));

            cols.push_back(col60.createVector(DT_LONG));
            cols.push_back(col61.createVector(DT_LONG));
            cols.push_back(col62.createVector(DT_LONG));
            cols.push_back(col63.createVector(DT_LONG));
            cols.push_back(col64.createVector(DT_LONG));
            cols.push_back(col65.createVector(DT_INT));
            cols.push_back(col66.createVector(DT_STRING));
            cols.push_back(col67.createVector(DT_STRING));
            cols.push_back(col68.createVector(DT_LONG));
            cols.push_back(col69.createVector(DT_LONG));

            cols.push_back(col70.createVector(DT_LONG));
            cols.push_back(col71.createVector(DT_LONG));
            cols.push_back(col72.createVector(DT_LONG));
            cols.push_back(col73.createVector(DT_LONG));
            cols.push_back(col74.createVector(DT_LONG));
            cols.push_back(col75.createVector(DT_LONG));
            cols.push_back(col76.createVector(DT_LONG));
            cols.push_back(col77.createVector(DT_LONG));
            cols.push_back(col78.createVector(DT_LONG));
            cols.push_back(col79.createVector(DT_LONG));

            cols.push_back(col80.createVector(DT_LONG));
            cols.push_back(col81.createVector(DT_LONG));
            cols.push_back(col82.createVector(DT_LONG));
            cols.push_back(col83.createVector(DT_LONG));
            cols.push_back(col84.createVector(DT_LONG));
            cols.push_back(col85.createVector(DT_LONG));
            cols.push_back(col86.createVector(DT_LONG));
            cols.push_back(col87.createVector(DT_LONG));
            cols.push_back(col88.createVector(DT_INT));
            cols.push_back(col89.createVector(DT_INT));

            cols.push_back(col90.createVector(DT_INT));
            cols.push_back(col91.createVector(DT_INT));
            cols.push_back(col92.createVector(DT_LONG));
            cols.push_back(col93.createVector(DT_CHAR));
            
            vector<string> colNames = snapshotDataTableMeta_.colNames_;
            if(receivedTimeFlag_) {
                cols.push_back(col94.createVector(DT_NANOTIMESTAMP));
                colNames.push_back("receivedTime");
            }
            if(dailyIndexFlag_) {
                cols.push_back(col95.createVector(DT_INT));
                colNames.push_back("dailyIndex");
            }
            if(outputElapsedFlag_) {
                colNames.push_back("perPenetrationTime");
                long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                for(int i = 0; i < col0.size(); ++i) {
                    col96.add(time - reachTimeVec[i]);
                }
                cols.push_back(col96.createVector(DT_NANOTIME));
            }

            TableSP data = Util::createTable(colNames, cols);
            vector<ConstantSP> args = {data};
            try{
                if(!transform.isNull())
                    data = transform->call(session_->getHeap().get(), args);
            }catch(exception &e){
                throw RuntimeException("call transform error " + string(e.what()));
            }

            if(insertedTable.isNull())
                throw RuntimeException("insertedTable is null");
            if(insertedTable ->columns() != data->columns())
                throw RuntimeException("The number of columns of the table to insert must be the same as that of the original table");
            
            INDEX rows;
            string errMsg;
            vector<ConstantSP> colData(data->columns());
            for(INDEX i = 0; i < data->columns(); ++i) {
                colData[i] = data->getColumn(i);
            }
            insertedTable->append(colData, rows, errMsg);
            if(errMsg != "") {
                LOG_ERR("[PluginAmdQuote]: OnMDsnapshot append failed, ", errMsg);
                return;
            }

            if (latencyFlag_) {
                long long diff = Util::toLocalNanoTimestamp(Util::getNanoEpochTime() - startTime);
                latencyLog(0, startTime, cnt, diff);
            }
        }
        
        
        if(fundSnapshotFlag_) {
            vector<ConstantSP> fcols;
            fcols.push_back(fcol0.createVector(DT_INT));
            fcols.push_back(fcol1.createVector(DT_SYMBOL));
            fcols.push_back(fcol2.createVector(DT_TIMESTAMP));
            fcols.push_back(fcol3.createVector(DT_STRING));
            fcols.push_back(fcol4.createVector(DT_LONG));
            fcols.push_back(fcol5.createVector(DT_LONG));
            fcols.push_back(fcol6.createVector(DT_LONG));
            fcols.push_back(fcol7.createVector(DT_LONG));
            fcols.push_back(fcol8.createVector(DT_LONG));
            fcols.push_back(fcol9.createVector(DT_LONG));

            fcols.push_back(fcol10.createVector(DT_LONG));
            fcols.push_back(fcol11.createVector(DT_LONG));
            fcols.push_back(fcol12.createVector(DT_LONG));
            fcols.push_back(fcol13.createVector(DT_LONG));
            fcols.push_back(fcol14.createVector(DT_LONG));
            fcols.push_back(fcol15.createVector(DT_LONG));
            fcols.push_back(fcol16.createVector(DT_LONG));
            fcols.push_back(fcol17.createVector(DT_LONG));
            fcols.push_back(fcol18.createVector(DT_LONG));
            fcols.push_back(fcol19.createVector(DT_LONG));

            fcols.push_back(fcol20.createVector(DT_LONG));
            fcols.push_back(fcol21.createVector(DT_LONG));
            fcols.push_back(fcol22.createVector(DT_LONG));
            fcols.push_back(fcol23.createVector(DT_LONG));
            fcols.push_back(fcol24.createVector(DT_LONG));
            fcols.push_back(fcol25.createVector(DT_LONG));
            fcols.push_back(fcol26.createVector(DT_LONG));
            fcols.push_back(fcol27.createVector(DT_LONG));
            fcols.push_back(fcol28.createVector(DT_LONG));
            fcols.push_back(fcol29.createVector(DT_LONG));

            fcols.push_back(fcol30.createVector(DT_LONG));
            fcols.push_back(fcol31.createVector(DT_LONG));
            fcols.push_back(fcol32.createVector(DT_LONG));
            fcols.push_back(fcol33.createVector(DT_LONG));
            fcols.push_back(fcol34.createVector(DT_LONG));
            fcols.push_back(fcol35.createVector(DT_LONG));
            fcols.push_back(fcol36.createVector(DT_LONG));
            fcols.push_back(fcol37.createVector(DT_LONG));
            fcols.push_back(fcol38.createVector(DT_LONG));
            fcols.push_back(fcol39.createVector(DT_LONG));

            fcols.push_back(fcol40.createVector(DT_LONG));
            fcols.push_back(fcol41.createVector(DT_LONG));
            fcols.push_back(fcol42.createVector(DT_LONG));
            fcols.push_back(fcol43.createVector(DT_LONG));
            fcols.push_back(fcol44.createVector(DT_LONG));
            fcols.push_back(fcol45.createVector(DT_LONG));
            fcols.push_back(fcol46.createVector(DT_LONG));
            fcols.push_back(fcol47.createVector(DT_LONG));
            fcols.push_back(fcol48.createVector(DT_LONG));
            fcols.push_back(fcol49.createVector(DT_LONG));

            fcols.push_back(fcol50.createVector(DT_LONG));
            fcols.push_back(fcol51.createVector(DT_LONG));
            fcols.push_back(fcol52.createVector(DT_LONG));
            fcols.push_back(fcol53.createVector(DT_LONG));
            fcols.push_back(fcol54.createVector(DT_LONG));
            fcols.push_back(fcol55.createVector(DT_LONG));
            fcols.push_back(fcol56.createVector(DT_LONG));
            fcols.push_back(fcol57.createVector(DT_LONG));
            fcols.push_back(fcol58.createVector(DT_LONG));
            fcols.push_back(fcol59.createVector(DT_LONG));

            fcols.push_back(fcol60.createVector(DT_LONG));
            fcols.push_back(fcol61.createVector(DT_LONG));
            fcols.push_back(fcol62.createVector(DT_LONG));
            fcols.push_back(fcol63.createVector(DT_LONG));
            fcols.push_back(fcol64.createVector(DT_LONG));
            fcols.push_back(fcol65.createVector(DT_INT));
            fcols.push_back(fcol66.createVector(DT_STRING));
            fcols.push_back(fcol67.createVector(DT_STRING));
            fcols.push_back(fcol68.createVector(DT_LONG));
            fcols.push_back(fcol69.createVector(DT_LONG));

            fcols.push_back(fcol70.createVector(DT_LONG));
            fcols.push_back(fcol71.createVector(DT_LONG));
            fcols.push_back(fcol72.createVector(DT_LONG));
            fcols.push_back(fcol73.createVector(DT_LONG));
            fcols.push_back(fcol74.createVector(DT_LONG));
            fcols.push_back(fcol75.createVector(DT_LONG));
            fcols.push_back(fcol76.createVector(DT_LONG));
            fcols.push_back(fcol77.createVector(DT_LONG));
            fcols.push_back(fcol78.createVector(DT_LONG));
            fcols.push_back(fcol79.createVector(DT_LONG));

            fcols.push_back(fcol80.createVector(DT_LONG));
            fcols.push_back(fcol81.createVector(DT_LONG));
            fcols.push_back(fcol82.createVector(DT_LONG));
            fcols.push_back(fcol83.createVector(DT_LONG));
            fcols.push_back(fcol84.createVector(DT_LONG));
            fcols.push_back(fcol85.createVector(DT_LONG));
            fcols.push_back(fcol86.createVector(DT_LONG));
            fcols.push_back(fcol87.createVector(DT_LONG));
            fcols.push_back(fcol88.createVector(DT_INT));
            fcols.push_back(fcol89.createVector(DT_INT));

            fcols.push_back(fcol90.createVector(DT_INT));
            fcols.push_back(fcol91.createVector(DT_INT));
            fcols.push_back(fcol92.createVector(DT_LONG));
            fcols.push_back(fcol93.createVector(DT_CHAR));
            
            vector<string> fcolNames = snapshotDataTableMeta_.colNames_;
            if(receivedTimeFlag_) {
                fcolNames.push_back("receivedTime");
                fcols.push_back(fcol94.createVector(DT_NANOTIMESTAMP));
            }
            if(dailyIndexFlag_) {
                fcolNames.push_back("dailyIndex");
                fcols.push_back(fcol95.createVector(DT_INT));
            }
            if(outputElapsedFlag_) {
                fcolNames.push_back("perPenetrationTime");
                long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                for(int i = 0; i < fcol0.size(); ++i) {
                    fcol96.add(time - fundReachTimeVec[i]);
                }
                fcols.push_back(fcol96.createVector(DT_NANOTIME));
            }

            TableSP data = Util::createTable(fcolNames, fcols);
            vector<ConstantSP> args = {data};
            try{
                if(!fundTransform.isNull())
                    data = fundTransform->call(session_->getHeap().get(), args);
            }catch(exception &e){
                throw RuntimeException("call transform error " + string(e.what()));
            }

            if(fundInsertedTable.isNull())
                throw RuntimeException("insertedTable is null");
            if(fundInsertedTable ->columns() != data->columns())
                throw RuntimeException("The number of columns of the table to insert must be the same as that of the original table");
            
            INDEX rows;
            string errMsg;
            vector<ConstantSP> colData(data->columns());
            for(INDEX i = 0; i < data->columns(); ++i) {
                colData[i] = data->getColumn(i);
            }
            fundInsertedTable->append(colData, rows, errMsg);
            if(errMsg != "") {
                LOG_ERR("[PluginAmdQuote]: OnMDsnapshot append failed, ", errMsg);
                return;
            }

            if (latencyFlag_) {
                long long diff = Util::toLocalNanoTimestamp(Util::getNanoEpochTime() - startTime);
                latencyLog(0, startTime, cnt, diff);
            }
        }
    }
    catch(exception &e){
        LOG_ERR("[PluginAmdQuote]: OnMDsnapshot failed, ", e.what());
    }
}

// 接受并处理快照数据(债券）
void AMDSpiImp::OnMDBondSnapshotHelper(timeMDBondSnapshot* snapshot, uint32_t cnt) {
    try{
        if(cnt == 0)return;
        long long startTime = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
        // std::lock_guard<std::mutex> amdLock_(amdMutex_);

        // 基金和股票数据插入到不同的表
        TableSP insertedTable;
        uint8_t varietyCategory = snapshot[0].bondSnapshot.variety_category;
        FunctionDefSP transform;

        AMDDataType datatype;
        if (varietyCategory == 3) { // 债券快照
            insertedTable = bondSnapshotData_;
            transform = bondSnapshotTransform_;
            datatype = AMD_BOND_SNAPSHOT;
            if(!bondSnapshotFlag_) {
                return;
            }
        }else {
            return ;
        }

        int market_type = snapshot[0].bondSnapshot.market_type;
        AMDTableType tableTyeTotal = getAmdTableType(datatype, market_type);
        if(tableTyeTotal == AMD_ERROR_TABLE_TYPE){
            LOG_ERR("[PluginAmdQuote]: error amd table type AMD_ERROR_TABLE_TYPE");
            return;
        }

        vector<long long> reachTimeVec;
        if(outputElapsedFlag_) {
            reachTimeVec.reserve(cnt);
        }

        DdbVector<int> col0(0, cnt);
        DdbVector<string> col1(0, cnt);
        DdbVector<long long> col2(0, cnt);
        DdbVector<string> col3(0, cnt);
        DdbVector<long long> col4(0, cnt);
        DdbVector<long long> col5(0, cnt);
        DdbVector<long long> col6(0, cnt);
        DdbVector<long long> col7(0, cnt);
        DdbVector<long long> col8(0, cnt);
        DdbVector<long long> col9(0, cnt);

        DdbVector<long long> col10(0, cnt);
        DdbVector<long long> col11(0, cnt);
        DdbVector<long long> col12(0, cnt);
        DdbVector<long long> col13(0, cnt);
        DdbVector<long long> col14(0, cnt);
        DdbVector<long long> col15(0, cnt);
        DdbVector<long long> col16(0, cnt);
        DdbVector<long long> col17(0, cnt);
        DdbVector<long long> col18(0, cnt);
        DdbVector<long long> col19(0, cnt);

        DdbVector<long long> col20(0, cnt);
        DdbVector<long long> col21(0, cnt);
        DdbVector<long long> col22(0, cnt);
        DdbVector<long long> col23(0, cnt);
        DdbVector<long long> col24(0, cnt);
        DdbVector<long long> col25(0, cnt);
        DdbVector<long long> col26(0, cnt);
        DdbVector<long long> col27(0, cnt);
        DdbVector<long long> col28(0, cnt);
        DdbVector<long long> col29(0, cnt);

        DdbVector<long long> col30(0, cnt);
        DdbVector<long long> col31(0, cnt);
        DdbVector<long long> col32(0, cnt);
        DdbVector<long long> col33(0, cnt);
        DdbVector<long long> col34(0, cnt);
        DdbVector<long long> col35(0, cnt);
        DdbVector<long long> col36(0, cnt);
        DdbVector<long long> col37(0, cnt);
        DdbVector<long long> col38(0, cnt);
        DdbVector<long long> col39(0, cnt);
        
        DdbVector<long long> col40(0, cnt);
        DdbVector<long long> col41(0, cnt);
        DdbVector<long long> col42(0, cnt);
        DdbVector<long long> col43(0, cnt);
        DdbVector<long long> col44(0, cnt);
        DdbVector<long long> col45(0, cnt);
        DdbVector<long long> col46(0, cnt);
        DdbVector<long long> col47(0, cnt);
        DdbVector<long long> col48(0, cnt);
        DdbVector<long long> col49(0, cnt);

        DdbVector<long long> col50(0, cnt);
        DdbVector<long long> col51(0, cnt);
        DdbVector<long long> col52(0, cnt);
        DdbVector<long long> col53(0, cnt);
        DdbVector<long long> col54(0, cnt);
        DdbVector<long long> col55(0, cnt);
        DdbVector<long long> col56(0, cnt);
        DdbVector<long long> col57(0, cnt);
        DdbVector<long long> col58(0, cnt);
        DdbVector<long long> col59(0, cnt);

        DdbVector<long long> col60(0, cnt);
        DdbVector<long long> col61(0, cnt);
        DdbVector<long long> col62(0, cnt);
        DdbVector<long long> col63(0, cnt);
        DdbVector<long long> col64(0, cnt);
        DdbVector<int> col65(0, cnt);
        DdbVector<string> col66(0, cnt);
        DdbVector<string> col67(0, cnt);
        DdbVector<long long> col68(0, cnt);
        DdbVector<long long> col69(0, cnt);

        DdbVector<long long> col70(0, cnt);
        DdbVector<long long> col71(0, cnt);
        DdbVector<long long> col72(0, cnt);
        DdbVector<long long> col73(0, cnt);
        DdbVector<long long> col74(0, cnt);
        DdbVector<long long> col75(0, cnt);
        DdbVector<long long> col76(0, cnt);
        DdbVector<long long> col77(0, cnt);
        DdbVector<long long> col78(0, cnt);
        DdbVector<long long> col79(0, cnt);

        DdbVector<long long> col80(0, cnt);
        DdbVector<long long> col81(0, cnt);
        DdbVector<long long> col82(0, cnt);
        DdbVector<long long> col83(0, cnt);
        DdbVector<long long> col84(0, cnt);
        DdbVector<long long> col85(0, cnt);
        DdbVector<long long> col86(0, cnt);
        DdbVector<long long> col87(0, cnt);
        DdbVector<int> col88(0, cnt);
        DdbVector<int> col89(0, cnt);

        DdbVector<int> col90(0, cnt);
        DdbVector<int> col91(0, cnt);
        DdbVector<long long> col92(0, cnt);
        DdbVector<char> col93(0, cnt);
        DdbVector<long long> col94(0, cnt);
        DdbVector<int> col95(0, cnt);
        DdbVector<long long> col96(0, cnt);


        for (uint32_t i = 0; i < cnt; ++i) {
            col0.add(snapshot[i].bondSnapshot.market_type);
            col1.add(snapshot[i].bondSnapshot.security_code);
            col2.add(convertTime(snapshot[i].bondSnapshot.orig_time));
            col3.add(snapshot[i].bondSnapshot.trading_phase_code);
            col4.add(snapshot[i].bondSnapshot.pre_close_price);
            col5.add(snapshot[i].bondSnapshot.open_price);
            col6.add(snapshot[i].bondSnapshot.high_price);
            col7.add(snapshot[i].bondSnapshot.low_price);
            col8.add(snapshot[i].bondSnapshot.last_price);
            col9.add(snapshot[i].bondSnapshot.close_price);

            int bidPriceIndex = 0;
            col10.add(snapshot[i].bondSnapshot.bid_price[bidPriceIndex++]);
            col11.add(snapshot[i].bondSnapshot.bid_price[bidPriceIndex++]);
            col12.add(snapshot[i].bondSnapshot.bid_price[bidPriceIndex++]);
            col13.add(snapshot[i].bondSnapshot.bid_price[bidPriceIndex++]);
            col14.add(snapshot[i].bondSnapshot.bid_price[bidPriceIndex++]);
            col15.add(snapshot[i].bondSnapshot.bid_price[bidPriceIndex++]);
            col16.add(snapshot[i].bondSnapshot.bid_price[bidPriceIndex++]);
            col17.add(snapshot[i].bondSnapshot.bid_price[bidPriceIndex++]);
            col18.add(snapshot[i].bondSnapshot.bid_price[bidPriceIndex++]);
            col19.add(snapshot[i].bondSnapshot.bid_price[bidPriceIndex++]);

            int bidVolumeIndex = 0;
            col20.add(snapshot[i].bondSnapshot.bid_volume[bidVolumeIndex++]);
            col21.add(snapshot[i].bondSnapshot.bid_volume[bidVolumeIndex++]);
            col22.add(snapshot[i].bondSnapshot.bid_volume[bidVolumeIndex++]);
            col23.add(snapshot[i].bondSnapshot.bid_volume[bidVolumeIndex++]);
            col24.add(snapshot[i].bondSnapshot.bid_volume[bidVolumeIndex++]);
            col25.add(snapshot[i].bondSnapshot.bid_volume[bidVolumeIndex++]);
            col26.add(snapshot[i].bondSnapshot.bid_volume[bidVolumeIndex++]);
            col27.add(snapshot[i].bondSnapshot.bid_volume[bidVolumeIndex++]);
            col28.add(snapshot[i].bondSnapshot.bid_volume[bidVolumeIndex++]);
            col29.add(snapshot[i].bondSnapshot.bid_volume[bidVolumeIndex++]);

            int offerPriceIndex = 0;
            col30.add(snapshot[i].bondSnapshot.offer_price[offerPriceIndex++]);
            col31.add(snapshot[i].bondSnapshot.offer_price[offerPriceIndex++]);
            col32.add(snapshot[i].bondSnapshot.offer_price[offerPriceIndex++]);
            col33.add(snapshot[i].bondSnapshot.offer_price[offerPriceIndex++]);
            col34.add(snapshot[i].bondSnapshot.offer_price[offerPriceIndex++]);
            col35.add(snapshot[i].bondSnapshot.offer_price[offerPriceIndex++]);
            col36.add(snapshot[i].bondSnapshot.offer_price[offerPriceIndex++]);
            col37.add(snapshot[i].bondSnapshot.offer_price[offerPriceIndex++]);
            col38.add(snapshot[i].bondSnapshot.offer_price[offerPriceIndex++]);
            col39.add(snapshot[i].bondSnapshot.offer_price[offerPriceIndex++]);

            int offerVolumeIndex = 0;
            col40.add(snapshot[i].bondSnapshot.offer_volume[offerVolumeIndex++]);
            col41.add(snapshot[i].bondSnapshot.offer_volume[offerVolumeIndex++]);
            col42.add(snapshot[i].bondSnapshot.offer_volume[offerVolumeIndex++]);
            col43.add(snapshot[i].bondSnapshot.offer_volume[offerVolumeIndex++]);
            col44.add(snapshot[i].bondSnapshot.offer_volume[offerVolumeIndex++]);
            col45.add(snapshot[i].bondSnapshot.offer_volume[offerVolumeIndex++]);
            col46.add(snapshot[i].bondSnapshot.offer_volume[offerVolumeIndex++]);
            col47.add(snapshot[i].bondSnapshot.offer_volume[offerVolumeIndex++]);
            col48.add(snapshot[i].bondSnapshot.offer_volume[offerVolumeIndex++]);
            col49.add(snapshot[i].bondSnapshot.offer_volume[offerVolumeIndex++]);

            col50.add(snapshot[i].bondSnapshot.num_trades);
            col51.add(snapshot[i].bondSnapshot.total_volume_trade);
            col52.add(snapshot[i].bondSnapshot.total_value_trade);
            col53.add(snapshot[i].bondSnapshot.total_bid_volume);
            col54.add(snapshot[i].bondSnapshot.total_offer_volume);
            col55.add(snapshot[i].bondSnapshot.weighted_avg_bid_price);
            col56.add(snapshot[i].bondSnapshot.weighted_avg_offer_price);
            // col57.add(snapshot[i].bondSnapshot.IOPV);
            col57.add(0);
            // col58.add(snapshot[i].bondSnapshot.yield_to_maturity);
            col58.add(LONG_LONG_MIN);
            col59.add(snapshot[i].bondSnapshot.high_limited);
            col60.add(snapshot[i].bondSnapshot.low_limited);
            // col61.add(snapshot[i].bondSnapshot.price_earning_ratio1);
            col61.add(LONG_LONG_MIN);
            // col62.add(snapshot[i].bondSnapshot.price_earning_ratio2);
            col62.add(LONG_LONG_MIN);
            col63.add(snapshot[i].bondSnapshot.change1);
            col64.add(snapshot[i].bondSnapshot.change2);
            col65.add(snapshot[i].bondSnapshot.channel_no);
            col66.add(snapshot[i].bondSnapshot.md_stream_id);
            col67.add(snapshot[i].bondSnapshot.instrument_status);
            // col68.add(snapshot[i].bondSnapshot.pre_close_iopv);
            col68.add(LONG_LONG_MIN);
            // col69.add(snapshot[i].bondSnapshot.alt_weighted_avg_bid_price);
            col69.add(LONG_LONG_MIN);
            // col70.add(snapshot[i].bondSnapshot.alt_weighted_avg_offer_price);
            col70.add(LONG_LONG_MIN);
            // col71.add(snapshot[i].bondSnapshot.etf_buy_number);
            // col72.add(snapshot[i].bondSnapshot.etf_buy_amount);
            // col73.add(snapshot[i].bondSnapshot.etf_buy_money);
            // col74.add(snapshot[i].bondSnapshot.etf_sell_number);
            // col75.add(snapshot[i].bondSnapshot.etf_sell_amount);
            // col76.add(snapshot[i].bondSnapshot.etf_sell_money);
            col71.add(LONG_LONG_MIN);
            col72.add(LONG_LONG_MIN);
            col73.add(LONG_LONG_MIN);
            col74.add(LONG_LONG_MIN);
            col75.add(LONG_LONG_MIN);
            col76.add(LONG_LONG_MIN);
            // col77.add(snapshot[i].bondSnapshot.total_warrant_exec_volume);
            // col78.add(snapshot[i].bondSnapshot.war_lower_price);
            // col79.add(snapshot[i].bondSnapshot.war_upper_price);
            col77.add(LONG_LONG_MIN);
            col78.add(LONG_LONG_MIN);
            col79.add(LONG_LONG_MIN);
            col80.add(snapshot[i].bondSnapshot.withdraw_buy_number);
            col81.add(snapshot[i].bondSnapshot.withdraw_buy_amount);
            col82.add(snapshot[i].bondSnapshot.withdraw_buy_money);
            col83.add(snapshot[i].bondSnapshot.withdraw_sell_number);
            col84.add(snapshot[i].bondSnapshot.withdraw_sell_amount);
            col85.add(snapshot[i].bondSnapshot.withdraw_sell_money);
            col86.add(snapshot[i].bondSnapshot.total_bid_number);
            col87.add(snapshot[i].bondSnapshot.total_offer_number);
            col88.add(snapshot[i].bondSnapshot.bid_trade_max_duration);
            col89.add(snapshot[i].bondSnapshot.offer_trade_max_duration);
            col90.add(snapshot[i].bondSnapshot.num_bid_orders);
            col91.add(snapshot[i].bondSnapshot.num_offer_orders);
            col92.add(snapshot[i].bondSnapshot.last_trade_time);
            col93.add(snapshot[i].bondSnapshot.variety_category);

            if(dailyIndexFlag_){
                int dailyIndex = INT_MIN;
                if(!getDailyIndex(dailyIndex, dailyIndex_, sizeof(dailyIndex_), snapshot[i].bondSnapshot.market_type, datatype, snapshot[i].bondSnapshot.channel_no, convertTime(snapshot[i].bondSnapshot.orig_time))){
                    LOG_ERR("[PluginAmdQuote]: getDailyIndex failed. ");
                    return;
                }
                col95.add(dailyIndex);
            }
            if (receivedTimeFlag_) {
                col94.add(snapshot[i].reachTime);
            }
            reachTimeVec.push_back(snapshot[i].reachTime);
        }

        vector<ConstantSP> cols;
        cols.push_back(col0.createVector(DT_INT));
        cols.push_back(col1.createVector(DT_SYMBOL));
        cols.push_back(col2.createVector(DT_TIMESTAMP));
        cols.push_back(col3.createVector(DT_STRING));
        cols.push_back(col4.createVector(DT_LONG));
        cols.push_back(col5.createVector(DT_LONG));
        cols.push_back(col6.createVector(DT_LONG));
        cols.push_back(col7.createVector(DT_LONG));
        cols.push_back(col8.createVector(DT_LONG));
        cols.push_back(col9.createVector(DT_LONG));

        cols.push_back(col10.createVector(DT_LONG));
        cols.push_back(col11.createVector(DT_LONG));
        cols.push_back(col12.createVector(DT_LONG));
        cols.push_back(col13.createVector(DT_LONG));
        cols.push_back(col14.createVector(DT_LONG));
        cols.push_back(col15.createVector(DT_LONG));
        cols.push_back(col16.createVector(DT_LONG));
        cols.push_back(col17.createVector(DT_LONG));
        cols.push_back(col18.createVector(DT_LONG));
        cols.push_back(col19.createVector(DT_LONG));

        cols.push_back(col20.createVector(DT_LONG));
        cols.push_back(col21.createVector(DT_LONG));
        cols.push_back(col22.createVector(DT_LONG));
        cols.push_back(col23.createVector(DT_LONG));
        cols.push_back(col24.createVector(DT_LONG));
        cols.push_back(col25.createVector(DT_LONG));
        cols.push_back(col26.createVector(DT_LONG));
        cols.push_back(col27.createVector(DT_LONG));
        cols.push_back(col28.createVector(DT_LONG));
        cols.push_back(col29.createVector(DT_LONG));

        cols.push_back(col30.createVector(DT_LONG));
        cols.push_back(col31.createVector(DT_LONG));
        cols.push_back(col32.createVector(DT_LONG));
        cols.push_back(col33.createVector(DT_LONG));
        cols.push_back(col34.createVector(DT_LONG));
        cols.push_back(col35.createVector(DT_LONG));
        cols.push_back(col36.createVector(DT_LONG));
        cols.push_back(col37.createVector(DT_LONG));
        cols.push_back(col38.createVector(DT_LONG));
        cols.push_back(col39.createVector(DT_LONG));

        cols.push_back(col40.createVector(DT_LONG));
        cols.push_back(col41.createVector(DT_LONG));
        cols.push_back(col42.createVector(DT_LONG));
        cols.push_back(col43.createVector(DT_LONG));
        cols.push_back(col44.createVector(DT_LONG));
        cols.push_back(col45.createVector(DT_LONG));
        cols.push_back(col46.createVector(DT_LONG));
        cols.push_back(col47.createVector(DT_LONG));
        cols.push_back(col48.createVector(DT_LONG));
        cols.push_back(col49.createVector(DT_LONG));

        cols.push_back(col50.createVector(DT_LONG));
        cols.push_back(col51.createVector(DT_LONG));
        cols.push_back(col52.createVector(DT_LONG));
        cols.push_back(col53.createVector(DT_LONG));
        cols.push_back(col54.createVector(DT_LONG));
        cols.push_back(col55.createVector(DT_LONG));
        cols.push_back(col56.createVector(DT_LONG));
        cols.push_back(col57.createVector(DT_LONG));
        cols.push_back(col58.createVector(DT_LONG));
        cols.push_back(col59.createVector(DT_LONG));

        cols.push_back(col60.createVector(DT_LONG));
        cols.push_back(col61.createVector(DT_LONG));
        cols.push_back(col62.createVector(DT_LONG));
        cols.push_back(col63.createVector(DT_LONG));
        cols.push_back(col64.createVector(DT_LONG));
        cols.push_back(col65.createVector(DT_INT));
        cols.push_back(col66.createVector(DT_STRING));
        cols.push_back(col67.createVector(DT_STRING));
        cols.push_back(col68.createVector(DT_LONG));
        cols.push_back(col69.createVector(DT_LONG));

        cols.push_back(col70.createVector(DT_LONG));
        cols.push_back(col71.createVector(DT_LONG));
        cols.push_back(col72.createVector(DT_LONG));
        cols.push_back(col73.createVector(DT_LONG));
        cols.push_back(col74.createVector(DT_LONG));
        cols.push_back(col75.createVector(DT_LONG));
        cols.push_back(col76.createVector(DT_LONG));
        cols.push_back(col77.createVector(DT_LONG));
        cols.push_back(col78.createVector(DT_LONG));
        cols.push_back(col79.createVector(DT_LONG));

        cols.push_back(col80.createVector(DT_LONG));
        cols.push_back(col81.createVector(DT_LONG));
        cols.push_back(col82.createVector(DT_LONG));
        cols.push_back(col83.createVector(DT_LONG));
        cols.push_back(col84.createVector(DT_LONG));
        cols.push_back(col85.createVector(DT_LONG));
        cols.push_back(col86.createVector(DT_LONG));
        cols.push_back(col87.createVector(DT_LONG));
        cols.push_back(col88.createVector(DT_INT));
        cols.push_back(col89.createVector(DT_INT));

        cols.push_back(col90.createVector(DT_INT));
        cols.push_back(col91.createVector(DT_INT));
        cols.push_back(col92.createVector(DT_LONG));
        cols.push_back(col93.createVector(DT_CHAR));
        
        if(receivedTimeFlag_)
            cols.push_back(col94.createVector(DT_NANOTIMESTAMP));
        if(dailyIndexFlag_)
            cols.push_back(col95.createVector(DT_INT));
        if(outputElapsedFlag_) {
            long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
            for(int i = 0; i < col0.size(); ++i) {
                col96.add(time - reachTimeVec[i]);
            }
            cols.push_back(col96.createVector(DT_NANOTIME));
        }

        vector<string> colNames = snapshotDataTableMeta_.colNames_;
        if(receivedTimeFlag_)
            colNames.push_back("receivedTime");
        if(dailyIndexFlag_)
            colNames.push_back("dailyIndex");
        if(outputElapsedFlag_)
            colNames.push_back("perPenetrationTime");
        
        TableSP data = Util::createTable(colNames, cols);
        vector<ConstantSP> args = {data};
        try{
            if(!transform.isNull())
                data = transform->call(session_->getHeap().get(), args);
        }catch(exception &e){
            throw RuntimeException("call transform error " + string(e.what()));
        }

        if(insertedTable.isNull())
            throw RuntimeException("insertedTable is null");
        if(insertedTable ->columns() != data->columns())
            throw RuntimeException("The number of columns of the table to insert must be the same as that of the original table");

        INDEX rows;
        string errMsg;
        vector<ConstantSP> colData(data->columns());
        for(INDEX i = 0; i < data->columns(); ++i) {
            colData[i] = data->getColumn(i);
        }
        insertedTable->append(colData, rows, errMsg);
        if(errMsg != "") {
            LOG_ERR("[PluginAmdQuote]: OnMDBondSnapshot append failed, ", errMsg);
            return;
        }

        if (latencyFlag_) {
            long long diff = Util::toLocalNanoTimestamp(Util::getNanoEpochTime() - startTime);
            latencyLog(3, startTime, cnt, diff);
        }
    }
    catch(exception &e){
        LOG_ERR("[PluginAmdQuote]: OnMDBondSnapshot failed, ", e.what());
    }
}

// 接受并处理逐笔委托数据（包含基金和股票数据）
void AMDSpiImp::OnMDTickOrderHelper(timeMDTickOrder* ticks, uint32_t cnt) {
    try{
        if(cnt == 0) return;
        long long startTime = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
        // std::lock_guard<std::mutex> amdLock_(amdMutex_);

        // 基金和股票数据插入到不同的表
        TableSP insertedTable = orderData_;
        FunctionDefSP transform = orderTransform_;
        TableSP fundInsertedTable = fundOrderData_;
        FunctionDefSP fundTransform = fundOrderTransform_;

        uint8_t varietyCategory;
        AMDDataType datatype;
        int market_type;
        
        vector<long long> reachTimeVec;
        if(outputElapsedFlag_) {
            reachTimeVec.reserve(cnt);
        }
        vector<long long> fundReachTimeVec;
        if(outputElapsedFlag_) {
            fundReachTimeVec.reserve(cnt);
        }

        DdbVector<int> col0(0, cnt);
        DdbVector<string> col1(0, cnt);
        DdbVector<int> col2(0, cnt);
        DdbVector<long long> col3(0, cnt);
        DdbVector<long long> col4(0, cnt);
        DdbVector<long long> col5(0, cnt);
        DdbVector<long long> col6(0, cnt);
        DdbVector<char> col7(0, cnt);
        DdbVector<char> col8(0, cnt);
        DdbVector<string> col9(0, cnt);
        DdbVector<long long> col10(0, cnt);
        DdbVector<long long> col11(0, cnt);
        DdbVector<char> col12(0, cnt);
        DdbVector<long long> col13(0, cnt);
        DdbVector<int> col14(0, cnt);
        DdbVector<long long> col15(0, cnt);        
        
        DdbVector<int> fcol0(0, cnt);
        DdbVector<string> fcol1(0, cnt);
        DdbVector<int> fcol2(0, cnt);
        DdbVector<long long> fcol3(0, cnt);
        DdbVector<long long> fcol4(0, cnt);
        DdbVector<long long> fcol5(0, cnt);
        DdbVector<long long> fcol6(0, cnt);
        DdbVector<char> fcol7(0, cnt);
        DdbVector<char> fcol8(0, cnt);
        DdbVector<string> fcol9(0, cnt);
        DdbVector<long long> fcol10(0, cnt);
        DdbVector<long long> fcol11(0, cnt);
        DdbVector<char> fcol12(0, cnt);
        DdbVector<long long> fcol13(0, cnt);
        DdbVector<int> fcol14(0, cnt);
        DdbVector<long long> fcol15(0, cnt);

        for (uint32_t i = 0; i < cnt; ++i) {
            varietyCategory = ticks[i].order.variety_category;
            if (varietyCategory == 1) {
                datatype = AMD_ORDER;
                if(!orderFlag_) {
                    continue;
                }
            } else if(varietyCategory == 2){
                datatype = AMD_FUND_ORDER;
                if(!fundOrderFlag_) {
                    continue;
                }
            } else {
                continue;
            }
            market_type = ticks[i].order.market_type;
            AMDTableType tableTyeTotal = getAmdTableType(datatype, market_type);

            if(tableTyeTotal == AMD_ERROR_TABLE_TYPE){
                LOG_ERR("[PluginAmdQuote]: error amd table type AMD_ERROR_TABLE_TYPE");
                return;
            }
            if(varietyCategory == 1) {
                col0.add(ticks[i].order.market_type);
                col1.add(ticks[i].order.security_code);
                col2.add(ticks[i].order.channel_no);
                col3.add(ticks[i].order.appl_seq_num);
                col4.add(convertTime(ticks[i].order.order_time));
                col5.add(ticks[i].order.order_price);
                col6.add(ticks[i].order.order_volume);
                col7.add(ticks[i].order.side);
                col8.add(ticks[i].order.order_type);
                col9.add(ticks[i].order.md_stream_id);
                col10.add(ticks[i].order.orig_order_no);
                col11.add(ticks[i].order.biz_index);
                col12.add(ticks[i].order.variety_category);
                if (receivedTimeFlag_) {
                    col13.add(ticks[i].reachTime);
                }
                reachTimeVec.push_back(ticks[i].reachTime);
                if(dailyIndexFlag_){
                    int dailyIndex = INT_MIN;
                    if(!getDailyIndex(dailyIndex, dailyIndex_, sizeof(dailyIndex_), ticks[i].order.market_type, datatype, ticks[i].order.channel_no, convertTime(ticks[i].order.order_time))){
                        LOG_ERR("[PluginAmdQuote]: getDailyIndex failed. ");
                        return;
                    }
                    col14.add(dailyIndex);
                }
            } else if(varietyCategory == 2) {
                fcol0.add(ticks[i].order.market_type);
                fcol1.add(ticks[i].order.security_code);
                fcol2.add(ticks[i].order.channel_no);
                fcol3.add(ticks[i].order.appl_seq_num);
                fcol4.add(convertTime(ticks[i].order.order_time));
                fcol5.add(ticks[i].order.order_price);
                fcol6.add(ticks[i].order.order_volume);
                fcol7.add(ticks[i].order.side);
                fcol8.add(ticks[i].order.order_type);
                fcol9.add(ticks[i].order.md_stream_id);
                fcol10.add(ticks[i].order.orig_order_no);
                fcol11.add(ticks[i].order.biz_index);
                fcol12.add(ticks[i].order.variety_category);
                if (receivedTimeFlag_) {
                    fcol13.add(ticks[i].reachTime);
                }
                fundReachTimeVec.push_back(ticks[i].reachTime);
                if(dailyIndexFlag_){
                    int dailyIndex = INT_MIN;
                    if(!getDailyIndex(dailyIndex, dailyIndex_, sizeof(dailyIndex_), ticks[i].order.market_type, datatype, ticks[i].order.channel_no, convertTime(ticks[i].order.order_time))){
                        LOG_ERR("[PluginAmdQuote]: getDailyIndex failed. ");
                        return;
                    }
                    fcol14.add(dailyIndex);
                }
            }
        }

        if(orderFlag_) {
            vector<ConstantSP> cols;
            cols.push_back(col0.createVector(DT_INT));
            cols.push_back(col1.createVector(DT_STRING));
            cols.push_back(col2.createVector(DT_INT));
            cols.push_back(col3.createVector(DT_LONG));
            cols.push_back(col4.createVector(DT_TIMESTAMP));
            cols.push_back(col5.createVector(DT_LONG));
            cols.push_back(col6.createVector(DT_LONG));
            cols.push_back(col7.createVector(DT_CHAR));
            cols.push_back(col8.createVector(DT_CHAR));
            cols.push_back(col9.createVector(DT_STRING));
            cols.push_back(col10.createVector(DT_LONG));
            cols.push_back(col11.createVector(DT_LONG));
            cols.push_back(col12.createVector(DT_CHAR));

            vector<string> colNames = orderTableMeta_.colNames_;
            if (receivedTimeFlag_) {     // to avoid flag corner case
                cols.push_back(col13.createVector(DT_NANOTIMESTAMP));
                colNames.push_back("receivedTime");
            }
            if(dailyIndexFlag_){
                cols.push_back(col14.createVector(DT_INT));
                colNames.push_back("dailyIndex");
            }
            if(outputElapsedFlag_) {
                colNames.push_back("perPenetrationTime");
                long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                for(int i = 0; i < col0.size(); ++i) {
                    col15.add(time - reachTimeVec[i]);
                }
                cols.push_back(col15.createVector(DT_NANOTIME));
            }

            TableSP data = Util::createTable(colNames, cols);

            vector<ConstantSP> args = {data};
            try{
                if(!transform.isNull())
                    data = transform->call(session_->getHeap().get(), args);
            }catch(exception &e){
                throw RuntimeException("call transform error " + string(e.what()));
            }

            if(insertedTable.isNull())
                throw RuntimeException("insertedTable is null");
            if(insertedTable ->columns() != data->columns()) {
                throw RuntimeException("The number of stock columns of the table to insert must be the same as that of the original table");
            }

            INDEX rows;
            string errMsg;
            vector<ConstantSP> colData(data->columns());
            for(INDEX i = 0; i < data->columns(); ++i) {
                colData[i] = data->getColumn(i);
            }
            insertedTable->append(colData, rows, errMsg);
            if(errMsg != "") {
                LOG_ERR("[PluginAmdQuote]: OnMDTickOrder append failed, ", errMsg);
                return;
            }

            if (latencyFlag_) {
                long long diff = Util::toLocalNanoTimestamp(Util::getNanoEpochTime() - startTime);
                latencyLog(2, startTime, cnt, diff);
            }
        }
        if (fundOrderFlag_) {
            vector<ConstantSP> fcols;
            fcols.push_back(fcol0.createVector(DT_INT));
            fcols.push_back(fcol1.createVector(DT_STRING));
            fcols.push_back(fcol2.createVector(DT_INT));
            fcols.push_back(fcol3.createVector(DT_LONG));
            fcols.push_back(fcol4.createVector(DT_TIMESTAMP));
            fcols.push_back(fcol5.createVector(DT_LONG));
            fcols.push_back(fcol6.createVector(DT_LONG));
            fcols.push_back(fcol7.createVector(DT_CHAR));
            fcols.push_back(fcol8.createVector(DT_CHAR));
            fcols.push_back(fcol9.createVector(DT_STRING));
            fcols.push_back(fcol10.createVector(DT_LONG));
            fcols.push_back(fcol11.createVector(DT_LONG));
            fcols.push_back(fcol12.createVector(DT_CHAR));
            
            vector<string> fcolNames = orderTableMeta_.colNames_;
            if (receivedTimeFlag_) {
                fcols.push_back(fcol13.createVector(DT_NANOTIMESTAMP));
                fcolNames.push_back("receivedTime");
            }
            if(dailyIndexFlag_) {
                fcols.push_back(fcol14.createVector(DT_INT));
                fcolNames.push_back("dailyIndex");
            }
            if(outputElapsedFlag_) {
                fcolNames.push_back("perPenetrationTime");
                long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                for(int i = 0; i < fcol0.size(); ++i) {
                    fcol15.add(time - fundReachTimeVec[i]);
                }
                fcols.push_back(fcol15.createVector(DT_NANOTIME));
            }

            TableSP data = Util::createTable(fcolNames, fcols);

            vector<ConstantSP> args = {data};
            try{
                if(!fundTransform.isNull())
                    data = fundTransform->call(session_->getHeap().get(), args);
            }catch(exception &e){
                throw RuntimeException("call transform error " + string(e.what()));
            }

            if(fundInsertedTable.isNull())
                throw RuntimeException("insertedTable is null");
            if(fundInsertedTable ->columns() != data->columns())
                throw RuntimeException("The number of fund columns of the table to insert must be the same as that of the original table");

            INDEX rows;
            string errMsg;
            vector<ConstantSP> colData(data->columns());
            for(INDEX i = 0; i < data->columns(); ++i) {
                colData[i] = data->getColumn(i);
            }
            fundInsertedTable->append(colData, rows, errMsg);
            if(errMsg != "") {
                LOG_ERR("[PluginAmdQuote]: OnMDTickOrder append failed, ", errMsg);
                return;
            }

            if (latencyFlag_) {
                long long diff = Util::toLocalNanoTimestamp(Util::getNanoEpochTime() - startTime);
                latencyLog(2, startTime, cnt, diff);
            }
        }

    }
    catch(exception &e){
        LOG_ERR("[PluginAmdQuote]: OnMDTickOrder failed, ", e.what());
    }
}

// 接受并处理逐笔委托数据（债券）
void AMDSpiImp::OnMDBondTickOrderHelper(timeMDBondTickOrder* ticks, uint32_t cnt) {
    try{
        if(cnt == 0)return;
        long long startTime = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
        // std::lock_guard<std::mutex> amdLock_(amdMutex_);

        // 基金和股票数据插入到不同的表
        TableSP insertedTable;
        FunctionDefSP transform;
        uint8_t varietyCategory = ticks[0].bondOrder.variety_category;
        AMDDataType datatype;
        if (varietyCategory == 3) { // 债券
            insertedTable = bondOrderData_;
            transform = bondOrderTransform_;
            datatype = AMD_BOND_ORDER;
            if(!bondOrderFlag_) {
                return;
            }
        } else {
            return ;
        }

        int market_type = ticks[0].bondOrder.market_type;
        AMDTableType tableTyeTotal = getAmdTableType(datatype, market_type);
        if(tableTyeTotal == AMD_ERROR_TABLE_TYPE){
            LOG_ERR("[PluginAmdQuote]: error amd table type AMD_ERROR_TABLE_TYPE");
            return;
        }

        vector<long long> reachTimeVec;
        if(outputElapsedFlag_) {
            reachTimeVec.reserve(cnt);
        }

        DdbVector<int> col0(0, cnt);
        DdbVector<string> col1(0, cnt);
        DdbVector<int> col2(0, cnt);
        DdbVector<long long> col3(0, cnt);
        DdbVector<long long> col4(0, cnt);
        DdbVector<long long> col5(0, cnt);
        DdbVector<long long> col6(0, cnt);
        DdbVector<char> col7(0, cnt);
        DdbVector<char> col8(0, cnt);
        DdbVector<string> col9(0, cnt);
        DdbVector<long long> col10(0, cnt);
        DdbVector<long long> col11(0, cnt);
        DdbVector<char> col12(0, cnt);
        DdbVector<long long> col13(0, cnt);
        DdbVector<int> col14(0, cnt);
        DdbVector<long long> col15(0, cnt);

        for (uint32_t i = 0; i < cnt; ++i) {
            col0.add(ticks[i].bondOrder.market_type);
            col1.add(ticks[i].bondOrder.security_code);
            col2.add(ticks[i].bondOrder.channel_no);
            col3.add(ticks[i].bondOrder.appl_seq_num);
            col4.add(convertTime(ticks[i].bondOrder.order_time));
            col5.add(ticks[i].bondOrder.order_price);
            col6.add(ticks[i].bondOrder.order_volume);
            col7.add(ticks[i].bondOrder.side);
            col8.add(ticks[i].bondOrder.order_type);
            col9.add(ticks[i].bondOrder.md_stream_id);
            col10.add(ticks[i].bondOrder.orig_order_no);
            col11.add(LONG_LONG_MIN);
            col12.add(ticks[i].bondOrder.variety_category);
            if (receivedTimeFlag_) {
                col13.add(ticks[i].reachTime);
            }
            reachTimeVec.push_back(ticks[i].reachTime);
            if(dailyIndexFlag_){
                int dailyIndex = INT_MIN;
                if(!getDailyIndex(dailyIndex, dailyIndex_, sizeof(dailyIndex_), ticks[i].bondOrder.market_type, datatype, ticks[i].bondOrder.channel_no, convertTime(ticks[i].bondOrder.order_time))){
                    LOG_ERR("[PluginAmdQuote]: getDailyIndex failed. ");
                    return;
                }
                col14.add(dailyIndex);
            }
        }

        vector<ConstantSP> cols;
        cols.push_back(col0.createVector(DT_INT));
        cols.push_back(col1.createVector(DT_STRING));
        cols.push_back(col2.createVector(DT_INT));
        cols.push_back(col3.createVector(DT_LONG));
        cols.push_back(col4.createVector(DT_TIMESTAMP));
        cols.push_back(col5.createVector(DT_LONG));
        cols.push_back(col6.createVector(DT_LONG));
        cols.push_back(col7.createVector(DT_CHAR));
        cols.push_back(col8.createVector(DT_CHAR));
        cols.push_back(col9.createVector(DT_STRING));
        cols.push_back(col10.createVector(DT_LONG));
        cols.push_back(col11.createVector(DT_LONG));
        cols.push_back(col12.createVector(DT_CHAR));

        if (receivedTimeFlag_) {
            cols.push_back(col13.createVector(DT_NANOTIMESTAMP));
        }
        if(dailyIndexFlag_)
            cols.push_back(col14.createVector(DT_INT));
        if(outputElapsedFlag_) {
            long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
            for(int i = 0; i < col0.size(); ++i) {
                col15.add(time - reachTimeVec[i]);
            }
            cols.push_back(col15.createVector(DT_NANOTIME));
        }

        vector<string> colNames = orderTableMeta_.colNames_;
        if(receivedTimeFlag_)
            colNames.push_back("receivedTime");
        if(dailyIndexFlag_)
            colNames.push_back("dailyIndex");
        if(outputElapsedFlag_)
            colNames.push_back("perPenetrationTime");

        TableSP data = Util::createTable(colNames, cols);

        vector<ConstantSP> args = {data};
        try{
            if(!transform.isNull())
                data = transform->call(session_->getHeap().get(), args);
        }catch(exception &e){
            throw RuntimeException("call transform error " + string(e.what()));
        }

        if(insertedTable.isNull())
            throw RuntimeException("insertedTable is null");
        if(insertedTable ->columns() != data->columns())
            throw RuntimeException("The number of columns of the table to insert must be the same as that of the original table");

        INDEX rows;
        string errMsg;
        vector<ConstantSP> colData(data->columns());
        for(INDEX i = 0; i < data->columns(); ++i) {
            colData[i] = data->getColumn(i);
        }
        insertedTable->append(colData, rows, errMsg);
        if(errMsg != "") {
            LOG_ERR("[PluginAmdQuote]: OnMDBondTickOrder append failed, ", errMsg);
            return;
        }

        if (latencyFlag_) { 
            long long diff = Util::toLocalNanoTimestamp(Util::getNanoEpochTime() - startTime);
            latencyLog(5, startTime, cnt, diff);
        }
    }
    catch(exception &e){
        LOG_ERR("[PluginAmdQuote]: OnMDBondTickOrder failed, ", e.what());
    }
}

// 接受并处理逐笔成交数据（包含基金和股票数据）
void AMDSpiImp::OnMDTickExecutionHelper(timeMDTickExecution* tick, uint32_t cnt)  {
    try{
        if(cnt == 0) return;
        long long startTime = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());

        TableSP insertedTable = executionData_;
        FunctionDefSP transform = executionTransform_;
        TableSP fundInsertedTable = fundExecutionData_;
        FunctionDefSP fundTransform = fundExecutionTransform_;

        uint8_t varietyCategory;
        AMDDataType datatype;
        int market_type;

        vector<long long> reachTimeVec;
        if(outputElapsedFlag_) {
            reachTimeVec.reserve(cnt);
        }
        vector<long long> fundReachTimeVec;
        if(outputElapsedFlag_) {
            fundReachTimeVec.reserve(cnt);
        }

        DdbVector<int> col0(0, cnt);
        DdbVector<string> col1(0, cnt);
        DdbVector<long long> col2(0, cnt);
        DdbVector<int> col3(0, cnt);
        DdbVector<long long> col4(0, cnt);
        DdbVector<long long> col5(0, cnt);
        DdbVector<long long> col6(0, cnt);
        DdbVector<long long> col7(0, cnt);
        DdbVector<long long> col8(0, cnt);
        DdbVector<long long> col9(0, cnt);
        DdbVector<char> col10(0, cnt);
        DdbVector<char> col11(0, cnt);
        DdbVector<string> col12(0, cnt);
        DdbVector<long long> col13(0, cnt);
        DdbVector<char> col14(0, cnt);
        DdbVector<long long> col15(0, cnt);
        DdbVector<int> col16(0, cnt);
        DdbVector<long long> col17(0, cnt);

        DdbVector<int> fcol0(0, cnt);
        DdbVector<string> fcol1(0, cnt);
        DdbVector<long long> fcol2(0, cnt);
        DdbVector<int> fcol3(0, cnt);
        DdbVector<long long> fcol4(0, cnt);
        DdbVector<long long> fcol5(0, cnt);
        DdbVector<long long> fcol6(0, cnt);
        DdbVector<long long> fcol7(0, cnt);
        DdbVector<long long> fcol8(0, cnt);
        DdbVector<long long> fcol9(0, cnt);
        DdbVector<char> fcol10(0, cnt);
        DdbVector<char> fcol11(0, cnt);
        DdbVector<string> fcol12(0, cnt);
        DdbVector<long long> fcol13(0, cnt);
        DdbVector<char> fcol14(0, cnt);
        DdbVector<long long> fcol15(0, cnt);
        DdbVector<int> fcol16(0, cnt);
        DdbVector<long long> fcol17(0, cnt);

        for (uint32_t i = 0; i < cnt; ++i) {
            varietyCategory = tick[i].execution.variety_category;
            if (varietyCategory == 1) {
                datatype = AMD_EXECUTION;
                if(!executionFlag_) {
                    continue;
                }
            } else if(varietyCategory == 2){
                datatype = AMD_FUND_EXECUTION;
                if(!fundExecutionFlag_) {
                    continue;
                }
            } else {
                continue;
            }
            market_type = tick[i].execution.market_type;
            AMDTableType tableTyeTotal = getAmdTableType(datatype, market_type);
            
            if(tableTyeTotal == AMD_ERROR_TABLE_TYPE){
                LOG_ERR("[PluginAmdQuote]: error amd table type AMD_ERROR_TABLE_TYPE");
                return;
            }

            if(varietyCategory == 1) {
                col0.add(tick[i].execution.market_type);
                col1.add(tick[i].execution.security_code);
                col2.add(convertTime(tick[i].execution.exec_time));
                col3.add(tick[i].execution.channel_no);
                col4.add(tick[i].execution.appl_seq_num);
                col5.add(tick[i].execution.exec_price);
                col6.add(tick[i].execution.exec_volume);
                col7.add(tick[i].execution.value_trade);
                col8.add(tick[i].execution.bid_appl_seq_num);
                col9.add(tick[i].execution.offer_appl_seq_num);
                col10.add(tick[i].execution.side);
                col11.add(tick[i].execution.exec_type);
                col12.add(tick[i].execution.md_stream_id);
                col13.add(tick[i].execution.biz_index);
                col14.add(tick[i].execution.variety_category);

                if (receivedTimeFlag_) {
                    col15.add(tick[i].reachTime);
                }
                reachTimeVec.push_back(tick[i].reachTime);
                if(dailyIndexFlag_){
                    int dailyIndex = INT_MIN;
                    if(!getDailyIndex(dailyIndex, dailyIndex_, sizeof(dailyIndex_), tick[i].execution.market_type, datatype, tick[i].execution.channel_no, convertTime(tick[i].execution.exec_time))){
                        LOG_ERR("[PluginAmdQuote]: getDailyIndex failed. ");
                        return;
                    }
                    col16.add(dailyIndex);
                }
            } else if(varietyCategory == 2) {
                fcol0.add(tick[i].execution.market_type);
                fcol1.add(tick[i].execution.security_code);
                fcol2.add(convertTime(tick[i].execution.exec_time));
                fcol3.add(tick[i].execution.channel_no);
                fcol4.add(tick[i].execution.appl_seq_num);
                fcol5.add(tick[i].execution.exec_price);
                fcol6.add(tick[i].execution.exec_volume);
                fcol7.add(tick[i].execution.value_trade);
                fcol8.add(tick[i].execution.bid_appl_seq_num);
                fcol9.add(tick[i].execution.offer_appl_seq_num);
                fcol10.add(tick[i].execution.side);
                fcol11.add(tick[i].execution.exec_type);
                fcol12.add(tick[i].execution.md_stream_id);
                fcol13.add(tick[i].execution.biz_index);
                fcol14.add(tick[i].execution.variety_category);

                if (receivedTimeFlag_) {
                    fcol15.add(tick[i].reachTime);
                }
                fundReachTimeVec.push_back(tick[i].reachTime);
                if(dailyIndexFlag_){
                    int dailyIndex = INT_MIN;
                    if(!getDailyIndex(dailyIndex, dailyIndex_, sizeof(dailyIndex_), tick[i].execution.market_type, datatype, tick[i].execution.channel_no, convertTime(tick[i].execution.exec_time))){
                        LOG_ERR("[PluginAmdQuote]: getDailyIndex failed. ");
                        return;
                    }
                    fcol16.add(dailyIndex);
                }
            }
        }

        if(executionFlag_) {
            vector<ConstantSP> cols;
            cols.push_back(col0.createVector(DT_INT));
            cols.push_back(col1.createVector(DT_SYMBOL));
            cols.push_back(col2.createVector(DT_TIMESTAMP));
            cols.push_back(col3.createVector(DT_INT));
            cols.push_back(col4.createVector(DT_LONG));
            cols.push_back(col5.createVector(DT_LONG));
            cols.push_back(col6.createVector(DT_LONG));
            cols.push_back(col7.createVector(DT_LONG));
            cols.push_back(col8.createVector(DT_LONG));
            cols.push_back(col9.createVector(DT_LONG));
            cols.push_back(col10.createVector(DT_CHAR));
            cols.push_back(col11.createVector(DT_CHAR));
            cols.push_back(col12.createVector(DT_STRING));
            cols.push_back(col13.createVector(DT_LONG));
            cols.push_back(col14.createVector(DT_CHAR));

            vector<string> colNames = executionTableMeta_.colNames_;
            if (receivedTimeFlag_) {
                cols.push_back(col15.createVector(DT_NANOTIMESTAMP));
                colNames.push_back("receivedTime");
            }
            if(dailyIndexFlag_) {
                cols.push_back(col16.createVector(DT_INT)); 
                colNames.push_back("dailyIndex");
            }
            if(outputElapsedFlag_) {
                colNames.push_back("perPenetrationTime");
                long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                for(int i = 0; i < col0.size(); ++i) {
                    col17.add(time - reachTimeVec[i]);
                }
                cols.push_back(col17.createVector(DT_NANOTIME));
            }

            TableSP data = Util::createTable(colNames, cols);

            vector<ConstantSP> args = {data};
            try{
                if(!transform.isNull())
                    data = transform->call(session_->getHeap().get(), args);
            }catch(exception &e){
                throw RuntimeException("call transform error " + string(e.what()));
            }

            if(insertedTable.isNull())
                throw RuntimeException("insertedTable is null");
            if(insertedTable ->columns() != data->columns())
                throw RuntimeException("The number of columns of the table to insert must be the same as that of the original table");

            INDEX rows;
            string errMsg;
            vector<ConstantSP> colData(data->columns());
            for(INDEX i = 0; i < data->columns(); ++i) {
                colData[i] = data->getColumn(i);
            }
            insertedTable->append(colData, rows, errMsg);
            if(errMsg != "") {
                LOG_ERR("[PluginAmdQuote]: OnMDTickExecution append failed, ", errMsg);
                return;
            }
            
            if (latencyFlag_) { 
                long long latency = Util::toLocalNanoTimestamp(Util::getNanoEpochTime() - startTime);
                latencyLog(1, startTime, cnt, latency);
            }
        }

        if(fundExecutionFlag_) {
            vector<ConstantSP> fcols;
            fcols.push_back(fcol0.createVector(DT_INT));
            fcols.push_back(fcol1.createVector(DT_SYMBOL));
            fcols.push_back(fcol2.createVector(DT_TIMESTAMP));
            fcols.push_back(fcol3.createVector(DT_INT));
            fcols.push_back(fcol4.createVector(DT_LONG));
            fcols.push_back(fcol5.createVector(DT_LONG));
            fcols.push_back(fcol6.createVector(DT_LONG));
            fcols.push_back(fcol7.createVector(DT_LONG));
            fcols.push_back(fcol8.createVector(DT_LONG));
            fcols.push_back(fcol9.createVector(DT_LONG));
            fcols.push_back(fcol10.createVector(DT_CHAR));
            fcols.push_back(fcol11.createVector(DT_CHAR));
            fcols.push_back(fcol12.createVector(DT_STRING));
            fcols.push_back(fcol13.createVector(DT_LONG));
            fcols.push_back(fcol14.createVector(DT_CHAR));

            vector<string> fcolNames = executionTableMeta_.colNames_;
            if (receivedTimeFlag_) {
                fcols.push_back(fcol15.createVector(DT_NANOTIMESTAMP));
                fcolNames.push_back("receivedTime");
            }
            if(dailyIndexFlag_) {
                fcols.push_back(fcol16.createVector(DT_INT)); 
                fcolNames.push_back("dailyIndex");
            }
            if(outputElapsedFlag_) {
                fcolNames.push_back("perPenetrationTime");
                long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                for(int i = 0; i < fcol0.size(); ++i) {
                    fcol17.add(time - fundReachTimeVec[i]);
                }
                fcols.push_back(fcol17.createVector(DT_NANOTIME));
            }

            TableSP data = Util::createTable(fcolNames, fcols);

            vector<ConstantSP> args = {data};
            try{
                if(!fundTransform.isNull())
                    data = fundTransform->call(session_->getHeap().get(), args);
            }catch(exception &e){
                throw RuntimeException("call transform error " + string(e.what()));
            }

            if(fundInsertedTable.isNull())
                throw RuntimeException("insertedTable is null");
            if(fundInsertedTable ->columns() != data->columns())
                throw RuntimeException("The number of columns of the table to insert must be the same as that of the original table");

            INDEX rows;
            string errMsg;
            vector<ConstantSP> colData(data->columns());
            for(INDEX i = 0; i < data->columns(); ++i) {
                colData[i] = data->getColumn(i);
            }
            fundInsertedTable->append(colData, rows, errMsg);
            if(errMsg != "") {
                LOG_ERR("[PluginAmdQuote]: OnMDTickExecution append failed, ", errMsg);
                return;
            }
            
            if (latencyFlag_) {
                long long latency = Util::toLocalNanoTimestamp(Util::getNanoEpochTime() - startTime);
                latencyLog(1, startTime, cnt, latency);
            }
        }
    }
    catch(exception& e){
        LOG_ERR("[PluginAmdQuote]: OnMDTickExecution failed, ", e.what());
    }
}

// 接受并处理逐笔成交数据（债券）
void AMDSpiImp::OnMDBondTickExecutionHelper(timeMDBondTickExecution* tick, uint32_t cnt)  {
    try{
        if(cnt == 0)return;
        long long startTime = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
        // std::lock_guard<std::mutex> amdLock_(amdMutex_);

        TableSP insertedTable;
        FunctionDefSP transform;
        char varietyCategory = tick[0].bondExecution.variety_category;
        AMDDataType datatype;
        if (varietyCategory == 3) { // 债券逐笔成交
            insertedTable = bondExecutionData_;
            transform = bondExecutionTransform_;
            datatype = AMD_BOND_EXECUTION;
            if(!bondExecutionFlag_) {
                return;
            }
        } else {
            return ;
        }

        int market_type = tick[0].bondExecution.market_type;
        AMDTableType tableTyeTotal = getAmdTableType(datatype, market_type);
        if(tableTyeTotal == AMD_ERROR_TABLE_TYPE){
            LOG_ERR("[PluginAmdQuote]: error amd table type AMD_ERROR_TABLE_TYPE");
            return;
        }

        vector<long long> reachTimeVec;
        if(outputElapsedFlag_) {
            reachTimeVec.reserve(cnt);
        }

        DdbVector<int> col0(0, cnt);
        DdbVector<string> col1(0, cnt);
        DdbVector<long long> col2(0, cnt);
        DdbVector<int> col3(0, cnt);
        DdbVector<long long> col4(0, cnt);
        DdbVector<long long> col5(0, cnt);
        DdbVector<long long> col6(0, cnt);
        DdbVector<long long> col7(0, cnt);
        DdbVector<long long> col8(0, cnt);
        DdbVector<long long> col9(0, cnt);
        DdbVector<char> col10(0, cnt);
        DdbVector<char> col11(0, cnt);
        DdbVector<string> col12(0, cnt);
        DdbVector<long long> col13(0, cnt);
        DdbVector<char> col14(0, cnt);
        DdbVector<long long> col15(0, cnt);
        DdbVector<int> col16(0, cnt);
        DdbVector<long long> col17(0, cnt);

        for (uint32_t i = 0; i < cnt; ++i) {
            col0.add(tick[i].bondExecution.market_type);
            col1.add(tick[i].bondExecution.security_code);
            col2.add(convertTime(tick[i].bondExecution.exec_time));
            col3.add(tick[i].bondExecution.channel_no);
            col4.add(tick[i].bondExecution.appl_seq_num);
            col5.add(tick[i].bondExecution.exec_price);
            col6.add(tick[i].bondExecution.exec_volume);
            col7.add(tick[i].bondExecution.value_trade);
            col8.add(tick[i].bondExecution.bid_appl_seq_num);
            col9.add(tick[i].bondExecution.offer_appl_seq_num);
            col10.add(tick[i].bondExecution.side);
            col11.add(tick[i].bondExecution.exec_type);
            col12.add(tick[i].bondExecution.md_stream_id);
            // col13.add(tick[i].biz_index);
            col13.add(LONG_LONG_MIN);
            col14.add(tick[i].bondExecution.variety_category);

            if (receivedTimeFlag_) {
                col15.add(tick[i].reachTime);
            }
            reachTimeVec.push_back(tick[i].reachTime);
            if(dailyIndexFlag_){
                int dailyIndex = INT_MIN;
                if(!getDailyIndex(dailyIndex, dailyIndex_, sizeof(dailyIndex_), tick[i].bondExecution.market_type, datatype, tick[i].bondExecution.channel_no, convertTime(tick[i].bondExecution.exec_time))){
                    LOG_ERR("[PluginAmdQuote]: getDailyIndex failed. ");
                    return;
                }
                col16.add(dailyIndex);
            }
        }

        vector<ConstantSP> cols;
        cols.push_back(col0.createVector(DT_INT));
        cols.push_back(col1.createVector(DT_SYMBOL));
        cols.push_back(col2.createVector(DT_TIMESTAMP));
        cols.push_back(col3.createVector(DT_INT));
        cols.push_back(col4.createVector(DT_LONG));
        cols.push_back(col5.createVector(DT_LONG));
        cols.push_back(col6.createVector(DT_LONG));
        cols.push_back(col7.createVector(DT_LONG));
        cols.push_back(col8.createVector(DT_LONG));
        cols.push_back(col9.createVector(DT_LONG));
        cols.push_back(col10.createVector(DT_CHAR));
        cols.push_back(col11.createVector(DT_CHAR));
        cols.push_back(col12.createVector(DT_STRING));
        cols.push_back(col13.createVector(DT_LONG));
        cols.push_back(col14.createVector(DT_CHAR));
        if (receivedTimeFlag_) {
            cols.push_back(col15.createVector(DT_NANOTIMESTAMP));
        }
        if(dailyIndexFlag_)
            cols.push_back(col16.createVector(DT_INT)); 
        if(outputElapsedFlag_) {
            long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
            for(int i = 0; i < col0.size(); ++i) {
                col17.add(time - reachTimeVec[i]);
            }
            cols.push_back(col17.createVector(DT_NANOTIME));
        }
    

        vector<string> colNames = executionTableMeta_.colNames_;
        if(receivedTimeFlag_)
            colNames.push_back("receivedTime");
        if(dailyIndexFlag_)
            colNames.push_back("dailyIndex");
        if(outputElapsedFlag_)
            colNames.push_back("perPenetrationTime");

        TableSP data = Util::createTable(colNames, cols);

        vector<ConstantSP> args = {data};
        try{
            if(!transform.isNull())
                data = transform->call(session_->getHeap().get(), args);
        }catch(exception &e){
            throw RuntimeException("call transform error " + string(e.what()));
        }

        if(insertedTable.isNull())
            throw RuntimeException("insertedTable is null");
        if(insertedTable ->columns() != data->columns())
            throw RuntimeException("The number of columns of the table to insert must be the same as that of the original table");

        INDEX rows;
        string errMsg;
        vector<ConstantSP> colData(data->columns());
        for(INDEX i = 0; i < data->columns(); ++i) {
            colData[i] = data->getColumn(i);
        }
        insertedTable->append(colData, rows, errMsg);
        if(errMsg != "") {
            LOG_ERR("[PluginAmdQuote]: OnMDBondTickExecution append failed, ", errMsg);
            return;
        }
        
        if (latencyFlag_) {
            long long latency = Util::toLocalNanoTimestamp(Util::getNanoEpochTime() - startTime);
            latencyLog(4, startTime, cnt, latency);
        }
    }
    catch(exception& e){
        LOG_ERR("[PluginAmdQuote]: OnMDBondTickExecution failed, ", e.what());
    }
}

// 接受并处理指数快照数据
void AMDSpiImp::OnMDIndexSnapshotHelper(timeMDIndexSnapshot* index, uint32_t cnt)  {
    try{
        if(cnt == 0)return;

        if(!indexFlag_) {
            return;
        }
        AMDDataType datatype = AMD_INDEX;
        int market_type = index[0].indexSnapshot.market_type;
        AMDTableType tableTyeTotal = getAmdTableType(datatype, market_type);
        if(tableTyeTotal == AMD_ERROR_TABLE_TYPE){
            LOG_ERR("[PluginAmdQuote]: error amd table type AMD_ERROR_TABLE_TYPE");
            return;
        }

        vector<long long> reachTimeVec;
        if(outputElapsedFlag_) {
            reachTimeVec.reserve(cnt);
        }

        DdbVector<int> col0(0, cnt);
        DdbVector<string> col1(0, cnt);
        DdbVector<long long> col2(0, cnt);
        DdbVector<string> col3(0, cnt);
        DdbVector<long long> col4(0, cnt);
        DdbVector<long long> col5(0, cnt);
        DdbVector<long long> col6(0, cnt);
        DdbVector<long long> col7(0, cnt);
        DdbVector<long long> col8(0, cnt);
        DdbVector<long long> col9(0, cnt);
        DdbVector<long long> col10(0, cnt);
        DdbVector<long long> col11(0, cnt);
        DdbVector<int> col12(0, cnt);
        DdbVector<string> col13(0, cnt);
        DdbVector<char> col14(0, cnt);
        DdbVector<long long> col15(0, cnt);
        DdbVector<int> col16(0, cnt);
        DdbVector<long long> col17(0, cnt);
        
        for (uint32_t i = 0; i < cnt; i++) {
            col0.add(index[i].indexSnapshot.market_type);
            col1.add(index[i].indexSnapshot.security_code);
            col2.add(convertTime(index[i].indexSnapshot.orig_time));
            col3.add(index[i].indexSnapshot.trading_phase_code);
            col4.add(index[i].indexSnapshot.pre_close_index);
            col5.add(index[i].indexSnapshot.open_index);
            col6.add(index[i].indexSnapshot.high_index);
            col7.add(index[i].indexSnapshot.low_index);
            col8.add(index[i].indexSnapshot.last_index);
            col9.add(index[i].indexSnapshot.close_index);
            col10.add(index[i].indexSnapshot.total_volume_trade);
            col11.add(index[i].indexSnapshot.total_value_trade);
            col12.add(index[i].indexSnapshot.channel_no);
            col13.add(index[i].indexSnapshot.md_stream_id);
            col14.add(index[i].indexSnapshot.variety_category);
            if (receivedTimeFlag_) {
                col15.add(index[i].reachTime);
            }
            if(dailyIndexFlag_){
                int dailyIndex = INT_MIN;
                if(!getDailyIndex(dailyIndex, dailyIndex_, sizeof(dailyIndex_), index[i].indexSnapshot.market_type, datatype, index[i].indexSnapshot.channel_no, convertTime(index[i].indexSnapshot.orig_time))){
                    LOG_ERR("[PluginAmdQuote]: getDailyIndex failed. ");
                    return;
                }
                col16.add(dailyIndex);
            }
            reachTimeVec.push_back(index[i].reachTime);
        }

        vector<ConstantSP> cols;
        cols.push_back(col0.createVector(DT_INT));
        cols.push_back(col1.createVector(DT_SYMBOL));
        cols.push_back(col2.createVector(DT_TIMESTAMP));
        cols.push_back(col3.createVector(DT_STRING));
        cols.push_back(col4.createVector(DT_LONG));
        cols.push_back(col5.createVector(DT_LONG));
        cols.push_back(col6.createVector(DT_LONG));
        cols.push_back(col7.createVector(DT_LONG));
        cols.push_back(col8.createVector(DT_LONG));
        cols.push_back(col9.createVector(DT_LONG));
        cols.push_back(col10.createVector(DT_LONG));
        cols.push_back(col11.createVector(DT_LONG));
        cols.push_back(col12.createVector(DT_INT));
        cols.push_back(col13.createVector(DT_STRING));
        cols.push_back(col14.createVector(DT_CHAR));
        if (receivedTimeFlag_) {
            cols.push_back(col15.createVector(DT_NANOTIMESTAMP));
        }
        if(dailyIndexFlag_)
            cols.push_back(col16.createVector(DT_INT));

        if(outputElapsedFlag_) {
            long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
            for(int i = 0; i < col0.size(); ++i) {
                col17.add(time - reachTimeVec[i]);
            }
            cols.push_back(col17.createVector(DT_NANOTIME));
        }
    
        vector<string> colNames = indexTableMeta_.colNames_;
        if(receivedTimeFlag_)
            colNames.push_back("receivedTime");
        if(dailyIndexFlag_)
            colNames.push_back("dailyIndex");
        if(outputElapsedFlag_)
            colNames.push_back("perPenetrationTime");

        TableSP data = Util::createTable(colNames, cols);

        vector<ConstantSP> args = {data};
        try{
            if(!indexTransform_.isNull())
                data = indexTransform_->call(session_->getHeap().get(), args);
        }catch(exception &e){
            throw RuntimeException("call transform error " + string(e.what()));
        }

        if(indexData_.isNull())
            throw RuntimeException("insertedTable is null");
        if(indexData_ ->columns() != data->columns())
            throw RuntimeException("The number of columns of the table to insert must be the same as that of the original table");
        INDEX rows;
        string errMsg;
        vector<ConstantSP> colData(data->columns());
        for(INDEX i = 0; i < data->columns(); ++i) {
            colData[i] = data->getColumn(i);
        }
        indexData_->append(colData, rows, errMsg);
        if(errMsg != "") {
            LOG_ERR("[PluginAmdQuote]: OnMDBondTickExecution append failed, ", errMsg);
            return;
        }
    }
    catch(exception& e){
        LOG_ERR("[PluginAmdQuote]: OnMDIndexSnapshot failed, ", e.what());
    }
}

// 接受并处理委托队列数据
void AMDSpiImp::OnMDOrderQueueHelper(timeMDOrderQueue* queue, uint32_t cnt) {
    try{
        if(cnt == 0)return;
        if(!orderQueueFlag_) {
            return;
        }
        AMDDataType datatype = AMD_ORDER_QUEUE;
        int market_type = queue[0].orderQueue.market_type;
        AMDTableType tableTyeTotal = getAmdTableType(datatype, market_type);
        if(tableTyeTotal == AMD_ERROR_TABLE_TYPE){
            LOG_ERR("[PluginAmdQuote]: error amd table type AMD_ERROR_TABLE_TYPE");
            return;
        }
        vector<long long> reachTimeVec;
        if(outputElapsedFlag_) {
            reachTimeVec.reserve(cnt);
        }

        DdbVector<int> col0(0, cnt);
        DdbVector<string> col1(0, cnt);
        DdbVector<long long> col2(0, cnt);
        DdbVector<char> col3(0, cnt);
        DdbVector<long long> col4(0, cnt);
        DdbVector<long long> col5(0, cnt);
        DdbVector<int> col6(0, cnt);
        DdbVector<int> col7(0, cnt);

        DdbVector<long long> col8(0, cnt);
        DdbVector<long long> col9(0, cnt);
        DdbVector<long long> col10(0, cnt);
        DdbVector<long long> col11(0, cnt);
        DdbVector<long long> col12(0, cnt);
        DdbVector<long long> col13(0, cnt);
        DdbVector<long long> col14(0, cnt);
        DdbVector<long long> col15(0, cnt);
        DdbVector<long long> col16(0, cnt);
        DdbVector<long long> col17(0, cnt);

        DdbVector<long long> col18(0, cnt);
        DdbVector<long long> col19(0, cnt);
        DdbVector<long long> col20(0, cnt);
        DdbVector<long long> col21(0, cnt);
        DdbVector<long long> col22(0, cnt);
        DdbVector<long long> col23(0, cnt);
        DdbVector<long long> col24(0, cnt);
        DdbVector<long long> col25(0, cnt);
        DdbVector<long long> col26(0, cnt);
        DdbVector<long long> col27(0, cnt);

        DdbVector<long long> col28(0, cnt);
        DdbVector<long long> col29(0, cnt);
        DdbVector<long long> col30(0, cnt);
        DdbVector<long long> col31(0, cnt);
        DdbVector<long long> col32(0, cnt);
        DdbVector<long long> col33(0, cnt);
        DdbVector<long long> col34(0, cnt);
        DdbVector<long long> col35(0, cnt);
        DdbVector<long long> col36(0, cnt);
        DdbVector<long long> col37(0, cnt);

        DdbVector<long long> col38(0, cnt);
        DdbVector<long long> col39(0, cnt);
        DdbVector<long long> col40(0, cnt);
        DdbVector<long long> col41(0, cnt);
        DdbVector<long long> col42(0, cnt);
        DdbVector<long long> col43(0, cnt);
        DdbVector<long long> col44(0, cnt);
        DdbVector<long long> col45(0, cnt);
        DdbVector<long long> col46(0, cnt);
        DdbVector<long long> col47(0, cnt);

        DdbVector<long long> col48(0, cnt);
        DdbVector<long long> col49(0, cnt);
        DdbVector<long long> col50(0, cnt);
        DdbVector<long long> col51(0, cnt);
        DdbVector<long long> col52(0, cnt);
        DdbVector<long long> col53(0, cnt);
        DdbVector<long long> col54(0, cnt);
        DdbVector<long long> col55(0, cnt);
        DdbVector<long long> col56(0, cnt);
        DdbVector<long long> col57(0, cnt);

        DdbVector<int> col58(0, cnt);
        DdbVector<string> col59(0, cnt);
        DdbVector<char> col60(0, cnt);
        DdbVector<long long> col61(0, cnt);
        DdbVector<int> col62(0, cnt);
        DdbVector<long long> col63(0, cnt);

        for (uint32_t i = 0; i < cnt; ++i) {
            col0.add(queue[i].orderQueue.market_type);
            col1.add(queue[i].orderQueue.security_code);
            col2.add(convertTime(queue[i].orderQueue.order_time));
            col3.add(queue[i].orderQueue.side);
            col4.add(queue[i].orderQueue.order_price);
            col5.add(queue[i].orderQueue.order_volume);
            col6.add(queue[i].orderQueue.num_of_orders);
            col7.add(queue[i].orderQueue.items);

            col8.add(queue[i].orderQueue.volume[0]);
            col9.add(queue[i].orderQueue.volume[1]);
            col10.add(queue[i].orderQueue.volume[2]);
            col11.add(queue[i].orderQueue.volume[3]);
            col12.add(queue[i].orderQueue.volume[4]);
            col13.add(queue[i].orderQueue.volume[5]);
            col14.add(queue[i].orderQueue.volume[6]);
            col15.add(queue[i].orderQueue.volume[7]);
            col16.add(queue[i].orderQueue.volume[8]);
            col17.add(queue[i].orderQueue.volume[9]);

            col18.add(queue[i].orderQueue.volume[10]);
            col19.add(queue[i].orderQueue.volume[11]);
            col20.add(queue[i].orderQueue.volume[12]);
            col21.add(queue[i].orderQueue.volume[13]);
            col22.add(queue[i].orderQueue.volume[14]);
            col23.add(queue[i].orderQueue.volume[15]);
            col24.add(queue[i].orderQueue.volume[16]);
            col25.add(queue[i].orderQueue.volume[17]);
            col26.add(queue[i].orderQueue.volume[18]);
            col27.add(queue[i].orderQueue.volume[19]);

            col28.add(queue[i].orderQueue.volume[20]);
            col29.add(queue[i].orderQueue.volume[21]);
            col30.add(queue[i].orderQueue.volume[22]);
            col31.add(queue[i].orderQueue.volume[23]);
            col32.add(queue[i].orderQueue.volume[24]);
            col33.add(queue[i].orderQueue.volume[25]);
            col34.add(queue[i].orderQueue.volume[26]);
            col35.add(queue[i].orderQueue.volume[27]);
            col36.add(queue[i].orderQueue.volume[28]);
            col37.add(queue[i].orderQueue.volume[29]);

            col38.add(queue[i].orderQueue.volume[30]);
            col39.add(queue[i].orderQueue.volume[31]);
            col40.add(queue[i].orderQueue.volume[32]);
            col41.add(queue[i].orderQueue.volume[33]);
            col42.add(queue[i].orderQueue.volume[34]);
            col43.add(queue[i].orderQueue.volume[35]);
            col44.add(queue[i].orderQueue.volume[36]);
            col45.add(queue[i].orderQueue.volume[37]);
            col46.add(queue[i].orderQueue.volume[38]);
            col47.add(queue[i].orderQueue.volume[39]);

            col48.add(queue[i].orderQueue.volume[40]);
            col49.add(queue[i].orderQueue.volume[41]);
            col50.add(queue[i].orderQueue.volume[42]);
            col51.add(queue[i].orderQueue.volume[43]);
            col52.add(queue[i].orderQueue.volume[44]);
            col53.add(queue[i].orderQueue.volume[45]);
            col54.add(queue[i].orderQueue.volume[46]);
            col55.add(queue[i].orderQueue.volume[47]);
            col56.add(queue[i].orderQueue.volume[48]);
            col57.add(queue[i].orderQueue.volume[49]);

            col58.add(queue[i].orderQueue.channel_no);
            col59.add(queue[i].orderQueue.md_stream_id);
            col60.add(queue[i].orderQueue.variety_category);
            if (receivedTimeFlag_) {
                col61.add(queue[i].reachTime);
            }
            reachTimeVec.push_back(queue[i].reachTime);
            if(dailyIndexFlag_){
                int dailyIndex = INT_MIN;
                if(!getDailyIndex(dailyIndex, dailyIndex_, sizeof(dailyIndex_), queue[i].orderQueue.market_type, datatype, queue[i].orderQueue.channel_no, convertTime(queue[i].orderQueue.order_time))){
                    LOG_ERR("[PluginAmdQuote]: getDailyIndex failed. ");
                    return;
                }
                col62.add(dailyIndex);
            }
        }

        vector<ConstantSP> cols;
        cols.push_back(col0.createVector(DT_INT));
        cols.push_back(col1.createVector(DT_SYMBOL));
        cols.push_back(col2.createVector(DT_TIMESTAMP));
        cols.push_back(col3.createVector(DT_CHAR));
        cols.push_back(col4.createVector(DT_LONG));
        cols.push_back(col5.createVector(DT_LONG));
        cols.push_back(col6.createVector(DT_INT));
        cols.push_back(col7.createVector(DT_INT));

        cols.push_back(col8.createVector(DT_LONG));
        cols.push_back(col9.createVector(DT_LONG));
        cols.push_back(col10.createVector(DT_LONG));
        cols.push_back(col11.createVector(DT_LONG));
        cols.push_back(col12.createVector(DT_LONG));
        cols.push_back(col13.createVector(DT_LONG));
        cols.push_back(col14.createVector(DT_LONG));
        cols.push_back(col15.createVector(DT_LONG));
        cols.push_back(col16.createVector(DT_LONG));
        cols.push_back(col17.createVector(DT_LONG));

        cols.push_back(col18.createVector(DT_LONG));
        cols.push_back(col19.createVector(DT_LONG));
        cols.push_back(col20.createVector(DT_LONG));
        cols.push_back(col21.createVector(DT_LONG));
        cols.push_back(col22.createVector(DT_LONG));
        cols.push_back(col23.createVector(DT_LONG));
        cols.push_back(col24.createVector(DT_LONG));
        cols.push_back(col25.createVector(DT_LONG));
        cols.push_back(col26.createVector(DT_LONG));
        cols.push_back(col27.createVector(DT_LONG));

        cols.push_back(col28.createVector(DT_LONG));
        cols.push_back(col29.createVector(DT_LONG));
        cols.push_back(col30.createVector(DT_LONG));
        cols.push_back(col31.createVector(DT_LONG));
        cols.push_back(col32.createVector(DT_LONG));
        cols.push_back(col33.createVector(DT_LONG));
        cols.push_back(col34.createVector(DT_LONG));
        cols.push_back(col35.createVector(DT_LONG));
        cols.push_back(col36.createVector(DT_LONG));
        cols.push_back(col37.createVector(DT_LONG));

        cols.push_back(col38.createVector(DT_LONG));
        cols.push_back(col39.createVector(DT_LONG));
        cols.push_back(col40.createVector(DT_LONG));
        cols.push_back(col41.createVector(DT_LONG));
        cols.push_back(col42.createVector(DT_LONG));
        cols.push_back(col43.createVector(DT_LONG));
        cols.push_back(col44.createVector(DT_LONG));
        cols.push_back(col45.createVector(DT_LONG));
        cols.push_back(col46.createVector(DT_LONG));
        cols.push_back(col47.createVector(DT_LONG));

        cols.push_back(col48.createVector(DT_LONG));
        cols.push_back(col49.createVector(DT_LONG));
        cols.push_back(col50.createVector(DT_LONG));
        cols.push_back(col51.createVector(DT_LONG));
        cols.push_back(col52.createVector(DT_LONG));
        cols.push_back(col53.createVector(DT_LONG));
        cols.push_back(col54.createVector(DT_LONG));
        cols.push_back(col55.createVector(DT_LONG));
        cols.push_back(col56.createVector(DT_LONG));
        cols.push_back(col57.createVector(DT_LONG));

        cols.push_back(col58.createVector(DT_INT));
        cols.push_back(col59.createVector(DT_STRING));
        cols.push_back(col60.createVector(DT_CHAR));
        if (receivedTimeFlag_) {
            cols.push_back(col61.createVector(DT_NANOTIMESTAMP));
        }
        if(dailyIndexFlag_)
            cols.push_back(col62.createVector(DT_INT)); 

        if(outputElapsedFlag_) {
            long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
            for(int i = 0; i < col0.size(); ++i) {
                col63.add(time - reachTimeVec[i]);
            }
            cols.push_back(col63.createVector(DT_NANOTIME));
        }
        
        vector<string> colNames = orderQueueMeta_.colNames_;
        if(receivedTimeFlag_)
            colNames.push_back("receivedTime");
        if(dailyIndexFlag_)
            colNames.push_back("dailyIndex");
        if(outputElapsedFlag_)
            colNames.push_back("perPenetrationTime");
        
        TableSP data = Util::createTable(colNames, cols);

        vector<ConstantSP> args = {data};
        try{
            if(!orderQueueTransform_.isNull())
                data = orderQueueTransform_->call(session_->getHeap().get(), args);
        }catch(exception &e){
            throw RuntimeException("call transform error " + string(e.what()));
        }

        if(orderQueueData_.isNull())
            throw RuntimeException("insertedTable is null");
        if(orderQueueData_ ->columns() != data->columns())
            throw RuntimeException("The number of columns of the table to insert must be the same as that of the original table");

        INDEX rows;
        string errMsg;
        vector<ConstantSP> colData(data->columns());
        for(INDEX i = 0; i < data->columns(); ++i) {
            colData[i] = data->getColumn(i);
        }
        orderQueueData_->append(colData, rows, errMsg);
        if(errMsg != "") {
            LOG_ERR("[PluginAmdQuote]: OnMDBondTickExecution append failed, ", errMsg);
            return;
        }
    }
    catch(exception &e){
        LOG_ERR("[PluginAmdQuote]: OnMDOrderQueue failed, ", e.what());
    }
}

void AMDSpiImp::pushSnashotData(amd::ama::MDSnapshot* snapshot, uint32_t cnt, long long time){
    for(uint32_t i = 0; i < cnt; ++i){
        auto data = timeMDSnapshot{time, snapshot[i]};
        snapshotBoundQueue_->blockingPush(data);
    }
}


void AMDSpiImp::pushOrderData(amd::ama::MDTickOrder* ticks, uint32_t cnt, long long time){
    for(uint32_t i = 0; i < cnt; ++i){
        auto data = timeMDTickOrder{time, ticks[i]};
        orderBoundQueue_->blockingPush(data);
        int channel = ticks[i].channel_no;
        /**
            * orderExecution push data logic:
            * if the data's channel is unknown, ignore this data.
            * 
            * the next pushExecutionData/pushBondOrderData/pushBondExecutionData
            * functions has same push logic
            */
        if(orderExecutionFlag_) {
            if (orderExecutionBoundQueue_.find(channel) != orderExecutionBoundQueue_.end()) {
                auto data = MDOrderExecution{true, time, {}};
                data.uni.tickOrder = ticks[i];
                orderExecutionBoundQueue_[channel]->blockingPush(data);
            }
        }
        if(fundOrderExecutionFlag_) {
            if (fundOrderExecutionBoundQueue_.find(channel) != fundOrderExecutionBoundQueue_.end()) {
                auto data = MDOrderExecution{true, time, {}};
                data.uni.tickOrder = ticks[i];
                fundOrderExecutionBoundQueue_[channel]->blockingPush(data);
            }
        }

    }
}

void AMDSpiImp::pushExecutionData(amd::ama::MDTickExecution* ticks, uint32_t cnt, long long time){
    for(uint32_t i = 0; i < cnt; ++i){
        auto data = timeMDTickExecution{time, ticks[i]};
        executionBoundQueue_->blockingPush(data);
        int channel = ticks[i].channel_no;
        if(orderExecutionFlag_) {
            if (orderExecutionBoundQueue_.find(channel) != orderExecutionBoundQueue_.end()) {
                auto data = MDOrderExecution{false, time, {}};
                data.uni.tickExecution = ticks[i];
                orderExecutionBoundQueue_[channel]->blockingPush(data);
            }
        }
        if(fundOrderExecutionFlag_) {
            if (fundOrderExecutionBoundQueue_.find(channel) != fundOrderExecutionBoundQueue_.end()) {
                auto data = MDOrderExecution{false, time, {}};
                data.uni.tickExecution = ticks[i];
                fundOrderExecutionBoundQueue_[channel]->blockingPush(data);
            }
        }
    }
}

void AMDSpiImp::pushIndexData(amd::ama::MDIndexSnapshot* index, uint32_t cnt, long long time){
    for(uint32_t i = 0; i < cnt; ++i){
        auto data = timeMDIndexSnapshot{time, index[i]};
        indexBoundQueue_->blockingPush(data);
    }
}

void AMDSpiImp::pushOrderQueueData(amd::ama::MDOrderQueue* queue, uint32_t cnt, long long time){
    for(uint32_t i = 0; i < cnt; ++i){
        auto data = timeMDOrderQueue{time, queue[i]};
        orderQueueBoundQueue_->blockingPush(data);
    }
}

void AMDSpiImp::pushBondSnapshotData(amd::ama::MDBondSnapshot* snapshots, uint32_t cnt, long long time) {
    for(uint32_t i = 0; i < cnt; ++i){
        auto data = timeMDBondSnapshot{time, snapshots[i]};
        bondSnapshotBoundQueue_->blockingPush(data);
    }
}

void AMDSpiImp::pushBondOrderData(amd::ama::MDBondTickOrder* ticks, uint32_t cnt, long long time){
    for(uint32_t i = 0; i < cnt; ++i){
        auto data = timeMDBondTickOrder{time, ticks[i]};
        bondOrderBoundQueue_->blockingPush(data);
        int channel = ticks[i].channel_no;
        if(bondOrderExecutionFlag_) {
            if (bondOrderExecutionBoundQueue_.find(channel) != bondOrderExecutionBoundQueue_.end()) {
                auto data = MDBondOrderExecution{true, time, {}};
                data.uni.tickOrder = ticks[i];
                bondOrderExecutionBoundQueue_[channel]->blockingPush(data);
            }
        }
    }
}

void AMDSpiImp::pushBondExecutionData(amd::ama::MDBondTickExecution* ticks, uint32_t cnt, long long time){
    for(uint32_t i = 0; i < cnt; ++i){
        auto data = timeMDBondTickExecution{time, ticks[i]};
        bondExecutionBoundQueue_->blockingPush(data);
        int channel = ticks[i].channel_no;
        if(bondOrderExecutionFlag_) {
            if (bondOrderExecutionBoundQueue_.find(channel) != bondOrderExecutionBoundQueue_.end()) {
                auto data = MDBondOrderExecution{false, time, {}};
                data.uni.tickExecution = ticks[i];
                bondOrderExecutionBoundQueue_[channel]->blockingPush(data);
            }
        }
    }
}

// set data
void AMDSpiImp::setOrderExecutionData(string type, std::unordered_map<int, TableSP> orderExecutionData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market) {
    if(type == "orderExecution") {
        orderExecutionData_ = orderExecutionData;
        orderExecutionTransform_ = transform;
        if(market == 101) {
            dailyIndex_[AMD_ORDER_EXECUTION_SH] = dailyStartTimestamp;
        }
        else if(market == 102) {
            dailyIndex_[AMD_ORDER_EXECUTION_SZ] = dailyStartTimestamp;
        }
    } else if(type == "fundOrderExecution") {
        fundOrderExecutionData_ = orderExecutionData;
        fundOrderExecutionTransform_ = transform;
        if(market == 101) {
            dailyIndex_[AMD_FUND_ORDER_EXECUTION_SH] = dailyStartTimestamp;
        }
        else if(market == 102) {
            dailyIndex_[AMD_FUND_ORDER_EXECUTION_SZ] = dailyStartTimestamp;
        }
    } else if(type == "bondOrderExecution") {
        bondOrderExecutionData_ = orderExecutionData;
        bondOrderExecutionTransform_ = transform;
        if(market == 101) {
            dailyIndex_[AMD_BOND_ORDER_EXECUTION_SH] = dailyStartTimestamp;
        }
        else if(market == 102) {
            dailyIndex_[AMD_BOND_ORDER_EXECUTION_SZ] = dailyStartTimestamp;
        }
    } 
}

void AMDSpiImp::setSnapshotData(TableSP snapshotData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market) {
    snapshotData_ = snapshotData;
    snapshotTransform_ = transform;
    snapshotFlag_ = true;
    if(market == 101)
        dailyIndex_[AMD_SNAPSHOT_SH] = dailyStartTimestamp;
    else if(market == 102)
        dailyIndex_[AMD_SNAPSHOT_SZ] = dailyStartTimestamp;
}

void AMDSpiImp::setExecutionData(TableSP executionData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market) {
    executionData_ = executionData;
    executionTransform_ = transform;
    executionFlag_ = true;
    if(market == 101)
        dailyIndex_[AMD_EXECUTION_SH] = dailyStartTimestamp;
    else if(market == 102)
        dailyIndex_[AMD_EXECUTION_SZ] = dailyStartTimestamp;
}

void AMDSpiImp::setOrderData(TableSP orderData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market) {
    orderData_ = orderData;
    orderTransform_ = transform;
    orderFlag_ = true;
    if(market == 101)
        dailyIndex_[AMD_ORDER_SH] = dailyStartTimestamp;
    else if(market == 102)
        dailyIndex_[AMD_ORDER_SZ] = dailyStartTimestamp;
}

void AMDSpiImp::setIndexData(TableSP indexData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market) {
    indexData_ = indexData;
    indexTransform_ = transform;
    indexFlag_ = true;
    if(market == 101)
        dailyIndex_[AMD_INDEX_SH] = dailyStartTimestamp;
    else if(market == 102)
        dailyIndex_[AMD_INDEX_SZ] = dailyStartTimestamp;
}

void AMDSpiImp::setOrderQueueData(TableSP orderQueueData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market) {
    orderQueueData_ = orderQueueData;
    orderQueueTransform_ = transform;
    orderQueueFlag_ = true;
    if(market == 101)
        dailyIndex_[AMD_ORDER_QUEUE_SH] = dailyStartTimestamp;
    else if(market == 102)
        dailyIndex_[AMD_ORDER_QUEUE_SZ] = dailyStartTimestamp;
}

void AMDSpiImp::setFundSnapshotData(TableSP snapshotData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market) {
    fundSnapshotData_ = snapshotData;
    fundSnapshotTransform_ = transform;
    fundSnapshotFlag_ = true;
    if(market == 101)
        dailyIndex_[AMD_FUND_SNAPSHOT_SH] = dailyStartTimestamp;
    else if(market == 102)
        dailyIndex_[AMD_FUND_SNAPSHOT_SZ] = dailyStartTimestamp;
}

void AMDSpiImp::setFundExecutionData(TableSP executionData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market) {
    fundExecutionData_ = executionData;
    fundExecutionTransform_ = transform;
    fundExecutionFlag_ = true;
    if(market == 101)
        dailyIndex_[AMD_FUND_EXECUTION_SH] = dailyStartTimestamp;
    else if(market == 102)
        dailyIndex_[AMD_FUND_EXECUTION_SZ] = dailyStartTimestamp;
}

void AMDSpiImp::setFundOrderData(TableSP orderData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market) {
    fundOrderData_ = orderData;
    fundOrderTransform_ = transform;
    fundOrderFlag_ = true;
    if(market == 101)
        dailyIndex_[AMD_FUND_ORDER_SH] = dailyStartTimestamp;
    else if(market == 102)
        dailyIndex_[AMD_FUND_ORDER_SZ] = dailyStartTimestamp;
}

void AMDSpiImp::setBondSnapshotData(TableSP snapshotData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market) {
    bondSnapshotData_ = snapshotData;
    bondSnapshotTransform_ = transform;
    bondSnapshotFlag_ = true;
    if(market == 101)
        dailyIndex_[AMD_BOND_SNAPSHOT_SH] = dailyStartTimestamp;
    else if(market == 102)
        dailyIndex_[AMD_BOND_SNAPSHOT_SZ] = dailyStartTimestamp;
}

void AMDSpiImp::setBondExecutionData(TableSP executionData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market) {
    bondExecutionData_ = executionData;
    bondExecutionTransform_ = transform;
    bondExecutionFlag_ = true;
    if(market == 101)
        dailyIndex_[AMD_BOND_EXECUTION_SH] = dailyStartTimestamp;
    else if(market == 102)
        dailyIndex_[AMD_BOND_EXECUTION_SZ] = dailyStartTimestamp;
}

void AMDSpiImp::setBondOrderData(TableSP orderData, FunctionDefSP transform, bool receivedTimeFlag, long long dailyStartTimestamp, int market) {
    bondOrderData_ = orderData;
    bondOrderTransform_ = transform;
    bondOrderFlag_ = true;
    if(market == 101)
        dailyIndex_[AMD_BOND_ORDER_SH] = dailyStartTimestamp;
    else if(market == 102)
        dailyIndex_[AMD_BOND_ORDER_SZ] = dailyStartTimestamp;
}

void AMDSpiImp::OnEvent(uint32_t level, uint32_t code, const char* event_msg, uint32_t len)
{
    static unordered_set<string> errorSet;
    string codeString = amd::ama::Tools::GetEventCodeString(code);
    LockGuard<Mutex> amdLock_(&AmdQuote::amdMutex_);
    if(codeString.find("Failed") != string::npos){
        if(errorSet.count(event_msg) == 0){
            errorSet.insert(event_msg);
            LOG_ERR("[PluginAmdQuote]: AMA event: " + codeString);
            LOG_INFO("[PluginAmdQuote] AMA event: ", std::string(event_msg));
        }
    }else{
        codeString.clear();
        LOG_INFO("[PluginAmdQuote] AMA event: ", std::string(event_msg));    
    }

}