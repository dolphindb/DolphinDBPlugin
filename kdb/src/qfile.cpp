#include "qfile.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <limits>
#include "zlib.h"
#include "ddbplugin/PluginLogger.h"

#include "Logger.h"

#include "kdb.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////

// zlib inflation parameters
constexpr int    ZLib_FORMAT_DETECT = 32;
constexpr size_t ZLib_CHUNK_SIZE    = 1 << 14;

//////////////////////////////////////////////////////////////////////////////

kdb::BinFile::BinFile(const string& path, const string& filename)
  : filename_{filename}, fp_{nullptr} {
    fp_ = fopen(path.c_str(), "rb");
}

kdb::BinFile::~BinFile() {
    if(fp_) {
        fclose(fp_);
    }
}

kdb::BinFile::operator bool() const {
    return fp_ && !ferror(fp_);
}

size_t kdb::BinFile::readInto(vector<byte>& buffer) {
    if(!fp_) {
        throw RuntimeException(PLUGIN_NAME ": "
            + filename_ + " access error.");
    }

    fseek(fp_, 0, SEEK_SET);
    vector<char> header(MAGIC_BYTES, '\0');
    const auto read = fread(header.data(), 1, header.size(), fp_);
    if(read < MAGIC_BYTES) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Read " + filename_ + " header failed.");
    }

    const string magic{header.cbegin(), header.cend()};
    if(magic == "kxzipped") {
        return inflateBody(buffer);
    } else {
        return readAll(buffer);
    }
}

size_t kdb::BinFile::getFileLen() const {
    assert(fp_);
    const auto p = ftell(fp_);
    Defer restore{[this, p](){ fseek(fp_, p, SEEK_SET); }};

    fseek(fp_, 0, SEEK_END);
    const auto len = ftell(fp_);
    if(len < 0) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Read " + filename_ + " failed.");
    }
    return static_cast<size_t>(len);
}

size_t kdb::BinFile::getBodyLen() const {
    const size_t fileLen = getFileLen();
    if(fileLen < MAGIC_BYTES) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Empty or truncated " + filename_ + ".");
    }
    return fileLen - MAGIC_BYTES;
}

size_t kdb::BinFile::readAll(vector<byte>& buffer, ptrdiff_t offset) {
    const size_t len = getFileLen();
    assert(fp_);
    if(static_cast<ptrdiff_t>(len) < offset) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Load " + filename_ + " too little data.");
    }

    const auto initLen = buffer.size();
    buffer.resize(initLen + len);
    fseek(fp_, offset, SEEK_SET);
    const auto read = fread(buffer.data() + initLen, 1, len, fp_);
    if(offset + read < len) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Load " + filename_ + " data incomplete.");
    }
    return len;
}

#define FILE_BOUNDARY_CHECK(condition)      \
    if(UNLIKELY(condition)) {               \
        throw RuntimeException(PLUGIN_NAME "Parsing failed, exceeding buffer bound");\
    }

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
    return ((double*)(src+pos))[0];
}

enum kdbCompressType {
    KDB_NO_COMPRESS = 0,
    KDB_COMPRESS_Q_IPC = 1,
    KDB_COMPRESS_GZIP = 2,
    KDB_COMPRESS_SNAPPY = 3,
    KDB_COMPRESS_LZ4HC = 4
};


#include "snappy.h"
#include "lz4.h"
#include <zlib.h>

long long decompressPlainText(unsigned char *src, size_t srcLen, unsigned char *dest, size_t destLen) {
    memcpy(dest, src, srcLen);
    return srcLen;
}

long long decompressQIpc(unsigned char *src, size_t srcLen, unsigned char *dest, size_t destLen) {
    throw RuntimeException(PLUGIN_NAME "unsupported compress type: q IPC");
}

// parameters for gzip process
#define WINDOWS_BITS 15
#define ENABLE_ZLIB_GZIP 32
#define GZIP_ENCODING 16

long long decompressGzip(unsigned char *src, size_t srcLen, unsigned char *dest, size_t destLen) {
    if(UNLIKELY(src == nullptr)) {
        throw RuntimeException(PLUGIN_NAME "parse col failed.");
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
        throw RuntimeException(PLUGIN_NAME "gzip decompress: init inflate failed.");
    }
    long long res = inflate (& strm, Z_NO_FLUSH);

    if (UNLIKELY(res < 0)){
        throw RuntimeException(PLUGIN_NAME "gzip decompress: inflate failed.");
    }
    inflateEnd (& strm);
    return destLen - strm.avail_out;
}

