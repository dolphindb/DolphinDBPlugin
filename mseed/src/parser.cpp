#include "parser.h"
#include "ScalarImp.h"
#include "Util.h"
#include "libmseed.h"
#include "Logger.h"
#include "ddbplugin/PluginLogger.h"
#include "ddbplugin/PluginLoggerImp.h"

using namespace std;
using namespace ddb;
Mutex mutexLock;

#ifdef _WIN32
FILE *SCFmemopen(void *buf, size_t size, const char *mode)
{
    char temppath[MAX_PATH - 13];
    if (0 == GetTempPath(sizeof(temppath), temppath))
        return NULL;

    char filename[MAX_PATH + 1];
    if (0 == GetTempFileName(temppath, "SC", 0, filename))
        return NULL;

    FILE *f = fopen(filename, "wb");
    if (NULL == f)
        return NULL;

    fwrite(buf, size, 1, f);
    fclose(f);

    return fopen(filename, mode);
}
#endif

extern void print_stderr(const char *message);

extern bool processFirstBlock(MS3Record *msr, char &typeStr, VectorSP &col, long long size);

extern int processOneBlock(MS3Record *msr, VectorSP &value, vector<string> &sBuffer, char type, int len);

ConstantSP mseedParse(Heap *heap, vector<ConstantSP> &args) {
    if (!(
            (args[0]->getType() == DT_STRING && args[0]->getForm() == DF_SCALAR) ||
            (args[0]->getType() == DT_CHAR && args[0]->getForm() == DF_VECTOR)))
        throw IllegalArgumentException(__FUNCTION__, "Data must be a string scalar or a character vector");
    FILE *fp = NULL;
    std::string data;
    long long size = 0;/*Stack array size. The stack array is used when the data size is less than MAX_STACK_BUFFER_SIZE. */
    if (args[0]->getType() == DT_STRING) {
        size = 0;
    }else{
        size = args[0]->size();
        if(size == 0){
            throw IllegalArgumentException(__FUNCTION__, "Vector size can't be zero");
        }else if(size > MAX_STACK_BUFFER_SIZE){
            size = 0;
        }
    }
    char bufferTemp[size];
    vector<char> bufferVector;
    char* buffer;
#ifdef __linux__
    if (args[0]->getType() == DT_STRING) {
        data = args[0]->getString();
        if (data.size() == 0) {
            throw IllegalArgumentException(__FUNCTION__, "Length of data string scalar must be larger than 0.");
        }
        fp = fmemopen((void *)(data.c_str()), data.size(), "rw");
        if (fp == NULL)
        {
            throw RuntimeException("Fmemopen fail. Because " + string(strerror(errno)));
        }
        size = data.size();
    } else {
        if(size > 0){
            buffer = bufferTemp;
        }else{
            size = args[0]->size();
            bufferVector.resize(size);
            buffer = bufferVector.data();
        }
        ((VectorSP) args[0])->getChar(0, size, (char *) buffer);
        fp = fmemopen((void *) buffer, size, "rw");
        if (fp == NULL)
        {
            throw RuntimeException("Fmemopen fail. Because " + string(strerror(errno)));
        }
    }
#else
    if (args[0]->getType() == DT_STRING)
    {
        data = args[0]->getString();
        mutexLock.lock();
        fp = SCFmemopen((void*)data.c_str(), data.size(), "rb");
        if (fp == NULL)
        {
            mutexLock.unlock();
            throw RuntimeException("SCFmemopen fail");
        }
        size = data.size();
    }
    else
    {
        if(size > 0){
            buffer = bufferTemp;
        }else{
            size = args[0]->size();
            bufferVector.resize(size);
            buffer = bufferVector.data();
        }
        ((VectorSP)args[0])->getChar(0, size, (char *)buffer);
        mutexLock.lock();
        fp = SCFmemopen((void *)buffer, size, "rb");
        if (fp == NULL)
        {
            mutexLock.unlock();
            throw RuntimeException("SCFmemopen fail");
        }
    }
#endif
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
    /* Loop over the input file */
    MS3FileParam dmsfp = {"", 0, 0, 0, 0, NULL, 0, 0, 0, {LMIO::LMIO_NULL, NULL, NULL, 0}};
    ms3_readmsr(&msr, NULL, NULL, NULL, flags, 0, NULL, &dmsfp);
    try {
        while ((retcode = ms3_readmsr(&msr, "the byte stream", NULL, NULL, flags, 0, fp, &dmsfp)) == MS_NOERROR) {
            if (first) {
                processFirstBlock(msr, type, col, size);
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
    }
    catch (RuntimeException &e) {
        ms3_readmsr(&msr, NULL, NULL, NULL, flags, 0, NULL, &dmsfp);
#ifndef __linux__
        mutexLock.unlock();
#endif
        throw e;
    }
    ms3_readmsr(&msr, NULL, NULL, NULL, flags, 0, NULL, &dmsfp);
#ifndef __linux__
    mutexLock.unlock();
#endif

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
    if (col.isNull())
        throw RuntimeException("The given miniSEED byte stream data cannot be parsed");

    cols.push_back(id);
    cols.push_back(VecTime);
    cols.push_back(col);
    vector<string> colName = {"id", "time", "value"};
    TableSP ret = Util::createTable(colName, cols);
    return ret;
}

bool isAvailableType(char type){
    if(type == 'i' || type == 'd' || type == 'f' || type == 'a')
        return true;
    PLUGIN_LOG_WARN(string("MseedPlugin : The mseed data type ") + type + " is not supported. ");
    return false;
}

ConstantSP mseedParseStream(Heap *heap, vector<ConstantSP> &args) {
    if (!(
            (args[0]->getType() == DT_STRING && args[0]->getForm() == DF_SCALAR) ||
            (args[0]->getType() == DT_CHAR && args[0]->getForm() == DF_VECTOR)))
        throw IllegalArgumentException(__FUNCTION__, "Data must be a string scalar or a character vector");
    FILE *fp = NULL;
    vector<char> bufferVector;
    std::string data;
    long long size = 0;
    if (args[0]->getType() == DT_STRING) {
        size = 1;
    }else{
        size = args[0]->size();
        if(size == 0){
            throw IllegalArgumentException(__FUNCTION__, "Vector size can't be zero");
        }else if(size > MAX_STACK_BUFFER_SIZE){
            size = 0;
        }
    }
    char bufferTemp[size];
    char* buffer;
#ifdef __linux__
    if (args[0]->getType() == DT_STRING) {
        data = args[0]->getString();
        fp = fmemopen((void *)(data.c_str()), data.size(), "rw");
        if (fp == NULL)
        {
            throw RuntimeException("Fmemopen fail. Because " + string(strerror(errno)));
        }
        size = data.size();
    } else {
        if(size > 0){
            buffer = bufferTemp;
        }else{
            size = args[0]->size();
            bufferVector.resize(size);
            buffer = bufferVector.data();
        }
        ((VectorSP) args[0])->getChar(0, size, (char *) buffer);
        fp = fmemopen((void *) buffer, size, "rw");
        if (fp == NULL)
        {
            throw RuntimeException("Fmemopen fail. Because " + string(strerror(errno)));
        }
    }
#else
    if (args[0]->getType() == DT_STRING)
    {
        data = args[0]->getString();
        mutexLock.lock();
        fp = SCFmemopen((void*)data.c_str(), data.size(), "rb");
        if (fp == NULL)
        {
            mutexLock.unlock();
            throw RuntimeException("SCFmemopen fail");
        }
        size = data.size();
    }
    else
    {
        if(size > 0){
            buffer = bufferTemp;
        }else{
            size = args[0]->size();
            bufferVector.resize(size);
            buffer = bufferVector.data();
        }
        ((VectorSP)args[0])->getChar(0, size, (char *)buffer);
        mutexLock.lock();
        fp = SCFmemopen((void *)buffer, size, "rb");
        if (fp == NULL)
        {
            mutexLock.unlock();
            throw RuntimeException("SCFmemopen fail");
        }
    }
#endif
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
    vector<int> expectedCount;
    vector<string> vecId;
    vector<long long> vecTime;
    vector<double> samprate;
    long long reclenIndex = 0;
    MS3FileParam dmsfp = {"", 0, 0, 0, 0, NULL, 0, 0, 0, {LMIO::LMIO_NULL, NULL, NULL, 0}};
    ms3_readmsr(&msr, NULL, NULL, NULL, flags, 0, NULL, &dmsfp);
    try {
        while ((retcode = ms3_readmsr(&msr, "the byte stream", NULL, NULL, flags, 0, fp, &dmsfp)) == MS_NOERROR) {
            if(!isAvailableType(msr->sampletype))
                break;
            if (first) {
                processFirstBlock(msr, type, col, size);
                first = false;
            }

            int len = msr->numsamples;
            processOneBlock(msr, col, sBuffer, type, len);

            num += len;
            reclenIndex += msr->reclen;
            msr->reclen=-1;
            blockNum.push_back(len);
            expectedCount.push_back(msr->samplecnt);
            vecId.push_back(string(msr->sid));
            vecTime.push_back(msr->starttime);
            samprate.push_back(msr->samprate);
        }
    }
    catch (RuntimeException &e) {
        ms3_readmsr(&msr, NULL, NULL, NULL, flags, 0, NULL, &dmsfp);
#ifndef __linux__
        mutexLock.unlock();
#endif
        throw e;
    }
    ms3_readmsr(&msr, NULL, NULL, NULL, flags, 0, NULL, &dmsfp);
    msr3_free(&msr);
#ifndef __linux__
    mutexLock.unlock();
#endif

    VectorSP id = Util::createVector(DT_SYMBOL, num);
    VectorSP VecTime = Util::createVector(DT_TIMESTAMP, num);
    int index = 0;

    int numOfBlock = blockNum.size();

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

    DictionarySP ret = Util::createDictionary(DT_STRING, NULL, DT_ANY, NULL);
    ret->set(new String("size"), new Long(reclenIndex));
    ret->set(new String("parseChunkProcessedCount"), new Long(blockNum.size()));

    if (col.isNull()) {
        return ret;
    }

    cols.push_back(id);
    cols.push_back(VecTime);
    cols.push_back(col);
    vector<string> colName = {"id", "time", "value"};
    TableSP tmpTable;
    tmpTable = Util::createTable(colName, cols);
    ret->set(new String("data"), tmpTable);

    VectorSP metaSid = Util::createVector(DT_STRING, 0, numOfBlock);
    metaSid->appendString(vecId.data(), numOfBlock);

    VectorSP metaStartTime = Util::createVector(DT_TIMESTAMP, 0, numOfBlock);
    vector<long long> vecSingleTime(numOfBlock);
    for (int i = 0; i < numOfBlock; ++i) {
        vecSingleTime[i] = vecTime[i];
    }
    metaStartTime->appendLong(vecSingleTime.data(), numOfBlock);

    VectorSP metaReceived = Util::createVector(DT_TIMESTAMP, numOfBlock, numOfBlock);
    ConstantSP curentTime = new Long(Util::getEpochTime());
    metaReceived->fill(0, numOfBlock, curentTime);

    VectorSP metaSample = Util::createVector(DT_INT, 0, numOfBlock);
    metaSample->appendInt(blockNum.data(), numOfBlock);

    VectorSP vecExpectedCount = Util::createVector(DT_INT, 0, numOfBlock);
    vecExpectedCount->appendInt(expectedCount.data(), numOfBlock);

    VectorSP vecSampleRate = Util::createVector(DT_DOUBLE, 0, numOfBlock);
    vecSampleRate->appendDouble(samprate.data(), numOfBlock);

    colName = {"id", "startTime", "receivedTime", "actualCount", "expectedCount", "sampleRate"};
    cols.clear();
    cols.push_back(metaSid);
    cols.push_back(metaStartTime);
    cols.push_back(metaReceived);
    cols.push_back(metaSample);
    cols.push_back(vecExpectedCount);
    cols.push_back(vecSampleRate);
    tmpTable = Util::createTable(colName, cols);
    ret->set(new String("metaData"), tmpTable);

    return ret;
}

ConstantSP mseedParseStreamInfo(Heap *heap, vector<ConstantSP> &args){
    if (!(
            (args[0]->getType() == DT_STRING && args[0]->getForm() == DF_SCALAR) ||
            (args[0]->getType() == DT_CHAR && args[0]->getForm() == DF_VECTOR)))
        throw IllegalArgumentException(__FUNCTION__, "Data must be a string scalar or a character vector");
    vector<char> bufferVector;
    long long size = 1;
    if (args[0]->getType() == DT_STRING) {
        size = 1;
    }else{
        size = args[0]->size();
        if(size == 0){
            throw IllegalArgumentException(__FUNCTION__, "Vector size can't be zero");
        }else if(size > MAX_STACK_BUFFER_SIZE){
            size = 0;
        }
    }
    char bufferTemp[size];
    char* buffer;
    std::string data;
    if (args[0]->getType() == DT_STRING) {
        data = args[0]->getString();
        size = data.size();
        buffer = (char*)data.c_str();
    } else {
        if(size > 0){
            buffer = bufferTemp;
        }else{
            size = args[0]->size();
            bufferVector.resize(size);
            buffer = bufferVector.data();
        }
        ((VectorSP) args[0])->getChar(0, size, (char *) buffer);
    }
    vector<string> sidVec;
    vector<int> blockVec;
    int index = 0;
    uint8_t formatversion;
    int complete = 0;
    int len = -1;
    while(index < size){
        len = ms3_detect(buffer + index, size - index, &formatversion);
        if(len <= 0){
            if (index == 0)
                throw IllegalArgumentException(__FUNCTION__ , "invalid data.");
            break;
        }
        char sid[50];
        char * flag = ms2_recordsid(buffer + index, sid, 50);
        if(flag == nullptr) {
            if (index == 0)
                throw IllegalArgumentException(__FUNCTION__, "invalid sid.");
            break;
        }
        blockVec.push_back(len);
        sidVec.push_back(sid);
        ++complete;
        index += len;
    }
    if(index > size) {
        index -= len;
        blockVec.pop_back();
        sidVec.pop_back();
        complete--;
    }
    DictionarySP ret = Util::createDictionary(DT_STRING, NULL, DT_ANY, NULL);
    ret->set(new String("size"), new Int(index));
    int dataSize = complete;
    VectorSP sidVector = Util::createVector(DT_STRING, dataSize, dataSize);
    VectorSP blockVector = Util::createVector(DT_INT, dataSize, dataSize);
    sidVector->setString(0, dataSize, sidVec.data());
    blockVector->setInt(0, dataSize, blockVec.data());
    vector<ConstantSP> cols = {sidVector, blockVector};
    vector<string> colNames = {"sid", "blockLen"};
    TableSP t = Util::createTable(colNames, cols);
    ret->set(new String("data"), t);
    return ret;
}