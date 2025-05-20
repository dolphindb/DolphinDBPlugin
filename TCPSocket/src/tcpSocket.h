#include "DolphinDBEverything.h"
#include "CoreConcept.h"
#include "ScalarImp.h"
#include "SysIO.h"
#include "ddbplugin/ThreadedQueue.h"

static string PLUGIN_TCP_PREFIX = "[PLUGIN::TCPSocket]: ";
#define TCP_BATCH_SIZE 10240

using argsT = std::vector<ddb::ConstantSP>;

extern "C" {

ddb::ConstantSP tcpCreateSubJob(ddb::Heap *heap, argsT &args);
ddb::ConstantSP tcpGetSubJobStat(ddb::Heap *heap, argsT &args);
ddb::ConstantSP tcpCancelSubJob(ddb::Heap *heap, argsT &args);
ddb::ConstantSP tcpConnect(ddb::Heap *heap, argsT &args);
ddb::ConstantSP tcpRead(ddb::Heap *heap, argsT &args);
ddb::ConstantSP tcpWrite(ddb::Heap *heap, argsT &args);
ddb::ConstantSP tcpClose(ddb::Heap *heap, argsT &args);

}
