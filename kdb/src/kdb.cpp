#include <stdlib.h>
#include <cstddef>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <string>
#include <zlib.h>
#include <exception>
#include <vector>
#include <iostream>
#include <map>
#include <ctime>

#include "snappy.h"
#include "lz4.h"
#include "Logger.h"
#include "kdb.h"
#include "Concurrent.h"
#include "Exceptions.h"
#include "ScalarImp.h"
#include "Util.h"

using namespace std;

// parameters for gzip process
#define WINDOWS_BITS 15
#define ENABLE_ZLIB_GZIP 32
#define GZIP_ENCODING 16

// for kdb+ parse use
#define KDB_SYM_DATA_START 4080
#define KDB_DOUBLE_NULL 18444492273895866368ULL
#define KDB_FLOAT_NULL 4290772992
#define KDB_NANOTIMESTAMP_GAP 946684800000000000
#define KDB_DATETIME_GAP 946684800000
#define KDB_MONTH_GAP 24000
#define KDB_DATE_GAP 10957

#define FILE_BOUNDARY_CHECK(condition)      \
    if(UNLIKELY(condition)) {               \
        throw RuntimeException("Parsing failed, exceeding buffer bound");\
    }

static Mutex LOCK_KDB;

ConstantSP safeOp(const ConstantSP &arg, std::function<ConstantSP(Connection *)> &&f) {
    if (arg->getType() == DT_RESOURCE && ((Connection *)(arg->getLong()) != nullptr)) {
        string desc = arg->getString();
        if(desc.find("kdb+ connection") == desc.npos) {
            throw IllegalArgumentException(__FUNCTION__, "[PLUGIN::KDB]: Invalid kdb+ connection object.");
        }
        auto conn = (Connection *)(arg->getLong());
        return f(conn);
    } else {
        throw IllegalArgumentException(__FUNCTION__, "[PLUGIN::KDB]: Invalid connection object.");
    }
}

static void kdbConnectionOnClose(Heap *heap, vector<ConstantSP> &args) {
    Connection* conn = (Connection*)(args[0]->getLong());
    if(conn!= nullptr) {
        delete conn;
        args[0]->setLong(0);
    }
}

Connection::Connection(const string& hostStr, const int& port, const string& usernamePassword): host_(hostStr), port_(port) {
    char* host = const_cast<char*>(hostStr.c_str());
    char* usr = const_cast<char*>(usernamePassword.c_str());
    I handle = khpunc(host, port, usr, 1000, 1);

    if(handle == 0) {
        throw RuntimeException("[PLUGIN::KDB]: Authentication error.");
    } else if(handle == -1) {
        throw RuntimeException("[PLUGIN::KDB]: Connection error.");
    } else if (handle == -2) {
        throw RuntimeException("[PLUGIN::KDB]: Connection time out.");
    }
    handle_ = handle;
}

Connection::~Connection() {
    kclose(handle_);
}

