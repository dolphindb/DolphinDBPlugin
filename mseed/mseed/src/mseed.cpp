#include <Exceptions.h>
#include <ScalarImp.h>
#include <Util.h>
#include <string>
#include <unordered_map>
#include "mseed.h"
#include "libmseed.h"

void print_stderr(const char *message)
{
	throw RuntimeException(message);
}

void processFirstBlock(MS3Record *msr, char &typeStr, VectorSP &col)
{
	int mIndex = 1024;
	if (msr->sampletype == 'i')
	{
		typeStr = 'i';
		col = Util::createVector(DT_INT, 0, mIndex);
	}
	else if (msr->sampletype == 'f')
	{
		typeStr = 'f';
		col = Util::createVector(DT_FLOAT, 0, mIndex);
	}
	else if (msr->sampletype == 'd')
	{
		typeStr = 'd';
		col = Util::createVector(DT_DOUBLE, 0, mIndex);
	}
	else if (msr->sampletype == 'a')
	{
		typeStr = 'a';
		col = Util::createVector(DT_SYMBOL, 0, mIndex);
	}
}

int processOneBlock(MS3Record *msr, VectorSP &value, vector<string> &sBuffer, char type)
{
	int len = msr->samplecnt;
	char *ptr = (char *)(msr->datasamples);
	if (msr->sampletype == 'a')
	{
		if (type != 'a')
			throw RuntimeException("Cannot convert from mseed asill to other type");
		sBuffer.push_back((char *)ptr);
	}
	else if (msr->sampletype == 'i')
	{
		if (type == 'a')
			throw RuntimeException("Cannot convert from mseed asill to DolphinDB INT");
		value->appendInt((int *)ptr, len);
	}
	else if (msr->sampletype == 'f')
	{
		if (type == 'a')
			throw RuntimeException("Cannot convert from mseed asill to DolphinDB FLOAT");
		value->appendFloat((float *)ptr, len);
	}
	else if (msr->sampletype == 'd')
	{
		if (type == 'a')
			throw RuntimeException("Cannot convert from mseed asill to DolphinDB DOUBLE");
		value->appendDouble((double *)ptr, len);
	}
}

ConstantSP mseedRead(Heap *heap, vector<ConstantSP> &args)
{
	if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR)
	{
		throw IllegalArgumentException(__FUNCTION__, "File must be a string");
	}
	std::string file = args[0]->getString();
	MS3Record *msr = 0;
	int retcode;
	/* Redirect libmseed logging facility to stderr for consistency */
	ms_loginit(print_stderr, NULL, print_stderr, NULL);
	static uint32_t flags = MSF_VALIDATECRC | MSF_PNAMERANGE | MSF_UNPACKDATA;
	int mIndex = 1024;

	bool first = true;
	vector<string> sBuffer(mIndex);
	vector<ConstantSP> cols;
	VectorSP col;
	char type;
	int c = 0;
	long long num = 0;
	vector<int> blockNum;
	vector<string> vecId;
	vector<long long> vecTime;
	vector<double> samprate;
	/* Loop over the input file */
	try{
		while ((retcode = ms3_readmsr(&msr, file.c_str(), NULL, NULL, flags, 0,NULL)) == MS_NOERROR)
		{
			if (first)
			{
				processFirstBlock(msr, type, col);
				first = false;
			}
			try
			{
				processOneBlock(msr, col, sBuffer, type);
			}
			catch (RuntimeException &e)
			{
				ms3_readmsr(&msr, NULL, NULL, NULL, flags, 0,NULL);
				throw e;
			}
			int len = msr->samplecnt;
			num += len;

			blockNum.push_back(len);
			vecId.push_back(string(msr->sid));
			vecTime.push_back(msr->starttime);
			samprate.push_back(msr->samprate);
		}
		
		}
	catch(RuntimeException &e){
		ms3_readmsr(&msr, NULL, NULL, NULL, flags, 0,NULL);
		throw e;
	} 
	/* Make sure everything is cleaned up */
	ms3_readmsr(&msr, NULL, NULL, NULL, flags, 0,NULL);

	VectorSP id = Util::createVector(DT_SYMBOL, num);
	VectorSP VecTime = Util::createVector(DT_TIMESTAMP, num);
	int index = 0;
	for (int i = 0; i < blockNum.size(); ++i)
	{
		int curNum = blockNum[i];
		string curId = vecId[i];
		id->fill(index, curNum, new String(curId));

		double curRate = samprate[i];
		long long curStart = vecTime[i] / 1000000;
		long long step = 1000 / samprate[i];
		long long buffer[Util::BUF_SIZE];
		int lines = curNum / Util::BUF_SIZE;
		int line = curNum % Util::BUF_SIZE;
		for (int x = 0; x < lines; ++x)
		{
			long long *p = VecTime->getLongBuffer(index + x * Util::BUF_SIZE, Util::BUF_SIZE, buffer);
			for (int y = 0; y < Util::BUF_SIZE; ++y)
			{
				p[y] = curStart;
				curStart += step;
			}
			VecTime->setLong(index, Util::BUF_SIZE, p);
		}
		long long *p = VecTime->getLongBuffer(index + lines * Util::BUF_SIZE, line, buffer);
		for (int y = 0; y < line; ++y)
		{
			p[y] = curStart;
			curStart += step;
		}
		VecTime->setLong(index, line, p);
		index += curNum;
	}
	cols.push_back(id);
	cols.push_back(VecTime);
	cols.push_back(col);
	vector<string> colName = {"id", "time", "value"};
	TableSP ret = Util::createTable(colName, cols);
	return ret;
}

