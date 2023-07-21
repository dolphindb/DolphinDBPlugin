#include "amdQuote.h"
#include "amdQuoteImp.h"
#include "amdSpiImp.h"
#include <iostream>
#include <string>

inline long long convertToAMDTime(long long dolphinTime, int days){
    int year;
    int month;
    int day;
    Util::parseDate(days, year, month, day);
    dolphinTime = dolphinTime % (24 * 60 * 60 * 1000);
    int milliSecond = dolphinTime % 1000;
    dolphinTime = dolphinTime / 1000;
    int second = dolphinTime % 60;
    dolphinTime = dolphinTime / 60;
    int minute = dolphinTime % 60;
    dolphinTime = dolphinTime / 60;
    int hour = dolphinTime % 60;
    return (((((((long long)year * 100) + month) * 100 + day) * 100 + hour) * 100 + minute) * 100 + second) * 1000 + milliSecond;
}

unordered_map<int, string> timeColumnNameForType = {
    {AMD_SNAPSHOT, "origTime"},
    {AMD_FUND_SNAPSHOT, "origTime"},
    {AMD_BOND_SNAPSHOT, "origTime"},
    {AMD_EXECUTION, "execTime"},
    {AMD_FUND_EXECUTION, "execTime"},
    {AMD_BOND_EXECUTION, "execTime"},
    {AMD_ORDER, "orderTime"},
    {AMD_FUND_ORDER, "orderTime"},
    {AMD_BOND_ORDER, "orderTime"},
    {AMD_INDEX, "origTime"},
    {AMD_ORDER_QUEUE, "orderTime"}
};

unordered_map<string, AMDDataType> NAME_TYPE = {
    {"snapshot", AMD_SNAPSHOT},
    {"fundSnapshot", AMD_FUND_SNAPSHOT},
    {"bondSnapshot", AMD_BOND_SNAPSHOT},
    {"execution", AMD_EXECUTION},
    {"fundExecution", AMD_FUND_EXECUTION},
    {"bondExecution", AMD_BOND_EXECUTION},
    {"order", AMD_ORDER},
    {"fundOrder", AMD_FUND_ORDER},
    {"bondOrder", AMD_BOND_ORDER},
    {"index", AMD_INDEX},
    {"orderQueue", AMD_ORDER_QUEUE}
};


