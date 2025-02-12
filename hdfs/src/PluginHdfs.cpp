#include "PluginHdfs.h"
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <unistd.h>
#include "Exceptions.h"
#include "Logger.h"
#include "kerberos.h"

using namespace std;

static Mutex mutex;

Mutex HDFS_MUTEX;

string getErrorMsgWithPrefix(const string &defaultString)
{
    if(errno != 0)
        return PLUGIN_HDFS_LOG_PREFIX + strerror(errno);
    else
        return PLUGIN_HDFS_LOG_PREFIX + defaultString;
}

template <typename T>
static T *getConnection(ConstantSP &handler)
{
    if (handler->getType() != DT_RESOURCE)
    {
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + "Invalid connection object.");
    }
    else if (handler->getLong() == 0)
    {
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + "The object is empty.");
    }
    else
    {
        return (T *)(handler->getLong());
    }
}

template <typename T>
static void hdfsOnClose(Heap *heap, vector<ConstantSP> &args)
{
    T *pObject = (T *)(args[0]->getLong());
    if (pObject != nullptr)
    {
        delete (T *)(args[0]->getLong());
        args[0]->setLong(0);
    }
}

static void hdfsOnCloseForConnect(Heap *heap, vector<ConstantSP> &args)
{
    LockGuard<Mutex> lock(&mutex);
    if(args[0]->getLong() == 0){
        LOG_ERR(PLUGIN_HDFS_LOG_PREFIX + "The object is empty. ");
        return;
    }
    auto fs =  (hdfs_internal *)(args[0]->getLong());
    if (hdfsDisconnect(fs) == -1)
    {
        args[0]->setLong(0);
        throw RuntimeException(getErrorMsgWithPrefix("Error occurred when disconnecting."));
    }
    args[0]->setLong(0);
}

ConstantSP hdfs_Connect(Heap *heap, vector<ConstantSP> &args)
{
    LockGuard<Mutex> lock(&HDFS_MUTEX);
    const auto usage = string("Usage: connect(nameMode, [port], [userName], [kerbTicketCachePath])\n");
    struct hdfsBuilder *pbld = hdfsNewBuilder();
    hdfsBuilderSetForceNewInstance(pbld);
    string host, userName, cachePath;

    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR)
         throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "nameMode[:port] must be a string scalar.");
    host = args[0]->getString();
    hdfsBuilderSetNameNode(pbld, host.c_str());
    if (args.size() > 1) {
        if (args[1]->getType() != DT_INT || args[1]->getForm() != DF_SCALAR)
             throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "port must be an integer.");
        int port = args[1]->getInt();
        if (port > 0) {
            hdfsBuilderSetNameNodePort(pbld, port);
        }
    }
    if (args.size() > 2 && !args[2]->isNull())
    {
        if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR || args[2]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "userName must be a string scalar.");
        userName = args[2]->getString();
        hdfsBuilderSetUserName(pbld, userName.c_str());
    }
    if(args.size() == 7) {
        if (args[3]->getType() != DT_STRING || args[3]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "kerbTicketCachePath must be a string scalar.");
        cachePath = args[3]->getString();
        if (args[4]->getType() != DT_STRING || args[4]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "keytabPath must be a string scalar.");
        string keytabPath = args[4]->getString();
        if (args[5]->getType() != DT_STRING || args[5]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "principal must be a string scalar.");
        string principal = args[5]->getString();
        if (args[6]->getType() != DT_STRING || args[6]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "lifeTime must be a string scalar.");
        string lifeTime = args[6]->getString();

        hdfsKInit(keytabPath, cachePath, principal, lifeTime);

    } else if (args.size() == 4 && !args[3]->isNull()) {
        if (args[3]->getType() != DT_STRING || args[3]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "kerbTicketCachePath must be a string scalar.");
        cachePath = args[3]->getString();
    } else if(args.size() > 4) {
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "connect should be either less than or equal to 4 parameter or just 7 parameters.");
    }
    hdfsBuilderSetKerbTicketCachePath(pbld, cachePath.c_str());

    hdfsFS fs = hdfsBuilderConnect(pbld);
    if (fs == nullptr)
        throw RuntimeException(getErrorMsgWithPrefix("Failed to connect to hdfs."));
    FunctionDefSP onClose(Util::createSystemProcedure("hdfsFS onClose()", hdfsOnCloseForConnect, 1, 1));
    ConstantSP ret = Util::createResource(
        (long long)fs,
        "hdfsFS connection",
        onClose,
        heap->currentSession());
    return ret;
}

