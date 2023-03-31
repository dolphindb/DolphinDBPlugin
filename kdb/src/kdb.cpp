#include <stdio.h>
#include <vector>
#include <iostream>
#include "Logger.h"

#include "kdb.h"
#include <Concurrent.h>
#include <Exceptions.h>
#include <ScalarImp.h>
#include <Util.h>

#include <map>
#include <ctime>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <zlib.h>
#include <ctime>

using namespace std;

// parameters for gzip process
#define WINDOWS_BITS 15
#define ENABLE_ZLIB_GZIP 32
#define GZIP_ENCODING 16

ConstantSP messageSP(const std::string &str) {
    auto message = Util::createConstant(DT_STRING);
    message->setString(str);
    return message;
}

Mutex kdbMutex;
ConstantSP safeOp(const ConstantSP &arg, std::function<ConstantSP(Connection *)> &&f) {
    if (arg->getType() == DT_RESOURCE && ((Connection *)(arg->getLong()) != nullptr)) {
        auto conn = (Connection *)(arg->getLong());
        return conn->connected() ? f(conn) : messageSP("Not connected yet.");
    } else {
        throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
    }
}

Connection::Connection(string hostStr, int port, string usernamePassword) {
    host_ = hostStr;
    port_ = port;

    char* host = const_cast<char*>(hostStr.c_str());
    char* usr = const_cast<char*>(usernamePassword.c_str());

    I handle = khpu(host, port, usr);
    if(handle == 0) {
        throw RuntimeException("Authentication error.");
    } else if(handle == -1) {
        throw RuntimeException("Connection error.");
    } else if (handle == -2) {
        throw RuntimeException("Connection time out.");
    }
    handle_ = handle;
    connected_ = true;
}

Connection::~Connection() {
    kclose(handle_);
}

bool Connection::connected() {
    return connected_;
}

static void kdbConnectionOnClose(Heap *heap, vector<ConstantSP> &args) {
    Connection* conn = (Connection*)(args[0]->getLong());
    if(conn!= nullptr) {
        delete (Connection *) (args[0]->getLong());
        args[0]->setLong(0);
    }
}

K Connection::kdbRun(string command) {
    char * arg = const_cast<char*>(command.c_str());
    K res = k(handle_, arg,(K)0);
    if(!res) {
        throw RuntimeException("kdb+ execution error. fail to execute " + command + ".");
    }
    if(res->t == -128) {
        string errMsg = res->s;
        throw RuntimeException("kdb+ execution error " + errMsg + ".");
    }
    return res;
}


