
#include <Exceptions.h>
#include <ScalarImp.h>
#include <Util.h>
#include <curl/curl.h>
#include <openssl/ssl.h>
#include <string>

#include"session.h"

using namespace std;
DictionarySP status_dict = Util::createDictionary(DT_STRING, nullptr, DT_ANY, nullptr);

size_t parserWriteData(void *ptr, size_t size, size_t nmemb, WriteBase *paraMdata) {

    size_t writeSize = size * nmemb;
    paraMdata->buffer_.append((char *) ptr, writeSize);
    long long start = 0;
    size_t end = 0;
    if (paraMdata->flag_ == HttpData) {
        WriteData *data = (WriteData *) paraMdata;
        if (*(data->needParse_) == true && !data->parser_.isNull()) {
            data->http_->readByte_ += writeSize;
            size_t mIndex = data->parserInterval_ == 0 ? 10240 : data->parserInterval_;
            Session *session = data->session_.get();
            if (session == nullptr)
                return -1;
            Heap *heap = session->getHeap().get();
            while ((data->buffer_.size() - end) > mIndex) {
                end += mIndex;
                try {
                    if (data->needStop_)
                        return -1;
                    vector<ConstantSP> args;
                    string paramString = data->buffer_.substr(start, end - start);
                    VectorSP paramSp = Util::createVector(DT_CHAR, paramString.size(), paramString.size());
                    paramSp->setChar(0, paramString.size(), paramString.c_str());
                    args.push_back(paramSp);
                    ConstantSP parser_result;
                    ConstantSP parser_data;
                    parser_result = data->parser_->call(heap, args);
                    if (parser_result->isTable()) {
                        parser_data = parser_result;
                    } else if (parser_result->isDictionary()) {
                        DictionarySP tmp = (DictionarySP) parser_result;
                        start -= mIndex;
                        ConstantSP digital = tmp->getMember("size");
                        if (digital->isNull() || !(digital->getType() != DT_INT || digital->getType() != DT_LONG) ||
                            digital->getForm() != DF_SCALAR) {
                            LOG_ERR("HttpClientPlugin: The parser must return a dictionary with STRING type keys and ANY type values. One of the keys must be \"size\" and the corresponding value must be a LONG or INT type scalar. ");
                        } else {
                            if (digital->getType() == DT_INT)
                                start += digital->getInt();
                            if (digital->getType() == DT_LONG)
                                start += digital->getLong();
                        }
                        ConstantSP parseChunkProcessedCount = tmp->getMember("parseChunkProcessedCount");
                        if (parseChunkProcessedCount->isNull() || !(parseChunkProcessedCount->getType() != DT_INT ||
                                                                    parseChunkProcessedCount->getType() != DT_LONG) ||
                            parseChunkProcessedCount->getForm() != DF_SCALAR) {
                            data->http_->lastparseChunkFailedTimestamp = Util::getEpochTime();
                        } else {
                            if (parseChunkProcessedCount->getType() == DT_INT)
                                data->http_->parseChunkProcessedCount_ += parseChunkProcessedCount->getInt();
                            if (parseChunkProcessedCount->getType() == DT_LONG)
                                data->http_->parseChunkProcessedCount_ += parseChunkProcessedCount->getLong();
                        }
                        parser_data = tmp->getMember("data");
                        if (parser_data->isNull() || !parser_data->isTable()) {
                            LOG_ERR("HttpClientPlugin: The parser must return a dictionary with STRING type keys and ANY type values. One of the keys must be \"data\" and the corresponding value must be a table. ");
                            //string tmp=data->buffer_.substr(start, end - start);
                            LOG_ERR(to_string(start) + " " + to_string(end) + "\n");
                            //while parse mseed fail
                            if (digital->getType() == DT_INT && digital->getForm() == DF_SCALAR)
                                LOG_ERR(to_string(digital->getInt()) + "\n");
                            if (digital->getType() == DT_LONG && digital->getForm() == DF_SCALAR)
                                LOG_ERR(to_string(digital->getLong()) + "\n");
                        }
                    } else {
                        LOG_ERR("HttpClientPlugin: The parser must return a dictionary with STRING type keys and ANY type values or a table. ");
                    }
                    if (!parser_data.isNull() && parser_data->isTable()) {
                        data->http_->dataNumber_ += parser_data->size();
                        if (data->handle_->isTable()) {
                            TableSP result = data->handle_;

                            TableSP table_insert = (TableSP) parser_data;
                            int length = table_insert->columns();
                            if (table_insert->columns() < length) {
                                LOG_ERR("HttpClientPlugin: The columns of the table returned is smaller than the handler table.");
                            }
                            if (table_insert->columns() > length)
                                LOG_ERR("HttpClientPlugin: The columns of the table returned is larger than the handler table, and the information may be ignored.");
                            vector<ConstantSP> args = {result, table_insert};
                            session->getFunctionDef("append!")->call(heap, args);
                        } else {
                            vector<ConstantSP> args = {parser_result};
                            ((FunctionDefSP) data->handle_)->call(heap, args);
                        }
                    } else {
                        data->http_->prseChunkFailedCount_++;
                    }
                }
                catch (exception &e) {
                    LOG_ERR(string("HttpClientPlugin: ") + e.what() + "\n bufferStartIndex:" + std::to_string(start) +
                            " bufferEndIndex: " + std::to_string(end) + "\n");
                }
                start += mIndex;
            }
            if (data->buffer_.size() - start == 0)
                data->buffer_ = "";
            else
                data->buffer_ = data->buffer_.substr(start, data->buffer_.size() - start);
        }
    } else if (paraMdata->flag_ == HttpHead && *(paraMdata->needParse_) == false) {
        WriteHead *data = (WriteHead *) paraMdata;
        long long index = data->startIndex;
        while (index < data->buffer_.size()) {
            index = data->startIndex;
            while (index < data->buffer_.size()) {
                if (data->buffer_[index] == '\n')
                    break;
                ++index;
            }
            if (index < data->buffer_.size()) {
                string tmp = data->buffer_.substr(data->startIndex, index - data->startIndex + 1);
                if (tmp == "Content-type: application/octet-stream\r\n" ||
                    tmp == "Content-Type: application/octet-stream\r\n") {
                    *(data->needParse_) = true;
                }
                data->startIndex = index + 1;
            }
        }
    }
    if (paraMdata->flag_ == HttpData && ((WriteData *) paraMdata)->needStop_)
        return -1;
    return writeSize;
}

