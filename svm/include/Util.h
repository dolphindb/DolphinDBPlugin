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

#include "CoreConcept.h"

using std::unordered_set;
using std::istream;

class ConstantFactory;
class CodeFactory;

class Util{
public:
	static string HOME_DIR;
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

	static __thread std::tr1::mt19937* m1;

private:
	static int cumMonthDays[13];
	static int monthDays[12];
	static int cumLeapMonthDays[13];
	static int leapMonthDays[12];
	static char escapes[128];
	static SmartPointer<ConstantFactory> constFactory_;
	static SmartPointer<CodeFactory> codeFactory_;

public:

	static bool isLittleEndian(){ int x=1; return *(char *)&x == 1;}
	static Constant* parseConstant(int type, const string& word);
	static Table* createTable(Dictionary* dict, int size);
	static Table* createTable(const vector<string>& colNames, const vector<DATA_TYPE>& colTypes, INDEX size, INDEX capacity);
	static Table* createTable(const vector<string>& colNames, const vector<ConstantSP>& cols);
	static Set* createSet(DATA_TYPE keyType, const SymbolBaseSP& symbolBase, INDEX capacity);
	static Dictionary* createDictionary(DATA_TYPE keyType, const SymbolBaseSP& keyBase, DATA_TYPE valueType, const SymbolBaseSP& valueBase);
	static Vector* createVector(DATA_TYPE type, INDEX size, INDEX capacity=0, bool fast=true, int extraParam=0,
		void* data=0, void** dataSegment=0, int segmentSizeInBit=0, bool containNull=false);
	static Vector* createSymbolVector(const SymbolBaseSP& symbolBase, INDEX size, INDEX capacity=0, bool fast=true,
		void* data=0, void** dataSegment=0, int segmentSizeInBit=0, bool containNull=false);
	static Vector* createRepeatingVector(const ConstantSP& scalar, INDEX length, int extraParam=0);
	static Vector* createRepeatingSymbolVector(const ConstantSP& scalar, INDEX length, const SymbolBaseSP& symbolBase);
	static Vector* createMatrix(DATA_TYPE type, int cols, int rows, int colCapacity,int extraParam=0,
			void* data=0, void** dataSegment=0, int segmentSizeInBit=0, bool containNull=false);
	static Vector* createSymbolMatrix(const SymbolBaseSP& symbolBase, int cols, int rows, int colCapacity, int* data=0, bool containNull=false);
	static Vector* createPair(DATA_TYPE type){
		Vector* pair=createVector(type,2);
		pair->setForm(DF_PAIR);
		return pair;
	}
	static Constant* createInstance(Constant* model, DATA_TYPE type, int extraParam=0);
	static Vector* createIndexVector(INDEX start, INDEX length);
	static Vector* createIndexVector(INDEX length, bool arrayOnly);
	static Constant* createConstant(DATA_TYPE dataType);
	static Constant* createNullConstant(DATA_TYPE dataType);
	static Constant* createResource(long long handle, const string& desc, const FunctionDefSP& onClose, Session* session);
	static FunctionDef* createOperatorFunction(const string& name, OptrFunc func, int minParamNum, int maxParamNum, bool aggregation);
	static FunctionDef* createSystemFunction(const string& name, SysFunc func, int minParamNum, int maxParamNum, bool aggregation);
	static FunctionDef* createSystemProcedure(const string& name, SysProc func, int minParamNum, int maxParamNum);
	static ObjectSP readObject(Session* session, const DataInputStreamSP& buffer);
	static ObjectSP readObject(const SQLContextSP& context, Session* session, const DataInputStreamSP& buffer);
	static Operator* readOperator(Session* session, const DataInputStreamSP& buffer);
	static DATA_TYPE getUpgradeDataType(DATA_TYPE type);
	static DATA_TYPE getUpgradeDataTypeForScanAndOver(DATA_TYPE type);
	static DATA_TYPE convertToIntegralDataType(const ConstantSP data);
	static DATA_TYPE convertToIntegralDataType(DATA_TYPE type);
	static char getDataTypeSymbol(DATA_TYPE type);
	static string getDataTypeString(DATA_TYPE type);
	static string getDataFormString(DATA_FORM form);
	static DATA_TYPE getDataType(const string& typestr);
	static int getDataTypeSize(DATA_TYPE type);
	static DATA_TYPE getDataType(char ch);
	static DATA_CATEGORY getCategory(DATA_TYPE type);
	static int countDays(int year, int month, int day);
	static bool parseDate(int days, int& year, int& month, int& day);

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
	static string getHomeDirectory();
	static string getAdobeReaderPath();
	static bool setFileTime(const string& filename, long long accessTime, long long modificationTime, int& errCode);
	static bool getDirectoryContent(const string& dir, vector<FileAttributes>& files,int& errCode);
	static bool readTextFile(const string& filename,vector<string>& lines);
	static bool readScriptFile(const string& filename,vector<string>& lines, string& errMsg);
	static bool readMessage(const string& filename, string& message);
	static bool writeMessage(const string& filename, const string& message);
	static bool createDirectory(const string& dir, int& errCode);
	static bool createDirectoryRecursive(const string& dir, string& errMsg);
	static bool removeDirectory(const string& dir, int& errCode);
	static bool removeDirectoryRecursive(const string& dir, string& errMsg);
	static bool exists(const string& filename, bool& isDir);
	static bool exists(const string& filename);
	static string getShortFilename(const string filename);
	static string getFilePath(const string filename);
	static long long getNanoTime();
	static long long getTime();
	static bool getLocalTime(struct tm& result);

	static void split(const char* s, char delim, vector<string> &elems);
	static void split(const string &s, char delim, vector<string> &elems);
	static vector<string> split(const string &s, char delim);

	inline static bool isDigit(char ch){return '0'<=ch && ch<='9';}
	inline static bool isDateDelimitor(char ch){return ch=='.' || ch=='/' || ch=='-';}
	inline static bool isLetter(char ch){return (ch>='a' && ch<='z') || (ch>='A' && ch<='Z');}
	inline static char escape(char original){return escapes[(int)original];}

	static bool is64BIT(){
		return sizeof(char*)==8;
	}
	static bool isWindows();
	static int getCoreCount();

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
	static int getLeadingBitSet(unsigned long long x);

	static string getLicensePubKey();
	static void cacheDataSource(const VectorSP& ds, bool enable, long long parentId = -1);
	static int execCmd(const char *cmd);
	static void sleep(int milliSeconds);

private:
	static bool readScriptFile(const string& parentPath,const string& filename, unordered_set<string> scriptAlias, vector<string>& lines, string& errMsg);
};

inline ConstantSP evaluateObject(const ObjectSP& obj, Heap* pHeap) {
	return obj->isConstant() && !((Constant*)obj.get())->isStatic() ? ConstantSP(obj) : obj->getReference(pHeap);
}

inline IO_ERR serializeCode(const ObjectSP& obj, const ByteArrayBufferSP& buffer){
	if(!obj->isLargeConstant())
		return obj->serialize(buffer);
	else
		return buffer->write(obj);
}

#endif /* UTIL_H_ */
