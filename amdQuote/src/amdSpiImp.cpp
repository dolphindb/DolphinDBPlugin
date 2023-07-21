
#include "amdSpiImp.h"
#include "CoreConcept.h"
#include "Exceptions.h"
#include "LocklessContainer.h"
#include "Types.h"
#include "amdQuoteImp.h"
#include "amdQuoteType.h"
#include <climits>
#include <iostream>

using dolphindb::Executor;

template <>
void AMDSpiImp::initQueueBuffer(vector<ConstantSP>& buffer, const AmdOrderExecutionTableMeta& meta) {
    buffer = vector<ConstantSP>(meta.colNames_.size());
    for(unsigned int i = 0; i < meta.colNames_.size(); ++i) {
        buffer[i] = Util::createVector(meta.colTypes_[i], 0, BUFFER_SIZE);
        ((VectorSP)buffer[i])->initialize();
    }
    buffer.push_back(Util::createVector(DT_NANOTIME, 0, BUFFER_SIZE));
}

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
                        orderExecutionStopFlag_, "[PLUGIN::AMDQUOTE]:", type
            ));
            orderExecutionThread_[it->first] = new Thread(executor);
            orderExecutionThread_[it->first]->start();
            auto buffer = vector<ConstantSP>();
            initQueueBuffer<AmdOrderExecutionTableMeta>(buffer, orderExecutionTableMeta_);
            orderExecutionBuffer_[it->first] = buffer;
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
                        fundOrderExecutionStopFlag_, "[PLUGIN::AMDQUOTE]:", type
            ));
            fundOrderExecutionThread_[it->first] = new Thread(executor);
            fundOrderExecutionThread_[it->first]->start();
            auto buffer = vector<ConstantSP>();
            initQueueBuffer<AmdOrderExecutionTableMeta>(buffer, orderExecutionTableMeta_);
            fundOrderExecutionBuffer_[it->first] = buffer;
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
                        bondOrderExecutionStopFlag_, "[PLUGIN::AMDQUOTE]:", type
            ));
            bondOrderExecutionThread_[it->first] = new Thread(executor);
            bondOrderExecutionThread_[it->first]->start();
            auto buffer = vector<ConstantSP>();
            initQueueBuffer<AmdOrderExecutionTableMeta>(buffer, orderExecutionTableMeta_);
            bondOrderExecutionBuffer_[it->first] = buffer;
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
            orderExecutionBuffer_.clear();
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
            fundOrderExecutionBuffer_.clear();
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
            bondOrderExecutionBuffer_.clear();
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

            vector<ConstantSP>& buffer = orderExecutionBuffer_[channel];
            for(unsigned int i = 0; i < buffer.size(); ++i) {
                ((Vector*)(buffer[i].get()))->clear();
            }
            vector<long long> reachTimeVector;
            reachTimeVector.reserve(cnt);
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
                int colNum = 0;
                if(datatype == AMD_ORDER_EXECUTION) {
                    if(ticks[i].orderOrExecution) {
                        if(marketType == 101) {
                            string securityCode = ticks[i].uni.tickOrder.security_code + string(".SH");
                            ((Vector*)(buffer[colNum++]).get())->appendString(&securityCode, 1);
                        } else if (marketType == 102) {
                            string securityCode = ticks[i].uni.tickOrder.security_code + string(".SZ");
                            ((Vector*)(buffer[colNum++]).get())->appendString(&securityCode, 1);
                        }

                        int orderDate = convertToDate(ticks[i].uni.tickOrder.order_time);
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&orderDate, 1);
                        int orderTime = convertToTime(ticks[i].uni.tickOrder.order_time);
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&orderTime, 1);
                        string securityIDSource = transMarket(ticks[i].uni.tickOrder.market_type);
                        ((Vector*)(buffer[colNum++]).get())->appendString(&securityIDSource, 1);
                        string securityType("StockType");
                        ((Vector*)(buffer[colNum++]).get())->appendString(&securityType, 1);

                        //  make sure dailyIndexFlagTotal always true
                        int dailyIndex = INT_MIN;
                        if(!getDailyIndex(dailyIndex,
                                            dailyIndex_,
                                            28,             //change sizeof to constant
                                            ticks[i].uni.tickOrder.market_type,
                                            datatype,
                                            ticks[i].uni.tickOrder.channel_no,
                                            convertTime(ticks[i].uni.tickOrder.order_time))){
                            LOG_ERR("[PLUGIN::AMDQUOTE]: getDailyIndex failed. ");
                            return;
                        }
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&dailyIndex, 1);
                        int sourceType = 0;
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&sourceType, 1);
                        int type = convertType(ticks[i].uni.tickOrder.order_type);
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&type, 1);
                        long long orderPrice = ticks[i].uni.tickOrder.order_price;
                        ((Vector*)(buffer[colNum++]).get())->appendLong(&orderPrice, 1);
                        long long orderVolume = ticks[i].uni.tickOrder.order_volume;
                        ((Vector*)(buffer[colNum++]).get())->appendLong(&orderVolume, 1);
                        int BSFlag = convertBSFlag(ticks[i].uni.tickOrder.side);
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&BSFlag, 1);
                        long long origOrderNo = ticks[i].uni.tickOrder.orig_order_no;
                        ((Vector*)(buffer[colNum++]).get())->appendLong(&origOrderNo, 1);
                        // same as the previous one
                        ((Vector*)(buffer[colNum++]).get())->appendLong(&origOrderNo, 1);
                        long long applSeqNum = ticks[i].uni.tickOrder.appl_seq_num;
                        ((Vector*)(buffer[colNum++]).get())->appendLong(&applSeqNum, 1);
                        int channelNo = ticks[i].uni.tickOrder.channel_no;
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&channelNo, 1);
                        reachTimeVector.push_back(ticks[i].reachTime);
                        long long reachTime = ticks[i].reachTime / 1000000;
                        ((Vector*)(buffer[colNum++]).get())->appendLong(&reachTime, 1);
                    } else {
                        if(marketType == 101) {
                            string securityCode = ticks[i].uni.tickExecution.security_code + string(".SH");
                            ((Vector*)(buffer[colNum++]).get())->appendString(&securityCode, 1);
                        } else if (marketType == 102) {
                            string securityCode = ticks[i].uni.tickExecution.security_code + string(".SZ");
                            ((Vector*)(buffer[colNum++]).get())->appendString(&securityCode, 1);
                        }
                        int orderDate = convertToDate(ticks[i].uni.tickExecution.exec_time);
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&orderDate, 1);
                        int orderTime = convertToTime(ticks[i].uni.tickExecution.exec_time);
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&orderTime, 1);
                        string securityIDSource = transMarket(ticks[i].uni.tickExecution.market_type);
                        ((Vector*)(buffer[colNum++]).get())->appendString(&securityIDSource, 1);
                        string securityType("StockType");
                        ((Vector*)(buffer[colNum++]).get())->appendString(&securityType, 1);

                        //  make sure dailyIndexFlagTotal always true
                        int dailyIndex = INT_MIN;
                        if(!getDailyIndex(dailyIndex,
                                            dailyIndex_,
                                            28,                 //change sizeof to constant
                                            ticks[i].uni.tickExecution.market_type,
                                            datatype,
                                            ticks[i].uni.tickExecution.channel_no,
                                            convertTime(ticks[i].uni.tickExecution.exec_time))){
                            LOG_ERR("[PLUGIN::AMDQUOTE]: getDailyIndex failed. ");
                            return;
                        }
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&dailyIndex, 1);
                        int sourceType = 1;
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&sourceType, 1);
                        int type = convertType(ticks[i].uni.tickExecution.exec_type);
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&type, 1);
                        long long orderPrice = ticks[i].uni.tickExecution.exec_price;
                        ((Vector*)(buffer[colNum++]).get())->appendLong(&orderPrice, 1);
                        long long orderVolume = ticks[i].uni.tickExecution.exec_volume;
                        ((Vector*)(buffer[colNum++]).get())->appendLong(&orderVolume, 1);
                        int BSFlag = convertBSFlag(ticks[i].uni.tickExecution.side);
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&BSFlag, 1);
                        long long origOrderNo = ticks[i].uni.tickExecution.bid_appl_seq_num;
                        ((Vector*)(buffer[colNum++]).get())->appendLong(&origOrderNo, 1);
                        long long offerApplSeqNum = ticks[i].uni.tickExecution.offer_appl_seq_num;
                        ((Vector*)(buffer[colNum++]).get())->appendLong(&offerApplSeqNum, 1);
                        long long applSeqNum = ticks[i].uni.tickExecution.appl_seq_num;
                        ((Vector*)(buffer[colNum++]).get())->appendLong(&applSeqNum, 1);
                        int channelNo = ticks[i].uni.tickExecution.channel_no;
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&channelNo, 1);
                        reachTimeVector.push_back(ticks[i].reachTime);
                        long long reachTime = ticks[i].reachTime / 1000000;
                        ((Vector*)(buffer[colNum++]).get())->appendLong(&reachTime, 1);
                    }
                }
            }

            vector<ConstantSP> cols;
            for(unsigned int i = 0; i < buffer.size()-1; ++i) {
                cols.push_back(buffer[i]);
            }

            vector<long long> bufVec;
            bufVec.resize(cols[15]->size());

            vector<string> colNames = orderExecutionTableMeta_.colNames_;
            if(outputElapsedFlag_) {
                colNames.push_back("perPenetrationTime");
                long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                for(int i = 0; i < cols[15]->size(); ++i) {
                    long long gap = time - reachTimeVector[i];
                    ((Vector*)(buffer[16]).get())->appendLong(&gap, 1);
                }
                cols.push_back(buffer[16]);
            }
            TableSP data = Util::createTable(colNames, cols);

            if(orderExecutionFlag_ && !(stockInsertedTable.isNull())) {
                vector<ConstantSP> args = {data};
                try {
                    if(!stockTransform.isNull())
                        data = stockTransform->call(session_->getHeap().get(), args);
                } catch (exception &e) {
                    throw RuntimeException("[PLUGIN::AMDQUOTE] call transform error " + string(e.what()));
                }
                if (stockInsertedTable.isNull()) {
                    throw RuntimeException("[PLUGIN::AMDQUOTE] stock insertedTable is null");
                }
                if (stockInsertedTable ->columns() != data->columns()) {
                    throw RuntimeException("[PLUGIN::AMDQUOTE] The number of columns of the table to insert must be the same as that of the original table");
                }
                args = {stockInsertedTable, data};
                LockGuard<Mutex> _(stockInsertedTable->getLock());
                vector<ConstantSP> colData(data->columns());
                INDEX rows;
                string errMsg;
                for(INDEX i = 0; i < data->columns(); ++i) {
                    colData[i] = data->getColumn(i);
                    if (colData[i]->getType() != stockInsertedTable->getColumnType(i)) {
                    throw RuntimeException("[PLUGIN::AMDQUOTE] The type of column " + std::to_string(i) +
                        " does not match. Expected: " + Util::getDataTypeString(stockInsertedTable->getColumnType(i)) +
                        ", Actual: " + Util::getDataTypeString(colData[i]->getType()) + ".");
                    }
                }
                stockInsertedTable->append(colData, rows, errMsg);
                if(errMsg != "") {
                    LOG_ERR("[PLUGIN::AMDQUOTE]: OnMDTickOrderExecution channel " + std::to_string(channel) + " append failed, " + errMsg);
                    return;
                }
                if (latencyFlag_) {
                    long long diff = Util::toLocalNanoTimestamp(Util::getNanoEpochTime() - startTime);
                    latencyLog(AMD_ORDER_EXECUTION, startTime, cnt, diff);
                }
            }
        } else {
            if(!fundOrderExecutionFlag_) { return; }

            TableSP fundInsertedTable = fundOrderExecutionData_[channel];
            FunctionDefSP fundTransform = fundOrderExecutionTransform_;
            int marketType;
            uint8_t varietyCategory;
            AMDDataType datatype;

            vector<ConstantSP>& buffer = fundOrderExecutionBuffer_[channel];
            for(unsigned int i = 0; i < buffer.size(); ++i) {
                ((Vector*)(buffer[i].get()))->clear();
            }

            vector<long long> reachTimeVector;
            reachTimeVector.reserve(cnt);
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
                } else if (varietyCategory == 2) {
                    datatype = AMD_FUND_ORDER_EXECUTION;
                } else {
                    return;
                }
                int colNum = 0;
                if(datatype == AMD_FUND_ORDER_EXECUTION) {
                    if(ticks[i].orderOrExecution) {
                        if(marketType == 101) {
                            string securityCode = ticks[i].uni.tickOrder.security_code + string(".SH");
                            ((Vector*)(buffer[colNum++]).get())->appendString(&securityCode, 1);
                        } else if (marketType == 102) {
                            string securityCode = ticks[i].uni.tickOrder.security_code + string(".SZ");
                            ((Vector*)(buffer[colNum++]).get())->appendString(&securityCode, 1);
                        }

                        int orderDate = convertToDate(ticks[i].uni.tickOrder.order_time);
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&orderDate, 1);
                        int orderTime = convertToTime(ticks[i].uni.tickOrder.order_time);
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&orderTime, 1);
                        string securityIDSource = transMarket(ticks[i].uni.tickOrder.market_type);
                        ((Vector*)(buffer[colNum++]).get())->appendString(&securityIDSource, 1);
                        string securityType("FundType");
                        ((Vector*)(buffer[colNum++]).get())->appendString(&securityType, 1);
                        //  make sure dailyIndexFlagTotal always true
                        int dailyIndex = INT_MIN;
                        if(!getDailyIndex(dailyIndex,
                                            dailyIndex_,
                                            28,             //change sizeof to constant
                                            ticks[i].uni.tickOrder.market_type,
                                            datatype,
                                            ticks[i].uni.tickOrder.channel_no,
                                            convertTime(ticks[i].uni.tickOrder.order_time))){
                            LOG_ERR("[PLUGIN::AMDQUOTE]: getDailyIndex failed. ");
                            return;
                        }
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&dailyIndex, 1);
                        int sourceType = 0;
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&sourceType, 1);
                        int type = convertType(ticks[i].uni.tickOrder.order_type);
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&type, 1);
                        long long orderPrice = ticks[i].uni.tickOrder.order_price;
                        ((Vector*)(buffer[colNum++]).get())->appendLong(&orderPrice, 1);
                        long long orderVolume = ticks[i].uni.tickOrder.order_volume;
                        ((Vector*)(buffer[colNum++]).get())->appendLong(&orderVolume, 1);
                        int BSFlag = convertBSFlag(ticks[i].uni.tickOrder.side);
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&BSFlag, 1);
                        long long origOrderNo = ticks[i].uni.tickOrder.orig_order_no;
                        ((Vector*)(buffer[colNum++]).get())->appendLong(&origOrderNo, 1);
                        // same as the previous one
                        ((Vector*)(buffer[colNum++]).get())->appendLong(&origOrderNo, 1);
                        long long applSeqNum = ticks[i].uni.tickOrder.appl_seq_num;
                        ((Vector*)(buffer[colNum++]).get())->appendLong(&applSeqNum, 1);
                        int channelNo = ticks[i].uni.tickOrder.channel_no;
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&channelNo, 1);
                        reachTimeVector.push_back(ticks[i].reachTime);
                        long long reachTime = ticks[i].reachTime / 1000000;
                        ((Vector*)(buffer[colNum++]).get())->appendLong(&reachTime, 1);
                    } else {
                        if(marketType == 101) {
                            string securityCode = ticks[i].uni.tickExecution.security_code + string(".SH");
                            ((Vector*)(buffer[colNum++]).get())->appendString(&securityCode, 1);
                        } else if (marketType == 102) {
                            string securityCode = ticks[i].uni.tickExecution.security_code + string(".SZ");
                            ((Vector*)(buffer[colNum++]).get())->appendString(&securityCode, 1);
                        }
                        int orderDate = convertToDate(ticks[i].uni.tickExecution.exec_time);
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&orderDate, 1);
                        int orderTime = convertToTime(ticks[i].uni.tickExecution.exec_time);
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&orderTime, 1);
                        string securityIDSource = transMarket(ticks[i].uni.tickExecution.market_type);
                        ((Vector*)(buffer[colNum++]).get())->appendString(&securityIDSource, 1);
                        string securityType("FundType");
                        ((Vector*)(buffer[colNum++]).get())->appendString(&securityType, 1);
                        //  make sure dailyIndexFlagTotal always true
                        int dailyIndex = INT_MIN;
                        if(!getDailyIndex(dailyIndex,
                                            dailyIndex_,
                                            28,                 //change sizeof to constant
                                            ticks[i].uni.tickExecution.market_type,
                                            datatype,
                                            ticks[i].uni.tickExecution.channel_no,
                                            convertTime(ticks[i].uni.tickExecution.exec_time))){
                            LOG_ERR("[PLUGIN::AMDQUOTE]: getDailyIndex failed. ");
                            return;
                        }
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&dailyIndex, 1);
                        int sourceType = 1;
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&sourceType, 1);
                        int type = convertType(ticks[i].uni.tickExecution.exec_type);
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&type, 1);
                        long long orderPrice = ticks[i].uni.tickExecution.exec_price;
                        ((Vector*)(buffer[colNum++]).get())->appendLong(&orderPrice, 1);
                        long long orderVolume = ticks[i].uni.tickExecution.exec_volume;
                        ((Vector*)(buffer[colNum++]).get())->appendLong(&orderVolume, 1);
                        int BSFlag = convertBSFlag(ticks[i].uni.tickExecution.side);
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&BSFlag, 1);
                        long long origOrderNo = ticks[i].uni.tickExecution.bid_appl_seq_num;
                        ((Vector*)(buffer[colNum++]).get())->appendLong(&origOrderNo, 1);
                        long long offerApplSeqNum = ticks[i].uni.tickExecution.offer_appl_seq_num;
                        ((Vector*)(buffer[colNum++]).get())->appendLong(&offerApplSeqNum, 1);
                        long long applSeqNum = ticks[i].uni.tickExecution.appl_seq_num;
                        ((Vector*)(buffer[colNum++]).get())->appendLong(&applSeqNum, 1);
                        int channelNo = ticks[i].uni.tickExecution.channel_no;
                        ((Vector*)(buffer[colNum++]).get())->appendInt(&channelNo, 1);
                        reachTimeVector.push_back(ticks[i].reachTime);
                        long long reachTime = ticks[i].reachTime / 1000000;
                        ((Vector*)(buffer[colNum++]).get())->appendLong(&reachTime, 1);
                    }
                }
            }
            vector<ConstantSP> fcols;
            for(unsigned int i = 0; i < buffer.size()-1; ++i) {
                fcols.push_back(buffer[i]);
            }

            vector<long long> bufVec;
            bufVec.resize(fcols[15]->size());

            vector<string> fcolNames = orderExecutionTableMeta_.colNames_;
            if(outputElapsedFlag_) {
                long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
                for(int i = 0; i < fcols[15]->size(); ++i) {
                    long long gap = time - reachTimeVector[i];
                    ((Vector*)(buffer[16]).get())->appendLong(&gap, 1);
                }
                fcols.push_back(buffer[16]);
                fcolNames.push_back("perPenetrationTime");
            }

            TableSP fData = Util::createTable(fcolNames, fcols);
            if (fundOrderExecutionFlag_ && !(fundInsertedTable.isNull())){
                vector<ConstantSP> fArgs = {fData};
                try{
                    if(!fundTransform.isNull())
                        fData = fundTransform->call(session_->getHeap().get(), fArgs);
                }catch(exception &e){
                    throw RuntimeException("[PLUGIN::AMDQUOTE] call transform error " + string(e.what()));
                }

                if(fundInsertedTable.isNull())
                    throw RuntimeException("[PLUGIN::AMDQUOTE] fund insertedTable is null");
                if(fundInsertedTable ->columns() != fData->columns()) {
                    throw RuntimeException("[PLUGIN::AMDQUOTE] The number of columns of the table to insert must be the same as that of the original table");
                }

                fArgs = {fundInsertedTable, fData};
                LockGuard<Mutex> _f(fundInsertedTable->getLock());

                vector<ConstantSP> fcolData(fData->columns());
                for(INDEX i = 0; i < fData->columns(); ++i) {
                    fcolData[i] = fData->getColumn(i);
                    if (fcolData[i]->getType() != fundInsertedTable->getColumnType(i)) {
                    throw RuntimeException("[PLUGIN::AMDQUOTE] The type of column " + std::to_string(i) +
                        " does not match. Expected: " + Util::getDataTypeString(fundInsertedTable->getColumnType(i)) +
                        ", Actual: " + Util::getDataTypeString(fcolData[i]->getType()) + ".");
                    }
                }
                INDEX frows;
                string fErrMsg;
                fundInsertedTable->append(fcolData, frows, fErrMsg);
                if(fErrMsg != "") {
                    LOG_ERR("[PLUGIN::AMDQUOTE]: OnMDTickFundOrderExecution channel " + std::to_string(channel) + " append failed, " + fErrMsg);
                    return;
                }
                if (latencyFlag_) {
                    long long diff = Util::toLocalNanoTimestamp(Util::getNanoEpochTime() - startTime);
                    latencyLog(AMD_FUND_ORDER_EXECUTION, startTime, cnt, diff);
                }
            }
        }
    }
    catch(exception &e){
        LOG_ERR("[PLUGIN::AMDQUOTE]: OnMDTickOrderExecution channel " + std::to_string(channel) + " failed, ", e.what());
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
            LOG_ERR("[PLUGIN::AMDQUOTE]: error amd table type AMD_ERROR_TABLE_TYPE");
            return;
        }

        vector<ConstantSP>& buffer = bondOrderExecutionBuffer_[channel];
        for(unsigned int i = 0; i < buffer.size(); ++i) {
            ((Vector*)(buffer[i].get()))->clear();
        }
        vector<long long> reachTimeVector;
        reachTimeVector.reserve(cnt);
        for (uint32_t i = 0; i < cnt; ++i) {
            int colNum = 0;
            if(ticks[i].orderOrExecution) {
                if(marketType == 101) {
                    string securityCode = ticks[i].uni.tickOrder.security_code + string(".SH");
                    ((Vector*)(buffer[colNum++]).get())->appendString(&securityCode, 1);
                } else if (marketType == 102) {
                    string securityCode = ticks[i].uni.tickOrder.security_code + string(".SZ");
                    ((Vector*)(buffer[colNum++]).get())->appendString(&securityCode, 1);
                }
                int orderDate = convertToDate(ticks[i].uni.tickOrder.order_time);
                ((Vector*)(buffer[colNum++]).get())->appendInt(&orderDate, 1);
                int orderTime = convertToTime(ticks[i].uni.tickOrder.order_time);
                ((Vector*)(buffer[colNum++]).get())->appendInt(&orderTime, 1);
                string securityIDSource = transMarket(ticks[i].uni.tickOrder.market_type);
                ((Vector*)(buffer[colNum++]).get())->appendString(&securityIDSource, 1);
                string securityType("BondType");
                ((Vector*)(buffer[colNum++]).get())->appendString(&securityType, 1);

                //  make sure dailyIndexFlagTotal always true
                int dailyIndex = INT_MIN;
                if(!getDailyIndex(dailyIndex,
                                    dailyIndex_,
                                    28,             //change sizeof to constant
                                    ticks[i].uni.tickOrder.market_type,
                                    datatype,
                                    ticks[i].uni.tickOrder.channel_no,
                                    convertTime(ticks[i].uni.tickOrder.order_time))){
                    LOG_ERR("[PLUGIN::AMDQUOTE]: getDailyIndex failed. ");
                    return;
                }
                ((Vector*)(buffer[colNum++]).get())->appendInt(&dailyIndex, 1);
                int sourceType = 0;
                ((Vector*)(buffer[colNum++]).get())->appendInt(&sourceType, 1);
                int type = convertType(ticks[i].uni.tickOrder.order_type);
                ((Vector*)(buffer[colNum++]).get())->appendInt(&type, 1);
                long long orderPrice = ticks[i].uni.tickOrder.order_price;
                ((Vector*)(buffer[colNum++]).get())->appendLong(&orderPrice, 1);
                long long orderVolume = ticks[i].uni.tickOrder.order_volume;
                ((Vector*)(buffer[colNum++]).get())->appendLong(&orderVolume, 1);
                int BSFlag = convertBSFlag(ticks[i].uni.tickOrder.side);
                ((Vector*)(buffer[colNum++]).get())->appendInt(&BSFlag, 1);
                long long origOrderNo = ticks[i].uni.tickOrder.orig_order_no;
                ((Vector*)(buffer[colNum++]).get())->appendLong(&origOrderNo, 1);
                // same as the previous one
                ((Vector*)(buffer[colNum++]).get())->appendLong(&origOrderNo, 1);
                long long applSeqNum = ticks[i].uni.tickOrder.appl_seq_num;
                ((Vector*)(buffer[colNum++]).get())->appendLong(&applSeqNum, 1);
                int channelNo = ticks[i].uni.tickOrder.channel_no;
                ((Vector*)(buffer[colNum++]).get())->appendInt(&channelNo, 1);
                reachTimeVector.push_back(ticks[i].reachTime);
                long long reachTime = ticks[i].reachTime / 1000000;
                ((Vector*)(buffer[colNum++]).get())->appendLong(&reachTime, 1);
            } else {
                if(marketType == 101) {
                    string securityCode = ticks[i].uni.tickExecution.security_code + string(".SH");
                    ((Vector*)(buffer[colNum++]).get())->appendString(&securityCode, 1);
                } else if (marketType == 102) {
                    string securityCode = ticks[i].uni.tickExecution.security_code + string(".SZ");
                    ((Vector*)(buffer[colNum++]).get())->appendString(&securityCode, 1);
                }
                int orderDate = convertToDate(ticks[i].uni.tickExecution.exec_time);
                ((Vector*)(buffer[colNum++]).get())->appendInt(&orderDate, 1);
                int orderTime = convertToTime(ticks[i].uni.tickExecution.exec_time);
                ((Vector*)(buffer[colNum++]).get())->appendInt(&orderTime, 1);
                string securityIDSource = transMarket(ticks[i].uni.tickExecution.market_type);
                ((Vector*)(buffer[colNum++]).get())->appendString(&securityIDSource, 1);
                string securityType("BondType");
                ((Vector*)(buffer[colNum++]).get())->appendString(&securityType, 1);

                //  make sure dailyIndexFlagTotal always true
                int dailyIndex = INT_MIN;
                if(!getDailyIndex(dailyIndex,
                                    dailyIndex_,
                                    28,             //change sizeof to constant
                                    ticks[i].uni.tickExecution.market_type,
                                    datatype,
                                    ticks[i].uni.tickExecution.channel_no,
                                    convertTime(ticks[i].uni.tickExecution.exec_time))){
                    LOG_ERR("[PLUGIN::AMDQUOTE]: getDailyIndex failed. ");
                    return;
                }
                ((Vector*)(buffer[colNum++]).get())->appendInt(&dailyIndex, 1);
                int sourceType = 1;
                ((Vector*)(buffer[colNum++]).get())->appendInt(&sourceType, 1);
                int type = convertType(ticks[i].uni.tickExecution.exec_type);
                ((Vector*)(buffer[colNum++]).get())->appendInt(&type, 1);
                long long orderPrice = ticks[i].uni.tickExecution.exec_price;
                ((Vector*)(buffer[colNum++]).get())->appendLong(&orderPrice, 1);
                long long orderVolume = ticks[i].uni.tickExecution.exec_volume;
                ((Vector*)(buffer[colNum++]).get())->appendLong(&orderVolume, 1);
                int BSFlag = convertBSFlag(ticks[i].uni.tickExecution.side);
                ((Vector*)(buffer[colNum++]).get())->appendInt(&BSFlag, 1);
                long long origOrderNo = ticks[i].uni.tickExecution.bid_appl_seq_num;
                ((Vector*)(buffer[colNum++]).get())->appendLong(&origOrderNo, 1);
                long long offerApplSeqNum = ticks[i].uni.tickExecution.offer_appl_seq_num;
                ((Vector*)(buffer[colNum++]).get())->appendLong(&offerApplSeqNum, 1);
                long long applSeqNum = ticks[i].uni.tickExecution.appl_seq_num;
                ((Vector*)(buffer[colNum++]).get())->appendLong(&applSeqNum, 1);
                int channelNo = ticks[i].uni.tickExecution.channel_no;
                ((Vector*)(buffer[colNum++]).get())->appendInt(&channelNo, 1);
                reachTimeVector.push_back(ticks[i].reachTime);
                long long reachTime = ticks[i].reachTime / 1000000;
                ((Vector*)(buffer[colNum++]).get())->appendLong(&reachTime, 1);
            }
        }

        vector<ConstantSP> cols;
        for(unsigned int i = 0; i < buffer.size()-1; ++i) {
            cols.push_back(buffer[i]);
        }

        vector<long long> bufVec;
        bufVec.resize(cols[15]->size());

        vector<string> colNames = orderExecutionTableMeta_.colNames_;
        if(outputElapsedFlag_) {
            long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
            for(int i = 0; i < cols[15]->size(); ++i) {
                long long gap = time - reachTimeVector[i];
                ((Vector*)(buffer[16]).get())->appendLong(&gap, 1);
            }
            cols.push_back(buffer[16]);
            colNames.push_back("perPenetrationTime");
        }
        TableSP data = Util::createTable(colNames, cols);

        vector<ConstantSP> args = {data};
        try{
            if(!transform.isNull())
                data = transform->call(session_->getHeap().get(), args);
        }catch(exception &e){
            throw RuntimeException("[PLUGIN::AMDQUOTE] call transform error " + string(e.what()));
        }

        if(insertedTable.isNull())
            throw RuntimeException("[PLUGIN::AMDQUOTE] insertedTable is null");
        if(insertedTable ->columns() != data->columns()) {
            throw RuntimeException("[PLUGIN::AMDQUOTE] The number of columns of the table to insert must be the same as that of the original table");
        }
        args = {insertedTable, data};
        LockGuard<Mutex> _(insertedTable->getLock());
        INDEX rows;
        string errMsg;
        vector<ConstantSP> colData(data->columns());
        for(INDEX i = 0; i < data->columns(); ++i) {
            colData[i] = data->getColumn(i);
            if (colData[i]->getType() != insertedTable->getColumnType(i)) {
            throw RuntimeException("[PLUGIN::AMDQUOTE] The type of column " + std::to_string(i) +
                " does not match. Expected: " + Util::getDataTypeString(insertedTable->getColumnType(i)) +
                ", Actual: " + Util::getDataTypeString(colData[i]->getType()) + ".");
            }
        }
        insertedTable->append(colData, rows, errMsg);
        if(errMsg != "") {
            LOG_ERR("[PLUGIN::AMDQUOTE]: OnMDTickBondOrderExecution channel " + std::to_string(channel) + " append failed, " + errMsg);
            return;
        }

        if (latencyFlag_) {
            long long diff = Util::toLocalNanoTimestamp(Util::getNanoEpochTime() - startTime);
            latencyLog(AMD_BOND_ORDER_EXECUTION, startTime, cnt, diff);
        }
    }
    catch(exception &e){
        LOG_ERR("[PLUGIN::AMDQUOTE]: OnMDTickBondOrderExecution channel " + std::to_string(channel) + " failed, ", e.what());
    }
}

