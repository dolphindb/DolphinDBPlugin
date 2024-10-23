/*
 * Logger.h
 *
 *  Created on: Mar 29, 2016
 *      Author: dzhou
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>
#include <chrono>
#include <iomanip>

#include "SmartPointer.h"
#include "Concurrent.h"
#include "Exceptions.h"
#include "LocklessContainer.h"
#include "SysIO.h"

using std::string;
using std::ofstream;
using std::stringstream;

enum class severity_type{DEBUG, INFO, WARNING, ERR};

class LogWriter : public Runnable {
public:
	LogWriter(const SmartPointer<BlockingBoundlessQueue<string>>& buffer, const string& fileName, long long sizeLimit);
	~LogWriter(){}

protected:
	virtual void run();

private:
	void archive();
	string createArchiveFileName() const;

private:
	SmartPointer<BlockingBoundlessQueue<string>> buffer_;
	string fileName_;
	long long sizeLimit_;
	int rowCount_;
	DataOutputStreamSP out_;
};

inline uint16_t shortThreadId() {
#ifdef LINUX
	uint64_t tid = pthread_self();
#else
	uint64_t tid = GetCurrentThreadId();
#endif
	tid = tid ^ (tid >> 16) ^ (tid >> 32) ^ (tid >> 48);
	return tid & 0xffff;
}

class Logger {
public:
	Logger() :  level_(severity_type::INFO){}
	~Logger(){}
	bool start(const string& fileName, long long sizeLimit);
	void stop();
	void setLogLevel(severity_type level) { level_ = level;}
    severity_type getLogLevel() { return level_; }

	template <severity_type level>
	struct SeverityTypeToString;

	template<severity_type severity , typename...Args>
	void print(const Args&...args ) {
		try {
			stringstream stream;
			stream << getTime()
				<< std::hex
				<< std::setfill('0')
				<< std::setw(4)
				<< ','
				<< shortThreadId()
				<< std::dec
				<< std::setw(0)
				<< SeverityTypeToString<severity>::value;

			//unpack parameters by initializer list
			//https://en.cppreference.com/w/cpp/language/parameter_pack
			std::initializer_list<int>{(stream << args, 0)...};

			buffer_->push(stream.str());
		} catch (...) {
			// ignore call exceptions, usually OOM
		}
	}

private:
	string getTime();

private:
	severity_type level_;
	SmartPointer<BlockingBoundlessQueue<string>> buffer_;
	ThreadSP thread_;
};

template <> struct Logger::SeverityTypeToString<severity_type::DEBUG> {
	static constexpr const char *const value = " <DEBUG> :";
};
template <> struct Logger::SeverityTypeToString<severity_type::INFO> {
	static constexpr const char *const value = " <INFO> :";
};
template <> struct Logger::SeverityTypeToString<severity_type::WARNING> {
	static constexpr const char *const value = " <WARNING> :";
};
template <> struct Logger::SeverityTypeToString<severity_type::ERR> {
	static constexpr const char *const value = " <ERROR> :";
};

extern Logger log_inst;

#ifdef VERBOSE_LOGGING
#include <cstring>
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define XLOG log_inst.print<severity_type::DEBUG>
#define XLOG_ERR log_inst.print<severity_type::ERR>
#define XLOG_INFO log_inst.print<severity_type::INFO>
#define XLOG_WARN log_inst.print<severity_type::WARNING>

#define LOG(...) XLOG(-1, "[", __FILENAME__, ":", __LINE__, "] ", __VA_ARGS__)
#define LOG_ERR(...) XLOG_ERR(-1, "[", __FILENAME__, ":", __LINE__, "] ", __VA_ARGS__)
#define LOG_INFO(...) XLOG_INFO(-1, "[", __FILENAME__, ":", __LINE__, "] ", __VA_ARGS__)
#define LOG_WARN(...) XLOG_WARN(-1, "[", __FILENAME__, ":", __LINE__, "] ", __VA_ARGS__)
#else
#define LOG(...) do { if (log_inst.getLogLevel() <= severity_type::DEBUG) {log_inst.print<severity_type::DEBUG>(__VA_ARGS__);} } while(0)
#define LOG_ERR(...) do { log_inst.print<severity_type::ERR>(__VA_ARGS__); } while(0)
#define LOG_INFO(...) do { if (log_inst.getLogLevel() <= severity_type::INFO) {log_inst.print<severity_type::INFO>(__VA_ARGS__);} } while(0)
#define LOG_WARN(...) do { if (log_inst.getLogLevel() <= severity_type::WARNING) {log_inst.print<severity_type::WARNING>(__VA_ARGS__);} } while(0)
#endif



#endif /* LOGGER_H_ */
