#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <string>
#include <exception>
#include <vector>
#include <iostream>
#include <map>
#include <ctime>
#include <zlib.h>

#include "Logger.h"
#include "Concurrent.h"
#include "Exceptions.h"
#include "ScalarImp.h"
#include "SpecialConstant.h"
#include "Util.h"

#include "kdb.h"
#include "endian.h"
#include "q2ddb.h"
#include "qfile.h"

using namespace std;

//////////////////////////////////////////////////////////////////////
// Constants

const char PATH_SEP = '/';

#if 1
// Parameters for gzip process
const int WINDOWS_BITS     = 15;
const int ENABLE_ZLIB_GZIP = 32;
const int GZIP_ENCODING    = 16;
#endif

// kdb+ IPC parameters
const int KDB_TIMEOUT = 1 * 1000;   // 1 sec
const int KDB_CAPABILITY = 1;       // 1 TB limit

enum KDB_IPC_error: int {
    KDB_IPC_AUTH_ERROR     =  0,
    KDB_IPC_CONN_ERROR     = -1,
    KDB_IPC_TIMEOUT_ERROR  = -2,
    KDB_IPC_SSL_INIT_ERROR = -3,
};

#if 1
// for kdb+ parse use
#define KDB_SYM_DATA_START 4080

#endif
// read kdb+ files in parallel
#define KDB_READ_SEQUENTIAL 1

//////////////////////////////////////////////////////////////////////
// DolphinDB Interops
namespace /*anonymous*/ {

    Mutex LOCK_KDB;

    int arg2Int(const ConstantSP& arg, const char* argName = nullptr,
        const string& usage = "", const char* caller = nullptr
    ) {
        assert(arg.get());
        if(arg->getType() != DT_INT) {
            const auto ref = caller  ? caller  : __FUNCTION__;
            const auto var = argName ? argName : "Arg";
            throw IllegalArgumentException(ref,
                usage + PLUGIN_NAME ": " + var + " should be an integer.");
        }
        return arg->getInt();
    }

    string arg2String(const ConstantSP& arg, const char* argName = nullptr,
        const string& usage = "", const char* caller = nullptr
    ) {
        assert(arg.get());
        if(arg->getType() != DT_STRING) {
            const auto ref = caller  ? caller  : __FUNCTION__;
            const auto var = argName ? argName : "Arg";
            throw IllegalArgumentException(ref,
                usage + PLUGIN_NAME ": " + var + " should be a string.");
        }
        return arg->getString();
    }

    Connection* arg2Connection(const ConstantSP& arg,
        const string& usage = "", const char* caller = nullptr
    ) {
        assert(arg.get());
        const auto ref = caller ? caller : __FUNCTION__;
        if(arg->getType() != DT_RESOURCE) {
            throw IllegalArgumentException(ref,
                usage + PLUGIN_NAME ": Invalid connection object.");
        }
        const string desc = arg->getString();
        if(desc.find(Connection::MARKER) == desc.npos) {
            throw IllegalArgumentException(ref,
                usage + PLUGIN_NAME ": Invalid kdb+ connection object.");
        }
        Connection* conn = reinterpret_cast<Connection*>(arg->getLong());
        return conn;
    }

    string& normalizePath(string& path) {
#       ifdef WINDOWS
        // Replace backward slash with forward slash
        path = Util::replace(path, '\\', PATH_SEP);
#       endif
        while(!path.empty() && path.back() == PATH_SEP) {
            path.pop_back();
        }
        return path;
    }

    ConstantSP safeOp(const ConstantSP &arg,
        std::function<ConstantSP(Connection *)> &&f
    ) {
        Connection* conn = arg2Connection(arg, "", __FUNCTION__);
        if(conn) {
            return f(conn);
        } else {
            throw IllegalArgumentException(__FUNCTION__,
                PLUGIN_NAME ": Connection object already closed.");
        }
    }

