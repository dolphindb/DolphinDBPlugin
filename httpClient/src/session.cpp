
#include <Exceptions.h>
#include <ScalarImp.h>
#include <Util.h>
#include <curl/curl.h>
#include <openssl/ssl.h>
#include <string>

#include"session.h"

using namespace std;
DictionarySP status_dict = Util::createDictionary(DT_STRING, nullptr, DT_ANY, nullptr);

string getUTC() {
    char szBuf[256] = {0};
    struct timeval tv;
    struct timezone tz;
    struct tm *p;
    gettimeofday(&tv, &tz);
    p = gmtime(&tv.tv_sec);
    snprintf(szBuf, 256, "%02d-%02d-%02dT%02d:%02d:%02d.%06ld", p->tm_year + 1900, p->tm_mon + 1, p->tm_mday,
             p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec);

    return szBuf;
}

size_t parserWriteData(void *ptr, size_t size, size_t nmemb, WriteBase *paraMdata) {

    size_t writeSize = size * nmemb;
    size_t paramStringStart = 0, paramStringEnd = 0;
    if (*(paraMdata->needParse_) == false && paraMdata->flag_ != HttpHead)
        return writeSize;
    try {
        if (paraMdata->flag_ == HttpDataParse) {
            paraMdata->buffer_.append((char *) ptr, writeSize);
            WriteData *data = (WriteData *) paraMdata;
            ConstantSP parser_result;
            ConstantSP parser_data;
            if (*(data->needParse_) == true) {
                data->http_->readByte_.fetch_add(writeSize);
                size_t parserInterval = data->parserInterval_ == 0 ? 10240 : data->parserInterval_;
                Session *session = data->session_.get();
                Heap *heap = session->getHeap().get();
                paramStringEnd = paramStringStart + parserInterval;
                while (data->buffer_.size() >= paramStringEnd) {
                    //make paramString len greater or equal parserInterval
                    vector<ConstantSP> args;
                    string paramString = data->buffer_.substr(paramStringStart, paramStringEnd - paramStringStart);
                    VectorSP paramSp = Util::createVector(DT_CHAR, paramString.size(), paramString.size());
                    paramSp->setChar(0, paramString.size(), paramString.c_str());
                    args.push_back(paramSp);
                    parser_result = data->parser_->call(heap, args);
                    if (parser_result->isDictionary()) {
                        DictionarySP tmp = (DictionarySP) parser_result;

                        //if don't have size, sub fail.
                        ConstantSP digital = tmp->getMember("size");
                        if (digital->isNull() || !digital->getType() || digital->getForm() != DF_SCALAR) {
                            throw RuntimeException(
                                    "The parser must return a dictionary with STRING type keys and ANY type values. One of the keys must be \"size\" and the corresponding value must be a INT type scalar. ");
                        }

                        //if don't have data, parse fail.
                        parser_data = tmp->getMember("data");
                        if (parser_data->isNull() || !parser_data->isTable()) {
                            data->http_->lastparseChunkFailedTimestamp.store(Util::getEpochTime());
                            data->http_->prseChunkFailedCount_.fetch_add(1);
                            //while parse fail
                            LOG_ERR("HttpClientPlugin: The parser must return a dictionary with STRING type keys and ANY type values. One of the keys must be \"data\" and the corresponding value must be a table. ");
                            LOG_ERR("start:" + to_string(paramStringStart));
                            LOG_ERR("end:" + to_string(paramStringEnd));
                        }

                        ConstantSP parseChunkProcessedCount = tmp->getMember("parseChunkProcessedCount");
                        if (!parseChunkProcessedCount->isNull()) {
                            if (parseChunkProcessedCount->getType() != DT_LONG ||
                                parseChunkProcessedCount->getForm() != DF_SCALAR) {
                                throw RuntimeException(
                                        "The parser must return a dictionary with STRING type keys and ANY type values. One of the keys must be \"parseChunkProcessedCount\" and the corresponding value must be a LONG type scalar. ");
                            }
                            data->http_->parseChunkProcessedCount_.fetch_add(parseChunkProcessedCount->getLong());
                        }
                        paramStringStart += digital->getInt();
                        paramStringEnd += parserInterval;
                        if(paramStringEnd - paramStringStart > 5 * parserInterval){
                            paramStringStart += parserInterval;
                        }
                    } else {
                        throw RuntimeException(
                                "The parser must return a dictionary with STRING type keys and ANY type values or a table. ");
                    }
                    if (!parser_data.isNull() && parser_data->isTable()) {
                        data->http_->dataNumber_.fetch_add(parser_data->size());
                        if (data->handle_->isTable()) {
                            TableSP handle = data->handle_;
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
                            ((FunctionDefSP) data->handle_)->call(heap, args);
                        }
                    }
                }
                data->buffer_ = data->buffer_.substr(paramStringStart, data->buffer_.size() - paramStringStart);
            }
        } else if (paraMdata->flag_ == HttpDataManage) {
            WriteData *data = (WriteData *) paraMdata;
            if (*(data->needParse_) == true) {
                data->http_->readByte_.fetch_add(writeSize);
                data->appendQueue_->push(string((char *) ptr, writeSize));
            }
            data->buffer_ = "";
        } else if (paraMdata->flag_ == HttpHead && *(paraMdata->needParse_) == false) {
            paraMdata->buffer_.append((char *) ptr, writeSize);
            WriteHead *data = (WriteHead *) paraMdata;
            size_t index = data->startIndex;
            while (index < data->buffer_.size()) {
                index = data->startIndex;
                while (index < data->buffer_.size()) {
                    if (data->buffer_[index] == '\n')
                        break;
                    ++index;
                }
                if (index < data->buffer_.size()) {
                    string tmp = data->buffer_.substr(data->startIndex,
                                                      index - data->startIndex + 1);
                    if (tmp == "Content-type: application/octet-stream\r\n" ||
                        tmp == "Content-Type: application/octet-stream\r\n") {
                        *(data->needParse_) = true;
                    }
                    data->startIndex = index + 1;
                }
            }
        }
        if (paraMdata->flag_ != HttpHead && ((WriteData *) paraMdata)->needStop_)
            return -1;
    }
    catch (exception &e) {
        LOG_ERR(string("HttpClientPlugin: ") + e.what() + "\n bufferStartIndex:" +
                std::to_string(paramStringStart) +
                " bufferEndIndex: " + std::to_string(paramStringStart) + "\n");
        return -1;
    }

    return
            writeSize;
}

