#include "xgboost.h"

#include "Concurrent.h"
#include "CoreConcept.h"
#include "FlatHashmap.h"
#include "Util.h"
#include "ScalarImp.h"
#include "Logger.h"

#include "xgboost/c_api.h"
#include "ddbplugin/PluginLogger.h"
#include "ddbplugin/PluginLoggerImp.h"

#include <iostream>
#include <vector>
#include <memory>

#define safe_xgboost(call) {    \
int err = (call);               \
if (err != 0)                   \
    throw RuntimeException("[PLUGIN::XGBOOST] " + string(XGBGetLastError()));  \
}

static const string XGBOOST_BOOSTER = "xgboost Booster";
static void checkObjective(string key, string value){
    if(key == "objective"){
        if(value != "reg:linear" && value != "reg:squarederror" && value != "reg:squaredlogerror" && value != "reg:logistic" &&
           value != "reg:pseudohubererror" && value != "binary:logistic" && value != "binary:logitraw" &&
           value != "binary:hinge" && value != "count:poisson" && value != "survival:cox" && value != "survival:aft" &&
           value != "aft_loss_distribution" && value != "multi:softmax" && value != "multi:softprob" && value != "rank:pairwise" &&
           value != "rank:ndcg" && value != "rank:map" && value != "reg:gamma" && value != "reg:tweedie"){
               throw RuntimeException("[PLUGIN::XGBOOST] Unknown objective function: " + value + ". Objective must be one of \'reg:squarederror\' \'reg:squaredlogerror\' " +
               "\'reg:logistic\' \'reg:pseudohubererror\' \'binary:logistic\' \'binary:logitraw\' \'binary:hinge\' \'count:poisson\' \'survival:cox\' \'survival:aft\' \'aft_loss_distribution\' " +
               "\'multi:softmax\' \'multi:softprob\' \'rank:pairwise\' \'rank:ndcg\' \'rank:map\' \'reg:gamma\' \'reg:tweedie\'");
        }
    }
}
static void xgboostBoosterOnClose(Heap *heap, vector<ConstantSP> &args) {
    BoosterHandle hBooster = (BoosterHandle) args[0]->getLong();
    if (nullptr != hBooster) {
        safe_xgboost(XGBoosterFree(hBooster));
        args[0]->setLong(0);
    }
}

static void tableToXGDMatrix(TableSP input, const vector<int> &xColIndices, const int rows, const int cols, float *data, DMatrixHandle *out) {
    float buf[Util::BUF_SIZE];
    for (int i = 0; i < cols; i++) {
        int index = xColIndices[i];
        ConstantSP col = input->getColumn(index);
        INDEX start = 0, len;
        float *d = data + i;
        while (start < rows) {
            len = std::min(Util::BUF_SIZE, rows - start);
            const float *p = col->getFloatConst(start, len, buf);
            for (int j = 0; j < len; j++, d += cols)
                *d = p[j];
            start += len;
        }
    }

    safe_xgboost(XGDMatrixCreateFromMat_omp(data, rows, cols, FLT_NMIN, out, 0));
}

static void matrixOrTableToXGDMatrix(ConstantSP input, const int rows, const int cols, float *data, DMatrixHandle *out) {
    float buf[Util::BUF_SIZE];
    INDEX start = 0, len, end;

    if (input->isMatrix()) {
        for (int i = 0; i < cols; i++) {
            float *d = data + i;
            start = rows * i;
            end = start + rows;
            while (start < end) {
                len = std::min(Util::BUF_SIZE, end - start);
                const float *p = input->getFloatConst(start, len, buf);
                for (int j = 0; j < len; j++, d += cols)
                    *d = p[j];
                start += len;
            }
        }
    }
    else {
        for (int i = 0; i < cols; i++) {
            float *d = data + i;
            start = 0;
            ConstantSP col = input->getColumn(i);
            while (start < rows) {
                len = std::min(Util::BUF_SIZE, rows - start);
                const float *p = col->getFloatConst(start, len, buf);
                for (int j = 0; j < len; j++, d += cols)
                    *d = p[j];
                start += len;
            }
        }
    }

    safe_xgboost(XGDMatrixCreateFromMat_omp(data, rows, cols, FLT_NMIN, out, 0));
}