TableSP Connection::getTable(const string& tablePath, const string& symFilePath) {
    LockGuard<Mutex> guard(&LOCK_KDB);
    // load symbol
    // must use the file name to load sym in kdb+
    // otherwise the serialized data could not find its sym list
    string symName = "";
    if(symFilePath.size() > 0) {
        vector<string> fields;
        Util::split(symFilePath, '/', fields);
        if(fields.size() == 0) {
            throw RuntimeException("[PLUGIN::KDB]: Invalid symFilePath");
        }
        symName = fields.back();
        // load sym to kdb
        string command = symName + ":get`:" + symFilePath;
        char * arg = const_cast<char*>(command.c_str());
        K res = k(handle_, arg,(K)0);
        if(!res) {
            throw RuntimeException("[PLUGIN::KDB]: kdb+ execution error. fail to execute " + command + ".");
        }
        if(res->t == -128) {
            string errMsg = res->s;
            throw RuntimeException("[PLUGIN::KDB]: kdb+ execution error " + errMsg + ".");
        }
    }

    // load table
    string loadCommand = "\\l " + tablePath;
    char * loadArg = const_cast<char*>(loadCommand.c_str());
    K loadRes = k(handle_, loadArg,(K)0);
    if(!loadRes) {
        throw RuntimeException("[PLUGIN::KDB]: kdb+ execution error. fail to execute " + loadCommand + ".");
    }
    if(loadRes->t == -128) {
        string errMsg = loadRes->s;
        throw RuntimeException("[PLUGIN::KDB]: load table failed: " + errMsg + ".");
    }

    // split table path, get table name, get cols
    vector<string> pathVec = Util::split(tablePath, '/');
    if(pathVec.size() == 0) {
        throw RuntimeException("[PLUGIN::KDB]: invalid file path " + tablePath + ".");
    }
    string tableName = pathVec[pathVec.size()-1];
    string colsCommand = "cols " + tableName;
    char * colsArg = const_cast<char*>(colsCommand.c_str());
    K colsRes = k(handle_, colsArg,(K)0);
    if(!colsRes) {
        throw RuntimeException("[PLUGIN::KDB]: kdb+ execution error. fail to execute " + colsCommand + ".");
    }
    if(colsRes->t == -128) {
        string errMsg = colsRes->s;
        throw RuntimeException("[PLUGIN::KDB]: get table failed: " + errMsg + ".");
    }

    int colNum = colsRes->n;
    vector<string> colNames(colNum);
    vector<ConstantSP> cols(colNum);
    if(kS(colsRes) == nullptr) {
        throw RuntimeException("[PLUGIN::KDB]: failed to get table cols");
    }
    for(int i = 0; i < colNum; i++) {
        colNames[i] = kS(colsRes)[i];
    }

    for(int i = 0; i < colNum; i++) {
        string queryCommand = "select " +  colNames[i] + " from " + tableName;
        char * queryArg = const_cast<char*>(queryCommand.c_str());
        K colRes = k(handle_, queryArg,(K)0);
        if(!colsRes) {
            throw RuntimeException("[PLUGIN::KDB]: kdb+ execution error. fail to execute " + colsCommand + ".");
        }
        if(colRes->t == -128) {
            string errMsg = colRes->s;
            throw RuntimeException("[PLUGIN::KDB]: kdb+ execution error " + errMsg + ".");
        }

        /**
         * colRes:                                          result content
         * colRes->t:                                       result type
         * colRes->k:                                       table content
         * kK(colRes->k)                                    table content list
         * kK(colRes->k)[1]:                                second table content, is colValue content
         * kK(kK(colRes->k)[1]):                            colValue list of table content
         * kK(kK(colRes->k)[1])[0]:                         first colValue
         * kK(kK(colRes->k)[1])[0]->t                       colValue type
         * kK(kK(kK(colRes->k)[1])[0]):                     nested list value list of colValue
         * kK(kK(kK(colRes->k)[1])[0])[index]:              nested list value at index position
         * kC(kK(kK(kK(colRes->k)[1])[0])[index]):          char list of nested list value
         * kK(kK(kK(colRes->k)[1])[0])[index]->n:           char list length
         * kC(kK(kK(kK(colRes->k)[1])[0])[index])[i]:       ith char value of char list
         */

        // checkout kK, kKkK, kKkKkG
        if(colRes->k == nullptr || kK(colRes->k) == nullptr || colRes->k->n < 2 || kK(colRes->k)[1] == nullptr ||
           kK(colRes->k)[1]->n < 1 || kK(kK(colRes->k)[1])[0] == nullptr || kG(kK(kK(colRes->k)[1])[0]) == nullptr) {
            throw RuntimeException("[PLUGIN::KDB]: failed to parse kdb+ query result");
        }
        int type = kK(kK(colRes->k)[1])[0]->t;
        long long length = (long long)(kK(kK(colRes->k)[1])[0]->n);
        void* ptr;
        switch(type) {
            case KDB_LIST:
            {
                vector<string> charArray;
                for(int index = 0; index < length; ++index) {
                    if(kK(kK(kK(colRes->k)[1])[0])[index] == 0 || kC(kK(kK(kK(colRes->k)[1])[0])[index]) == 0) {
                        throw RuntimeException("[PLUGIN::KDB]: failed to parse kdb+ query result");
                    }
                    int constantType = int(kK(kK(kK(colRes->k)[1])[0])[index]->t);
                    string str = "";
                    // check type of nested list
                    if(constantType == 10) {
                        for(int i = 0; i < kK(kK(kK(colRes->k)[1])[0])[index]->n; ++i) {
                            str.push_back(kC(kK(kK(kK(colRes->k)[1])[0])[index])[i]);
                        }
                    } else if (constantType == -10) {
                        str.push_back(kK(kK(kK(colRes->k)[1])[0])[index]->g);
                    } else {
                        throw RuntimeException("[PLUGIN::KDB]: loadTable() only support char nested array");
                    }
                    charArray.emplace_back(str);
                }
                cols[i] = Util::createVector(DT_STRING,0,length);
                ((Vector*)(cols[i].get()))->appendString(charArray.data(), length);
                break;
            }
            case KDB_BOOL:
                ptr = kG(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_BOOL,0,length);
                ((Vector*)(cols[i].get()))->appendBool((char *)ptr, length);
                break;
            case KDB_GUID:
                ptr = kG(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_UUID,0,length);
                for(int j = 0; j < length; j++) {
                    int pos = j*16;
                    // flip guid
                    for(int h = 0; h < 8; h++) {
                        char tmp = ((char* )ptr)[pos+15-h];
                        ((char* )ptr)[pos+15-h] = ((char* )ptr)[pos+h];
                        ((char* )ptr)[pos+h] = tmp;
                    }
                }
                ((Vector*)(cols[i].get()))->appendGuid((Guid *)ptr, length);
                break;
            case KDB_BYTE:
                // DolphinDB don't have type of byte, so convert byte to char
                ptr = kG(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_CHAR,0,length);
                ((Vector*)(cols[i].get()))->appendChar((char *)ptr, length);
                break;
            case KDB_SHORT:
                ptr = kH(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_SHORT,0,length);
                ((Vector*)(cols[i].get()))->appendShort((short *)ptr, length);
                break;
            case KDB_INT:
                ptr = kI(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_INT, 0, length);
                ((Vector*)(cols[i].get()))->appendInt((int *)ptr, length);
                break;
            case KDB_LONG:
                ptr = kJ(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_LONG,0,length);
                ((Vector*)(cols[i].get()))->appendLong((long long *)ptr, length);
                break;
            case KDB_FLOAT:
            {
                ptr = kE(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_FLOAT, 0, length);

                for(int j = 0; j < length; j++) {
                    if(((unsigned int *)ptr)[j] == KDB_FLOAT_NULL) {
                        ((float *)ptr)[j] = FLT_NMIN;
                    }
                }

                ((Vector*)(cols[i].get()))->appendFloat((float *)ptr, length);
                break;
            }
            case KDB_DOUBLE:
            {
                ptr = kF(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_DOUBLE,0,length,true);
                for(int j = 0; j < length; j++) {
                    if(((unsigned long long *)ptr)[j] == KDB_DOUBLE_NULL) {
                        ((double *)ptr)[j] = DBL_NMIN;
                    }
                }
                ((Vector*)(cols[i].get()))->appendDouble((double *)ptr, length);
                break;
            }
            case KDB_CHAR:
                ptr = kG(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_CHAR,0,length,true);
                ((Vector*)(cols[i].get()))->appendChar((char *)ptr, length);
                break;
            case KDB_STRING:
                ptr = kS(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_SYMBOL,0,length);
                ((Vector*)(cols[i].get()))->appendString((const char**)ptr, length);
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
                    ((long long *)ptr)[j] += KDB_NANOTIMESTAMP_GAP;
                }
                ((Vector*)(cols[i].get()))->appendLong((long long *)ptr, length);
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
                    ((int *)ptr)[j] += KDB_MONTH_GAP;
                }
                ((Vector*)(cols[i].get()))->appendInt((int *)ptr, length);
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
                    ((int *)ptr)[j] += KDB_DATE_GAP;
                }
                ((Vector*)(cols[i].get()))->appendInt((int *)ptr, length);
                break;
            }
            case KDB_DATETIME:
            {
                // kdb+'s datetime is double, start from 2000.01.01T00:00:00.000
                ptr = kF(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_TIMESTAMP, 0, length);
                vector<long long> longVec(length);
                for(int j = 0; j < length; j++) {
                    longVec[j] = ((double *)ptr)[j]*86400000 + KDB_DATETIME_GAP;
                }
                ((Vector*)(cols[i].get()))->appendLong((long long *)(longVec.data()), length);
            }
                break;
            case KDB_TIMESPAN:
                ptr = kJ(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_NANOTIME,0,length);
                ((Vector*)(cols[i].get()))->appendLong((long long *)ptr, length);
                break;
            case KDB_MINUTE:
                ptr = kI(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_MINUTE, 0, length);
                ((Vector*)(cols[i].get()))->appendInt((int *)ptr, length);
                break;
            case KDB_SECOND:
                ptr = kI(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_SECOND, 0, length);
                ((Vector*)(cols[i].get()))->appendInt((int *)ptr, length);
                break;
            case KDB_TIME:
                ptr = kI(kK(kK(colRes->k)[1])[0]);
                cols[i] = Util::createVector(DT_TIME, 0, length);
                ((Vector*)(cols[i].get()))->appendInt((int *)ptr, length);
                break;
            default:
                throw RuntimeException("[PLUGIN::KDB]: Can't parse column " + colNames[i] + " .");
        }
        ((Vector*)(cols[i].get()))->setNullFlag(((Vector*)(cols[i].get()))->hasNull());
        r0(colRes);
        LOG("[PLUGIN::KDB] load col" + colNames[i] + " size: " + to_string(((Vector*)(cols[i].get()))->size()) + ".");
    }
    r0(colsRes);
    //drop table, release memory in kdb+
    string dropCommand = tableName + ":0";
    char* dropArg = const_cast<char*>(dropCommand.c_str());
    K dropRes = k(handle_, dropArg,(K)0);
    if(!dropRes) {
        LOG_INFO("[PLUGIN::KDB]: kdb+ execution error. fail to execute " + dropCommand + ".");
    }
    if(dropRes->t == -128) {
        string errMsg = dropRes->s;
        LOG_INFO("[PLUGIN::KDB]: drop table failed: " + errMsg + ".");
    }

    // drop sym table
    dropCommand = symName + ":0";
    dropArg = const_cast<char*>(dropCommand.c_str());
    dropRes = k(handle_, dropArg,(K)0);
    if(!dropRes) {
        LOG_INFO("[PLUGIN::KDB]: kdb+ execution error. fail to execute " + dropCommand + ".");
    }
    if(dropRes->t == -128) {
        string errMsg = dropRes->s;
        LOG_INFO("[PLUGIN::KDB]: drop sym failed: " + errMsg + ".");
    }
    for(ConstantSP col: cols) {
        col->setTemporary(true);
    }
    return Util::createTable(colNames, cols);
}

