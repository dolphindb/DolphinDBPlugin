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
#ifdef _MSC_VER

#else
	#include <tr1/random>
#endif
#include <chrono>

#ifdef _MSC_VER
	#define EXPORT_DECL _declspec(dllexport)
#else
	#define EXPORT_DECL 
#endif

#include "DolphinDB.h"

namespace dolphindb {

class ConstantFactory;

class EXPORT_DECL Util{
public:
	static string VER;
	static int VERNUM;
	static string BUILD;
#ifdef _MSC_VER
	const static int BUF_SIZE = 1024;
#else
	const static int BUF_SIZE;
#endif
	static int DISPLAY_ROWS;
	static int DISPLAY_COLS;
	static int DISPLAY_WIDTH;
	static int CONST_VECTOR_MAX_SIZE;
	static int SEQUENCE_SEARCH_NUM_THRESHOLD;
	static double SEQUENCE_SEARCH_RATIO_THRESHOLD;
	static int MAX_LENGTH_FOR_ANY_VECTOR;
	static const bool LITTLE_ENDIAN_ORDER;

private:
	static int cumMonthDays[13];
	static int monthDays[12];
	static int cumLeapMonthDays[13];
	static int leapMonthDays[12];
	static char escapes[128];
	static string duSyms[10];
	static long long tmporalDurationRatioMatrix[9][10];
	static long long tmporalRatioMatrix[81];
	static long long tmporalUplimit[9];
	static SmartPointer<ConstantFactory> constFactory_;

public:
	static Constant* parseConstant(int type, const string& word);
	static Constant* createConstant(DATA_TYPE dataType);
	static Constant* createNullConstant(DATA_TYPE dataType);
	static Constant* createBool(char val);
	static Constant* createChar(char val);
	static Constant* createShort(short val);
	static Constant* createInt(int val);
	static Constant* createLong(long long val);
	static Constant* createFloat(float val);
	static Constant* createDouble(double val);
	static Constant* createString(const string& val);
	static Constant* createDate(int year, int month, int day);
	static Constant* createDate(int days);
	static Constant* createMonth(int year, int month);
	static Constant* createMonth(int months);
	static Constant* createNanoTime(int hour, int minute, int second, int nanosecond);
	static Constant* createNanoTime(long long nanoseconds);
	static Constant* createTime(int hour, int minute, int second, int millisecond);
	static Constant* createTime(int milliseconds);
	static Constant* createSecond(int hour, int minute, int second);
	static Constant* createSecond(int seconds);
	static Constant* createMinute(int hour, int minute);
	static Constant* createMinute(int minutes);
	static Constant* createNanoTimestamp(int year, int month, int day, int hour, int minute, int second, int nanosecond);
	static Constant* createNanoTimestamp(long long nanoseconds);
	static Constant* createTimestamp(int year, int month, int day, int hour, int minute, int second, int millisecond);
	static Constant* createTimestamp(long long nanoseconds);
	static Constant* createDateTime(int year, int month, int day, int hour, int minute, int second);
	static Constant* createDateTime(int seconds);

	static bool isFlatDictionary(Dictionary* dict);
	static Table* createTable(Dictionary* dict, int size);
	static Table* createTable(const vector<string>& colNames, const vector<DATA_TYPE>& colTypes, INDEX size, INDEX capacity);
	static Table* createTable(const vector<string>& colNames, const vector<ConstantSP>& cols);
	static Set* createSet(DATA_TYPE keyType, INDEX capacity);
	static Dictionary* createDictionary(DATA_TYPE keyType, DATA_TYPE valueType);
	static Vector* createVector(DATA_TYPE type, INDEX size, INDEX capacity=0, bool fast=true, int extraParam=0,	void* data=0, bool containNull=false);
	static Vector* createMatrix(DATA_TYPE type, int cols, int rows, int colCapacity,int extraParam=0, void* data=0, bool containNull = false);
	static Vector* createDoubleMatrix(int cols, int rows);
	static Vector* createPair(DATA_TYPE type){
		Vector* pair=createVector(type,2);
		pair->setForm(DF_PAIR);
		return pair;
	}
	static Vector* createIndexVector(INDEX start, INDEX length);
	static Vector* createIndexVector(INDEX length, bool arrayOnly);

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

	static void toGuid(const unsigned char*, char* str);
	static bool fromGuid(const char* str, unsigned char* data);

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
	static void split(const char* s, char delim, vector<string> &elems);
	static vector<string> split(const string &s, char delim);
	inline static bool isDigit(char ch){return '0'<=ch && ch<='9';}
	inline static bool isDateDelimitor(char ch){return ch=='.' || ch=='/' || ch=='-';}
	inline static bool isLetter(char ch){return (ch>='a' && ch<='z') || (ch>='A' && ch<='Z');}
	static char escape(char original);
	static void writeDoubleQuotedString(string& dest, const string& source);

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

	static char* allocateMemory(INDEX size, bool throwIfFail = true);
	static bool isLittleEndian(){ int x=1; return *(char *)&x == 1;}
	static bool is64BIT(){return sizeof(char*)==8;}
	static bool isWindows();
	static int getCoreCount();
	static long long getPhysicalMemorySize();
	static void sleep(int milliSeconds);
	static int getLastErrorCode();
	static string getLastErrorMessage();
	static string getErrorMessage(int errCode);
};

};

#endif /* UTIL_H_ */