static ConstantSP trainImpl(Heap *heap, const DMatrixHandle hTrain[], const int rows, const bool hasXgbModel, const ConstantSP &xgbModel, const bool hasParams, const ConstantSP &y, const ConstantSP &params, const int numBoostRound) {
    vector<float> floatTarget;
    vector<unsigned> uintTarget;
    if (y->getCategory() == FLOATING) {
        floatTarget.resize(rows);
        float buf[Util::BUF_SIZE];
        float *d = floatTarget.data();
        INDEX start = 0, len;
        while (start < rows) {
            len = std::min(Util::BUF_SIZE, rows - start);
            const float *p = y->getFloatConst(start, len, buf);
            std::memcpy(d, p, sizeof(float) * len);
            start += len;
            d += len;
        }
        safe_xgboost(XGDMatrixSetFloatInfo(hTrain[0], "label", floatTarget.data(), rows));
    }
    else {
        // No negative values check
        uintTarget.resize(rows);
        int buf[Util::BUF_SIZE];
        unsigned *d = uintTarget.data();
        INDEX start = 0, len;
        while (start < rows) {
            len = std::min(Util::BUF_SIZE, rows - start);
            const int *p = y->getIntConst(start, len, buf);
            std::memcpy(d, p, sizeof(unsigned) * len);
            start += len;
            d += len;
        }
        safe_xgboost(XGDMatrixSetUIntInfo(hTrain[0], "label", uintTarget.data(), rows));
    }

    // create the booster and load some parameters
    BoosterHandle hBooster;
    if (hasXgbModel)
        hBooster = (BoosterHandle) xgbModel->getLong();
    else
        safe_xgboost(XGBoosterCreate(hTrain, 1, &hBooster));
    safe_xgboost(XGBoosterSetParam(hBooster, "validate_parameters", "True"));
    if (hasParams) {
        ConstantSP keys = params->keys();
        ConstantSP values = params->values();
        int keySize = keys->size();
        for (int i = 0; i < keySize; i++) {
            string k = keys->getString(i);
            string v = values->get(i)->getString();
            checkObjective(k,v);
            safe_xgboost(XGBoosterSetParam(hBooster, k.c_str(), v.c_str()));
        }
    }

    // perform learning iterations
    for (int iter = 0; iter < numBoostRound; iter++)
        safe_xgboost(XGBoosterUpdateOneIter(hBooster, iter, hTrain[0]));

    if (hasXgbModel)
        return xgbModel;
    else {
        FunctionDefSP onClose(Util::createSystemProcedure("xgboost Booster onClose()", xgboostBoosterOnClose, 1, 1));
        return Util::createResource((long long)hBooster, XGBOOST_BOOSTER, onClose, heap->currentSession());
    }
}

