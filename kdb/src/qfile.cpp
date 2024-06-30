#include "qfile.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <limits>
#include "zlib.h"

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
        return inflateBody2(buffer);
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
        throw RuntimeException("Parsing failed, exceeding buffer bound");\
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
    long long num = rl(src, pos);
    return reinterpret_cast<double&>(num);
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
    // TODO do some verification
    memcpy(dest, src, srcLen);
    return srcLen;
}

long long decompressQIpc(unsigned char *src, size_t srcLen, unsigned char *dest, size_t destLen) {
    throw RuntimeException("unsupported compress type: q IPC");
}

// parameters for gzip process
#define WINDOWS_BITS 15         // TODO weird, should change
#define ENABLE_ZLIB_GZIP 32
#define GZIP_ENCODING 16

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
    size_t length;
    bool getLengthSuccess = snappy::GetUncompressedLength(reinterpret_cast<char *>(src), srcLen, &length);
    if(!getLengthSuccess) {
        throw RuntimeException("snappy get uncompressed length failed.");
    }

    bool success = snappy::RawUncompress(reinterpret_cast<char *>(src), srcLen, reinterpret_cast<char *>(dest));
    if(!success) {
        throw RuntimeException("snappy decompress failed.");
    }
    return length;
}

long long decompressLz4hc(unsigned char *src, size_t srcLen, unsigned char *dest, size_t destLen) {
    return LZ4_decompress_safe(reinterpret_cast<char *>(src), reinterpret_cast<char *>(dest), srcLen, destLen);
}

std::size_t kdb::BinFile::inflateBody2(std::vector<byte>& buffer) {
    assert(fp_);
    const auto initLen = buffer.size();


    fseek(fp_, 0, SEEK_END);
    long long fileLen = ftell(fp_)-8;

    fseek(fp_, 8, SEEK_SET);
    vector<unsigned char> srcVec;
    srcVec.resize(fileLen);
    unsigned char *src = srcVec.data();
    size_t bytesRead = fread(src, 1, fileLen, fp_);
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

    buffer.resize(originBlockSize * blockSize);
    long long offset = 0;
    for(long long i = 0; i < blockSize; i++) {
        long long decompressSize = -1;
        switch(blockVec[i].second) {
            case kdbCompressType::KDB_NO_COMPRESS:
                decompressSize = decompressPlainText(src, blockVec[i].first, buffer.data()+offset, originBlockSize);
                break;
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
            default:
                break;
        }

        if (UNLIKELY(decompressSize < 0)) {
            throw RuntimeException("invalid depression.");
        }
        src+=blockVec[i].first;
        offset+=decompressSize;
    }
    buffer.resize(offset);

    LOG_WARN("Expected depressed file size: ", offset, ", Actually: ", originSize);

    assert(buffer.size() >= initLen);
    return buffer.size() - initLen;
}

size_t kdb::BinFile::inflateBody(vector<byte>& buffer) {
    assert(fp_);
    const auto initLen = buffer.size();

    try {
        ZLibStream inflater{fp_, MAGIC_BYTES};
        ZLibStream::Header header;
        if(inflater.isDeflated(&header)) {
            inflater.inflate(buffer);
        } else {
            ostringstream msg;
            msg << "(0x" << hex << uppercase << setfill('0')
                << setw(2) << unsigned{header.CMF}
                << setw(2) << unsigned{header.FLG}
                << ")";
            LOG(PLUGIN_NAME ": "
                + filename_ + " is not really deflated " + msg.str() + ".");
            readAll(buffer, MAGIC_BYTES);
        }
    }
    catch(const string& err) {
        throw RuntimeException(PLUGIN_NAME ": [zlib] " + err);
    }

    assert(buffer.size() >= initLen);
    return buffer.size() - initLen;
}

//////////////////////////////////////////////////////////////////////////////

kdb::ZLibStream::ZLibStream(FILE* fp, ptrdiff_t offset)
  : fp_{fp}, offset_{offset}
{}

bool kdb::ZLibStream::isDeflated(Header* ph) const {
    assert(fp_);
    const auto p = ftell(fp_);
    Defer restore{[this, p](){ fseek(fp_, p, SEEK_SET); }};

    Header header;
    fseek(fp_, offset_, SEEK_SET);
    const auto read = fread(&header, sizeof(header), 1, fp_);
    if(read < 1) {
        throw string{"error reading stream header"};
    }

    if(ph) {
        *ph = header;
    }
    return header.compressMethod == CM_DEFLATE;
}

