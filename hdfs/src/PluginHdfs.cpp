#include "PluginHdfs.h"

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

template <typename T>
static T *getConnection(ConstantSP &handler)
{
    if (handler->getType() != DT_RESOURCE)
    {
        throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
    }
    else if (handler->getLong() == 0)
    {
        throw IllegalArgumentException(__FUNCTION__, "The object is empty.");
    }
    else
    {
        return (T *)(handler->getLong());
    }
}

ConstantSP hdfs_Connect(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: connect(nameMode, port, [userName], [kerbTicketCachePath])\n");

    struct hdfsBuilder *pbld = hdfsNewBuilder();
    string host, userName, path;
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, usage + "nameMode[:port] must be a legal string.");
    host = args[0]->getString();
    hdfsBuilderSetNameNode(pbld, host.c_str());
    if (args[1]->getType() != DT_INT || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, usage + "port must be an integer.");
    hdfsBuilderSetNameNodePort(pbld, args[1]->getInt());
    if (args.size() > 2)
    {
        if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR || args[2]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, usage + "userName must be a legal string.");
        userName = args[2]->getString();
        hdfsBuilderSetUserName(pbld, userName.c_str());
    }
    if (args.size() > 3)
    {
        if (args[3]->getType() != DT_STRING || args[3]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, usage + "kerbTicketCachePath must be a legal string.");
        path = args[3]->getString();
        hdfsBuilderSetKerbTicketCachePath(pbld, path.c_str());
    }

    hdfsFS fs = hdfsBuilderConnect(pbld);
    if (fs == nullptr)
        throw RuntimeException("Failed to connect to hdfs.");
    FunctionDefSP onClose(Util::createSystemProcedure("hdfsFS onClose()", hdfsOnClose<hdfs_internal>, 1, 1));
    return Util::createResource(
        (long long)fs,
        "hdfsFS connection",
        onClose,
        heap->currentSession());
}

ConstantSP hdfs_Disconnect(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: disconnect(hdfsFS).\n");
    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "hdfsFS connection")
        throw IllegalArgumentException(__FUNCTION__, usage + "hdfsFS should be a hdfsFS handle.");
    auto fs = getConnection<hdfs_internal>(args[0]);
    if (hdfsDisconnect(fs) == -1)
    {
        args[0]->setLong(0);
        throw RuntimeException("Error occurred when disconnecting.");
    }
    args[0]->setLong(0);
    return new Void();
}