/*
 * we can't get endian info from binary file
 * the machine that persisted kdb+ file and the machine that load file into DolphinDB
 * should have same endian
 */

// decode short
short rh(unsigned char* src, long long pos) {
    return ((short*)(src+pos))[0];
}

// decode int
int ri(unsigned char* src, long long pos) {
    return ((int*)(src+pos))[0];
}

// decode long
long long rl(unsigned char* src, long long pos) {
    return ((long long*)(src+pos))[0];
}

// decode double
double rd(unsigned char* src, long long pos) {
    long long num = rl(src, pos);
    return reinterpret_cast<double&>(num);
}

long long decompressPlainText(unsigned char *src, size_t srcLen, unsigned char *dest, size_t destLen) {
    // TODO do some verification
    memcpy(dest, src, srcLen);
    return srcLen;
}

long long decompressQIpc(unsigned char *src, size_t srcLen, unsigned char *dest, size_t destLen) {
    throw RuntimeException("unsupported compress type: q IPC");
}

long long decompressGzip(unsigned char *src, size_t srcLen, unsigned char *dest, size_t destLen) {
    if(UNLIKELY(src == nullptr)) {
        throw RuntimeException("parse col failed.");
    }
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.next_in = src;
    strm.avail_in = srcLen;
    strm.next_out = dest;
    strm.avail_out = destLen;
    if (inflateInit2 (& strm, WINDOWS_BITS | ENABLE_ZLIB_GZIP) < 0){
        throw RuntimeException("gzip decompress: init inflate failed.");
    }
    long long res = inflate (& strm, Z_NO_FLUSH);

    if (UNLIKELY(res < 0)){
        throw RuntimeException("gzip decompress: inflate failed.");
    }
    inflateEnd (& strm);
    return destLen - strm.avail_out;
}

long long decompressSnappy(unsigned char *src, size_t srcLen, unsigned char *dest, size_t destLen) {
    // string destStr;
    bool success = snappy::RawUncompress(reinterpret_cast<char *>(src), srcLen, reinterpret_cast<char *>(dest));
    if(!success) {
        throw RuntimeException("snappy decompress failed.");
    }
    // memcpy(dest, destStr.c_str(), destStr.size());
    return destLen;
}

long long decompressLz4hc(unsigned char *src, size_t srcLen, unsigned char *dest, size_t destLen) {
    return LZ4_decompress_safe(reinterpret_cast<char *>(src), reinterpret_cast<char *>(dest), srcLen, destLen);
}


long long decompress(FILE * fp, vector<unsigned char>& dest) {
    if(!fp) {
        throw RuntimeException("invalid file descriptor");
    }
    fseek(fp, 0, SEEK_END);
    long long fileLen = ftell(fp)-8;

    fseek(fp, 8, SEEK_SET);
    vector<unsigned char> srcVec;
    srcVec.resize(fileLen);
    unsigned char *src = srcVec.data();
    size_t bytesRead = fread(src, 1, fileLen, fp);
    FILE_BOUNDARY_CHECK((long long)bytesRead != fileLen)

    // read meta data of compressed file
    // read block num of compressed file
    FILE_BOUNDARY_CHECK(fileLen < 8)
    long long blockSize = rl(src, fileLen-8);

    FILE_BOUNDARY_CHECK(fileLen < 8 + 8*blockSize + 32)
    long long bufPos = fileLen-8-8*blockSize - 32;

    // read compress type&level
    bufPos+=4;
    int compressType = src[bufPos];
    bufPos+=1;
    // int compressLevel = src[bufPos]; // currently no use

    if(UNLIKELY(compressType == KDB_COMPRESS_Q_IPC)) {
        throw RuntimeException("unsupported compress type: q IPC");
    }
    // else if(UNLIKELY(compressType == KDB_COMPRESS_LZ4HC)) {
    //     throw RuntimeException("unsupported compress type: lz4hc");
    // }
    // read uncompress size
    bufPos+=3;
    long long originSize = rl(src, bufPos);
    bufPos+=8;
    // long long compressSize = rl(src, bufPos); // currently no use
    bufPos+=8;
    long long originBlockSize = rl(src, bufPos);

    // read every compress block size
    vector<pair<size_t, size_t>> blockVec(blockSize);
    for(long long i = 0; i < blockSize; i++) {
        bufPos+=8;
        FILE_BOUNDARY_CHECK(bufPos+8 > fileLen)
        size_t len = ri(src, bufPos);
        size_t type = ri(src, bufPos+4);
        blockVec[i] = pair<size_t, size_t>{len, type};
    }

    dest.resize(originBlockSize * blockSize);
    long long offset = 0;
    for(long long i = 0; i < blockSize; i++) {
        long long decompressSize = -1;
        switch(blockVec[i].second) {
            case kdbCompressType::KDB_NO_COMPRESS:
                decompressSize = decompressPlainText(src, blockVec[i].first, dest.data()+offset, originBlockSize);
                break;
            case kdbCompressType::KDB_COMPRESS_Q_IPC:
                decompressSize = decompressQIpc(src, blockVec[i].first, dest.data()+offset, originBlockSize);
                break;
            case kdbCompressType::KDB_COMPRESS_GZIP:
                decompressSize = decompressGzip(src, blockVec[i].first, dest.data()+offset, originBlockSize);
                break;
            case kdbCompressType::KDB_COMPRESS_SNAPPY:
                decompressSize = decompressSnappy(src, blockVec[i].first, dest.data()+offset, originBlockSize);
                break;
            case kdbCompressType::KDB_COMPRESS_LZ4HC:
                decompressSize = decompressLz4hc(src, blockVec[i].first, dest.data()+offset, originBlockSize);
                break;
            default:
                break;
        }

        if (UNLIKELY(decompressSize < 0)) {
            throw RuntimeException("invalid depression.");
        }
        src+=blockVec[i].first;
        offset+=decompressSize;
    }
    LOG_WARN("Expected depressed file size: ", offset, ", Actually: ", originSize);
    return originSize;
}