std::string findCookie(std::string headers) {
    int pos = -1;
    pos = headers.find("Set-Cookie");
    if (-1 != pos) {
        int pos2 = headers.find('\n', pos);
        if (-1 != pos2 && pos2 - pos - 12 > 0) {
            return headers.substr(pos + 12, pos2 - pos - 12);
        }
    }
    return "";
}

template<class T>
static void httpConnectionOnClose(Heap *heap, vector<ConstantSP> &args) {
    delete (T *) (args[0]->getLong());
}

ConstantSP safeOp(const ConstantSP &arg, std::function<ConstantSP(HttpSession *)> &&f) {
    if (arg->getType() == DT_RESOURCE && arg->getLong() != 0) {
        auto conn = (HttpSession *) (arg->getLong());
        return f(conn);
    } else {
        throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
    }
}

CURL *initAndConfigCurl(httpClient::RequestMethod method, const ConstantSP &params, const ConstantSP &timeout,
                        const ConstantSP &headers, string &cookies) {
    CURL *curl = curl_easy_init();
    if (!curl)
        throw RuntimeException("Could not initialize request object");


    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.47.0");
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout->getLong());
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    //curl_easy_setopt(curl, CURLOPT_TIMEOUT, 150000);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 50);
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    //curl_easy_setopt(curl, CURLOPT_TIMEOUT, 150000);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 50);
    return curl;
}

