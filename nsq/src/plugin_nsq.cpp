#include "plugin_nsq.h"

#include <HSNsqApi.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#include "HSNsqSpiImpl.h"
#include "ScalarImp.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif // _WIN32

#include <fstream>
#include <iostream>
#include <map>
#include <vector>
using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::map;
using std::string;
using std::vector;
void msleep(int msecs) {
#ifdef _WIN32
    Sleep(msecs);
#else
    usleep(msecs * 1000);
#endif // WIN32
}
#define HS_URL "tcp://14.21.47.91:8881"
#define RETRY_TIMES 30

SmartPointer<Session> SESSION = nullptr;
vector<string> TABLE_NAMES(6, "");
vector<bool> SUBSCRIBE_STATUS(6, 0);
vector<Status> STATUS(6);

// CHSNsqApi's destruct function is protected
CHSNsqApi *LP_NSQ_API = NULL;
CHSNsqSpiImpl *LP_NSP_SPI = NULL;

int N_REQUEST_ID = 0;
int I_RETRIES = 0;
int N_COUNT = 0;

CHSNsqReqSecuDepthMarketDataField REQ_FIELD;
CHSNsqReqSecuTransactionRebuildField REQ_TRANS_REBUILD;
CHSNsqReqSecuDepthMarketDataField REQ_FIELD_SUB[1000];

std::vector<string> SH_CODES, SZ_CODES;

bool RECEIVED_TIME_FLAG = false;
bool GET_ALL_FIELD_NAMES_FLAG = false;
bool IS_CONNECTED = false;

