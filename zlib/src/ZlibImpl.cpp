/*
 *  ZlibImpl.cpp
 *
 *  Created on: May 8, 2018
 *      Author: jccai
 */

#include <zlib.h>
#include "ZlibImpl.h"
#include "SysIO.h"
#include <ScalarImp.h>



#ifdef _WIN32
// Windows doesn't have symbolic links.
#define lstat stat
// DO NOT include <io.h>, instead create functions in io_win32.{h,cc} and import
// them like we do below.
#endif

#define windowBits 15

static std::string PLUGIN_ZLIB_PREFIX = "[PLUGIN::ZLIB]: ";

int def(DataInputStreamSP& srcFile, DataOutputStreamSP& dstFile, int level, std::string& srcFileName, std::string& dstFileName);
int inf(DataInputStreamSP& srcFile, DataOutputStreamSP& dstFile, std::string& srcFileName, std::string& dstFileName);
void fileParse(DataInputStreamSP& srcFile, DataOutputStreamSP& dstFile, vector<ConstantSP>& args, int flag,
    std::string& src, std::string& dst);

ConstantSP compressFile(Heap* heap, vector<ConstantSP>& args) {
    DataInputStreamSP srcFile;
    DataOutputStreamSP dstFile;
    int zLevel = -1;

    if(args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException("zlib::compressFile",
                                       "Invalid argument type, input filename should be a string scalar.");
    }
    if(args.size() == 2) {
        if((args[1]->getType() != DT_INT && args[1]->getType() != DT_LONG) || args[1]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException("zlib::compressFile",
                                           "Invalid argument type, zlib compression level should be an integer scalar.");
        }
        zLevel = args[1]->getType() == DT_INT ? args[1]->getInt() : args[1]->getLong();
        if(zLevel < -1 || zLevel > 9)
            throw IllegalArgumentException("zlib::compressFile",
                                           "Invalid argument type, zlib compression level should be a integer within a range of [-1, 9].");
    }

    std::string src = args[0]->getString();
    if(src[src.size() - 1] == '/') {
        src = src.substr(0, src.size() - 1);
    }
    bool isDir = false;
    if(!Util::exists(src, isDir)) {
        throw IOException(src + " not found.");
    } else if(isDir) {
        std::vector<FileAttributes> files;
        std::string erroMsg;
        if(!Util::getDirectoryContent(src, files, erroMsg)) {
            throw IOException(PLUGIN_ZLIB_PREFIX + "Failed to open directory: " + erroMsg);
        }
        int size = files.size();
        for(int i = 0; i < size; ++i) {
            const FileAttributes& file = files[i];
            if(file.name.substr(file.name.find_last_of('.') + 1) == "gz" || 
                file.name[0] == '.') continue;
            vector<ConstantSP> fileArgs = args;
            fileArgs[0] = new String(src + "/" + file.name);
            compressFile(heap, fileArgs);
        }
        return Util::createConstant(DT_VOID);
    } else {
        std::string srcFileName, dstFileName;
        fileParse(srcFile, dstFile, args, 0, srcFileName, dstFileName);
        def(srcFile, dstFile, zLevel, srcFileName, dstFileName);
        ConstantSP ret = Util::createConstant(DT_STRING);
        ret->setString(args[0]->getString() + ".gz");
        return ret;
    }
}

ConstantSP decompressFile(Heap* heap, vector<ConstantSP>& args) {
    std::ignore = heap;
    DataInputStreamSP srcFile;
    DataOutputStreamSP dstFile;
    std::string srcFileName, dstFileName;
    fileParse(srcFile, dstFile, args, 1, srcFileName, dstFileName);
    inf(srcFile, dstFile, srcFileName, dstFileName);
    ConstantSP ret = Util::createConstant(DT_STRING);
    std::string output = args[0]->getString();
    ret->setString(output.substr(0, output.size()-3));
    return ret;
}