ConstantSP
HttpSession::httpRequest(SessionSP &session, httpClient::RequestMethod method,
                         const ConstantSP &handle,
                         const std::string &url, const ConstantSP &params,
                         const ConstantSP &timeout,
                         const ConstantSP &headers, const FunctionDefSP &parser, int parserInterval,
                         bool mutiThread, FunctionDefSP streamHeadParser,
                         vector<shared_ptr<SynchronizedQueue<string>>> queue, int parserNum,
                         shared_ptr<SynchronizedQueue<string>> appendQueue) {
    if (session.isNull())
        return new Bool(false);
    ConstantSP res = Util::createDictionary(DT_STRING, nullptr, DT_ANY, nullptr);
    std::string urlString = url;
    CURL *curl = (curl_ == NULL) ? curl_easy_init() : curl_;

    if (!curl)
        throw RuntimeException("Could not initialize request object");
    curl_ = curl;
    char errorBuf[CURL_ERROR_SIZE];
    string paramString;
    if (params->getForm() == DF_DICTIONARY)
        paramString = httpClient::getParamString(params);
    else
        paramString = params->getString();
    switch (method) {
        case httpClient::GET:
            if (!paramString.empty()) {
                urlString += "?";
                urlString += paramString;
            }
            break;
        case httpClient::POST:
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, paramString.c_str());
            break;
        default:
            throw RuntimeException("Unknown request method");
    }

    curl_slist *headerList = NULL;
    if (headers->getForm() == DF_DICTIONARY)
        headerList = httpClient::setHeaders(headers, headerList);
    else {
        string str_headers = headers->getString();
        if (!str_headers.empty()) {
            int pos = str_headers.find_first_of(':');
            int last = str_headers.find_last_of(':');
            if (pos != last || pos == -1 || pos == 0 || pos == (int) str_headers.size() - 1) {
                throw IllegalArgumentException(__FUNCTION__,
                                               "If the parameter 'header' is a STRING, it must be in the format of XXX:XXX. ");
            }
        }
        headerList = curl_slist_append(headerList, str_headers.c_str());
    }
    if (cookie_.getString() != "") {
        headerList = curl_slist_append(headerList, (("Cookie: " + cookie_.getString()).c_str()));
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);

    curl_easy_setopt(curl, CURLOPT_URL, urlString.c_str());
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.47.0");
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuf);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout->getLong());

    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

    bool needParse = false;
    head_ = WriteHead(HttpHead, &needParse);
    if (mutiThread)
        data_ = WriteData(HttpDataManage, parserInterval, handle, parser, streamHeadParser, session,
                          data_.needStop_, this, &needParse, queue, parserNum, appendQueue);
    else
        data_ = WriteData(HttpDataParse, parserInterval, handle, parser, streamHeadParser, session,
                          data_.needStop_, this, &needParse, queue, parserNum, appendQueue);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &head_);
    if (parser.isNull()) {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, httpClient::curlWriteData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data_);
    } else {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, parserWriteData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (WriteData *) &data_);
    }

    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    //curl_easy_setopt(curl, CURLOPT_TIMEOUT, 150000);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 50);

    if (params->getForm() == DF_DICTIONARY)
        paramString = httpClient::getParamString(params);
    else
        paramString = params->getString();
    CURLcode ret;
    try {
        ret = curl_easy_perform(curl);
    }
    catch (exception &e) {
        throw e;
    }
    if (findCookie(head_.buffer_) != "") {
        setCookie(findCookie(head_.buffer_), session);
    }
    if (data_.needStop_) {
        return new Void();
    }
    
    size_t paramStringStart = 0;
    if (data_.flag_ == HttpDataParse) {
        ConstantSP parser_result;
        ConstantSP parser_data;
        if (*(data_.needParse_) == true) {
            size_t parserInterval = data_.parserInterval_ == 0 ? 10240 : data_.parserInterval_;
            Session *session = data_.session_.get();
            Heap *heap = session->getHeap().get();
            while (data_.buffer_.size() > paramStringStart) {
                //make paramString len greater or equal parserInterval
                vector<ConstantSP> args;
                string paramString = data_.buffer_.substr(paramStringStart, data_.buffer_.size() - paramStringStart);
                VectorSP paramSp = Util::createVector(DT_CHAR, paramString.size(), paramString.size());
                paramSp->setChar(0, paramString.size(), paramString.c_str());
                args.push_back(paramSp);
                parser_result = data_.parser_->call(heap, args);
                if (parser_result->isDictionary()) {
                    DictionarySP tmp = (DictionarySP) parser_result;

                    //if don't have size, sub fail.
                    ConstantSP digital = tmp->getMember("size");
                    if (digital->isNull() || !digital->getType() || digital->getForm() != DF_SCALAR) {
                        throw RuntimeException(
                                "The parser must return a dictionary with STRING type keys and ANY type values. One of the keys must be \"size\" and the corresponding value must be a INT type scalar. ");
                    }

                    //if don't have data, parse fail.
                    parser_data = tmp->getMember("data");
                    if (parser_data->isNull() || !parser_data->isTable()) {
                        data_.http_->lastparseChunkFailedTimestamp.store(Util::getEpochTime());
                        data_.http_->prseChunkFailedCount_.fetch_add(1);
                        //while parse fail
                        LOG_ERR("HttpClientPlugin: The parser must return a dictionary with STRING type keys and ANY type values. One of the keys must be \"data\" and the corresponding value must be a table. ");
                        LOG_ERR("start:" + to_string(paramStringStart));
                    }

                    ConstantSP parseChunkProcessedCount = tmp->getMember("parseChunkProcessedCount");
                    if (!parseChunkProcessedCount->isNull()) {
                        if (parseChunkProcessedCount->getType() != DT_LONG ||
                            parseChunkProcessedCount->getForm() != DF_SCALAR) {
                            throw RuntimeException(
                                    "The parser must return a dictionary with STRING type keys and ANY type values. One of the keys must be \"parseChunkProcessedCount\" and the corresponding value must be a LONG type scalar. ");
                        }
                        data_.http_->parseChunkProcessedCount_.fetch_add(parseChunkProcessedCount->getLong());
                    }
                    paramStringStart += digital->getInt();
                    if(digital->getInt() == 0){
                        paramStringStart += parserInterval;
                    }
                } else {
                    throw RuntimeException(
                            "The parser must return a dictionary with STRING type keys and ANY type values or a table. ");
                }
            }
            if (!parser_data.isNull() && parser_data->isTable()) {
                Session *session = data_.session_.get();
                Heap *heap = session->getHeap().get();
                data_.http_->dataNumber_.fetch_add(parser_data->size());
                if (data_.handle_->isTable()) {
                    TableSP handle = data_.handle_;
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
                    ((FunctionDefSP) data_.handle_)->call(heap, args);
                }
            }
            data_.buffer_ = "";
        }
    }
    if (ret != 0) {
        return res;
        string errorMsg(errorBuf);
        curl_slist_free_all(headerList);
        throw RuntimeException("curl returns: " + errorMsg);
    }
    long responseCode;
    double elapsed;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
    curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &elapsed);
    if (responseCode != 200)
        LOG_ERR("HttpClientPlugin: " + head_.buffer_ + data_.buffer_);
    curl_slist_free_all(headerList);
    return res;
}


