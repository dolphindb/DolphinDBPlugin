/*
 * SysIOEx.h
 *
 *  Created on: Apr 30, 2018
 *      Author: dzhou
 */

#ifndef SYSIOEX_H_
#define SYSIOEX_H_

#include <string>
#include <istream>
#include <stdio.h>

#include "CoreConcept.h"

class VolumeMapper;
class Decoder;
class DBResource;
class FileResource;
class BlockIOTask;
class BasicBlockIOTask;
class HugeBuffer;
class BigArrayInputStream;
class BlockFileInputStream;
class BlockFileOutputStream;

typedef SmartPointer<VolumeMapper> VolumeMapperSP;
typedef SmartPointer<Decoder> DecoderSP;
typedef SmartPointer<DBResource> DBResourceSP;
typedef SmartPointer<FileResource> FileResourceSP;
typedef SmartPointer<BlockIOTask> BlockIOTaskSP;
typedef SmartPointer<SynchronizedQueue<BlockIOTaskSP>> BlockIOQueueSP;
typedef SmartPointer<BasicBlockIOTask> BasicBlockIOTaskSP;
typedef SmartPointer<HugeBuffer> HugeBufferSP;
typedef SmartPointer<BigArrayInputStream> BigArrayInputStreamSP;
typedef SmartPointer<BlockFileInputStream> BlockFileInputStreamSP;
typedef SmartPointer<BlockFileOutputStream> BlockFileOutputStreamSP;

class Decoder {
public:
	Decoder(int id, bool appendable) : id_(id), appendable_(appendable), codeSymbolAsString_(false){}
	virtual ~Decoder(){}
	virtual VectorSP code(const VectorSP& vec) = 0;
	virtual IO_ERR code(const VectorSP& vec, const DataOutputStreamSP& out, int& checksum) = 0;
	virtual IO_ERR decode(const VectorSP& vec, INDEX rowOffset, bool fullLoad, int checksum, const DataInputStreamSP& in,
			long long byteSize, long byteOffset, long long& postByteOffset) = 0;
	inline int getID() const {return id_;}
	inline bool isAppendable() const {return appendable_;}
	inline bool codeSymbolAsString() const { return codeSymbolAsString_;}
	void codeSymbolAsString(bool enabled) { codeSymbolAsString_ = enabled;}
private:
	int id_;
	bool appendable_;
	bool codeSymbolAsString_;
};

class VolumeMapper {
public:
	VolumeMapper(vector<string>& volumes, int workers);
	int getMappedDeviceId(const string& path);

private:
	int workers_;
	unordered_map<int, int> deviceMap_;
};

class DBResource : public Constant {
public:
	DBResource(const string& desc) : desc_(desc){}
	virtual ~DBResource(){}
	virtual DATA_TYPE getType() const {return DT_RESOURCE;}
	virtual DATA_TYPE getRawType() const { return DT_STRING;}
	virtual DATA_CATEGORY getCategory() const { return SYSTEM;}
	virtual bool copyable() const {return false;}
	virtual ConstantSP getInstance() const { throw RuntimeException("DBResource is not copyable.");}
	virtual ConstantSP getValue() const { throw RuntimeException("DBResource is not copyable.");}
	virtual IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const { throw RuntimeException("DBResource is not able to serialize.");}
	virtual bool containNotMarshallableObject() const {return true;}
	virtual string getString() const { return desc_;}

protected:
	string desc_;
};

class FileResource : public DBResource {
public:
	FileResource(const string& fileName, FILE* file) : DBResource(fileName), file_(file), closed_(false){}
	virtual ~FileResource();
	FILE* getFilePointer() const { return file_;}
	bool close();

private:
	FILE* file_;
	bool closed_;
};