/**
ConstantSP trainEx(Heap *heap, vector<ConstantSP> &args) {
    string funcName = "xgboost::trainEx";
    string syntax = "Usage: " + funcName + "(dtrain, yColName, xColNames, [params], [numBoostRound=10], [xgbModel]). ";

    if (!args[0]->isTable()) {
        throw IllegalArgumentException(funcName, syntax + "dtrain must be a basic table.");
    }
    TableSP dtrain = args[0];
    if (!dtrain->isBasicTable()) {
        throw IllegalArgumentException(funcName, syntax + "dtrain must be a basic table.");
    }

    if (!args[1]->isScalar() || args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException(funcName, syntax + "yColName must be a string scalar.");
    }
    string yColName = args[1]->getString();
    int yColIndex = dtrain->getColumnIndex(yColName);
    if (yColIndex < 0) {
        throw IllegalArgumentException(funcName, syntax + "dtrain does not contain column '" + yColName + "'.");
    }
    ConstantSP yCol = dtrain->getColumn(yColIndex);
    if (yCol->getCategory() != FLOATING && yCol->getCategory() != INTEGRAL) {
        throw IllegalArgumentException(funcName, syntax + "y column must be of floating or integral type.");
    }

    if ((!args[2]->isArray() && !args[2]->isScalar()) || args[2]->getType() != DT_STRING) {
        throw IllegalArgumentException(funcName, syntax + "xColNames must be a string scalar or vector.");
    }
    ConstantSP xColNames = args[2];

    const int rows = dtrain->rows();
    const int cols = xColNames->isScalar() ? 1 : xColNames->size();
    vector<int> xColIndices;
    for (int i = 0; i < cols; i++) {
        const string &colName = xColNames->getString(i);
        int index = dtrain->getColumnIndex(colName);
        if (index < 0)
            throw IllegalArgumentException(funcName, syntax + "dtrain does not contain column '" + colName + "'.");
        xColIndices.push_back(index);
    }

    DictionarySP params;
    bool hasParams = false;
    if (args.size() >= 4 && !args[3]->isNull()) {
        if (!args[3]->isDictionary()) {
            throw IllegalArgumentException(funcName, syntax + "params must be a dictionary with string keys.");
        }
        params = args[3];
        if (params->getKeyType() != DT_STRING) {
            throw IllegalArgumentException(funcName, syntax + "params must be a dictionary with string keys.");
        }
        hasParams = true;
    }

    int numBoostRound = 10;
    if (args.size() >= 5 && !args[4]->isNull()) {
        if (!args[4]->isScalar() || args[4]->getCategory() != INTEGRAL ||
            (numBoostRound = args[4]->getInt()) <= 0) {
            throw IllegalArgumentException(funcName, syntax + "numBoostRound must be a positive integer.");
        }
    }

    ConstantSP xgbModel;
    bool hasXgbModel = false;
    if (args.size() >= 6 && !args[5]->isNull()) {
        if (args[5]->getType() != DT_RESOURCE || args[5]->getString() != XGBOOST_BOOSTER) {
            throw IllegalArgumentException(funcName, syntax + "xgbModel must be an xgboost Booster resource.");
        }
        hasXgbModel = true;
        xgbModel = args[5];
    }

    // --------- End of argument validation ----------

    // create the train data
    vector<float> data(rows * cols);
    DMatrixHandle hTrain[1];
    tableToXGDMatrix(dtrain, xColIndices, rows, cols, data.data(), &hTrain[0]);

    ConstantSP model = trainImpl(heap, hTrain, rows, hasXgbModel, xgbModel, hasParams, yCol, params, numBoostRound);
    safe_xgboost(XGDMatrixFree(hTrain[0]));

    return model;
}
*/