vector<string> loadSym(string symSrc) {
    vector<string> symVec;
    FILE * fp = fopen(symSrc.c_str(), "rb");
     if (UNLIKELY(!fp)){
        throw RuntimeException("[PLUGIN::KDB]: Open sym file failed\n");
    }
    Defer df([=](){fclose(fp);});

    fseek(fp, 0, SEEK_END);
    long long fileLen = ftell(fp)-8;
    fseek(fp, 8, SEEK_SET);
    vector<unsigned char> srcVec;
    srcVec.resize(fileLen);
    unsigned char *src = srcVec.data();

    size_t bytesRead = fread(src, 1, fileLen, fp);
    if(UNLIKELY((long long)bytesRead != fileLen)) {
        throw RuntimeException("[PLUGIN::KDB]: Load sym file failed\n");
    }

    vector<char> charVec;
    for(long long i = 0; i < fileLen; i++) {
        if(src[i] == '\0') {
            string str(charVec.begin(), charVec.end());
            symVec.push_back(str);
            charVec = vector<char>();
        } else {
            charVec.push_back(src[i]);
        }
    }
    return symVec;
}

ConstantSP loadSplayedCol(const string& colSrc, const vector<string>& symList, const string& symName) {
    FILE * fp = fopen (colSrc.c_str(), "rb");
    if (UNLIKELY(!fp)){
        return new String("Open cols " + colSrc + " failed\n");
    }
    Defer df([=](){fclose(fp);});

    // decompress if compressed
    vector<unsigned char> startSrcVec(9);
    auto startSrc = startSrcVec.data();
    size_t bytesRead = fread(startSrc, 1, 8, fp);
    if(UNLIKELY(bytesRead != 8)) {
        return new String("Read " + colSrc + " col header failed.");
    }
    startSrc[8] = '\0';
    string headStr = "";
    headStr = (char*)startSrc;

    unsigned char *src = nullptr;
    vector<unsigned char> srcVec;
    long long dep;
    long long fileLen;
    if(headStr == "kxzipped") {
        //decompress
        try {
            dep = decompress(fp, srcVec);
        } catch(exception& e) {
             return new String("[PLUGIN::KDB]: " + colSrc + " depression failed. " + string(e.what()));
        }

        src = srcVec.data();
        fileLen = dep;
    } else {
        fseek(fp, 0, SEEK_END);
        fileLen = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        srcVec.resize(fileLen);
        src = srcVec.data();
        bytesRead = fread(src, 1, fileLen, fp);
        if(UNLIKELY((long long)bytesRead != fileLen)) {
            return new String("load col " + colSrc + " failed.");
        }
    }

    long long bufPos = 2;
    if(UNLIKELY(bufPos >= fileLen)) {
        return new String("Parsing failed, exceeding buffer bound");
    }
    int type = src[bufPos];
    bufPos = 8;
    if(UNLIKELY(bufPos+8 > fileLen)) {
        return new String("Parsing failed, exceeding buffer bound");
    }
    long long giveLength = rl(src, bufPos);
    bufPos = 16;
    if(UNLIKELY(bufPos > fileLen)) {
        return new String("Parsing failed, exceeding buffer bound");
    }
    switch(type) {
        case KDB_LIST: // have risk, maybe have some unknown mechanism
        {
            if(bufPos == fileLen) {
                VectorSP vec = Util::createVector(DT_SYMBOL, 0, 0, true);
                return vec;
            }
            // change logic for sym name find
            vector<char> charVec;
            long long symLength = 0;
            while(symLength + bufPos < fileLen) {
                auto tmpChar = (src[symLength + bufPos]);
                if(tmpChar != 0) {
                    charVec.push_back(tmpChar);
                } else {
                    break;
                }
                symLength++;
            }
            string str(charVec.begin(), charVec.end());

            if(symName != "" && str == symName) {
                if(UNLIKELY(symList.size() == 0)) {
                    return new String("Sym file hasn't loaded.");
                }
                bufPos = KDB_SYM_DATA_START;

                // unknow things, maybe useful
                if(UNLIKELY(bufPos+3 >= fileLen)) {
                    return new String("Parsing failed, exceeding buffer bound");
                }
                int flag = src[bufPos+3];

                bufPos+=8;
                if(UNLIKELY(bufPos+8 > fileLen)) {
                    return new String("Parsing failed, exceeding buffer bound");
                }
                long long length = rl(src, bufPos);
                // int lengthRsv = length;
                bufPos+=8;

                // FIXME use this flag, temporary get the right result
                if(length == 0 && (flag == 0 || flag == 1)) {
                    length = (fileLen - bufPos) / 8;
                }

                VectorSP vec = Util::createVector(DT_SYMBOL, 0, length, true);
                long long * longSrc = reinterpret_cast<long long *>(src + bufPos);
                vector<string> strVec(length);
                long long symLength = symList.size();
                for(long long i = 0; i < length; i++) {
                    long long index = longSrc[i];
                    if(index == LONG_MIN || index < 0) {
                        strVec[i] = "";
                    } else {
                        if(index >= symLength) {
                            // if exceeding symList, assign empty string
                            strVec[i] = "";
                            continue;
                        }
                        strVec[i] = symList[index];
                    }
                }
                vec->appendString(strVec.data(), length);
                LOG("[PLUGIN::KDB]: load col "+ colSrc + " size: " + to_string(((Vector*)vec.get())->size()) + ".");
                return vec;
            } else if(str == "") {
                // here assumes that, if str is "", it must be a nested column
                // kdb plugin loadFile() function currently not support nested column

                // get colName#
                string colSrcSharp = colSrc + "#";
                FILE * fpSharp = fopen(colSrcSharp.c_str(), "rb");
                if (UNLIKELY(!fpSharp)){
                    return new String("Open cols " + colSrcSharp + " failed\n");
                } else {
                    LOG("[PLUGIN::KDB]: Open "+ colSrcSharp + ".");
                }

                Defer df([=](){fclose(fpSharp);});
                // decompress if compressed
                vector<unsigned char> startSrcSharpVec(9);
                auto startSrcSharp = startSrcSharpVec.data();
                size_t bytesRead = fread(startSrcSharp, 1, 8, fpSharp);
                if(UNLIKELY(bytesRead != 8)) {
                    return new String("Read " + colSrcSharp + " col header failed\n");
                }
                startSrcSharp[8] = '\0';
                string headStrSharp = "";
                headStrSharp = (char*)startSrcSharp;

                unsigned char *srcSharp = nullptr;
                vector<unsigned char> srcVecSharp;
                long long dep;
                long long fileLenSharp;
                if(headStrSharp == "kxzipped") {
                    //decompress
                    try {
                        dep = decompress(fpSharp, srcVecSharp);
                    } catch(exception& e) {
                         return new String("[PLUGIN::KDB]: " + colSrcSharp + " depression failed. " + string(e.what()));
                    }
                    srcSharp = srcVecSharp.data();
                    fileLenSharp = dep;
                } else {
                    fseek(fpSharp, 0, SEEK_END);
                    fileLenSharp = ftell(fpSharp);
                    fseek(fpSharp, 0, SEEK_SET);
                    srcVecSharp.resize(fileLenSharp);
                    srcSharp = srcVecSharp.data();
                    bytesRead = fread(srcSharp, 1, fileLenSharp, fpSharp);
                }

                vector<string> strVec;
                long long bufPosSharp = KDB_SYM_DATA_START;
                bufPosSharp += 16;
                bufPos = KDB_SYM_DATA_START;
                bufPos += 16;

                while (bufPos < fileLen) {
                    int offset = ri(src, bufPos);
                    bufPos += 4;
                    bufPos += 3;
                    if(UNLIKELY(bufPos >= fileLen)) {
                        return new String("Parsing failed, exceeding buffer bound");
                    }
                    int flag = src[bufPos];
                    string builder = "";
                    long long tmpPosSharp = offset + bufPosSharp;
                    if(flag == 1) {                                             // no list of char
                        if(tmpPosSharp < fileLenSharp) {
                            if(int(srcSharp[tmpPosSharp]) == 0x89) {            // multiple char
                                tmpPosSharp += 1;
                                long long len = int(srcSharp[tmpPosSharp]);
                                tmpPosSharp += 1;
                                for(long long i = 0; i < len && tmpPosSharp+i < fileLenSharp; ++i) {
                                    builder.push_back(srcSharp[tmpPosSharp+i]);
                                }
                            } else if(int(srcSharp[tmpPosSharp]) == 0x75) {     // single char
                                builder.push_back(srcSharp[tmpPosSharp+1]);
                            } else {
                                return new String("loadFile() only support char nested list column");
                            }
                        } else {
                            return new String("parse col " + colSrc + " failed\n");
                        }
                    } else {                                                    // list of char
                        tmpPosSharp += 2;
                        if(int(srcSharp[tmpPosSharp]) != 0x0a) {
                            return new String("loadFile() only support char nested list column");
                        } else {
                            tmpPosSharp += 2;
                            tmpPosSharp += 4;
                            if(UNLIKELY(tmpPosSharp+8 > fileLenSharp)) {
                                return new String("Parsing failed, exceeding buffer bound");
                            }
                            long long len = rl(srcSharp, tmpPosSharp);
                            tmpPosSharp += 8;
                            for(long long i = 0; i < len && tmpPosSharp+i < fileLenSharp; ++i) {
                                builder.push_back(srcSharp[tmpPosSharp + i]);
                            }
                        }
                    }
                    bufPos += 1;
                    strVec.push_back(builder);
                }

                VectorSP vec = Util::createVector(DT_STRING, 0, strVec.size(), true);
                vec->appendString(strVec.data(), strVec.size());
                LOG("[PLUGIN::KDB]: load col "+ colSrc + " size: " + to_string(((Vector*)vec.get())->size()) + ".");
                return vec;
            } else {
                // sym file to be find
                return new String("sym file: '" + str + "' is needed");
            }
        }
        case KDB_BOOL:
        {
            if(UNLIKELY(bufPos == fileLen)) {
                VectorSP vec = Util::createVector(DT_BOOL, 0, 0, true);
                return vec;
            }
            long long length = (fileLen - bufPos) / 1;
            if(giveLength != 0 && giveLength < length) {
                length = giveLength;
            }
            char * boolSrc = reinterpret_cast<char *>(src + bufPos);
            VectorSP vec = Util::createVector(DT_BOOL, 0, length, true);
            vec->appendBool((char *)boolSrc, length);
            LOG("[PLUGIN::KDB]: load col "+ colSrc + " size: " + to_string(((Vector*)vec.get())->size()) + ".");
            return vec;
        }
        case KDB_GUID:
        {
            if(UNLIKELY(bufPos == fileLen)) {
                VectorSP vec = Util::createVector(DT_UUID, 0, 0, true);
                return vec;
            }
            long long length = (fileLen - bufPos) / 16;
            if(giveLength != 0 && giveLength < length) {
                length = giveLength;
            }
            unsigned char * charSrc = reinterpret_cast<unsigned char *>(src+bufPos);
            for(long long j = 0; j < length; j++) {
                long long pos = j*16;
                for(int h = 0; h < 8; h++) {
                    unsigned char tmp = charSrc[pos+15-h];
                    charSrc[pos+15-h] = charSrc[pos+h];
                    charSrc[pos+h] = tmp;
                }
            }
            VectorSP vec = Util::createVector(DT_UUID, 0, length, true);
            vec->appendGuid((Guid *)charSrc, length);
            LOG("[PLUGIN::KDB]: load col "+ colSrc + " size: " + to_string(((Vector*)vec.get())->size()) + ".");
            return vec;
        }
        case KDB_BYTE:
        case KDB_CHAR:
        {
            if(UNLIKELY(bufPos == fileLen)) {
                VectorSP vec = Util::createVector(DT_CHAR, 0, 0, true);
                return vec;
            }
            long long length = (fileLen - bufPos) / 1;
            if(giveLength != 0 && giveLength < length) {
                length = giveLength;
            }
            char * charSrc = reinterpret_cast<char *>(src + bufPos);
            VectorSP vec = Util::createVector(DT_CHAR, 0, length, true);
            vec->appendChar((char *)charSrc, length);
            LOG("[PLUGIN::KDB]: load col "+ colSrc + " size: " + to_string(((Vector*)vec.get())->size()) + ".");
            return vec;
        }
        case KDB_SHORT:
        {
            if(UNLIKELY(bufPos == fileLen)) {
                VectorSP vec = Util::createVector(DT_SHORT, 0, 0, true);
                return vec;
            }
            long long length = (fileLen - bufPos) / 2;
            if(giveLength != 0 && giveLength < length) {
                length = giveLength;
            }
            short * shortSrc = reinterpret_cast<short *>(src + bufPos);
            VectorSP vec = Util::createVector(DT_SHORT, 0, length, true);
            vec->appendShort((short *)shortSrc, length);
            LOG("[PLUGIN::KDB]: load col "+ colSrc + " size: " + to_string(((Vector*)vec.get())->size()) + ".");
            return vec;
        }
        case KDB_INT:
        {
            if(UNLIKELY(bufPos == fileLen)) {
                VectorSP vec = Util::createVector(DT_INT, 0, 0, true);
                return vec;
            }
            long long length = (fileLen - bufPos) / 4;
            if(giveLength != 0 && giveLength < length) {
                length = giveLength;
            }
            int * intSrc = reinterpret_cast<int *>(src + bufPos);
            VectorSP vec = Util::createVector(DT_INT, 0, length, true);
            vec->appendInt((int *)intSrc, length);
            LOG("[PLUGIN::KDB]: load col "+ colSrc + " size: " + to_string(((Vector*)vec.get())->size()) + ".");
            return vec;
        }
        case KDB_LONG:
        {
            if(UNLIKELY(bufPos == fileLen)) {
                VectorSP vec = Util::createVector(DT_LONG, 0, 0, true);
                return vec;
            }
            long long length = (fileLen - bufPos) / 8;
            if(giveLength != 0 && giveLength < length) {
                length = giveLength;
            }
            long long * longSrc = reinterpret_cast<long long *>(src + bufPos);
            VectorSP vec = Util::createVector(DT_LONG, 0, length, true);
            vec->appendLong((long long *)longSrc, length);
            LOG("[PLUGIN::KDB]: load col "+ colSrc + " size: " + to_string(((Vector*)vec.get())->size()) + ".");
            return vec;
        }
        case KDB_FLOAT:
        {
            if(UNLIKELY(bufPos == fileLen)) {
                VectorSP vec = Util::createVector(DT_FLOAT, 0, 0, true);
                return vec;
            }
            long long length = (fileLen - bufPos) / 4;
            if(giveLength != 0 && giveLength < length) {
                length = giveLength;
            }
            float * floatSrc = reinterpret_cast<float *>(src + bufPos);
            VectorSP vec = Util::createVector(DT_FLOAT, 0, length, true);

            for(long long i = 0; i < length; i++) {
                if(((unsigned int *)floatSrc)[i] == KDB_FLOAT_NULL) {
                    floatSrc[i] = FLT_NMIN;
                }
            }

            vec->appendFloat((float *)floatSrc, length);
            LOG("[PLUGIN::KDB]: load col "+ colSrc + " size: " + to_string(((Vector*)vec.get())->size()) + ".");
            return vec;
        }
        case KDB_DOUBLE:
        {
            if(UNLIKELY(bufPos == fileLen)) {
                VectorSP vec = Util::createVector(DT_DOUBLE, 0, 0, true);
                return vec;
            }
            long long length = (fileLen - bufPos) / 8;
            if(giveLength != 0 && giveLength < length) {
                length = giveLength;
            }
            double * doubleSrc = reinterpret_cast<double *>(src+bufPos);
            VectorSP vec = Util::createVector(DT_DOUBLE, 0, length, true);

            for(long long i = 0; i < length; i++) {
                if(((unsigned long long *)doubleSrc)[i] == KDB_DOUBLE_NULL) {
                    doubleSrc[i] = DBL_NMIN;
                }
            }

            vec->appendDouble((double *)doubleSrc, length);
            LOG("[PLUGIN::KDB]: load col "+ colSrc + " size: " + to_string(((Vector*)vec.get())->size()) + ".");
            return vec;
        }
        case KDB_TIMESTAMP:
        {
            if(UNLIKELY(bufPos == fileLen)) {
                VectorSP vec = Util::createVector(DT_TIMESTAMP, 0, 0, true);
                return vec;
            }
            long long length = (fileLen - bufPos) / 8;
            if(giveLength != 0 && giveLength < length) {
                length = giveLength;
            }
            long long * longSrc = reinterpret_cast<long long *>(src + bufPos);
            unsigned long long nan = 1;
            nan = nan << 63;
            for(long long i = 0; i < length; i++) {
                if(((unsigned long long *)longSrc)[i] == nan) {
                    continue;
                }
                longSrc[i] += KDB_NANOTIMESTAMP_GAP;
            }
            VectorSP vec = Util::createVector(DT_NANOTIMESTAMP, 0, length, true);
            vec->appendLong((long long *)longSrc, length);
            LOG("[PLUGIN::KDB]: load col "+ colSrc + " size: " + to_string(((Vector*)vec.get())->size()) + ".");
            return vec;
        }
        case KDB_MONTH:
        {
            if(UNLIKELY(bufPos == fileLen)) {
                VectorSP vec = Util::createVector(DT_MONTH, 0, 0, true);
                return vec;
            }
            long long length = (fileLen - bufPos) / 4;
            if(giveLength != 0 && giveLength < length) {
                length = giveLength;
            }
            int * intSrc = reinterpret_cast<int *>(src + bufPos);
            unsigned int nan = 1 << 31;
            for(long long i = 0; i < length; i++) {
                if(((unsigned int *)(intSrc))[i] == nan) {
                    continue;
                }
                intSrc[i] += KDB_MONTH_GAP;
            }
            VectorSP vec = Util::createVector(DT_MONTH, 0, length, true);
            vec->appendInt((int *)intSrc, length);
            LOG("[PLUGIN::KDB]: load col "+ colSrc + " size: " + to_string(((Vector*)vec.get())->size()) + ".");
            return vec;
        }
        case KDB_DATE:
        {
            if(UNLIKELY(bufPos == fileLen)) {
                VectorSP vec = Util::createVector(DT_DATE, 0, 0, true);
                return vec;
            }
            long long length = (fileLen - bufPos) / 4;
            if(giveLength != 0 && giveLength < length) {
                length = giveLength;
            }
            int * intSrc = reinterpret_cast<int *>(src + bufPos);
            unsigned int nan = 1 << 31;
            for(long long i = 0; i < length; i++) {
                if(((unsigned int *)(intSrc))[i] == nan) {
                    continue;
                }
                intSrc[i] += KDB_DATE_GAP;
            }
            VectorSP vec = Util::createVector(DT_DATE, 0, length, true);
            vec->appendInt((int *)intSrc, length);
            LOG("[PLUGIN::KDB]: load col "+ colSrc + " size: " + to_string(((Vector*)vec.get())->size()) + ".");
            return vec;
        }
        case KDB_DATETIME:
        {
            if(UNLIKELY(bufPos == fileLen)) {
                VectorSP vec = Util::createVector(DT_TIMESTAMP, 0, 0, true);
                return vec;
            }
            long long length = (fileLen - bufPos) / 8;
            if(giveLength != 0 && giveLength < length) {
                length = giveLength;
            }
            double * doubleSrc = reinterpret_cast<double *>(src+bufPos);
            VectorSP vec = Util::createVector(DT_TIMESTAMP, 0, length, true);
            vector<long long> longSrc(length);
            for(long long i = 0; i < length; i++) {
                longSrc[i] = (long long)((doubleSrc[i]) * 86400000 + KDB_DATETIME_GAP);
            }
            vec->appendLong((long long *)(longSrc.data()), length);
            LOG("[PLUGIN::KDB]: load col "+ colSrc + " size: " + to_string(((Vector*)vec.get())->size()) + ".");
            return vec;
        }
        case KDB_TIMESPAN:
        {
            if(UNLIKELY(bufPos == fileLen)) {
                VectorSP vec = Util::createVector(DT_NANOTIME, 0, 0, true);
                return vec;
            }
            long long length = (fileLen - bufPos) / 8;
            if(giveLength != 0 && giveLength < length) {
                length = giveLength;
            }
            long long * longSrc = reinterpret_cast<long long *>(src + bufPos);
            VectorSP vec = Util::createVector(DT_NANOTIME, 0, length, true);
            vec->appendLong((long long *)longSrc, length);
            LOG("[PLUGIN::KDB]: load col "+ colSrc + " size: " + to_string(((Vector*)vec.get())->size()) + ".");
            return vec;
        }
        case KDB_MINUTE:
        {
            if(UNLIKELY(bufPos == fileLen)) {
                VectorSP vec = Util::createVector(DT_MINUTE, 0, 0, true);
                return vec;
            }
            long long length = (fileLen - bufPos) / 4;
            if(giveLength != 0 && giveLength < length) {
                length = giveLength;
            }
            int * intSrc = reinterpret_cast<int *>(src + bufPos);
            VectorSP vec = Util::createVector(DT_MINUTE, 0, length, true);
            vec->appendInt((int *)intSrc, length);
            LOG("[PLUGIN::KDB]: load col "+ colSrc + " size: " + to_string(((Vector*)vec.get())->size()) + ".");
            return vec;
        }
        case KDB_SECOND:
        {
            if(UNLIKELY(bufPos == fileLen)) {
                VectorSP vec = Util::createVector(DT_SECOND, 0, 0, true);
                return vec;
            }
            long long length = (fileLen - bufPos) / 4;
            if(giveLength != 0 && giveLength < length) {
                length = giveLength;
            }
            int * intSrc = reinterpret_cast<int *>(src + bufPos);
            VectorSP vec = Util::createVector(DT_SECOND, 0, length, true);
            vec->appendInt((int *)intSrc, length);
            LOG("[PLUGIN::KDB]: load col "+ colSrc + " size: " + to_string(((Vector*)vec.get())->size()) + ".");
            return vec;
        }
        case KDB_TIME:
        {
            if(UNLIKELY(bufPos == fileLen)) {
                VectorSP vec = Util::createVector(DT_TIME, 0, 0, true);
                return vec;
            }
            long long length = (fileLen - bufPos) / 4;
            if(giveLength != 0 && giveLength < length) {
                length = giveLength;
            }
            int * intSrc = reinterpret_cast<int *>(src + bufPos);
            VectorSP vec = Util::createVector(DT_TIME, 0, length, true);
            vec->appendInt((int *)intSrc, length);
            LOG("[PLUGIN::KDB]: load col "+ colSrc + " size: " + to_string(((Vector*)vec.get())->size()) + ".");
            return vec;
        }
        case KDB_STRING:
        {
            /*
             * Splayed table mustn't have non-enumerated symbol column.
             * Symbol column must have been enumerated before persisting.
             */
        }
        default:
            return new String("[PLUGIN::KDB]: can't parse column " + colSrc + " .");
    }
    VectorSP vec = Util::createVector(DT_VOID, 0);
    return vec;
}
class GetColRunnable: public Runnable {
public:
    GetColRunnable(vector<ConstantSP>& cols, int colIndex, const string& colPath,
                    const vector<string>& symList, const string& symName):
    colIndex_(colIndex),
    colPath_(colPath),
    symName_(symName),
    cols_(cols),
    symList_(symList) {}

void run() override {
    try {
        ConstantSP vec;
        try {
            vec = loadSplayedCol(colPath_, symList_, symName_);
        } catch(std::exception &exception) {
            vec = new String(exception.what());
        }
        vec->setNullFlag(true);
        cols_[colIndex_] = vec;
    } catch (std::exception& ex) {
        LOG_ERR(string("[PLUGIN::KDB]: parse kdb col file failed: ") + ex.what());
    } catch (...) {
        LOG_ERR("[PLUGIN::KDB]: parse kdb col file failed.");
    }
}
private:
    int colIndex_;
    string colPath_;
    string symName_;
    vector<ConstantSP>& cols_;
    const vector<string>& symList_;
};

