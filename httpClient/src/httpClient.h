#include <CoreConcept.h>

extern "C" ConstantSP httpGet(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP httpPost(Heap *heap, vector<ConstantSP> &args);

namespace httpClient {
enum RequestMethod { GET, POST };
ConstantSP httpRequest(RequestMethod method,
                       const ConstantSP &url, const ConstantSP &params, const ConstantSP &timeout);
}