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

class Logger {
public:
	Logger() :  level_(severity_type::INFO){}
	~Logger(){}
	bool start(const string& fileName, long long sizeLimit);
	void stop();
	void setLogLevel(severity_type level) { level_ = level;}
    severity_type getLogLevel() { return level_; }

	template<severity_type severity , typename...Args>
	void print(Args...args ){
		try{
			stringstream stream;
			switch( severity ){
				case severity_type::DEBUG:
					if(level_ > severity_type::DEBUG)
						return;
					stream<<"<DEBUG> :";
					break;
				case severity_type::INFO:
					if(level_ > severity_type::INFO)
						return;
					stream<<"<INFO> :";
					break;
				case severity_type::WARNING:
					if(level_ > severity_type::WARNING)
						return;
					stream<<"<WARNING> :";
					break;
				case severity_type::ERR:
					stream<<"<ERROR> :";
					break;
			};
			printImpl(stream, args... );
		}
		catch(...){
			//ignore call exceptions, usually OOM
		}
	}

private:
	void printImpl(stringstream& stream){
		buffer_->push(getTime() + " " + stream.str());
	}
	template<typename First, typename...Rest>
	void printImpl(stringstream& stream, First parm1, Rest...parm){
		stream<<parm1;
		printImpl(stream, parm...);
	}
	string getTime();

private:
	severity_type level_;
	SmartPointer<BlockingBoundlessQueue<string>> buffer_;
	ThreadSP thread_;
};

extern Logger log_inst;

#ifdef VERBOSE_LOGGING
#include <cstring>
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define XLOG log_inst.print<severity_type::DEBUG>
#define XLOG_ERR log_inst.print<severity_type::ERR>
#define XLOG_INFO log_inst.print<severity_type::INFO>
#define XLOG_WARN log_inst.print<severity_type::WARNING>

#define LOG(...) XLOG("[", __FILENAME__, ":", __LINE__, "] ", __VA_ARGS__)
#define LOG_ERR(...) XLOG_ERR("[", __FILENAME__, ":", __LINE__, "] ", __VA_ARGS__)
#define LOG_INFO(...) XLOG_INFO("[", __FILENAME__, ":", __LINE__, "] ", __VA_ARGS__)
#define LOG_WARN(...) XLOG_WARN("[", __FILENAME__, ":", __LINE__, "] ", __VA_ARGS__)
#else
#define LOG(...) do { if (log_inst.getLogLevel() <= severity_type::DEBUG) {log_inst.print<severity_type::DEBUG>(__VA_ARGS__);} } while(0)
#define LOG_ERR(...) do { log_inst.print<severity_type::ERR>(__VA_ARGS__); } while(0)
#define LOG_INFO(...) do { if (log_inst.getLogLevel() <= severity_type::INFO) {log_inst.print<severity_type::INFO>(__VA_ARGS__);} } while(0)
#define LOG_WARN(...) do { if (log_inst.getLogLevel() <= severity_type::WARNING) {log_inst.print<severity_type::WARNING>(__VA_ARGS__);} } while(0)
#endif



#endif /* LOGGER_H_ */