//@see https://www.zlib.net/zlib_how.html
size_t kdb::ZLibStream::inflate(vector<byte>& buffer) {
    assert(fp_);

    const auto prev = buffer.size();
    const auto status = inflateChunks(buffer);
    switch(status) {
        case Z_ERRNO:
            throw string{"error reading from file"};
        case Z_STREAM_ERROR:
            throw string{"invalid compression level"};
        case Z_DATA_ERROR:
            throw string{"invalid or incomplete deflate data"};
        case Z_MEM_ERROR:
            throw string{"out of memory"};
        case Z_VERSION_ERROR:
            throw string{"zlib version mismatch"};
        default:
            if(status < Z_OK) {
                throw "unknown error (" + to_string(status) + ")";
            }
    }

    assert(buffer.size() >= prev);
    return buffer.size() - prev;
}

#pragma pack(push, 1)
struct kdb::ZLibStream::Trailer {
    //{
    byte   tag[5];
    byte   pad_xxx[3];
    size_t length;
    //}

    static const byte TAG[5];

    bool isValid() const noexcept {
        return memcmp(tag, TAG, sizeof(TAG)) == 0;
    }
};
static_assert(
    sizeof(kdb::ZLibStream::Trailer) == 8+8,
    "kxzipped file trailer"
);

const kdb::byte kdb::ZLibStream::Trailer::TAG[] = {
    0x03, 0x00, 0x00, 0x00, 0x02,
};
#pragma pack(pop)

//@see https://www.zlib.net/zlib_how.html
int kdb::ZLibStream::inflateChunks(vector<byte>& buffer) {
    assert(fp_);
    fseek(fp_, offset_, SEEK_SET);

    // There could be multiple consecutive zlib streams in a single kdb+ file!
    int status = Z_DATA_ERROR;
    while(!feof(fp_)) {
        // Initialize for a new zlib stream
        z_stream stream;
        stream.zalloc = Z_NULL;
        stream.zfree  = Z_NULL;
        stream.opaque = Z_NULL;
        stream.avail_in = 0;
        stream.next_in  = Z_NULL;

        status = ::inflateInit(&stream);
        if(status != Z_OK) {
            return status;
        }
        assert(stream.total_out == 0);
        Defer cleanup{[&stream](){ ::inflateEnd(&stream); }};

        // Read in the zlib stream chunk by chunk
        array<byte, ZLib_CHUNK_SIZE> deflated;
        do {
            const auto read = fread(deflated.data(), 1, ZLib_CHUNK_SIZE, fp_);
            if(ferror(fp_)) {
                return Z_ERRNO;
            }

            // Deal with padded trailer at the end of compressed kdb+ files
            if(read >= sizeof(Trailer)) {
                const auto trailer =
                        reinterpret_cast<const Trailer*>(deflated.data());
                if(trailer->isValid()) {
                    return buffer.size() == trailer->length
                        ? Z_OK : Z_DATA_ERROR;
                }
            }

            stream.avail_in = read;
            stream.next_in  = deflated.data();
            if(stream.avail_in == 0) {
                break;
            }

            // Inflate the zlib stream chunk by chunk
            do {
                const auto offset = buffer.size();
                buffer.resize(offset + ZLib_CHUNK_SIZE);
                stream.avail_out = ZLib_CHUNK_SIZE;
                stream.next_out  = buffer.data() + offset;

                status = ::inflate(&stream, Z_NO_FLUSH);
                assert(status != Z_STREAM_ERROR);   //state not clobbered
                switch(status) {
                    case Z_NEED_DICT:  return Z_DATA_ERROR;
                    case Z_DATA_ERROR: ;//fall through
                    case Z_MEM_ERROR:  return status;
                    default:  assert(status >= Z_OK);
                }

                const auto have = ZLib_CHUNK_SIZE - stream.avail_out;
                assert(have >= 0);
                buffer.resize(offset + have);
            }
            while(stream.avail_out == 0);
        }
        while(LIKELY(status != Z_STREAM_END));

        // At the end of the zlib stream, there may still be data remaining...
        if(stream.avail_in || !feof(fp_)) {
            fseek(fp_, -stream.avail_in, SEEK_CUR);
        }
    }
    return status == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
ConstantSP kdb::loadFileEx(DatabaseUpdater &dbUpdater, const string &tablePath, const vector<string> &symList, const string &symName, const vector<string> &colNames) {
}

//////////////////////////////////////////////////////////////////////////////