class BlockIOTask {
public:
	BlockIOTask(int devId) : cancelled_(false), devId_(devId), retCode_(OK), latch_(0) {}
	virtual void execute() = 0;
	virtual ~BlockIOTask(){}
	void wait();
	void done();
	void done(const string& errMsg);
	inline int getDeviceId() const { return devId_;}
	inline const string& getErrorMessage() const {return errMsg_;}
	inline IO_ERR getReturnCode() const {return retCode_;}
	inline bool isComplete(){ return latch_.getCount() == 0;}
	inline void cancel(){ cancelled_ = true;}
	inline bool isCancelled() const {return cancelled_;}

protected:
	bool cancelled_;
	int devId_;
	IO_ERR retCode_;
	string errMsg_;
	CountDownLatch latch_;
};

class BasicBlockIOTask : public BlockIOTask{
public:
	BasicBlockIOTask(int devId) : BlockIOTask(devId), read_(true), offset_(0), length_(0), actualLength_(0), fileOffset_(0), file_(0){}
	virtual ~BasicBlockIOTask(){}
	virtual void execute();
	void setTask(bool read, const VectorSP& bufObj, int offset, int length, const FileResourceSP& file);
	inline int getActualLength() const {return actualLength_;}
	inline int getLength() const {return length_;}
	inline long long getOffset() const {return offset_;}
	inline long long getFileOffset() const {return fileOffset_;}
	inline void setFileOffset(long long offset){ fileOffset_ = offset;}
	VectorSP getBufferObject() const {return bufObj_;}
	char* getBuffer() const { return (char*)bufObj_->getDataArray();}
	FileResourceSP getFilePointer() const {return fileObj_;}

private:
	bool read_;
	int offset_;
	int length_;
	int actualLength_;
	long long fileOffset_;
	VectorSP bufObj_;
	FileResourceSP fileObj_;
	FILE* file_;
};

class HugeBuffer {
public:
	HugeBuffer(const char** dataSegment, int segmentSizeInBit, long long offset, long long size);
	IO_ERR readBytes(char* buf, int length);
	inline size_t remaining() const { return size_ - cursor_;}

private:
	char** dataSegment_;
	int segmentSizeInBit_;
	int segmentMask_;
	long long cursor_;
	long long size_;
};

class BigArrayInputStream : public DataInputStream {
public:
	BigArrayInputStream(const HugeBufferSP& obj, int bufSize = 2048) : DataInputStream(BIGARRAY_STREAM, bufSize), hugeBuf_(obj){}

protected:
	virtual IO_ERR internalStreamRead(char* buf, size_t length, size_t& actualLength);
	virtual IO_ERR internalClose(){return OK;}

private:
	HugeBufferSP hugeBuf_;
};

class BlockFileInputStream : public DataInputStream {
public:
	BlockFileInputStream(const string& filename, int devId, long long fileLength, int bufSize = -1, long long offset = 0, long long length = -1);
	long long getFileOffset() const {return fileOffset_;}

protected:
	virtual IO_ERR internalStreamRead(char* buf, size_t length, size_t& actualLength);
	virtual IO_ERR internalClose();
	virtual bool internalMoveToPosition(long long offset);

private:
	bool taskInBackground_;
	bool fixedLength_;
	int bufOffset_;
	int bufSize_;
	int bufCapacity_;
	long long fileOffset_;
	long long fileLength_;
	FileResourceSP fileObj_;
	BasicBlockIOTaskSP task_;
	VectorSP bufObj_;
	VectorSP secondBufObj_;
	char* blockBuf_;
};

class BlockFileOutputStream : public DataOutputStream {
public:
	BlockFileOutputStream(const string& filename, const string& openMode, int devId, int blockSize);
	virtual ~BlockFileOutputStream(){}
	const string& getErrorMessage() const {return errMsg_;}

protected:
	virtual IO_ERR internalFlush(size_t size);
	virtual IO_ERR internalClose();
	virtual char* createBuffer(size_t& capacity);

private:
	bool taskInBackground_;
	int blockSize_;
	string errMsg_;
	FileResourceSP fileObj_;
	BasicBlockIOTaskSP task_;
	VectorSP bufObj_;
	VectorSP secondBufObj_;
};

#endif /* SYSIOEX_H_ */