template <typename T>
inline void structReader(vector<ConstantSP>& buffer, T& data) {
    throw RuntimeException("[PLUGIN::AMDQUOTE] Unsupported AMD data structure");
}

template <>
inline void structReader(vector<ConstantSP>& buffer, timeMDOrderQueue& data) {
    int colNum = 0;
    int marketType = data.orderQueue.market_type;
    ((Vector*)(buffer[colNum++]).get())->appendInt(&marketType, 1);
    string securityCode = data.orderQueue.security_code;
    ((Vector*)(buffer[colNum++]).get())->appendString(&securityCode, 1);
    long long orderTime = convertTime(data.orderQueue.order_time);
    ((Vector*)(buffer[colNum++]).get())->appendLong(&orderTime, 1);
    char side = data.orderQueue.side;
    ((Vector*)(buffer[colNum++]).get())->appendChar(&side, 1);
    long long orderPrice = data.orderQueue.order_price;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&orderPrice, 1);
    long long orderVolume = data.orderQueue.order_volume;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&orderVolume, 1);
    int numOfOrders = data.orderQueue.num_of_orders;
    ((Vector*)(buffer[colNum++]).get())->appendInt(&numOfOrders, 1);
    int items = data.orderQueue.items;
    ((Vector*)(buffer[colNum++]).get())->appendInt(&items, 1);

    long long volume0 = data.orderQueue.volume[0];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume0, 1);
    long long volume1 = data.orderQueue.volume[1];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume1, 1);
    long long volume2 = data.orderQueue.volume[2];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume2, 1);
    long long volume3 = data.orderQueue.volume[3];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume3, 1);
    long long volume4 = data.orderQueue.volume[4];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume4, 1);
    long long volume5 = data.orderQueue.volume[5];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume5, 1);
    long long volume6 = data.orderQueue.volume[6];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume6, 1);
    long long volume7 = data.orderQueue.volume[7];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume7, 1);
    long long volume8 = data.orderQueue.volume[8];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume8, 1);
    long long volume9 = data.orderQueue.volume[9];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume9, 1);
    long long volume10 = data.orderQueue.volume[10];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume10, 1);

    long long volume11 = data.orderQueue.volume[11];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume11, 1);
    long long volume12 = data.orderQueue.volume[12];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume12, 1);
    long long volume13 = data.orderQueue.volume[13];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume13, 1);
    long long volume14 = data.orderQueue.volume[14];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume14, 1);
    long long volume15 = data.orderQueue.volume[15];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume15, 1);
    long long volume16 = data.orderQueue.volume[16];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume16, 1);
    long long volume17 = data.orderQueue.volume[17];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume17, 1);
    long long volume18 = data.orderQueue.volume[18];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume18, 1);
    long long volume19 = data.orderQueue.volume[19];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume19, 1);
    long long volume20 = data.orderQueue.volume[20];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume20, 1);

    long long volume21 = data.orderQueue.volume[21];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume21, 1);
    long long volume22 = data.orderQueue.volume[22];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume22, 1);
    long long volume23 = data.orderQueue.volume[23];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume23, 1);
    long long volume24 = data.orderQueue.volume[24];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume24, 1);
    long long volume25 = data.orderQueue.volume[25];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume25, 1);
    long long volume26 = data.orderQueue.volume[26];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume26, 1);
    long long volume27 = data.orderQueue.volume[27];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume27, 1);
    long long volume28 = data.orderQueue.volume[28];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume28, 1);
    long long volume29 = data.orderQueue.volume[29];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume29, 1);
    long long volume30 = data.orderQueue.volume[30];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume30, 1);

    long long volume31 = data.orderQueue.volume[31];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume31, 1);
    long long volume32 = data.orderQueue.volume[32];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume32, 1);
    long long volume33 = data.orderQueue.volume[33];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume33, 1);
    long long volume34 = data.orderQueue.volume[34];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume34, 1);
    long long volume35 = data.orderQueue.volume[35];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume35, 1);
    long long volume36 = data.orderQueue.volume[36];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume36, 1);
    long long volume37 = data.orderQueue.volume[37];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume37, 1);
    long long volume38 = data.orderQueue.volume[38];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume38, 1);
    long long volume39 = data.orderQueue.volume[39];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume39, 1);
    long long volume40 = data.orderQueue.volume[40];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume40, 1);

    long long volume41 = data.orderQueue.volume[41];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume41, 1);
    long long volume42 = data.orderQueue.volume[42];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume42, 1);
    long long volume43 = data.orderQueue.volume[43];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume43, 1);
    long long volume44 = data.orderQueue.volume[44];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume44, 1);
    long long volume45 = data.orderQueue.volume[45];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume45, 1);
    long long volume46 = data.orderQueue.volume[46];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume46, 1);
    long long volume47 = data.orderQueue.volume[47];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume47, 1);
    long long volume48 = data.orderQueue.volume[48];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume48, 1);
    long long volume49 = data.orderQueue.volume[49];
    ((Vector*)(buffer[colNum++]).get())->appendLong(&volume49, 1);

    int channelNo = data.orderQueue.channel_no;
    ((Vector*)(buffer[colNum++]).get())->appendInt(&channelNo, 1);
    string mdStreamId = data.orderQueue.md_stream_id;
    ((Vector*)(buffer[colNum++]).get())->appendString(&mdStreamId, 1);
    char varietyCategory = data.orderQueue.variety_category;
    ((Vector*)(buffer[colNum++]).get())->appendChar(&varietyCategory, 1);
}

