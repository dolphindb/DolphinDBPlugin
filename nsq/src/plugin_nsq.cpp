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

std::vector<string> sh_codes, sz_codes;

bool receivedTimeFlag = false;
bool getAllFieldNamesFlag = false;
bool isConnected = false;

vector<string> snapshotColumnNames{"ExchangeID",
                                   "InstrumentID",
                                   "LastPrice",
                                   "PreClosePrice",
                                   "OpenPrice",
                                   "HighPrice",
                                   "LowPrice",
                                   "ClosePrice",
                                   "UpperLimitPrice",
                                   "LowerLimitPrice",
                                   "TradeDate",
                                   "UpdateTime",
                                   "TradeVolume",
                                   "TradeBalance",
                                   "AveragePrice",
                                   "BidPrice0",
                                   "BidPrice1",
                                   "BidPrice2",
                                   "BidPrice3",
                                   "BidPrice4",
                                   "BidPrice5",
                                   "BidPrice6",
                                   "BidPrice7",
                                   "BidPrice8",
                                   "BidPrice9",
                                   "AskPrice0",
                                   "AskPrice1",
                                   "AskPrice2",
                                   "AskPrice3",
                                   "AskPrice4",
                                   "AskPrice5",
                                   "AskPrice6",
                                   "AskPrice7",
                                   "AskPrice8",
                                   "AskPrice9",
                                   "BidVolume0",
                                   "BidVolume1",
                                   "BidVolume2",
                                   "BidVolume3",
                                   "BidVolume4",
                                   "BidVolume5",
                                   "BidVolume6",
                                   "BidVolume7",
                                   "BidVolume8",
                                   "BidVolume9",
                                   "AskVolume0",
                                   "AskVolume1",
                                   "AskVolume2",
                                   "AskVolume3",
                                   "AskVolume4",
                                   "AskVolume5",
                                   "AskVolume6",
                                   "AskVolume7",
                                   "AskVolume8",
                                   "AskVolume9",
                                   "TradesNum",
                                   "InstrumentTradeStatus",
                                   "TotalBidVolume",
                                   "TotalAskVolume",
                                   "MaBidPrice",
                                   "MaAskPrice",
                                   "MaBondBidPrice",
                                   "MaBondAskPrice",
                                   "YieldToMaturity",
                                   "IOPV",
                                   "EtfBuycount",
                                   "EtfSellCount",
                                   "EtfBuyVolume",
                                   "EtfBuyBalance",
                                   "EtfSellVolume",
                                   "EtfSellBalance",
                                   "TotalWarrantExecVolume",
                                   "WarrantLowerPrice",
                                   "WarrantUpperPrice",
                                   "CancelBuyNum",
                                   "CancelSellNum",
                                   "CancelBuyVolume",
                                   "CancelSellVolume",
                                   "CancelBuyValue",
                                   "CancelSellValue",
                                   "TotalBuyNum",
                                   "TotalSellNum",
                                   "DurationAfterBuy",
                                   "DurationAfterSell",
                                   "BidOrdersNum",
                                   "AskOrdersNum",
                                   "PreIOPV",};

vector<string> tradeColumnNames{"ExchangeID", "InstrumentID", "TransFlag", "SeqNo",     "ChannelNo",
                                "TradeDate",  "TransactTime", "TrdPrice",  "TrdVolume", "TrdMoney",
                                "TrdBuyNo",   "TrdSellNo",    "TrdBSFlag", "BizIndex",};

vector<string> ordersColumnNames{"ExchangeID", "InstrumentID", "TransFlag", "SeqNo",     "ChannelNo",
                                "TradeDate",  "TransactTime", "OrdPrice",  "OrdVolume", "OrdSide",
                                "OrdType",    "OrdNo",        "BizIndex",};

