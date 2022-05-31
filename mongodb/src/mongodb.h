
#include <CoreConcept.h>

extern "C" ConstantSP mongodbConnect(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP mongodbLoad(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP mongodbAggregate(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP mongodbClose(const ConstantSP &handle, const ConstantSP &b);

class Defer {
public:
    Defer(std::function<void()> code): code(code) {}
    ~Defer() { code(); }
private:
    std::function<void()> code;
};