/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2015, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/
/* <DESC>
 * one way to set the necessary OpenSSL locking callbacks if you want to do
 * multi-threaded transfers with HTTPS/FTPS with libcurl built to use OpenSSL.
 * </DESC>
 */
/*
 * This is not a complete stand-alone example.
 *
 * Author: Jeremy Brown
 */

#include "httpClient.h"
#include "ddbplugin/CommonInterface.h"
#include "ddbplugin/PluginLoggerImp.h"
#include <stdio.h>
#include <pthread.h>
#include <openssl/err.h>
#include <vector>
#include <Exceptions.h>
#include <ScalarImp.h>
#include <Util.h>
#include <curl/curl.h>
#include <openssl/ssl.h>
#include <string>
#include<urlencode.h>
#include <mutex>
using namespace std;

namespace ddb {

void checkDictionaryContent(DictionarySP params){
    if(params->getForm() != DF_DICTIONARY){
        return;
    }
    ConstantSP keys = params->keys();
    for (int i = 0; i < keys->size(); i++) {
        ConstantSP key = keys->get(i);
        if(key->getString().empty() || params->getMember(key)->getString().empty()){
            throw RuntimeException("the key or value in dictionary can not be emtpy");
        }
    }
}

ddb::HttpRequestConfig getHttpRequestConfig(DictionarySP params){
    ddb::HttpRequestConfig config;
    //proxy
    ConstantSP proxy = params->getMember("proxy");
    if(!proxy->isNull()){
        if(proxy->getType() != DT_STRING || proxy->getForm() != DF_SCALAR){
            throw IllegalArgumentException(__FUNCTION__, "config proxy must be a string scalar");
        }
        config.proxy_ = proxy->getString();
    }
    //proxy_username
    ConstantSP proxy_username = params->getMember("proxy_username");
    if(!proxy_username->isNull()){
        if(proxy_username->getType() != DT_STRING || proxy_username->getForm() != DF_SCALAR){
            throw IllegalArgumentException(__FUNCTION__, "config proxy_username must be a string scalar");
        }
        config.proxyUsername_ = proxy_username->getString();
    }
    //proxy_password
    ConstantSP proxy_password = params->getMember("proxy_password");
    if(!proxy_password->isNull()){
        if(proxy_password->getType() != DT_STRING || proxy_password->getForm() != DF_SCALAR){
            throw IllegalArgumentException(__FUNCTION__, "config proxy_password must be a string scalar");
        }
        config.proxyPassword_ = proxy_password->getString();
    }
    return config;
}

ConstantSP handleHttpRequest(vector<ConstantSP> &args, ddb::RequestMethod method){
    ConstantSP url = args[0];
    ConstantSP params, timeout, headers;

    if (url->getType() != DT_STRING || url->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, "Url must be a string scalar");
    if (args.size() >= 2 && !args[1]->isNull()) {
        params = args[1];
        if ((params->getType() != DT_STRING || params->getForm() != DF_SCALAR) &&
            (params->getForm() != DF_DICTIONARY || ((DictionarySP) params)->getKeyType() != DT_STRING))
            throw IllegalArgumentException(__FUNCTION__, "Params must be a string scalar or a dictionary with STRING key");
        if(params->getForm() == DF_DICTIONARY){
            checkDictionaryContent(params);
        }
    } else
        params = new String("");
    if (args.size() >= 3) {
        timeout = args[2];
        if (timeout.isNull() || !timeout->isNumber() || timeout->getForm() != DF_SCALAR || timeout->getLong() < 0)
            throw IllegalArgumentException(__FUNCTION__, "Timeout must be an nonnegative integer scalar");
    } else
        timeout = new Long(0);
    if (args.size() >= 4 && !args[3]->isNull()) {
        headers = args[3];
        if ((headers->getType() != DT_STRING || headers->getForm() != DF_SCALAR) &&
            (headers->getForm() != DF_DICTIONARY ||
             ((DictionarySP) headers)->getKeyType() != DT_STRING ||
             ((DictionarySP) headers)->getType() != DT_STRING))
            throw IllegalArgumentException(__FUNCTION__, "Headers must be a string or a dictionary with STRING-STRING key-value type");
        if(headers->getForm() == DF_DICTIONARY){
            checkDictionaryContent(headers);
        }
    } else
        headers = new String("");

    ddb::HttpRequestConfig config;
    if(args.size() >= 5 && !args[4]->isNull()){
        if (args[4]->getForm() != DF_DICTIONARY) {
            throw IllegalArgumentException(__FUNCTION__, "config must be a dictionary");
        }
        DictionarySP dic = args[4];
        if(dic->getKeyType() != DT_STRING){
            throw IllegalArgumentException(__FUNCTION__, "config must be a dictionary whose key type is string");
        }
        config = getHttpRequestConfig(dic);
    }
    return ddb::httpRequest(method, url, params, timeout, headers, config);
}

    /* This array will store all of the mutexes available to OpenSSL. */
    vector<Mutex> mutex_buf;
#if OPENSSL_VERSION_NUMBER < 0x30000000L
    static void locking_function(int mode, int n, const char *file, int line)
    {
        if (mode & CRYPTO_LOCK)
            mutex_buf[n].lock();
        else
            mutex_buf[n].unlock();
    }

    static unsigned long id_function(void)
    {
        return ((unsigned long)pthread_self());
    }
#endif
    int thread_setup(void)
    {
        int i;
        int nums = CRYPTO_num_locks();
        for (i = 0; i < nums; i++)
            mutex_buf.emplace_back(Mutex());
        CRYPTO_set_id_callback(id_function);
        CRYPTO_set_locking_callback(locking_function);
        return 1;
    }

