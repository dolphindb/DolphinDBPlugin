#include "amdQuoteType.h"
#include "Types.h"

TableSP getOrderExecutionSchema(bool receivedTimeFlag, bool dailyIndexFlag, bool outputElapsedFlag) {
    AmdOrderExecutionTableMeta meta;
    vector<ConstantSP> cols(2);
    if (outputElapsedFlag) {
        meta.colNames_.push_back("perPenetrationTime");
        meta.colTypes_.push_back(DT_NANOTIME);
    }
    cols[0] = Util::createVector(DT_STRING, meta.colNames_.size());
    cols[1] = Util::createVector(DT_STRING, meta.colTypes_.size());
    for (unsigned int i = 0; i < meta.colNames_.size(); i++) {
        cols[0]->setString(i, meta.colNames_[i]);
        cols[1]->setString(i, Util::getDataTypeString(meta.colTypes_[i]));
    }

    std::vector<string> colNames = {"name", "type"};
    TableSP table = Util::createTable(colNames, cols);

    return table;
}

TableSP getSnapshotSchema(bool receivedTimeFlag, bool dailyIndexFlag, bool outputElapsedFlag) {
    AmdSnapshotTableMeta snapShotTableMeta;
    if (receivedTimeFlag) {
        snapShotTableMeta.colNames_.push_back("receivedTime");
        snapShotTableMeta.colTypes_.push_back(DT_NANOTIMESTAMP);
    }
    if (dailyIndexFlag) {
        snapShotTableMeta.colNames_.push_back("dailyIndex");
        snapShotTableMeta.colTypes_.push_back(DT_INT);
    }
    if (outputElapsedFlag) {
        snapShotTableMeta.colNames_.push_back("perPenetrationTime");
        snapShotTableMeta.colTypes_.push_back(DT_NANOTIME);
    }
    vector<ConstantSP> cols(2);
    cols[0] = Util::createVector(DT_STRING, snapShotTableMeta.colNames_.size());
    cols[1] = Util::createVector(DT_STRING, snapShotTableMeta.colTypes_.size());
    for (unsigned int i = 0; i < snapShotTableMeta.colNames_.size(); i++) {
        cols[0]->setString(i, snapShotTableMeta.colNames_[i]);
        cols[1]->setString(i, Util::getDataTypeString(snapShotTableMeta.colTypes_[i]));
    }

    std::vector<string> colNames = {"name", "type"};
    TableSP table = Util::createTable(colNames, cols);

    return table;
}

TableSP getExecutionSchema(bool receivedTimeFlag, bool dailyIndexFlag, bool outputElapsedFlag) {
    AmdExecutionTableMeta executionTableMeta;
    if (receivedTimeFlag) {
        executionTableMeta.colNames_.push_back("receivedTime");
        executionTableMeta.colTypes_.push_back(DT_NANOTIMESTAMP);
    }
    if (dailyIndexFlag) {
        executionTableMeta.colNames_.push_back("dailyIndex");
        executionTableMeta.colTypes_.push_back(DT_INT);
    }
    if (outputElapsedFlag) {
        executionTableMeta.colNames_.push_back("perPenetrationTime");
        executionTableMeta.colTypes_.push_back(DT_NANOTIME);
    }
    vector<ConstantSP> cols(2);
    cols[0] = Util::createVector(DT_STRING, executionTableMeta.colNames_.size());
    cols[1] = Util::createVector(DT_STRING, executionTableMeta.colTypes_.size());
    for (unsigned int i = 0; i < executionTableMeta.colNames_.size(); i++) {
        cols[0]->setString(i, executionTableMeta.colNames_[i]);
        cols[1]->setString(i, Util::getDataTypeString(executionTableMeta.colTypes_[i]));
    }

    std::vector<string> colNames = {"name", "type"};
    TableSP table = Util::createTable(colNames, cols);

    return table;
}

TableSP getOrderSchema(bool receivedTimeFlag, bool dailyIndexFlag, bool outputElapsedFlag) {
    AmdOrderTableMeta orderTableMeta;
    if (receivedTimeFlag) {
        orderTableMeta.colNames_.push_back("receivedTime");
        orderTableMeta.colTypes_.push_back(DT_NANOTIMESTAMP);
    }
    if (dailyIndexFlag) {
        orderTableMeta.colNames_.push_back("dailyIndex");
        orderTableMeta.colTypes_.push_back(DT_INT);
    }
    if (outputElapsedFlag) {
        orderTableMeta.colNames_.push_back("perPenetrationTime");
        orderTableMeta.colTypes_.push_back(DT_NANOTIME);
    }
    vector<ConstantSP> cols(2);
    cols[0] = Util::createVector(DT_STRING, orderTableMeta.colNames_.size());
    cols[1] = Util::createVector(DT_STRING, orderTableMeta.colTypes_.size());
    for (unsigned int i = 0; i < orderTableMeta.colNames_.size(); i++) {
        cols[0]->setString(i, orderTableMeta.colNames_[i]);
        cols[1]->setString(i, Util::getDataTypeString(orderTableMeta.colTypes_[i]));
    }

    std::vector<string> colNames = {"name", "type"};
    TableSP table = Util::createTable(colNames, cols);

    return table;
}

