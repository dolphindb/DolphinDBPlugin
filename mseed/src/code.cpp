#include "code.h"
#include <Exceptions.h>
#include <ScalarImp.h>
#include <Util.h>
#include <string>
#include <unordered_map>
#include "mseed.h"
#include "libmseed.h"
#include "parser.h"
#include "msrcode.h"

using namespace ddb;

ConstantSP mseedCode(string sid, long long startTime, double sampleRate, VectorSP value, int blockSize) {

    if(sid == "")
        throw RuntimeException("The sid cannot be empty");
    startTime *= 1000000;
    int dataSize = value->size();
    int dataOffect = 0;
    int mIndex = 8192;
    DATA_TYPE type = value->getType();
    uint32_t flags = MSF_FLUSHDATA;
    long long step = 1000000000 / sampleRate;
    MS3Record *msr = NULL;
    if (!(msr = msr3_init(msr))) {
        throw RuntimeException("Could not allocate MS3Record, out of memory");
    }
    string ret;
    strcpy(msr->sid, sid.c_str());
    msr->samprate = sampleRate;
    msr->crc = 0;
    msr->pubversion = 2;
    msr->formatversion = 2;
    msr->reclen = blockSize;

    while(dataOffect < dataSize) {
        int subDataSize = min(dataSize - dataOffect, mIndex);
        msr->numsamples = subDataSize;
        msr->datasize = subDataSize;
        msr->starttime = startTime + dataOffect * step;
        shared_ptr<double> buffer = make_shared<double>(mIndex);
        switch (type) {
            case DT_INT: {
                int* bufferPtr = (int*)buffer.get();
                msr->sampletype = 'i';
                msr->encoding = DE_STEIM2;
                msr->datasamples = value->getIntBuffer(dataOffect, subDataSize, bufferPtr);
                break;
            }
            case DT_FLOAT: {
                float * bufferPtr = (float*)buffer.get();
                msr->sampletype = 'f';
                msr->encoding = DE_FLOAT32;
                msr->datasamples = value->getFloatBuffer(dataOffect, subDataSize, bufferPtr);
                break;
            }
            case DT_DOUBLE: {
                double * bufferPtr = (double*)buffer.get();
                msr->sampletype = 'd';
                msr->encoding = DE_FLOAT64;
                msr->datasamples = value->getDoubleBuffer(dataOffect, subDataSize, bufferPtr);
                break;
            }
            default:
                break;
        }
        dataOffect += subDataSize;
        string subRet = msr3_code_mseed2(msr, flags, 0);
        ret.append(subRet.c_str(), subRet.size());
    }
    msr->datasamples = NULL;
    msr3_free(&msr);
    ConstantSP retTmp = Util::createVector(DT_CHAR, ret.size());
    retTmp->setChar(0, ret.size(), ret.c_str());
    return retTmp;
}