template <>
inline void structReader(vector<ConstantSP>& buffer, timeMDIndexSnapshot& data) {
    int colNum = 0;
    int marketType = data.indexSnapshot.market_type;
    ((Vector*)(buffer[colNum++]).get())->appendInt(&marketType, 1);
    string securityCode = data.indexSnapshot.security_code;
    ((Vector*)(buffer[colNum++]).get())->appendString(&securityCode, 1);
    long long origTime = convertTime(data.indexSnapshot.orig_time);
    ((Vector*)(buffer[colNum++]).get())->appendLong(&origTime, 1);
    string tradingPhaseCode = data.indexSnapshot.trading_phase_code;
    ((Vector*)(buffer[colNum++]).get())->appendString(&tradingPhaseCode, 1);
    long long preCloseIndex = data.indexSnapshot.pre_close_index;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&preCloseIndex, 1);
    long long openIndex = data.indexSnapshot.open_index;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&openIndex, 1);
    long long highIndex = data.indexSnapshot.high_index;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&highIndex, 1);

    long long lowIndex = data.indexSnapshot.low_index;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&lowIndex, 1);
    long long lastIndex = data.indexSnapshot.last_index;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&lastIndex, 1);
    long long closeIndex = data.indexSnapshot.close_index;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&closeIndex, 1);
    long long totalVolumeTrade = data.indexSnapshot.total_volume_trade;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&totalVolumeTrade, 1);
    long long totalValueTrade = data.indexSnapshot.total_value_trade;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&totalValueTrade, 1);
    int channelNo = data.indexSnapshot.channel_no;
    ((Vector*)(buffer[colNum++]).get())->appendInt(&channelNo, 1);
    string mdStreamId = data.indexSnapshot.md_stream_id;
    ((Vector*)(buffer[colNum++]).get())->appendString(&mdStreamId, 1);

    char varietyCategory = data.indexSnapshot.variety_category;
    ((Vector*)(buffer[colNum++]).get())->appendChar(&varietyCategory, 1);
}

