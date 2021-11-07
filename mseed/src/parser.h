#include "CoreConcept.h"
#include "unpack.h"

extern "C" ConstantSP mseedParse(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP mseedParseStream(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP mseedParseStreamInfo(Heap *heap, vector<ConstantSP> &args);
extern "C" void ms_rloginit(void (*log_print)(const char *), const char *logprefix,
                            void (*diag_print)(const char *), const char *errprefix,
                            int maxmessages);

#define MAX_STACK_BUFFER_SIZE 10240 * 100