using GetColRunnableSP = SmartPointer<GetColRunnable>;

TableSP loadSplayedTable(string tablePath, vector<string>& symList, string symName) {
    if(tablePath.size() > 0 && tablePath[tablePath.size()-1] != '/') {
        tablePath.push_back('/');
    }
    // read .d file, get column names
    string dotD = tablePath + ".d";
    FILE * fp = fopen(dotD.c_str(), "rb");
    if (!fp){
        throw RuntimeException("[PLUGIN::KDB]: Open .d file failed, this is not a splayed table\n");
    }
    Defer df([=](){fclose(fp);});

    // decompress if compressed
    vector<unsigned char> startSrcVec(9);
    auto startSrc = startSrcVec.data();
    size_t bytesRead = fread(startSrc, 1, 8, fp);
    if(bytesRead != 8) {
        throw RuntimeException("[PLUGIN::KDB]: read header failed\n");
    }
    startSrc[8] = '\0';
    string headStr = "";
    headStr = (char*)startSrc;

    unsigned char *src = nullptr;
    vector<unsigned char> srcVec;
    long long dep;
    long long fileLen;
    if(headStr == "kxzipped") {
        //decompress
        try {
            dep = decompress(fp, srcVec);
        } catch(exception& e) {
            throw RuntimeException("[PLUGIN::KDB]: .d depression failed. " + string(e.what()));
        }
        // skip head 00 completion of file
        src = srcVec.data()+8;
        fileLen = dep-8;
    } else {
        fseek(fp, 0, SEEK_END);
        fileLen = ftell(fp)-8;
        fseek(fp, 8, SEEK_SET);
        srcVec.resize(fileLen);
        src = srcVec.data();
        bytesRead = fread(src, 1, fileLen, fp);
        if((long long)bytesRead != fileLen) {
            throw RuntimeException("[PLUGIN::KDB]: load .d file failed.");
        }
    }

    // read
    vector<string> colNameVec;
    vector<char> charVec;
    for(long long i = 0; i < fileLen; i++) {
        if(src[i] == '\0') {
            string str(charVec.begin(), charVec.end());
            colNameVec.push_back(str);
            charVec.clear();
        } else {
            charVec.push_back(src[i]);
        }
    }

    int colNameSize = colNameVec.size();
    vector<ConstantSP> cols(colNameSize);

    // read every column file, decompress if compressed, get column data

    // serial version
    // for(int i = 0; i < colNameSize; i++) {
    //     cols[i] = loadSplayedCol(tablePath+colNameVec[i], symList);
    // }
    // return Util::createTable(colNameVec, cols);

    // parallel version
    vector<ThreadSP> colThreads(colNameSize);
    for(int i = 0; i < colNameSize; i++) {
        GetColRunnableSP runnable = new GetColRunnable(cols, i, tablePath+colNameVec[i], symList, symName);
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
        if(col->getForm() != DF_VECTOR ) {
            if(col->getType() == DT_STRING && !col->isNull()) {
                throw RuntimeException("[PLUGIN::KDB]: " + col->getString());
            } else {
                throw RuntimeException("[PLUGIN::KDB]: parsing col failed");
            }
        }
        col->setTemporary(true);
    }
    TableSP table = Util::createTable(colNameVec, cols);
    return table;
}