    void kdbConnectionOnClose(Heap *heap, vector<ConstantSP> &args) {
        assert(args.size() >= 1);

        // Use unique_ptr<> to manage conn until it is reset.
        unique_ptr<Connection> conn;
        try {
            conn.reset(arg2Connection(args[0], __FUNCTION__));
        }
        catch(IllegalArgumentException& iae) {
            throw RuntimeException(iae.what());
        }

        if(conn) {
            conn.reset();
            args[0]->setLong(0);
        }
    }

}//namespace /*anonymouse*/

//////////////////////////////////////////////////////////////////////
// Class Implementation

Connection::Connection(
    const string& hostStr, const int port, const string& usernamePassword)
  : host_{hostStr}, port_{port}
{
    const auto handle = khpunc(
        kdb::sym(hostStr), port, kdb::sym(usernamePassword),
        KDB_TIMEOUT, KDB_CAPABILITY);
    switch(handle) {
        case KDB_IPC_AUTH_ERROR:
            throw RuntimeException(PLUGIN_NAME ": Authentication error.");
        case KDB_IPC_CONN_ERROR:
            throw RuntimeException(PLUGIN_NAME ": Connection error.");
        case KDB_IPC_TIMEOUT_ERROR:
            throw RuntimeException(PLUGIN_NAME ": Connection time out.");
        default:
            if(handle < 0) {
                throw RuntimeException(PLUGIN_NAME ": q-IPC error.");
            } else {
                handle_ = handle;
            }
    }
}

Connection::~Connection() {
    kclose(handle_);
}

KPtr Connection::kExec(const string& command) const {
    KPtr res{ k(handle_, kdb::sym(command), nullptr) };
    if(!res) {
        throw RuntimeException(PLUGIN_NAME ": "
            "kdb+ network error: " + command + '(' + strerror(errno) + ").");
    } else
    if(res->t == kdb::K_ERROR) {
        const string errMsg = res->s;
        throw RuntimeException(PLUGIN_NAME ": "
            "kdb+ execution error: " + command + "('" + errMsg + ").");
    }
    return res;
}

// must use the file name to load sym in kdb+
// otherwise the enumerated data could not find its sym list
string Connection::loadSymFile(const string& symFilePath) const {
    if(symFilePath.empty()) {
        return "";
    }

    const auto fields = Util::split(symFilePath, PATH_SEP);
    if(fields.empty()) {
        throw RuntimeException(PLUGIN_NAME ": Invalid symPath.");
    }
    const string symName = fields.back();

    // load sym to kdb
    const string command = symName + R"(:get hsym`$")" + symFilePath + '"';
    kExec(command);

    LOG(PLUGIN_NAME ": Loaded enum sym `" + symName + " in kdb+");
    return symName;
}

ConstantSP Connection::loadColumn(
    const string& tableName, const string& colName
) const {
    assert(!tableName.empty());
    if(colName.empty()) {
        throw RuntimeException(PLUGIN_NAME ": invalid column name.");
    }

    const string queryCommand = tableName + "`" + colName;
    KPtr colRes{ kExec(queryCommand) };
    if(!kdb::isValidList(colRes.get())) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Failed to load column " + tableName + "." + colName + ".");
    }

    auto colVal = kdb::toDDB::fromK(colRes.get(), colName);
    auto col = dynamic_cast<Vector*>(colVal.get());
    assert(col);
    col->setNullFlag(col->hasNull());
    if(!col->size() && col->getType() == DT_ANY) {
        kdb::fakeEmptyAnyColumn(col, tableName, colName);
    }

    LOG(PLUGIN_NAME ": Loaded column " + tableName + "." + colName
        + " (" + to_string(static_cast<kdb::Type>(colRes->t))
        + "->" + to_string(col->getType()) + ") "
          "size=" + to_string(col->size()));
    return colVal;
}