void SplitString(string &s, std::vector<std::string> &v, const string &c) {
#if 1
    size_t n = s.find_last_not_of("\r\n");
    if (n != string::npos) {
        s.erase(n + 1, s.size() - n);
    }
    n = s.find_first_not_of("\r\n");
    if (n != string::npos) {
        s.erase(0, n);
    }
#endif
    string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while (string::npos != pos2) {
        v.push_back(s.substr(pos1, pos2 - pos1));
        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if (pos1 != s.length()) {
        v.push_back(s.substr(pos1));
    }
}

vector<string> SNAPSHOT_COLUMN_NAMES{"ExchangeID", "InstrumentID", "LastPrice", "PreClosePrice", "OpenPrice",
                                   "HighPrice", "LowPrice", "ClosePrice", "UpperLimitPrice", "LowerLimitPrice",
                                   "TradeDate", "UpdateTime", "TradeVolume", "TradeBalance", "AveragePrice",
                                   "BidPrice0", "BidPrice1", "BidPrice2", "BidPrice3", "BidPrice4",
                                   "BidPrice5", "BidPrice6", "BidPrice7", "BidPrice8", "BidPrice9",
                                   "AskPrice0", "AskPrice1", "AskPrice2", "AskPrice3", "AskPrice4",
                                   "AskPrice5", "AskPrice6", "AskPrice7", "AskPrice8", "AskPrice9",
                                   "BidVolume0", "BidVolume1", "BidVolume2", "BidVolume3", "BidVolume4",
                                   "BidVolume5", "BidVolume6", "BidVolume7", "BidVolume8", "BidVolume9",
                                   "AskVolume0", "AskVolume1", "AskVolume2", "AskVolume3", "AskVolume4",
                                   "AskVolume5", "AskVolume6", "AskVolume7", "AskVolume8", "AskVolume9",
                                   "TradesNum", "InstrumentTradeStatus", "TotalBidVolume", "TotalAskVolume", "MaBidPrice",
                                   "MaAskPrice", "MaBondBidPrice", "MaBondAskPrice", "YieldToMaturity", "IOPV",
                                   "EtfBuycount", "EtfSellCount", "EtfBuyVolume", "EtfBuyBalance", "EtfSellVolume",
                                   "EtfSellBalance", "TotalWarrantExecVolume", "WarrantLowerPrice", "WarrantUpperPrice", "CancelBuyNum",
                                   "CancelSellNum", "CancelBuyVolume", "CancelSellVolume", "CancelBuyValue", "CancelSellValue",
                                   "TotalBuyNum", "TotalSellNum", "DurationAfterBuy", "DurationAfterSell", "BidOrdersNum",
                                   "AskOrdersNum", "PreIOPV",};

vector<string> TRADE_COLUMN_NAMES{"ExchangeID", "InstrumentID", "TransFlag", "SeqNo",     "ChannelNo",
                                "TradeDate",  "TransactTime", "TrdPrice",  "TrdVolume", "TrdMoney",
                                "TrdBuyNo",   "TrdSellNo",    "TrdBSFlag", "BizIndex",};

vector<string> ORDER_COLUMN_NAMES{"ExchangeID", "InstrumentID", "TransFlag", "SeqNo",     "ChannelNo",
                                "TradeDate",  "TransactTime", "OrdPrice",  "OrdVolume", "OrdSide",
                                "OrdType",    "OrdNo",        "BizIndex",};

vector<string> ADDED_SNAPSHOT_COLUMN_NAMES{"Bid1Count", "MaxBid1Count", "Ask1Count", "MaxAsk1Count",
                                        "Bid1Qty0", "Bid1Qty1", "Bid1Qty2", "Bid1Qty3", "Bid1Qty4",
                                        "Bid1Qty5", "Bid1Qty6", "Bid1Qty7", "Bid1Qty8", "Bid1Qty9",
                                        "Bid1Qty10", "Bid1Qty11", "Bid1Qty12", "Bid1Qty13", "Bid1Qty14",
                                        "Bid1Qty15", "Bid1Qty16", "Bid1Qty17", "Bid1Qty18", "Bid1Qty19",
                                        "Bid1Qty20", "Bid1Qty21", "Bid1Qty22", "Bid1Qty23", "Bid1Qty24",
                                        "Bid1Qty25", "Bid1Qty26", "Bid1Qty27", "Bid1Qty28", "Bid1Qty29",
                                        "Bid1Qty30", "Bid1Qty31", "Bid1Qty32", "Bid1Qty33", "Bid1Qty34",
                                        "Bid1Qty35", "Bid1Qty36", "Bid1Qty37", "Bid1Qty38", "Bid1Qty39",
                                        "Bid1Qty40", "Bid1Qty41", "Bid1Qty42", "Bid1Qty43", "Bid1Qty44",
                                        "Bid1Qty45", "Bid1Qty46", "Bid1Qty47", "Bid1Qty48", "Bid1Qty49",
                                        "Ask1Qty0", "Ask1Qty1", "Ask1Qty2", "Ask1Qty3", "Ask1Qty4",
                                        "Ask1Qty5", "Ask1Qty6", "Ask1Qty7", "Ask1Qty8", "Ask1Qty9",
                                        "Ask1Qty10", "Ask1Qty11", "Ask1Qty12", "Ask1Qty13", "Ask1Qty14",
                                        "Ask1Qty15", "Ask1Qty16", "Ask1Qty17", "Ask1Qty18", "Ask1Qty19",
                                        "Ask1Qty20", "Ask1Qty21", "Ask1Qty22", "Ask1Qty23", "Ask1Qty24",
                                        "Ask1Qty25", "Ask1Qty26", "Ask1Qty27", "Ask1Qty28", "Ask1Qty29",
                                        "Ask1Qty30", "Ask1Qty31", "Ask1Qty32", "Ask1Qty33", "Ask1Qty34",
                                        "Ask1Qty35", "Ask1Qty36", "Ask1Qty37", "Ask1Qty38", "Ask1Qty39",
                                        "Ask1Qty40", "Ask1Qty41", "Ask1Qty42", "Ask1Qty43", "Ask1Qty44",
                                        "Ask1Qty45", "Ask1Qty46", "Ask1Qty47", "Ask1Qty48", "Ask1Qty49",};

vector<string> TRADE_AND_ORDER_MERGED_COLUMN_NAMES{
    "ExchangeID", "InstrumentID", "TransFlag",     "SeqNo",     "ChannelNo",
    "TradeDate",  "TransactTime", "Trade2_Order1", "Price",     "Volume",
    "TrdMoney",   "TrdBuyNo",     "TrdSellNo",     "TrdBSFlag", "BizIndex",
    "OrdSide",    "OrdType",      "OrdNo",         "TrdVolume",
};

vector<DATA_TYPE> SNAPSHOT_TYPES{
    DT_SYMBOL, DT_SYMBOL, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE,
    DT_DATE,   DT_TIME,   DT_LONG,   DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE,
    DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE,
    DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_LONG,   DT_LONG,   DT_LONG,   DT_LONG,   DT_LONG,
    DT_LONG,   DT_LONG,   DT_LONG,   DT_LONG,   DT_LONG,   DT_LONG,   DT_LONG,   DT_LONG,   DT_LONG,   DT_LONG,
    DT_LONG,   DT_LONG,   DT_LONG,   DT_LONG,   DT_LONG,   DT_LONG,   DT_CHAR,   DT_LONG,   DT_LONG,   DT_DOUBLE,
    DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_INT,    DT_INT,    DT_LONG,   DT_DOUBLE, DT_LONG,
    DT_DOUBLE, DT_LONG,   DT_DOUBLE, DT_DOUBLE, DT_INT,    DT_INT,    DT_LONG,   DT_LONG,   DT_DOUBLE, DT_DOUBLE,
    DT_INT,    DT_INT,    DT_INT,    DT_INT,    DT_INT,    DT_INT,    DT_DOUBLE};

vector<DATA_TYPE> TRADE_TYPES{DT_SYMBOL, DT_SYMBOL, DT_INT,    DT_LONG, DT_INT,  DT_DATE, DT_TIME,
                             DT_DOUBLE, DT_LONG,   DT_DOUBLE, DT_LONG, DT_LONG, DT_CHAR, DT_LONG};

vector<DATA_TYPE> ORDER_TYPES{DT_SYMBOL, DT_SYMBOL, DT_INT,  DT_LONG, DT_INT,  DT_DATE, DT_TIME,
                             DT_DOUBLE,  DT_LONG,   DT_CHAR, DT_CHAR, DT_LONG, DT_LONG};

vector<DATA_TYPE> ADDED_SNAPSHOT_TYPES{
    DT_INT, DT_INT, DT_INT, DT_INT,
    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
    DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG, DT_LONG,
};

vector<DATA_TYPE> TRADE_AND_ORDER_MERGED_TYPES{
    DT_SYMBOL, DT_SYMBOL, DT_INT,  DT_LONG,   DT_INT,
    DT_DATE,   DT_TIME,   DT_INT,  DT_DOUBLE, DT_LONG,
    DT_DOUBLE, DT_LONG,   DT_LONG, DT_CHAR,   DT_LONG,
    DT_CHAR,   DT_CHAR,   DT_LONG, DT_LONG
};

TableSP getSnapshotSchema() {
    if (RECEIVED_TIME_FLAG == true && SNAPSHOT_COLUMN_NAMES[SNAPSHOT_COLUMN_NAMES.size() - 1] != "ReceivedTime") {
        SNAPSHOT_COLUMN_NAMES.push_back("ReceivedTime");
        SNAPSHOT_TYPES.push_back(DT_NANOTIMESTAMP);
    }

    if (RECEIVED_TIME_FLAG == false && SNAPSHOT_COLUMN_NAMES[SNAPSHOT_COLUMN_NAMES.size() - 1] == "ReceivedTime") {
        SNAPSHOT_COLUMN_NAMES.pop_back();
        SNAPSHOT_TYPES.pop_back();
    }

    vector<ConstantSP> cols(2);
    vector<string> colNames = SNAPSHOT_COLUMN_NAMES;
    vector<DATA_TYPE> colTypes = SNAPSHOT_TYPES;
    if (GET_ALL_FIELD_NAMES_FLAG) {
        colNames.insert(colNames.end(), ADDED_SNAPSHOT_COLUMN_NAMES.begin(), ADDED_SNAPSHOT_COLUMN_NAMES.end());
        colTypes.insert(colTypes.end(), ADDED_SNAPSHOT_TYPES.begin(), ADDED_SNAPSHOT_TYPES.end());
    }
    cols[0] = Util::createVector(DT_STRING, colNames.size());
    cols[1] = Util::createVector(DT_STRING, colTypes.size());
    for (unsigned int i = 0; i < colNames.size(); i++) {
        cols[0]->setString(i, colNames[i]);
        cols[1]->setString(i, Util::getDataTypeString(colTypes[i]));
    }

    std::vector<string> schemaColNames = {"name", "type"};
    return Util::createTable(schemaColNames, cols);
}

TableSP getTradeSchema() {
    if (RECEIVED_TIME_FLAG == true && TRADE_COLUMN_NAMES.back() != "ReceivedTime") {
        TRADE_COLUMN_NAMES.push_back("ReceivedTime");
        TRADE_TYPES.push_back(DT_NANOTIMESTAMP);
    }

    if (RECEIVED_TIME_FLAG == false && TRADE_COLUMN_NAMES.back() == "ReceivedTime") {
        TRADE_COLUMN_NAMES.pop_back();
        TRADE_TYPES.pop_back();
    }

    if (RECEIVED_TIME_FLAG == true && TRADE_AND_ORDER_MERGED_COLUMN_NAMES.back() != "ReceivedTime") {
        TRADE_AND_ORDER_MERGED_COLUMN_NAMES.push_back("ReceivedTime");
        TRADE_AND_ORDER_MERGED_TYPES.push_back(DT_NANOTIMESTAMP);
    }

    if (RECEIVED_TIME_FLAG == false && TRADE_AND_ORDER_MERGED_COLUMN_NAMES.back() == "ReceivedTime") {
        TRADE_AND_ORDER_MERGED_COLUMN_NAMES.pop_back();
        TRADE_AND_ORDER_MERGED_TYPES.pop_back();
    }

    vector<string> colNames;
    vector<DATA_TYPE> colTypes;
    if (GET_ALL_FIELD_NAMES_FLAG) {
        colNames = TRADE_AND_ORDER_MERGED_COLUMN_NAMES;
        colTypes = TRADE_AND_ORDER_MERGED_TYPES;
    } else {
        colNames = TRADE_COLUMN_NAMES;
        colTypes = TRADE_TYPES;
    }

    vector<ConstantSP> cols(2);
    cols[0] = Util::createVector(DT_STRING, colNames.size());
    cols[1] = Util::createVector(DT_STRING, colTypes.size());
    for (unsigned int i = 0; i < colNames.size(); i++) {
        cols[0]->setString(i, colNames[i]);
        cols[1]->setString(i, Util::getDataTypeString(colTypes[i]));
    }
    std::vector<string> schemaColNames = {"name", "type"};
    TableSP table = Util::createTable(schemaColNames, cols);

    return table;
}

TableSP getOrdersSchema() {
    if (RECEIVED_TIME_FLAG == true && ORDER_COLUMN_NAMES.back() != "ReceivedTime") {
        ORDER_COLUMN_NAMES.push_back("ReceivedTime");
        ORDER_TYPES.push_back(DT_NANOTIMESTAMP);
    }

    if (RECEIVED_TIME_FLAG == false && ORDER_COLUMN_NAMES.back() == "ReceivedTime") {
        ORDER_COLUMN_NAMES.pop_back();
        ORDER_TYPES.pop_back();
    }

    if (RECEIVED_TIME_FLAG == true && TRADE_AND_ORDER_MERGED_COLUMN_NAMES.back() != "ReceivedTime") {
        TRADE_AND_ORDER_MERGED_COLUMN_NAMES.push_back("ReceivedTime");
        TRADE_AND_ORDER_MERGED_TYPES.push_back(DT_NANOTIMESTAMP);
    }

    if (RECEIVED_TIME_FLAG == false && TRADE_AND_ORDER_MERGED_COLUMN_NAMES.back() == "ReceivedTime") {
        TRADE_AND_ORDER_MERGED_COLUMN_NAMES.pop_back();
        TRADE_AND_ORDER_MERGED_TYPES.pop_back();
    }

    vector<string> colNames;
    vector<DATA_TYPE> colTypes;
    if (GET_ALL_FIELD_NAMES_FLAG) {
        colNames = TRADE_AND_ORDER_MERGED_COLUMN_NAMES;
        colTypes = TRADE_AND_ORDER_MERGED_TYPES;
    } else {
        colNames = ORDER_COLUMN_NAMES;
        colTypes = ORDER_TYPES;
    }

    vector<ConstantSP> cols(2);
    cols[0] = Util::createVector(DT_STRING, colNames.size());
    cols[1] = Util::createVector(DT_STRING, colTypes.size());
    for (unsigned int i = 0; i < colNames.size(); i++) {
        cols[0]->setString(i, colNames[i]);
        cols[1]->setString(i, Util::getDataTypeString(colTypes[i]));
    }
    std::vector<string> schemaColNames = {"name", "type"};
    TableSP table = Util::createTable(schemaColNames, cols);

    return table;
}

ConstantSP nsqClose(Heap *heap, vector<ConstantSP> &arguments) {
    LockGuard<Mutex> lockGuard(&NSQ_MUTEX);
    SESSION = heap->currentSession()->copy();
    SESSION->setUser(heap->currentSession()->getUser());
    if (LP_NSQ_API == NULL) {
        std::string errMsg = " Failed to close(). There is no connection to close.";
        throw RuntimeException(NSQ_PREFIX + errMsg);
    }

    // unsubscribe all
    LP_NSQ_API->ReleaseApi();
    LP_NSQ_API = NULL;
    if (LP_NSP_SPI == NULL) {
        std::string errMsg = " Failed to close(). There is no connection to close.";
        throw RuntimeException(NSQ_PREFIX + errMsg);
    }
    delete LP_NSP_SPI;
    LP_NSP_SPI = NULL;
    LOG_INFO(NSQ_PREFIX + " close() success");

    // set status to 0
    for (int i = 0; i < 6; i++) {
        SUBSCRIBE_STATUS[i] = 0;

        STATUS[i].processedMsgCount = 0;
        STATUS[i].failedMsgCount = 0;
        STATUS[i].lastErrMsg = "";
        STATUS[i].lastFailedTimestamp = Timestamp(LLONG_MIN);
    }

    IS_CONNECTED = false;
    RECEIVED_TIME_FLAG = false;
    GET_ALL_FIELD_NAMES_FLAG = false;
    return new Void();
}

ConstantSP nsqConnect(Heap *heap, vector<ConstantSP> &arguments) {
    LockGuard<Mutex> lockGuard(&NSQ_MUTEX);
    bool success = false;
    if (LP_NSP_SPI != NULL){
        std::string errMsg = " You are already connected. To reconnect, please execute close() and try again.";
        throw RuntimeException(NSQ_PREFIX + errMsg);
    }
    Defer df1([&](){
        if(!success && LP_NSP_SPI != NULL){
            for (int i = 0; i < 6; i++) {
                SUBSCRIBE_STATUS[i] = 0;

                STATUS[i].processedMsgCount = 0;
                STATUS[i].failedMsgCount = 0;
                STATUS[i].lastErrMsg = "";
                STATUS[i].lastFailedTimestamp = Timestamp(LLONG_MIN);
            }
            // unsubscribe all
            if (LP_NSQ_API != NULL) {
                LP_NSQ_API->ReleaseApi();
                // free(lpNsqApi);
                LP_NSQ_API = NULL;
            }
            delete LP_NSP_SPI;
            LP_NSP_SPI = NULL;
        }
    });
    SESSION = heap->currentSession()->copy();
    SESSION->setUser(heap->currentSession()->getUser());

    if (arguments.size() > 1) {
        if (arguments[1]->getForm() != DF_DICTIONARY) {
            throw IllegalArgumentException(__FUNCTION__, "options must be a dictionary");
        }

        DictionarySP options = arguments[1];
        VectorSP keys = options->keys();
        for(int i = 0; i < options->size(); ++i) {
            ConstantSP key = keys->get(i);
            if(key->getType() != DT_STRING)
                throw IllegalArgumentException(__FUNCTION__, "key of options must be string");
            std::string str = key->getString();
            if(str != "receivedTime" && str != "getAllFieldNames")
                throw IllegalArgumentException(__FUNCTION__, "key of options must be 'receivedTime' or 'getAllFieldNames'");

            if (str == "receivedTime") {
                ConstantSP value = options->getMember("receivedTime");
                if (value->getType() != DT_BOOL) {
                    throw IllegalArgumentException(__FUNCTION__, "value of 'receivedTime' must be boolean");
                }
                RECEIVED_TIME_FLAG = value->getBool();
            }

            if (str == "getAllFieldNames") {
                ConstantSP value = options->getMember("getAllFieldNames");
                if (value->getType() != DT_BOOL) {
                    throw IllegalArgumentException(__FUNCTION__, "value of 'getAllFieldNames' must be boolean");
                }
                GET_ALL_FIELD_NAMES_FLAG = value->getBool();
            }
        }
    } else {
        RECEIVED_TIME_FLAG = false;
        GET_ALL_FIELD_NAMES_FLAG = false;
    }

    // lpNsqApi = NewNsqApi("./log/");
    std::string configPath = arguments[0]->getString();
    if (!Util::exists(configPath)) {
        std::string errMsg = " Initialization failed. Please check the config file path and the configuration.";
        throw RuntimeException(NSQ_PREFIX + errMsg);
    }
    LP_NSQ_API = NewNsqApiExt("./log", configPath.c_str());

    if (LP_NSQ_API == nullptr) {
        std::string errMsg = " lpNsqApi is uninitialized";
        throw RuntimeException(NSQ_PREFIX + errMsg);
    }

    LP_NSP_SPI = new CHSNsqSpiImpl(LP_NSQ_API);
    LP_NSQ_API->RegisterSpi(LP_NSP_SPI);

    // Note: The RegisterFront interface is not useful in NSQ,
    // which configures the market site and disaster recovery address through the configuration file.
    // This interface is retained for compatibility with the HS Brokerage Counter and the call is invalid
    LP_NSQ_API->RegisterFront(HS_URL);

    // Note: The Init interface needs to be called, the interface parameters in NSQ can be blank,
    // and the initialization parameters are read from the configuration file.
    // To maintain compatibility with HS brokerage counters
    // int iRet = lpNsqApi->Init("license_3rd.dat");
    int iRet = LP_NSQ_API->Init("license_3rd.dat");
    if (iRet != 0) {
        std::string errMsg = " Initialization failed. Please check the config file path and the configuration.";
        throw RuntimeException(NSQ_PREFIX + errMsg);
    }

    // if TCP timeout, change to backup ip
    I_RETRIES = RETRY_TIMES;
    if (!LP_NSP_SPI->GetConnectStatus() && (I_RETRIES--) > 0) {
        msleep(100);
    }
    if(!LP_NSP_SPI->GetConnectStatus()) {
        std::string errMsg = " Failed to connect to server. Please check the config file path and the configuration.";
        throw RuntimeException(NSQ_PREFIX + errMsg);
    }

    CHSNsqReqUserLoginField reqLoginField;
    memset(&reqLoginField, 0, sizeof(reqLoginField));

    CHSNsqReqSecuTransactionRebuildField reqTransRebuild;
    memset(&reqTransRebuild, 0, sizeof(reqTransRebuild));

    // get username, password, code
    // readUserFile(reqLoginField, reqTransRebuild);

    iRet = LP_NSQ_API->ReqUserLogin(&reqLoginField, 0);
    if (iRet != 0) {
        std::string errMsg =
            " login failed: iRet" + std::to_string(iRet) + ", error: " + std::string(LP_NSQ_API->GetApiErrorMsg(iRet));
        throw RuntimeException(NSQ_PREFIX + errMsg);
    }

    I_RETRIES = RETRY_TIMES;
    while (!LP_NSP_SPI->GetLoginStatus() && (I_RETRIES--) > 0) {
        msleep(100);
    }

    if (!LP_NSP_SPI->GetLoginStatus()) {
        std::string errMsg = " login failed, error exit! error: " + std::string(LP_NSQ_API->GetApiErrorMsg(0));
        throw RuntimeException(NSQ_PREFIX + errMsg);
    }

    IS_CONNECTED = true;
    success = true;

    return new Void();
}

bool checkColumns(const TableSP& tbl, int type) {
    vector<DATA_TYPE> colTypes;
    if (type == 0) { // snapshot
        colTypes = SNAPSHOT_TYPES;
        if (GET_ALL_FIELD_NAMES_FLAG) {
            colTypes.insert(colTypes.end(), ADDED_SNAPSHOT_TYPES.begin(), ADDED_SNAPSHOT_TYPES.end());
        }
    } else if (type == 1) {
        if (GET_ALL_FIELD_NAMES_FLAG) {
            colTypes = TRADE_AND_ORDER_MERGED_TYPES;
        } else {
            colTypes = TRADE_TYPES;
        }
    } else if (type == 2) {
        if (GET_ALL_FIELD_NAMES_FLAG) {
            colTypes = TRADE_AND_ORDER_MERGED_TYPES;
        } else {
            colTypes = ORDER_TYPES;
        }
    } else {
        return false;
    }
    if (tbl->columns() != (INDEX)colTypes.size()) {
        return false;
    }
	int numColumns = tbl->columns();
    for (int i = 0; i < numColumns; i++) {
		DATA_TYPE dt = tbl->getColumnType(i);
        if (dt != colTypes[i]) {
            // for compatibility
            // STRING：SYMBOL
            // DOUBlE：FLOAT
            // LONG：INT
            bool f = false;
            if (dt == DT_SYMBOL && colTypes[i] == DT_STRING) {
                f = true;
            } else if (dt == DT_FLOAT && colTypes[i] == DT_DOUBLE) {
                f = true;
            } else if (dt == DT_INT && colTypes[i] == DT_LONG) {
                f = true;
            }
            if (f)
                continue;
            return false;
        }
    }
    return true;
}

int setReqFieldSub(int flag) {
    int nCount = 0;
    if (flag == 0) { // SH
        memcpy(REQ_FIELD_SUB[0].ExchangeID, HS_EI_SSE,
               sizeof(HS_EI_SSE)); // nCount=0, subscribe all
        for (size_t i = 0; i < SH_CODES.size(); i++) {
            if (SH_CODES[i].length() == 6) {
                memcpy(REQ_FIELD_SUB[nCount].ExchangeID, HS_EI_SSE, sizeof(HS_EI_SSE));
                memcpy(REQ_FIELD_SUB[nCount++].InstrumentID, SH_CODES[i].c_str(), sizeof(SH_CODES[i]));
            }
        }
    } else if (flag == 1) { // SZ
        memcpy(REQ_FIELD_SUB[0].ExchangeID, HS_EI_SZSE,
               sizeof(HS_EI_SZSE)); // nCount=0, subscribe all
        for (size_t i = 0; i < SZ_CODES.size(); i++) {
            if (SZ_CODES[i].length() == 6) {
                memcpy(REQ_FIELD_SUB[nCount].ExchangeID, HS_EI_SZSE, sizeof(HS_EI_SZSE));
                memcpy(REQ_FIELD_SUB[nCount++].InstrumentID, SZ_CODES[i].c_str(), sizeof(SZ_CODES[i]));
            }
        }
    }

    return nCount;
}

ConstantSP subscribe(Heap *heap, vector<ConstantSP> &arguments) {
    LockGuard<Mutex> lockGuard(&NSQ_MUTEX);
    SESSION = heap->currentSession()->copy();
    SESSION->setUser(heap->currentSession()->getUser());
    if (LP_NSQ_API == NULL || !LP_NSP_SPI->GetConnectStatus()) {
        std::string errMsg = " API is not initialized. Please check whether the connection is set up via connect().";
        throw RuntimeException(NSQ_PREFIX + errMsg);
    }
    if (arguments[2]->getForm() != DF_TABLE) {
        std::string errMsg = " The third parameter `streamTable` must be a shared stream table.";
        throw RuntimeException(NSQ_PREFIX + errMsg);
    } else {
        Table *tblSP = dynamic_cast<Table *>(arguments[2].get());
        if (tblSP->getTableType() != REALTIMETBL || !tblSP->isSharedTable()) {
            std::string errMsg = " The third parameter `streamTable` must be a shared stream table.";
            throw RuntimeException(NSQ_PREFIX + errMsg);
        }
    }
    if (arguments[1]->getString() == "sh") {
        // SH
        N_COUNT = setReqFieldSub(0);
    } else if (arguments[1]->getString() == "sz") {
        N_COUNT = setReqFieldSub(1);
    } else {
        std::string errMsg = " The second parameter `location` must be `sh` or `sz`.";
        throw RuntimeException(NSQ_PREFIX + errMsg);
    }
    int table_index = -1;
    if (arguments[0]->getString() == "snapshot") {
        if (!checkColumns(arguments[2], 0)) {
            std::string errMsg = " Subscription failed. Please check if the schema of “streamTable” is correct.";
            throw RuntimeException(NSQ_PREFIX + errMsg);
        }
        if (arguments[1]->getString() == "sh") {
            table_index = 0;
        } else if (arguments[1]->getString() == "sz") {
            table_index = 1;
        }
    } else if (arguments[0]->getString() == "trade") {
        if (!checkColumns(arguments[2], 1)) {
            std::string errMsg = " Subscription failed. Please check if the schema of “streamTable” is correct.";
            throw RuntimeException(NSQ_PREFIX + errMsg);
        }
        if (arguments[1]->getString() == "sh") {
            table_index = 2;
        } else if (arguments[1]->getString() == "sz") {
            table_index = 3;
        }
    } else if (arguments[0]->getString() == "orders") {
        if (!checkColumns(arguments[2], 2)) {
            std::string errMsg = " Subscription failed. Please check if the schema of “streamTable” is correct.";
            throw RuntimeException(NSQ_PREFIX + errMsg);
        }
        if (arguments[1]->getString() == "sh") {
            table_index = 4;
        } else if (arguments[1]->getString() == "sz") {
            table_index = 5;
        }
    } else {
        std::string errMsg = " The first parameter `type` must be  `snapshot`, `trade` or `orders`.";
        throw RuntimeException(NSQ_PREFIX + errMsg);
    }
    if (table_index < 0 || table_index > 5) {
        std::string errMsg = " Subscribe failed: Please pass in the correct parameters";
        throw RuntimeException(NSQ_PREFIX + errMsg);
    }
    if (SUBSCRIBE_STATUS[table_index] == true) {
        std::string errMsg = " Subscription already exists. To update subscription, call unsubscribe() and try again.";
        throw RuntimeException(NSQ_PREFIX + errMsg);
    }
    TableSP tsp = arguments[2];
    TABLE_NAMES[table_index] = tsp->getName();

    if (arguments[0]->getString() == "snapshot") {
        if (arguments[1]->getString() == "sh") {
            LP_NSQ_API->ReqSecuDepthMarketDataSubscribe(REQ_FIELD_SUB, N_COUNT, 10001);
        } else if (arguments[1]->getString() == "sz") {
            LP_NSQ_API->ReqSecuDepthMarketDataSubscribe(REQ_FIELD_SUB, N_COUNT, 10002);
        }
    } else if (arguments[0]->getString() == "trade") {
        if (arguments[1]->getString() == "sh") {
            LP_NSQ_API->ReqSecuTransactionSubscribe(HS_TRANS_Trade, REQ_FIELD_SUB, N_COUNT, 10003);
        } else if (arguments[1]->getString() == "sz") {
            LP_NSQ_API->ReqSecuTransactionSubscribe(HS_TRANS_Trade, REQ_FIELD_SUB, N_COUNT, 10004);
        }
    } else if (arguments[0]->getString() == "orders") {
        if (arguments[1]->getString() == "sh") {
            LP_NSQ_API->ReqSecuTransactionSubscribe(HS_TRANS_Entrust, REQ_FIELD_SUB, N_COUNT, 10005);
        } else if (arguments[1]->getString() == "sz") {
            LP_NSQ_API->ReqSecuTransactionSubscribe(HS_TRANS_Entrust, REQ_FIELD_SUB, N_COUNT, 10006);
        }
    }

    return new Void();
}

ConstantSP unsubscribe(Heap *heap, vector<ConstantSP> &arguments) {
    LockGuard<Mutex> lockGuard(&NSQ_MUTEX);
    SESSION = heap->currentSession()->copy();
    SESSION->setUser(heap->currentSession()->getUser());
    if (LP_NSQ_API == NULL || !LP_NSP_SPI->GetConnectStatus()) {
        std::string errMsg = " API is not initialized. Please check whether the connection is set up via connect().";
        throw RuntimeException(NSQ_PREFIX + errMsg);
    }
    if (arguments[1]->getString() == "sh") {
        N_COUNT = setReqFieldSub(0);
    } else if (arguments[1]->getString() == "sz") {
        N_COUNT = setReqFieldSub(1);
    } else {
        std::string errMsg = " The second parameter `location` must be `sh` or `sz`.";
        throw RuntimeException(NSQ_PREFIX + errMsg);
    }
    if (arguments[0]->getString() == "snapshot") {
        if (arguments[1]->getString() == "sh") {
            LP_NSQ_API->ReqSecuDepthMarketDataCancel(REQ_FIELD_SUB, N_COUNT, 10001);
        } else if (arguments[1]->getString() == "sz") {
            LP_NSQ_API->ReqSecuDepthMarketDataCancel(REQ_FIELD_SUB, N_COUNT, 10002);
        }
    } else if (arguments[0]->getString() == "trade") {
        if (arguments[1]->getString() == "sh") {
            LP_NSQ_API->ReqSecuTransactionCancel(HS_TRANS_Trade, REQ_FIELD_SUB, N_COUNT, 10003);
        } else if (arguments[1]->getString() == "sz") {
            LP_NSQ_API->ReqSecuTransactionCancel(HS_TRANS_Trade, REQ_FIELD_SUB, N_COUNT, 10004);
        }
    } else if (arguments[0]->getString() == "orders") {
        if (arguments[1]->getString() == "sh") {
            LP_NSQ_API->ReqSecuTransactionCancel(HS_TRANS_Entrust, REQ_FIELD_SUB, N_COUNT, 10005);
        } else if (arguments[1]->getString() == "sz") {
            LP_NSQ_API->ReqSecuTransactionCancel(HS_TRANS_Entrust, REQ_FIELD_SUB, N_COUNT, 10006);
        }
    } else {
        std::string errMsg = " The first parameter `type` must be  `snapshot`, `trade` or `orders`.";
        throw RuntimeException(NSQ_PREFIX + errMsg);
    }
    return new Void();
}

/**
 * @brief Get the Subscription Status object
 *
 * @param heap
 * @param arguments
 * @return ConstantSP
 *     is connected: true
 *     processedMsgCount: 1
 *     lastErrMsg: xxx
 *     failedMsgCount: 0
 *     lastFailedTimestamp: xxx
 *     topics:
 *          (snapshot, sh): true
 *          (snapshot, sz): false
 *          (trade, sh): false
 *          (trade, sz): true
 *          (orders, sh): false
 *          (orders, sz): false
 */
ConstantSP getSubscriptionStatus(Heap *heap, vector<Constant> &arguments) {
    LockGuard<Mutex> lockGuard(&NSQ_MUTEX);
    SESSION = heap->currentSession()->copy();

    vector<DATA_TYPE> types{DT_STRING, DT_BOOL, DT_BOOL, DT_LONG, DT_STRING, DT_LONG, DT_TIMESTAMP};
    vector<string> names{"topicType",  "isConnected",    "isSubscribed",       "processedMsgCount",
                         "lastErrMsg", "failedMsgCount", "lastFailedTimestamp"};
    TableSP tsp = Util::createTable(names, types, 0, 6);

    vector<string> topic_types{"(snapshot, sh)", "(snapshot, sz)", "(trade, sh)",
                               "(trade, sz)",    "(orders, sh)",    "(orders, sz)"};

    for (int i = 0; i < 6; i++) {
        vector<ConstantSP> columns(7);
        for (int j = 0; j < 7; j++) {
            columns[j] = Util::createVector(types[j], 1, 1);
        }
        columns[0]->set(0, new String(topic_types[i]));
        columns[1]->set(0, new Bool((LP_NSP_SPI == NULL) ? false : LP_NSP_SPI->GetConnectStatus()));
        columns[2]->set(0, new Bool(SUBSCRIBE_STATUS[i]));
        columns[3]->set(0, new Long(STATUS[i].processedMsgCount));
        columns[4]->set(0, new String(STATUS[i].lastErrMsg));
        columns[5]->set(0, new Long(STATUS[i].failedMsgCount));

        // columns[6]->set(0, new Timestamp(LLONG_MIN));
        columns[6]->set(0, new Timestamp(STATUS[i].lastFailedTimestamp));
        // columns[6]->set(0, nullptr);
        INDEX insertedRows;
        string errMsg;
        tsp->append(columns, insertedRows, errMsg);
    }

    return tsp;
}

ConstantSP getSchema(Heap *heap, vector<ConstantSP> &arguments) { // type
    LockGuard<Mutex> lockGuard(&NSQ_MUTEX);
    if (IS_CONNECTED == false) {
        throw RuntimeException(NSQ_PREFIX + " call the connect function first");
    }
    if (arguments[0]->getForm() != DF_SCALAR || arguments[0]->getType() != DT_STRING) {
        throw RuntimeException(NSQ_PREFIX + " first argument illegal, should be string");
    }
    string nsqDataType = arguments[0]->getString();
    ConstantSP table;
    if (nsqDataType == "snapshot") {
        table = getSnapshotSchema();
    } else if (nsqDataType == "trade") {
        table = getTradeSchema();
    } else if (nsqDataType == "orders") {
        table = getOrdersSchema();
    } else {
        throw RuntimeException(NSQ_PREFIX + " first argument illegal, should be one of `snapshot`, `trade` or `orders`");
    }

    return table;
}