TableSP Connection::getTable(string tablePath, string symFilePath) {
    LockGuard<Mutex> guard(&kdbMutex);
    // load symbol
    // must use the file name to load sym in kdb+
    // otherwise the serialized data could not find its sym list
    string symName = "";
    if(symFilePath.size() > 0) {
        vector<string> fields;
        Util::split(symFilePath, '/', fields);
        if(fields.size() == 0) {
            throw RuntimeException("Invalid symFilePath");
        }
        symName = fields.back();
        // load sym to kdb
        string arg = symName + ":get`:" + symFilePath;
        kdbRun(arg);
    }

    // make sure the last char is not '/'
    if(tablePath[tablePath.size()-1] == '/') {
        tablePath = tablePath.substr(0, tablePath.size()-1);
    }

    // load table
    string loadCommand = "\\l " + tablePath;
    char * loadArg = const_cast<char*>(loadCommand.c_str());
    K loadRes = k(handle_, loadArg,(K)0);
    if(!loadRes) {
        throw RuntimeException("kdb+ execution error. fail to execute " + loadCommand + ".");
    }
    if(loadRes->t == -128) {
        string errMsg = loadRes->s;
        throw RuntimeException("load table failed: " + errMsg + ".");
    }

    // split table path, get table name, get cols
    vector<string> pathVec = Util::split(tablePath, '/');
    string tableName = pathVec[pathVec.size()-1];
    string colsCommand = "cols " + tableName;
    char * colsArg = const_cast<char*>(colsCommand.c_str());
    K colsRes = k(handle_, colsArg,(K)0);
    if(!colsRes) {
        throw RuntimeException("kdb+ execution error. fail to execute " + colsCommand + ".");
    }
    if(colsRes->t == -128) {
        string errMsg = colsRes->s;
        throw RuntimeException("get table failed: " + errMsg + ".");
    }
    
    int colNum = colsRes->n;
    vector<string> colNames(colNum);
    vector<ConstantSP> cols(colNum);
    for(int i = 0; i < colNum; i++) {
        colNames[i] = kS(colsRes)[i];
    }

    for(int i = 0; i < colNum; i++) {
        string queryCommand = "select " +  colNames[i] + " from " + tableName;
        char * queryArg = const_cast<char*>(queryCommand.c_str());
        K colRes = k(handle_, queryArg,(K)0);
        if(!colsRes) {
            throw RuntimeException("kdb+ execution error. fail to execute " + colsCommand + ".");
        }
        if(colRes->t == -128) {
            string errMsg = colRes->s;
            throw RuntimeException("kdb+ execution error " + errMsg + ".");
        }

        int type = kK(kK(colRes->k)[1])[0]->t;
        long long length = (long long)(kK(kK(colRes->k)[1])[0]->n);

        void* ptr;
        switch(type) {
            case KDB_LIST:
                throw RuntimeException("can't parse column " + colNames[i] + ", it's a nested column.");
                break;
            case KDB_BOOL:
                ptr = kG(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_BOOL,0,length);
                ((VectorSP)cols[i])->appendBool((char *)ptr, length);
                break;
            case KDB_GUID:
                ptr = kG(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_UUID,0,length);
                for(int j = 0; j < length; j++) {
                    int pos = j*16;
                    for(int h = 0; h < 8; h++) {
                        char tmp = ((char* )ptr)[pos+15-h];
                        ((char* )ptr)[pos+15-h] = ((char* )ptr)[pos+h];
                        ((char* )ptr)[pos+h] = tmp;
                    }
                }
                ((VectorSP)cols[i])->appendGuid((Guid *)ptr, length);
                break;
            case KDB_BYTE:
                // DolphinDB don't have type of byte, so convert byte to char
                {
                    ptr = kG(kK(kK(colRes->k)[1])[0]);
                    cols[i] = Util::createVector(DT_CHAR,0,length);
                    ((VectorSP)cols[i])->appendChar((char *)ptr, length);
                }
                break;
            case KDB_SHORT:
                ptr = kH(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_SHORT,0,length);
                ((VectorSP)cols[i])->appendShort((short *)ptr, length);
                break;
            case KDB_INT:
                ptr = kI(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_INT, 0, length);
                ((VectorSP)cols[i])->appendInt((int *)ptr, length);
                break;
            case KDB_LONG:
                ptr = kJ(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_LONG,0,length);
                ((VectorSP)cols[i])->appendLong((long long *)ptr, length);
                break;
            case KDB_FLOAT:
            {
                ptr = kE(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_FLOAT, 0, length);

                Float nan = Float();
                nan.setNull();
                double nanValue = nan.getFloat();
                for(int j = 0; j < length; j++) {
                    if(((unsigned int *)ptr)[j] == 4290772992) {
                        ((float *)ptr)[j] = nanValue;
                    }
                }

                ((VectorSP)cols[i])->appendFloat((float *)ptr, length);
                break;
            }
            case KDB_DOUBLE:
            {
                ptr = kF(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_DOUBLE,0,length,true);
                Double nan = Double();
                nan.setNull();
                double nanValue = nan.getDouble();
                for(int j = 0; j < length; j++) {
                    if(((unsigned long long *)ptr)[j] == 18444492273895866368ULL) {
                        ((double *)ptr)[j] = nanValue;
                    }
                }
                ((VectorSP)cols[i])->appendDouble((double *)ptr, length);
                break;
            }
            case KDB_CHAR:
                ptr = kG(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_CHAR,0,length,true);
                ((VectorSP)cols[i])->appendChar((char *)ptr, length);
                break;
            case KDB_STRING:
                ptr = kS(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_SYMBOL,0,length);
                ((VectorSP)cols[i])->appendString((const char**)ptr, length);
                break;
            case KDB_TIMESTAMP:
            {
                ptr = kJ(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_NANOTIMESTAMP,0,length);
                unsigned long long nan = 1;
                nan = nan << 63;
                for(int j = 0; j < length; j++) {
                    // kdb+ time starts at 2000.01.01
                    // while dolphinDB start at 1970.01.01
                    // their nano time stamp gap 946684800000000000
                    // following time types as same
                    if(((unsigned long long *)ptr)[j] == nan) {
                        continue;
                    }
                    ((long long *)ptr)[j] += 946684800000000000;
                }
                ((VectorSP)cols[i])->appendLong((long long *)ptr, length);
                break;
            }
            case KDB_MONTH:
            {
                ptr = kI(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_MONTH, 0, length);
                unsigned int nan = 1<<31;
                for(int j = 0; j < length; j++) {
                    if(((unsigned int *)ptr)[j] == nan) {
                        continue;
                    } 
                    ((int *)ptr)[j] += 24000;
                }
                ((VectorSP)cols[i])->appendInt((int *)ptr, length);
                break;
            }

            case KDB_DATE:
            {
                ptr = kI(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_DATE, 0, length);
                unsigned int nan = 1<<31;
                for(int j = 0; j < length; j++) {
                    if(((unsigned int *)ptr)[j] == nan) {
                        continue;
                    }
                    ((int *)ptr)[j] += 10957;
                }
                ((VectorSP)cols[i])->appendInt((int *)ptr, length);
                break;
            }

            case KDB_DATETIME:
            {
                // kdb+'s datetime is double, start from 2000.01.01T00:00:00.000
                ptr = kF(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_TIMESTAMP, 0, length);
                vector<long long> longVec(length);
                for(int j = 0; j < length; j++) {
                    longVec[j] = ((double *)ptr)[j]*86400000 + 946684800000;
                }
                ((VectorSP)cols[i])->appendLong((long long *)(longVec.data()), length);
            }
                break;
            case KDB_TIMESPAN:
                ptr = kJ(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_NANOTIME,0,length);
                ((VectorSP)cols[i])->appendLong((long long *)ptr, length);
                break;
            case KDB_MINUTE:
                ptr = kI(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_MINUTE, 0, length);
                ((VectorSP)cols[i])->appendInt((int *)ptr, length);
                break;
            case KDB_SECOND:
                ptr = kI(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_SECOND, 0, length);
                ((VectorSP)cols[i])->appendInt((int *)ptr, length);
                break;
            case KDB_TIME:
                ptr = kI(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_TIME, 0, length);
                ((VectorSP)cols[i])->appendInt((int *)ptr, length);
                break;
            default:
                throw RuntimeException("Can't parse column " + colNames[i] + " .");
        }
        ((VectorSP)cols[i])->setNullFlag(((VectorSP)cols[i])->hasNull());
        r0(colRes);
        LOG_INFO(colNames[i] + " " + to_string(((VectorSP)cols[i])->size()) + ".");
    }
    r0(colsRes);
    //drop table, release memory in kdb+
    string dropCommand = tableName + ":0";
    char* dropArg = const_cast<char*>(dropCommand.c_str());
    K dropRes = k(handle_, dropArg,(K)0);
    if(!dropRes) {
        LOG_INFO("kdb+ execution error. fail to execute " + dropCommand + ".");
    }
    if(dropRes->t == -128) {
        string errMsg = dropRes->s;
        LOG_INFO("drop table failed: " + errMsg + ".");
    }

    // drop sym table
    dropCommand = symName + ":0";
    dropArg = const_cast<char*>(dropCommand.c_str());
    dropRes = k(handle_, dropArg,(K)0);
    if(!dropRes) {
        LOG_INFO("kdb+ execution error. fail to execute " + dropCommand + ".");
    }
    if(dropRes->t == -128) {
        string errMsg = dropRes->s;
        LOG_INFO("drop sym failed: " + errMsg + ".");
    }

    return Util::createTable(colNames, cols);
}


