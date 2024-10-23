#include "CoreConcept.h"
#include "ScalarImp.h"
#include "ddbplugin/ThreadedQueue.h"

static string PLUGIN_TCP_PREFIX = "[PLUGIN::TCPSocket]: ";
#define TCP_BATCH_SIZE 10240

extern "C" ConstantSP tcpCreateSubJob(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP tcpGetSubJobStat(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP tcpCancelSubJob(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP tcpConnect(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP tcpRead(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP tcpWrite(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP tcpClose(Heap *heap, vector<ConstantSP> &args);

