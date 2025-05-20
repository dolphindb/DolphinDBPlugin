#include "mseed.h"
#include <Exceptions.h>
#include <ScalarImp.h>
#include <Util.h>
#include <string>
#include <unordered_map>
#include <fstream>
#include "libmseed.h"
#include "ddbplugin/CommonInterface.h"

using namespace std;
using namespace ddb;

void print_stderr(const char *message) {
    //Don't do anything
}

class oneLogInit {
public:
    oneLogInit() {
        ms_loginit(print_stderr, nullptr, print_stderr, nullptr);
    }
};

oneLogInit obj;

void processFirstBlock(MS3Record *msr, char &typeStr, VectorSP &col, long long size) {
    int mIndex = size / 256 * 100 + 1;
    if (msr->sampletype == 'i') {
        typeStr = 'i';
        col = Util::createVector(DT_INT, 0, mIndex);
    } else if (msr->sampletype == 'f') {
        typeStr = 'f';
        col = Util::createVector(DT_FLOAT, 0, mIndex);
    } else if (msr->sampletype == 'd') {
        typeStr = 'd';
        col = Util::createVector(DT_DOUBLE, 0, mIndex);
    }
    else{
        throw RuntimeException(string("The MSEED data type") + msr->sampletype + "is not supported");
    }
}

void processOneBlock(MS3Record *msr, VectorSP &value, vector<string> &sBuffer, char type, int len) {
    char *ptr = (char *) (msr->datasamples);
    switch (type) {
        case 'i': {
            int buffer[len];
            switch (msr->sampletype) {
                case 'i':
                    assert(msr->datasize >= len * sizeof(int));
                    value->appendInt((int *) ptr, len);
                    break;
                case 'a':
                    throw RuntimeException("Can not convert from asill type in miniSEED into INT type in DolphinDB.");
                case 'f': {
                    assert(msr->datasize >= len * sizeof(float));
                    for (int i = 0; i < len; ++i) {
                        buffer[i] = ((float *) ptr)[i];
                    }
                    value->appendInt(buffer, len);
                    break;
                }
                case 'd': {
                    assert(msr->datasize >= len * sizeof(double));
                    for (int i = 0; i < len; ++i) {
                        buffer[i] = ((double *) ptr)[i];
                    }
                    value->appendInt(buffer, len);
                    break;
                }
                default:
                    throw RuntimeException(string("Can not convert from ") + msr->sampletype + " type in miniSEED into INT type in DolphinDB.");
            }
            break;
        }
        case 'f': {
            float buffer[len];
            switch (msr->sampletype) {
                case 'a':
                    throw RuntimeException("Can not convert from asill type in miniSEED into FLOAT type in DolphinDB.");
                case 'i': {
                    assert(msr->datasize >= len * sizeof(int));
                    for (int i = 0; i < len; ++i) {
                        buffer[i] = ((int *) ptr)[i];
                    }
                    value->appendFloat(buffer, len);
                    break;
                }
                case 'd': {
                    assert(msr->datasize >= len * sizeof(double));
                    for (int i = 0; i < len; ++i) {
                        buffer[i] = ((double *) ptr)[i];
                    }
                    value->appendFloat(buffer, len);
                    break;
                }
                case 'f':
                    assert(msr->datasize >= len * sizeof(float));
                    value->appendFloat((float *) ptr, len);
                    break;
                default:
                    throw RuntimeException(string("Can not convert from ") + msr->sampletype + " type in miniSEED into INT type in DolphinDB.");
            }
            break;
        }
        case 'd': {
            double buffer[len];
            switch (msr->sampletype) {
                case 'a':
                    throw RuntimeException(
                            "Can not convert from asill type in miniSEED into DOUBLE type in DolphinDB.");
                case 'i': {
                    assert(msr->datasize >= len * sizeof(int));
                    for (int i = 0; i < len; ++i) {
                        buffer[i] = ((int *) ptr)[i];
                    }
                    value->appendDouble(buffer, len);
                    break;
                }
                case 'f': {
                    assert(msr->datasize >= len * sizeof(float));
                    for (int i = 0; i < len; ++i) {
                        buffer[i] = ((float *) ptr)[i];
                    }
                    value->appendDouble(buffer, len);
                    break;
                }
                case 'd':
                    assert(msr->datasize >= len * sizeof(double));
                    value->appendDouble((double *) ptr, len);
                    break;
                default:
                    throw RuntimeException(string("Can not convert from ") + msr->sampletype + " type in miniSEED into INT type in DolphinDB.");
            }
            break;
        }
        default:
            throw RuntimeException(string("The MSEED data type") + type + "is not supported");
    }
}

