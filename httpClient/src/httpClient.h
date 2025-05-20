#include "DolphinDBEverything.h"
#include <CoreConcept.h>
#include <curl/curl.h>
#include <openssl/ssl.h>

using argsT = std::vector<ddb::ConstantSP>;

extern "C" ddb::ConstantSP httpGet(ddb::Heap *heap, argsT &args);
extern "C" ddb::ConstantSP httpPost(ddb::Heap *heap, argsT &args);
extern "C" ddb::ConstantSP httpPut(ddb::Heap *heap, argsT &args);
extern "C" ddb::ConstantSP httpDelete(ddb::Heap *heap, argsT &args);

namespace ddb {
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
} // namespace ddb