ConstantSP train(Heap *heap, vector<ConstantSP> &args) {
    string funcName = "xgboost::train";
    string syntax = "Usage: " + funcName + "(Y, X, [params], [numBoostRound=10], [model]). ";
    if (args[0]->getForm() != DF_VECTOR || !args[0]->isNumber() || args[0]->size() == 0) {
        throw IllegalArgumentException(funcName, syntax + "Y must be a numeric vector.");
    }
    for (auto i = 0; i < args[0]->size(); i++) {
        if (args[0]->get(i)->isNull()) {
            throw IllegalArgumentException(funcName, syntax + "Y must be a numeric vector.");
        }
    }
    ConstantSP Y = args[0];

    if (!args[1]->isMatrix() && !args[1]->isTable()) {
        throw IllegalArgumentException(funcName, syntax + "X must be a matrix or a table.");
    }
    ConstantSP X = args[1];
    const int rows = X->rows();
    const int cols = X->columns();

    if (X->isTable()) {
        TableSP t = X;
        if (!t->isBasicTable())
            throw IllegalArgumentException(funcName, syntax + "X must be a basic table.");
        for (int i = 0; i < cols; i++) {
            if (!t->getColumn(i)->isNumber())
                throw IllegalArgumentException(funcName, syntax + "Every column in X must be of a numeric type.");
        }
    } else { // x is matrix
        if (!X->isNumber()) {
            throw IllegalArgumentException(funcName, syntax + "X must be a numeric matrix");
        }
    }

    if (Y->size() != rows) {
        throw IllegalArgumentException(funcName, syntax + "The dimension of dependent doesn't match the dimension of independent factors.");
    }

    DictionarySP params;
    bool hasParams = false;
    if (args.size() >= 3 && !args[2]->isNull()) {
        if (!args[2]->isDictionary()) {
            throw IllegalArgumentException(funcName, syntax + "params must be a dictionary with string keys.");
        }
        params = args[2];
        if (params->getKeyType() != DT_STRING) {
            throw IllegalArgumentException(funcName, syntax + "params must be a dictionary with string keys.");
        }
        hasParams = true;
    }

    int numBoostRound = 10;
    if (args.size() >= 4 && !args[3]->isNull()) {
        if (!args[3]->isScalar() || args[3]->getCategory() != INTEGRAL ||
            (numBoostRound = args[3]->getInt()) <= 0) {
            throw IllegalArgumentException(funcName, syntax + "numBoostRound must be a positive integer.");
        }
    }

    ConstantSP xgbModel;
    bool hasXgbModel = false;
    if (args.size() >= 5 && !args[4]->isNull()) {
        if (args[4]->getType() != DT_RESOURCE || args[4]->getString() != XGBOOST_BOOSTER) {
            throw IllegalArgumentException(funcName, syntax + "model must be an xgboost Booster resource.");
        }
        hasXgbModel = true;
        xgbModel = args[4];
    }

    // --------- End of argument validation ----------

    // create the train data
    vector<float> data(rows * cols);
    DMatrixHandle hTrain[1];
    matrixOrTableToXGDMatrix(X, rows, cols, data.data(), &hTrain[0]);

    ConstantSP model = trainImpl(heap, hTrain, rows, hasXgbModel, xgbModel, hasParams, Y, params, numBoostRound);
    safe_xgboost(XGDMatrixFree(hTrain[0]));

    return model;
}

static ConstantSP predictImpl(Heap *heap, const BoosterHandle hBooster, const DMatrixHandle hTest, const int optionMask, const int ntreeLimit, const int training, const bool outputMatrix, const int rows) {
    bst_ulong outLen = 0;
    const float *f = nullptr;
    safe_xgboost(XGBoosterPredict(hBooster, hTest, optionMask, ntreeLimit, training, &outLen, &f));
    if (!f) {
        throw RuntimeException("[PLUGIN::XGBOOST] predict failed.");
    }

    ConstantSP out = outputMatrix ? Util::createMatrix(DT_FLOAT, rows, outLen/rows, rows) : Util::createVector(DT_FLOAT, outLen);
    float buf[Util::BUF_SIZE];
    INDEX start = 0, len;
    while (start < (INDEX) outLen) {
        len = std::min(Util::BUF_SIZE, (INDEX) outLen - start);
        float *p = out->getFloatBuffer(start, len, buf);
        std::memcpy(p, f + start, sizeof(float) * len);
        out->setFloat(start, len, p);
        start += len;
    }
    if (outputMatrix) {
        FunctionDefSP transpose = heap->currentSession()->getFunctionDef("transpose");
        out = transpose->call(heap, out, nullptr);
    }
    return out;
}

