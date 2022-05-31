#include <CoreConcept.h>
#include <Logger.h>
#include"httpClient.h"
#include <sys/stat.h>

using namespace std;

extern "C" ConstantSP httpCreateSubJob(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP httpCancelSubJob(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP httpGetJobStat(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP httpCreateMutiParserSubJob(Heap *heap, vector<ConstantSP> &args);
enum HttpType {
    HttpHead = 0,
    HttpDataParse = 1,
    HttpDataManage = 2,
    HttpTypeNone
};

class MutilThreadString {
public:
    string getString() {
        string ret;
        lock_.lock();
        try {
            ret = data_;
        }
        catch (exception &e) {
            lock_.unlock();
            throw e;
        }
        lock_.unlock();
        return ret;
    }

    void setString(string &param) {
        lock_.lock();
        try {
            data_ = param;
        }
        catch (exception &e) {
            lock_.unlock();
            throw e;
        }
        lock_.unlock();
    }

    void operator+=(const string &param) {
        lock_.lock();
        try {
            data_ += param;
        }
        catch (exception &e) {
            lock_.unlock();
            throw e;
        }
        lock_.unlock();
    }

private:
    string data_;
    Mutex lock_;
};

class HttpSession;

class MutiThreadParse;

class WriteBase {
public:
    WriteBase(HttpType flag, bool *needParse) :
            flag_(flag), needParse_(needParse) {}

    HttpType flag_;
    string buffer_;
    bool *needParse_;
};

class WriteHead : public WriteBase {
public:
    WriteHead(HttpType flag, bool *needParse) : WriteBase(flag, needParse) {}

    long long startIndex = 0;
};

class WriteData : public WriteBase {
public:
    WriteData(HttpType flag, int parserInterval, ConstantSP handle, FunctionDefSP parser,
              FunctionDefSP streamHeadParser, const SessionSP &session,
              bool needStop, HttpSession *http, bool *needParse,
              vector<shared_ptr<SynchronizedQueue<string>>> &queue, int threadNum,
              shared_ptr<SynchronizedQueue<string>> &appendQueue) :
            WriteBase(flag, needParse),
            needStop_(needStop), parserInterval_(parserInterval),
            handle_(handle), parser_(parser),
            session_(session), http_(http), streamHeadParser_(streamHeadParser),
            appendQueue_(appendQueue) {}

    WriteData(HttpType flag, bool *needParse, bool needStop) : WriteBase(flag, needParse), needStop_(needStop) {}

    FunctionDefSP getStreamHeadParser() {
        return streamHeadParser_;
    }

    bool needStop_;
    int parserInterval_;
    ConstantSP handle_;
    FunctionDefSP parser_;
    SessionSP session_;
    HttpSession *http_;
    FunctionDefSP streamHeadParser_;
    shared_ptr<SynchronizedQueue<string>> appendQueue_;
};

class HttpSession {
public:
    WriteData data_;
    WriteHead head_;
    Semaphore isStopped_;
    MutilThreadString cookie_;
    string cookieSet_;
    std::string startUrl_;
    std::string endUrl_;
    std::string runningUrl_;
    CURL *curl_ = NULL;
    atomic<long long> readByte_;
    atomic<long long> dataNumber_;
    atomic<long long> parseChunkProcessedCount_;
    atomic<long long> prseChunkFailedCount_;
    atomic<long long> lastparseChunkFailedTimestamp;

    std::string str() {
        return startUrl_;
    }

    HttpSession(std::string startUrl, std::string endUrl, string cookieSet, SessionSP session, string runningUrl) :
            data_(HttpDataParse, nullptr, false),
            head_(HttpTypeNone, nullptr), isStopped_(1), cookieSet_(cookieSet), runningUrl_(runningUrl) {
        isStopped_.acquire();
        lastparseChunkFailedTimestamp.store(LONG_MIN);
        readByte_.store(0);
        dataNumber_.store(0);
        parseChunkProcessedCount_.store(0);
        prseChunkFailedCount_.store(0);
        startUrl_ = startUrl;
        endUrl_ = endUrl;
        ConstantSP params, timeout, headers, parser, handle;
        if (cookieSet != "") {
            FILE *file = fopen(cookieSet.c_str(), "rb");
            if (file != NULL) {
                char buffer[1024];
                int n;
                while ((n = fread(buffer, 1, 1024, file)) != 0) {
                    cookie_ += string(buffer, n);
                }
                fclose(file);
            } else {
                LOG_ERR("HttpClientPlugin: Can't open the file " + cookieSet);
            }
        } else {
            vector<ConstantSP> args;
            ConstantSP ret = session->getFunctionDef("getHomeDir")->call(session->getHeap().get(), args);
            if (ret->getType() != DT_STRING || ret->getForm() != DF_SCALAR) {
                LOG_ERR("HttpClientPlugin: getHomeDir must be return a string scalar");
            } else {
                string tmpUrl = startUrl;
                for (size_t i = 0; i < tmpUrl.size(); ++i) {
                    if (tmpUrl[i] == '/')
                        tmpUrl[i] = '_';
                }
                string dir = ret->getString() + "/httpClientCookie/" + tmpUrl;
                FILE *file = fopen(dir.c_str(), "rb");
                if (file != NULL) {
                    char buffer[1024];
                    int n;
                    while ((n = fread(buffer, 1, 1024, file)) != 0) {
                        cookie_ += string(buffer, n);
                    }
                    fclose(file);
                } else {
                    LOG_ERR("HttpClientPlugin: Can't open the file " + dir);
                }
            }
        }
    }

    ConstantSP httpRequest(SessionSP &session, httpClient::RequestMethod method, const ConstantSP &handle,
                           const std::string &url, const ConstantSP &params, const ConstantSP &timeout,
                           const ConstantSP &headers, const FunctionDefSP &parser, int parserInterval, bool mutiThread,
                           FunctionDefSP streamHeadParser, vector<shared_ptr<SynchronizedQueue<string>>> queue,
                           int parserNum, shared_ptr<SynchronizedQueue<string>> appendQueue);

    ~HttpSession() {
        if (curl_ != nullptr && endUrl_ != "") {
            curl_easy_setopt(curl_, CURLOPT_URL, endUrl_.c_str());
            curl_easy_perform(curl_);
        }
        if (curl_ != nullptr) {
            curl_easy_cleanup(curl_);
        }
    }

    void cleanCurl() {
        curl_easy_cleanup(curl_);
    }

    void setCookie(string param, SessionSP session) {
        cookie_.setString(param);
        if (cookieSet_ != "") {
            FILE *file = fopen(cookieSet_.c_str(), "wb");
            if (file != nullptr) {
                string tmp = cookie_.getString();
                fwrite(tmp.c_str(), 1, tmp.size(), file);
                fclose(file);
            } else {
                LOG_ERR("HttpClientPlugin: Can't open the file " + cookieSet_);
            }
        } else {
            vector<ConstantSP> args;
            ConstantSP ret = session->getFunctionDef("getHomeDir")->call(session->getHeap().get(), args);
            if (ret->getType() != DT_STRING || ret->getForm() != DF_SCALAR) {
                LOG_ERR("HttpClientPlugin: getHomeDir must be return a string scalar");
            } else {
                string tmpUrl = startUrl_;
                for (size_t i = 0; i < tmpUrl.size(); ++i) {
                    if (tmpUrl[i] == '/')
                        tmpUrl[i] = '_';
                }
                string dir = ret->getString() + "/httpClientCookie/" + tmpUrl;
                mkdir((ret->getString() + "/httpClientCookie").c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
                FILE *file = fopen(dir.c_str(), "wb");
                if (file != nullptr) {
                    string tmp = cookie_.getString();
                    fwrite(tmp.c_str(), 1, tmp.size(), file);
                    fclose(file);
                } else {
                    LOG_ERR("HttpClientPlugin: Can't write to the file " + dir);
                }
            }
        }
    }
};

class DummyOutput : public Output {
public:
    virtual bool timeElapsed(long long nanoSeconds) { return true; }

    virtual bool write(const ConstantSP &obj) { return true; }

    virtual bool message(const string &msg) { return true; }

    virtual void enableIntermediateMessage(bool enabled) {}

    virtual IO_ERR done() { return OK; }

    virtual IO_ERR done(const string &errMsg) { return OK; }

    virtual bool start() { return true; }

    virtual bool start(const string &message) { return true; }

    virtual IO_ERR writeReady() { return OK; }

    virtual ~DummyOutput() {}

    virtual OUTPUT_TYPE getOutputType() const { return STDOUT; }

    virtual void close() {}

    virtual void setWindow(INDEX index, INDEX size) {};

    virtual IO_ERR flush() { return OK; }
};


class AppendTable : public Runnable {
public:
    AppendTable(Heap *heap, httpClient::RequestMethod method, const FunctionDefSP &parser, int parserInterval,
                ConstantSP handle, HttpSession *httpobj, vector<string> &url, const ConstantSP &params,
                const ConstantSP &timeout, const ConstantSP &headers, int cycles)
            : parser_(parser), handle_(handle), http_(httpobj),
              parserInterval_(parserInterval), url_(url), params_(params), headers_(headers), timeout_(timeout),
              cycles_(cycles) {
        session_ = heap->currentSession()->copy();
        session_->setUser(heap->currentSession()->getUser());
        session_->setOutput(new DummyOutput);
    }

    void run() override;

    long long getCycles_completed() {
        return cycles_completed;
    }

    void incCycles_completed() {
        ++cycles_completed;
    }

private:
    FunctionDefSP parser_;
    ConstantSP handle_;
    HttpSession *http_;
    int parserInterval_;
    vector<string> url_;
    ConstantSP params_;
    ConstantSP headers_;
    ConstantSP timeout_;
    int cycles_;
    SessionSP session_;
    long long cycles_completed = 0;
};

class SubConnection {
private:
    string runningUrl_;
    bool connected_;
    long long createTime_{};
    Heap *heap_{};
    ThreadSP thread_;
    SessionSP session_;
    SmartPointer<AppendTable> appendTable_;
public:
    HttpSession *http_;

    SubConnection();

    SubConnection(vector<string> url, ConstantSP params, int parserInterval, ConstantSP timeout, ConstantSP headers,
                  ConstantSP handle, Heap *heap, FunctionDefSP parser, httpClient::RequestMethod method, int cycles,
                  string cookieSet);

    ~SubConnection();

    string getRunnningUrl() {
        return runningUrl_;
    }

    long long getCreateTime() const {
        return createTime_;
    }

    long long getCycles_completed() {
        return appendTable_->getCycles_completed();
    }

    SessionSP getSession() {
        return session_;
    }

    Heap *getHeap() {
        return heap_;
    }

    void cancelThread() {
        thread_->cancel();
    }
};

class MutiThreadParse : public Runnable {
public:
    MutiThreadParse(Heap *heap, FunctionDefSP &parser, ConstantSP &handle, int parserInterval, int minBlockSize, int maxBlockSize,
                    shared_ptr<SynchronizedQueue<string>> queue, shared_ptr<HttpSession> http)
            : parser_(parser), handle_(handle), parserInterval_(parserInterval), minBlockSize_(minBlockSize), maxBlockSize_(maxBlockSize), queue_(queue), http_(http) {
        session_ = heap->currentSession()->copy();
        session_->setUser(heap->currentSession()->getUser());
        session_->setOutput(new DummyOutput);
    }

    void parseAndAppendToHandle(Session *session, Heap *heap, const string & data, int parseInterval){
        //prepare arsered data index
        size_t paramStringStart = 0, paramStringEnd = 0;
        buffer_ += data;
        paramStringEnd = parseInterval + paramStringStart;
        while (buffer_.size() >= paramStringEnd) {
            try {
                vector<ConstantSP> args;
                string paramString = buffer_.substr(paramStringStart, paramStringEnd - paramStringStart);
                VectorSP paramSp = Util::createVector(DT_CHAR, paramString.size(), paramString.size());
                paramSp->setChar(0, paramString.size(), paramString.c_str());
                args.push_back(paramSp);
                ConstantSP parser_result;
                ConstantSP parser_data;
                parser_result = parser_->call(heap, args);
                if (parser_result->isDictionary()) {
                    DictionarySP tmp = (DictionarySP) parser_result;

                    //if don't have size, parser fail.
                    ConstantSP digital = tmp->getMember("size");
                    if (digital->isNull() || !digital->getType() || digital->getForm() != DF_SCALAR) {
                        throw RuntimeException("The parser must return a dictionary with STRING type keys and ANY type values. One of the keys must be \"size\" and the corresponding value must be a INT type scalar. ");
                    }

                    //if don't have data, parse fail.
                    parser_data = tmp->getMember("data");
                    if (parser_data->isNull() || !parser_data->isTable()) {
                        http_->lastparseChunkFailedTimestamp.store(Util::getEpochTime());
                        http_->prseChunkFailedCount_.fetch_add(1);
                        //while parse fail
                        LOG_ERR("HttpClientPlugin: The parser must return a dictionary with STRING type keys and ANY type values. One of the keys must be \"data\" and the corresponding value must be a table. ");
                        LOG_ERR("start:" + to_string(paramStringStart));
                        LOG_ERR("end:" + to_string(paramStringEnd));
                    }

                    ConstantSP parseChunkProcessedCount = tmp->getMember("parseChunkProcessedCount");
                    if (!parseChunkProcessedCount.isNull()) {
                        if (parseChunkProcessedCount->getType() != DT_LONG ||
                            parseChunkProcessedCount->getForm() != DF_SCALAR) {
                            throw RuntimeException(
                                    "The parser must return a dictionary with STRING type keys and ANY type values. One of the keys must be \"parseChunkProcessedCount\" and the corresponding value must be a LONG type scalar. ");
                        }
                        http_->parseChunkProcessedCount_.fetch_add(parseChunkProcessedCount->getLong());
                    }
                    size_t newStart = digital->getInt();
                    if(newStart == 0){
                        // check equal here
                        if(paramStringEnd - paramStringStart >= maxBlockSize_){
                            paramStringStart += minBlockSize_;
                        }
                    }else{
                        paramStringStart += newStart;
                    }
                    if(paramStringStart + parseInterval > paramStringEnd){
                        paramStringEnd += parseInterval;
                    }
                } else {
                    LOG_ERR("HttpClientPlugin: The parser must return a dictionary with STRING type keys and ANY type values or a table. ");
                }

                if (!parser_data.isNull() && parser_data->isTable()) {
                    http_->dataNumber_.fetch_add(parser_data->size());
                    if (handle_->isTable()) {
                        TableSP handle = handle_;

                        TableSP table_insert = (TableSP) parser_data;
                        int length = handle->columns();
                        if (table_insert->columns() < length) {
                            LOG_ERR("HttpClientPlugin: The columns of the table returned is smaller than the handler table.");
                        }
                        if (table_insert->columns() > length)
                            LOG_ERR("HttpClientPlugin: The columns of the table returned is larger than the handler table, and the information may be ignored.");
                        vector<ConstantSP> args = {handle, table_insert};
                        session->getFunctionDef("append!")->call(heap, args);
                    } else {
                        vector<ConstantSP> args = {parser_result};
                        ((FunctionDefSP) handle_)->call(heap, args);
                    }
                }
            }
            catch (exception &e) {
                LOG_ERR(string("HttpClientPlugin: ") + e.what() + "\n bufferStartIndex:" +
                        std::to_string(paramStringStart) +
                        " bufferEndIndex: " + std::to_string(paramStringEnd) + "\n");
                throw e;
            }
        }
        if (buffer_.size() - paramStringStart == 0)
            buffer_ = "";
        else
            buffer_ = buffer_.substr(paramStringStart, buffer_.size() - paramStringStart);
    }

    void run() override {
        Session *session = session_.get();
        Heap *heap = session->getHeap().get();
        while (true) {
            try {
                string popString;
                queue_->blockingPop(popString);
                parseAndAppendToHandle(session, heap, popString, parserInterval_);

                string tmpString = "";
                
                while(queue_->size() == 0 && buffer_.size() >= (size_t)maxBlockSize_){
                    parseAndAppendToHandle(session, heap, tmpString,  buffer_.size());
                }
                if(queue_->size() == 0 && buffer_.size() >= (size_t)minBlockSize_){
                    parseAndAppendToHandle(session, heap, tmpString,  buffer_.size());
                }
            }
            catch (exception &e) {
                LOG_ERR(string("HttpClientPlugin: ") + e.what());
            }
        }
    }

    void appendDataToQueue(const string &data) {
        queue_->push(data);
    }

private:
    FunctionDefSP parser_;
    ConstantSP handle_;
    int parserInterval_;
    int minBlockSize_;
    int maxBlockSize_;
    shared_ptr<SynchronizedQueue<string>> queue_;
    shared_ptr<HttpSession> http_;
    string buffer_;
    SessionSP session_;
};

class ManageMessage : public Runnable {
public:
    ManageMessage(Heap *heap,
                  const FunctionDefSP &streamHeadParser, int parserInterval, int minBlockSize, int maxBlockSize,
                  shared_ptr<HttpSession> httpobj,
                  vector<shared_ptr<SynchronizedQueue<string>>> &queue, int threadNum,
                  shared_ptr<SynchronizedQueue<string>> receive,
                  shared_ptr<Semaphore> cleanManageBuffer,
                  shared_ptr<atomic<bool>> needCleanMangeBuffer)
            : streamHeadParser_(streamHeadParser), 
            minBlockSize_(minBlockSize), maxBlockSize_(maxBlockSize), queue_(queue), receive_(receive), threadNum_(threadNum),
              cleanManageBuffer_(cleanManageBuffer), needCleanMangeBuffer_(needCleanMangeBuffer) {
        streamInfoParserInterval_ = (parserInterval == 0 ? 10240 : parserInterval) * threadNum_ * 1.5;
        session_ = heap->currentSession()->copy();
        session_->setUser(heap->currentSession()->getUser());
        session_->setOutput(new DummyOutput);
    }

    void parseAndAssign(Session *session, Heap *heap, const string &receiveString, int parseInterval) {
        size_t paramStringStart = 0, paramStringEnd = 0;
        buffer_ += receiveString;
        paramStringEnd = parseInterval + paramStringStart;
        while (buffer_.size() >= paramStringEnd) {
            try {
                vector<ConstantSP> args;
                string paramString = buffer_.substr(paramStringStart, paramStringEnd - paramStringStart);
                VectorSP paramSp = Util::createVector(DT_CHAR, paramString.size(), paramString.size());
                paramSp->setChar(0, paramString.size(), paramString.c_str());
                args.push_back(paramSp);
                ConstantSP parser_result = streamHeadParser_->call(heap, args);
                if (parser_result->getForm() != DF_DICTIONARY) {
                    throw RuntimeException(
                            "The streamHeadParser result must be a dictionary");
                }
                ConstantSP parser_data = ((DictionarySP) parser_result)->getMember("data");
                if (parser_data.isNull() || !parser_data->isTable() ||
                    ((TableSP) parser_data)->columns() != 2) {
                    throw RuntimeException(
                            "The streamHeadParser result data must be a table with two column");
                }
                ConstantSP parser_size = ((DictionarySP) parser_result)->getMember("size");
                if (parser_size.isNull() || parser_size->getType() != DT_INT ||
                    parser_size->getForm() != DF_SCALAR)
                    throw RuntimeException(
                            "The streamHeadParser result size must be a int scalar");
                VectorSP sid = parser_data->getColumn(0);
                if (!(sid->getType() == DT_SYMBOL || sid->getType() == DT_STRING))
                    throw RuntimeException(
                            "The 1rd column of streamHeadParser result must be type of symbol, string");
                VectorSP blockLen = parser_data->getColumn(1);
                if (blockLen->getType() != DT_INT)
                    throw RuntimeException(
                            "The 2rd column of streamHeadParser result must be type of int");
                int size = sid->size();
                char *sidBuffer[size];
                sid->getStringConst(0, size, sidBuffer);
                int lenBuffer[size];
                blockLen->getInt(0, size, lenBuffer);
                int threadNum = threadNum_;
                vector<string> resVec(threadNum);
                std::hash<string> hashObj;
                size_t index = 0;
                for (int i = 0; i < size; ++i) {
                    size_t t = hashObj(string(sidBuffer[i])) % threadNum;
                    resVec[t].append(buffer_.c_str() + paramStringStart + index, lenBuffer[i]);
                    index += lenBuffer[i];
                }
                size_t newStart = parser_size->getInt();
                if (newStart > buffer_.size()) {
                    throw RuntimeException(
                            "The streamHeadParser result size must be not greateran give array size");
                }
                if (index > newStart) {
                    throw RuntimeException(
                            "The streamHeadParser result size must be not greater than datalen sum");
                }
                for (int i = 0; i < threadNum; ++i) {
                    vector<shared_ptr<SynchronizedQueue<string>>> queue = queue_;
                    if (resVec[i] != "")
                        queue[i]->push(resVec[i]);
                }
                if(newStart == 0){
                    // check equal here
                    if(paramStringEnd - paramStringStart >= maxBlockSize_){
                        paramStringStart += minBlockSize_;
                    }
                }else{
                    paramStringStart += newStart;
                }
                if (paramStringStart + parseInterval > paramStringEnd){
                    paramStringEnd += parseInterval;
                }
            }catch (exception &e) {
                LOG_ERR(string("HttpClientPlugin: ") + e.what() + "\n bufferStartIndex:" +
                        std::to_string(paramStringStart) +
                        " bufferEndIndex: " + std::to_string(paramStringEnd) + "\n");
            }
        }
        if (buffer_.size() - paramStringStart == 0)
            buffer_ = "";
        else
            buffer_ = buffer_.substr(paramStringStart, buffer_.size() - paramStringStart);  
    }


    void run() {
        Session *session = session_.get();
        Heap *heap = session->getHeap().get();
        while (true) {
            //while the http reconnect, the appender will push a string like ""
            if (needCleanMangeBuffer_->load() == true && receive_->size() == 0) {
                cleanBuffer();
                continue;
            }
            string popString;
            receive_->blockingPop(popString);
            parseAndAssign(session, heap, popString, streamInfoParserInterval_);

            //while free, parse buffer str
            string tmpString = "";
            while(receive_->size() == 0 && buffer_.size() >= (size_t)maxBlockSize_){
                parseAndAssign(session, heap, tmpString, buffer_.size());
            }
            if(receive_->size() == 0 && buffer_.size() >= (size_t)minBlockSize_){
                parseAndAssign(session, heap, tmpString, buffer_.size());
            }
        }
    }

private:

    void cleanBuffer() {
        Session *session = session_.get();
        Heap *heap = session->getHeap().get();
        //Make sure all data received has been parsed
        string tmpString = "";
        while(receive_->size() == 0 && buffer_.size() >= (size_t)maxBlockSize_){
            parseAndAssign(session, heap, tmpString, buffer_.size());
        }
        if(receive_->size() == 0 && buffer_.size() >= (size_t)minBlockSize_){
            parseAndAssign(session, heap, tmpString, buffer_.size());
        }
        buffer_ = "";
        needCleanMangeBuffer_->store(false);
        cleanManageBuffer_->release();
    }

    FunctionDefSP streamHeadParser_;
    int streamInfoParserInterval_;
    int minBlockSize_;
    int maxBlockSize_;
    vector<shared_ptr<SynchronizedQueue<string>>> queue_;
    SessionSP session_;
    shared_ptr<SynchronizedQueue<string>> receive_;
    string buffer_;
    int threadNum_;
    shared_ptr<Semaphore> cleanManageBuffer_;
    shared_ptr<atomic<bool>> needCleanMangeBuffer_;
};

class AppendMessage : public Runnable {
public:
    AppendMessage(Heap *heap, httpClient::RequestMethod method, const FunctionDefSP &parser,
                  const FunctionDefSP &streamHeadParser, int parserInterval, int minBlockSize,
                  ConstantSP handle, shared_ptr<HttpSession> httpobj, vector<string> &url, const ConstantSP &params,
                  const ConstantSP &timeout, const ConstantSP &headers, int cycles,
                  vector<shared_ptr<SynchronizedQueue<string>>> &queue, int threadNum,
                  shared_ptr<SynchronizedQueue<string>> appendQueue,
                  shared_ptr<Semaphore> cleanManageBuffer, shared_ptr<atomic<bool>> needCleanManageBuffer)
            : method_(method), parser_(parser), streamHeadParser_(streamHeadParser), handle_(handle), http_(httpobj),
              parserInterval_(parserInterval), url_(url), params_(params), headers_(headers), timeout_(timeout),
              cycles_(cycles), threadNum_(threadNum), queue_(queue), appendQueue_(appendQueue),
              cleanManageBuffer_(cleanManageBuffer), needCleanManageBuffer_(needCleanManageBuffer) {
        session_ = heap->currentSession()->copy();
        session_->setUser(heap->currentSession()->getUser());
        session_->setOutput(new DummyOutput);
        cleanManageBuffer->acquire();
    }

    void run() {
        int index = 0;
        bool stopFlag = false;
        while (cycles_ == -1 || index < cycles_) {
            if (index != 0)
                LOG_WARN("HttpClientPlugin: " + http_->runningUrl_ + "The HTTP connection was reconnected.");
            ++index;
            for (size_t i = 0; i < url_.size(); ++i) {
                string url = url_[i];
                stopFlag = http_->data_.needStop_ || stopFlag;
                if (stopFlag)
                    break;
                try {
                    http_->httpRequest(session_, method_, handle_, url, params_, timeout_, headers_, parser_,
                                       parserInterval_, true, streamHeadParser_, queue_, threadNum_, appendQueue_);
                }
                catch (exception &e) {
                    LOG_ERR(string("HttpClientPlugin: ") + e.what());
                }
                LOG_WARN("HttpClientPlugin: " + http_->runningUrl_ + "The HTTP connection was disconnected.");
                stopFlag = http_->data_.needStop_ || stopFlag;
                if (stopFlag)
                    break;
            }
            stopFlag = http_->data_.needStop_ || stopFlag;
            if (stopFlag)
                break;
            incCycles_completed();

            //cout<<"set needCleanManageBuffer_ = true"<<endl;
            needCleanManageBuffer_->store(true);
            string empty = "";
            appendQueue_->push(empty);
            cleanManageBuffer_->acquire();
        }
        http_->isStopped_.release();
    }

    long long getCycles_completed() {
        return cycles_completed;
    }

    void incCycles_completed() {
        ++cycles_completed;
    }

private:
    httpClient::RequestMethod method_;
    FunctionDefSP parser_;
    FunctionDefSP streamHeadParser_;
    ConstantSP handle_;
    shared_ptr<HttpSession> http_;
    int parserInterval_;
    vector<string> url_;
    ConstantSP params_;
    ConstantSP headers_;
    ConstantSP timeout_;
    int cycles_;
    int threadNum_;
    vector<shared_ptr<SynchronizedQueue<string>>> queue_;
    SessionSP session_;
    long long cycles_completed = 0;
    shared_ptr<SynchronizedQueue<string>> appendQueue_;
    shared_ptr<Semaphore> cleanManageBuffer_;
    shared_ptr<atomic<bool>> needCleanManageBuffer_;
};

class SubConnectionMutiThread {
public:

    SubConnectionMutiThread(int parserThreadNum, vector<string> url, ConstantSP params, int minBlockSize, int maxBlockSize,
                            ConstantSP timeout, ConstantSP headers,
                            ConstantSP handle, Heap *heap, FunctionDefSP parser, FunctionDefSP streamHeadParser,
                            int threadNum, httpClient::RequestMethod method, int cycles,
                            string cookieSet, int parserInterval);

    ~SubConnectionMutiThread();

    string getRunnningUrl() {
        return runningUrl_;
    }

    long long getCreateTime() const {
        return createTime_;
    }

    SessionSP getSession() {
        return session_;
    }

    void cancelThread() {
        for (ThreadSP t : parserThread_) {
            t->cancel();
        }
        thread_->cancel();
    }

    shared_ptr<HttpSession> getHttp() {
        return http_;
    }

    long long getCycles_completed() {
        return appendMessage_->getCycles_completed();
    }

private:
    shared_ptr<HttpSession> http_;
    SessionSP session_;

    string runningUrl_;
    bool connected_;
    long long createTime_{};
    Heap *heap_{};
    vector<ThreadSP> parserThread_;
    vector<SmartPointer<MutiThreadParse>> mutiThreadParse_;
    ThreadSP thread_;
    SmartPointer<AppendMessage> appendMessage_;
    vector<shared_ptr<SynchronizedQueue<string>>> queueVec_;
    ThreadSP manageThead_;
    SmartPointer<ManageMessage> manageMessage_;
};