long long decompressSnappy(unsigned char *src, size_t srcLen, unsigned char *dest, size_t destLen) {
    // string destStr;
    size_t length;
    bool getLengthSuccess = snappy::GetUncompressedLength(reinterpret_cast<char *>(src), srcLen, &length);
    if(!getLengthSuccess) {
        throw RuntimeException(PLUGIN_NAME "snappy get uncompressed length failed.");
    }

    bool success = snappy::RawUncompress(reinterpret_cast<char *>(src), srcLen, reinterpret_cast<char *>(dest));
    if(!success) {
        throw RuntimeException(PLUGIN_NAME "snappy decompress failed.");
    }
    return length;
}

long long decompressLz4hc(unsigned char *src, size_t srcLen, unsigned char *dest, size_t destLen) {
    auto ret = LZ4_decompress_safe(reinterpret_cast<char *>(src), reinterpret_cast<char *>(dest), srcLen, destLen);
    if (ret < 0) {
        throw RuntimeException(PLUGIN_NAME "lz4hc decompress failed.");
    }
    return ret;
}

std::size_t kdb::BinFile::inflateBody(std::vector<byte>& buffer) {
    assert(fp_);
    const auto initLen = buffer.size();

    fseek(fp_, 0, SEEK_END);
    int64_t fileLen = ftell(fp_)-int64_t(MAGIC_BYTES);

    fseek(fp_, MAGIC_BYTES, SEEK_SET);
    vector<unsigned char> srcVec;
    srcVec.resize(fileLen);
    unsigned char *src = srcVec.data();
    size_t bytesRead = fread(src, 1, fileLen, fp_);
    FILE_BOUNDARY_CHECK((long long)bytesRead != fileLen)

    // read meta data of compressed file
    // read block num of compressed file
    FILE_BOUNDARY_CHECK(fileLen < int64_t(LONG_BYTES))
    int64_t blockSize = rl(src, fileLen-int64_t(LONG_BYTES));
    if (blockSize < 0 || fileLen < blockSize) {
        throw RuntimeException(PLUGIN_NAME "invalid kxzipped blockSize " + std::to_string(blockSize));
    }

    FILE_BOUNDARY_CHECK(fileLen < int64_t(LONG_BYTES * (1+4+blockSize)))
    int64_t bufPos = fileLen-int64_t(LONG_BYTES) * (1+4+blockSize);

    // read compress type&level
    bufPos+=4;
    // int compressType = src[bufPos];
    bufPos+=1;
    // int compressLevel = src[bufPos]; // currently no use
    // read uncompress size
    bufPos+=3;
    int64_t originSize = rl(src, bufPos);
    bufPos+=LONG_BYTES;
    // long long compressSize = rl(src, bufPos); // currently no use
    bufPos+=LONG_BYTES;
    int64_t originBlockSize = rl(src, bufPos);

    // read every compress block size
    vector<pair<size_t, size_t>> blockVec(blockSize);
    for(int64_t i = 0; i < blockSize; i++) {
        bufPos+=LONG_BYTES;
        FILE_BOUNDARY_CHECK(bufPos+ int64_t(LONG_BYTES) > fileLen)
        size_t len = ri(src, bufPos);
        size_t type = ri(src, bufPos+4);
        blockVec[i] = pair<size_t, size_t>{len, type};
    }

    buffer.resize(originBlockSize * blockSize);
    size_t offset = 0;
    for(int64_t i = 0; i < blockSize; i++) {
        int64_t decompressSize = -1;
        FILE_BOUNDARY_CHECK(srcVec.size() - (src- srcVec.data()) < blockVec[i].first);
        switch(blockVec[i].second) {
            case kdbCompressType::KDB_COMPRESS_Q_IPC:
                decompressSize = decompressQIpc(src, blockVec[i].first, buffer.data()+offset, originBlockSize);
                break;
            case kdbCompressType::KDB_COMPRESS_GZIP:
                decompressSize = decompressGzip(src, blockVec[i].first, buffer.data()+offset, originBlockSize);
                break;
            case kdbCompressType::KDB_COMPRESS_SNAPPY:
                decompressSize = decompressSnappy(src, blockVec[i].first, buffer.data()+offset, originBlockSize);
                break;
            case kdbCompressType::KDB_COMPRESS_LZ4HC:
                decompressSize = decompressLz4hc(src, blockVec[i].first, buffer.data()+offset, originBlockSize);
                break;
            case kdbCompressType::KDB_NO_COMPRESS:
            default:
                decompressSize = decompressPlainText(src, blockVec[i].first, buffer.data()+offset, originBlockSize);
        }

        if (UNLIKELY(decompressSize < 0)) {
            throw RuntimeException(PLUGIN_NAME "invalid decompression.");
        }
        src+=blockVec[i].first;
        offset+=decompressSize;
    }
    buffer.resize(offset);

    PLUGIN_LOG_WARN("Expected decompressed file size: ", offset, ", Actually: ", originSize);

    assert(buffer.size() >= initLen);
    return buffer.size() - initLen;
}