/*
 * we can't get endian info from binary file
 * the machine that persisted kdb+ file and the machine that load file into DolphinDB
 * should have same endian
 */

// decode short
short rh(unsigned char* src, int pos) {
    return ((short*)(src+pos))[0];
}

// decode int
int ri(unsigned char* src, int pos) {
    return ((int*)(src+pos))[0];
}

// decode long
long long rl(unsigned char* src, int pos) {
    return ((long long*)(src+pos))[0];
}

// decode double
double rd(unsigned char* src, int pos) {
    long long num = rl(src, pos);
    return reinterpret_cast<double&>(num);
}

int decompress(unsigned char *src, size_t src_len, unsigned char *dest, size_t dest_len, int compressType)
{
    if(compressType == 0) {
        memcpy(dest, src, src_len);
        return src_len;
    }
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.next_in = src;
    strm.avail_in = src_len;
    strm.next_out = dest;
    strm.avail_out = dest_len;
    if (inflateInit2 (& strm, WINDOWS_BITS | ENABLE_ZLIB_GZIP) < 0){
        return -2;
        // throw RuntimeException("Depression failed, can't init inflate");
    }
    int res = inflate (& strm, Z_NO_FLUSH);

    if (res < 0){
        return -2;
        // throw RuntimeException("Depression failed, can't inflate, err " + to_string(res) + ".");
    }
    inflateEnd (& strm);
    return dest_len - strm.avail_out;
}