TableSP Connection::getTable(
    const string& tablePath, const string& symFilePath
) const {
    LockGuard<Mutex> guard(&LOCK_KDB);

    // load symbol
    const string symName = loadSymFile(symFilePath);

    // load table
    const string loadCommand = R"(\l )" + tablePath;
    kExec(loadCommand);

    // split table path, get table name, get cols
    const auto pathVec = Util::split(tablePath, PATH_SEP);
    if(pathVec.empty()) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Invalid file path " + tablePath + ".");
    }
    const string tableName = pathVec.back();
    const string colsCommand = "cols " + tableName;
    KPtr colsRes = kExec(colsCommand);
    if(!kdb::isValidListOf(colsRes.get(), kdb::K_STRING)) {
        throw RuntimeException(PLUGIN_NAME ": Failed to get table cols.");
    }

    // load each column
    const size_t colNum = static_cast<size_t>(colsRes->n);
    vector<string> colNames(colNum);
    transform(kS(colsRes.get()), kS(colsRes.get()) + colNum, colNames.begin(),
        static_cast<S(*)(const string&)>(&kdb::sym)
    );

    vector<ConstantSP> cols(colNum);
    transform(colNames.cbegin(), colNames.cend(), cols.begin(),
        [&](const string& colName) { return loadColumn(tableName, colName); }
    );

    // drop table & sym, release memory in kdb+
    string dropCommand = "![`.;();0b;] (),`" + tableName;
    if(!symName.empty()) {
        dropCommand += "`" + symName;
    }
    KPtr dropRes{ kExec(dropCommand) };

    // create table in DolphinDB
    return Util::createTable(colNames, cols);
}

string Connection::str() const {
    return host_ + ":" + to_string(port_);
}

//////////////////////////////////////////////////////////////////////
// ??????

#if 1
const I SEC_PER_DAY   = 24 * 60 * 60;
const I KDB_MONTH_GAP = 24000;  //q) neg["i"$0001.01m] + 12i
const I KDB_DATE_GAP  = 10957;  //q) neg["i"$1970.01.01]
const J KDB_DATETIME_GAP      = KDB_DATE_GAP * SEC_PER_DAY * 1000LL;
const J KDB_NANOTIMESTAMP_GAP = KDB_DATETIME_GAP * 1000000LL;

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

long long decompress(unsigned char *src, size_t src_len, unsigned char *dest, size_t dest_len, int compressType)
{
    if(UNLIKELY(src == nullptr)) {
        return -4;
    }
    if(UNLIKELY(compressType == 0)) {
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
    }
    long long res = inflate (& strm, Z_NO_FLUSH);

    if (UNLIKELY(res < 0)){
        return -2;
    }
    inflateEnd (& strm);
    return dest_len - strm.avail_out;
}

long long decompress(FILE * fp, vector<unsigned char>& dest) {
    if(!fp) {
        return -4;
    }
    fseek(fp, 0, SEEK_END);
    long long fileLen = ftell(fp)-8;

    fseek(fp, 8, SEEK_SET);
    vector<unsigned char> srcVec;
    srcVec.resize(fileLen);
    unsigned char *src = srcVec.data();
    size_t bytesRead = fread(src, 1, fileLen, fp);
    if(UNLIKELY((long long)bytesRead != fileLen)) {
        return -4;
    }

    // read meta data of compressed file
    // read block num of compressed file
    if(UNLIKELY(fileLen < 8)) {
        return -4;
    }
    long long blockSize = rl(src, fileLen-8);

    if(UNLIKELY(fileLen < 8 + 8*blockSize + 32)) {
        return -4;
    }
    long long bufPos = fileLen-8-8*blockSize - 32;

    // read compress type&level
    bufPos+=4;
    int compressType = src[bufPos];
    bufPos+=1;
    // int compressLevel = src[bufPos]; // currently no use

    // only support type 2 zlib
    if(UNLIKELY(compressType == 1 || compressType == 3 || compressType == 4)) {
        return -3;
    }

    // read uncompress size
    bufPos+=3;
    long long originSize = rl(src, bufPos);
    bufPos+=8;
    // long long compressSize = rl(src, bufPos); // currently no use
    bufPos+=8;
    long long originBlockSize = rl(src, bufPos);

    // read every compress block size
    // TODO maybe different blocks would have different compress type
    vector<pair<size_t, size_t>> blockVec(blockSize);
    for(long long i = 0; i < blockSize; i++) {
        bufPos+=8;
        if(UNLIKELY(bufPos+8 > fileLen)) {
            throw RuntimeException("[PLUGIN::KDB]: Parsing failed, exceeding buffer bound.");
        }
        size_t len = ri(src, bufPos);
        size_t type = ri(src, bufPos+4);
        blockVec[i] = pair<size_t, size_t>{len, type};
    }

    dest.resize(originBlockSize * blockSize);
    // unsigned char *destRet = dest;
    long long offset = 0;
    for(long long i = 0; i < blockSize; i++) {
        long long decompressSize = -1;
        try {
            decompressSize = decompress(src,blockVec[i].first, dest.data()+offset, originBlockSize, blockVec[i].second);
        } catch(exception& e) {
            throw RuntimeException("[PLUGIN::KDB]: depression failed. " + string(e.what()));
        }

        if (UNLIKELY(decompressSize == -1)) {
            return -1;
        } else if (UNLIKELY(decompressSize == -2)) {
            return -2;
        } else if(UNLIKELY(decompressSize == -4)) {
            return -4;
        }
        src+=blockVec[i].first;
        offset+=decompressSize;
    }
    LOG("Expected depressed file size: ", offset, ", Actually: ", originSize);
    return originSize;
}
#endif