namespace kdb {
class BatchColumnReader;
using BatchColumnReaderSP = SmartPointer<BatchColumnReader>;
class BatchColumnReader {
  public:
    BatchColumnReader(const string &colPath, const string &symName, const vector<string> &symList, long long batchSize)
        : batchSize_(batchSize), colPath_(colPath), symName_(symName), symList_(symList) {
        FILE *fp = fopen(colPath_.c_str(), "rb");
        if (!(fp && !ferror(fp))) {
            throw RuntimeException(PLUGIN_NAME "Open column " + colPath + " failed.");
        }
        fp_ = fp;
    }
    BatchColumnReader(const BatchColumnReader &reader) = delete;
    BatchColumnReader(BatchColumnReader &&reader) = delete;

    long long decompress(kdbCompressType type, unsigned char *src, size_t blockSize, unsigned char *dest, size_t originBlockSize) {
        switch (type) {
            case kdbCompressType::KDB_NO_COMPRESS:
                return decompressPlainText(src, blockSize, dest, originBlockSize);
            case kdbCompressType::KDB_COMPRESS_Q_IPC:
                return decompressQIpc(src, blockSize, dest, originBlockSize);
            case kdbCompressType::KDB_COMPRESS_GZIP:
                return decompressGzip(src, blockSize, dest, originBlockSize);
                break;
            case kdbCompressType::KDB_COMPRESS_SNAPPY:
                return decompressSnappy(src, blockSize, dest, originBlockSize);
                break;
            case kdbCompressType::KDB_COMPRESS_LZ4HC:
                return decompressLz4hc(src, blockSize, dest, originBlockSize);
                break;
        }
        return -1;
    }
    void init() {
        fseek(fp_, 0, SEEK_SET);
        vector<char> header(MAGIC_BYTES, '\0');
        const auto read = fread(header.data(), 1, header.size(), fp_);
        if (read < MAGIC_BYTES) {
            throw RuntimeException(PLUGIN_NAME "Read " + colPath_ + " header failed.");
        }

        const string magic{header.cbegin(), header.cend()};
        if (magic == "kxzipped") {
            compressed_ = true;

            fseek(fp_, 0, SEEK_END);
            fileTotalLength_ = ftell(fp_);

            // read meta, bug not read all data into!
            vector<byte> metaBuffer(LONG_BYTES);
            fseek(fp_, fileTotalLength_ - int64_t(LONG_BYTES), SEEK_SET);
            size_t bytes = fread(metaBuffer.data(), 1, LONG_BYTES, fp_);
            if (bytes != LONG_BYTES) {
                throw RuntimeException(PLUGIN_NAME "read blockSize failed.");
            }
            int64_t blockSize = rl(metaBuffer.data(), 0);
            if (blockSize < 0 || fileTotalLength_ < blockSize || fileTotalLength_ - LONG_BYTES * (1 + 4 + blockSize) < MAGIC_BYTES) {
                throw RuntimeException(PLUGIN_NAME "invalid blockSize " + std::to_string(blockSize));
            }
            fseek(fp_, fileTotalLength_ - int64_t(LONG_BYTES) * (1 + 4 + blockSize), SEEK_SET);
            metaBuffer.resize(LONG_BYTES * (4 + blockSize));
            bytes = fread(metaBuffer.data(), 1, LONG_BYTES * (4 + blockSize), fp_);
            if (bytes != LONG_BYTES * (4 + blockSize)) {
                throw RuntimeException(PLUGIN_NAME "read post file meta info failed.");
            }

            int64_t bufPos = INT_BYTES;
            // int compressType = metaBuffer.data()[bufPos];
            bufPos += INT_BYTES;
            /*long long originSize = */ rl(metaBuffer.data(), bufPos);  // NOTE maybe use originSize to compare
            bufPos += LONG_BYTES;
            // long long compressSize = rl(src, bufPos); // currently no use
            bufPos += LONG_BYTES;
            originBlockSize_ = rl(metaBuffer.data(), bufPos);

            blockMetaVec_.resize(blockSize);
            if (blockSize < 1) {
                throw RuntimeException(PLUGIN_NAME "invalid kxzipped block size " + std::to_string(blockSize));
            }
            for (int64_t i = 0; i < blockSize; i++) {
                bufPos += LONG_BYTES;
                FILE_BOUNDARY_CHECK(bufPos + int64_t(LONG_BYTES) > fileTotalLength_)
                size_t len = ri(metaBuffer.data(), bufPos);
                size_t type = ri(metaBuffer.data(), bufPos + int64_t(INT_BYTES));
                blockMetaVec_[i] = pair<size_t, size_t>{len, type};
            }

            // HACK read very first block
            fseek(fp_, MAGIC_BYTES, SEEK_SET);
            vector<byte> rawBuf(blockMetaVec_[0].first);
            std::ignore = fread(rawBuf.data(), 1, blockMetaVec_[0].first, fp_);

            vector<byte> &parserBuf = parser_.getBuffer();

            size_t block0Size = blockMetaVec_[0].first;
            size_t block0Type = blockMetaVec_[0].second;

            parserBuf.resize(originBlockSize_);
            FILE_BOUNDARY_CHECK(rawBuf.size() < block0Size);
            int64_t realSize = decompress(kdbCompressType(block0Type), rawBuf.data(), block0Size, parserBuf.data(), originBlockSize_);
            parserBuf.resize(realSize);
            blockOffset_++;

            if (UNLIKELY(realSize < 0)) {
                throw RuntimeException(PLUGIN_NAME "invalid decompression.");
            }
            type_ = parser_.getStruct(colPath_, symList_, symName_, count_);
        } else {
            compressed_ = false;
            // read a particular number of data to get col meta
            fseek(fp_, 0, SEEK_END);
            fileTotalLength_ = ftell(fp_);
            int64_t readSize = BASIC_READ_BYTES;
            if (fileTotalLength_ < int64_t(BASIC_READ_BYTES)) {
                readSize = fileTotalLength_;
            }
            fseek(fp_, 0, SEEK_SET);
            vector<byte> &parserBuffer = parser_.getBuffer();
            parserBuffer.resize(readSize);
            std::ignore = fread(parserBuffer.data(), 1, readSize, fp_);
            offset_ = readSize;
            // NOTE one block is enough to getStruct for all kinds of data, not have to use another getBatch
            type_ = parser_.getStruct(colPath_, symList_, symName_, count_);

        }
    }
    ~BatchColumnReader() {
        if (fp_ != nullptr) {
            fclose(fp_);
            fp_ = nullptr;
        }
    }