int decompress(FILE * fp, vector<unsigned char>& dest) {
    fseek(fp, 0, SEEK_END);
    int file_len = ftell(fp)-8;
    
    fseek(fp, 8, SEEK_SET);
    // src = (unsigned char*)malloc(file_len);
    vector<unsigned char> srcVec;
    srcVec.resize(file_len);
    unsigned char *src;
    src = srcVec.data();
    int bytesRead = fread(src, 1, file_len, fp);

    // read meta data of compressed file
    // read block num of compressed file
    long long blockSize = rl(src, file_len-8);
    // LOG_INFO("size: " + to_string(blockSize)+".");
    int bufPos = file_len-8-8*blockSize - 32;

    // read compress type&level
    bufPos+=4;
    int compressType = src[bufPos];
    bufPos+=1;
    int compressLevel = src[bufPos];
    if(compressType == 1 || compressType == 3 || compressType == 4) {
        return -3;
        // throw RuntimeException("Unsupported kdb compress type, please use another way to load data.");
    }

    // read uncompress size
    bufPos+=3;
    long long originSize = rl(src, bufPos);
    bufPos+=8;
    long long compressSize = rl(src, bufPos);
    bufPos+=8;
    long long originBlockSize = rl(src, bufPos);
    
    // read every compress block size
    // TODO, maybe different blocks would have different compress type
    vector<pair<int, int>> blockVec(blockSize);
    for(int i = 0; i < blockSize; i++) {
        bufPos+=8;
        int len = ri(src, bufPos);
        int type = ri(src, bufPos+4);
        blockVec[i] = pair<int, int>{len, type};
    }
    
    // unsigned char *dest;
    // dest = (unsigned char*)malloc(originBlockSize * blockSize);
    dest.resize(originBlockSize * blockSize);
    // unsigned char *destRet = dest;
    int offset = 0;
    for(int i = 0; i < blockSize; i++) {
        int decompressSize = decompress(src,blockVec[i].first,dest.data()+offset,originBlockSize, blockVec[i].second);
        if (decompressSize == -1) {
            return -1;
            // throw RuntimeException("Depression failed, can't init inflate");
        } else if (decompressSize == -2) {
            return -2;
            // throw RuntimeException("Depression failed, can't inflate.");
        }
        src+=blockVec[i].first;
        // dest+=decompressSize;
        offset+=decompressSize;
    }

    // return pair<unsigned char*, int>(destRet, originSize);
    return int(originSize);
}

vector<string> loadSym(string symSrc) {
    vector<string> symVec;
    FILE * fp = fopen (symSrc.c_str(), "rb");
     if (!fp){
        throw RuntimeException("Open sym file failed\n");
        return symVec;
    }
    fseek(fp, 0, SEEK_END);
    int file_len = ftell(fp)-8;
    fseek(fp, 8, SEEK_SET);
    vector<unsigned char> srcVec;
    srcVec.resize(file_len);
    unsigned char *src = srcVec.data();
    // src = (unsigned char*)malloc(file_len);
    
    int bytesRead = fread (src, 1, file_len, fp);
    fclose(fp);
    
    vector<char> charVec;
    for(int i = 0; i < file_len; i++) {
        if(src[i] == '\0') {
            string str(charVec.begin(), charVec.end());
            symVec.push_back(str);
            charVec = vector<char>();
        } else {
            charVec.push_back(src[i]);
        }
    }
    
    // free(src);
    return symVec;
}

ConstantSP loadSplayedCol(string, vector<string>&);

class GetColRunnable: public Runnable {
    public:
        GetColRunnable( vector<ConstantSP>& cols, int colIndex, string colPath, vector<string>& symList): cols_(cols), colIndex_(colIndex), colPath_(colPath), symList_(symList){
        }
    void run() {
        ConstantSP vec;
        try {
            vec = loadSplayedCol(colPath_, symList_);
        } catch(std::exception &exception) {
            vec = new String(exception.what());
        }
        
        vec->setNullFlag(true);
        cols_[colIndex_] = vec;
    }
    private:
        vector<ConstantSP>& cols_;
        int colIndex_;
        string colPath_;
        vector<string>& symList_;
};

using GetColRunnableSP = SmartPointer<GetColRunnable>;

