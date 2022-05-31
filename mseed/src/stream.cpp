#include <Exceptions.h>
#include <ScalarImp.h>
#include <Util.h>
#include <string>
#include <unordered_map>
#include "mseed.h"
#include "libmseed.h"
#include "stream.h"
#include "parser.h"
#include "msrcode.h"

extern "C" ConstantSP mseedStreamize(Heap *heap, vector<ConstantSP> &args) {
    if (args[0]->getType() == DT_VOID || args[0]->getForm() != DF_TABLE) {
        throw IllegalArgumentException(__FUNCTION__, "Data must be a table");
    }
    ConstantSP sampleRateSP = args[1];
    double sampleRate;
    if(sampleRateSP->getForm() == DF_SCALAR){
        if (sampleRateSP->getType() == DT_INT) {
            sampleRate = sampleRateSP->getInt();
        } else if (sampleRateSP->getType() == DT_LONG) {
            sampleRate = sampleRateSP->getLong();
        } else if (sampleRateSP->getType() == DT_FLOAT) {
            sampleRate = sampleRateSP->getFloat();
        } else if (sampleRateSP->getType() == DT_DOUBLE) {
            sampleRate = sampleRateSP->getDouble();
        }else{
            throw IllegalArgumentException(__FUNCTION__, "SampleRate must be a scalar of type int, long, float or double");
        }
    }
    else{
        throw IllegalArgumentException(__FUNCTION__, "SampleRate must be a scalar of type int, long, float or double");
    }
    int blockSize = 512;
    if (args.size() > 2) {
        if (args[2]->getType() != DT_INT || args[2]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, "blockSize must be a int scalar");
        }
        blockSize = args[2]->getInt();
    }
    TableSP tableData = (TableSP) (args[0]);
    size_t offect = 0, stepBegin = 0, sum = tableData->size();
    if (tableData->columns() < 3)
        throw IllegalArgumentException(__FUNCTION__, "The number of table rows must be greater than or equal to 3");
    VectorSP sid = tableData->getColumn(0);
    if (sid.isNull() || (sid->getType() != DT_SYMBOL && sid->getType() != DT_STRING))
        throw IllegalArgumentException(__FUNCTION__, "The sid column must be of type SYMBOL or STRING");
    VectorSP ts = tableData->getColumn(1);
    if (ts.isNull() || ts->getType() != DT_TIMESTAMP)
        throw IllegalArgumentException(__FUNCTION__, "The ts column must be of type TIMESTAMP");
    VectorSP data = tableData->getColumn(2);
    if (data.isNull() || (data->getType() != DT_INT && data->getType() != DT_FLOAT && data->getType() != DT_DOUBLE))
        throw IllegalArgumentException(__FUNCTION__, "The data column must be of type INT, FLOAT, DOUBLE");
    if (sid->getType() == DT_STRING) {
        VectorSP tmp = Util::createVector(DT_SYMBOL, 0, sum);
        char *sidData[sum];
        for (size_t i = 0; i < sum; i += 10240) {
            int size = min((size_t) 10240, sum - i);
            tmp->appendString(const_cast<const char **>(sid->getStringConst(i, size, sidData)), size);
        }
        sid = tmp;
    }
    ConstantSP ret = Util::createVector(DT_CHAR, 0);
    int mIndex = 8192;
    vector<int> sidBuffer(mIndex);
    vector<long long> tsBuffer(mIndex);
    vector<double> dataBuffer(mIndex);
    const int *sidPtr = sid->getIntConst(0, 0, sidBuffer.data());
    const long long *tsPtr = ts->getLongConst(0, 0, tsBuffer.data());
    const void *dataPtr;
    double step = 1000 / sampleRate;
    while (offect < sum) {
        VectorSP value;
        int size = min(sum - stepBegin, (size_t) mIndex);
        sidPtr = sid->getIntConst(offect, size, sidBuffer.data());
        tsPtr = ts->getLongConst(offect, size, tsBuffer.data());
        if (data->getType() == DT_INT)
            dataPtr = data->getIntConst(offect, size, (int*)dataBuffer.data());
        else if (data->getType() == DT_FLOAT)
            dataPtr = data->getFloatBuffer(offect, size, (float *) (dataBuffer.data()));
        else
            dataPtr = data->getDoubleConst(offect, size, dataBuffer.data());
        if(tsPtr[offect - stepBegin] == LONG_MIN)
            throw RuntimeException("The column ts can't be null");
        if (size == 1) {
            if (data->getType() == DT_INT) {
                if(((int*)dataPtr)[0] == INT_MIN)
                    throw RuntimeException("The value of mseed data can't be null");
                value = Util::createVector(DT_INT, 1, 1);
                value->setInt(0, ((int*)dataPtr)[0]);
            } else if (data->getType() == DT_FLOAT) {
                if(((float *)dataPtr)[0] == FLT_NMIN)
                    throw RuntimeException("The value of mseed data can't be null");
                value = Util::createVector(DT_FLOAT, 1, 1);
                value->setFloat(0, ((float*)dataPtr)[0]);
            } else {
                if(((double *)dataPtr)[0] == DBL_NMIN)
                    throw RuntimeException("The value of mseed data can't be null");
                value = Util::createVector(DT_DOUBLE, 1, 1);
                value->appendDouble(0, ((double*)dataPtr)[0]);
            }
            ((VectorSP) ret)->append(mseedCode(sid->getString(offect), tsPtr[0], sampleRate, value, blockSize));
            ++stepBegin;
            ++offect;
        } else {
            int sidBegin = sidPtr[offect - stepBegin];
            ++offect;
            while (offect < stepBegin + size) {
                if(tsPtr[offect - stepBegin] == LONG_MIN)
                    throw RuntimeException("The column ts can't be null");
                if (tsPtr[offect - stepBegin] - tsPtr[offect - stepBegin - 1] != step)
                    break;
                int sidCur = sidPtr[offect - stepBegin];
                if (sidCur != sidBegin)
                    break;
                ++offect;
            }
            int processSize = offect - stepBegin;
            if (data->getType() == DT_INT) {
                for(int i = 0; i < processSize; ++i){
                    if(((int*)dataPtr)[i] == INT_MIN)
                        throw RuntimeException("The value of mseed data can't be null");
                }
                value = Util::createVector(DT_INT, 0, processSize);
                value->appendInt((int *) dataPtr, processSize);
            } else if (data->getType() == DT_FLOAT) {
                for(int i = 0; i < processSize; ++i){
                    if(((float*)dataPtr)[i] == FLT_NMIN)
                        throw RuntimeException("The value of mseed data can't be null");
                }
                value = Util::createVector(DT_FLOAT, 0, processSize);
                value->appendFloat((float *) dataPtr, processSize);
            } else {
                for(int i = 0; i < processSize; ++i){
                    if(((double*)dataPtr)[i] == DBL_NMIN)
                        throw RuntimeException("The value of mseed data can't be null");
                }
                value = Util::createVector(DT_DOUBLE, 0, processSize);
                value->appendDouble((double *) dataPtr, processSize);
            }
            VectorSP mseedRet = mseedCode(sid->getString(stepBegin), tsPtr[0], sampleRate, value, blockSize);
            ((VectorSP) ret)->append(mseedRet);
            stepBegin = offect;
        }
    }
    return ret;
}