    // return 0 if getEnd;
    long long getBatch(const VectorSP& buffer) {
        if (meetEnd_) {
            // return 0;
        }
        buffer->clear();
        buffer->reserve(batchSize_);
        size_t batch = batchSize_;
        if (count_ != 0 && count_ - countOffset_ < batchSize_) {
            batch = count_ - countOffset_;
        }
        // if not enough, read more
        while (!parser_.isValidBuffer(batch)) {
            if (meetEnd_) {
                break;
            }
            readInBuffer(parser_.getBuffer(), meetEnd_);
        }
        parser_.getBatch(batch, buffer, meetEnd_, symList_, symName_);
        countOffset_ += buffer->size();
        return buffer->size();
    }

    void readInBuffer(vector<byte> &parserBuf, bool &meetEnd) {
        if (compressed_) {
            if (blockOffset_ >= blockMetaVec_.size()) {
                meetEnd = true;
                return;
            }
            vector<byte> rawBuf(blockMetaVec_[blockOffset_].first);
            std::ignore = fread(rawBuf.data(), 1, blockMetaVec_[blockOffset_].first, fp_);

            long long originSize = parserBuf.size();
            parserBuf.resize(originSize + originBlockSize_);

            size_t blockSize = blockMetaVec_[blockOffset_].first;
            size_t block0Type = blockMetaVec_[blockOffset_].second;

            FILE_BOUNDARY_CHECK(rawBuf.size() < blockSize);
            long long realSize =
                decompress(kdbCompressType(block0Type), rawBuf.data(), blockSize, parserBuf.data() + originSize, originBlockSize_);
            parserBuf.resize(originSize + realSize);
            blockOffset_++;
            if (blockOffset_ >= blockMetaVec_.size()) {
                meetEnd = true;
            }
        } else {
            size_t readSize = BASIC_READ_BYTES;
            if (offset_ >= fileTotalLength_) {
                meetEnd = true;
                return;
            }
            if (fileTotalLength_ - offset_ < int64_t(readSize)) {
                readSize = fileTotalLength_ - offset_;
            }
            long long originSize = parserBuf.size();
            parserBuf.resize(originSize + readSize);
            std::ignore = fread(parserBuf.data() + originSize, 1, readSize, fp_);
            offset_ += readSize;
            if (offset_ >= fileTotalLength_) {
                meetEnd = true;
                return;
            }
        }
    }