vector<string> loadSymList(const string& symPath) {
    kdb::BinFile symFile{symPath, symPath};
    if(UNLIKELY(!symFile)) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Open sym file " + symPath + " failed.");
    }

    kdb::Parser parser;
    symFile.readInto(parser.getBuffer());
    const auto symList = parser.getStrings(symPath);

    LOG(PLUGIN_NAME ": Loaded enum sym from " + symPath + " "
        "size=" + to_string(symList.size()));
    return symList;
}

ConstantSP loadSplayedNestedColumn(const string& colPath,
    const vector<string>& symList, const string& symName, kdb::Parser& parser
) {
#if 0
    const auto colMapPath = colPath + '#';
    kdb::BinFile colMapFile{colMapPath, colMapPath};
    if(UNLIKELY(!colMapFile)) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Open nested column " + colMapPath + " failed.");
    }

    kdb::Parser mapParser;
    colMapFile.readInto(mapParser.getBuffer());
    mapParser.initialize();
    return parser.nestedList(mapParser, symList, symName, colPath, colMapPath);
#endif
throw RuntimeException(to_string(__LINE__));
}

VectorSP loadSplayedColumn(const string& colPath,
    const vector<string>& symList, const string& symName
) {
    kdb::BinFile colFile{colPath, colPath};
    if(UNLIKELY(!colFile)) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Open column " + colPath + " failed.");
    }

    kdb::Parser parser;
    colFile.readInto(parser.getBuffer());
    VectorSP col = parser.getVector(colPath, symList, symName);
    assert(!col.isNull());

    LOG(PLUGIN_NAME ": Loaded splayed column from " + colPath + " "
        + "type=" + to_string(col->getType()) + " "
          "size=" + to_string(col->size()));
    return col;
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
             return new String("[PLUGIN::KDB]: depression failed. " + string(e.what()));
        }

        if(UNLIKELY(dep == -1)) {
            return new String("Depression failed, can't init inflate");
        } else if(UNLIKELY(dep == -2)) {
            return new String("Depression failed, can't inflate.");
        } else if(UNLIKELY(dep == -3)) {
            return new String("Unsupported kdb compress type, please use loadTable().");
        } else if(UNLIKELY(dep == -4)) {
            return new String("load " + colSrc + " col file failed.");
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
        case 0: // have risk, maybe have some unknown mechanism
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
                if(length == 0 && flag == 0) {
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
                         return new String("[PLUGIN::KDB]: depression failed. " + string(e.what()));
                    }
                    if(UNLIKELY(dep == -1)) {
                        return new String("Depression failed, can't init inflate");
                    } else if(UNLIKELY(dep == -2)) {
                        return new String("Depression failed, can't inflate.");
                    } else if(UNLIKELY(dep == -3)) {
                        return new String("Unsupported kdb compress type, please use loadTable().");
                    } else if(UNLIKELY(dep == -4)) {
                        return new String("Load " + colSrcSharp + " col file failed.");
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
                // un-match pattern, to be find
                return new String("load Sym file failed.");
            }
        }
        case kdb::K_BOOL:
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
        case kdb::K_GUID:
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
        case kdb::K_BYTE:
        case kdb::K_CHAR:
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
        case kdb::K_SHORT:
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
        case kdb::K_INT:
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
        case kdb::K_LONG:
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
        case kdb::K_FLOAT:
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
//                if(((unsigned int *)floatSrc)[i] == KDB_FLOAT_NULL) {
                    floatSrc[i] = FLT_NMIN;