ConstantSP kdbConnect(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string("Usage: connect(host, port, usernamePassword). ");
    if(args[0]->getType() != DT_STRING){
        throw IllegalArgumentException(__FUNCTION__, usage + "[PLUGIN::KDB]: host should be a string.");
    }
    string hostStr = args[0]->getString();

    if(args[1]->getType() != DT_INT){
        throw IllegalArgumentException(__FUNCTION__, usage + "[PLUGIN::KDB]: port should be a integer.");
    }
    I port = args[1]->getInt();

    string usrStr = "";
    if(args.size() == 3) {
        if(args[2]->getType() != DT_STRING){
            throw IllegalArgumentException(__FUNCTION__, usage + "[PLUGIN::KDB]: usernamePassword should be a string.");
        }
        usrStr = args[2]->getString();
    }

    Connection* cup = new Connection(hostStr, port, usrStr);
    string desc = "kdb+ connection to [" + cup->str() + "]";
    FunctionDefSP onClose(Util::createSystemProcedure("kdb+ connection onClose()", kdbConnectionOnClose, 1, 1));
    return Util::createResource((long long)cup, desc, onClose, heap->currentSession());
}

ConstantSP kdbLoadTable(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string("Usage: loadTable(handle, tablePath, symPath). ");

    if(args[1]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "[PLUGIN::KDB]: tablePath should be a string.");
    }
    string tablePath = args[1]->getString();