    DATA_TYPE getType() {
        if (type_ == 0) {
            return DT_ANY;
        }
        return type_;
    }

  private:
    constexpr static std::size_t MAGIC_BYTES = 8;
    constexpr static std::size_t LONG_BYTES = 8;
    constexpr static std::size_t INT_BYTES = 4;
    static constexpr std::size_t BASIC_READ_BYTES = 1 << 17;  // 128k

  private:
    FILE *fp_ = nullptr;
    bool compressed_ = false;
    bool meetEnd_ = false;
    size_t batchSize_;
    size_t originBlockSize_{};
    size_t count_ = 0;
    size_t countOffset_ = 0;
    long long offset_ = 0;       // for no compress
    size_t blockOffset_ = 0;  // for compress
    vector<pair<size_t, size_t>> blockMetaVec_;
    int64_t fileTotalLength_=0;
    vector<byte> buffers_;
    Parser parser_;
    string colPath_;
    string symName_;
    const vector<string> &symList_;
    DATA_TYPE type_ = DATA_TYPE(0);
};
}  // namespace kdb

ConstantSP kdb::extractSchema(const string &tablePath, const vector<string> &symList, const vector<string> &colNames,
                              const string &symPath, const string &symName) {
    vector<DATA_TYPE> types;
    for (const string &name : colNames) {
        BatchColumnReader reader(tablePath + name, symName, symList, 1);
        reader.init();
        types.push_back(reader.getType());
    }

    int colNums = types.size();
    VectorSP names = Util::createVector(DT_STRING, 0, colNums);
    VectorSP typeString = Util::createVector(DT_STRING, 0, colNums);
    VectorSP typeInt = Util::createVector(DT_INT, 0, colNums);

    for (int i = 0; i < colNums; ++i) {
        string name = colNames[i];
        DATA_TYPE colType = types[i];
        string typeStr = Util::getDataTypeString(colType);
        int colTypeInt = colType;
        names->appendString(&name, 1);
        typeString->appendString(&typeStr, 1);
        typeInt->appendInt(&colTypeInt, 1);
    }
    vector<string> retColNames{"name", "typeString", "typeInt"};
    vector<ConstantSP> cols{names, typeString, typeInt};
    return Util::createTable(retColNames, cols);
}

ConstantSP kdb::loadFileEx(DatabaseUpdater &dbUpdater, const string &tablePath, const vector<string> &symList,
                           const string &symName, const vector<string> &colNames, long long batchSize) {
    vector<BatchColumnReaderSP> batchReaders;
    vector<ConstantSP> cols;
    for (const string& name : colNames) {
        batchReaders.emplace_back(new BatchColumnReader(tablePath + name, symName, symList, batchSize));
        BatchColumnReaderSP reader = batchReaders.back();
        reader->init();
        cols.emplace_back(Util::createVector(reader->getType(), 0, batchSize));
    }
    while (true) {
        long long iterRet = -1;
        for (auto i = 0U; i < batchReaders.size(); ++i) {
            long long ret = batchReaders[i]->getBatch(cols[i]);
            cols[i]->setNullFlag(cols[i]->hasNull());
            if (iterRet == -1) {
                iterRet = ret;
            } else {
                if (iterRet != ret) {
                    throw RuntimeException(PLUGIN_NAME "expect buffer length " + std::to_string(iterRet) + ", actual " +
                                           std::to_string(ret));
                }
            }
        }
        if (iterRet == 0) {
            ConstantSP tmpTable = Util::createTable(colNames, cols);
            dbUpdater.append(tmpTable);
            return dbUpdater.getTableHandle();
        }
        ConstantSP tmpTable = Util::createTable(colNames, cols);
        tmpTable->setTemporary(true);
        dbUpdater.append(tmpTable);
    }
    return dbUpdater.getTableHandle();
}

//////////////////////////////////////////////////////////////////////////////