void procesWrite(VectorSP &value, string &sid, double sampleRate, long long &curTime, int mIndex, DATA_TYPE type, bool &cover, int i, string &file)
{
	MS3Record *msr = NULL;
	uint32_t flags = MSF_FLUSHDATA;
	int rv;
	if (!(msr = msr3_init(msr)))
	{
		throw RuntimeException("Could not allocate MS3Record, out of memory");
	}
	strcpy(msr->sid, sid.c_str());
	msr->samprate = sampleRate;
	msr->crc = 0;
	msr->numsamples = mIndex;
	msr->starttime = curTime;
	msr->pubversion = 2;
	msr->formatversion = 2;
	msr->datasize = mIndex;
	msr->reclen = 512;
	switch (type)
	{
	case DT_INT:
	{
		int buffer[mIndex];
		msr->sampletype = 'i';
		msr->encoding = DE_STEIM2;
		msr->datasamples = value->getIntBuffer(i * mIndex, mIndex, buffer);
		break;
	}
	case DT_FLOAT:
	{
		float buffer[mIndex];
		msr->sampletype = 'f';
		msr->encoding = DE_FLOAT32;
		msr->datasamples = value->getFloatBuffer(i * mIndex, mIndex, buffer);
		break;
	}
	case DT_DOUBLE:
	{
		double buffer[mIndex];
		msr->sampletype = 'd';
		msr->encoding = DE_FLOAT64;
		msr->datasamples = value->getDoubleBuffer(i * mIndex, mIndex, buffer);
		break;
	}
	}
	msr->samplecnt = mIndex;
	if (cover)
	{
		rv = msr3_writemseed(msr, file.c_str(), cover, flags, 0);
		cover = false;
	}
	else
		rv = msr3_writemseed(msr, file.c_str(), cover, flags, 0);
	msr->datasamples = NULL;
	msr3_free(&msr);
	if (rv < 0)
	{
		throw RuntimeException("Error writing miniSEED to " + file);
	}
}

ConstantSP mseedWrite(Heap *heap, vector<ConstantSP> &args)
{
	if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR)
	{
		throw IllegalArgumentException(__FUNCTION__, "File must be a string");
	}
	if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
	{
		throw IllegalArgumentException(__FUNCTION__, "Sid must be a string");
	}
	if (args[2]->getType() != DT_TIMESTAMP || args[2]->getForm() != DF_SCALAR)
	{
		throw IllegalArgumentException(__FUNCTION__, "StartTime must be a timestamp");
	}
	if (args[3]->getType() != DT_DOUBLE || args[3]->getForm() != DF_SCALAR)
	{
		throw IllegalArgumentException(__FUNCTION__, "SampleRate must be a double");
	}
	if (args[4]->getForm() != DF_VECTOR || !(args[4]->getType() == DT_INT || args[4]->getType() == DT_FLOAT || args[4]->getType() == DT_DOUBLE))
	{
		throw IllegalArgumentException(__FUNCTION__, "Value must be a vector of int ,float and double");
	}
	bool cover = false;
	if (args.size() > 5)
	{
		if (args[5]->getType() != DT_BOOL || args[5]->getForm() != DF_SCALAR)
		{
			throw IllegalArgumentException(__FUNCTION__, "Cover must be a bool");
		}
		else
			cover = args[5]->getBool();
	}
	std::string file = args[0]->getString();
	std::string sid = args[1]->getString();
	long long startTime = args[2]->getLong();
	double sampleRate = args[3]->getDouble();
	VectorSP value = args[4];

	int dataLen = value->size();
	DATA_TYPE type = value->getType();
	uint32_t flags = MSF_FLUSHDATA;
	int rv;

	int mIndex = 8192;
	long long step = 1000000000 / sampleRate * mIndex;

	int lines = dataLen / mIndex;
	int line = dataLen % mIndex;
	long long curTime = startTime * 1000000;
	for (int i = 0; i < lines; ++i)
	{
		procesWrite(value, sid, sampleRate, curTime, mIndex, type, cover, i, file);
		curTime += step;
	}
	if (line != 0)
	{
		MS3Record *msr = NULL;
		if (!(msr = msr3_init(msr)))
		{
			throw RuntimeException("Could not allocate MS3Record, out of memory");
		}
		strcpy(msr->sid, sid.c_str());
		msr->samprate = sampleRate;
		msr->crc = 0;
		msr->numsamples = line;
		msr->starttime = curTime;
		msr->datasize = line;
		msr->pubversion = 2;
		msr->formatversion = 2;
		msr->reclen = 512;
		switch (type)
		{
		case DT_INT:
		{
			int buffer[line];
			msr->sampletype = 'i';
			msr->encoding = DE_STEIM2;
			msr->datasamples = value->getIntBuffer(lines * mIndex, line, buffer);
			break;
		}
		case DT_FLOAT:
		{
			float buffer[line];
			msr->sampletype = 'f';
			msr->encoding = DE_FLOAT32;
			msr->datasamples = value->getFloatBuffer(lines * mIndex, line, buffer);
			break;
		}
		case DT_DOUBLE:
		{
			double buffer[line];
			msr->sampletype = 'd';
			msr->encoding = DE_FLOAT64;
			msr->datasamples = value->getDoubleBuffer(lines * mIndex, line, buffer);
			break;
		}
		}
		rv = msr3_writemseed(msr, file.c_str(), cover, flags, 0);
		msr->datasamples = NULL;
		msr3_free(&msr);
		if (rv < 0)
		{
			throw RuntimeException("Error writing miniSEED to " + file);
		}
	}
	return new Bool(true);
}