#ifdef WINDOWS
	//replace forward slash with back slash
	tablePath = Util::replace(tablePath, '\\', '/');
#endif
    while(tablePath.size() > 0 && tablePath.back() == '/') {
        tablePath.pop_back();
    }
    /* if user use loadTable, file may not be at same machine with dolphindb
     * so the existence verification is useless
     */
    // if(!Util::exists(tablePath) && !Util::existsDir(tablePath)) {
    //     throw IllegalArgumentException(__FUNCTION__, usage + "[PLUGIN::KDB]: tablePath [" + tablePath + "] doesn't exist.");
    // }

    string symFilePath = "";
    if(args.size() == 3) {
        if(args[2]->getType() != DT_STRING) {
            throw IllegalArgumentException(__FUNCTION__, usage + "[PLUGIN::KDB]: symPath should be a string.");
        }
        symFilePath = args[2]->getString();
    }
#ifdef WINDOWS
    //replace forward slash with back slash
    symFilePath = Util::replace(symFilePath, '\\', '/');
#endif
    while(symFilePath.size() > 0 && symFilePath.back() == '/') {
        symFilePath.pop_back();
    }
    // if(symFilePath!= "" && !Util::exists(symFilePath)) {
    //     if(Util::existsDir(symFilePath)) {
    //         throw IllegalArgumentException(__FUNCTION__, usage + "[PLUGIN::KDB]: symPath [" +
    //                                         symFilePath + "] should be a single file, not a directory.");
    //     }
    //     throw IllegalArgumentException(__FUNCTION__, usage + "[PLUGIN::KDB]: symPath [" + symFilePath + "] doesn't exist.");
    // }
    return safeOp(args[0], [&](Connection *conn) {return conn->getTable(tablePath, symFilePath);});
}