TableSP loadSplayedTable(string tablePath, vector<string>& symList) {
    if(tablePath[tablePath.size()-1] != '/') {
        tablePath.push_back('/');
    }
    // read .d file, get column names
    string dotD = tablePath + ".d";
    FILE * fp = fopen (dotD.c_str(), "rb");
     if (!fp){
        throw RuntimeException("Open .d file failed, this is not a splayed table\n");
    }
    
    // decompress if compressed
    unsigned char *startSrc;
    startSrc = (unsigned char*)malloc(9);
    int bytesRead = fread(startSrc, 1, 8, fp);
    startSrc[8] = '\0';
    string headStr = "";
    headStr = (char*)startSrc;
    free(startSrc);

    unsigned char *src = new unsigned char;
    vector<unsigned char> srcVec;
    int dep;
    int file_len;
    if(headStr == "kxzipped") {
        //decompress
        dep = decompress(fp, srcVec);
        if(dep == -1) {
            throw RuntimeException(".d file Depression failed, can't init inflate");
        } else if(dep == -2) {
            throw RuntimeException(".d file Depression failed, can't inflate.");
        } else if(dep == -3) {
            throw RuntimeException(".d file Unsupported kdb compress type, please use another way to load data.");
        }
        // skip head 00 completion of file
        src = srcVec.data()+8;
        file_len = dep-8;
    } else {
        fseek(fp, 0, SEEK_SET);
        fseek(fp, 0, SEEK_END);
        file_len = ftell(fp)-8;
        fseek(fp, 8, SEEK_SET);
        srcVec.resize(file_len);
        src = srcVec.data();
        bytesRead = fread(src, 1, file_len, fp);
        fclose(fp);
    }

    // read
    vector<string> colNameVec;
    vector<char> charVec;
    for(int i = 0; i < file_len; i++) {
        if(src[i] == '\0') {
            string str(charVec.begin(), charVec.end());
            colNameVec.push_back(str);
            charVec = vector<char>();
        } else {
            charVec.push_back(src[i]);
        }
    }

    // if(headStr == "kxzipped") {
    //     free(dep.first);
    // } else {
    //     free(src);
    // }
    
    int colNameSize = colNameVec.size();
    vector<ConstantSP> cols(colNameSize);
    // read every column file, decompress if compressed, get column data
    // for(int i = 0; i < colNameSize; i++) {
    //     cols[i] = loadSplayedCol(tablePath+colNameVec[i], symList);
    // }
    // return Util::createTable(colNameVec, cols);

    //parallel version
    vector<ThreadSP> colThreads(colNameSize);
    for(int i = 0; i < colNameSize; i++) {
        GetColRunnableSP runnable = new GetColRunnable(cols, i, tablePath+colNameVec[i], symList);
        ThreadSP thread = new Thread(runnable);
        colThreads[i] = thread;
        if(!thread->isStarted()) {
            thread->start();
        }
    }
    for(auto thread: colThreads) {
        thread->join();
    }
    for(auto col: cols) {
        if(col->getType() == DT_STRING) {
            throw RuntimeException(col->getString());
        }
    }
    TableSP table = Util::createTable(colNameVec, cols);
    return table;
}

