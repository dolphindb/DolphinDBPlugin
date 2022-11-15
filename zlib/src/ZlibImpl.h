/*
 *  ZlibImpl.h
 *
 *  Created on: May 8, 2018
 *      Author: jccai
 */

#ifndef ZLIBPLUGIN_H_
#define ZLIBPLUGIN_H_

#include "CoreConcept.h"
#include "Util.h"
#include <cstring>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#define CHUNK 16384

extern "C" ConstantSP compressFile(Heap* heap, vector<ConstantSP>& args);

extern "C" ConstantSP decompressFile(Heap* heap, vector<ConstantSP>& args);

extern "C" ConstantSP createZlibInputStream(Heap* heap, vector<ConstantSP>& args);

class ZlibBuffer
{
public:
    ZlibBuffer(): ZlibBuffer(50 * CHUNK) { }
    size_t sgetn(char *buf, size_t size);
    size_t sputn(const char *buf, size_t size);
    size_t in_avail() { return cur-exp; }
    size_t total() { return end-beg; }
    ~ZlibBuffer() { delete [] beg; }
private:
    ZlibBuffer(size_t capacity): beg(new char[capacity]), exp(beg), cur(beg), end(beg+capacity) { }
    size_t remain() {return end-cur; }
    char *beg;
    char *exp;
    char *cur;
    char *end;
};

inline size_t ZlibBuffer::sgetn(char *buf, size_t size) {
    size_t avail = in_avail();
    size_t actual = std::min(avail, size);
    memmove(buf, exp, actual);
    exp += actual;
    return actual;
}

inline size_t ZlibBuffer::sputn(const char *buf, size_t size) {
    if(size > remain()) {
        size_t mv = exp-beg;
        memmove(beg, exp, in_avail());
        cur -= mv;
        exp = beg;
        if(size > remain())
            throw RuntimeException("ZlibBuffer: not enough buffer space.");
    }
    memcpy(cur, buf, size);
    cur += size;
    return size;
}
#if 0
class ZlibInputStream : public DataInputStream
{
public:
    ZlibInputStream(std::fstream &&src);
    ZlibInputStream(std::stringstream &&src);
protected:
    virtual IO_ERR internalStreamRead(char* buf, size_t length, size_t& actualLength) override;
    virtual IO_ERR internalClose() override;
private:
    void initZlib();
    std::fstream src_file;
    std::stringstream src_string;
    std::iostream &src_;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];
    unsigned have;
    ZlibBuffer buffer;
};
#endif
#endif /* ZLIBPLUGIN_H_ */