/**
ConstantSP predictEx(Heap *heap, vector<ConstantSP> &args) {
    string funcName = "xgboost::predictEx";
    string syntax = "Usage: " + funcName + "(model, data, xColNames, [outputMargin=false], [ntreeLimit=0], [predLeaf=false], [predContribs=false], [training=false]). ";

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != XGBOOST_BOOSTER) {
        throw IllegalArgumentException(funcName, syntax + "model must be an xgboost Booster resource.");
    }
    BoosterHandle hBooster = (BoosterHandle) args[0]->getLong();
    if (!args[1]->isTable()) {
        throw IllegalArgumentException(funcName, syntax + "data must be a basic table.");
    }
    TableSP input = args[1];
    if (!input->isBasicTable()) {
        throw IllegalArgumentException(funcName, syntax + "data must be a basic table.");
    }

    if ((!args[2]->isArray() && !args[2]->isScalar()) || args[2]->getType() != DT_STRING) {
        throw IllegalArgumentException(funcName, syntax + "xColNames must be a string scalar or vector.");
    }
    ConstantSP xColNames = args[2];

    int optionMask = 0;
    if (args.size() >= 4 && !args[3]->isNull()) {
        if (!args[3]->isScalar() || args[3]->getType() != DT_BOOL) {
            throw IllegalArgumentException(funcName, syntax + "outputMargin must be a boolean scalar.");
        }
        optionMask |= (args[3]->getBool() ? 1 : 0);
    }

    int ntreeLimit = 0;
    if (args.size() >= 5 && !args[4]->isNull()) {
        if (!args[4]->isScalar() || args[4]->getCategory() != INTEGRAL) {
            throw IllegalArgumentException(funcName, syntax + "ntreeLimit must be a non-negative integer.");
        }
        ntreeLimit = args[4]->getInt();
        if (ntreeLimit < 0) {
            throw IllegalArgumentException(funcName, syntax + "ntreeLimit must be a non-negative integer.");
        }
    }

    bool outputMatrix = false;
    if (args.size() >= 6 && !args[5]->isNull()) {
        if (!args[5]->isScalar() || args[5]->getType() != DT_BOOL) {
            throw IllegalArgumentException(funcName, syntax + "predLeaf must be a boolean scalar.");
        }
        optionMask |= (args[5]->getBool() ? 2 : 0);
        outputMatrix = args[5]->getBool();
    }

    if (args.size() >= 7 && !args[6]->isNull()) {
        if (!args[6]->isScalar() || args[6]->getType() != DT_BOOL) {
            throw IllegalArgumentException(funcName, syntax + "predContribs must be a boolean scalar.");
        }
        optionMask |= (args[6]->getBool() ? 4 : 0);
    }

    int training = 0;
    if (args.size() >= 8 && !args[7]->isNull()) {
        if (!args[7]->isScalar() || args[7]->getType() != DT_BOOL) {
            throw IllegalArgumentException(funcName, syntax + "training must be a boolean scalar.");
        }
        training = args[7]->getBool();
    }

    const int rows = input->rows();
    const int cols = xColNames->isScalar() ? 1 : xColNames->size();
    vector<int> xColIndices;
    for (int i = 0; i < cols; i++) {
        const string &colName = xColNames->getString(i);
        int index = input->getColumnIndex(colName);
        if (index < 0)
            throw IllegalArgumentException(funcName, syntax + "data does not contain column '" + colName + "'.");
        xColIndices.push_back(index);
    }

    // create the train data
    vector<float> data(rows * cols);
    DMatrixHandle hTest;
    tableToXGDMatrix(input, xColIndices, rows, cols, data.data(), &hTest);

    ConstantSP out = predictImpl(heap, hBooster, hTest, optionMask, ntreeLimit, training, outputMatrix, rows);
    safe_xgboost(XGDMatrixFree(hTest));

    return out;
}
*/