template <>
inline void structReader(vector<ConstantSP>& buffer, timeMDTickExecution& data) {
    int colNum = 0;
    int marketType = data.execution.market_type;
    ((Vector*)(buffer[colNum++]).get())->appendInt(&marketType, 1);
    string securityCode = data.execution.security_code;
    ((Vector*)(buffer[colNum++]).get())->appendString(&securityCode, 1);
    long long execTime = convertTime(data.execution.exec_time);
    ((Vector*)(buffer[colNum++]).get())->appendLong(&execTime, 1);
    int channelNo = data.execution.channel_no;
    ((Vector*)(buffer[colNum++]).get())->appendInt(&channelNo, 1);
    long long applSeqNum = data.execution.appl_seq_num;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&applSeqNum, 1);
    long long execPrice = data.execution.exec_price;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&execPrice, 1);
    long long execVolume = data.execution.exec_volume;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&execVolume, 1);
    long long valueTrade = data.execution.variety_category;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&valueTrade, 1);

    long long bidAppSeqNum = data.execution.bid_appl_seq_num;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&bidAppSeqNum, 1);
    long long offerApplSeqNum = data.execution.offer_appl_seq_num;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&offerApplSeqNum, 1);
    char side = data.execution.side;
    ((Vector*)(buffer[colNum++]).get())->appendChar(&side, 1);
    char execType = data.execution.exec_type;
    ((Vector*)(buffer[colNum++]).get())->appendChar(&execType, 1);
    string mdStreamId = data.execution.md_stream_id;
    ((Vector*)(buffer[colNum++]).get())->appendString(&mdStreamId, 1);
    long long bizIndex = data.execution.biz_index;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&bizIndex, 1);
    char varietyCategory = data.execution.variety_category;
    ((Vector*)(buffer[colNum++]).get())->appendChar(&varietyCategory, 1);
}