TableSP getIndexSchema(bool receivedTimeFlag, bool dailyIndexFlag, bool outputElapsedFlag) {
    AmdIndexTableMeta indexTableMeta;
    if (receivedTimeFlag) {
        indexTableMeta.colNames_.push_back("receivedTime");
        indexTableMeta.colTypes_.push_back(DT_NANOTIMESTAMP);
    }
    if (dailyIndexFlag) {
        indexTableMeta.colNames_.push_back("dailyIndex");
        indexTableMeta.colTypes_.push_back(DT_INT);
    }
    if (outputElapsedFlag) {
        indexTableMeta.colNames_.push_back("perPenetrationTime");
        indexTableMeta.colTypes_.push_back(DT_NANOTIME);
    }
    vector<ConstantSP> cols(2);
    cols[0] = Util::createVector(DT_STRING, indexTableMeta.colNames_.size());
    cols[1] = Util::createVector(DT_STRING, indexTableMeta.colTypes_.size());
    for (unsigned int i = 0; i < indexTableMeta.colNames_.size(); i++) {
        cols[0]->setString(i, indexTableMeta.colNames_[i]);
        cols[1]->setString(i, Util::getDataTypeString(indexTableMeta.colTypes_[i]));
    }

    std::vector<string> colNames = {"name", "type"};
    TableSP table = Util::createTable(colNames, cols);

    return table;
}

TableSP getOrderQueueSchema(bool receivedTimeFlag, bool dailyIndexFlag, bool outputElapsedFlag) {
    AmdOrderQueueTableMeta orderQueueTableMeta;
    if (receivedTimeFlag) {
        orderQueueTableMeta.colNames_.push_back("receivedTime");
        orderQueueTableMeta.colTypes_.push_back(DT_NANOTIMESTAMP);
    }
    if (dailyIndexFlag) {
        orderQueueTableMeta.colNames_.push_back("dailyIndex");
        orderQueueTableMeta.colTypes_.push_back(DT_INT);
    }
    if (outputElapsedFlag) {
        orderQueueTableMeta.colNames_.push_back("perPenetrationTime");
        orderQueueTableMeta.colTypes_.push_back(DT_NANOTIME);
    }
    vector<ConstantSP> cols(2);
    cols[0] = Util::createVector(DT_STRING, orderQueueTableMeta.colNames_.size());
    cols[1] = Util::createVector(DT_STRING, orderQueueTableMeta.colTypes_.size());
    for (unsigned int i = 0; i < orderQueueTableMeta.colNames_.size(); i++) {
        cols[0]->setString(i, orderQueueTableMeta.colNames_[i]);
        cols[1]->setString(i, Util::getDataTypeString(orderQueueTableMeta.colTypes_[i]));
    }

    std::vector<string> colNames = {"name", "type"};
    TableSP table = Util::createTable(colNames, cols);

    return table;
}

// TODO(ruibinhuang@dolphindb.com): check the real attributes of the table
bool checkSchema(const string& type, TableSP table) {
    INDEX tableColumns = table->columns();
    if(receivedTimeFlag)
        tableColumns--;
    if(dailyIndexFlag)
        tableColumns--;
    if(outputElapsedFlag)
        tableColumns--;
    if (type == "snapshot") {
        if (tableColumns!= 94) {
            return false;
        }
    } else if (type == "execution") {
        if (tableColumns != 15) {
            return false;
        }
    } else if (type == "index") {
        if (tableColumns != 15) {
            return false;
        }
    } else if (type == "orderQueue") {
        if (tableColumns != 61) {
            return false;
        }
    } else if (type == "fundSnapshot") {
        if (tableColumns != 94) {
            return false;
        }
    } else if (type == "fundExecution") {
        if (tableColumns != 15) {
            return false;
        }
    } else if (type == "fundOrder") {
        if (tableColumns != 13) {
            return false;
        }
    } else if(type == "order") {
        if (tableColumns != 13) {
            return false;
        }
    }else if (type == "bondSnapshot") {
        if (tableColumns != 94) {
            return false;
        }
    } else if (type == "bondExecution") {
        if (tableColumns != 15) {
            return false;
        }
    } else if (type == "bondOrder") {
        if (tableColumns != 13) {
            return false;
        }
    } else if(type == "orderExecution" || type == "fundOrderExecution" || type == "bondOrderExecution") {
        tableColumns += 2;   //because two flags always on, so plus 2 to pass check
        if (tableColumns != 16) {       //FIXME columns number not depends on param
            return false;
        }
    } else{
        throw IllegalArgumentException(__FUNCTION__, "error type " + type);
    }

    return true;
}