#ifdef XGBOOST_2_0_0
// use XGBoosterPredictFromDMatrix only for xgboost2.0.0
static ConstantSP predictImpl2(Heap *heap, const BoosterHandle hBooster, const DMatrixHandle hTest, const int rows, const int type, const long long iterationStart, const long long iterationEnd, const bool training, const bool strictShape) {
    string configStr = "{";
    configStr += "\"type\":";
    configStr += std::to_string(type);
    configStr += ", \"training\":";
    configStr += training ? "true": "false";
    configStr += ", \"iteration_begin\":";
    configStr += std::to_string(iterationStart);
    configStr += ", \"iteration_end\":";
    configStr += std::to_string(iterationEnd);
    configStr += ", \"strict_shape\":";
    configStr += strictShape ? "true": "false";
    configStr += "}";
    const char *config = configStr.c_str();
    bst_ulong const *outShape = nullptr;
    bst_ulong outDim = 0;
    const float *outResult = nullptr;
    safe_xgboost(XGBoosterPredictFromDMatrix(hBooster, hTest, config, &outShape, &outDim, &outResult));
    if (!outShape || !outResult) {
        throw RuntimeException("[PLUGIN::XGBOOST] predict failed.");
    }

    ConstantSP out = new Void();
    if(outDim == 1) {
        long long outLen = outShape[0];
        out = Util::createVector(DT_FLOAT, outLen);

        float buf[Util::BUF_SIZE];
        INDEX start = 0, len;
        while (start < (INDEX) outLen) {
            len = std::min(Util::BUF_SIZE, (INDEX) outLen - start);
            float *p = out->getFloatBuffer(start, len, buf);
            std::memcpy(p, outResult + start, sizeof(float) * len);
            out->setFloat(start, len, p);
            start += len;
        }
    } else if (outDim == 2){
        int colNum = outShape[0];
        int colLength = outShape[1];
        out = Util::createMatrix(DT_FLOAT, colNum, colLength, colNum * colLength);
        float buf[Util::BUF_SIZE];
        long long outLen = colNum * colLength;
        INDEX start = 0, len;
        while (start < (INDEX) outLen) {
            len = std::min(Util::BUF_SIZE, (INDEX) outLen - start);
            float *p = out->getFloatBuffer(start, len, buf);
            std::memcpy(p, outResult + start, sizeof(float) * len);
            out->setFloat(start, len, p);
            start += len;
        }
    } else {
        throw RuntimeException("[PLUGIN::XGBOOST] xgboost don't support " + std::to_string(outDim) + " dim prediction result.");
    }

    return out;
}