//                }
            }

            vec->appendFloat((float *)floatSrc, length);
            LOG("[PLUGIN::KDB]: load col "+ colSrc + " size: " + to_string(((Vector*)vec.get())->size()) + ".");
            return vec;
        }
        case kdb::K_DOUBLE:
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
//                if(((unsigned long long *)doubleSrc)[i] == KDB_DOUBLE_NULL) {
                    doubleSrc[i] = DBL_NMIN;
//                }
            }

            vec->appendDouble((double *)doubleSrc, length);
            LOG("[PLUGIN::KDB]: load col "+ colSrc + " size: " + to_string(((Vector*)vec.get())->size()) + ".");
            return vec;
        }
        case kdb::K_TIMESTAMP:
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
        case kdb::K_MONTH:
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
        case kdb::K_DATE:
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
        case kdb::K_DATETIME:
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
        case kdb::K_TIMESPAN:
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
        case kdb::K_MINUTE:
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
        case kdb::K_SECOND:
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
        case kdb::K_TIME:
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
        case kdb::K_STRING:
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
    symList_(symList) {;}

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

//@see https://code.kx.com/q/database/object/
//@see https://code.kx.com/q/kb/splayed-tables/
TableSP loadSplayedTable(string tablePath,
    vector<string>& symList, string symName
) {
    if(!tablePath.empty() && tablePath.back() != PATH_SEP) {
        tablePath.push_back(PATH_SEP);
    }

    // Read .d file, get column names
    const string dotD = tablePath + ".d";
    vector<string> colNames = loadSymList(dotD);
cerr << tablePath << '\t' << colNames.size() << '\n';
for(const auto c : colNames) cerr << '\t' << c;
cerr << endl;

    const size_t colNum = colNames.size();
    vector<ConstantSP> cols;
    cols.reserve(colNum);

#if KDB_READ_SEQUENTIAL
    transform(colNames.cbegin(), colNames.cend(), back_inserter(cols),
        [&tablePath, &symList, &symName](const string& colName) {
            return loadSplayedColumn(tablePath + colName, symList, symName);
        }
    );
#else
    vector<ThreadSP> colThreads(colNum);
    transform(colNames.cbegin(), colNames.cend(), cols.begin(), colThreads.begin(),
        [&tablePath, &symList, &symName](const string& colName, ConstantSP& col) {
            GetColumnRunnableSP runnable{new GetColumnRunnable{
                col, colName, tablePath + colName, symList, symName
            }};
            return ThreadSP{new Thread{runnable}};
        }
    );
    for(auto thread : colThreads) {
        assert(thread.get());
        if(!thread->isStarted()) {
            thread->start();
        }
    }
    for(auto thread : colThreads) {
        assert(thread.get());
        thread->join();
    }
    for(const auto col : cols) {
        assert(col.get());
        if(col->getForm() != DF_VECTOR) {
            if(col->getType() != DT_STRING && !col->isNull()) {
                throw RuntimeException(col->getString());
            }
            else {
                throw RuntimeException(PLUGIN_NAME ": " "Column parse failed.");
            }
        }
        col->setTemporary(true);
    }
throw RuntimeException("'nyi: parallel");
#endif

    return Util::createTable(colNames, cols);

#if 0
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
            throw RuntimeException("[PLUGIN::KDB]: depression failed. " + string(e.what()));
        }
        if(dep == -1) {
            throw RuntimeException("[PLUGIN::KDB]: .d file Depression failed, can't init inflate");
        } else if(dep == -2) {
            throw RuntimeException("[PLUGIN::KDB]: .d file Depression failed, can't inflate.");
        } else if(dep == -3) {
            throw RuntimeException("[PLUGIN::KDB]: .d file Unsupported kdb compress type, please use loadTable().");
        } else if(dep == -4) {
            throw RuntimeException("[PLUGIN::KDB]: load .d file failed.");
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
#endif
}

