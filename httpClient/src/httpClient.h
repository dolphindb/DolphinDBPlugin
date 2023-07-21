#include <CoreConcept.h>
#include <curl/curl.h>
#include <openssl/ssl.h>

extern "C" ConstantSP httpGet(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP httpPost(Heap *heap, vector<ConstantSP> &args);

namespace httpClient {
    enum RequestMethod {
        GET, POST
    };

    size_t curlWriteData(void *ptr, size_t size, size_t nmemb, string *data);

    void getParamString(const DictionarySP &params, string& output);

    curl_slist *setHeaders(const DictionarySP &headers, curl_slist *slist);

    ConstantSP httpRequest(RequestMethod method,
                           const ConstantSP &url, const ConstantSP &params, const ConstantSP &timeout,
                           const ConstantSP &headers);
}