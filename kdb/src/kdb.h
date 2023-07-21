#include "CoreConcept.h"
#include "k.h"

extern "C" ConstantSP kdbConnect(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP kdbLoadTable(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP kdbLoadFile(Heap *heap, vector<ConstantSP> &arguments);
extern "C" ConstantSP kdbClose(Heap *heap, vector<ConstantSP> &arguments);

enum kdbType {
    KDB_LIST = 0,
    KDB_BOOL = 1,
    KDB_GUID = 2,
    KDB_BYTE = 4,
    KDB_SHORT = 5,
    KDB_INT = 6,
    KDB_LONG = 7,
    KDB_FLOAT = 8,
    KDB_DOUBLE = 9,
    KDB_CHAR = 10,
    KDB_STRING = 11,
    KDB_TIMESTAMP = 12,
    KDB_MONTH = 13,
    KDB_DATE = 14,
    KDB_DATETIME = 15,
    KDB_TIMESPAN = 16,
    KDB_MINUTE = 17,
    KDB_SECOND = 18,
    KDB_TIME = 19,
    KDB_TABLE = 98,
    KDB_DICT = 99,
};

class Defer {
public:
    Defer(std::function<void()> code)
        : code(code)
    {
    }
    ~Defer() { code(); }

private:
    std::function<void()> code;
};

class Connection {
public:
    Connection(const string& host, const int& port, const string& usernamePassword);
    ~Connection();

    TableSP getTable(const string& tablePath, const string& symFilePath);
    string str() {
        return host_ + ":" + std::to_string(port_);
    }
private:
    string host_;
    int port_;
    int handle_;
    // Mutex kdbMutex_;
};