void AppendTable::run() {
    int index = 0;
    bool stopFlag = false;
    while (cycles_ == -1 || index < cycles_) {
        ++index;
        for (size_t i = 0; i < url_.size(); ++i) {
            string url = url_[i];
            stopFlag = http_->data_.needStop_ || stopFlag;
            if (stopFlag)
                break;
            try {
                vector<shared_ptr<SynchronizedQueue<string>>> queue;
                shared_ptr<SynchronizedQueue<string>> appendQueue;
                http_->httpRequest(session_, httpClient::GET, handle_, url, params_, timeout_,
                                   headers_,
                                   parser_,
                                   parserInterval_, false, nullptr, queue, 0, appendQueue);
            }
            catch (exception &e) {
                LOG_ERR(string("HttpClientPlugin: ") + e.what());
            }
            stopFlag = http_->data_.needStop_ || stopFlag;
            if (stopFlag)
                break;
        }
        stopFlag = http_->data_.needStop_ || stopFlag;
        if (!stopFlag && http_ != nullptr) {
            LOG_WARN("HttpClientPlugin: " + http_->runningUrl_ +
                     "The HTTP connection was disconnected.");
        }
        if (stopFlag)
            break;
        incCycles_completed();
    }
    http_->isStopped_.release();
}

SubConnection::SubConnection() {
    connected_ = false;
}

SubConnection::~SubConnection() {
    delete http_;
    if (connected_) {
        connected_ = false;
    }
}

