#include <CoreConcept.h>
#include <curl/curl.h>
#include <openssl/ssl.h>
#include "ddbplugin/CommonInterface.h"
extern "C" ConstantSP httpGet(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP httpPost(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP httpPut(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP httpDelete(Heap *heap, vector<ConstantSP> &args);

namespace httpClient {
    enum RequestMethod {
        GET, POST, PUT, DELETE_
    };

    struct HttpRequestConfig{
        std::string proxy_;
        std::string proxyUsername_;
        std::string proxyPassword_;
    };

    size_t curlWriteData(void *ptr, size_t size, size_t nmemb, string *data);

    void getParamString(const DictionarySP &params, string& output);

    curl_slist *setHeaders(const DictionarySP &headers, curl_slist *slist);

    ConstantSP httpRequest(RequestMethod method,
                           const ConstantSP &url, const ConstantSP &params, const ConstantSP &timeout,
                           const ConstantSP &headers, const HttpRequestConfig& config);
}