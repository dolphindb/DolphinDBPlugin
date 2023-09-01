#include <string>

#include "CoreConcept.h"

#include "k.h"
#include "kptr.h"

#define PLUGIN_NAME "[PLUGIN::KDB]"

extern "C" {

    ConstantSP kdbConnect(Heap *heap, vector<ConstantSP> &arguments);
    ConstantSP kdbLoadTable(Heap *heap, vector<ConstantSP> &arguments);
    ConstantSP kdbLoadFile(Heap *heap, vector<ConstantSP> &arguments);
    ConstantSP kdbClose(Heap *heap, vector<ConstantSP> &arguments);

}//extern "C"

////////////////////////////////////////////////////////////////////////////////

class Connection {
public:
    static constexpr char const* MARKER = "kdb+ connection";

public:
    Connection(const std::string& host, const int port, const std::string& usernamePassword);
    ~Connection();

    std::string str() const;

    TableSP getTable(const std::string& tablePath, const std::string& symFilePath) const;

private:
    KPtr kExec(const string& command) const;

    std::string loadSymFile(const std::string& symFilePath) const;
    ConstantSP loadColumn(const std::string& tableName, const std::string& colName) const;

private:
    std::string host_;
    int port_;
    int handle_;
    // Mutex kdbMutex_;

};//class Connection

class Defer {
public:
    using action_t = std::function<void()>;

public:
    Defer(action_t code) : code_{code} {}
    ~Defer() { code_(); }

private:
    const action_t code_;
};