ConstantSP predict(Heap *heap, vector<ConstantSP> &args) {
    string funcName = "xgboost::predict";
    string syntax = "Usage: " + funcName + "(model, X, [type=0], [iterationRange], [strictShape=false], [training=false]). ";

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != XGBOOST_BOOSTER) {
        throw IllegalArgumentException(funcName, syntax + "model must be an xgboost Booster resource.");
    }
    BoosterHandle hBooster = (BoosterHandle) args[0]->getLong();
    if (!args[1]->isMatrix() && !args[1]->isTable()) {
        throw IllegalArgumentException(funcName, syntax + "X must be a matrix or a table.");
    }
    ConstantSP X = args[1];

    const int rows = X->rows();
    const int cols = X->columns();

    if (X->isTable()) {
        TableSP t = X;
        if (!t->isBasicTable())
            throw IllegalArgumentException(funcName, syntax + "X must be a basic table.");
        for (int i = 0; i < cols; i++) {
            if (!t->getColumn(i)->isNumber())
                throw IllegalArgumentException(funcName, syntax + "Every column in X must be of a numeric type.");
        }
    } else { // x is matrix
        if (!X->isNumber()) {
            throw IllegalArgumentException(funcName, syntax + "X must be a numeric matrix");
        }
    }

    int type = 0;
    if (args.size() >= 3 && !args[2]->isNull()) {
        if (!args[2]->isScalar() || args[2]->getCategory() != INTEGRAL) {
            throw IllegalArgumentException(funcName, syntax + "type must be an integer scalar.");
        }
        type = args[2]->getLong();
        if(type < 0 || type > 6) {
            throw IllegalArgumentException(funcName, syntax + "type must be in the range of 0 to 6.");
        }
    }

    long long iterationStart = 0;
    long long iterationEnd = 0;
    if (args.size() >= 4 && !args[3]->isNull()) {
        if (!args[3]->isPair() || args[3]->getCategory() != INTEGRAL) {
            throw IllegalArgumentException(funcName, syntax + "iterationPair must be a integer pair.");
        }
        iterationStart = args[3]->getLong(0);
        iterationEnd = args[3]->getLong(1);
        if (iterationStart < 0 || iterationEnd < 0) {
            throw IllegalArgumentException(funcName, syntax + "both number of iterationPair must be a non-negative integer.");
        }
        if(iterationEnd < iterationStart) {
            throw IllegalArgumentException(funcName, syntax + "first number of iterationPair must less than second number.");
        }
        // HACK xgboost has bug, if iterationEnd greater than INT_MAX would crash
        if(iterationEnd > INT_MAX) {
            throw IllegalArgumentException(funcName, syntax + "both number of iterationPair must less than 2147483648.");
        }
    } else {
        const char * key = "best_iteration";
        const char * out = "";
        int success;
        safe_xgboost(XGBoosterGetAttr(hBooster, key, &out, &success));
        try {
            if(out != nullptr) {
                int bestIteration = std::atoi(out);
                iterationEnd = bestIteration + 1;
            }
        } catch(...) {
            PLUGIN_LOG_ERR("[PLUGIN::XGBOOST] parse best_iteration failed.");
        }
    }

    bool strictShape = false;
    if (args.size() >= 5 && !args[4]->isNull()) {
        if (!args[4]->isScalar() || args[4]->getType() != DT_BOOL) {
            throw IllegalArgumentException(funcName, syntax + "strictShape must be a boolean scalar.");
        }
        strictShape = args[4]->getBool();
    }

    bool training = 0;
    if (args.size() >= 6 && !args[5]->isNull()) {
        if (!args[5]->isScalar() || args[5]->getType() != DT_BOOL) {
            throw IllegalArgumentException(funcName, syntax + "training must be a boolean scalar.");
        }
        training = args[5]->getBool();
    }
    if (args.size() > 6) {
            throw IllegalArgumentException(funcName, syntax +
                "expects 2~6 argument(s), but the actual number of arguments is: " + std::to_string(args.size()) + ".");
    }

    // create the train data
    vector<float> data(rows * cols);
    DMatrixHandle hTest = nullptr;
    matrixOrTableToXGDMatrix(X, rows, cols, data.data(), &hTest);
    if (!hTest) {
        throw RuntimeException("[PLUGIN::XGBOOST] failed to get matrix handle.");
    }

    ConstantSP out = predictImpl2(heap, hBooster, hTest, rows, type, iterationStart, iterationEnd, training, strictShape);
    safe_xgboost(XGDMatrixFree(hTest));

    return out;
}
#else
ConstantSP predict(Heap *heap, vector<ConstantSP> &args) {
    string funcName = "xgboost::predict";
    string syntax = "Usage: " + funcName + "(model, X, [outputMargin=false], [ntreeLimit=0], [predLeaf=false], [predContribs=false], [training=false]). ";

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != XGBOOST_BOOSTER) {
        throw IllegalArgumentException(funcName, syntax + "model must be an xgboost Booster resource.");
    }
    BoosterHandle hBooster = (BoosterHandle) args[0]->getLong();
    if (!args[1]->isMatrix() && !args[1]->isTable()) {
        throw IllegalArgumentException(funcName, syntax + "X must be a matrix or a table.");
    }
    ConstantSP X = args[1];

    const int rows = X->rows();
    const int cols = X->columns();

    if (X->isTable()) {
        TableSP t = X;
        if (!t->isBasicTable())
            throw IllegalArgumentException(funcName, syntax + "X must be a basic table.");
        for (int i = 0; i < cols; i++) {
            if (!t->getColumn(i)->isNumber())
                throw IllegalArgumentException(funcName, syntax + "Every column in X must be of a numeric type.");
        }
    } else { // x is matrix
        if (!X->isNumber()) {
            throw IllegalArgumentException(funcName, syntax + "X must be a numeric matrix");
        }
    }

    int optionMask = 0;
    if (args.size() >= 3 && !args[2]->isNull()) {
        if (!args[2]->isScalar() || args[2]->getType() != DT_BOOL) {
            throw IllegalArgumentException(funcName, syntax + "outputMargin must be a boolean scalar.");
        }
        optionMask |= (args[2]->getBool() ? 1 : 0);
    }

    int ntreeLimit = 0;
    if (args.size() >= 4 && !args[3]->isNull()) {
        if (!args[3]->isScalar() || args[3]->getCategory() != INTEGRAL) {
            throw IllegalArgumentException(funcName, syntax + "ntreeLimit must be a non-negative integer.");
        }
        ntreeLimit = args[3]->getInt();
        if (ntreeLimit < 0) {
            throw IllegalArgumentException(funcName, syntax + "ntreeLimit must be a non-negative integer.");
        }
    }

    bool outputMatrix = false;
    if (args.size() >= 5 && !args[4]->isNull()) {
        if (!args[4]->isScalar() || args[4]->getType() != DT_BOOL) {
            throw IllegalArgumentException(funcName, syntax + "predLeaf must be a boolean scalar.");
        }
        optionMask |= (args[4]->getBool() ? 2 : 0);
        outputMatrix = args[4]->getBool();
    }

    if (args.size() >= 6 && !args[5]->isNull()) {
        if (!args[5]->isScalar() || args[5]->getType() != DT_BOOL) {
            throw IllegalArgumentException(funcName, syntax + "predContribs must be a boolean scalar.");
        }
        optionMask |= (args[5]->getBool() ? 4 : 0);
    }

    int training = 0;
    if (args.size() >= 7 && !args[6]->isNull()) {
        if (!args[6]->isScalar() || args[6]->getType() != DT_BOOL) {
            throw IllegalArgumentException(funcName, syntax + "training must be a boolean scalar.");
        }
        training = args[6]->getBool();
    }

    // create the train data
    vector<float> data(rows * cols);
    DMatrixHandle hTest;
    matrixOrTableToXGDMatrix(X, rows, cols, data.data(), &hTest);

    ConstantSP out = predictImpl(heap, hBooster, hTest, optionMask, ntreeLimit, training, outputMatrix, rows);
    safe_xgboost(XGDMatrixFree(hTest));

    return out;
}
#endif