ConstantSP httpCreateSubJob(Heap *heap, vector<ConstantSP> &args) {
    const auto usage = string(
            "Usage: createSubJob(consumer, table, parser).\n"
    );

    ConstantSP params, timeout, headers, parser, handle;
    string cookiesSet;
    vector<string> url;
    int parserInterval, cycles;
    if (args[0]->getType() != DT_STRING ||
        (args[0]->getForm() != DF_VECTOR && args[0]->getForm() != DF_SCALAR)) {
        throw IllegalArgumentException(__FUNCTION__,
                                       "Url must be a string scalar or a string vector. ");
    } else if (args[0]->getForm() == DF_VECTOR) {
        int size = args[0]->size();
        if (size == 0) {
            throw IllegalArgumentException(__FUNCTION__,
                                           "If the parameter 'url' is a vector, its length must be greater than 0. ");
        }
        for (int i = 0; i < size; ++i) {
            url.push_back(args[0]->getString(i));
        }
    } else {
        url.push_back(args[0]->getString());
    }
    handle = args[1];
    if (!handle->isTable())
        throw IllegalArgumentException(__FUNCTION__, "Hadle must be a table. ");
    parser = args[2];
    if (parser->getType() != DT_FUNCTIONDEF || parser->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, "Parser must be a function. ");
    if (args.size() >= 4 && !args[3]->isNull()) {
        if (args[3]->getType() != DT_INT || args[3]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__,
                                           "ParserInterval must be an integer scalar");
        }
        parserInterval = args[3]->getInt();
        if (parserInterval <= 0)
            throw RuntimeException("ParserInterval must be a positive integer scalar");
    } else
        parserInterval = 0;
    if (args.size() >= 5 && !args[4]->isNull()) {
        if (args[4]->getType() != DT_INT || args[4]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, "Timeout must be an integer. ");
        cycles = args[4]->getInt();
    } else
        cycles = -1;
    if (args.size() >= 6 && !args[5]->isNull()) {
        if (args[5]->getType() != DT_STRING || args[4]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, "CookieSet must be an string scalar. ");
        cookiesSet = args[5]->getString();
    }
    if (args.size() >= 7 && !args[6]->isNull()) {
        params = args[6];
        if ((params->getType() != DT_STRING || params->getForm() != DF_SCALAR) &&
            (params->getForm() != DF_DICTIONARY ||
             ((DictionarySP) params)->getKeyType() != DT_STRING ||
             ((DictionarySP) params)->getType() != DT_STRING))
            throw IllegalArgumentException(__FUNCTION__,
                                           "Params must be a string or a dictionary with STRING-STRING key-value type");
    } else params = new String("");
    if (args.size() >= 8 && !args[7]->isNull()) {
        timeout = args[7];
        if (timeout->getType() != DT_INT || timeout->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, "Timeout must be an integer scalar. ");
    } else
        timeout = new Int(0);
    if (args.size() >= 9 && !args[8]->isNull()) {
        headers = args[8];
        if ((headers->getType() != DT_STRING || headers->getForm() != DF_SCALAR) &&
            (headers->getForm() != DF_DICTIONARY ||
             ((DictionarySP) headers)->getKeyType() != DT_STRING ||
             ((DictionarySP) headers)->getType() != DT_STRING))
            throw IllegalArgumentException(__FUNCTION__,
                                           "Headers must be a string or a dictionary with STRING-STRING key-value type");
    } else
        headers = new String("");
    std::unique_ptr<SubConnection> cup(
            new SubConnection(url, params, parserInterval, timeout, headers, handle, heap, parser,
                              httpClient::GET,
                              cycles, cookiesSet));
    FunctionDefSP onClose(
            Util::createSystemProcedure("http sub connection onClose()",
                                        httpConnectionOnClose<SubConnection>, 1, 1));
    ConstantSP conn = Util::createResource(
            (long long) cup.release(),
            "http subscribe connection",
            onClose,
            heap->currentSession()
    );
    status_dict->set(std::to_string(conn->getLong()), conn);
    LOG_WARN("HttpClientPlugin: start a httpCreateSubJob");
    return conn;
}

