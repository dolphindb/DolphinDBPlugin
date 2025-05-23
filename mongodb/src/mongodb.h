
#include <CoreConcept.h>
#include "ddbplugin/CommonInterface.h"

extern "C" ConstantSP mongodbConnect(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP mongodbLoad(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP mongodbAggregate(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP mongodbClose(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP mongodbParseJson(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP mongodbGetCollections(Heap *heap, vector<ConstantSP> &arguments);

class Defer {
public:
    Defer(std::function<void()> code): code(code) {}
    ~Defer() { code(); }
private:
    std::function<void()> code;
};