vector<string> addedSnapshotColumnNames{"Bid1Count", "MaxBid1Count", "Ask1Count", "MaxAsk1Count", 
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

vector<string> tradeAndOrdersMergedColumnNames{
    "ExchangeID", "InstrumentID", "TransFlag",     "SeqNo",     "ChannelNo",
    "TradeDate",  "TransactTime", "Trade2_Order1", "Price",     "Volume", 
    "TrdMoney",   "TrdBuyNo",     "TrdSellNo",     "TrdBSFlag", "BizIndex",
    "OrdSide",    "OrdType",      "OrdNo",         "TrdVolume",
};

// 原始的类型，用来创建columns和进行验证
vector<DATA_TYPE> snapshotTypes{
    DT_SYMBOL, DT_SYMBOL, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE,
    DT_DATE,   DT_TIME,   DT_LONG,   DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE,
    DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE,
    DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_LONG,   DT_LONG,   DT_LONG,   DT_LONG,   DT_LONG,
    DT_LONG,   DT_LONG,   DT_LONG,   DT_LONG,   DT_LONG,   DT_LONG,   DT_LONG,   DT_LONG,   DT_LONG,   DT_LONG,
    DT_LONG,   DT_LONG,   DT_LONG,   DT_LONG,   DT_LONG,   DT_LONG,   DT_CHAR,   DT_LONG,   DT_LONG,   DT_DOUBLE,
    DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_DOUBLE, DT_INT,    DT_INT,    DT_LONG,   DT_DOUBLE, DT_LONG,
    DT_DOUBLE, DT_LONG,   DT_DOUBLE, DT_DOUBLE, DT_INT,    DT_INT,    DT_LONG,   DT_LONG,   DT_DOUBLE, DT_DOUBLE,
    DT_INT,    DT_INT,    DT_INT,    DT_INT,    DT_INT,    DT_INT,    DT_DOUBLE};

vector<DATA_TYPE> tradeTypes{DT_SYMBOL, DT_SYMBOL, DT_INT,    DT_LONG, DT_INT,  DT_DATE, DT_TIME,
                             DT_DOUBLE, DT_LONG,   DT_DOUBLE, DT_LONG, DT_LONG, DT_CHAR, DT_LONG};

vector<DATA_TYPE> ordersTypes{DT_SYMBOL, DT_SYMBOL, DT_INT,  DT_LONG, DT_INT,  DT_DATE, DT_TIME,
                             DT_DOUBLE,  DT_LONG,   DT_CHAR, DT_CHAR, DT_LONG, DT_LONG};

vector<DATA_TYPE> addedSnapshotTypes{
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

vector<DATA_TYPE> tradeAndOrdersMergedTypes{
    DT_SYMBOL, DT_SYMBOL, DT_INT,  DT_LONG,   DT_INT,  
    DT_DATE,   DT_TIME,   DT_INT,  DT_DOUBLE, DT_LONG, 
    DT_DOUBLE, DT_LONG,   DT_LONG, DT_CHAR,   DT_LONG,
    DT_CHAR,   DT_CHAR,   DT_LONG, DT_LONG
};

TableSP getSnapshotSchema() {
    if (receivedTimeFlag == true && snapshotColumnNames[snapshotColumnNames.size() - 1] != "ReceivedTime") {
        snapshotColumnNames.push_back("ReceivedTime");
        snapshotTypes.push_back(DT_NANOTIMESTAMP);
    }

    if (receivedTimeFlag == false && snapshotColumnNames[snapshotColumnNames.size() - 1] == "ReceivedTime") {
        snapshotColumnNames.pop_back();
        snapshotTypes.pop_back();
    }

    vector<ConstantSP> cols(2);
    vector<string> colNames = snapshotColumnNames;
    vector<DATA_TYPE> colTypes = snapshotTypes;
    if (getAllFieldNamesFlag) {
        colNames.insert(colNames.end(), addedSnapshotColumnNames.begin(), addedSnapshotColumnNames.end());
        colTypes.insert(colTypes.end(), addedSnapshotTypes.begin(), addedSnapshotTypes.end());
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
    if (receivedTimeFlag == true && tradeColumnNames.back() != "ReceivedTime") {
        tradeColumnNames.push_back("ReceivedTime");
        tradeTypes.push_back(DT_NANOTIMESTAMP);
    }

    if (receivedTimeFlag == false && tradeColumnNames.back() == "ReceivedTime") {
        tradeColumnNames.pop_back();
        tradeTypes.pop_back();
    }

    if (receivedTimeFlag == true && tradeAndOrdersMergedColumnNames.back() != "ReceivedTime") {
        tradeAndOrdersMergedColumnNames.push_back("ReceivedTime");
        tradeAndOrdersMergedTypes.push_back(DT_NANOTIMESTAMP);
    }

    if (receivedTimeFlag == false && tradeAndOrdersMergedColumnNames.back() == "ReceivedTime") {
        tradeAndOrdersMergedColumnNames.pop_back();
        tradeAndOrdersMergedTypes.pop_back();
    }

    vector<string> colNames;
    vector<DATA_TYPE> colTypes;
    if (getAllFieldNamesFlag) {
        colNames = tradeAndOrdersMergedColumnNames;
        colTypes = tradeAndOrdersMergedTypes;
    } else {
        colNames = tradeColumnNames;
        colTypes = tradeTypes;
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
    if (receivedTimeFlag == true && ordersColumnNames.back() != "ReceivedTime") {
        ordersColumnNames.push_back("ReceivedTime");
        ordersTypes.push_back(DT_NANOTIMESTAMP);
    }

    if (receivedTimeFlag == false && ordersColumnNames.back() == "ReceivedTime") {
        ordersColumnNames.pop_back();
        ordersTypes.pop_back();
    }

    if (receivedTimeFlag == true && tradeAndOrdersMergedColumnNames.back() != "ReceivedTime") {
        tradeAndOrdersMergedColumnNames.push_back("ReceivedTime");
        tradeAndOrdersMergedTypes.push_back(DT_NANOTIMESTAMP);
    }

    if (receivedTimeFlag == false && tradeAndOrdersMergedColumnNames.back() == "ReceivedTime") {
        tradeAndOrdersMergedColumnNames.pop_back();
        tradeAndOrdersMergedTypes.pop_back();
    }

    vector<string> colNames;
    vector<DATA_TYPE> colTypes;
    if (getAllFieldNamesFlag) {
        colNames = tradeAndOrdersMergedColumnNames;
        colTypes = tradeAndOrdersMergedTypes;
    } else {
        colNames = ordersColumnNames;
        colTypes = ordersTypes;
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

SmartPointer<Session> session = nullptr;
// vector<TableSP> tables(6, nullptr);
vector<string> tablenames(6, "");
vector<bool> subscribe_status(6, 0);
vector<Status> status(6);

// 一个全局的表结构
// TableSP tsp = nullptr;
// CHSNsqApi的析构函数被protected了
CHSNsqApi *lpNsqApi = NULL;
CHSNsqSpiImpl *lpNsqSpi = NULL;

int nRequestID = 0;
int iRetries = 0;
int nCount = 0;

CHSNsqReqSecuDepthMarketDataField reqField;
CHSNsqReqSecuTransactionRebuildField reqTransRebuild;
CHSNsqReqSecuDepthMarketDataField reqField_sub[1000];

Mutex apiMutex;


ConstantSP nsqClose(Heap *heap, vector<ConstantSP> &arguments) {
    session = heap->currentSession()->copy();
    LockGuard<Mutex> guard{&apiMutex};
    if (lpNsqApi == NULL) {
        std::string errMsg = "Failed to close(). There is no connection to close.";
        throw RuntimeException(errMsg);
    }

    // 先取消所有订阅
    lpNsqApi->ReleaseApi();
    // free(lpNsqApi);  // 不能free， sdk里会管理他的生命周期
    lpNsqApi = NULL;
    if (lpNsqSpi == NULL) {
        std::string errMsg = "Failed to close(). There is no connection to close.";
        throw RuntimeException(errMsg);
    }
    // printf("1\n");
    delete lpNsqSpi;
    lpNsqSpi = NULL;
    LOG_INFO("nsq: close() success");
    // 将订阅状态都设为0
    for (int i = 0; i < 6; i++) {
        subscribe_status[i] = 0;

        status[i].processedMsgCount = 0;
        status[i].failedMsgCount = 0;
        status[i].lastErrMsg = "";
        status[i].lastFailedTimestamp = Timestamp(LLONG_MIN);
    }
    
    isConnected = false;
    receivedTimeFlag = false;
    getAllFieldNamesFlag = false;
    return new Void();
}

ConstantSP nsqConnect(Heap *heap, vector<ConstantSP> &arguments) {
    bool success = false;
    if (lpNsqSpi != NULL){
        std::string errMsg = "You are already connected. To reconnect, please execute close() and try again.";
        throw RuntimeException(errMsg);
    }
    Defer df1([&](){
        if(!success && lpNsqSpi != NULL){
            for (int i = 0; i < 6; i++) {
                subscribe_status[i] = 0;

                status[i].processedMsgCount = 0;
                status[i].failedMsgCount = 0;
                status[i].lastErrMsg = "";
                status[i].lastFailedTimestamp = Timestamp(LLONG_MIN);
            }
            // 先取消所有订阅
            if (lpNsqApi != NULL) {
                lpNsqApi->ReleaseApi();
                // free(lpNsqApi);
                lpNsqApi = NULL;
            }
            delete lpNsqSpi;
            lpNsqSpi = NULL;
        }
    });
    session = heap->currentSession()->copy();
    LockGuard<Mutex> guard{&apiMutex};

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
                receivedTimeFlag = value->getBool();
            }

            if (str == "getAllFieldNames") {
                ConstantSP value = options->getMember("getAllFieldNames");
                if (value->getType() != DT_BOOL) {
                    throw IllegalArgumentException(__FUNCTION__, "value of 'getAllFieldNames' must be boolean");
                }
                getAllFieldNamesFlag = value->getBool();
            }
        }
    } else {
        receivedTimeFlag = false;
        getAllFieldNamesFlag = false;
    }

    //初始化行情api
    // lpNsqApi = NewNsqApi("./log/");
    std::string config_path = arguments[0]->getString();
    // 判断文件是否存在
    std::ifstream f(config_path.c_str());
    if (!f.good()) {
        printf("config file not found\n");
        // 文件不存在，抛出异常.
        std::string errMsg = "Initialization failed. Please check the config file path and the configuration.";
        throw RuntimeException(errMsg);
    }
    lpNsqApi = NewNsqApiExt("./log", config_path.c_str());
    lpNsqSpi = new CHSNsqSpiImpl(lpNsqApi);
    lpNsqApi->RegisterSpi(lpNsqSpi);

    // 说明：RegisterFront接口NSQ中无用，NSQ通过配置文件配置行情站点和灾备地址。为了和恒生经纪柜台保持兼容而保留该接口，调用无效
    lpNsqApi->RegisterFront("tcp://14.21.47.91:8881");

    // 说明：Init接口需调用，NSQ中接口参数均可不填，初始化参数从配置文件中读取。为了和恒生经纪柜台保持兼容
    // int iRet = lpNsqApi->Init("license_3rd.dat");
    int iRet = lpNsqApi->Init("license_3rd.dat");
    if (iRet != 0) {
        std::string errMsg = "Initialization failed. Please check the config file path and the configuration.";
        throw RuntimeException(errMsg);
    }

    //若Tcp连接超时，自动切换到备用地址
    msleep(3000);
    if (!lpNsqSpi->GetConnectStatus()) {
        // switch address!\n"); msleep(3000); 直接抛出异常，不然死循环
        std::string errMsg = "Failed to connect to server. Please check the config file path and the configuration.";
        throw RuntimeException(errMsg);
    }

    CHSNsqReqUserLoginField reqLoginField;
    memset(&reqLoginField, 0, sizeof(reqLoginField));

    CHSNsqReqSecuTransactionRebuildField reqTransRebuild;
    memset(&reqTransRebuild, 0, sizeof(reqTransRebuild));
    //获取用户名、密码、订阅代码
    // readUserFile(reqLoginField, reqTransRebuild);

    iRet = lpNsqApi->ReqUserLogin(&reqLoginField, 0);
    if (iRet != 0) {
        std::string errMsg =
            "login failed: iRet" + std::to_string(iRet) + ", error: " + std::string(lpNsqApi->GetApiErrorMsg(iRet));
        throw RuntimeException(errMsg);
    }

    iRetries = 30; /* 等待登录成功，等待3秒超时 */
    while (!lpNsqSpi->GetLoginStatus() && (iRetries--) > 0) {
        // printf("waiting for login\n");
        msleep(100);
    }

    //当登录出现错误时，程序会打印出错误原因，之后直接退出。
    if (!lpNsqSpi->GetLoginStatus()) {
        std::string errMsg = "login failed, error exit! error: " + std::string(lpNsqApi->GetApiErrorMsg(0));
        throw RuntimeException(errMsg);
    }
    if (lpNsqApi == nullptr) {
        std::string errMsg = "!! lpNsqApi == nullptr";
        throw RuntimeException(errMsg);
    }

    isConnected = true;
    success = true;

    return new Void();
}

bool checkColumns(const TableSP& tbl, int type) {
    vector<DATA_TYPE> col_types;
    if (type == 0) { // snapshot
        col_types = snapshotTypes;
        if (getAllFieldNamesFlag) {
            col_types.insert(col_types.end(), addedSnapshotTypes.begin(), addedSnapshotTypes.end());
        }
    } else if (type == 1) {
        if (getAllFieldNamesFlag) {
            col_types = tradeAndOrdersMergedTypes;
        } else {
            col_types = tradeTypes;
        }
    } else if (type == 2) {
        if (getAllFieldNamesFlag) {
            col_types = tradeAndOrdersMergedTypes;
        } else {
            col_types = ordersTypes;
        }
    } else {
        return false;
    }
    if (tbl->columns() != col_types.size()) {
        return false;
    }
	int numColumns = tbl->columns();
    for (size_t i = 0; i < numColumns; i++) {
		DATA_TYPE dt = tbl->getColumnType(i);
        if (dt != col_types[i]) {
            // 类型兼容
            // STRING：SYMBOL
            // DOUBlE：FLOAT
            // LONG：INT
            bool f = false;
            if (dt == DT_SYMBOL && col_types[i] == DT_STRING) {
                f = true;
            } else if (dt == DT_FLOAT && col_types[i] == DT_DOUBLE) {
                f = true;
            } else if (dt == DT_INT && col_types[i] == DT_LONG) {
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
    if (flag == 0) { // 上海
        memcpy(reqField_sub[0].ExchangeID, HS_EI_SSE,
               sizeof(HS_EI_SSE)); // nCount=0时，全市场订阅
        for (size_t i = 0; i < sh_codes.size(); i++) {
            if (sh_codes[i].length() == 6) {
                memcpy(reqField_sub[nCount].ExchangeID, HS_EI_SSE, sizeof(HS_EI_SSE));
                memcpy(reqField_sub[nCount++].InstrumentID, sh_codes[i].c_str(), sizeof(sh_codes[i]));
            }
        }
    } else if (flag == 1) { // 深圳
        memcpy(reqField_sub[0].ExchangeID, HS_EI_SZSE,
               sizeof(HS_EI_SZSE)); // nCount=0时，全市场订阅
        for (size_t i = 0; i < sz_codes.size(); i++) {
            if (sz_codes[i].length() == 6) {
                memcpy(reqField_sub[nCount].ExchangeID, HS_EI_SZSE, sizeof(HS_EI_SZSE));
                memcpy(reqField_sub[nCount++].InstrumentID, sz_codes[i].c_str(), sizeof(sz_codes[i]));
            }
        }
    }

    return nCount;
}

// 订阅，并且进行列类型检查，设置传递的表
ConstantSP subscribe(Heap *heap, vector<ConstantSP> &arguments) {
    session = heap->currentSession()->copy();
    if (lpNsqApi == NULL || !lpNsqSpi->GetConnectStatus()) {
        std::string errMsg = "API is not initialized. Please check whether the connection is set up via connect().";
        throw RuntimeException(errMsg);
    }
    if (arguments[2]->getForm() != DF_TABLE) {
        std::string errMsg = "The third parameter `streamTable` must be a shared stream table.";
        throw RuntimeException(errMsg);
    } else {
        Table *tblSP = dynamic_cast<Table *>(arguments[2].get());
        if (tblSP->getTableType() != REALTIMETBL || !tblSP->isSharedTable()) {
            std::string errMsg = "The third parameter `streamTable` must be a shared stream table.";
            throw RuntimeException(errMsg);
        }
    }
    LockGuard<Mutex> guard{&apiMutex};
    if (arguments[1]->getString() == "sh") {
        //上海市场
        nCount = setReqFieldSub(0);
    } else if (arguments[1]->getString() == "sz") {
        nCount = setReqFieldSub(1);
    } else {
        std::string errMsg = "The second parameter `location` must be `sh` or `sz`.";
        throw RuntimeException(errMsg);
    }
    int table_index = -1;
    if (arguments[0]->getString() == "snapshot") {
        if (!checkColumns(arguments[2], 0)) {
            std::string errMsg = "Subscription failed. Please check if the schema of “streamTable” is correct.";
            throw RuntimeException(errMsg);
        }
        if (arguments[1]->getString() == "sh") {
            table_index = 0;
        } else if (arguments[1]->getString() == "sz") {
            table_index = 1;
        }
    } else if (arguments[0]->getString() == "trade") {
        if (!checkColumns(arguments[2], 1)) {
            std::string errMsg = "Subscription failed. Please check if the schema of “streamTable” is correct.";
            throw RuntimeException(errMsg);
        }
        if (arguments[1]->getString() == "sh") {
            table_index = 2;
        } else if (arguments[1]->getString() == "sz") {
            table_index = 3;
        }
    } else if (arguments[0]->getString() == "orders") {
        if (!checkColumns(arguments[2], 2)) {
            std::string errMsg = "Subscription failed. Please check if the schema of “streamTable” is correct.";
            throw RuntimeException(errMsg);
        }
        if (arguments[1]->getString() == "sh") {
            table_index = 4;
        } else if (arguments[1]->getString() == "sz") {
            table_index = 5;
        }
    } else {
        std::string errMsg = "The first parameter `type` must be  `snapshot`, `trade` or `orders`.";
        throw RuntimeException(errMsg);
    }
    // printf("table_index: %d\n", table_index);
    // for (int i = 0; i < 6; i++) {
    //     // printf("subscribe stattus[%d]: %d\n", i, subscribe_status[i]);
    // }
    if (table_index < 0 || table_index > 5) {
        std::string errMsg = "subscribe failed: Please pass in the correct parameters";
        throw RuntimeException(errMsg);
    }
    if (subscribe_status[table_index] == true) {
        std::string errMsg = "Subscription already exists. To update subscription, call unsubscribe() and try again.";
        throw RuntimeException(errMsg);
    }
    // 设置表
    // tables[table_index] = arguments[2];
    TableSP tsp = arguments[2];
    tablenames[table_index] = tsp->getName();
    // std::string tmp(tables[table_index]->getName());
    // printf("table name: %s\n", tmp.c_str());

    if (arguments[0]->getString() == "snapshot") {
        if (arguments[1]->getString() == "sh") {
            lpNsqApi->ReqSecuDepthMarketDataSubscribe(reqField_sub, nCount, 10001);
        } else if (arguments[1]->getString() == "sz") {
            lpNsqApi->ReqSecuDepthMarketDataSubscribe(reqField_sub, nCount, 10002);
        }
    } else if (arguments[0]->getString() == "trade") {
        if (arguments[1]->getString() == "sh") {
            lpNsqApi->ReqSecuTransactionSubscribe(HS_TRANS_Trade, reqField_sub, nCount, 10003);
        } else if (arguments[1]->getString() == "sz") {
            lpNsqApi->ReqSecuTransactionSubscribe(HS_TRANS_Trade, reqField_sub, nCount, 10004);
        }
    } else if (arguments[0]->getString() == "orders") {
        if (arguments[1]->getString() == "sh") {
            lpNsqApi->ReqSecuTransactionSubscribe(HS_TRANS_Entrust, reqField_sub, nCount, 10005);
        } else if (arguments[1]->getString() == "sz") {
            lpNsqApi->ReqSecuTransactionSubscribe(HS_TRANS_Entrust, reqField_sub, nCount, 10006);
        }
    }

    return new Void();
}

ConstantSP unsubscribe(Heap *heap, vector<ConstantSP> &arguments) {
    session = heap->currentSession()->copy();
    if (lpNsqApi == NULL || !lpNsqSpi->GetConnectStatus()) {
        std::string errMsg = "API is not initialized. Please check whether the connection is set up via connect().";
        throw RuntimeException(errMsg);
    }
    LockGuard<Mutex> guard{&apiMutex};
    if (arguments[1]->getString() == "sh") {
        //上海市场
        nCount = setReqFieldSub(0);
    } else if (arguments[1]->getString() == "sz") {
        nCount = setReqFieldSub(1);
    } else {
        std::string errMsg = "The second parameter `location` must be `sh` or `sz`.";
        throw RuntimeException(errMsg);
    }
    if (arguments[0]->getString() == "snapshot") {
        if (arguments[1]->getString() == "sh") {
            lpNsqApi->ReqSecuDepthMarketDataCancel(reqField_sub, nCount, 10001);
        } else if (arguments[1]->getString() == "sz") {
            lpNsqApi->ReqSecuDepthMarketDataCancel(reqField_sub, nCount, 10002);
        }
    } else if (arguments[0]->getString() == "trade") {
        if (arguments[1]->getString() == "sh") {
            lpNsqApi->ReqSecuTransactionCancel(HS_TRANS_Trade, reqField_sub, nCount, 10003);
        } else if (arguments[1]->getString() == "sz") {
            lpNsqApi->ReqSecuTransactionCancel(HS_TRANS_Trade, reqField_sub, nCount, 10004);
        }
    } else if (arguments[0]->getString() == "orders") {
        if (arguments[1]->getString() == "sh") {
            lpNsqApi->ReqSecuTransactionCancel(HS_TRANS_Entrust, reqField_sub, nCount, 10005);
        } else if (arguments[1]->getString() == "sz") {
            lpNsqApi->ReqSecuTransactionCancel(HS_TRANS_Entrust, reqField_sub, nCount, 10006);
        }
    } else {
        std::string errMsg = "The first parameter `type` must be  `snapshot`, `trade` or `orders`.";
        throw RuntimeException(errMsg);
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
    session = heap->currentSession()->copy();
    // 创建一个表
    vector<DATA_TYPE> types{DT_STRING, DT_BOOL, DT_BOOL, DT_LONG, DT_STRING, DT_LONG, DT_TIMESTAMP};
    vector<string> names{"topicType",  "isConnected",    "isSubscribed",       "processedMsgCount",
                         "lastErrMsg", "failedMsgCount", "lastFailedTimestamp"};
    TableSP tsp = Util::createTable(names, types, 0, 6);

    vector<string> topic_types{"(snapshot, sh)", "(snapshot, sz)", "(trade, sh)",
                               "(trade, sz)",    "(orders, sh)",    "(orders, sz)"};

    // 对每一行都设置数据
    for (int i = 0; i < 6; i++) {
        vector<ConstantSP> columns(7);
        for (int j = 0; j < 7; j++) {
            columns[j] = Util::createVector(types[j], 1, 1);
        }
        columns[0]->set(0, new String(topic_types[i]));
        columns[1]->set(0, new Bool((lpNsqSpi == NULL) ? false : lpNsqSpi->GetConnectStatus()));
        columns[2]->set(0, new Bool(subscribe_status[i]));
        columns[3]->set(0, new Long(status[i].processedMsgCount));
        columns[4]->set(0, new String(status[i].lastErrMsg));
        columns[5]->set(0, new Long(status[i].failedMsgCount));

        // columns[6]->set(0, new Timestamp(LLONG_MIN));
        // printf("%d\n", NULL == 0);
        columns[6]->set(0, new Timestamp(status[i].lastFailedTimestamp));
        // columns[6]->set(0, nullptr);
        INDEX insertedRows;
        string errMsg;
        tsp->append(columns, insertedRows, errMsg);
    }

    return tsp;
}

ConstantSP getSchema(Heap *heap, vector<ConstantSP> &arguments) { // type 
    if (isConnected == false) {
        throw RuntimeException("call the connect function first");
    }
    if (arguments[0]->getForm() != DF_SCALAR || arguments[0]->getType() != DT_STRING) {
        throw RuntimeException("first argument illegal, should be string");
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
        throw RuntimeException("first argument illegal, should be one of `snapshot`, `trade` or `orders`");
    }

    return table;
}