//////////////////////////////////////////////////////////////////////////////
// DolphinDB Plugin API

ConstantSP kdbConnect(Heap *heap, vector<ConstantSP> &args){
    const string usage = "Usage: connect(host, port, usernamePassword). ";
    assert(args.size() >= 3-1);

    const string hostStr = arg2String(args[0], "host", usage, __FUNCTION__);
    const int port = arg2Int(args[1], "port", usage, __FUNCTION__);

    string usrStr = "";
    if(args.size() >= 3) {
        usrStr = arg2String(args[2], "usernamePassword", usage, __FUNCTION__);
    }

    // Use unique_ptr<> to manage cup until Util::createResource() takes over.
    /*
    //FIXME: make_unique<>() is available only after C++14...
    auto cup = make_unique<Connection>(hostStr, port, usrStr);
    /*/
    unique_ptr<Connection> cup{ new Connection(hostStr, port, usrStr) };
    //*/
    const string desc = Connection::MARKER + (" to [" + cup->str() + "]");
    FunctionDefSP onClose{
        Util::createSystemProcedure(
            "kdb+ connection onClose()", &kdbConnectionOnClose, 1, 1)
    };
    // FIXME: Still not quite safe!
    //  If Util::createResource throws, cup will be dangling forever.
    static_assert(sizeof(long long) >= sizeof(Connection*),
        "ensure enough space to store the pointer");
    return Util::createResource(reinterpret_cast<long long>(cup.release()),
        desc, onClose, heap->currentSession());
}

ConstantSP kdbLoadTable(Heap *heap, vector<ConstantSP> &args){
    const string usage = "Usage: loadTable(handle, tablePath, symPath). ";
    assert(args.size() >= 3-1);

    string tablePath = arg2String(args[1], "tablePath", usage, __FUNCTION__);
    tablePath = normalizePath(tablePath);

    string symFilePath = "";
    if(args.size() >= 3) {
        symFilePath = arg2String(args[2], "symPath", usage, __FUNCTION__);
    }
    symFilePath = normalizePath(symFilePath);

    return safeOp(args[0],
        [&](Connection *conn) { return conn->getTable(tablePath, symFilePath); }
    );
}

ConstantSP kdbLoadFile(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string("Usage: loadFile(tablePath, symPath). ");
    assert(args.size() >= 2-1);

    string tablePath = arg2String(args[0], "tablePath", usage, __FUNCTION__);
    tablePath = normalizePath(tablePath);
    if(!(Util::exists(tablePath) || Util::existsDir(tablePath))) {
        throw IllegalArgumentException(__FUNCTION__,
            usage + PLUGIN_NAME ": "
            "tablePath [" + tablePath + "] does not exist.");
    }

    string symFilePath = "";
    if(args.size() >= 2) {
        symFilePath = arg2String(args[1], "symPath", usage, __FUNCTION__);
    }
    symFilePath = normalizePath(symFilePath);
    if(!(symFilePath.empty() || Util::exists(symFilePath))) {
        const char* extra = Util::existsDir(symFilePath)
            ? "should be a file, not a directory"
            : "does not exist";
        throw IllegalArgumentException(__FUNCTION__,
            usage + PLUGIN_NAME ": "
            "symPath [" + symFilePath + "] " + extra + '.');
    }

    vector<string> symList;
    if(!symFilePath.empty()) {
        symList = loadSymList(symFilePath);
    }

    vector<string> fields;
    Util::split(symFilePath, PATH_SEP, fields);
    string symName = "";
    if(!fields.empty()) {
        symName = fields.back();
    }
cerr << tablePath << '\t' << '|' << '\t' << symName << endl;
    return loadSplayedTable(tablePath, symList, symName);
}

ConstantSP kdbClose(Heap *heap, vector<ConstantSP> &args) {
    const string usage = "Usage: close(handle). ";
    assert(args.size() >= 1);

    auto conn = arg2Connection(args[0], usage, __FUNCTION__);
    if(conn != nullptr) {
        delete conn;
        args[0]->setLong(0);
    }
    return new Void();
}