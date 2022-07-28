#include <CoreConcept.h>
#include <Logger.h>
#include"httpClient.h"
#include <sys/stat.h>

using namespace std;

extern "C" ConstantSP httpCreateSubJob(Heap *heap, vector<ConstantSP> args);
extern "C" ConstantSP httpCancelSubJob(Heap *heap, vector<ConstantSP> args);
extern "C" ConstantSP httpGetJobStat(Heap *heap, vector<ConstantSP> &args);
enum HttpType {
    HttpHead = 0,
    HttpData
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

    \

    long long startIndex = 0;
};

class WriteData : public WriteBase {
public:
    WriteData(HttpType flag, int parserInterval, ConstantSP handle, FunctionDefSP parser, const SessionSP &session,
              bool needStop, HttpSession *http, bool *needParse) : WriteBase(flag, needParse),
                                                                   needStop_(needStop), parserInterval_(parserInterval),
                                                                   handle_(handle),
                                                                   parser_(parser), session_(session), http_(http) {}

    bool needStop_;
    int parserInterval_;
    ConstantSP handle_;
    FunctionDefSP parser_;
    SessionSP session_;
    HttpSession *http_;

};

class HttpSession {
public:
    WriteData data_;
    WriteHead head_;
    Semaphore isStopped_;
    std::string startUrl_;
    std::string endUrl_;
    std::string runningUrl_;
    CURL *curl_ = NULL;
    long long readByte_ = 0;
    long long dataNumber_ = 0;
    long long parseChunkProcessedCount_ = 0;
    long long prseChunkFailedCount_ = 0;
    Timestamp lastparseChunkFailedTimestamp = Timestamp(LONG_MIN);
    std::string str() {
        return startUrl_;
    }

    HttpSession(std::string startUrl, std::string endUrl, string cookieSet, SessionSP session, string runningUrl) :
            data_(HttpData, 256, nullptr, nullptr, nullptr, false, this, nullptr),
            head_(HttpHead, nullptr), isStopped_(1), cookieSet_(cookieSet), runningUrl_(runningUrl) {
        startUrl_ = startUrl;
        endUrl_ = endUrl;
        ConstantSP params, timeout, headers, parser, handle;
        isStopped_.acquire();
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
                           const ConstantSP &headers, const FunctionDefSP &parser, int parserInterval);

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

    MutilThreadString cookie_;
    string cookieSet_;
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
            : method_(method), parser_(parser), handle_(handle), http_(httpobj),
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
    httpClient::RequestMethod method_;
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
    bool flag_ = true;
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