template <>
inline void structReader(vector<ConstantSP>& buffer, timeMDBondTickExecution& data) {
    int colNum = 0;
    int marketType = data.bondExecution.market_type;
    ((Vector*)(buffer[colNum++]).get())->appendInt(&marketType, 1);
    string securityCode = data.bondExecution.security_code;
    ((Vector*)(buffer[colNum++]).get())->appendString(&securityCode, 1);
    long long execTime = convertTime(data.bondExecution.exec_time);
    ((Vector*)(buffer[colNum++]).get())->appendLong(&execTime, 1);
    int channelNo = data.bondExecution.channel_no;
    ((Vector*)(buffer[colNum++]).get())->appendInt(&channelNo, 1);
    long long applSeqNum = data.bondExecution.appl_seq_num;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&applSeqNum, 1);
    long long execPrice = data.bondExecution.exec_price;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&execPrice, 1);
    long long execVolume = data.bondExecution.exec_volume;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&execVolume, 1);
    long long valueTrade = data.bondExecution.variety_category;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&valueTrade, 1);

    long long bidAppSeqNum = data.bondExecution.bid_appl_seq_num;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&bidAppSeqNum, 1);
    long long offerApplSeqNum = data.bondExecution.offer_appl_seq_num;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&offerApplSeqNum, 1);
    char side = data.bondExecution.side;
    ((Vector*)(buffer[colNum++]).get())->appendChar(&side, 1);
    char execType = data.bondExecution.exec_type;
    ((Vector*)(buffer[colNum++]).get())->appendChar(&execType, 1);
    string mdStreamId = data.bondExecution.md_stream_id;
    ((Vector*)(buffer[colNum++]).get())->appendString(&mdStreamId, 1);
    long long bizIndex = LONG_LONG_MIN;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&bizIndex, 1);
    char varietyCategory = data.bondExecution.variety_category;
    ((Vector*)(buffer[colNum++]).get())->appendChar(&varietyCategory, 1);
}

template <>
inline void structReader(vector<ConstantSP>& buffer, timeMDBondTickOrder& data) {
    int colNum = 0;
    int marketType = data.bondOrder.market_type;
    ((Vector*)(buffer[colNum++]).get())->appendInt(&marketType, 1);
    string securityCode(data.bondOrder.security_code);
    ((Vector*)(buffer[colNum++]).get())->appendString(&securityCode, 1);
    int channelNo = data.bondOrder.channel_no;
    ((Vector*)(buffer[colNum++]).get())->appendInt(&channelNo, 1);
    long long applSeqNum = data.bondOrder.appl_seq_num;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&applSeqNum, 1);
    long long orderTime = convertTime(data.bondOrder.order_time);
    ((Vector*)(buffer[colNum++]).get())->appendLong(&orderTime, 1);
    long long orderPrice = data.bondOrder.order_price;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&orderPrice, 1);
    long long orderVolume = data.bondOrder.order_volume;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&orderVolume, 1);
    int side = data.bondOrder.side;
    ((Vector*)(buffer[colNum++]).get())->appendInt(&side, 1);
    int orderType = data.bondOrder.order_type;
    ((Vector*)(buffer[colNum++]).get())->appendInt(&orderType, 1);
    string mdStreamId = data.bondOrder.md_stream_id;
    ((Vector*)(buffer[colNum++]).get())->appendString(&mdStreamId, 1);
    long long origOrderNo = data.bondOrder.orig_order_no;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&origOrderNo, 1);
    long long bizIndex = LONG_LONG_MIN;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&bizIndex, 1);
    int varietyCategory = data.bondOrder.variety_category;
    ((Vector*)(buffer[colNum++]).get())->appendInt(&varietyCategory, 1);
}

template <>
inline void structReader(vector<ConstantSP>& buffer, timeMDTickOrder& data) {
    int colNum = 0;
    int marketType = data.order.market_type;
    ((Vector*)(buffer[colNum++]).get())->appendInt(&marketType, 1);
    string securityCode(data.order.security_code);
    ((Vector*)(buffer[colNum++]).get())->appendString(&securityCode, 1);
    int channelNo = data.order.channel_no;
    ((Vector*)(buffer[colNum++]).get())->appendInt(&channelNo, 1);
    long long applSeqNum = data.order.appl_seq_num;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&applSeqNum, 1);
    long long orderTime = convertTime(data.order.order_time);
    ((Vector*)(buffer[colNum++]).get())->appendLong(&orderTime, 1);
    long long orderPrice = data.order.order_price;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&orderPrice, 1);
    long long orderVolume = data.order.order_volume;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&orderVolume, 1);
    int side = data.order.side;
    ((Vector*)(buffer[colNum++]).get())->appendInt(&side, 1);
    int orderType = data.order.order_type;
    ((Vector*)(buffer[colNum++]).get())->appendInt(&orderType, 1);
    string mdStreamId = data.order.md_stream_id;
    ((Vector*)(buffer[colNum++]).get())->appendString(&mdStreamId, 1);
    long long origOrderNo = data.order.orig_order_no;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&origOrderNo, 1);
    long long bizIndex = data.order.biz_index;
    ((Vector*)(buffer[colNum++]).get())->appendLong(&bizIndex, 1);
    int varietyCategory = data.order.variety_category;
    ((Vector*)(buffer[colNum++]).get())->appendInt(&varietyCategory, 1);
}

