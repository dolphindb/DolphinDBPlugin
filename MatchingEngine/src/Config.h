//
// Created by jccai on 19-5-24.
//

#ifndef MATCHINGENGINE_CONFIG_H
#define MATCHINGENGINE_CONFIG_H

#include "DolphinDBEverything.h"
#include <Util.h>
#include <string>

struct Config {
    Config() = default;

    bool validate() {
        if (inputScheme_.isNull()) {
            throw ddb::IllegalArgumentException("Config::validate", "missing inputScheme");
        }
        if (mapping_.empty()) {
            throw ddb::IllegalArgumentException("Config::validate", "missing fields");
        }
        initialized = true;
        return true;
    }

    bool initialized = false;
    ddb::TableSP inputScheme_;
    std::vector<string> inputColumnNames_;
    std::vector<int> mapping_;
    uint32_t pricePrecision_;
    uint32_t bookDepth_;
};

using ConfigPtr = std::shared_ptr<Config>;

inline ddb::TableSP createTableFromModel(const ddb::TableSP &model) {
    std::vector<ddb::ConstantSP> columns;
    std::vector<string> columnNames;
    for (ddb::INDEX i = 0; i < model->columns(); ++i) {
        columns.push_back(ddb::Util::createVector(model->getColumnType(i), 0, ddb::Util::BUF_SIZE));
        columnNames.push_back(model->getColumnName(i));
    }
    return ddb::Util::createTable(columnNames, columns);
}

namespace MappingUtils {

enum Fields { op, symbol, id, quantity, condition, price, thresholdPrice, expiredTime };

static const std::vector<string> fieldsNames = {
    "op", "symbol", "id", "quantity", "condition", "price", "thresholdPrice", "expiredTime",
};

inline const string &fieldsName(Fields ordinal) {
    return fieldsNames[ordinal];
}

}    // namespace MappingUtils

#endif    // MATCHINGENGINE_CONFIG_H