ConstantSP mseedRead(Heap *heap, vector<ConstantSP> &args) {
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, "filePath must be a string scalar");
    }
    std::string file = args[0]->getString();
    MS3Record *msr = nullptr;
    int retcode;
    static uint32_t flags = MSF_VALIDATECRC | MSF_PNAMERANGE | MSF_UNPACKDATA;

    bool first = true;
    vector<string> sBuffer(0);
    vector<ConstantSP> cols;
    VectorSP col;
    char type = 'z';
    long long num = 0;
    vector<int> blockNum;
    vector<string> vecId;
    vector<long long> vecTime;
    vector<double> samprate;
    /* Check if file exists */
    std::ifstream infile(file.c_str());
    if (!infile.good()) {
        throw IllegalArgumentException(__FUNCTION__, "File doesn't exist");
    }
    /* Loop over the input file */
    MS3FileParam dmsfp = {"", 0, 0, 0, 0, NULL, 0, 0, 0, {LMIO::LMIO_NULL, NULL, NULL, 0}};
    while ((retcode = ms3_readmsr(&msr, file.c_str(), NULL, NULL, flags, 0, NULL, &dmsfp)) == MS_NOERROR) {
        if (first) {
            processFirstBlock(msr, type, col, file.size());
            first = false;
        }

        int len = msr->numsamples;
        processOneBlock(msr, col, sBuffer, type, len);

        num += len;

        blockNum.push_back(len);
        vecId.push_back(string(msr->sid));
        vecTime.push_back(msr->starttime);
        samprate.push_back(msr->samprate);
    }

    /* Make sure everything is cleaned up */
    ms3_readmsr(&msr, NULL, NULL, NULL, flags, 0, NULL, &dmsfp);

    VectorSP id = Util::createVector(DT_SYMBOL, num);
    VectorSP VecTime = Util::createVector(DT_TIMESTAMP, num);
    int index = 0;
    for (size_t i = 0; i < blockNum.size(); ++i) {
        int curNum = blockNum[i];
        id->fill(index, curNum, new String(vecId[i]));

        long long curStart = vecTime[i];
        long long step = 1000 / samprate[i];
        long long buffer[Util::BUF_SIZE];

        int offect = 0;
        while(offect < curNum){
            int size = curNum - offect >= Util::BUF_SIZE ? Util::BUF_SIZE : curNum - offect;
            long long *p = VecTime->getLongBuffer(index + offect, size, buffer);
            for (int y = 0; y < size; ++y) {
                p[y] = curStart;
                curStart += step;
            }
            VecTime->setLong(index + offect, size, p);
            offect += size;
        }
        index += curNum;
    }
    cols.push_back(id);
    cols.push_back(VecTime);
    cols.push_back(col);
    vector<string> colName = {"id", "time", "value"};
    TableSP ret = Util::createTable(colName, cols);
    return ret;
}

void procesWrite(VectorSP &value, string &sid, double sampleRate, long long &curTime, int mIndex, DATA_TYPE type,
                 bool &cover, int i, string &file) {
    MS3Record *msr = NULL;
    uint32_t flags = MSF_FLUSHDATA;
    int rv;
    if (!(msr = msr3_init(msr))) {
        throw RuntimeException("Could not allocate MS3Record, out of memory");
    }
    strcpy(msr->sid, sid.c_str());
    msr->samprate = sampleRate;
    msr->crc = 0;
    msr->numsamples = mIndex;
    msr->starttime = curTime;
    msr->pubversion = 2;
    msr->formatversion = 2;
    msr->datasize = mIndex;
    msr->reclen = 512;
    int buffer[mIndex * 2];
    switch (type) {
        case DT_INT: {
            msr->sampletype = 'i';
            msr->encoding = DE_STEIM2;
            msr->datasamples = value->getIntBuffer(i * mIndex, mIndex, buffer);
            break;
        }
        case DT_FLOAT: {
            msr->sampletype = 'f';
            msr->encoding = DE_FLOAT32;
            msr->datasamples = value->getFloatBuffer(i * mIndex, mIndex, (float *) buffer);
            break;
        }
        case DT_DOUBLE: {
            msr->sampletype = 'd';
            msr->encoding = DE_FLOAT64;
            msr->datasamples = value->getDoubleBuffer(i * mIndex, mIndex, (double *) buffer);
            break;
        }
        default:
            break;
    }
    msr->samplecnt = mIndex;
    if (cover) {
        rv = msr3_writemseed(msr, file.c_str(), cover, flags, 0);
        cover = false;
    } else
        rv = msr3_writemseed(msr, file.c_str(), cover, flags, 0);
    msr->datasamples = NULL;
    msr3_free(&msr);
    if (rv < 0) {
        throw RuntimeException("Failed to write miniSEED formatted data to " + file);
    }
}

