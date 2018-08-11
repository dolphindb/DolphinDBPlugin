/*
 *  AWSS3.h
 *
 *  Created on: May 2, 2018
 *      Author: jccai
 */

#ifndef AWSS3PLUGIN_H_
#define AWSS3PLUGIN_H_

#include "CoreConcept.h"
#include "Util.h"
#include "SysIO.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <zlib.h>

#define CHUNK 16384

extern "C" ConstantSP getS3Object(Heap* heap, vector<ConstantSP>& args);
extern "C" ConstantSP listS3Object(Heap* heap, vector<ConstantSP>& args);
extern "C" ConstantSP readS3Object(Heap* heap, vector<ConstantSP>& args);
extern "C" void deleteS3Object(Heap* heap, vector<ConstantSP>& args);
extern "C" void uploadS3Object(Heap* heap, vector<ConstantSP>& args);
extern "C" ConstantSP listS3Bucket(Heap* heap, vector<ConstantSP>& args);
extern "C" void deleteS3Bucket(Heap* heap, vector<ConstantSP>& args);
extern "C" void createS3Bucket(Heap* heap, vector<ConstantSP>& args);

extern "C" ConstantSP createS3InputStream(Heap* heap, vector<ConstantSP>& args);

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

class S3InputStream : public DataInputStream
{
public:
    S3InputStream(Aws::S3::S3Client, Aws::S3::Model::GetObjectRequest, Aws::SDKOptions);
    //virtual ~S3InputStream() {internalClose();}
protected:
    virtual IO_ERR internalStreamRead(char* buf, size_t length, size_t& actualLength) override;
    virtual IO_ERR internalClose() override;
private:
    IO_ERR fileStream(char *buf, size_t length, size_t& actualLength);
    IO_ERR fileStreamZlib(char *buf, size_t length, size_t& actualLength);
    Aws::S3::S3Client s3Client_;
    Aws::S3::Model::GetObjectRequest objectRequest_;
    unsigned long long begin_ = 0;
    bool eof_ = false;
    bool err_ = false;
    Aws::SDKOptions options_;
    struct zlibStruct {
        z_stream strm;
        unsigned char in[CHUNK];
        unsigned char out[CHUNK];
        unsigned have;
        ZlibBuffer buffer;
    };
    zlibStruct* zStruct = nullptr;
};

#endif /* AWSS3PLUGIN_H_ */