ConstantSP httpCreateMutiParserSubJob(Heap *heap, vector<ConstantSP> &args) {
    const auto usage = string(
            "Usage: createmutiThreadSubJob(consumer, table, parser).\n"
    );

    ConstantSP params, timeout, headers, parser, handle, streamHeadParser;
    int threadNum;
    string cookiesSet;
    vector<string> url;
    int parserInterval, minBlockSize, maxBlockeSize, cycles;
    if (args[0]->getType() != DT_STRING ||
        (args[0]->getForm() != DF_VECTOR && args[0]->getForm() != DF_SCALAR)) {
        throw IllegalArgumentException(__FUNCTION__, "Url must be a string scalar or a string vector. ");
    } else if (args[0]->getForm() == DF_VECTOR) {
        int size = args[0]->size();
        if (size == 0) {
            throw IllegalArgumentException(__FUNCTION__,
                                           "If the parameter 'url' is a vector, its length must be greater than 0. ");
        }
        for (int i = 0; i < size; ++i) {
            url.push_back(args[0]->getString(i));
        }
    } else {
        url.push_back(args[0]->getString());
    }
    handle = args[1];
    if (!handle->isTable())
        throw IllegalArgumentException(__FUNCTION__, "Hadle must be a table. ");
    parser = args[2];
    if (parser->getType() != DT_FUNCTIONDEF || parser->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, "Parser must be a function. ");
    streamHeadParser = args[3];
    if (streamHeadParser->getType() != DT_FUNCTIONDEF || streamHeadParser->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, "StreamHeadParser must be a function. ");
    if (!args[4]->isNull()) {
        if (args[4]->getType() != DT_INT || args[4]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, "ThreadNum must be an integer scalar");
        }
        threadNum = args[4]->getInt();
        if (threadNum <= 0)
            throw RuntimeException("ThreadNum must be a positive integer scalar");
    } else
        throw RuntimeException("ThreadNum must be a positive integer scalar");
    if (!args[5]->isNull()) {
        if (args[5]->getType() != DT_INT || args[5]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, "ParserInterval must be an integer scalar");
        }
        parserInterval = args[5]->getInt();
        if (parserInterval <= 0)
            throw RuntimeException("ParserInterval must be a positive integer scalar");
    } else
        throw RuntimeException("ParserInterval must be a positive integer scalar");
    if (!args[6]->isNull()) {
        if (args[6]->getType() != DT_INT || args[6]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, "MinBlockSize must be an integer scalar");
        }
        minBlockSize = args[6]->getInt();
        if (minBlockSize <= 0)
            throw RuntimeException("MinBlockSize must be a positive integer scalar");
    } else
        throw RuntimeException("MinBlockSize must be a positive integer scalar");
    if (!args[7]->isNull()) {
        if (args[7]->getType() != DT_INT || args[7]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, "MaxBlockSize must be an integer scalar");
        }
        maxBlockeSize = args[7]->getInt();
        if (maxBlockeSize <= 0)
            throw RuntimeException("MaxBlockeSize must be a positive integer scalar");
    } else
        throw RuntimeException("MaxBlockeSize must be a positive integer scalar");
    if (args.size() >= 9 && !args[8]->isNull()) {
        if (args[8]->getType() != DT_INT || args[8]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, "Cycles must be an integer. ");
        cycles = args[8]->getInt();
    } else
        cycles = -1;
    if (args.size() >= 10 && !args[9]->isNull()) {
        if (args[9]->getType() != DT_STRING || args[9]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, "CookieSet must be an string scalar. ");
        cookiesSet = args[9]->getString();
    }
    if (args.size() >= 10 && !args[9]->isNull()) {
        params = args[9];
        if ((params->getType() != DT_STRING || params->getForm() != DF_SCALAR) &&
            (params->getForm() != DF_DICTIONARY ||
             ((DictionarySP) params)->getKeyType() != DT_STRING ||
             ((DictionarySP) params)->getType() != DT_STRING))
            throw IllegalArgumentException(__FUNCTION__,
                                           "Params must be a string or a dictionary with STRING-STRING key-value type");
    } else params = new String("");
    if (args.size() >= 11 && !args[10]->isNull()) {
        timeout = args[10];
        if (timeout->getType() != DT_INT || timeout->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, "Timeout must be an integer scalar. ");
    } else
        timeout = new Int(0);
    if (args.size() >= 12 && !args[11]->isNull()) {
        headers = args[11];
        if ((headers->getType() != DT_STRING || headers->getForm() != DF_SCALAR) &&
            (headers->getForm() != DF_DICTIONARY ||
             ((DictionarySP) headers)->getKeyType() != DT_STRING ||
             ((DictionarySP) headers)->getType() != DT_STRING))
            throw IllegalArgumentException(__FUNCTION__,
                                           "Headers must be a string or a dictionary with STRING-STRING key-value type");
    } else
        headers = new String("");
    if(parserInterval < maxBlockeSize){
        throw IllegalArgumentException(__FUNCTION__, "parserInterval must > maxBlockSize");
    }
    std::unique_ptr<SubConnectionMutiThread> cup(new SubConnectionMutiThread(threadNum, url, params, minBlockSize, maxBlockeSize, timeout,
                                        headers, handle, heap, parser, streamHeadParser, threadNum,
                                        httpClient::GET,cycles, cookiesSet,parserInterval));
    FunctionDefSP onClose(Util::createSystemProcedure("http sub connection onClose()",httpConnectionOnClose<SubConnectionMutiThread>, 1, 1));
    ConstantSP conn = Util::createResource((long long) cup.release(),"http subscribe mutithreadParser connection",onClose,heap->currentSession());
    status_dict->set(std::to_string(conn->getLong()), conn);
    LOG_WARN("HttpClientPlugin: start a httpCreateMutiThreadSubJob");
    return conn;
}