void fileParse(DataInputStreamSP &srcFile, DataOutputStreamSP &dstFile, vector<ConstantSP>& args, int flag,
    std::string& src, std::string& dst) {
    if(args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException("compressFile/decompressFile",
            "Invalid argument type, input filename should be a string scalar.");
    }
    src = args[0]->getString();
    if(flag == 1 && (src.size() < 4 || src.substr(src.size()-3, src.size()) != ".gz")) {
        throw IllegalArgumentException("decompressFile",
                                       "Invalid argument type, input filename should be \"*.gz\".");
    }
    if(flag == 0)
        dst = src + ".gz";
    else
        dst = src.substr(0, src.size()-3);

    FILE* srcFilePtr = Util::fopen(src.c_str(), "rb");
    if(srcFilePtr == nullptr){
        throw IOException(PLUGIN_ZLIB_PREFIX + "Input file " + src + " not found.");
    }
    srcFile = new DataInputStream(srcFilePtr, CHUNK);
    FILE* dstFilePtr = Util::fopen(dst.c_str(), "wb");
    if(dstFilePtr == nullptr){
        throw IOException(PLUGIN_ZLIB_PREFIX + "Output file " + dst + " cannot be created.");
    }
    dstFile = new DataOutputStream(dstFilePtr, CHUNK);
}

int def(DataInputStreamSP &srcFile, DataOutputStreamSP &dstFile, int level, std::string& srcFileName, std::string& dstFileName)
{
    int flush;
    int have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    int zlibRet = deflateInit2(&strm, level, Z_DEFLATED, windowBits + 16, 8, Z_DEFAULT_STRATEGY);
    if (zlibRet != Z_OK) {
        throw IOException(PLUGIN_ZLIB_PREFIX + "compress init error.");
    }
    do {
        size_t readCount = 0;
        IO_ERR streamRet = srcFile->readBytes(reinterpret_cast<char *>(in), CHUNK, readCount);
        if(streamRet != OK && streamRet != END_OF_STREAM) {
            (void)deflateEnd(&strm);
            throw RuntimeException(PLUGIN_ZLIB_PREFIX + "Failed to read file " + srcFileName + ". ");
        }
        strm.avail_in = readCount;

        flush = streamRet == END_OF_STREAM ? Z_FINISH : Z_NO_FLUSH;
        if(streamRet != END_OF_STREAM && readCount == 0) {
            continue;
        }
        strm.next_in = in;
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            zlibRet = deflate(&strm, flush);
            if(zlibRet != Z_OK && zlibRet != Z_STREAM_END && zlibRet != Z_BUF_ERROR) {
                (void)deflateEnd(&strm);
                throw IOException(PLUGIN_ZLIB_PREFIX + "Failed to compress file " + srcFileName + 
                    ", Z_OK/Z_STREAM_END/Z_BUF_ERROR expected, but got " + std::to_string(zlibRet) + ".");
            }
            have = CHUNK - strm.avail_out;
            if(dstFile->write(reinterpret_cast<char*>(out), have) != OK) {
                (void)deflateEnd(&strm);
                throw IOException(PLUGIN_ZLIB_PREFIX + "Failed to write output file " + dstFileName + ".");
            }
        } while (strm.avail_out == 0);
        if(streamRet == END_OF_STREAM || zlibRet == Z_STREAM_END) {
            break;
        }
    } while (true);
    if(zlibRet != Z_STREAM_END) {
        (void)deflateEnd(&strm);
        throw IOException(PLUGIN_ZLIB_PREFIX + "Failed to compress file " + srcFileName + 
            ", Z_STREAM_END expected, but got " + std::to_string(zlibRet) + ".");
    }
    (void)deflateEnd(&strm);
    return Z_OK;
}

