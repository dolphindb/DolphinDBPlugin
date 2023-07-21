/*
 *  ZlibImpl.cpp
 *
 *  Created on: May 8, 2018
 *      Author: jccai
 */

#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <iostream>
#include <cassert>
#include <exception>
#include <zlib.h>
#include "ZlibImpl.h"

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ScalarImp.h>

#define windowBits 15

int def(std::iostream &srcFile, std::iostream &dstFile, int level);
int inf(std::iostream &srcFile, std::iostream &dstFile);
void fileParse(std::fstream &srcFile, std::fstream &dstFile, vector<ConstantSP>& args, int flag);

ConstantSP compressFile(Heap* heap, vector<ConstantSP>& args) {
    std::fstream srcFile;
    std::fstream dstFile;
    int zLevel = -1;

    if(args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException("compressFile",
                                       "Invalid argument type, input filename should be a string scalar.");
    }
    if(args.size() == 2) {
        if((args[1]->getType() != DT_INT && args[1]->getType() != DT_LONG) || args[1]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException("compressFile",
                                           "Invalid argument type, zlib compression level should be an integer scalar.");
        }
        zLevel = args[1]->getType() == DT_INT ? args[1]->getInt() : args[1]->getLong();
        if(zLevel < -1 || zLevel > 9)
            throw IllegalArgumentException("compressFile",
                                           "Invalid argument type, zlib compression level should be a integer within a range of [-1, 9].");
    }

    std::string src = args[0]->getString();
    if(src[src.size() - 1] == '/') {
        src = src.substr(0, src.size() - 1);
    }

    struct stat info;
    if(lstat(src.c_str(), &info) != 0) {
        throw IOException(src + " not found.");
    } else if(S_ISDIR(info.st_mode)) { // S_ISDIR means it's a directory
        DIR *dir;
        struct dirent *ent;
        if((dir = opendir(src.c_str())) != NULL) {
            while((ent = readdir(dir)) != NULL) {
                std::string fileName = ent->d_name;
                // Skip ., .., hidden entries, and .gz
                if (fileName == "." || fileName == ".." || fileName[0] == '.' ||
                    fileName.substr(fileName.find_last_of('.') + 1) == "gz") continue;

                vector<ConstantSP> fileArgs = args;
                fileArgs[0] = new String(src + "/" + fileName);
                compressFile(heap, fileArgs);
            }
            closedir(dir);
        } else {
            throw IOException("Could not open directory: " + src);
        }
        return Util::createConstant(DT_VOID);
    } else if(S_ISREG(info.st_mode)) { // S_ISREG means it's a regular file
        fileParse(srcFile, dstFile, args, 0);
        def(srcFile, dstFile, zLevel);
        ConstantSP ret = Util::createConstant(DT_STRING);
        ret->setString(args[0]->getString() + ".gz");
        return ret;
    } else {
        throw IOException(src + " is not a regular file or directory.");
    }
}

ConstantSP decompressFile(Heap* heap, vector<ConstantSP>& args) {
    std::fstream srcFile;
    std::fstream dstFile;
    fileParse(srcFile, dstFile, args, 1);
    inf(srcFile, dstFile);
    ConstantSP ret = Util::createConstant(DT_STRING);
    std::string output = args[0]->getString();
    ret->setString(output.substr(0, output.size()-3));
    return ret;
}

void fileParse(std::fstream &srcFile, std::fstream &dstFile, vector<ConstantSP>& args, int flag) {
    std::string src, dst;
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

    try {
        srcFile.open(src, std::ios::in | std::ios::binary);
        if(!srcFile) {
            throw IOException(src + " not found.");
        }
        dstFile.open(dst, std::ios::out | std::ios::binary);
    } catch(IOException notFound) {
        throw notFound;
    } catch(...) {
        throw IOException("input or ouput file error.");
    }
}

int def(std::iostream &srcFile, std::iostream &dstFile, int level)
{
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit2(&strm, level, Z_DEFLATED, windowBits + 16, 8, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) {
        throw IOException("zlib: compress init error.");
    }
    do {
        srcFile.read(reinterpret_cast<char *>(in), CHUNK);
        strm.avail_in = srcFile.gcount();

        if (srcFile.bad()) {
            (void)deflateEnd(&strm);
            throw IOException("zlib: input file stream is bad.");
        }
        flush = srcFile.eof() ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);
            if(ret == Z_STREAM_ERROR)
                throw IOException("zlib: Z_STREAM_ERROR, state clobbered.");
            have = CHUNK - strm.avail_out;
            dstFile.write(reinterpret_cast<const char*>(out), have);
            if(dstFile.bad()) {
                (void)deflateEnd(&strm);
                throw IOException("zlib: output file stream is bad.");
            }
        } while (strm.avail_out == 0);
        if(strm.avail_in != 0)
            throw IOException("zlib: strm.avail_in != 0, not all input was used.");
    } while (flush != Z_FINISH);
    if(ret != Z_STREAM_END)
        throw IOException("zlib: Z_STREAM_END, stream was not complete.");
    (void)deflateEnd(&strm);
    return Z_OK;
}

int inf(std::iostream &srcFile, std::iostream &dstFile)
{
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit2(&strm, windowBits + 16);
    if (ret != Z_OK) {
        throw IOException("zlib: decompress init error.");
    }
    do {
        srcFile.read(reinterpret_cast<char *>(in), CHUNK);
        strm.avail_in = srcFile.gcount();
        if (srcFile.bad()) {
            (void)inflateEnd(&strm);
            throw IOException("zlib: input file stream is bad.");
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            if(ret == Z_STREAM_ERROR)
                throw IOException("zlib: Z_STREAM_ERROR, state clobbered.");
            switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    (void)inflateEnd(&strm);
                    throw IOException("zlib: Z_NEED_DICT/Z_DATA_ERROR/Z_MEM_ERROR, decompress error.");
            }
            have = CHUNK - strm.avail_out;
            dstFile.write(reinterpret_cast<const char*>(out), have);
            if(dstFile.bad()) {
                (void)inflateEnd(&strm);
                throw IOException("zlib: output file stream is bad.");
            }
        } while (strm.avail_out == 0);
    } while (ret != Z_STREAM_END);
    if(ret != Z_STREAM_END)
        throw IOException("zlib: Z_STREAM_END, stream was not complete.");
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