ConstantSP loadSplayedCol(string colSrc, vector<string>& symList) {
    FILE * fp = fopen (colSrc.c_str(), "rb");
    if (!fp){
        throw RuntimeException("Open cols " + colSrc + " failed\n");
    } else {
        LOG_INFO("Open "+ colSrc + ".");
    }
    
    // decompress if compressed
    unsigned char *startSrc;
    startSrc = (unsigned char*)malloc(9);
    int bytesRead = fread(startSrc, 1, 8, fp);
    startSrc[8] = '\0';
    string headStr = "";
    headStr = (char*)startSrc;
    free(startSrc);

    unsigned char *src = new unsigned char;
    // pair<unsigned char*, int> dep;
    vector<unsigned char> srcVec;
    int dep;
    int file_len;
    if(headStr == "kxzipped") {
        //decompress
        dep = decompress(fp, srcVec);
        if(dep == -1) {
            return new String("Depression failed, can't init inflate");
        } else if(dep == -2) {
            return new String("Depression failed, can't inflate.");
        } else if(dep == -3) {
            return new String("Unsupported kdb compress type, please use another way to load data.");
        }
        src = srcVec.data();
        file_len = dep;
        fclose(fp);
    } else {
        fseek(fp, 0, SEEK_SET);
        fseek(fp, 0, SEEK_END);
        file_len = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        src = (unsigned char*)malloc(file_len);
        bytesRead = fread(src, 1, file_len, fp);
        fclose(fp);
    }

    int bufPos = 2;
    int type = src[bufPos];
    bufPos = 16;
    switch(type) {
        case 0: // have risk, maybe have some unknown mechanism
        {
            vector<char> charVec(3);
            charVec[0] = src[bufPos];
            charVec[1] = src[bufPos+1];
            charVec[2] = src[bufPos+2];
            string str(charVec.begin(), charVec.end());
            
            if(str == "sym") {
                if(symList.size() == 0) {
                    // free(src);

                    // throw RuntimeException("Sym file hasn't loaded.");
                    LOG_ERR("Sym file hasn't loaded.");
                    return new String("Sym file hasn't loaded.");
                }
                bufPos = 4080;
                // unknow things, maybe useful
                // int flag = src[bufPos+3];
                bufPos+=8;
                int length = rl(src, bufPos);
                bufPos+=8;
                if(length == 0) {
                    length = (file_len - bufPos) / 8;
                }
                LOG_INFO("col length: "+ to_string(length) +".");

                VectorSP vec = Util::createVector(DT_SYMBOL, 0, length, true);
                
                long long * longSrc = reinterpret_cast<long long *>(src + bufPos);
                vector<string> strVec(length);
                int symLength = symList.size();
                for(int i = 0; i < length; i++) {
                    long long index = longSrc[i];
                    if(index == LONG_MIN) {
                        strVec[i] = "";
                    } else {
                        if(index >= symLength) {
                            // free(src);
                            // throw RuntimeException("Exceeding sym file.");
                            LOG_ERR("Exceeding sym file.");
                            return new String("Exceeding sym file.");
                        }
                        strVec[i] = symList[index];
                    }
                }
                vec->appendString(strVec.data(), length);
                // free(src);
                return vec;
            } else {
                // un-match pattern, to be find
                // free(src);
                throw RuntimeException("Sym file parse failed.");
            }
        }
            break;
        case KDB_BOOL:
        {
            int length = (file_len - bufPos) / 1;
            LOG_INFO("col length: "+ to_string(length) +".");
            char * boolSrc = reinterpret_cast<char *>(src + bufPos);
            VectorSP vec = Util::createVector(DT_BOOL, 0, length, true);
            vec->appendBool((char *)boolSrc, length);
            // free(src);
            return vec;
        }
            break;
        case KDB_GUID:
        {
            int length = (file_len - bufPos) / 16;
            LOG_INFO("col length: "+ to_string(length) +".");
            unsigned char * charSrc = reinterpret_cast<unsigned char *>(src+bufPos);
            for(int j = 0; j < length; j++) {
                int pos = j*16;
                for(int h = 0; h < 8; h++) {
                    unsigned char tmp = charSrc[pos+15-h];
                    charSrc[pos+15-h] = charSrc[pos+h];
                    charSrc[pos+h] = tmp;
                }
            }
            VectorSP vec = Util::createVector(DT_UUID, 0, length, true);
            vec->appendGuid((Guid *)charSrc, length);
            // free(src);
            return vec;
        }
            break;
        case KDB_BYTE:
        {
            int length = (file_len - bufPos) / 1;
            LOG_INFO("col length: "+ to_string(length) +".");
            char * charSrc = reinterpret_cast<char *>(src + bufPos);
            VectorSP vec = Util::createVector(DT_CHAR, 0, length, true);
            vec->appendChar((char *)charSrc, length);
            // free(src);
            return vec;
        }
            break;
        case KDB_SHORT:
        {
            int length = (file_len - bufPos) / 2;
            LOG_INFO("col length: "+ to_string(length) +".");
            short * shortSrc = reinterpret_cast<short *>(src + bufPos);
            VectorSP vec = Util::createVector(DT_SHORT, 0, length, true);
            vec->appendShort((short *)shortSrc, length);
            // free(src);
            return vec;
        }
            break;
        case KDB_INT:
        {
            int length = (file_len - bufPos) / 4;
            LOG_INFO("col length: "+ to_string(length) +".");
            int * intSrc = reinterpret_cast<int *>(src + bufPos);
            VectorSP vec = Util::createVector(DT_INT, 0, length, true);
            vec->appendInt((int *)intSrc, length);

            // free(src);
            return vec;
        }
            break;
        case KDB_LONG:
        {
            int length = (file_len - bufPos) / 8;
            LOG_INFO("col length: "+ to_string(length) +".");
            long long * longSrc = reinterpret_cast<long long *>(src + bufPos);
            VectorSP vec = Util::createVector(DT_LONG, 0, length, true);
            vec->appendLong((long long *)longSrc, length);


            
            // free(src);
            return vec;
        }
            break;
        case KDB_FLOAT:
        {
            int length = (file_len - bufPos) / 4;
            LOG_INFO("col length: "+ to_string(length) +".");
            float * floatSrc = reinterpret_cast<float *>(src + bufPos);
            VectorSP vec = Util::createVector(DT_FLOAT, 0, length, true);

            Float nan = Float();
            nan.setNull();
            double nanValue = nan.getFloat();
            for(int i = 0; i < length; i++) {
                if(((unsigned int *)floatSrc)[i] == 4290772992) {
                    floatSrc[i] = nanValue;
                }
            }

            vec->appendFloat((float *)floatSrc, length);
            // free(src);
            return vec;
        }
            break;
        case KDB_DOUBLE:
        {
            int length = (file_len - bufPos) / 8;
            LOG_INFO("col length: "+ to_string(length) +".");
            double * doubleSrc = reinterpret_cast<double *>(src+bufPos);
            VectorSP vec = Util::createVector(DT_DOUBLE, 0, length, true);

            Double nan = Double();
            nan.setNull();
            double nanValue = nan.getDouble();
            for(int i = 0; i < length; i++) {
                if(((unsigned long long *)doubleSrc)[i] == 18444492273895866368ULL) {
                    doubleSrc[i] = nanValue;
                }
            }

            vec->appendDouble((double *)doubleSrc, length);
            // free(src);
            return vec;
        }
            break;
        case KDB_CHAR:
        {
            int length = (file_len - bufPos) / 1;
            LOG_INFO("col length: "+ to_string(length) +".");
            char * charSrc = reinterpret_cast<char *>(src + bufPos);
            VectorSP vec = Util::createVector(DT_CHAR, 0, length, true);
            vec->appendChar((char *)charSrc, length);
            // free(src);
            return vec;
        }
            break;
        case KDB_STRING:
        {
            /*
             * Splayed table mustn't have non-enumerated symbol column.
             * Symbol column must have been enumerated before persisting.
             */
        }
            break;
        case KDB_TIMESTAMP:
        {
            int length = (file_len - bufPos) / 8;
            LOG_INFO("col length: "+ to_string(length) +".");
            long long * longSrc = reinterpret_cast<long long *>(src + bufPos);
            unsigned long long nan = 1;
            nan = nan << 63;
            for(int i = 0; i < length; i++) {
                if(((unsigned long long *)longSrc)[i] == nan) {
                    continue;
                }
                longSrc[i] += 946684800000000000;
            }
            VectorSP vec = Util::createVector(DT_NANOTIMESTAMP, 0, length, true);
            vec->appendLong((long long *)longSrc, length);
            // free(src);
            return vec;
        }
            break;
        case KDB_MONTH:
        {
            int length = (file_len - bufPos) / 4;
            LOG_INFO("col length: "+ to_string(length) +".");
            int * intSrc = reinterpret_cast<int *>(src + bufPos);
            unsigned int nan = 1 << 31;
            for(int i = 0; i < length; i++) {
                if(((unsigned int *)(intSrc))[i] == nan) {
                    continue;
                }
                intSrc[i] += 24000;
            }
            VectorSP vec = Util::createVector(DT_MONTH, 0, length, true);
            vec->appendInt((int *)intSrc, length);
            // free(src);
            return vec;
        }
            break;
        case KDB_DATE:
        {
            int length = (file_len - bufPos) / 4;
            LOG_INFO("col length: "+ to_string(length) +".");
            int * intSrc = reinterpret_cast<int *>(src + bufPos);
            unsigned int nan = 1 << 31;
            for(int i = 0; i < length; i++) {
                if(((unsigned int *)(intSrc))[i] == nan) {
                    continue;
                }
                intSrc[i] += 10957;
            }
            VectorSP vec = Util::createVector(DT_DATE, 0, length, true);
            vec->appendInt((int *)intSrc, length);
            // free(src);
            return vec;
        }
            break;
        case KDB_DATETIME:
        {
                // ptr = kF(kK(kK(colRes->k)[1])[0]);
                // cols[i] = Util::createVector(DT_TIMESTAMP, 0, length);
                // vector<long long> longVec(length);
                // for(int j = 0; j < length; j++) {
                //     longVec[j] = ((double *)ptr)[j]*86400000 + 946684800000;
                // }
                // ((VectorSP)cols[i])->appendLong((long long *)(longVec.data()), length);
            int length = (file_len - bufPos) / 8;
            LOG_INFO("col length: "+ to_string(length) +".");
            double * doubleSrc = reinterpret_cast<double *>(src+bufPos);
            VectorSP vec = Util::createVector(DT_TIMESTAMP, 0, length, true);
            vector<long long> longSrc(length);
            for(int i = 0; i < length; i++) {
                longSrc[i] = (long long)((doubleSrc[i]) * 86400000 + 946684800000);
            }
            vec->appendLong((long long *)(longSrc.data()), length);
            // free(src);
            return vec;
        }
            break;
        case KDB_TIMESPAN:
        {
            int length = (file_len - bufPos) / 8;
            LOG_INFO("col length: "+ to_string(length) +".");
            long long * longSrc = reinterpret_cast<long long *>(src + bufPos);
            VectorSP vec = Util::createVector(DT_NANOTIME, 0, length, true);
            vec->appendLong((long long *)longSrc, length);
            // free(src);
            return vec;
        }
            break;
        case KDB_MINUTE:
        {
            int length = (file_len - bufPos) / 4;
            LOG_INFO("col length: "+ to_string(length) +".");
            int * intSrc = reinterpret_cast<int *>(src + bufPos);
            VectorSP vec = Util::createVector(DT_MINUTE, 0, length, true);
            vec->appendInt((int *)intSrc, length);
            // free(src);
            return vec;
        }
            break;
        case KDB_SECOND:
        {
            int length = (file_len - bufPos) / 4;
            LOG_INFO("col length: "+ to_string(length) +".");
            int * intSrc = reinterpret_cast<int *>(src + bufPos);
            VectorSP vec = Util::createVector(DT_SECOND, 0, length, true);
            vec->appendInt((int *)intSrc, length);
            // free(src);
            return vec;
        }
            break;
        case KDB_TIME:
        {
            int length = (file_len - bufPos) / 4;
            LOG_INFO("col length: "+ to_string(length) +".");
            int * intSrc = reinterpret_cast<int *>(src + bufPos);
            VectorSP vec = Util::createVector(DT_TIME, 0, length, true);
            vec->appendInt((int *)intSrc, length);
            // free(src);
            return vec;
        }
            break;
        default:
            // free(src);
            throw RuntimeException("can't parse column " + colSrc + " .");
    }
    // free(src);
    VectorSP vec = Util::createVector(DT_VOID, 0);
    return vec;
}