int inf(DataInputStreamSP &srcFile, DataOutputStreamSP &dstFile, std::string& srcFileName, std::string& dstFileName)
{
    int zlibRet;
    int have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    zlibRet = inflateInit2(&strm, windowBits + 16);
    if (zlibRet != Z_OK) {
        throw IOException("zlib: decompress init error.");
    }
    IO_ERR readRet;
    do {
        size_t readCount = 0;
        readRet = srcFile->readBytes(reinterpret_cast<char *>(in), CHUNK, readCount);
        if(readRet != OK && readRet != END_OF_STREAM) {
            inflateEnd(&strm);
            throw RuntimeException(PLUGIN_ZLIB_PREFIX + "Failed to read file " + srcFileName + ". ");
        }
        strm.avail_in = readCount;
        int flush = readRet == END_OF_STREAM ? Z_FINISH : Z_NO_FLUSH;
        if(readRet != END_OF_STREAM && readCount == 0) {
            continue;
        }
        strm.next_in = in;
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            zlibRet = inflate(&strm, flush);
            if(zlibRet != Z_OK && zlibRet != Z_STREAM_END && zlibRet != Z_BUF_ERROR) {
                (void)inflateEnd(&strm);
                throw IOException(PLUGIN_ZLIB_PREFIX + "Failed to decompress file " + srcFileName + 
                    ", Z_OK/Z_STREAM_END/Z_BUF_ERROR expected, but got " + std::to_string(zlibRet) + ".");
            }
            have = CHUNK - strm.avail_out;
            if(dstFile->write(reinterpret_cast<char*>(out), have) != OK) {
                inflateEnd(&strm);
                throw IOException(PLUGIN_ZLIB_PREFIX + "Failed to write output file " + dstFileName + ".");
            }
        } while (strm.avail_out == 0);
        if(readRet == END_OF_STREAM || zlibRet == Z_STREAM_END) {
            break;
        }
    } while (true);
    if(zlibRet != Z_STREAM_END) {
        inflateEnd(&strm);
        throw IOException(PLUGIN_ZLIB_PREFIX + "Failed to decompress file " + srcFileName + 
            ", Z_STREAM_END expected, but got " + std::to_string(zlibRet) + ".");
    }
    (void)inflateEnd(&strm);
    return Z_OK;
}
#if 0
ConstantSP createZlibInputStream(Heap* heap, vector<ConstantSP>& args) {
    if(args[0]->getType() != DT_STRING) {
        throw IllegalArgumentException("decompressFile",
            "Invalid argument type, input filename should be a string.");
    }
    std::string file;
    file = args[0]->getString();
    {
        std::fstream test;
        try {
            test.open(file, std::ios::in | std::ios::binary);
        } catch (...) {
            throw IOException("input file not found.");
        }
    }
    DataInputStreamSP ret = new ZlibInputStream(std::fstream(file, std::ios_base::in | std::ios_base::binary));
    return ret;
}
ZlibInputStream::ZlibInputStream(std::fstream &&src): DataInputStream(FILE_STREAM), src_file(std::move(src)), src_(src_file) {
    initZlib();
}

ZlibInputStream::ZlibInputStream(std::stringstream &&src): DataInputStream(ARRAY_STREAM), src_string(std::move(src)), src_(src_string) {
    initZlib();
}

void ZlibInputStream::initZlib() {
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    int ret = inflateInit2(&strm, windowBits + 16);
    if (ret != Z_OK) {
        throw IOException("zlib: decompress init error.");
    }
}

IO_ERR ZlibInputStream::internalStreamRead(char* buf, size_t length, size_t& actualLength) {
    int ret;
    actualLength = 0;
    if(src_.eof()) {
        ret = Z_STREAM_END;
        goto Result;
    }
    else if(!src_.eof() && buffer.in_avail() >= length) {
        ret = Z_OK;
        goto Result;
    }
    else {
        while(!src_.eof() && buffer.in_avail() < length) {
            src_.read(reinterpret_cast<char *>(in), CHUNK);
            strm.avail_in = src_.gcount();
            if (src_.bad()) {
                ret = Z_ERRNO;
                goto Result;
            }
            strm.next_in = in;
            do {
                strm.avail_out = CHUNK;
                strm.next_out = out;
                ret = inflate(&strm, src_.eof() ? Z_FINISH : Z_NO_FLUSH);
                if(ret != Z_STREAM_END && ret != Z_OK) {
                    ret = Z_ERRNO;
                    goto Result;
                }
                have = CHUNK - strm.avail_out;
                buffer.sputn(reinterpret_cast<const char*>(out), have);
            } while(strm.avail_out == 0);
            if (src_.eof()) {
                ret = Z_STREAM_END;
                goto Result;
            }
            if (buffer.in_avail() >= length) {
                ret = Z_OK;
                break;
            }
        }
    }
    Result:
    if(ret == Z_OK) {
        actualLength = length;
        buffer.sgetn(buf, actualLength);
        return OK;
    }
    else if (ret == Z_STREAM_END) {
        actualLength = std::min(length, buffer.in_avail());
        buffer.sgetn(buf, actualLength);
        return buffer.in_avail() ? OK : END_OF_STREAM;
    }
    else {
        return CORRUPT;
    }
}

IO_ERR ZlibInputStream::internalClose() { 
    (void)inflateEnd(&strm);
    return OK;
}
#endif
