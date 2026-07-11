#pragma once

#include "DolphinDBEverything.h"
#include "MQTTUtil.h"
#include "ddbplugin/CommonInterface.h"

using ddb::ConstantSP;
using ddb::Heap;
using std::vector;

extern "C" ConstantSP mqttConnect(Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP mqttClose(Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP mqttPublish(Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP mqttSubscribe(Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP mqttUnsubscribe(Heap *heap, const vector<ConstantSP> &args);
extern "C" ConstantSP getSubscriberStat(Heap *heap, const vector<ConstantSP> &args);

namespace MQTTPlugin {
using namespace ddb;

class MQTTInstance {
  public:
    MQTTInstance() = default;
    ~MQTTInstance() = default;
    MQTTInstance(const MQTTInstance &) = delete;
    MQTTInstance &operator=(const MQTTInstance &) = delete;

    static MQTTInstance &getInstance() {
        static MQTTInstance instance;
        return instance;
    }

    // interfaces
    ConstantSP mqttConnect(Heap *heap, const vector<ConstantSP> &args);
    ConstantSP mqttClose(const vector<ConstantSP> &args);
    ConstantSP mqttPublish(const vector<ConstantSP> &args);
    ConstantSP mqttSubscribe(Heap *heap, const vector<ConstantSP> &args);
    ConstantSP mqttUnsubscribe(const vector<ConstantSP> &args);
    ConstantSP getSubscriberStat();

  private:
    Mutex subscribersLock_;
    DictionarySP subscribers_ = Util::createDictionary(DT_STRING, 0, DT_ANY, 0);
};

}  // namespace MQTTPlugin