template <>
inline void structReader(vector<ConstantSP>& buffer, timeMDBondSnapshot& data) {
    int colNum = 0;
    int marketType          = data.bondSnapshot.market_type;
    ((Vector*)buffer[colNum++].get())->appendInt(&marketType, 1);
    string securityCode     = data.bondSnapshot.security_code;
    ((Vector*)buffer[colNum++].get())->appendString(&securityCode, 1);
    long long origTime      = convertTime(data.bondSnapshot.orig_time);
    ((Vector*)buffer[colNum++].get())->appendLong(&origTime, 1);
    string tradingPhaseCode = data.bondSnapshot.trading_phase_code;
    ((Vector*)buffer[colNum++].get())->appendString(&tradingPhaseCode, 1);
    long long preClosePrice = data.bondSnapshot.pre_close_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&preClosePrice, 1);
    long long openPrice     = data.bondSnapshot.open_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&openPrice, 1);
    long long highPrice     = data.bondSnapshot.high_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&highPrice, 1);
    long long lowPrice      = data.bondSnapshot.low_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&lowPrice, 1);
    long long lastPrice     = data.bondSnapshot.last_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&lastPrice, 1);
    long long closePrice    = data.bondSnapshot.close_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&closePrice, 1);

    int bidPriceIndex = 0;
    long long bidPrice1 = data.bondSnapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice1, 1);
    long long bidPrice2 = data.bondSnapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice2, 1);
    long long bidPrice3 = data.bondSnapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice3, 1);
    long long bidPrice4 = data.bondSnapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice4, 1);
    long long bidPrice5 = data.bondSnapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice5, 1);
    long long bidPrice6 = data.bondSnapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice6, 1);
    long long bidPrice7 = data.bondSnapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice7, 1);
    long long bidPrice8 = data.bondSnapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice8, 1);
    long long bidPrice9 = data.bondSnapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice9, 1);
    long long bidPrice10= data.bondSnapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice10, 1);

    int bidVolumeIndex = 0;
    long long bidVolume1 = data.bondSnapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume1, 1);
    long long bidVolume2 = data.bondSnapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume2, 1);
    long long bidVolume3 = data.bondSnapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume3, 1);
    long long bidVolume4 = data.bondSnapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume4, 1);
    long long bidVolume5 = data.bondSnapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume5, 1);
    long long bidVolume6 = data.bondSnapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume6, 1);
    long long bidVolume7 = data.bondSnapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume7, 1);
    long long bidVolume8 = data.bondSnapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume8, 1);
    long long bidVolume9 = data.bondSnapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume9, 1);
    long long bidVolume10= data.bondSnapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume10, 1);

    int offerPriceIndex = 0;
    long long offerPrice1 = data.bondSnapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice1, 1);
    long long offerPrice2 = data.bondSnapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice2, 1);
    long long offerPrice3 = data.bondSnapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice3, 1);
    long long offerPrice4 = data.bondSnapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice4, 1);
    long long offerPrice5 = data.bondSnapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice5, 1);
    long long offerPrice6 = data.bondSnapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice6, 1);
    long long offerPrice7 = data.bondSnapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice7, 1);
    long long offerPrice8 = data.bondSnapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice8, 1);
    long long offerPrice9 = data.bondSnapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice9, 1);
    long long offerPrice10= data.bondSnapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice10, 1);

    int offerVolumeIndex = 0;
    long long offerVolume1  = data.bondSnapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume1, 1);
    long long offerVolume2  = data.bondSnapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume2, 1);
    long long offerVolume3  = data.bondSnapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume3, 1);
    long long offerVolume4  = data.bondSnapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume4, 1);
    long long offerVolume5  = data.bondSnapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume5, 1);
    long long offerVolume6  = data.bondSnapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume6, 1);
    long long offerVolume7  = data.bondSnapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume7, 1);
    long long offerVolume8  = data.bondSnapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume8, 1);
    long long offerVolume9  = data.bondSnapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume9, 1);
    long long offerVolume10 = data.bondSnapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume10, 1);

    long long numTrades             = data.bondSnapshot.num_trades;
    ((Vector*)buffer[colNum++].get())->appendLong(&numTrades, 1);
    long long totalVolumeTrade      = data.bondSnapshot.total_volume_trade;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalVolumeTrade, 1);
    long long totalValueTrade       = data.bondSnapshot.total_value_trade;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalValueTrade, 1);
    long long totalBidVolume        = data.bondSnapshot.total_bid_volume;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalBidVolume, 1);
    long long totalOfferVolume      = data.bondSnapshot.total_offer_volume;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalOfferVolume, 1);
    long long weightedAvgBidPrice   = data.bondSnapshot.weighted_avg_bid_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&weightedAvgBidPrice, 1);
    long long weightedAvgOfferPrice = data.bondSnapshot.weighted_avg_offer_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&weightedAvgOfferPrice, 1);
    long long ioPV                  = 0;
    ((Vector*)buffer[colNum++].get())->appendLong(&ioPV, 1);
    long long yieldToMaturity       = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&yieldToMaturity, 1);
    long long highLimited           = data.bondSnapshot.high_limited;
    ((Vector*)buffer[colNum++].get())->appendLong(&highLimited, 1);

    long long lowLimited             = data.bondSnapshot.low_limited;
    ((Vector*)buffer[colNum++].get())->appendLong(&lowLimited, 1);
    long long priceEarningRatio1     = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&priceEarningRatio1, 1);
    long long priceEarningRatio2     = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&priceEarningRatio2, 1);
    long long change1                = data.bondSnapshot.change1;
    ((Vector*)buffer[colNum++].get())->appendLong(&change1, 1);
    long long change2                = data.bondSnapshot.change2;
    ((Vector*)buffer[colNum++].get())->appendLong(&change2, 1);
    int channelNo                    = data.bondSnapshot.channel_no;
    ((Vector*)buffer[colNum++].get())->appendInt(&channelNo, 1);
    string mdStreamID                = data.bondSnapshot.md_stream_id;
    ((Vector*)buffer[colNum++].get())->appendString(&mdStreamID, 1);
    string instrumentStatus          = data.bondSnapshot.instrument_status;
    ((Vector*)buffer[colNum++].get())->appendString(&instrumentStatus, 1);
    long long preCloseIOPV           = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&preCloseIOPV, 1);
    long long altWeightedAvgBidPrice = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&altWeightedAvgBidPrice, 1);

    long long altWeightedAvgOfferPrice = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&altWeightedAvgOfferPrice, 1);
    long long etfBuyNumber             = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&etfBuyNumber, 1);
    long long etfBuyAmount             = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&etfBuyAmount, 1);
    long long etfBuyMoney              = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&etfBuyMoney, 1);
    long long etfSellNumber            = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&etfSellNumber, 1);
    long long etfSellAmount            = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&etfSellAmount, 1);
    long long etfSellMoney             = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&etfSellMoney, 1);
    long long totalWarrantExecVolume   = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalWarrantExecVolume, 1);
    long long warLowerPrice            = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&warLowerPrice, 1);

    long long warUpperPrice       = LONG_LONG_MIN;
    ((Vector*)buffer[colNum++].get())->appendLong(&warUpperPrice, 1);
    long long withdrawBuyNumber   = data.bondSnapshot.withdraw_buy_number;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawBuyNumber, 1);
    long long withdrawBuyAmount   = data.bondSnapshot.withdraw_buy_amount;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawBuyAmount, 1);
    long long withdrawBuyMoney    = data.bondSnapshot.withdraw_buy_money;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawBuyMoney, 1);
    long long withdrawSellNumber  = data.bondSnapshot.withdraw_sell_number;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawSellNumber, 1);
    long long withdrawSellAmount  = data.bondSnapshot.withdraw_sell_amount;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawSellAmount, 1);
    long long withdrawSellMoney   = data.bondSnapshot.withdraw_sell_money;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawSellMoney, 1);
    long long totalBidNumber      = data.bondSnapshot.total_bid_number;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalBidNumber, 1);
    long long totalOfferNumber    = data.bondSnapshot.total_offer_number;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalOfferNumber, 1);
    int bidTradeMaxDuration       = data.bondSnapshot.bid_trade_max_duration;
    ((Vector*)buffer[colNum++].get())->appendInt(&bidTradeMaxDuration, 1);

    int offerTradeMaxDuration       = data.bondSnapshot.offer_trade_max_duration;
    ((Vector*)buffer[colNum++].get())->appendInt(&offerTradeMaxDuration, 1);
    int numBidOrders                = data.bondSnapshot.num_bid_orders;
    ((Vector*)buffer[colNum++].get())->appendInt(&numBidOrders, 1);
    long long numOfferOrders        = data.bondSnapshot.num_offer_orders;
    ((Vector*)buffer[colNum++].get())->appendLong(&numOfferOrders, 1);
    long long lastTradeTime         = data.bondSnapshot.last_trade_time;
    ((Vector*)buffer[colNum++].get())->appendLong(&lastTradeTime, 1);
    char varietyCategory            = data.bondSnapshot.variety_category;
    ((Vector*)buffer[colNum++].get())->appendChar(&varietyCategory, 1);
}