std::string findCookie(std::string headers) {
    int pos = -1;
    pos = headers.find("Set-Cookie");
    if (-1 != pos) {
        int pos2 = headers.find('\n', pos);
        if (-1 != pos2) {
            if (headers[pos + 1])
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


ConstantSP
HttpSession::httpRequest(SessionSP &session, httpClient::RequestMethod method, const ConstantSP &handle,
                         const std::string &url, const ConstantSP &params, const ConstantSP &timeout,
                         const ConstantSP &headers, const FunctionDefSP &parser, int parserInterval) {
    if (session.isNull())
        return new Bool(false);
    ConstantSP res = Util::createDictionary(DT_STRING, nullptr, DT_ANY, nullptr);
    std::string urlString = url;
    CURL *curl = (curl_ == NULL) ? curl_easy_init() : curl_;
    curl_ = curl;
    if (curl) {
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
        data_ = WriteData(HttpData, parserInterval, handle, parser, session, data_.needStop_, this, &needParse);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &head_);
        if (parser.isNull()) {
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, httpClient::curlWriteData);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data_);
        } else {
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, parserWriteData);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data_);
        }

        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
        //curl_easy_setopt(curl, CURLOPT_TIMEOUT, 150000);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 50);

        if (params->getForm() == DF_DICTIONARY)
            paramString = httpClient::getParamString(params);
        else
            paramString = params->getString();

        CURLcode ret = curl_easy_perform(curl);
        if (findCookie(head_.buffer_) != "") {
            setCookie(findCookie(head_.buffer_), session);
        }
        if (data_.needStop_) {
            return new Void();
        }
        if (!parser.isNull() && needParse && data_.buffer_.size() > 0) {
            try {
                std::vector<ConstantSP> args;
                args.push_back(new String(data_.buffer_));
                ConstantSP parser_result;
                ConstantSP parser_data;
                Heap *heap = session->getHeap().get();
                parser_result = data_.parser_->call(heap, args);
                if (parser_result->isTable()) {
                    parser_data = parser_result;
                } else if (parser_result->isDictionary()) {
                    DictionarySP tmp = (DictionarySP) parser_result;
                    ConstantSP parseChunkProcessedCount = tmp->getMember("parseChunkProcessedCount");
                    if (parseChunkProcessedCount->isNull() || !(parseChunkProcessedCount->getType() != DT_INT ||
                                                                parseChunkProcessedCount->getType() != DT_LONG) ||
                        parseChunkProcessedCount->getForm() != DF_SCALAR) {
                        data_.http_->lastparseChunkFailedTimestamp = Util::getEpochTime();
                    } else {
                        if (parseChunkProcessedCount->getType() == DT_INT)
                            data_.http_->parseChunkProcessedCount_ += parseChunkProcessedCount->getInt();
                        if (parseChunkProcessedCount->getType() == DT_LONG)
                            data_.http_->parseChunkProcessedCount_ += parseChunkProcessedCount->getLong();
                    }
                    parser_data = tmp->getMember("data");
                    if (parser_data->isNull() || !parser_data->isTable()) {
                        LOG_ERR("HttpClientPlugin: The parser must return a dictionary with STRING type keys and ANY type values. One of the keys must be \"data\" and the corresponding value must be a table. ");
                    }
                } else {
                    LOG_ERR("HttpClientPlugin: The parser must return a dictionary with STRING type keys and ANY type values or a table. ");
                }
                if (!parser_data.isNull() && parser_data->isTable()) {
                    dataNumber_ += parser_data->size();
                    if (data_.handle_->isTable()) {
                        TableSP result = data_.handle_;

                        TableSP table_insert = (TableSP) parser_data;
                        int length = table_insert->columns();
                        if (table_insert->columns() < length) {
                            LOG_ERR("HttpClientPlugin: The columns of the table returned is smaller than the handler table.");
                        }
                        if (table_insert->columns() > length)
                            LOG_ERR("HttpClientPlugin: The columns of the table returned is larger than the handler table, and the information may be ignored.");
                        vector<ConstantSP> args = {result, table_insert};
                        session->getFunctionDef("append!")->call(heap, args);
                    } else {
                        vector<ConstantSP> args = {parser_result};
                        ((FunctionDefSP) data_.handle_)->call(heap, args);
                    }
                }
            }
            catch (exception &e) {
                LOG_ERR(string("HttpClientPlugin: ") + e.what());
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
    } else {
        throw RuntimeException("Could not initialize request object");
    }
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
                http_->httpRequest(session_, httpClient::GET, handle_, url, params_, timeout_, headers_, parser_,
                                   parserInterval_);
            }
            catch (exception &e) {
                LOG_ERR(string("HttpClientPlugin: ") + e.what());
            }
            stopFlag = http_->data_.needStop_ || stopFlag;
            if (stopFlag)
                break;
        }
        stopFlag = http_->data_.needStop_ || stopFlag;
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