ConstantSP saveModel(Heap *heap, vector<ConstantSP> &args) {
    string funcName = "xgboost::saveModel";
    string syntax = "Usage: " + funcName + "(model, filePath). ";

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != XGBOOST_BOOSTER) {
        throw IllegalArgumentException(funcName, syntax + "model must be an xgboost Booster resource.");
    }
    BoosterHandle hBooster = (BoosterHandle) args[0]->getLong();

    if (!args[1]->isScalar() || args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException(funcName, syntax + "fname must be a string.");
    }

    safe_xgboost(XGBoosterSaveModel(hBooster, args[1]->getString().c_str()));
    return new Void();
}

ConstantSP loadModel(Heap *heap, vector<ConstantSP> &args) {
    string funcName = "xgboost::loadModel";
    string syntax = "Usage: " + funcName + "(filePath). ";

    if (!args[0]->isScalar() || args[0]->getType() != DT_STRING) {
        throw IllegalArgumentException(funcName, syntax + "fname must be a string.");
    }

    BoosterHandle hBooster;
    safe_xgboost(XGBoosterCreate(0, 0, &hBooster));
    safe_xgboost(XGBoosterLoadModel(hBooster, args[0]->getString().c_str()));

    FunctionDefSP onClose(Util::createSystemProcedure("xgboost Booster onClose()", xgboostBoosterOnClose, 1, 1));
    return Util::createResource((long long) hBooster, XGBOOST_BOOSTER, onClose, heap->currentSession());
}