ConstantSP httpCancelSubJob(Heap *heap, vector<ConstantSP> &args) {
    std::string usage = "Usage: cancelSubJob(connection or connection ID). ";
    string key;
    ConstantSP conn = nullptr;
    auto handle = args[0];
    switch (handle->getType()) {
        case DT_RESOURCE:
            key = std::to_string(handle->getLong());
            conn = status_dict->getMember(key);
            if (conn->isNothing())
                throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
            break;
        case DT_STRING:
            key = handle->getString();
            conn = status_dict->getMember(key);
            if (conn->isNothing())
                throw IllegalArgumentException(__FUNCTION__, "Invalid connection string.");
            else
                break;
        case DT_LONG:
            key = std::to_string(handle->getLong());
            conn = status_dict->getMember(key);
            if (conn->isNothing())
                throw IllegalArgumentException(__FUNCTION__, "Invalid connection integer.");
            else
                break;
        case DT_INT:
            key = std::to_string(handle->getInt());
            conn = status_dict->getMember(key);
            if (conn->isNothing())
                throw IllegalArgumentException(__FUNCTION__, "Invalid connection integer.");
            else
                break;
        default:
            throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
    }

    if (conn->getString() == "http subscribe connection") {
        SubConnection *sc = (SubConnection *) (conn->getLong());
        bool bRemoved = status_dict->remove(new String(key));
        if (bRemoved && sc != nullptr) {
            HttpSession *http = sc->http_;
            http->data_.needStop_ = true;
            http->isStopped_.acquire();
            sc->cancelThread();
            LOG_WARN("subscription: " + std::to_string(conn->getLong()) + " is stopped. ");
        }
    } else if (conn->getString() == "http subscribe mutithreadParser connection") {
        SubConnectionMutiThread *sc = (SubConnectionMutiThread *) (conn->getLong());
        bool bRemoved = status_dict->remove(new String(key));
        if (bRemoved && sc != nullptr) {
            shared_ptr<HttpSession> http = sc->getHttp();
            http->data_.needStop_ = true;
            http->isStopped_.acquire();
            sc->cancelThread();
            LOG_WARN("subscription: " + std::to_string(conn->getLong()) + " is stopped. ");
        }
    } else {
        throw RuntimeException("handle msut be a http connection");
    }
    return new Void();
}

string creatRunningUrl(vector<string> url) {
    string ret = url[0];
    for (size_t i = 1; i < url.size(); ++i) {
        ret += (", " + url[i]);
    }
    return ret;
}

SubConnection::SubConnection(vector<string> url, ConstantSP params, int parserInterval, ConstantSP timeout,ConstantSP headers, ConstantSP handle, Heap *heap, FunctionDefSP parser,
                             httpClient::RequestMethod method, int cycles, string cookieSet)
        : heap_(heap) {
    session_ = heap->currentSession()->copy();
    http_ = new HttpSession(url[0], "", cookieSet, session_, creatRunningUrl(url));
    runningUrl_ = creatRunningUrl(url);
    connected_ = true;
    createTime_ = Util::getEpochTime();
    session_->setUser(heap->currentSession()->getUser());
    appendTable_ = new AppendTable(heap, method, parser, parserInterval, handle, http_, url, params,
                                   timeout, headers, cycles);
    thread_ = new Thread(appendTable_);
    if (!thread_->isStarted()) {
        thread_->detach();
        thread_->start();
    }
}

SubConnectionMutiThread::SubConnectionMutiThread(int parserNum, vector<string> url, ConstantSP params,
                                                 int minBlockSize, int maxBlockSize, ConstantSP timeout, ConstantSP headers, ConstantSP handle, Heap *heap,
                                                 FunctionDefSP parser, FunctionDefSP streamHeadParser, int threadNum, httpClient::RequestMethod method,
                                                 int cycles, string cookieSet, int parserInterval)
        {
    session_ = heap->currentSession()->copy();
    http_ = make_shared<HttpSession>(url[0], "", cookieSet, session_, creatRunningUrl(url));
    runningUrl_ = creatRunningUrl(url);
    connected_ = true;
    createTime_ = Util::getEpochTime();
    session_->setUser(heap->currentSession()->getUser());
    for (int i = 0; i < parserNum; ++i) {
        shared_ptr<SynchronizedQueue<string>> queue = make_shared<SynchronizedQueue<string>>();
        SmartPointer<MutiThreadParse> parse = new MutiThreadParse(heap, parser, handle, parserInterval, minBlockSize, maxBlockSize, queue,http_);
        ThreadSP thread = new Thread(parse);
        mutiThreadParse_.push_back(parse);
        parserThread_.push_back(thread);
        queueVec_.push_back(queue);
        if (!thread->isStarted()) {
            thread->detach();
            thread->start();
        }
    }
    //cout<<"queueVec_--" + to_string((long long)(queueVec_[0].get()))<<endl;
    shared_ptr<Semaphore> cleanManageBuffer_ = make_shared<Semaphore>(1);
    shared_ptr<atomic<bool>> needCleanMangeBuffer = make_shared<atomic<bool>>(false);
    shared_ptr<SynchronizedQueue<string>> receiveQueue = make_shared<SynchronizedQueue<string>>();
    manageMessage_ = new ManageMessage(heap, streamHeadParser, parserInterval, minBlockSize, maxBlockSize, http_, queueVec_,
                                       threadNum, receiveQueue, cleanManageBuffer_, needCleanMangeBuffer);
    manageThead_ = new Thread(manageMessage_);
    if (!manageThead_->isStarted()) {
        manageThead_->detach();
        manageThead_->start();
    }
    appendMessage_ = new AppendMessage(heap, method, parser, streamHeadParser, parserInterval, minBlockSize,
                                       handle, http_, url, params, timeout, headers,
                                       cycles, queueVec_, threadNum, receiveQueue, cleanManageBuffer_,
                                       needCleanMangeBuffer);
    thread_ = new Thread(appendMessage_);
    if (!thread_->isStarted()) {
        thread_->detach();
        thread_->start();
    }
}

