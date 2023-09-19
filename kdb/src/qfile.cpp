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
const int ZLib_FORMAT_DETECT = 32;
const size_t ZLib_CHUNK_SIZE = 1 << 14;

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
    if(UNLIKELY(read < MAGIC_BYTES)) {
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
    if(UNLIKELY(len < 0)) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Read " + filename_ + " failed.");
    }
    return static_cast<size_t>(len);
}

size_t kdb::BinFile::getBodyLen() const {
    const size_t fileLen = getFileLen();
    if(UNLIKELY(fileLen < MAGIC_BYTES)) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Empty or truncated " + filename_ + ".");
    }
    return fileLen - MAGIC_BYTES;
}

size_t kdb::BinFile::readAll(vector<byte>& buffer, ptrdiff_t offset) {
    assert(fp_);
    const size_t len = getFileLen();
    if(UNLIKELY(static_cast<ptrdiff_t>(len) < offset)) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Load " + filename_ + " too little data.");
    }

    const auto initLen = buffer.size();
    buffer.resize(initLen + len);
    fseek(fp_, offset, SEEK_SET);
    const auto read = fread(buffer.data() + initLen, 1, len, fp_);
    if(UNLIKELY(offset + read < len)) {
        throw RuntimeException(PLUGIN_NAME ": "
            "Load " + filename_ + " data incomplete.");
    }
    return len;
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
    if(UNLIKELY(read < 1)) {
        throw string("error reading stream header");
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
            throw string("error reading from file");
        case Z_STREAM_ERROR:
            throw string("invalid compression level");
        case Z_DATA_ERROR:
            throw string("invalid or incomplete deflate data");
        case Z_MEM_ERROR:
            throw string("out of memory");
        case Z_VERSION_ERROR:
            throw string("zlib versoin mismatch");
        default:
            if(UNLIKELY(status < Z_OK)) {
                throw "unknown error (" + to_string(status) + ")";
            }
    }

    assert(buffer.size() >= prev);
    return buffer.size() - prev;
}

//@see https://www.zlib.net/zlib_how.html
int kdb::ZLibStream::inflateChunks(vector<byte>& buffer) {
    assert(fp_);

    const auto prev = buffer.size();
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
        if(UNLIKELY(status != Z_OK)) {
            return status;
        }
        assert(stream.total_out == 0);
        Defer cleanup{[&stream](){ ::inflateEnd(&stream); }};

        // Read in the zlib stream chunk by chunk
        array<byte, ZLib_CHUNK_SIZE> deflated;
        do {
            const auto read = fread(deflated.data(), 1, ZLib_CHUNK_SIZE, fp_);
            if(UNLIKELY(ferror(fp_))) {
                return Z_ERRNO;
            }

            // Deal with padded trailer at the end of compressed kdb+ files
            static const byte TRAILER[] = { 0x03, 0x00, 0x00, 0x00, 0x02 };
            if(LIKELY(read >= sizeof(TRAILER))) {
                if(UNLIKELY(
                    memcmp(deflated.data(), TRAILER, sizeof(TRAILER)) == 0
                )) {
                    return Z_OK;
                }
            }

            stream.avail_in = read;
            stream.next_in  = deflated.data();
            if(UNLIKELY(stream.avail_in == 0)) {
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
            while(LIKELY(stream.avail_out == 0));
        }
        while(LIKELY(status != Z_STREAM_END));

        // At the end of the zlib stream, there is still data remaining...
        if(stream.avail_in || !feof(fp_)) {
            LOG(PLUGIN_NAME ": "
                "Additioal zlib stream found in kdb+ data file "
                "(last stream = " + to_string(stream.total_in) + ","
                " data so far = " + to_string(buffer.size() - prev) + ").");
            fseek(fp_, -stream.avail_in, SEEK_CUR);
        }
    }
    return status == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

//////////////////////////////////////////////////////////////////////////////