template <>
inline void structReader(vector<ConstantSP>& buffer, timeMDSnapshot& data) {
    int colNum = 0;
    int marketType          = data.snapshot.market_type;
    ((Vector*)buffer[colNum++].get())->appendInt(&marketType, 1);
    string securityCode     = data.snapshot.security_code;
    ((Vector*)buffer[colNum++].get())->appendString(&securityCode, 1);
    long long origTime      = convertTime(data.snapshot.orig_time);
    ((Vector*)buffer[colNum++].get())->appendLong(&origTime, 1);
    string tradingPhaseCode = data.snapshot.trading_phase_code;
    ((Vector*)buffer[colNum++].get())->appendString(&tradingPhaseCode, 1);
    long long preClosePrice = data.snapshot.pre_close_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&preClosePrice, 1);
    long long openPrice     = data.snapshot.open_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&openPrice, 1);
    long long highPrice     = data.snapshot.high_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&highPrice, 1);
    long long lowPrice      = data.snapshot.low_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&lowPrice, 1);
    long long lastPrice     = data.snapshot.last_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&lastPrice, 1);
    long long closePrice    = data.snapshot.close_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&closePrice, 1);

    int bidPriceIndex = 0;
    long long bidPrice1 = data.snapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice1, 1);
    long long bidPrice2 = data.snapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice2, 1);
    long long bidPrice3 = data.snapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice3, 1);
    long long bidPrice4 = data.snapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice4, 1);
    long long bidPrice5 = data.snapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice5, 1);
    long long bidPrice6 = data.snapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice6, 1);
    long long bidPrice7 = data.snapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice7, 1);
    long long bidPrice8 = data.snapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice8, 1);
    long long bidPrice9 = data.snapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice9, 1);
    long long bidPrice10= data.snapshot.bid_price[bidPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidPrice10, 1);

    int bidVolumeIndex = 0;
    long long bidVolume1 = data.snapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume1, 1);
    long long bidVolume2 = data.snapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume2, 1);
    long long bidVolume3 = data.snapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume3, 1);
    long long bidVolume4 = data.snapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume4, 1);
    long long bidVolume5 = data.snapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume5, 1);
    long long bidVolume6 = data.snapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume6, 1);
    long long bidVolume7 = data.snapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume7, 1);
    long long bidVolume8 = data.snapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume8, 1);
    long long bidVolume9 = data.snapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume9, 1);
    long long bidVolume10= data.snapshot.bid_volume[bidVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&bidVolume10, 1);

    int offerPriceIndex = 0;
    long long offerPrice1 = data.snapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice1, 1);
    long long offerPrice2 = data.snapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice2, 1);
    long long offerPrice3 = data.snapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice3, 1);
    long long offerPrice4 = data.snapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice4, 1);
    long long offerPrice5 = data.snapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice5, 1);
    long long offerPrice6 = data.snapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice6, 1);
    long long offerPrice7 = data.snapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice7, 1);
    long long offerPrice8 = data.snapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice8, 1);
    long long offerPrice9 = data.snapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice9, 1);
    long long offerPrice10= data.snapshot.offer_price[offerPriceIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerPrice10, 1);

    int offerVolumeIndex = 0;
    long long offerVolume1  = data.snapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume1, 1);
    long long offerVolume2  = data.snapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume2, 1);
    long long offerVolume3  = data.snapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume3, 1);
    long long offerVolume4  = data.snapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume4, 1);
    long long offerVolume5  = data.snapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume5, 1);
    long long offerVolume6  = data.snapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume6, 1);
    long long offerVolume7  = data.snapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume7, 1);
    long long offerVolume8  = data.snapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume8, 1);
    long long offerVolume9  = data.snapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume9, 1);
    long long offerVolume10 = data.snapshot.offer_volume[offerVolumeIndex++];
    ((Vector*)buffer[colNum++].get())->appendLong(&offerVolume10, 1);

    long long numTrades             = data.snapshot.num_trades;
    ((Vector*)buffer[colNum++].get())->appendLong(&numTrades, 1);
    long long totalVolumeTrade      = data.snapshot.total_volume_trade;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalVolumeTrade, 1);
    long long totalValueTrade       = data.snapshot.total_value_trade;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalValueTrade, 1);
    long long totalBidVolume        = data.snapshot.total_bid_volume;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalBidVolume, 1);
    long long totalOfferVolume      = data.snapshot.total_offer_volume;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalOfferVolume, 1);
    long long weightedAvgBidPrice   = data.snapshot.weighted_avg_bid_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&weightedAvgBidPrice, 1);
    long long weightedAvgOfferPrice = data.snapshot.weighted_avg_offer_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&weightedAvgOfferPrice, 1);
    long long ioPV                  = data.snapshot.IOPV;
    ((Vector*)buffer[colNum++].get())->appendLong(&ioPV, 1);
    long long yieldToMaturity       = data.snapshot.yield_to_maturity;
    ((Vector*)buffer[colNum++].get())->appendLong(&yieldToMaturity, 1);
    long long highLimited           = data.snapshot.high_limited;
    ((Vector*)buffer[colNum++].get())->appendLong(&highLimited, 1);

    long long lowLimited             = data.snapshot.low_limited;
    ((Vector*)buffer[colNum++].get())->appendLong(&lowLimited, 1);
    long long priceEarningRatio1     = data.snapshot.price_earning_ratio1;
    ((Vector*)buffer[colNum++].get())->appendLong(&priceEarningRatio1, 1);
    long long priceEarningRatio2     = data.snapshot.price_earning_ratio2;
    ((Vector*)buffer[colNum++].get())->appendLong(&priceEarningRatio2, 1);
    long long change1                = data.snapshot.change1;
    ((Vector*)buffer[colNum++].get())->appendLong(&change1, 1);
    long long change2                = data.snapshot.change2;
    ((Vector*)buffer[colNum++].get())->appendLong(&change2, 1);
    int channelNo                    = data.snapshot.channel_no;
    ((Vector*)buffer[colNum++].get())->appendInt(&channelNo, 1);
    string mdStreamID                = data.snapshot.md_stream_id;
    ((Vector*)buffer[colNum++].get())->appendString(&mdStreamID, 1);
    string instrumentStatus          = data.snapshot.instrument_status;
    ((Vector*)buffer[colNum++].get())->appendString(&instrumentStatus, 1);
    long long preCloseIOPV           = data.snapshot.pre_close_iopv;
    ((Vector*)buffer[colNum++].get())->appendLong(&preCloseIOPV, 1);
    long long altWeightedAvgBidPrice = data.snapshot.alt_weighted_avg_bid_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&altWeightedAvgBidPrice, 1);

    long long altWeightedAvgOfferPrice = data.snapshot.alt_weighted_avg_offer_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&altWeightedAvgOfferPrice, 1);
    long long etfBuyNumber             = data.snapshot.etf_buy_number;
    ((Vector*)buffer[colNum++].get())->appendLong(&etfBuyNumber, 1);
    long long etfBuyAmount             = data.snapshot.etf_buy_amount;
    ((Vector*)buffer[colNum++].get())->appendLong(&etfBuyAmount, 1);
    long long etfBuyMoney              = data.snapshot.etf_buy_money;
    ((Vector*)buffer[colNum++].get())->appendLong(&etfBuyMoney, 1);
    long long etfSellNumber            = data.snapshot.etf_sell_number;
    ((Vector*)buffer[colNum++].get())->appendLong(&etfSellNumber, 1);
    long long etfSellAmount            = data.snapshot.etf_sell_amount;
    ((Vector*)buffer[colNum++].get())->appendLong(&etfSellAmount, 1);
    long long etfSellMoney             = data.snapshot.etf_sell_money;
    ((Vector*)buffer[colNum++].get())->appendLong(&etfSellMoney, 1);
    long long totalWarrantExecVolume   = data.snapshot.total_warrant_exec_volume;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalWarrantExecVolume, 1);
    long long warLowerPrice            = data.snapshot.war_lower_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&warLowerPrice, 1);

    long long warUpperPrice       = data.snapshot.war_upper_price;
    ((Vector*)buffer[colNum++].get())->appendLong(&warUpperPrice, 1);
    long long withdrawBuyNumber   = data.snapshot.withdraw_buy_number;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawBuyNumber, 1);
    long long withdrawBuyAmount   = data.snapshot.withdraw_buy_amount;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawBuyAmount, 1);
    long long withdrawBuyMoney    = data.snapshot.withdraw_buy_money;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawBuyMoney, 1);
    long long withdrawSellNumber  = data.snapshot.withdraw_sell_number;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawSellNumber, 1);
    long long withdrawSellAmount  = data.snapshot.withdraw_sell_amount;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawSellAmount, 1);
    long long withdrawSellMoney   = data.snapshot.withdraw_sell_money;
    ((Vector*)buffer[colNum++].get())->appendLong(&withdrawSellMoney, 1);
    long long totalBidNumber      = data.snapshot.total_bid_number;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalBidNumber, 1);
    long long totalOfferNumber    = data.snapshot.total_offer_number;
    ((Vector*)buffer[colNum++].get())->appendLong(&totalOfferNumber, 1);
    int bidTradeMaxDuration       = data.snapshot.bid_trade_max_duration;
    ((Vector*)buffer[colNum++].get())->appendInt(&bidTradeMaxDuration, 1);

    int offerTradeMaxDuration       = data.snapshot.offer_trade_max_duration;
    ((Vector*)buffer[colNum++].get())->appendInt(&offerTradeMaxDuration, 1);
    int numBidOrders                = data.snapshot.num_bid_orders;
    ((Vector*)buffer[colNum++].get())->appendInt(&numBidOrders, 1);
    long long numOfferOrders        = data.snapshot.num_offer_orders;
    ((Vector*)buffer[colNum++].get())->appendLong(&numOfferOrders, 1);
    long long lastTradeTime         = data.snapshot.last_trade_time;
    ((Vector*)buffer[colNum++].get())->appendLong(&lastTradeTime, 1);
    char varietyCategory            = data.snapshot.variety_category;
    ((Vector*)buffer[colNum++].get())->appendChar(&varietyCategory, 1);
}

template <typename T>
inline int getOriginTime(T& data) {
    return 0;
}

template <>
inline int getOriginTime(timeMDSnapshot& data) {
    return data.snapshot.orig_time;
}

template <typename T>
inline int getChannelNo(T& data) {
    return 0;
}

template <>
inline int getChannelNo(timeMDSnapshot& data) {
    return data.snapshot.channel_no;
}

template <typename T>
inline int getMarketType(T& data) {
    return 0;
}

template <>
inline int getMarketType(timeMDSnapshot& data) {
    return data.snapshot.market_type;
}

template <>
inline int getMarketType(timeMDTickOrder& data) {
    return data.order.market_type;
}

template <>
inline int getMarketType(timeMDTickExecution& data) {
    return data.execution.market_type;
}

template <>
inline int getMarketType(timeMDBondSnapshot& data) {
    return data.bondSnapshot.market_type;
}

template <>
inline int getMarketType(timeMDBondTickOrder& data) {
    return data.bondOrder.market_type;
}

template <>
inline int getMarketType(timeMDBondTickExecution& data) {
    return data.bondExecution.market_type;
}

template <>
inline int getMarketType(timeMDIndexSnapshot& data) {
    return data.indexSnapshot.market_type;
}

template <>
inline int getMarketType(timeMDOrderQueue& data) {
    return data.orderQueue.market_type;
}

