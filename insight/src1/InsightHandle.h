#ifndef INSIGHT_HANDLE_H
#define INSIGHT_HANDLE_H

#include "base_define.h"
#include "mdc_client_factory.h"
#include "client_interface.h"
#include "message_handle.h"

#include "CoreConcept.h"
#include "Concurrent.h"
#include "Util.h"
#include "ScalarImp.h"

#include <string>
#include <vector>

using namespace com::htsc::mdc::gateway;
using namespace com::htsc::mdc::model;
using namespace com::htsc::mdc::insight::model;

class InsightHandle : public MessageHandle{
public:
    InsightHandle(SessionSP session, DictionarySP handles);
    virtual ~InsightHandle();
    void OnMarketData(const com::htsc::mdc::insight::model::MarketData &record);
    void OnLoginSuccess();
    void OnLoginFailed(int error_no, const std::string &message);
    void OnNoConnections();
    void OnGeneralError(const com::htsc::mdc::insight::model::InsightErrorContext &context);

    TableSP getReceiveNum(string type);

protected:
    inline void setColumnsStock(std::vector<ConstantSP> &columns, long long receiveTime, const com::htsc::mdc::insight::model::MDStock &record, int index = 0);
    inline void handleStockData(std::vector<ConstantSP> &columns);

    inline void setColumnsIndex(std::vector<ConstantSP> &columns, long long receiveTime, const com::htsc::mdc::insight::model::MDIndex &record, int index = 0);
    inline void handleIndexData(std::vector<ConstantSP> &columns);

    inline void setColumnsFuture(std::vector<ConstantSP> &columns, long long receiveTime, const com::htsc::mdc::insight::model::MDFuture &record, int index = 0);
    inline void handleFutureData(std::vector<ConstantSP> &columns);

    inline void setColumnsTransaction(std::vector<ConstantSP> &columns, long long receiveTime, const com::htsc::mdc::insight::model::MDTransaction &record, int index = 0);
    inline void handleTransactionData(std::vector<ConstantSP> &columns);

    inline void setColumnsOrder(std::vector<ConstantSP> &columns, long long receiveTime, const com::htsc::mdc::insight::model::MDOrder &record, int index = 0);
    inline void handleOrderData(std::vector<ConstantSP> &columns);

    int countDays(int day){
        int year = day / 10000;
        day %= 10000;
        int month = day / 100;
        day %= 100;
        return Util::countDays(year, month, day);
    }

    int realTime(int fake){
        int real = 0;
        real += 3600000 * (fake / 10000000);
        fake %= 10000000;
        real += 60000 * (fake / 100000);
        fake %= 100000;
        real += fake;
        return real;
    }

    int realMinTime(int fake){
        int real = 0;
        real += 3600000 * (fake / 10000000);
        fake %= 10000000;
        real += 60000 * (fake / 100000);
        return real;
    }

protected:
    struct pairHash{
        std::size_t operator()(std::pair<int,int> const& p) const noexcept{
            std::size_t h1 = std::hash<int>{}(p.first);
            std::size_t h2 = std::hash<int>{}(p.second);
            return h1 ^ h2;
        }
    };

protected:
    struct ColumnsStock{
        vector<ConstantSP> columns;

        ColumnsStock(std::vector<std::string> &columnNames, std::size_t &columnSize){
            columns.resize(columnSize);
            createColumnsStock(columns);
        }

        void createColumnsStock(std::vector<ConstantSP> &columns, INDEX colNum = 1);
    };

    struct ColumnsIndex{
        vector<ConstantSP> columns;

        ColumnsIndex(std::vector<std::string> &columnNames, std::size_t &columnSize){
            columns.resize(columnSize);
            createColumnsIndex(columns);
        }

        void createColumnsIndex(std::vector<ConstantSP> &columns, INDEX colNum = 1);
    };

    struct ColumnsFuture{
        vector<ConstantSP> columns;

        ColumnsFuture(std::vector<std::string> &columnNames, std::size_t &columnSize){
            columns.resize(columnSize);
            createColumnsFuture(columns);
        }

        void createColumnsFuture(std::vector<ConstantSP> &columns, INDEX colNum = 1);
    };

    struct ColumnsTransaction{
        vector<ConstantSP> columns;

        ColumnsTransaction(std::vector<std::string> &columnNames, std::size_t &columnSize){
            columns.resize(columnSize);
            createColumnsTransaction(columns);
        }

        void createColumnsTransaction(std::vector<ConstantSP> &columns, INDEX colNum = 1);
    };

    struct ColumnsOrder{
        vector<ConstantSP> columns;

        ColumnsOrder(std::vector<std::string> &columnNames, std::size_t &columnSize){
            columns.resize(columnSize);
            createColumnsOrder(columns);
        }

        void createColumnsOrder(std::vector<ConstantSP> &columns, INDEX colNum = 1);
    };

protected:
    ConstantSP handleStock;
    Mutex *lockStock;
    std::vector<std::string> columnNamesStock;
    std::size_t columnSizeStock;

    ConstantSP handleIndex;
    Mutex *lockIndex;
    std::vector<std::string> columnNamesIndex;
    std::size_t columnSizeIndex;

    ConstantSP handleFuture;
    Mutex *lockFuture;
    std::vector<std::string> columnNamesFuture;
    std::size_t columnSizeFuture;

    ConstantSP handleTransaction;
    Mutex *lockTransaction;
    std::vector<std::string> columnNamesTransaction;
    std::size_t columnSizeTransaction;

    ConstantSP handleOrder;
    Mutex *lockOrder;
    std::vector<std::string> columnNamesOrder;
    std::size_t columnSizeOrder;

    SessionSP session_;
};

#endif 