ConstantSP hdfs_Disconnect(Heap *heap, vector<ConstantSP> &args)
{
    LockGuard<Mutex> lock(&HDFS_MUTEX);
    const auto usage = string("Usage: disconnect(hdfsFS).\n");
    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "hdfsFS connection")
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "hdfsFS should be a hdfsFS handle.");
    auto fs = getConnection<hdfs_internal>(args[0]);
    if (hdfsDisconnect(fs) == -1)
    {
        args[0]->setLong(0);
        throw RuntimeException(getErrorMsgWithPrefix("Error occurred when disconnecting."));
    }
    args[0]->setLong(0);
    return new Void();
}

ConstantSP hdfs_Exists(Heap *heap, vector<ConstantSP> &args)
{
    LockGuard<Mutex> lock(&HDFS_MUTEX);
    const auto usage = string("Usage: exists(hdfsFS, path).\n");
    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "hdfsFS connection")
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "hdfsFS should be a hdfsFS handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "path must be a string scalar.");

    if (hdfsExists(getConnection<hdfs_internal>(args[0]), args[1]->getString().c_str()) == -1)
        throw RuntimeException(getErrorMsgWithPrefix("The path does not exist."));
    return new Void();
}

ConstantSP hdfs_Copy(Heap *heap, vector<ConstantSP> &args)
{
    LockGuard<Mutex> lock(&HDFS_MUTEX);
    const auto usage = string("Usage: copy(hdfsFS1,src,hdfsFS2,dst).\n");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "hdfsFS connection")
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "hdfsFS1 should be a hdfsFS handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "src path must be a string scalar.");
    if (args[2]->getType() != DT_RESOURCE || args[2]->getString() != "hdfsFS connection")
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "hdfsFS2 should be a hdfsFS handle.");
    if (args[3]->getType() != DT_STRING || args[3]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "dst path must be a string scalar.");

    if (hdfsCopy(getConnection<hdfs_internal>(args[0]), args[1]->getString().c_str(), getConnection<hdfs_internal>(args[2]), args[3]->getString().c_str()) == -1)
        throw RuntimeException(getErrorMsgWithPrefix("Error occurred when copying"));
    return new Void();
}

ConstantSP hdfs_Move(Heap *heap, vector<ConstantSP> &args)
{
    LockGuard<Mutex> lock(&HDFS_MUTEX);
    const auto usage = string("Usage: move(hdfsFS1, src, hdfsFS2, dst).\n");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "hdfsFS connection")
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "hdfsFS1 should be a hdfsFS handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "src path must be a string scalar.");
    if (args[2]->getType() != DT_RESOURCE || args[2]->getString() != "hdfsFS connection")
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "hdfsFS2 should be a hdfsFS handle.");
    if (args[3]->getType() != DT_STRING || args[3]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "dst path must be a string scalar.");

    if(strcmp(args[1]->getString().c_str(), args[3]->getString().c_str()) == 0){
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "src path is same as dst path.");
    }
    if (hdfsMove(getConnection<hdfs_internal>(args[0]), args[1]->getString().c_str(), getConnection<hdfs_internal>(args[2]), args[3]->getString().c_str()) == -1)
        throw RuntimeException(getErrorMsgWithPrefix("Error occurred when moving"));
    return new Void();
}

