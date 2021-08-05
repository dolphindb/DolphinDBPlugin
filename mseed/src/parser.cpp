#include "parser.h"
#include "ScalarImp.h"
#include "Util.h"
#include "libmseed.h"

using namespace std;
Mutex mutexLock;

#ifndef LINUX
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

extern bool processFirstBlock(MS3Record *msr, char &typeStr, VectorSP &col);

extern int processOneBlock(MS3Record *msr, VectorSP &value, vector<string> &sBuffer, char type);

ConstantSP mseedParse(Heap *heap, vector<ConstantSP> &args) {
    if (!(
            (args[0]->getType() == DT_STRING && args[0]->getForm() == DF_SCALAR) ||
            (args[0]->getType() == DT_CHAR && args[0]->getForm() == DF_VECTOR)))
        throw IllegalArgumentException(__FUNCTION__, "Data must be a string scalar or a character vector");
    FILE *fp = NULL;
    std::shared_ptr<char> ptr;
    char *buffer = NULL;
#ifdef LINUX
    if (args[0]->getType() == DT_STRING) {
        std::string data = args[0]->getString();
        int size = data.size();
        try {
            buffer = (char *) malloc(size * sizeof(char));
        }
        catch (std::bad_alloc) {
            throw RuntimeException(
                    "The given miniSEED byte stream data cannot be parsed because there is not enough memory");
        }
        if (buffer == NULL) {
            throw RuntimeException(
                    "The given miniSEED byte stream data cannot be parsed because there is not enough memory");
        }
        std::shared_ptr<char> sptr(buffer);
        ptr = sptr;
        const char *cdata = data.c_str();
        for (int i = 0; i < size; ++i) {
            buffer[i] = cdata[i];
        }
        fp = fmemopen((void *) buffer, size, "rw");
    } else {
        int size = args[0]->size();
        try {
            buffer = (char *) malloc(size * sizeof(char));
        }
        catch (std::bad_alloc) {
            throw RuntimeException(
                    "The given miniSEED byte stream data cannot be parsed because there is not enough memory");
        }
        if (buffer == NULL) {
            throw RuntimeException(
                    "The given miniSEED byte stream data cannot be parsed because there is not enough memory");
        }
        std::shared_ptr<char> sptr(buffer);
        ptr = sptr;
        ((VectorSP) args[0])->getChar(0, size, (char *) buffer);
        fp = fmemopen((void *) buffer, size, "rw");
    }
#else
    if (args[0]->getType() == DT_STRING)
    {
        std::string data = args[0]->getString();
        int size = data.size();
        try{
            buffer = (char *)malloc(size * sizeof(char));
        }
        catch(std::bad_alloc){
            throw RuntimeException("The given miniSEED byte stream data cannot be parsed because there is not enough memory");
        }
        if (buffer == NULL)
        {
            throw RuntimeException("The given miniSEED byte stream data cannot be parsed because there is not enough memory");
        }
        std::shared_ptr<char> sptr(buffer);
        ptr = sptr;
        const char *cdata = data.c_str();
        for (int i = 0; i < size; ++i)
        {
            buffer[i] = cdata[i];
        }
        mutexLock.lock();
        fp = SCFmemopen((void *)buffer, size, "rb");
        if (fp == NULL)
        {
            mutexLock.unlock();
            throw RuntimeException("Out the memory");
        }
    }
    else
    {
        int size = args[0]->size();
        try{
        buffer = (char *)malloc(size * sizeof(char));
        }
        catch(std::bad_alloc){
            throw RuntimeException("The given miniSEED byte stream data cannot be parsed because there is not enough memory");
        }
        if (buffer == NULL)
        {
            throw RuntimeException("The given miniSEED byte stream data cannot be parsed because there is not enough memory");
        }
        std::shared_ptr<char> sptr(buffer);
        ptr = sptr;
        ((VectorSP)args[0])->getChar(0, size, (char *)buffer);
        mutexLock.lock();
        fp = SCFmemopen((void *)buffer, size, "rb");
        if (fp == NULL)
        {
            mutexLock.unlock();
            throw RuntimeException("The given miniSEED byte stream data cannot be parsed because there is not enough memory");
        }
    }
#endif
    MS3Record *msr = nullptr;
    int retcode;
    static uint32_t flags = MSF_VALIDATECRC | MSF_PNAMERANGE | MSF_UNPACKDATA;
    int mIndex = 1024;

    bool first = true;
    vector<string> sBuffer(mIndex);
    vector<ConstantSP> cols;
    VectorSP col;
    char type;
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
                processFirstBlock(msr, type, col);
                first = false;
            }

            processOneBlock(msr, col, sBuffer, type);

            int len = msr->samplecnt;
            num += len;

            blockNum.push_back(len);
            vecId.push_back(string(msr->sid));
            vecTime.push_back(msr->starttime);
            samprate.push_back(msr->samprate);
        }
    }
    catch (RuntimeException &e) {
        ms3_readmsr(&msr, NULL, NULL, NULL, flags, 0, NULL, &dmsfp);
#ifndef LINUX
        mutexLock.unlock();
#endif
        throw e;
    }
    ms3_readmsr(&msr, NULL, NULL, NULL, flags, 0, NULL, &dmsfp);
