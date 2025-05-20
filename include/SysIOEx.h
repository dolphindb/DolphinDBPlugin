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

namespace ddb {
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
class S3Storage;
class Encrypter;

typedef SmartPointer<Encrypter> EncrypterSP;
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
	DBResource(const string& desc) : Constant(DF_SCALAR, DT_RESOURCE, SYSTEM), desc_(desc){}
	virtual ~DBResource(){}
	virtual DATA_TYPE getRawType() const { return DT_STRING;}
	virtual bool copyable() const {return false;}
	virtual ConstantSP getInstance() const { throw RuntimeException("DBResource is not copyable.");}
	virtual ConstantSP getValue() const { throw RuntimeException("DBResource is not copyable.");}
	virtual IO_ERR serialize(const ByteArrayCodeBufferSP& buffer) const { throw RuntimeException("DBResource is not able to serialize.");}
	virtual bool containNotMarshallableObject() const {return true;}
	virtual string getString() const { return desc_;}

protected:
	string desc_;
};

} // namespace ddb

#endif /* SYSIOEX_H_ */