ConstantSP hdfs_Delete(Heap *heap, vector<ConstantSP> &args)
{
    LockGuard<Mutex> lock(&HDFS_MUTEX);
    const auto usage = string("Usage: delete(hdfsFS, path, recursive).\n");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "hdfsFS connection")
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "hdfsFS should be a hdfsFS handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "path must be a string scalar.");
    if (args[2]->getType() != DT_INT || args[2]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "recursive should be integer.");

    if (hdfsDelete(getConnection<hdfs_internal>(args[0]), args[1]->getString().c_str(), args[2]->getInt()) == -1)
        throw RuntimeException(getErrorMsgWithPrefix("Error occurred when deleting"));
    return new Void();
}

ConstantSP hdfs_Rename(Heap *heap, vector<ConstantSP> &args)
{
    LockGuard<Mutex> lock(&HDFS_MUTEX);
    const auto usage = string("Usage: rename(hdfsFS, oldPath, newPath).\n");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "hdfsFS connection")
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "hdfsFS should be a hdfsFS handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "oldPath must be a string scalar.");
    if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "newPath must be a string scalar.");

    if (hdfsRename(getConnection<hdfs_internal>(args[0]), args[1]->getString().c_str(), args[2]->getString().c_str()) == -1)
        throw RuntimeException(getErrorMsgWithPrefix("Error occurred when renaming"));
    return new Void();
}

ConstantSP hdfs_CreateDirectory(Heap *heap, vector<ConstantSP> &args)
{
    LockGuard<Mutex> lock(&HDFS_MUTEX);
    const auto usage = string("Usage: createDirectory(hdfsFS, path).\n");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "hdfsFS connection")
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "hdfsFS should be a hdfsFS handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "path must be a string scalar.");

    if (hdfsCreateDirectory(getConnection<hdfs_internal>(args[0]), args[1]->getString().c_str()) == -1)
        throw RuntimeException(getErrorMsgWithPrefix("Error occurred when creating a directory"));
    return new Void();
}

static short encodeMode(short mode) {
    if(mode < 0 || mode >777) {
        throw RuntimeException(PLUGIN_HDFS_LOG_PREFIX + "Invalid mode \"" + to_string(mode) + "\".");
    }
    short m1 = mode % 10;
    mode /= 10;
    short m2 = mode % 10;
    mode /= 10;
    short m3 = mode % 10;
    if(m1 > 7 || m2 > 7 || m3 > 7) {
        throw RuntimeException(PLUGIN_HDFS_LOG_PREFIX + "Invalid mode \"" + to_string(m3) + to_string(m2) + to_string(m1)  + "\".");
    }
    return m3*64 + m2*8 + m1;
}

static short decodeMode(short mode) {
    if(mode < 0 || mode >511) {
        LOG_WARN(PLUGIN_HDFS_LOG_PREFIX + "Invalid mode \"" + to_string(mode) + "\".");
    }
    short m1 = mode % 8;
    mode /= 8;
    short m2 = mode % 8;
    mode /= 8;
    short m3 = mode % 8;

    return m3*100 + m2*10 + m1;
}

ConstantSP hdfs_Chmod(Heap *heap, vector<ConstantSP> &args)
{
    LockGuard<Mutex> lock(&HDFS_MUTEX);
    const auto usage = string("Usage: chmod(hdfsFS, path, mode).\n");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "hdfsFS connection")
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "hdfsFS should be a hdfsFS handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "path must be a string scalar.");
    if (args[2]->getType() != DT_INT || args[2]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "mode must be a integer.");

    if (hdfsChmod(getConnection<hdfs_internal>(args[0]), args[1]->getString().c_str(), encodeMode(args[2]->getShort())) == -1)
        throw RuntimeException(getErrorMsgWithPrefix("Error occurred when changing the ownership"));
    return new Void();
}