ConstantSP hdfs_Exists(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: exists(hdfsFS, path).\n");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "hdfsFS connection")
        throw IllegalArgumentException(__FUNCTION__, usage + "hdfsFS should be a hdfsFS handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, usage + "path must be a legal string.");

    if (hdfsExists(getConnection<hdfs_internal>(args[0]), args[1]->getString().c_str()) == -1)
        throw RuntimeException("The path does not exist.");
    return new Void();
}

ConstantSP hdfs_Copy(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: copy(hdfsFS1,src,hdfsFS2,dst).\n");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "hdfsFS connection")
        throw IllegalArgumentException(__FUNCTION__, usage + "hdfsFS1 should be a hdfsFS handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, usage + "src path must be a legal string.");
    if (args[2]->getType() != DT_RESOURCE || args[2]->getString() != "hdfsFS connection")
        throw IllegalArgumentException(__FUNCTION__, usage + "hdfsFS2 should be a hdfsFS handle.");
    if (args[3]->getType() != DT_STRING || args[3]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, usage + "dst path must be a legal string.");

    if (hdfsCopy(getConnection<hdfs_internal>(args[0]), args[1]->getString().c_str(), getConnection<hdfs_internal>(args[2]), args[3]->getString().c_str()) == -1)
        throw RuntimeException("Error occurred when copying");
    return new Void();
}

ConstantSP hdfs_Move(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: move(hdfsFS1, src, hdfsFS2, dst).\n");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "hdfsFS connection")
        throw IllegalArgumentException(__FUNCTION__, usage + "hdfsFS1 should be a hdfsFS handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, usage + "src path must be a legal string.");
    if (args[2]->getType() != DT_RESOURCE || args[2]->getString() != "hdfsFS connection")
        throw IllegalArgumentException(__FUNCTION__, usage + "hdfsFS2 should be a hdfsFS handle.");
    if (args[3]->getType() != DT_STRING || args[3]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, usage + "dst path must be a legal string.");

    if (hdfsMove(getConnection<hdfs_internal>(args[0]), args[1]->getString().c_str(), getConnection<hdfs_internal>(args[2]), args[3]->getString().c_str()) == -1)
        throw RuntimeException("Error occurred when copying");
    return new Void();
}

ConstantSP hdfs_Delete(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: delete(hdfsFS, path, recursive).\n");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "hdfsFS connection")
        throw IllegalArgumentException(__FUNCTION__, usage + "hdfsFS should be a hdfsFS handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, usage + "path must be a legal string.");
    if (args[2]->getType() != DT_INT || args[2]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, usage + "recursive should be integer.");

    if (hdfsDelete(getConnection<hdfs_internal>(args[0]), args[1]->getString().c_str(), args[2]->getInt()) == -1)
        throw RuntimeException("Error occurred when deleting");
    return new Void();
}

ConstantSP hdfs_Rename(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: rename(hdfsFS, oldPath, newPath).\n");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "hdfsFS connection")
        throw IllegalArgumentException(__FUNCTION__, usage + "hdfsFS should be a hdfsFS handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, usage + "oldPath must be a legal string.");
    if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, usage + "oldPath must be a legal string.");

    if (hdfsRename(getConnection<hdfs_internal>(args[0]), args[1]->getString().c_str(), args[2]->getString().c_str()) == -1)
        throw RuntimeException("Error occurred when renaming");
    return new Void();
}

ConstantSP hdfs_CreateDirectory(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: createDirectory(hdfsFS, path).\n");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "hdfsFS connection")
        throw IllegalArgumentException(__FUNCTION__, usage + "hdfsFS should be a hdfsFS handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, usage + "path must be a legal string.");

    if (hdfsCreateDirectory(getConnection<hdfs_internal>(args[0]), args[1]->getString().c_str()) == -1)
        throw RuntimeException("Error occurred when creating a directory");
    return new Void();
}

ConstantSP hdfs_Chmod(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: chmod(hdfsFS, path, mode).\n");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "hdfsFS connection")
        throw IllegalArgumentException(__FUNCTION__, usage + "hdfsFS should be a hdfsFS handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, usage + "path must be a legal string.");
    if (args[2]->getType() != DT_INT || args[2]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, usage + "mode must be a integer.");

    if (hdfsChmod(getConnection<hdfs_internal>(args[0]), args[1]->getString().c_str(), args[2]->getShort()) == -1)
        throw RuntimeException("Error occurred when changing the ownership");
    return new Void();
}

ConstantSP hdfs_getListDirectory(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: getListDirectory(hdfsFS, path).\n");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "hdfsFS connection")
        throw IllegalArgumentException(__FUNCTION__, usage + "hdfsFS should be a hdfsFS handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, usage + "path must be a legal string.");

    auto fileInfo = new FileInfo();
    fileInfo->info = hdfsListDirectory(getConnection<hdfs_internal>(args[0]), args[1]->getString().c_str(), &fileInfo->size);
    if (fileInfo->info == nullptr)
        throw RuntimeException("Error occurred when get directory information.");

    FunctionDefSP onClose(Util::createSystemProcedure("FileInfo onClose()", hdfsOnClose<FileInfo>, 1, 1));
    return Util::createResource(
        (long long)fileInfo,
        "FileInfo connection",
        onClose,
        heap->currentSession());
}

ConstantSP hdfs_listDirectory(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: listDirectory(fileInfo).\n");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "FileInfo connection")
        throw IllegalArgumentException(__FUNCTION__, usage + "fileInfo should be a fileInfo handle.");
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
        permissionVec->setShort(i, fileInfo->info[i].mPermissions);
        lastAccessVec->setInt(i, fileInfo->info[i].mLastAccess);
    }
    vector<string> colNames = {"mKind", "mName", "mLastMod", "mSize", "mReplication", "mBlockSize", "mOwner", "mGroup", "mPermissions", "mLastAccess"};
    vector<ConstantSP> cols = {kingVec, nameVec, lastModVec, sizeVec, repVec, blockSizeVec, ownerVec, groupVec, permissionVec, lastAccessVec};

    return Util::createTable(colNames, cols);
}

ConstantSP hdfs_freeFileInfo(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: freeFileInfo(fileInfo).\n");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "FileInfo connection")
        throw IllegalArgumentException(__FUNCTION__, usage + "fileInfo should be a fileInfo handle.");
    auto fileInfo = getConnection<FileInfo>(args[0]);
    hdfsFreeFileInfo(fileInfo->info, fileInfo->size);
    args[0]->setLong(0);

    return new Void();
}

ConstantSP hdfs_readFile(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: readFile(hdfsFS, path, handler).\n");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "hdfsFS connection")
        throw IllegalArgumentException(__FUNCTION__, usage + "hdfsFS should be a hdfsFS handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, usage + "path must be a legal string.");
    if (args[2]->getType() != DT_FUNCTIONDEF)
        throw IllegalArgumentException(__FUNCTION__, usage + "handler should be a function.");
    hdfsFS fs = getConnection<hdfs_internal>(args[0]);
    string path = args[1]->getString();
    FunctionDefSP handler = args[2];
    if (handler->getParamCount() != 2)
        throw IllegalArgumentException(__FUNCTION__, usage + "handler function should accept two parameter.");
    hdfsFile pFile = hdfsOpenFile(fs, path.c_str(), O_RDONLY, 0, 0, 0);
    auto fileInfo = new FileInfo();
    fileInfo->info = hdfsListDirectory(fs, path.c_str(), &fileInfo->size);
    if (fileInfo->info == nullptr || fileInfo->size != 1)
        throw RuntimeException("Error occurred when get file information.");
    uint64_t *bufSize = new uint64_t;
    *bufSize = fileInfo->info[0].mSize;
    char *buffer = new char[*bufSize];
    void *vBuf = buffer;
    tSize readSize = *bufSize;
    hdfsRead(fs, pFile, vBuf, readSize);

    FunctionDefSP onCloseChar(Util::createSystemProcedure("hdfs readFile onClose()", hdfsOnClose<char>, 1, 1));
    FunctionDefSP onCloseLong(Util::createSystemProcedure("hdfs readFile onClose()", hdfsOnClose<uint64_t>, 1, 1));
    vector<ConstantSP> handlerArgs(2);
    handlerArgs[0] = Util::createResource((long long)vBuf, "hdfs readFile address", onCloseChar, heap->currentSession());
    handlerArgs[1] = Util::createResource((long long)bufSize, "hdfs readFile length", onCloseLong, heap->currentSession());
    return handler->call(heap, handlerArgs);
}

ConstantSP hdfs_writeFile(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: writeFile(hdfsFS, path, tb, handler).\n");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "hdfsFS connection")
        throw IllegalArgumentException(__FUNCTION__, usage + "hdfsFS should be a hdfsFS handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, usage + "path must be a legal string.");
    if (!args[2]->isTable())
        throw IllegalArgumentException(__FUNCTION__, usage + "tb must be a table");
    if (args[3]->getType() != DT_FUNCTIONDEF)
        throw IllegalArgumentException(__FUNCTION__, usage + "handler should be a function.");
    hdfsFS fs = getConnection<hdfs_internal>(args[0]);
    string path = args[1]->getString();
    TableSP tb = args[2];
    FunctionDefSP handler = args[3];
    if (handler->getParamCount() != 1)
        throw IllegalArgumentException(__FUNCTION__, usage + "handler function should accept one parameter.");

    vector<ConstantSP> handlerArgs(1);
    handlerArgs[0] = tb;
    VectorSP vsp = handler->call(heap, handlerArgs);
    if (vsp->size() != 2 || !vsp->isNumber() || vsp->hasNull())
        throw RuntimeException("invalid return value of handler");

    void *buffer = (void *)vsp->getLong(0);
    uint64_t *length = (uint64_t *)vsp->getLong(1);
    hdfsFile pFile = hdfsOpenFile(fs, path.c_str(), O_WRONLY | O_CREAT, 0, 0, 0);
    if(pFile==nullptr)
        throw RuntimeException("Failed to open file for writing!");
    tSize writeRes = hdfsWrite(fs, pFile, buffer, *length);
    if (hdfsFlush(fs, pFile))
        throw RuntimeException("Failed to 'flush' writeFile");
    if (writeRes == -1)
        throw RuntimeException("error occured while writing data to hdfs");
    hdfsCloseFile(fs, pFile);
    return new Void();
}