#ifndef LINUX
    mutexLock.unlock();
#endif

    VectorSP id = Util::createVector(DT_SYMBOL, num);
    VectorSP VecTime = Util::createVector(DT_TIMESTAMP, num);
    int index = 0;
    for (size_t i = 0; i < blockNum.size(); ++i) {
        int curNum = blockNum[i];
        string curId = vecId[i];
        id->fill(index, curNum, new String(curId));

        long long curStart = vecTime[i];
        long long step = 1000 / samprate[i];
        long long buffer[Util::BUF_SIZE];
        int lines = curNum / Util::BUF_SIZE;
        int line = curNum % Util::BUF_SIZE;
        for (int x = 0; x < lines; ++x) {
            long long *p = VecTime->getLongBuffer(index + x * Util::BUF_SIZE, Util::BUF_SIZE, buffer);
            for (int y = 0; y < Util::BUF_SIZE; ++y) {
                p[y] = curStart;
                curStart += step;
            }
            VecTime->setLong(index, Util::BUF_SIZE, p);
        }
        long long *p = VecTime->getLongBuffer(index + lines * Util::BUF_SIZE, line, buffer);
        for (int y = 0; y < line; ++y) {
            p[y] = curStart;
            curStart += step;
        }
        VecTime->setLong(index, line, p);
        index += line;
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

void processOneBlockStream(MS3Record *msr, VectorSP &value, vector<string> &sBuffer, char type) {
    int len = min(msr->numsamples, msr->samplecnt);
    char *ptr = (char *) (msr->datasamples);
    switch (type) {
        case 'a': {
            if (msr->sampletype != 'a')
                throw RuntimeException("Can not convert from asill type in  miniSEED into INT type in DolphinDB.");
            sBuffer.push_back((char *) ptr);
        }
        case 'i': {
            int buffer[len];
            switch (msr->sampletype) {
                case 'a':
                    throw RuntimeException("Can not convert from asill type in miniSEED into INT type in DolphinDB.");
                    break;
                case 'f': {
                    for (int i = 0; i < len; ++i) {
                        buffer[i] = ((float *) ptr)[i];
                    }
                    value->appendInt(buffer, len);
                    break;
                }
                case 'd': {
                    for (int i = 0; i < len; ++i) {
                        buffer[i] = ((double *) ptr)[i];
                    }
                    value->appendInt(buffer, len);
                    break;
                }
                case 'i':
                    value->appendInt((int *) ptr, len);
                    break;
                default:
                    break;
            }
            break;
        }
        case 'f': {
            float buffer[len];
            switch (msr->sampletype) {
                case 'a':
                    throw RuntimeException("Can not convert from asill type in miniSEED into FLOAT type in DolphinDB.");
                    break;
                case 'i': {
                    for (int i = 0; i < len; ++i) {
                        buffer[i] = ((int *) ptr)[i];
                    }
                    value->appendFloat(buffer, len);
                    break;
                }
                case 'd': {
                    for (int i = 0; i < len; ++i) {
                        buffer[i] = ((double *) ptr)[i];
                    }
                    value->appendFloat(buffer, len);
                    break;
                }
                case 'f':
                    value->appendFloat((float *) ptr, len);
                    break;
                default:
                    break;
            }
            break;
        }
        case 'd': {
            double buffer[len];
            switch (msr->sampletype) {
                case 'a':
                    throw RuntimeException(
                            "Can not convert from asill type in miniSEED into DOUBLE type in DolphinDB.");
                    break;
                case 'i': {
                    for (int i = 0; i < len; ++i) {
                        buffer[i] = ((int *) ptr)[i];
                    }
                    value->appendDouble(buffer, len);
                    break;
                }
                case 'f': {
                    for (int i = 0; i < len; ++i) {
                        buffer[i] = ((float *) ptr)[i];
                    }
                    value->appendDouble(buffer, len);
                    break;
                }
                case 'd':
                    value->appendDouble((double *) ptr, len);
                    break;
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
}

ConstantSP mseedParseStream(Heap *heap, vector<ConstantSP> &args) {
    if (!(
            (args[0]->getType() == DT_STRING && args[0]->getForm() == DF_SCALAR) ||
            (args[0]->getType() == DT_CHAR && args[0]->getForm() == DF_VECTOR)))
        throw IllegalArgumentException(__FUNCTION__, "Data must be a string scalar or a character vector");
    FILE *fp = NULL;
    int size = 0;
    std::shared_ptr<char> ptr;
    char *buffer = NULL;
#ifdef LINUX
    if (args[0]->getType() == DT_STRING) {
        std::string data = args[0]->getString();
        size = data.size();
        try {
            buffer = (char *) malloc(size * sizeof(char));
        }
        catch (std::bad_alloc) {
            throw RuntimeException(
                    "The given miniSEED byte stream data cannot be parsed because there is not enough memory");
        }
        if (buffer == NULL) {
            throw RuntimeException(
                    "The given miniSEED byte stream data cannot be parsed because there is not enough memory");
        }
        std::shared_ptr<char> sptr(buffer);
        ptr = sptr;
        const char *cdata = data.c_str();
        for (int i = 0; i < size; ++i) {
            buffer[i] = cdata[i];
        }
        fp = fmemopen((void *) buffer, size, "rw");
    } else {
        size = args[0]->size();
        try {
            buffer = (char *) malloc(size * sizeof(char));
        }
        catch (std::bad_alloc) {
            throw RuntimeException(
                    "The given miniSEED byte stream data cannot be parsed because there is not enough memory");
        }
        if (buffer == NULL) {
            throw RuntimeException(
                    "The given miniSEED byte stream data cannot be parsed because there is not enough memory");
        }
        std::shared_ptr<char> sptr(buffer);
        ptr = sptr;
        ((VectorSP) args[0])->getChar(0, size, (char *) buffer);
        fp = fmemopen((void *) buffer, size, "rw");
    }
#else
    if (args[0]->getType() == DT_STRING)
    {
        std::string data = args[0]->getString();
        size = data.size();
        try{
            buffer = (char *)malloc(size * sizeof(char));
        }
        catch(std::bad_alloc){
            throw RuntimeException("The given miniSEED byte stream data cannot be parsed because there is not enough memory");
        }
        if (buffer == NULL)
        {
            throw RuntimeException("The given miniSEED byte stream data cannot be parsed because there is not enough memory");
        }
        std::shared_ptr<char> sptr(buffer);
        ptr = sptr;
        const char *cdata = data.c_str();
        for (int i = 0; i < size; ++i)
        {
            buffer[i] = cdata[i];
        }
        mutexLock.lock();
        fp = SCFmemopen((void *)buffer, size, "rb");
        if (fp == NULL)
        {
            mutexLock.unlock();
            throw RuntimeException("Out the memory");
        }
    }
    else
    {
        size = args[0]->size();
        try{
        buffer = (char *)malloc(size * sizeof(char));
        }
        catch(std::bad_alloc){
            throw RuntimeException("The given miniSEED byte stream data cannot be parsed because there is not enough memory");
        }
        if (buffer == NULL)
        {
            throw RuntimeException("The given miniSEED byte stream data cannot be parsed because there is not enough memory");
        }
        std::shared_ptr<char> sptr(buffer);
        ptr = sptr;
        ((VectorSP)args[0])->getChar(0, size, (char *)buffer);
        mutexLock.lock();
        fp = SCFmemopen((void *)buffer, size, "rb");
        if (fp == NULL)
        {
            mutexLock.unlock();
            throw RuntimeException("The given miniSEED byte stream data cannot be parsed because there is not enough memory");
        }
    }
#endif
    MS3Record *msr = nullptr;
    int retcode;
    static uint32_t flags = MSF_VALIDATECRC | MSF_PNAMERANGE | MSF_UNPACKDATA;
    int mIndex = 1024;

    bool first = true;
    vector<string> sBuffer(mIndex);
    vector<ConstantSP> cols;
    VectorSP col;
    char type;
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
            if (first) {
                processFirstBlock(msr, type, col);
                first = false;
            }

            processOneBlockStream(msr, col, sBuffer, type);

            int len = min(msr->numsamples, msr->samplecnt);
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
#ifndef LINUX
        mutexLock.unlock();
#endif
        throw e;
    }
    if(msr!= nullptr && msr->reclen > 0 && reclenIndex == 0 && msr->reclen <= size)
        reclenIndex = msr->reclen;
    ms3_readmsr(&msr, NULL, NULL, NULL, flags, 0, NULL, &dmsfp);
    msr3_free(&msr);
#ifndef LINUX
    mutexLock.unlock();
#endif

    VectorSP id = Util::createVector(DT_SYMBOL, num);
    VectorSP VecTime = Util::createVector(DT_TIMESTAMP, num);
    int index = 0;

    int numOfBlock = blockNum.size();

    for (size_t i = 0; i < blockNum.size(); ++i) {
        int curNum = blockNum[i];
        string curId = vecId[i];
        id->fill(index, curNum, new String(curId));

        long long curStart = vecTime[i];
        long long step = 1000 / samprate[i];
        long long buffer[Util::BUF_SIZE];
        int lines = curNum / Util::BUF_SIZE;
        int line = curNum % Util::BUF_SIZE;
        for (int x = 0; x < lines; ++x) {
            long long *p = VecTime->getLongBuffer(index + x * Util::BUF_SIZE, Util::BUF_SIZE, buffer);
            for (int y = 0; y < Util::BUF_SIZE; ++y) {
                p[y] = curStart;
                curStart += step;
            }
            VecTime->setLong(index, Util::BUF_SIZE, p);
        }
        long long *p = VecTime->getLongBuffer(index + lines * Util::BUF_SIZE, line, buffer);
        for (int y = 0; y < line; ++y) {
            p[y] = curStart;
            curStart += step;
        }
        VecTime->setLong(index, line, p);
        index += line;
    }

    DictionarySP ret = Util::createDictionary(DT_STRING, NULL, DT_ANY, NULL);
    ret->set(new String("size"), new Long(reclenIndex));
    ret->set(new String("parseChunkProcessedCount"), new Long(blockNum.size()));

    if (col.isNull())
        return ret;

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