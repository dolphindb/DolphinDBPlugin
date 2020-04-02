#include <Exceptions.h>
#include <ScalarImp.h>
#include <Util.h>
#include "httpClient.h"
#include <curl/curl.h>
#include <openssl/ssl.h>
#include <string>

ConstantSP httpGet(Heap *heap, vector<ConstantSP> &args) {
    ConstantSP url = args[0];
    ConstantSP params, timeout;

    if (url->getType() != DT_STRING || url->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, "url must be a string scalar");
    if (args.size() >= 2 && !args[1]->isNull()) {
        params = args[1];
        if ((params->getType() != DT_STRING || params->getForm() != DF_SCALAR) &&
            (params->getForm() != DF_DICTIONARY ||
            ((DictionarySP)params)->getKeyType() != DT_STRING ||
            ((DictionarySP)params)->getType() != DT_STRING))
            throw IllegalArgumentException(__FUNCTION__, "params must be a string or a dictionary with STRING-STRING key-value type");
    }
    else
        params = new String("");
    if (args.size() >= 3) {
        timeout = args[2];
        if (!timeout->isNumber() || timeout->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, "timeout must be an integer");
    }
    else
        timeout = new Int(0);

    return httpClient::httpRequest(httpClient::GET, url, params, timeout);
}

ConstantSP httpPost(Heap *heap, vector<ConstantSP> &args) {
    ConstantSP url = args[0];
    ConstantSP params, timeout;

    if (url->getType() != DT_STRING || url->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, "url must be a string scalar");
    if (args.size() >= 2 && !args[1]->isNull()) {
        params = args[1];
        if (params->getType() != DT_STRING && (params->getForm() != DF_DICTIONARY || ((DictionarySP)params)->getKeyType() != DT_STRING))
            throw IllegalArgumentException(__FUNCTION__, "params must be a string or a dictionary with keys of type string");
    }
    else
        params = new String("");
    if (args.size() >= 3) {
        timeout = args[2];
        if (!timeout->isNumber() || timeout->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, "timeout must be an integer");
    }
    else
        timeout = new Long(0);

    return httpClient::httpRequest(httpClient::POST, url, params, timeout);
}

namespace httpClient {

size_t curlWriteData(void *ptr, size_t size, size_t nmemb, string *data) {
    data->append((char *)ptr, size * nmemb);
    return size * nmemb;
}

const string getParamString(const DictionarySP &params) {
    string paramString;
    ConstantSP keys = params->keys();
    for (int i = 0; i < keys->size(); i++) {
        if (i != 0)
            paramString += "&";
        ConstantSP key = keys->get(i);
        ConstantSP value = params->getMember(key);
        paramString += key->getString();
        paramString += "=";
        paramString += value->getString();
    }
    return paramString;
}

ConstantSP httpRequest(RequestMethod method, const ConstantSP &url, const ConstantSP &params, const ConstantSP &timeout) {
    ConstantSP res = Util::createDictionary(DT_STRING, nullptr, DT_ANY, nullptr);
    string urlString = url->getString();

    SSL_library_init();
    CURL *curl = curl_easy_init();
    if (curl) {
        char errorBuf[CURL_ERROR_SIZE];
        string paramString;

        if (params->getForm() == DF_DICTIONARY)
            paramString = getParamString(params);
        else
            paramString = params->getString();

        if (!paramString.empty()) {
            switch (method) {
            case GET:
                urlString += "?";
                urlString += paramString;
                break;
            case POST:
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, paramString.c_str());
                break;
            default:
                throw RuntimeException("Unknown request method");
            }
        }
        curl_easy_setopt(curl, CURLOPT_URL, urlString.c_str());
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.44.0");
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuf);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout->getLong());

        string responseString;
        string headerString;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerString);

        CURLcode ret = curl_easy_perform(curl);
        if (ret != 0) {
            string errorMsg(errorBuf);
            curl_easy_cleanup(curl);
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

        curl_easy_cleanup(curl);
    }
    else {
        throw RuntimeException("Could not initialize request object");
    }
    return res;
}
}