extern "C" ConstantSP testAmdData(Heap *heap, vector<ConstantSP> &arguments){
    if(STOP_TEST) {
        return new Void();
    }
    // LOG_ERR("[PLUGIN::AMDQUOTE::testAmdData]: use testAmdData to append data");
    if(arguments[0]->getType() != DT_STRING || arguments[0]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, "data type must be a string");
    if(arguments[1]->getForm() != DF_TABLE)
        throw IllegalArgumentException(__FUNCTION__, "data must be a table");
    int cnt = 1;
    if(arguments.size() > 2){
        if(arguments[2]->getType() != DT_INT || arguments[2]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, "cnt must be a int");
        cnt = arguments[2]->getInt();
    }
    string dataType = arguments[0]->getString();
    TableSP data = arguments[1];

    AMDDataType amdDataType;
    string timeColumnName;
    if(NAME_TYPE.count(dataType) != 0){
        amdDataType = NAME_TYPE[dataType];
    }
    else
        throw IllegalArgumentException(__FUNCTION__, "error dataType: " + dataType);
    if(timeColumnNameForType.count(amdDataType) != 0){
        timeColumnName = timeColumnNameForType[amdDataType];
    }
    else
        throw IllegalArgumentException(__FUNCTION__, "error dataType: " + std::to_string(amdDataType));

    if(!data->contain(timeColumnName))
        throw IllegalArgumentException(__FUNCTION__, "data must contain a column named " + string(timeColumnName));
    if(!data->contain("channelNo"))
        throw IllegalArgumentException(__FUNCTION__, "data must contain a column named channelNo");
    if(!data->contain("market"))
        throw IllegalArgumentException(__FUNCTION__, "data must contain a column named market");

    vector<string> colNames;
    vector<ConstantSP> cols;
    size_t colsNum = data->columns();
    for(size_t i = 0; i < colsNum; ++i){
        colNames.push_back(data->getColumnName(i));
        cols.push_back(data->getColumn(i));
    }
    ConstantSP timeStampColumn = data->getColumn(timeColumnName);
    vector<ConstantSP> args = {timeStampColumn};
    VectorSP days = heap->currentSession()->getFunctionDef("date")->call(heap, args);
    cols.push_back(days);
    colNames.push_back("days");
    int rows = data->rows();
    data = Util::createTable(colNames, cols);

    ResultSet resultSet = data;
    int channelNoIndex = data->getColumnIndex("channelNo");
    int marketIndex = data->getColumnIndex("market");
    int dayIndex = data->getColumnIndex("days");
    int timeIndex = data->getColumnIndex(timeColumnName);
    AMDSpiImp * amdSpi = AmdQuote::getInstance()->getAMDSpi();

    long long time = Util::toLocalNanoTimestamp(Util::getNanoEpochTime());
    if(amdDataType == AMD_SNAPSHOT || amdDataType == AMD_FUND_SNAPSHOT){
        for(int i = 0; i < cnt; ++i){
            if(STOP_TEST) {
                return new Void();
            }
            amd::ama::MDSnapshot* snapshot = new amd::ama::MDSnapshot[rows];
            Defer df([=](){delete[] snapshot;});
            int index = 0;
            resultSet.first();
            while(!resultSet.isAfterLast()){
                snapshot[index].channel_no = resultSet.getInt(channelNoIndex);
                snapshot[index].market_type = resultSet.getInt(marketIndex);
                snapshot[index].variety_category = amdDataType == AMD_SNAPSHOT ? 1 : 2;
                snapshot[index].orig_time = convertToAMDTime(resultSet.getLong(timeIndex), resultSet.getInt(dayIndex));
                snapshot[index].security_code[0] = '\0';
                snapshot[index].trading_phase_code[0] = '\0';
                snapshot[index].md_stream_id[0] = '\0';
                snapshot[index].instrument_status[0] = '\0';
                index++;
                resultSet.next();
            }
            amdSpi->pushSnashotData(snapshot, rows, time);
        }
    }else if(amdDataType == AMD_EXECUTION || amdDataType == AMD_FUND_EXECUTION){
        for(int i = 0; i < cnt; ++i){
            if(STOP_TEST) {
                return new Void();
            }
            amd::ama::MDTickExecution* snapshot = new amd::ama::MDTickExecution[rows];
            Defer df([=](){delete[] snapshot;});
            int index = 0;
            resultSet.first();
            while(!resultSet.isAfterLast()){
                snapshot[index].channel_no = resultSet.getInt(channelNoIndex);
                snapshot[index].market_type = resultSet.getInt(marketIndex);
                snapshot[index].variety_category = amdDataType == AMD_EXECUTION ? 1 : 2;
                snapshot[index].exec_time = convertToAMDTime(resultSet.getLong(timeIndex), resultSet.getInt(dayIndex));
                snapshot[index].security_code[0] = 'a';
                snapshot[index].security_code[1] = '\0';
                snapshot[index].md_stream_id[0] = '\0';
                index++;
                resultSet.next();
            }
            amdSpi->pushExecutionData(snapshot, rows, time);
        }
    }else if(amdDataType == AMD_ORDER || amdDataType == AMD_FUND_ORDER){
        for(int i = 0; i < cnt; ++i){
            if(STOP_TEST) {
                return new Void();
            }
            amd::ama::MDTickOrder* snapshot = new amd::ama::MDTickOrder[rows];
            Defer df([=](){delete[] snapshot;});
            int index = 0;
            resultSet.first();
            while(!resultSet.isAfterLast()){
                snapshot[index].channel_no = resultSet.getInt(channelNoIndex);
                snapshot[index].market_type = resultSet.getInt(marketIndex);
                snapshot[index].variety_category = amdDataType == AMD_ORDER ? 1 : 2;
                snapshot[index].order_time = convertToAMDTime(resultSet.getLong(timeIndex), resultSet.getInt(dayIndex));
                snapshot[index].security_code[0] = 'a';
                snapshot[index].security_code[1] = '\0';
                snapshot[index].md_stream_id[0] = '\0';
                index++;
                resultSet.next();
            }
            amdSpi->pushOrderData(snapshot, rows, time);
        }
    }else if(amdDataType == AMD_BOND_SNAPSHOT){
        for(int i = 0; i < cnt; ++i){
            if(STOP_TEST) {
                return new Void();
            }
            amd::ama::MDBondSnapshot* snapshot = new amd::ama::MDBondSnapshot[rows];
            Defer df([=](){delete[] snapshot;});
            int index = 0;
            resultSet.first();
            while(!resultSet.isAfterLast()){
                snapshot[index].channel_no = resultSet.getInt(channelNoIndex);
                snapshot[index].market_type = resultSet.getInt(marketIndex);
                snapshot[index].variety_category = 3;
                snapshot[index].orig_time = convertToAMDTime(resultSet.getLong(timeIndex), resultSet.getInt(dayIndex));
                snapshot[index].security_code[0] = '\0';
                snapshot[index].trading_phase_code[0] = '\0';
                snapshot[index].md_stream_id[0] = '\0';
                snapshot[index].instrument_status[0] = '\0';
                index++;
                resultSet.next();
            }
            amdSpi->pushBondSnapshotData(snapshot, rows, time);
        }
    }else if(amdDataType == AMD_BOND_EXECUTION){
        for(int i = 0; i < cnt; ++i){
            if(STOP_TEST) {
                return new Void();
            }
            amd::ama::MDBondTickExecution* snapshot = new amd::ama::MDBondTickExecution[rows];
            Defer df([=](){delete[] snapshot;});
            int index = 0;
            resultSet.first();
            while(!resultSet.isAfterLast()){
                snapshot[index].channel_no = resultSet.getInt(channelNoIndex);
                snapshot[index].market_type = resultSet.getInt(marketIndex);
                snapshot[index].variety_category = 3;
                snapshot[index].exec_time = convertToAMDTime(resultSet.getLong(timeIndex), resultSet.getInt(dayIndex));
                snapshot[index].security_code[0] = 'a';
                snapshot[index].security_code[0] = '\0';
                snapshot[index].md_stream_id[0] = '\0';
                index++;
                resultSet.next();
            }
            amdSpi->pushBondExecutionData(snapshot, rows, time);
        }
    }else if(amdDataType == AMD_BOND_ORDER){
        for(int i = 0; i < cnt; ++i){
            if(STOP_TEST) {
                return new Void();
            }
            amd::ama::MDBondTickOrder* snapshot = new amd::ama::MDBondTickOrder[rows];
            Defer df([=](){delete[] snapshot;});
            int index = 0;
            resultSet.first();
            while(!resultSet.isAfterLast()){
                snapshot[index].channel_no = resultSet.getInt(channelNoIndex);
                snapshot[index].market_type = resultSet.getInt(marketIndex);
                snapshot[index].variety_category = 3;
                snapshot[index].order_time = convertToAMDTime(resultSet.getLong(timeIndex), resultSet.getInt(dayIndex));
                snapshot[index].security_code[0] = 'a';
                snapshot[index].security_code[1] = '\0';
                snapshot[index].md_stream_id[0] = '\0';
                index++;
                resultSet.next();
            }
            amdSpi->pushBondOrderData(snapshot, rows, time);
        }
    }else if(amdDataType == AMD_INDEX){
        for(int i = 0; i < cnt; ++i){
            if(STOP_TEST) {
                return new Void();
            }
            amd::ama::MDIndexSnapshot* snapshot = new amd::ama::MDIndexSnapshot[rows];
            Defer df([=](){delete[] snapshot;});
            int index = 0;
            resultSet.first();
            while(!resultSet.isAfterLast()){
                snapshot[index].channel_no = resultSet.getInt(channelNoIndex);
                snapshot[index].market_type = resultSet.getInt(marketIndex);
                snapshot[index].variety_category = amdDataType == AMD_ORDER ? 1 : 2;
                snapshot[index].orig_time = convertToAMDTime(resultSet.getLong(timeIndex), resultSet.getInt(dayIndex));
                snapshot[index].security_code[0] = '\0';
                snapshot[index].md_stream_id[0] = '\0';
                index++;
                resultSet.next();
            }
            amdSpi->pushIndexData(snapshot, rows, time);
        }
    }else if(amdDataType == AMD_ORDER_QUEUE){
        for(int i = 0; i < cnt; ++i){
            if(STOP_TEST) {
                return new Void();
            }
            amd::ama::MDOrderQueue* snapshot = new amd::ama::MDOrderQueue[rows];
            Defer df([=](){delete[] snapshot;});
            int index = 0;
            resultSet.first();
            while(!resultSet.isAfterLast()){
                snapshot[index].channel_no = resultSet.getInt(channelNoIndex);
                snapshot[index].market_type = resultSet.getInt(marketIndex);
                snapshot[index].variety_category = amdDataType == AMD_ORDER ? 1 : 2;
                snapshot[index].order_time = convertToAMDTime(resultSet.getLong(timeIndex), resultSet.getInt(dayIndex));
                snapshot[index].security_code[0] = '\0';
                snapshot[index].md_stream_id[0] = '\0';
                index++;
                resultSet.next();
            }
            amdSpi->pushOrderQueueData(snapshot, rows, time);
        }
    }
    return new Void();
}