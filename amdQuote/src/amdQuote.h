#ifndef __AMD_QUOTE_H
#define __AMD_QUOTE_H

#include <exception>
#include <mutex>
#include <ostream>
#include <string>
#include <unordered_map>
#include <iostream>

#include "Exceptions.h"
#include "FlatHashmap.h"
#include "ama.h"
#include "ama_tools.h"

#include "CoreConcept.h"
#include "amdQuoteType.h"
#include "Logger.h"
#include "Util.h"
#include "ScalarImp.h"
#include "Plugin.h"


// 根据参数构造AmdQuote对象，并返回对象，后续接口第一个参数需要传这个对象
extern "C" ConstantSP amdConnect(Heap *heap, vector<ConstantSP> &arguments);
// 根据传入行情数据类型订阅行情数据，返回空
extern "C" ConstantSP subscribe(Heap *heap, vector<ConstantSP> &arguments);
// 取消订阅行情数据，返回空
extern "C" ConstantSP unsubscribe(Heap *heap, vector<ConstantSP> &arguments);
// 手动释放资源
extern "C" ConstantSP amdClose(Heap *heap, vector<ConstantSP> &arguments);
// 根据传入行情数据类型, 返回对应的表结构
extern "C" ConstantSP getSchema(Heap *heap, vector<ConstantSP> &arguments);
// 运维函数，获取当前连接状态
extern "C" ConstantSP getStatus(Heap *heap, vector<ConstantSP> &arguments);
// 打开延迟统计功能
extern "C" ConstantSP enableLatencyStatistics(Heap *heap, vector<ConstantSP> &arguments);

// 内部函数， 非插件接口处理断开连接 清理资源
extern "C" void closeAmd(Heap *heap, vector<ConstantSP> &arguments);

extern "C" ConstantSP getCodeList(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP getETFCodeList(Heap *heap, vector<ConstantSP> &arguments);


#endif