ConstantSP hdfs_getListDirectory(Heap *heap, vector<ConstantSP> &args)
{
    LockGuard<Mutex> lock(&HDFS_MUTEX);
    const auto usage = string("Usage: getListDirectory(hdfsFS, path).\n");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "hdfsFS connection")
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "hdfsFS should be a hdfsFS handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "path must be a string scalar.");

    auto fileInfo = new FileInfo();
    fileInfo->size = 0;
    fileInfo->info = hdfsListDirectory(getConnection<hdfs_internal>(args[0]), args[1]->getString().c_str(), &fileInfo->size);
    if (fileInfo->info == nullptr && errno != 0){
        throw RuntimeException(getErrorMsgWithPrefix("Error occurred when get directory information."));
    }
    FunctionDefSP onClose(Util::createSystemProcedure("FileInfo onClose()", hdfsOnClose<FileInfo>, 1, 1));
    //TODO check memory leak
    return Util::createResource(
        (long long)fileInfo,
        "FileInfo connection",
        onClose,
        heap->currentSession());
}

ConstantSP hdfs_listDirectory(Heap *heap, vector<ConstantSP> &args)
{
    LockGuard<Mutex> lock(&HDFS_MUTEX);
    const auto usage = string("Usage: listDirectory(fileInfo).\n");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "FileInfo connection")
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "fileInfo should be a fileInfo handle.");
    auto fileInfo = getConnection<FileInfo>(args[0]);

    ConstantSP kingVec = Util::createVector(DT_STRING, fileInfo->size);
    ConstantSP nameVec = Util::createVector(DT_STRING, fileInfo->size);
    ConstantSP lastModVec = Util::createVector(DT_DATETIME, fileInfo->size);
    ConstantSP sizeVec = Util::createVector(DT_LONG, fileInfo->size);
    ConstantSP repVec = Util::createVector(DT_SHORT, fileInfo->size);
    ConstantSP blockSizeVec = Util::createVector(DT_LONG, fileInfo->size);
    ConstantSP ownerVec = Util::createVector(DT_STRING, fileInfo->size);
    ConstantSP groupVec = Util::createVector(DT_STRING, fileInfo->size);
    ConstantSP permissionVec = Util::createVector(DT_SHORT, fileInfo->size);
    ConstantSP lastAccessVec = Util::createVector(DT_DATETIME, fileInfo->size);
    for (int i = 0; i < fileInfo->size; i++)
    {
        kingVec->setString(i, fileInfo->info[i].mKind == kObjectKindFile ? "F" : "D");
        nameVec->setString(i, fileInfo->info[i].mName);
        lastModVec->setInt(i, fileInfo->info[i].mLastMod);
        sizeVec->setLong(i, fileInfo->info[i].mSize);
        repVec->setShort(i, fileInfo->info[i].mReplication);
        blockSizeVec->setLong(i, fileInfo->info[i].mBlockSize);
        ownerVec->setString(i, fileInfo->info[i].mOwner);
        groupVec->setString(i, fileInfo->info[i].mGroup);
        permissionVec->setShort(i, decodeMode(fileInfo->info[i].mPermissions));
        lastAccessVec->setInt(i, fileInfo->info[i].mLastAccess);
    }
    vector<string> colNames = {"mKind", "mName", "mLastMod", "mSize", "mReplication", "mBlockSize", "mOwner", "mGroup", "mPermissions", "mLastAccess"};
    vector<ConstantSP> cols = {kingVec, nameVec, lastModVec, sizeVec, repVec, blockSizeVec, ownerVec, groupVec, permissionVec, lastAccessVec};

    return Util::createTable(colNames, cols);
}

ConstantSP hdfs_freeFileInfo(Heap *heap, vector<ConstantSP> &args)
{
    LockGuard<Mutex> lock(&HDFS_MUTEX);
    const auto usage = string("Usage: freeFileInfo(fileInfo).\n");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "FileInfo connection")
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "fileInfo should be a fileInfo handle.");
    auto fileInfo = getConnection<FileInfo>(args[0]);
    hdfsFreeFileInfo(fileInfo->info, fileInfo->size);
    args[0]->setLong(0);

    return new Void();
}

