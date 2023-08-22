#include <type_traits>
#include <memory>

#include "CoreConcept.h"

#include "k.h"

extern "C" {

    ConstantSP kdbConnect(Heap *heap, vector<ConstantSP> &arguments);
    ConstantSP kdbLoadTable(Heap *heap, vector<ConstantSP> &arguments);
    ConstantSP kdbLoadFile(Heap *heap, vector<ConstantSP> &arguments);
    ConstantSP kdbClose(Heap *heap, vector<ConstantSP> &arguments);

}//extern "C"

////////////////////////////////////////////////////////////////////////////////

struct KDeleter {
    void operator()(K k) const;
};
using KPtr = std::unique_ptr<typename std::remove_pointer<K>::type, KDeleter>;

class Connection {
public:
    static const char* const marker;

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

enum kdbType: short {
    KDB_LIST = 0,
    KDB_BOOL = (KB),
    KDB_GUID = (UU),
    KDB_BYTE = (KG),
    KDB_SHORT = (KH),
    KDB_INT = (KI),
    KDB_LONG = (KJ),
    KDB_FLOAT = (KE),
    KDB_DOUBLE = (KF),
    KDB_CHAR = (KC),
    KDB_STRING = (KS),
    KDB_TIMESTAMP = (KP),
    KDB_MONTH = (KM),
    KDB_DATE = (KD),
    KDB_DATETIME = (KZ),
    KDB_TIMESPAN = (KN),
    KDB_MINUTE = (KU),
    KDB_SECOND = (KV),
    KDB_TIME = (KT),
    KDB_ENUM_MIN = 20,
    KDB_ENUM_MAX = 76,
    KDB_NESTED_MIN = 77,
    KDB_NESTED_MAX = 97,
    KDB_TABLE = (XT),
    KDB_DICT = (XD),
    KDB_FUNCTION_MIN = 100,
    KDB_FUNCTION_MAX = 112,
    KDB_ERROR = -128,
};

class Defer {
public:
    using action_t = std::function<void()>;

public:
    Defer(action_t code) : code_{code} {}
    ~Defer() { code_(); }

private:
    const action_t code_;
};