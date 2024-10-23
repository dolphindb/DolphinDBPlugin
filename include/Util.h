/*
 * Util.h
 *
 *  Created on: Sep 2, 2012
 *      Author: dzhou
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <string.h>
#include <vector>
#include <unordered_set>
#include <ctime>
#include <tr1/random>
#include <chrono>

#include "CoreConcept.h"

using std::unordered_set;
using std::istream;

class ConstantFactory;
class CodeFactory;

#ifdef NDEBUG
#define ASSERT(x) do { (void)sizeof(x);} while (0)
#else
#include <cassert>
#define ASSERT(x) assert(x)
#endif

class Util{
public:
	static string HOME_DIR;
	static string WORKING_DIR;
	static string EXEC_DIR;
	static string START_CMD;
	static string LOG_ABOSULTE_PATH;
	static string CLUSTER_CONFIG_FULLPATH;
	static string CLUSTER_NODES_FULLPATH;
	static string CONFIG_FULLPATH;
    static string TRACE_LOG_DIR;
    static string VER;
	static int VERNUM;
	static string CLIENT_NAME;
	static string BUILD;
	static int EXPIRATION;
	static int BUF_SIZE;
	static int DISPLAY_ROWS;
	static int DISPLAY_COLS;
	static int DISPLAY_WIDTH;
	static int CONST_VECTOR_MAX_SIZE;
	static int SEQUENCE_SEARCH_NUM_THRESHOLD;
	static double SEQUENCE_SEARCH_RATIO_THRESHOLD;
	static int MAX_KEY_RANGE_FOR_BITSET;
	static double MAX_KEY_SIZE_RATIO_FOR_BITSET;
	static int MAX_KEY_RANGE_FOR_BITMAP;
	static double MAX_KEY_SIZE_RATIO_FOR_BITMAP;
	static long long CONNECT_BREATHE_TIME;
	static int SEGMENT_SIZE_IN_BIT;
	static long long MIN_SIZE_FOR_HUGE_VECTOR;
	static long long MAX_SIZE_FOR_FAST_VECTOR;
	static long long MAX_LENGTH_FOR_ANY_VECTOR;
	static long long MAX_SIZE_FOR_SEGMENT_RANGE;
	static int MAX_PARTITIONS;
	static int MAX_ITERATIONS;
	static int NON_CONTINUOUS_MEMORY_SCALE_FACTOR;
	static double MAX_MEMORY_SIZE;
	static int DISK_IO_CONCURRENCY_LEVEL;
	static const bool LITTLE_ENDIAN_ORDER;

	static __thread std::tr1::mt19937* m1;

private:
	static int cumMonthDays[13];
	static int monthDays[12];
	static int cumLeapMonthDays[13];
	static int leapMonthDays[12];
	static char escapes[128];
	static string duSyms[10];
	static long long tmporalDurationRatioMatrix[11][10];
	static long long tmporalRatioMatrix[121];
	static long long tmporalUplimit[11];
	static SmartPointer<ConstantFactory> constFactory_;
	static unordered_map<int, int> temporalOrder_;

public:
	static void initTemporalOrder();
	static char* allocateMemory(INDEX size, bool throwIfFail = true);
	static bool isLittleEndian(){ int x=1; return *(char *)&x == 1;}
	static Constant* parseConstant(int type, const string& word);
	static bool isFlatDictionary(Dictionary* dict);
	static Table* createTable(Dictionary* dict, int size);
	static Table* createTable(const vector<string>& colNames, const vector<DATA_TYPE>& colTypes, INDEX size, INDEX capacity);
	static Table* createTable(const vector<string>& colNames, const vector<ConstantSP>& cols);
	static TableSP reloadExpiredTable(Heap* heap, const TableSP& tbl);
	static Set* createSet(DATA_TYPE keyType, const SymbolBaseSP& symbolBase, INDEX capacity);
	/**
	 * @param keyExtraParam Extra information for key type, e.g., scale for decimal type.
	 * @param valueExtraParam Extra information for value type, e.g., scale for decimal type.
	 */
	static Dictionary* createDictionary(DATA_TYPE keyType, const SymbolBaseSP& keyBase, DATA_TYPE valueType,
										const SymbolBaseSP& valueBase, bool isOrdered = true, int keyExtraParam = 0,
										int valueExtraParam = 0);
	static Vector* createVector(DATA_TYPE type, INDEX size, INDEX capacity=0, bool fast=true, int extraParam=0,
		void* data=0, void** dataSegment=0, int segmentSizeInBit=0, bool containNull=false);
	static Vector* createSymbolVector(const SymbolBaseSP& symbolBase, INDEX size, INDEX capacity=0, bool fast=true,
		void* data=0, void** dataSegment=0, int segmentSizeInBit=0, bool containNull=false);
	static Vector* createCompressedVector(long long estimatedSize);
	static Vector* createRepeatingVector(const ConstantSP& scalar, INDEX length, int extraParam=0);
	static Vector* createRepeatingSymbolVector(const ConstantSP& scalar, INDEX length, const SymbolBaseSP& symbolBase);
	static Vector* createSubVector(const VectorSP& source, INDEX offset, INDEX length);
	static Vector* createArrayVector(DATA_TYPE type, INDEX size, INDEX valueSize = 0, INDEX capacity = 0,
									INDEX valueCapacity = 0, bool fastMode = true, int extraParam = 0);
	static Vector* createMatrix(DATA_TYPE type, int cols, int rows, int colCapacity,int extraParam=0,
			void* data=0, void** dataSegment=0, int segmentSizeInBit=0, bool containNull=false);
	static Vector* createSymbolMatrix(const SymbolBaseSP& symbolBase, int cols, int rows, int colCapacity, int* data=0, bool containNull=false);
	static Vector* createDoubleMatrix(int cols, int rows);
	static Vector* createPair(DATA_TYPE type, int extraParam = 0) {
		Vector* pair = createVector(type, 2, 2, /*fast*/true, extraParam);
		pair->setForm(DF_PAIR);
		return pair;
	}
	static bool recommendFastVector(int rowSize, int typeSize);
	static Constant* createInstance(Constant* model, DATA_TYPE type, int extraParam=0);
	static Vector* createIndexVector(INDEX start, INDEX length);
	static Vector* createIndexVector(INDEX length, bool arrayOnly);
	static VectorSP createInverseIndexVector(const VectorSP& index, INDEX length);
	static Constant* createConstant(DATA_TYPE dataType, int extraParam = 0);
	static Constant* createNullConstant(DATA_TYPE dataType, int extraParam = 0);
	static VectorSP prepareCleanDoubleVector(const VectorSP& x, int isFastMode);
	static ConstantSP asContiguous(const ConstantSP& x);

	static DataInputStreamSP createBlockFileInputStream(const string& filename, int devId, long long fileLength, int bufSize, long long offset, long long length);
	static Constant* createResource(long long handle, const string& desc, const FunctionDefSP& onClose, Session* session);
	static FunctionDef* createOperatorFunction(const string& name, OptrFunc func, int minParamNum, int maxParamNum, bool aggregation);
	static FunctionDef* createSystemFunction(const string& name, SysFunc func, int minParamNum, int maxParamNum, bool aggregation);
	static FunctionDef* createSystemProcedure(const string& name, SysProc func, int minParamNum, int maxParamNum);
	static FunctionDef* createPartialFunction(const FunctionDefSP& func, const vector<ConstantSP>& args);
	static ObjectSP createRegularFunctionCall(const FunctionDefSP& func, vector<ConstantSP>& args, bool qualifier = false, bool partialCall = false);
	static ObjectSP readObject(Session* session, const DataInputStreamSP& buffer);
	static ObjectSP readObject(const SQLContextSP& context, Session* session, const DataInputStreamSP& buffer);
	static Operator* readOperator(Session* session, const DataInputStreamSP& buffer);

	static DATA_TYPE getUpgradeDataType(DATA_TYPE type);
	static DATA_TYPE getUpgradeDataTypeForScanAndOver(DATA_TYPE type);
	static DATA_TYPE convertToIntegralDataType(const ConstantSP data);
	static DATA_TYPE convertToIntegralDataType(DATA_TYPE type);
	static long long getTemporalConversionRatio(DATA_TYPE first, DATA_TYPE second);
	static char getDataTypeSymbol(DATA_TYPE type);
	static string getDataTypeString(DATA_TYPE type);
	static string getDataFormString(DATA_FORM form);
	static string getTableTypeString(TABLE_TYPE type);
	static DATA_TYPE getDataType(const string& typestr);
	static DATA_FORM getDataForm(const string& formstr);
	static int getDataTypeSize(DATA_TYPE type);
	static DATA_TYPE getDataType(char ch);
	static DATA_CATEGORY getCategory(DATA_TYPE type);
	static DURATION_UNIT getDurationUnit(const string& typestr);
	static long long getTemporalDurationConversionRatio(DATA_TYPE t, DURATION_UNIT du);
	static long long getTemporalUplimit(DATA_TYPE t);

	//assume y>0
	template<class T>
	static T mod(const T& x, const T& y){
		T tmp=x % y;
		if(tmp<0)
			return tmp+y;
		else
			return tmp;
	}

	//assume y>0
	template<class T>
	static T divide(const T& x, const T& y){
		T tmp=x / y;
		if(x>=0)
			return tmp;
		else if(x%y<0)
			return tmp-1;
		else
			return tmp;
	}

	//assume y>0
	template<class T>
	static void divide(const T& x, const T& y, T& factor, T& remainder){
		factor=x / y;
		remainder=x % y;
		if(remainder<0){
			remainder+=y;
			--factor;
		}
	}

	/**
	 * Convert unsigned byte sequences to hex string.
	 *
	 * littleEndian: if true, the first byte is the least significant and should be printed at the most right.
	 * str: the length of buffer must be at least 2 * len.
	 */
	static void toHex(const unsigned char* data, int len, bool littleEndian, char* str);
	/**
	 * Convert hex string to unsigned byte sequences.
	 *
	 * len: must be a positive even number.
	 * littleEndian: if true, the first byte is the least significant, i.e. the leftmost characters would be converted to the rightmost byte.
	 * data: the length of buffer must be at least len/2
	 */
	static bool fromHex(const char* str, int len, bool littleEndian, unsigned char* data);

	static bool equalIgnoreCase(const string& str1, const string& str2);
	static string lower(const string& str);
	static string upper(const string& str);
	static char toUpper(char ch);
	static char toLower(char ch);
	static string ltrim(const string& str);
	static string trim(const string& str);
	static string strip(const string& str);
	static int wc(const char* str);
	static bool endWith(const string& str, const string& end);
	static bool startWith(const string& str, const string& start);
	static bool strWildCmp(const char* wildstring, const char* matchstring);
	static bool strCaseInsensitiveWildCmp(const char* str, const char* pat);
	static string replace(const string& str, const string& pattern, const string& replacement);
	static string replace(const string& str, char pattern, char replacement);
	static string convert(int val);
	static string longToString(long long val);
	static string doubleToString(double val);
	static bool isVariableCandidate(const string& word);
	static string literalConstant(const string& str);

	static string getRegistryString(const string& subKey, const string& name, bool machine);
	static string getEnv(const string& name, const string& defaultValue);
	static int getEnv(const string& name, int defaultValue);
	static bool getEnv(const string& name, bool defaultValue);
	static string getWorkingDirectory();
	static string getExecutableDirectory();
	static string getAdobeReaderPath();
	static FILE* fopen(const char* filename, const char* mode);
	static int rename(const char* oldName, const char* newName);
	static bool setFileTime(const string& filename, long long accessTime, long long modificationTime, int& errCode);
	static bool getDirectoryContent(const string& dir, vector<FileAttributes>& files, string& errMsg);
	static bool readTextFile(const string& filename,vector<string>& lines);
	static bool readScriptFile(const string& filename,vector<string>& lines, string& errMsg);
	static bool readMessage(const string& filename, string& message);
	static bool writeMessage(const string& filename, const string& message);
	static bool createDirectory(const string& dir, string& errMsg);
	static bool createDirectoryRecursive(const string& dir, string& errMsg);
	static bool removeDirectory(const string& dir, string& errMsg);
	static bool removeDirectoryRecursive(const string& dir, string& errMsg);
	static bool truncFile(const string& filename, long long newSize, string& errMsg);
	static bool removeFile(const string& file, string& errMsg);
	static bool copyFile(const string& srcFile , const string& destFile, string& errMsg);
	static bool syncFile(FILE* fp);
	static bool exists(const string& filename, bool& isDir);
	static bool exists(const string& filename);
	static bool existsDir(const string& filename);
	static long long getFileLength(const string& filename);
	static string getShortFilename(const string& filename);
	static string getFilePath(const string& filename);
	static bool isAbosultePath(const string& filename);

	static int countDays(int year, int month, int day);
	static int parseYear(int days);
	static void parseDate(int days, int& year, int& month, int& day);
	static int getMonthEnd(int days);
	static int getMonthStart(int days);
	static long long getNanoBenchmark();
	static long long getEpochTime();
	static long long getNanoEpochTime();
	static bool getLocalTime(struct tm& result);
	static bool getLocalTime(time_t t, struct tm& result);
	static int toLocalDateTime(int epochTime);
	static int* toLocalDateTime(int* epochTimes, int n);
	static long long toLocalTimestamp(long long epochTime);
	static long long* toLocalTimestamp(long long* epochTimes, int n);
	static long long toLocalNanoTimestamp(long long epochNanoTime);
	static long long* toLocalNaoTimestamp(long long* epochNanoTimes, int n);
	static string toMicroTimestampStr(std::chrono::system_clock::time_point& tp, bool printDate = false);
	static int compareTemporalGranularity(DATA_TYPE dt1, DATA_TYPE dt2) {
		if (getCategory(dt1) != TEMPORAL || getCategory(dt2) != TEMPORAL) {
			return 0;
		}
		const unordered_map<int, int>::const_iterator leftOrderIt = temporalOrder_.find(dt1);
		const unordered_map<int, int>::const_iterator rightOrderIt = temporalOrder_.find(dt2);

		if (leftOrderIt == temporalOrder_.cend() || rightOrderIt == temporalOrder_.cend()) {
			return 0;
		}
		int leftOrder = leftOrderIt->second;
		int rightOrder = rightOrderIt->second;
		if (leftOrder > rightOrder) {
			return 1;
		}
		else if (leftOrder == rightOrder) {
			return 0;
		}
		else return -1;
	}

	static void split(const char* s, char delim, vector<string> &elems);
	static void split(const string &s, char delim, vector<string> &elems);
	static vector<string> split(const string &s, char delim);

	inline static bool isDigit(char ch){return '0'<=ch && ch<='9';}
	inline static bool isDateDelimitor(char ch){return ch=='.' || ch=='/' || ch=='-';}
	inline static bool isLetter(char ch){return (ch>='a' && ch<='z') || (ch>='A' && ch<='Z');}
	inline static char escape(char original){return escapes[(int)original];}
	inline static bool isDFSUrl(const string& url){return url.length()>6 && Util::lower(url.substr(0, 6)) == "dfs://";}
	inline static bool isIMOLTPUrl(const std::string &url) {
		return url.length() > 7 && Util::lower(url.substr(0, 7)) == "oltp://";
	}
	static void writeDoubleQuotedString(string& dest, const string& source);

	static bool is64BIT(){
		return sizeof(char*)==8;
	}
	static bool isWindows();
	static int getCoreCount();
	static long long getPhysicalMemorySize();

	static void decode(char* buf, size_t& length);
	static void decodeFunctionFullName(const string& fullName, string& module, string& name);

	static inline void memrcpy(char* dest, const char* src, size_t size){
		const char* buf = src + (size -1);
		while(size--)
			*dest++ = *buf--;
	}

	static inline bool isPowerOfTwo(unsigned long long x) {
		return !(x == 0) && !(x & (x - 1));
	}

	static inline int getLeadingZeroBitCount(unsigned long long value){
		unsigned int high = value >> 32;
		if(high == 0){
			unsigned int low = value;
			return 32 + __builtin_clz(low);
		}
		else
			return __builtin_clz(high);
	}

	static inline unsigned long long getGreatestPowerOfTwo(unsigned long long value){
		return 1ll<<(64 - getLeadingZeroBitCount(value) - 1);
	}

	static string getLicensePubKey();
	static void cacheDataSource(const VectorSP& ds, bool enable, long long parentId = -1);
	static int execCmd(const char *cmd);
	static void sleep(int milliSeconds);
	static int getLastErrorCode();
	static string getLastErrorMessage();
	static string getErrorMessage(int errCode);
	static int rand(int x){ return (*Util::m1)() % x;}
	static unsigned int checksum(FILE* fp, long long offset, long long len);

	/**
	 * @brief Get the current license type of server
	 *
	 * @return One of 'free', 'commercial' or 'trial'
	 */
	static string getLicenseType();
	static int getLicenseExpiration();

private:
	static bool readScriptFile(const string& parentPath,const string& filename, unordered_set<string> scriptAlias, vector<string>& lines, string& errMsg);
};

inline ConstantSP evaluateObject(const ObjectSP& obj, Heap* pHeap) {
	return obj->isConstant() && !((Constant*)obj.get())->isStatic() ? ConstantSP(obj) : obj->getReference(pHeap);
}

inline ConstantSP copyIfNecessary(const ConstantSP& obj) {
	return (!obj->isTemporary() && obj->copyable()) ? obj->getValue() : obj;
}

inline IO_ERR serializeCode(Heap* pHeap, const ObjectSP& obj, const ByteArrayCodeBufferSP& buffer){
	if(!obj->isLargeConstant())
		return obj->serialize(pHeap, buffer);
	else
		return buffer->write(obj);
}

#endif /* UTIL_H_ */