template <typename DataStruct, typename Meta>
void AMDSpiImp::genericAMDHelper(
    DataStruct* data,
    uint32_t cnt,
    TableSP& insertedTable,
    FunctionDefSP transform,
    bool& flag,
    vector<ConstantSP>& buffer,
    const Meta& meta,
    const string& typeStr,
    AMDDataType datatype
) noexcept {
    try{
    if(cnt == 0) return;
    long long startTime = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
    vector<long long> reachTimeVec;
    if(outputElapsedFlag_) { reachTimeVec.reserve(cnt); }

    for(unsigned int i = 0; i < buffer.size(); ++i) {
        ((Vector*)(buffer[i].get()))->clear();
    }

    for (uint32_t i = 0; i < cnt; ++i) {
        if(!flag) { continue; }
        structReader<DataStruct>(buffer, data[i]);

        if (receivedTimeFlag_) {
            long long reachTime = data[i].reachTime;
            ((Vector*)buffer[meta.colNames_.size()].get())->appendLong(&reachTime, 1);
        }

        int marketType = getMarketType<DataStruct>(data[i]);
        if(dailyIndexFlag_){
            int dailyIndex = INT_MIN;
            if(!getDailyIndex(dailyIndex, dailyIndex_, sizeof(dailyIndex_), marketType,
                datatype, getChannelNo(data[i]), convertTime(getOriginTime(data[i]))) ) {
                LOG_ERR("[PLUGIN::AMDQUOTE]: getDailyIndex failed. ");
                return;
            }
            ((Vector*)buffer[meta.colNames_.size()+1].get())->appendInt(&dailyIndex, 1);
        }
        reachTimeVec.push_back(data[i].reachTime);
    }
    if(flag) {
        vector<ConstantSP> cols;
        for(int i = 0; i < meta.colNames_.size(); ++i) {
            cols.push_back(buffer[i]);
        }
        vector<string> colNames = meta.colNames_;
        if(receivedTimeFlag_) {
            colNames.push_back("receivedTime");
            cols.push_back(buffer[colNames.size()-1]);
        }
        if(dailyIndexFlag_) {
            colNames.push_back("dailyIndex");
            cols.push_back(buffer[colNames.size()-1]);
        }
        if(outputElapsedFlag_) {
            colNames.push_back("perPenetrationTime");
            long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
            for(int i = 0; i < cols[0]->size(); ++i) {
                long long gap = time - reachTimeVec[i];
                ((Vector*)buffer[colNames.size()-1].get())->appendLong(&gap, 1);
            }
            cols.push_back(buffer[colNames.size()-1]);
        }

        TableSP data = Util::createTable(colNames, cols);
        vector<ConstantSP> args = {data};
        try{
            if(!transform.isNull())
                data = transform->call(session_->getHeap().get(), args);
        } catch (exception &e){
            throw RuntimeException("[PLUGIN::AMDQUOTE] call transform error " + string(e.what()));
        }

        if(insertedTable.isNull())
            throw RuntimeException("[PLUGIN::AMDQUOTE] insertedTable is null");
        if(insertedTable ->columns() != data->columns())
            throw RuntimeException("[PLUGIN::AMDQUOTE] The number of columns in the table to be inserted "
                + string("must be the same as the number of columns in the original table."));

        INDEX rows;
        string errMsg;
        vector<ConstantSP> colData(data->columns());
        for(INDEX i = 0; i < data->columns(); ++i) {
            colData[i] = data->getColumn(i);
            if (colData[i]->getType() != insertedTable->getColumnType(i)) {
            throw RuntimeException("[PLUGIN::AMDQUOTE] The type of column " + std::to_string(i) +
                " does not match. Expected: " + Util::getDataTypeString(insertedTable->getColumnType(i)) +
                ", Actual: " + Util::getDataTypeString(colData[i]->getType()) + ".");
            }
        }
        LockGuard<Mutex> _(insertedTable->getLock());
        insertedTable->append(colData, rows, errMsg);
        if(errMsg != "") {
            LOG_ERR("[PLUGIN::AMDQUOTE]: " + typeStr + " data append failed, ", errMsg);
            return;
        }

        for(unsigned int i = 0; i < buffer.size(); ++i) {
            ((Vector*)(buffer[i].get()))->clear();
        }

        if (latencyFlag_) {
            long long diff = Util::toLocalNanoTimestamp(Util::getNanoEpochTime() - startTime);
            latencyLog(datatype, startTime, cnt, diff);
        }
    }
    }
    catch(exception &exception){
        string errMsg = exception.what();
        if(errMsg.find("[PLUGIN::AMDQUOTE]") == string::npos) {
            LOG_ERR("[PLUGIN::AMDQUOTE]: ", errMsg);
        } else {
            LOG_ERR(errMsg);
        }
    }
}

void AMDSpiImp::OnMDSnapshotHelper(timeMDSnapshot* snapshot, uint32_t cnt) {
    genericAMDHelper<timeMDSnapshot, AmdSnapshotTableMeta>(
        snapshot,
        cnt,
        snapshotData_,
        snapshotTransform_,
        snapshotFlag_,
        snapshotBuffer_,
        snapshotDataTableMeta_,
        "snapshot",
        AMD_SNAPSHOT);
}
void AMDSpiImp::OnMDFundSnapshotHelper(timeMDSnapshot* snapshot, uint32_t cnt) {
    genericAMDHelper<timeMDSnapshot, AmdSnapshotTableMeta>(
        snapshot,
        cnt,
        fundSnapshotData_,
        fundSnapshotTransform_,
        fundSnapshotFlag_,
        fundSnapshotBuffer_,
        snapshotDataTableMeta_,
        "fundSnapshot",
        AMD_FUND_SNAPSHOT);
}

// 接受并处理快照数据(债券）
void AMDSpiImp::OnMDBondSnapshotHelper(timeMDBondSnapshot* snapshot, uint32_t cnt) {
    genericAMDHelper<timeMDBondSnapshot, AmdSnapshotTableMeta>(
        snapshot,
        cnt,
        bondSnapshotData_,
        bondSnapshotTransform_,
        bondSnapshotFlag_,
        bondSnapshotBuffer_,
        snapshotDataTableMeta_,
        "bondSnapshot",
        AMD_BOND_SNAPSHOT);
}

void AMDSpiImp::OnMDTickOrderHelper(timeMDTickOrder* ticks, uint32_t cnt) {
    genericAMDHelper<timeMDTickOrder, AmdOrderTableMeta>(
        ticks,
        cnt,
        orderData_,
        orderTransform_,
        orderFlag_,
        orderBuffer_,
        orderTableMeta_,
        "order",
        AMD_ORDER);
}

void AMDSpiImp::OnMDTickFundOrderHelper(timeMDTickOrder* ticks, uint32_t cnt) {
    genericAMDHelper<timeMDTickOrder, AmdOrderTableMeta>(
        ticks,
        cnt,
        fundOrderData_,
        fundOrderTransform_,
        fundOrderFlag_,
        fundOrderBuffer_,
        orderTableMeta_,
        "order",
        AMD_FUND_ORDER);
}

// 接受并处理逐笔委托数据（债券）
void AMDSpiImp::OnMDBondTickOrderHelper(timeMDBondTickOrder* ticks, uint32_t cnt) {
    genericAMDHelper<timeMDBondTickOrder, AmdOrderTableMeta>(
        ticks,
        cnt,
        bondOrderData_,
        bondOrderTransform_,
        bondOrderFlag_,
        bondOrderBuffer_,
        orderTableMeta_,
        "bondOrder",
        AMD_BOND_ORDER);
}

void AMDSpiImp::OnMDTickExecutionHelper(timeMDTickExecution* tick, uint32_t cnt) {
    genericAMDHelper<timeMDTickExecution, AmdExecutionTableMeta>(
        tick,
        cnt,
        executionData_,
        executionTransform_,
        executionFlag_,
        executionBuffer_,
        executionTableMeta_,
        "execution",
        AMD_EXECUTION);
}

void AMDSpiImp::OnMDTickFundExecutionHelper(timeMDTickExecution* tick, uint32_t cnt)  {
    genericAMDHelper<timeMDTickExecution, AmdExecutionTableMeta>(
        tick,
        cnt,
        fundExecutionData_,
        fundExecutionTransform_,
        fundExecutionFlag_,
        fundExecutionBuffer_,
        executionTableMeta_,
        "fundExecution",
        AMD_FUND_EXECUTION);

}

void AMDSpiImp::OnMDBondTickExecutionHelper(timeMDBondTickExecution* tick, uint32_t cnt) {
    genericAMDHelper<timeMDBondTickExecution, AmdExecutionTableMeta>(
        tick,
        cnt,
        bondExecutionData_,
        bondExecutionTransform_,
        bondExecutionFlag_,
        bondExecutionBuffer_,
        executionTableMeta_,
        "bondExecution",
        AMD_BOND_EXECUTION);
}

void AMDSpiImp::OnMDIndexSnapshotHelper(timeMDIndexSnapshot* index, uint32_t cnt)  {
    genericAMDHelper<timeMDIndexSnapshot, AmdIndexTableMeta>(
        index,
        cnt,
        indexData_,
        indexTransform_,
        indexFlag_,
        indexBuffer_,
        indexTableMeta_,
        "index",
        AMD_INDEX);
}

// 接受并处理委托队列数据
void AMDSpiImp::OnMDOrderQueueHelper(timeMDOrderQueue* queue, uint32_t cnt) {
    genericAMDHelper<timeMDOrderQueue, AmdOrderQueueTableMeta>(
        queue,
        cnt,
        orderQueueData_,
        orderQueueTransform_,
        orderQueueFlag_,
        orderQueueBuffer_,
        orderQueueMeta_,
        "orderQueue",
        AMD_ORDER_QUEUE);
}

void AMDSpiImp::pushSnashotData(amd::ama::MDSnapshot* snapshot, uint32_t cnt, long long time){
    for(uint32_t i = 0; i < cnt; ++i){
        auto data = timeMDSnapshot{time, snapshot[i]};
        if(snapshot[i].variety_category == 1) {
            snapshotBoundQueue_->blockingPush(data);
        } else {
            fundSnapshotBoundQueue_->blockingPush(data);
        }
    }
}


void AMDSpiImp::pushOrderData(amd::ama::MDTickOrder* ticks, uint32_t cnt, long long time){
    for(uint32_t i = 0; i < cnt; ++i){
        auto data = timeMDTickOrder{time, ticks[i]};
        if(ticks[i].variety_category == 1) {
            orderBoundQueue_->blockingPush(data);
        } else{
            fundOrderBoundQueue_->blockingPush(data);
        }
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
        if(ticks[i].variety_category == 1) {
            executionBoundQueue_->blockingPush(data);
        } else {
            fundExecutionBoundQueue_->blockingPush(data);
        }
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
    static LocklessHashmap<string, int> errorMap;
    string codeString = amd::ama::Tools::GetEventCodeString(code);
    // LockGuard<Mutex> amdLock_(&AmdQuote::amdMutex_);
    if(codeString.find("Failed") != string::npos){
        if(ERROR_LOG) {
            LOG_ERR("[PLUGIN::AMDQUOTE]: AMA event: " + codeString);
            LOG_INFO("[PLUGIN::AMDQUOTE] AMA event: ", std::string(event_msg));
        } else {
            int val = 1;
            if(errorMap.find(event_msg, val) == 0){
                errorMap.insert(event_msg, val);
                LOG_ERR("[PLUGIN::AMDQUOTE]: AMA event: " + codeString);
                LOG_INFO("[PLUGIN::AMDQUOTE] AMA event: ", std::string(event_msg));
            }
        }
    }else{
        codeString.clear();
        LOG_INFO("[PLUGIN::AMDQUOTE] AMA event: ", std::string(event_msg));
    }

}