ConstantSP hdfs_readFile(Heap *heap, vector<ConstantSP> &args)
{
    LockGuard<Mutex> lock(&HDFS_MUTEX);
    const auto usage = string("Usage: readFile(hdfsFS, path, handler).\n");
    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "hdfsFS connection")
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "hdfsFS should be a hdfsFS handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "path must be a string scalar.");
    if (args[2]->getType() != DT_FUNCTIONDEF)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "handler should be a function.");
    hdfsFS fs = getConnection<hdfs_internal>(args[0]);
    string path = args[1]->getString();
    FunctionDefSP handler = args[2];
    if (handler->getParamCount() != 2)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "handler function should accept two parameter.");
    hdfsFile pFile = hdfsOpenFile(fs, path.c_str(), O_RDONLY, 0, 0, 0);
    auto fileInfo = new FileInfo();
    fileInfo->info = hdfsListDirectory(fs, path.c_str(), &fileInfo->size);
    if (fileInfo->info == nullptr || fileInfo->size != 1)
        throw RuntimeException(getErrorMsgWithPrefix("Error occurred when get file information."));
    uint64_t *bufSize = new uint64_t;
    *bufSize = fileInfo->info[0].mSize;
    char *buffer = new char[*bufSize];
    tSize readSize = *bufSize;
    int readBytes;
    uint64_t readOffset = 0;
    while(readOffset < *bufSize){
        readBytes = hdfsPread(fs, pFile, 0, buffer + readOffset, readSize);
        readOffset += readBytes;
        if(readBytes == -1){
            throw RuntimeException(getErrorMsgWithPrefix("Error occurred when read file data."));
        }
    }
    if(hdfsCloseFile(fs, pFile) == -1){
        throw RuntimeException(getErrorMsgWithPrefix("Error occurred when close file handle."));
    }
    FunctionDefSP onCloseChar(Util::createSystemProcedure("hdfs readFile onClose()", hdfsOnClose<char>, 1, 1));
    FunctionDefSP onCloseLong(Util::createSystemProcedure("hdfs readFile onClose()", hdfsOnClose<uint64_t>, 1, 1));
    vector<ConstantSP> handlerArgs(2);
    handlerArgs[0] = Util::createResource((long long)buffer, "hdfs readFile address", onCloseChar, heap->currentSession());
    handlerArgs[1] = Util::createResource((long long)bufSize, "hdfs readFile length", onCloseLong, heap->currentSession());
    return handler->call(heap, handlerArgs);
}

ConstantSP hdfs_writeFile(Heap *heap, vector<ConstantSP> &args)
{
    LockGuard<Mutex> lock(&HDFS_MUTEX);
    const auto usage = string("Usage: writeFile(hdfsFS, path, tb, handler).\n");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "hdfsFS connection")
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "hdfsFS should be a hdfsFS handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "path must be a string scalar.");
    if (!args[2]->isTable())
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "tb must be a table");
    if (args[3]->getType() != DT_FUNCTIONDEF)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "handler should be a function.");
    hdfsFS fs = getConnection<hdfs_internal>(args[0]);
    string path = args[1]->getString();
    TableSP tb = args[2];
    FunctionDefSP handler = args[3];
    if (handler->getParamCount() != 1)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_LOG_PREFIX + usage + "handler function should accept one parameter.");

    vector<ConstantSP> handlerArgs(1);
    handlerArgs[0] = tb;
    VectorSP vsp = handler->call(heap, handlerArgs);
    if (vsp->size() != 2 || !vsp->isNumber() || vsp->hasNull())
        throw RuntimeException(getErrorMsgWithPrefix("invalid return value of handler"));

    void *buffer = (void *)vsp->getLong(0);
    uint64_t *length = (uint64_t *)vsp->getLong(1);
    hdfsFile pFile = hdfsOpenFile(fs, path.c_str(), O_WRONLY | O_CREAT, 0, 0, 0);
    if(pFile==nullptr)
        throw RuntimeException(getErrorMsgWithPrefix("Failed to open file for writing!"));
    tSize writeRes = hdfsWrite(fs, pFile, buffer, *length);
    if (hdfsFlush(fs, pFile))
        throw RuntimeException(getErrorMsgWithPrefix("Failed to 'flush' writeFile"));
    if (writeRes == -1)
        throw RuntimeException(getErrorMsgWithPrefix("error occured while writing data to hdfs"));
    hdfsCloseFile(fs, pFile);
    return new Void();
}