ConstantSP kdbLoadFile(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string("Usage: loadFile(tablePath, symPath). ");
    if(args[0]->getType() != DT_STRING) {
        throw IllegalArgumentException(__FUNCTION__, usage + "[PLUGIN::KDB]: tablePath should be a string.");
    }
    string tablePath = args[0]->getString();
#ifdef WINDOWS
	//replace forward slash with back slash
	tablePath = Util::replace(tablePath, '\\', '/');
#endif
    while(tablePath.size() > 0 && tablePath.back() == '/') {
        tablePath.pop_back();
    }
    if(!Util::exists(tablePath) && !Util::existsDir(tablePath)) {
        throw IllegalArgumentException(__FUNCTION__, usage + "[PLUGIN::KDB]: tablePath [" + tablePath + "] doesn't exist.");
    }
    string symFilePath = "";
    if(args.size() == 2) {
        if(args[1]->getType() != DT_STRING) {
            throw IllegalArgumentException(__FUNCTION__, usage + "[PLUGIN::KDB]: symPath should be a string.");
        }
        symFilePath = args[1]->getString();
    }
#ifdef WINDOWS
	//replace forward slash with back slash
	symFilePath = Util::replace(symFilePath, '\\', '/');
#endif
    while(symFilePath.size() > 0 && symFilePath.back() == '/') {
        symFilePath.pop_back();
    }
    if(symFilePath!= "" && !Util::exists(symFilePath)) {
        if(Util::existsDir(symFilePath)) {
            throw IllegalArgumentException(__FUNCTION__, usage + "[PLUGIN::KDB]: symPath [" +
                                            symFilePath + "] should be a single file, not a directory.");
        }
        throw IllegalArgumentException(__FUNCTION__, usage + "[PLUGIN::KDB]: symPath [" + symFilePath + "] doesn't exist.");
    }

    vector<string> symList;
    if(symFilePath.size() > 0) {
        symList = loadSym(symFilePath);
    }
    vector<string> fields;
    Util::split(symFilePath, '/', fields);
    string symName = "";
    if(!(fields.size() == 0)) {
        symName = fields.back();
    }
    return loadSplayedTable(tablePath, symList, symName);
}

ConstantSP kdbClose(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string("Usage: close(handle). ");
    if (args[0]->getType() == DT_RESOURCE) {
        string desc = args[0]->getString();
        if(desc.find("kdb+ connection") == desc.npos) {
            throw IllegalArgumentException(__FUNCTION__, "[PLUGIN::KDB]: Invalid kdb+ connection object.");
        }
        Connection* conn = (Connection*)(args[0]->getLong());
        if(conn!= nullptr) {
            delete conn;
            args[0]->setLong(0);
        } else {
            throw IllegalArgumentException(__FUNCTION__, "[PLUGIN::KDB]: Invalid connection object.");
        }
    } else {
        throw IllegalArgumentException(__FUNCTION__, "[PLUGIN::KDB]: Invalid connection object.");
    }
    return new Void();
}