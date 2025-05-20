#include <string>

#include "CoreConcept.h"
#include "ScalarImp.h"
#include "ddbplugin/Plugin.h"

#include "k.h"
#include "kptr.h"

#define PLUGIN_NAME "[PLUGIN::KDB] "
using ddb::ConstantSP;
using ddb::Heap;
using ddb::TableSP;
using ddb::FunctionDefSP;
using std::vector;

extern "C" {

    ConstantSP kdbConnect(Heap *heap, vector<ConstantSP> &arguments);
    ConstantSP kdbLoadTable(Heap *heap, vector<ConstantSP> &arguments);
    ConstantSP kdbLoadFile(Heap *heap, vector<ConstantSP> &arguments);
    ConstantSP kdbClose(Heap *heap, vector<ConstantSP> &arguments);

    ConstantSP kdbExecute(Heap *heap, vector<ConstantSP> &arguments);
    ConstantSP kdbLoadTableEx(Heap *heap, vector<ConstantSP> &arguments);
    ConstantSP kdbLoadFileEx(Heap *heap, vector<ConstantSP> &arguments);

    ConstantSP kdbExtractTableSchema(Heap *heap, vector<ConstantSP> &arguments);
    ConstantSP kdbExtractFileSchema(Heap *heap, vector<ConstantSP> &arguments);

}//extern "C"

////////////////////////////////////////////////////////////////////////////////

class Connection {
public:
    static constexpr char const* MARKER = "kdb+ connection";

public:
    Connection(const std::string& host, const int port, const std::string& usernamePassword);
    ~Connection();

    std::string str() const;

    TableSP getTable(const std::string &tablePath, const std::string &symFilePath) const;
    ConstantSP loadTableEx(Heap *heap, ddb::DBHandleWrapper dbHandle, ConstantSP tableName, ConstantSP partitionColumns,
                           TableSP schema, long long batchSize, FunctionDefSP transform, ConstantSP sortColumns,
                           string &pathOrScript, string &symPath);
    ConstantSP execute(const std::string &qScript) const;
    ConstantSP extractSchema(const std::string &tablePath, const std::string &symPath) const;

  private:
    KPtr kExec(const string& command) const;

    std::string loadSymFile(const std::string& symFilePath) const;
    ConstantSP loadColumn(const std::string& tableName, const std::string& colName) const;

private:
    std::string host_;
    int port_;
    int handle_;

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