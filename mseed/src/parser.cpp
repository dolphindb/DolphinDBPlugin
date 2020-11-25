#include "parser.h"
#include "ScalarImp.h"
#include "Util.h"
#include "libmseed.h"

#if defined(__GNUC__) && __GNUC__ >= 4
#define LIKELY(x) (__builtin_expect((x), 1))
#define UNLIKELY(x) (__builtin_expect((x), 0))
#else
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#endif

Mutex mutexLock;

#ifndef LINUX
FILE *SCFmemopen(void *buf, size_t size, const char *mode)
{
	char temppath[MAX_PATH - 13];
	if (0 == GetTempPath(sizeof(temppath), temppath))
		return NULL;

	char filename[MAX_PATH + 1];
	if (0 == GetTempFileName(temppath, "SC", 0, filename))
		return NULL;

	FILE *f = fopen(filename, "wb");
	if (NULL == f)
		return NULL;

	fwrite(buf, size, 1, f);
	fclose(f);

	return fopen(filename, mode);
}
#endif

extern void print_stderr(const char *message);

extern bool processFirstBlock(MS3Record *msr, char &typeStr, VectorSP &col);

extern int processOneBlock(MS3Record *msr, VectorSP &value, vector<string> &sBuffer, char type);

ConstantSP mseedParse(Heap *heap, vector<ConstantSP> &args)
{
	if (!(
			(args[0]->getType() == DT_STRING && args[0]->getForm() == DF_SCALAR) ||
			(args[0]->getType() == DT_CHAR && args[0]->getForm() == DF_VECTOR)))
		throw IllegalArgumentException(__FUNCTION__, "Data must be a string scalar or a character vector");
	FILE *fp = NULL;
	std::shared_ptr<char> ptr;
	char* buffer=NULL;
#ifdef LINUX
	if (args[0]->getType() == DT_STRING)
	{
		std::string data = args[0]->getString();
		int size = data.size();
		try{
			buffer = (char *)malloc(size * sizeof(char));
		}
		catch(std::bad_alloc){
			throw RuntimeException("The given miniSEED file cannot be parsed because there is not enough memory");
		}
		if (buffer == NULL)
		{
			throw RuntimeException("The given miniSEED file cannot be parsed because there is not enough memory");
		}
		std::shared_ptr<char> sptr(buffer);
		ptr = sptr;
		const char *cdata = data.c_str();
		for (int i = 0; i < size; ++i)
		{
			buffer[i] = cdata[i];
		}
		fp = fmemopen((void *)buffer, size, "rw");
	}
	else
	{
		int size = args[0]->size();
		try{
			buffer = (char *)malloc(size * sizeof(char));
		}
		catch(std::bad_alloc){
			throw RuntimeException("The given miniSEED file cannot be parsed because there is not enough memory");
		}
		if (buffer == NULL)
		{
			throw RuntimeException("The given miniSEED byte stream cannot be parsed because there is not enough memory");
		}
		std::shared_ptr<char> sptr(buffer);
		ptr = sptr;
		((VectorSP)args[0])->getChar(0, size, (char *)buffer);
		fp = fmemopen((void *)buffer, size, "rw");
	}
#else
	if (args[0]->getType() == DT_STRING)
	{
		std::string data = args[0]->getString();
		int size = data.size();
		try{
			buffer = (char *)malloc(size * sizeof(char));
		}
		catch(std::bad_alloc){
			throw RuntimeException("The given miniSEED file cannot be parsed because there is not enough memory");
		}
		if (buffer == NULL)
		{
			throw RuntimeException("The given miniSEED byte stream cannot be parsed because there is not enough memory");
		}
		std::shared_ptr<char> sptr(buffer);
		ptr = sptr;
		const char *cdata = data.c_str();
		for (int i = 0; i < size; ++i)
		{
			buffer[i] = cdata[i];
		}
		mutexLock.lock();
		fp = SCFmemopen((void *)buffer, size, "rb");
		if (fp == NULL)
		{
			mutexLock.unlock();
			throw RuntimeException("Out the memory");
		}
	}
	else
	{
		int size = args[0]->size();
		try{
		buffer = (char *)malloc(size * sizeof(char));
		}
		catch(std::bad_alloc){
			throw RuntimeException("The given miniSEED file cannot be parsed because there is not enough memory");
		}
		if (buffer == NULL)
		{
			throw RuntimeException("The given miniSEED byte stream cannot be parsed because there is not enough memory");
		}
		std::shared_ptr<char> sptr(buffer);
		ptr = sptr;
		((VectorSP)args[0])->getChar(0, size, (char *)buffer);
		mutexLock.lock();
		fp = SCFmemopen((void *)buffer, size, "rb");
		if (fp == NULL)
		{
			mutexLock.unlock();
			throw RuntimeException("The given miniSEED byte stream cannot be parsed because there is not enough memory");
		}
	}
#endif
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
	try
	{
		while ((retcode = ms3_readmsr(&msr, "the byte stream", NULL, NULL, flags, 0, fp)) == MS_NOERROR)
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
				ms3_readmsr(&msr, NULL, NULL, NULL, flags, 0, NULL);
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
	catch (RuntimeException &e)
	{
		ms3_readmsr(&msr, NULL, NULL, NULL, flags, 0, NULL);
#ifndef LINUX
		mutexLock.unlock();
#endif
		throw e;
	}
	ms3_readmsr(&msr, NULL, NULL, NULL, flags, 0, NULL);
#ifndef LINUX
	mutexLock.unlock();
#endif

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
		index += line;
	}
	cols.push_back(id);
	cols.push_back(VecTime);
	cols.push_back(col);
	vector<string> colName = {"id", "time", "value"};
	TableSP ret = Util::createTable(colName, cols);
	return ret;
}
