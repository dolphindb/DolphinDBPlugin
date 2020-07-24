#include <CoreConcept.h>

#ifdef WINDOWS
    #define EXPORT_DLL __declspec(dllexport)
#else
    #define EXPORT_DLL
#endif

extern "C" ConstantSP EXPORT_DLL httpGet(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP EXPORT_DLL httpPost(Heap *heap, vector<ConstantSP> &args);

namespace httpClient {
enum RequestMethod { GET, POST };
ConstantSP httpRequest(RequestMethod method, const ConstantSP &url,
                       const ConstantSP &params, const ConstantSP &timeout,
                       const ConstantSP &nobody);
}