ConstantSP mseedWrite(Heap *heap, vector<ConstantSP> &args) {
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, "filePath must be a string scalar");
    }
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, "Sid must be a string scalar");
    }
    if (args[2]->getType() != DT_TIMESTAMP || args[2]->getForm() != DF_SCALAR || args[2]->isNull()) {
        throw IllegalArgumentException(__FUNCTION__, "StartTime must be a timestamp scalar");
    }
    ConstantSP sampleRateSP = args[3];
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
    if (args[4]->getForm() != DF_VECTOR ||
        !(args[4]->getType() == DT_INT || args[4]->getType() == DT_FLOAT || args[4]->getType() == DT_DOUBLE)) {
        throw IllegalArgumentException(__FUNCTION__, "Value must be a int vector, float vector or double vector");
    }
    bool cover = false;
    if (args.size() > 5) {
        if (args[5]->getType() != DT_BOOL || args[5]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, "Overwrite must be a bool scalar");
        } else
            cover = args[5]->getBool();
    }
    std::string file = args[0]->getString();
    std::string sid = args[1]->getString();
    long long startTime = args[2]->getLong();
    VectorSP value = args[4];

    int dataLen = value->size();
    DATA_TYPE type = value->getType();
    uint32_t flags = MSF_FLUSHDATA;
    int rv;

    int mIndex = 8192;
    long long step = 1000000000 / sampleRate * mIndex;

    int lines = dataLen / mIndex;
    int line = dataLen % mIndex;
    long long curTime = startTime * 1000000;
    for (int i = 0; i < lines; ++i) {
        if(curTime == LONG_MIN)
            throw RuntimeException("The startTime can't be null");
        procesWrite(value, sid, sampleRate, curTime, mIndex, type, cover, i, file);
        curTime += step;
    }
    if (line != 0) {
        if(curTime == LONG_MIN)
            throw RuntimeException("The startTime can't be null");
        MS3Record *msr = NULL;
        if (!(msr = msr3_init(msr))) {
            throw RuntimeException("Could not allocate MS3Record, out of memory");
        }
        strcpy(msr->sid, sid.c_str());
        msr->samprate = sampleRate;
        msr->crc = 0;
        msr->numsamples = line;
        msr->starttime = curTime;
        msr->datasize = line;
        msr->pubversion = 2;
        msr->formatversion = 2;
        msr->reclen = 512;
        int buffer[line * 2];
        switch (type) {
            case DT_INT: {
                msr->sampletype = 'i';
                msr->encoding = DE_STEIM2;
                msr->datasamples = value->getIntBuffer(lines * mIndex, line, buffer);
                break;
            }
            case DT_FLOAT: {
                msr->sampletype = 'f';
                msr->encoding = DE_FLOAT32;
                msr->datasamples = value->getFloatBuffer(lines * mIndex, line, (float *) buffer);
                break;
            }
            case DT_DOUBLE: {
                msr->sampletype = 'd';
                msr->encoding = DE_FLOAT64;
                msr->datasamples = value->getDoubleBuffer(lines * mIndex, line, (double *) buffer);
                break;
            }
            default:
                break;
        }
        rv = msr3_writemseed(msr, file.c_str(), cover, flags, 0);
        msr->datasamples = NULL;
        msr3_free(&msr);
        if (rv < 0) {
            throw RuntimeException("Failed to write miniSEED formated data to " + file);
        }
    }
    return new Bool(true);
}