    class Init
    {
    public:
        Init()
        {
            curl_global_init(CURL_GLOBAL_ALL);
            thread_setup();
        }
    };

    Init init;

    size_t curlWriteData(void *ptr, size_t size, size_t nmemb, string *data)
    {
        data->append((char *)ptr, size * nmemb);
        return size * nmemb;
    }

    void getParamString(const DictionarySP &params, string& output) {
        output.clear();
        ConstantSP keys = params->keys();
        for (int i = 0; i < keys->size(); i++) {
            if (i != 0)
                output += "&";
            ConstantSP key = keys->get(i);
            ConstantSP value = params->getMember(key);
            output += key->getString();
            output += "=";
            output += (urlencode::EncodeString(value->getString()));
        }
    }

    curl_slist *setHeaders(const DictionarySP &headers, curl_slist *slist) {
        ConstantSP keys = headers->keys();
        for (int i = 0; i < keys->size(); i++) {
            string strHeader;
            ConstantSP key = keys->get(i);
            ConstantSP value = headers->getMember(key);
            strHeader += key->getString();
            strHeader += ':';
            strHeader += value->getString();
            slist = curl_slist_append(slist, strHeader.c_str());
        }
        return slist;
    }

    size_t lengthRemained;
    static size_t read_cb(char *ptr, size_t size, size_t nmemb, void *userdata)
    {
        const char* data = reinterpret_cast<const char*>(userdata);
        size_t count = std::min(size * nmemb, lengthRemained);
        memcpy(ptr, data, count);
        lengthRemained -= count;
        return count;
    }

    ConstantSP
    httpRequest(RequestMethod method, const ConstantSP &url, const ConstantSP &params, const ConstantSP &timeout,
                const ConstantSP &headers, const HttpRequestConfig& config) {
        ConstantSP res = Util::createDictionary(DT_STRING, nullptr, DT_ANY, nullptr);
        string urlString = url->getString();
        CURL *curl = curl_easy_init();
        if (curl) {
            char errorBuf[CURL_ERROR_SIZE];
            string paramString;

            if (params->getForm() == DF_DICTIONARY)
                getParamString(params, paramString);
            else
                paramString = params->getString();

            curl_slist *headerList = NULL;
            switch (method) {
                case GET:
                    if (!paramString.empty()) {
                        urlString += "?";
                        urlString += paramString;
                    }
                    break;
                case POST:
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, paramString.c_str());
                    break;
                case PUT:
                    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_cb);
                    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
                    curl_easy_setopt(curl, CURLOPT_READDATA, paramString.c_str());
                    lengthRemained = paramString.size();
                    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(paramString.size()));
                    headerList = curl_slist_append(headerList, "Expect:");
                    break;
                case DELETE_:
                    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
                    break;
                default:
                    curl_easy_cleanup(curl);
                    throw RuntimeException("Unknown request method");
            }

            if (headers->getForm() == DF_DICTIONARY)
                headerList = setHeaders(headers, headerList);
            else {
                string str_headers = headers->getString();
                if (!str_headers.empty()) {
                    int pos = str_headers.find_first_of(':');
                    int last = str_headers.find_last_of(':');
                    if (pos != last || pos == -1 || pos == 0 || pos == (int) str_headers.size() - 1) {
                        curl_easy_cleanup(curl);
                        curl_slist_free_all(headerList);
                        throw IllegalArgumentException(__FUNCTION__, "If the parameter 'header' is a STRING, it must be in the format of XXX:XXX. ");
                    }
                }
                headerList = curl_slist_append(headerList, str_headers.c_str());
            }
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);

            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_URL, urlString.c_str());
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.47.0");
            curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
            curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
            curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuf);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout->getLong());

            curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

            if(!config.proxy_.empty()){
                curl_easy_setopt(curl, CURLOPT_PROXY, config.proxy_.c_str());
                if(!config.proxyUsername_.empty()){
                    curl_easy_setopt(curl, CURLOPT_PROXYUSERNAME, config.proxyUsername_.c_str());
                }
                if(!config.proxyPassword_.empty()){
                    curl_easy_setopt(curl, CURLOPT_PROXYPASSWORD, config.proxyPassword_.c_str());
                }
            }

            string responseString;
            string headerString;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteData);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerString);

            CURLcode ret = curl_easy_perform(curl);
            if (ret != 0) {
                string errorMsg(errorBuf);
                curl_easy_cleanup(curl);
                curl_slist_free_all(headerList);
                throw RuntimeException("curl returns: " + errorMsg);
            }

            long responseCode;
            double elapsed;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
            curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &elapsed);

            res->set(new String("text"), new String(responseString));
            res->set(new String("elapsed"), new Double(elapsed));
            res->set(new String("headers"), new String(headerString));
            res->set(new String("responseCode"), new Int(responseCode));
            curl_slist_free_all(headerList);
            curl_easy_cleanup(curl);
        } else {
            throw RuntimeException("Could not initialize request object");
        }
        return res;
    }
} // namespace ddb

ddb::ConstantSP httpGet(ddb::Heap *heap, argsT &args)
{
    std::ignore = heap;
    return handleHttpRequest(args, ddb::GET);
}

ddb::ConstantSP httpPost(ddb::Heap *heap, argsT &args)
{
    std::ignore = heap;
    return handleHttpRequest(args, ddb::POST);
}

ddb::ConstantSP httpPut(ddb::Heap *heap, argsT &args)
{
    std::ignore = heap;
    static std::mutex mtx;
    std::unique_lock<std::mutex> lk(mtx);
    return handleHttpRequest(args, ddb::PUT);
}

ddb::ConstantSP httpDelete(ddb::Heap *heap, argsT &args)
{
    std::ignore = heap;
    return handleHttpRequest(args, ddb::DELETE_);
}