ConstantSP httpGetJobStat(Heap *heap, vector<ConstantSP> &args) {
    int size = status_dict->size();
    ConstantSP connectionIdVec = Util::createVector(DT_STRING, size);
    ConstantSP userVec = Util::createVector(DT_STRING, size);
    ConstantSP runningUrlVec = Util::createVector(DT_STRING, size);
    ConstantSP timestampVec = Util::createVector(DT_TIMESTAMP, size);
    ConstantSP cycles_completedVec = Util::createVector(DT_LONG, size);
    ConstantSP cookiesVec = Util::createVector(DT_STRING, size);
    ConstantSP readByteVec = Util::createVector(DT_LONG, size);
    ConstantSP dataNumberVec = Util::createVector(DT_LONG, size);
    ConstantSP lastparseChunkFailedTimestampVec = Util::createVector(DT_TIMESTAMP, size);
    ConstantSP parseChunkFailedCountVec = Util::createVector(DT_LONG, size);
    ConstantSP parseChunkProcessedCountVec = Util::createVector(DT_LONG, size);
    VectorSP keys = status_dict->keys();
    for (int i = 0; i < keys->size(); i++) {
        string key = keys->getString(i);
        connectionIdVec->setString(i, key);
        ConstantSP conn = status_dict->getMember(key);
        if (conn->getString() == "http subscribe mutithreadParser connection") {
            auto *sc = (SubConnectionMutiThread *) (conn->getLong());
            cycles_completedVec->setLong(i, sc->getCycles_completed());
            runningUrlVec->setString(i, sc->getRunnningUrl());
            timestampVec->setLong(i, sc->getCreateTime());
            userVec->setString(i, sc->getSession()->getUser()->getUserId());
            cookiesVec->setString(i, sc->getHttp()->cookie_.getString());
            readByteVec->setLong(i, sc->getHttp()->readByte_);
            dataNumberVec->setLong(i, sc->getHttp()->dataNumber_);
            lastparseChunkFailedTimestampVec->setLong(i, sc->getHttp()->lastparseChunkFailedTimestamp);
            parseChunkFailedCountVec->setLong(i, sc->getHttp()->prseChunkFailedCount_);
            parseChunkProcessedCountVec->setLong(i, sc->getHttp()->parseChunkProcessedCount_);
        } else {
            auto *sc = (SubConnection *) (conn->getLong());
            cycles_completedVec->setLong(i, sc->getCycles_completed());
            runningUrlVec->setString(i, sc->getRunnningUrl());
            timestampVec->setLong(i, sc->getCreateTime());
            userVec->setString(i, sc->getSession()->getUser()->getUserId());
            cookiesVec->setString(i, sc->http_->cookie_.getString());
            readByteVec->setLong(i, sc->http_->readByte_);
            dataNumberVec->setLong(i, sc->http_->dataNumber_);
            lastparseChunkFailedTimestampVec->setLong(i, sc->http_->lastparseChunkFailedTimestamp);
            parseChunkFailedCountVec->setLong(i, sc->http_->prseChunkFailedCount_);
            parseChunkProcessedCountVec->setLong(i, sc->http_->parseChunkProcessedCount_);
        }
    }

    vector<string> colNames = {"subscriptionId", "user", "runningUrl", "cookie", "cycles_completed",
                               "createTimestamp",
                               "readByte", "dataNumber", "lastparseChunkFailedTimestamp",
                               "failedParseChunkCount",
                               "processedParseChunkCount"};
    vector<ConstantSP> cols = {connectionIdVec, userVec, runningUrlVec, cookiesVec,
                               cycles_completedVec, timestampVec, readByteVec, dataNumberVec,
                               lastparseChunkFailedTimestampVec, parseChunkFailedCountVec, parseChunkProcessedCountVec};
    return Util::createTable(colNames, cols);
}

SubConnectionMutiThread::~SubConnectionMutiThread() {
    for (size_t i = 0; i < parserThread_.size(); ++i) {
        parserThread_[i]->cancel();
    }
    manageThead_->cancel();
    if (connected_) {
        connected_ = false;
    }
}