ConstantSP kdbConnect(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string("Usage: connect(host, port, usernamePassword). ");
    if(args[0]->getType() != DT_STRING){
        throw IllegalArgumentException(__FUNCTION__, usage + "host should be a string.");
    }
    string hostStr = args[0]->getString();

    if(args[1]->getType() != DT_INT){
        throw IllegalArgumentException(__FUNCTION__, usage + "port should be a integer.");
    }
    I port = args[1]->getInt();

    string usrStr = "";
    if(args.size() == 3) {
        if(args[2]->getType() != DT_STRING){
            throw IllegalArgumentException(__FUNCTION__, usage + "usernamePassword should be a string.");
        }
        usrStr = args[2]->getString();
    }

    unique_ptr<Connection> cup(new Connection(hostStr, port, usrStr));
    const char *fmt = "kdb+ connection to [%s]";
    vector<char> descBuf(cup->str().size() + strlen(fmt));
    sprintf(descBuf.data(), fmt, cup->str().c_str());
    FunctionDefSP onClose(Util::createSystemProcedure("kdb+ connection onClose()", kdbConnectionOnClose, 1, 1));
    return Util::createResource((long long)cup.release(), descBuf.data(), onClose, heap->currentSession());
}

ConstantSP kdbLoadTable(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string("Usage: loadTable(handle, tablePath, symPath). ");

    if(args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "tablePath should be a string.");
    }
    string tablePath = args[1]->getString();

    string symFilePath = "";
    if(args.size() == 3) {
        if(args[2]->getType() != DT_STRING) {
            throw IllegalArgumentException(__FUNCTION__, usage + "symPath should be a string.");
        }
        symFilePath = args[2]->getString();
    }
    return safeOp(args[0], [&](Connection *conn) {return conn->getTable(tablePath, symFilePath);});
}

ConstantSP kdbLoadFile(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string("Usage: loadFile(tablePath, symPath). ");
    if(args[0]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "tablePath should be a string.");
    }
    string tablePath = args[0]->getString();

    string symFilePath = "";
    if(args.size() == 2) {
        if(args[1]->getType() != DT_STRING) {
            throw IllegalArgumentException(__FUNCTION__, usage + "symPath should be a string.");
        }
        symFilePath = args[1]->getString();
    }

    vector<string> symList(0);
    if(symFilePath.size() > 0) {
        symList = loadSym(symFilePath);
    }
    return loadSplayedTable(tablePath, symList);
}

ConstantSP kdbClose(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string("Usage: close(handle). ");
    if (args[0]->getType() == DT_RESOURCE) {
        Connection* conn = (Connection*)(args[0]->getLong());
        if(conn!= nullptr) {
            delete (Connection *) (args[0]->getLong());
            args[0]->setLong(0);
        }
    } else {
        throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
    }
    return new Void();
}