ConstantSP httpCreateSubJob(Heap *heap, vector<ConstantSP> args) {
    const auto usage = string(
            "Usage: createSubJob(consumer, table, parser).\n"
    );

    ConstantSP params, timeout, headers, parser, handle;
    string cookiesSet;
    vector<string> url;
    int parserInterval, cycles;
    if (args[0]->getType() != DT_STRING || (args[0]->getForm() != DF_VECTOR && args[0]->getForm() != DF_SCALAR)) {
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
    if (args.size() >= 4 && !args[3]->isNull()) {
        if (args[3]->getType() != DT_INT || args[3]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, "ParserInterval must be an integer scalar");
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
            new SubConnection(url, params, parserInterval, timeout, headers, handle, heap, parser, httpClient::GET,
                              cycles, cookiesSet));
    FunctionDefSP onClose(
            Util::createSystemProcedure("http sub connection onClose()", httpConnectionOnClose<SubConnection>, 1, 1));
    ConstantSP conn = Util::createResource(
            (long long) cup.release(),
            "http subscribe connection",
            onClose,
            heap->currentSession()
    );
    status_dict->set(std::to_string(conn->getLong()), conn);
    return conn;
}

ConstantSP httpCancelSubJob(Heap *heap, vector<ConstantSP> args) {
    std::string usage = "Usage: cancelSubJob(connection or connection ID). ";
    SubConnection *sc = nullptr;
    string key;
    ConstantSP conn = nullptr;
    auto handle = args[0];
    switch (handle->getType()) {
        case DT_RESOURCE:
            sc = (SubConnection *) (handle->getLong());
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
                sc = (SubConnection *) (conn->getLong());
            break;
        case DT_LONG:
            key = std::to_string(handle->getLong());
            conn = status_dict->getMember(key);
            if (conn->isNothing())
                throw IllegalArgumentException(__FUNCTION__, "Invalid connection integer.");
            else
                sc = (SubConnection *) (conn->getLong());
            break;
        case DT_INT:
            key = std::to_string(handle->getInt());
            conn = status_dict->getMember(key);
            if (conn->isNothing())
                throw IllegalArgumentException(__FUNCTION__, "Invalid connection integer.");
            else
                sc = (SubConnection *) (conn->getLong());
            break;
        default:
            throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
    }
    bool bRemoved = status_dict->remove(new String(key));
    if (bRemoved && sc != nullptr) {
        sc->http_->data_.needStop_ = true;
        sc->http_->isStopped_.acquire();
        sc->cancelThread();
        LOG_INFO("subscription: " + std::to_string(conn->getLong()) + " is stopped. ");
    }
    return new Void();
}

SubConnection::SubConnection(vector<string> url, ConstantSP params, int parserInterval, ConstantSP timeout,
                             ConstantSP headers, ConstantSP handle, Heap *heap, FunctionDefSP parser,
                             httpClient::RequestMethod method, int cycles, string cookieSet)
        : heap_(heap) {
    session_ = heap->currentSession()->copy();
    http_ = new HttpSession(url[0], "", cookieSet, session_);
    runningUrl_ = url[0];
    for (size_t i = 1; i < url.size(); ++i) {
        runningUrl_ += (", " + url[i]);
    }
    connected_ = true;
    createTime_ = Util::getEpochTime();
    session_->setUser(heap->currentSession()->getUser());
    appendTable_ = new AppendTable(heap, method, parser, parserInterval, handle, http_, url, params, timeout, headers,
                                   cycles);
    thread_ = new Thread(appendTable_);
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
        auto *sc = (SubConnection *) (conn->getLong());
        cycles_completedVec->setLong(i, sc->getCycles_completed());
        runningUrlVec->setString(i, sc->getRunnningUrl());
        timestampVec->setLong(i, sc->getCreateTime());
        userVec->setString(i, sc->getSession()->getUser()->getUserId());
        cookiesVec->setString(i, sc->http_->cookie_.getString());
        readByteVec->setLong(i, sc->http_->readByte_);
        dataNumberVec->setLong(i, sc->http_->dataNumber_);
        lastparseChunkFailedTimestampVec->setLong(i, sc->http_->lastparseChunkFailedTimestamp.getLong());
        parseChunkFailedCountVec->setLong(i, sc->http_->prseChunkFailedCount_);
        parseChunkProcessedCountVec->setLong(i, sc->http_->parseChunkProcessedCount_);
    }

    vector<string> colNames = {"subscriptionId", "user", "runningUrl", "cookie", "cycles_completed", "createTimestamp",
                               "readByte", "dataNumber", "lastparseChunkFailedTimestamp", "failedParseChunkCount",
                               "processedParseChunkCount"};
    vector<ConstantSP> cols = {connectionIdVec, userVec, runningUrlVec, cookiesVec, cycles_completedVec, timestampVec,
                               readByteVec, dataNumberVec, lastparseChunkFailedTimestampVec, parseChunkFailedCountVec,
                               parseChunkProcessedCountVec};
    return Util::createTable(colNames, cols);
}