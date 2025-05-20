#include "HdfsPlugin.h"

#include "Concurrent.h"
#include "Exceptions.h"
#include "HdfsKerberos.h"
#include "HdfsWrapper.h"
#include "Logger.h"
#include "ScalarImp.h"
#include "Types.h"
#include "ddbplugin/CommonInterface.h"
#include "ddbplugin/PluginLoggerImp.h"
#include "hdfs.h"

using namespace hdfsPlugin;

static Mutex mutex;

ConstantSP hdfs_Connect(Heap *heap, vector<ConstantSP> &args)
{
    LockGuard<Mutex> lock(&mutex);
    const auto usage = string("Usage: connect(nameNode, [port], [username], [kerbTicketCachePath], [keytabPath], [principal], [lifetime]) ");
    struct hdfsBuilder *bld = hdfsNewBuilder();
    hdfsBuilderSetForceNewInstance(bld);
    string host, userName, cachePath;

    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR)
         throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "nameMode[:port] must be a string scalar.");
    host = args[0]->getString();
    hdfsBuilderSetNameNode(bld, host.c_str());
    if (args.size() > 1 && !args[1]->isNull()) {
        if (args[1]->getType() != DT_INT || args[1]->getForm() != DF_SCALAR)
             throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "port must be an integer.");
        int port = args[1]->getInt();
        if (port > 0) {
            hdfsBuilderSetNameNodePort(bld, port);
        }
    }
    if (args.size() > 2 && !args[2]->isNull())
    {
        if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR || args[2]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "username must be a string scalar.");
        userName = args[2]->getString();
        hdfsBuilderSetUserName(bld, userName.c_str());
    }
    if(args.size() == 7) {
        if (!args[3]->isNull() && (args[3]->getType() != DT_STRING || args[3]->getForm() != DF_SCALAR))
            throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "kerbTicketCachePath must be a string scalar.");
        cachePath = args[3]->getString();
        if (!args[4]->isNull() && (args[4]->getType() != DT_STRING || args[4]->getForm() != DF_SCALAR))
            throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "keytabPath must be a string scalar.");
        string keytabPath = args[4]->getString();
        if (!args[5]->isNull() && (args[5]->getType() != DT_STRING || args[5]->getForm() != DF_SCALAR))
            throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "principal must be a string scalar.");
        string principal = args[5]->getString();
        if (!args[6]->isNull() && (args[6]->getType() != DT_STRING || args[6]->getForm() != DF_SCALAR))
            throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "lifetime must be a string scalar.");
        string lifeTime = args[6]->getString();

        hdfsKInit(keytabPath, cachePath, principal, lifeTime);

    } else if (args.size() == 4 && !args[3]->isNull()) {
        if (args[3]->getType() != DT_STRING || args[3]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "kerbTicketCachePath must be a string scalar.");
        cachePath = args[3]->getString();
    } else if(args.size() > 4) {
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "connect should be either less than or equal to 4 parameter or just 7 parameters.");
    }
    hdfsBuilderSetKerbTicketCachePath(bld, cachePath.c_str());

    hdfsFS fs = hdfsBuilderConnect(bld);
    if (fs == nullptr) {
        throw RuntimeException(getErrorMsgWithPrefix("connect"));
    }
    return new HdfsConnection(heap, fs);
}

ConstantSP hdfs_Disconnect(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: disconnect(conn) ");
    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != HDFS_CONN_DESC) {
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "conn should be a hdfsFS conn handle.");
    }
    HdfsConnection::disconnect(args[0]);
    return new Void();
}

ConstantSP hdfs_Exists(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: exists(conn, path) ");
    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != HDFS_CONN_DESC)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "conn should be a hdfsFS conn handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "path must be a string scalar.");

    HdfsConnection::exists(args[0], args[1]->getString());
    return new Bool(true);
}

ConstantSP hdfs_Copy(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: copy(sourceConn, sourcePath, targetConn, targetPath) ");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != HDFS_CONN_DESC)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "sourceConn should be a hdfsFS handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "sourcePath must be a string scalar.");
    if (args[2]->getType() != DT_RESOURCE || args[2]->getString() != HDFS_CONN_DESC)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "targetConn should be a hdfsFS handle.");
    if (args[3]->getType() != DT_STRING || args[3]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "targetPath must be a string scalar.");

    HdfsConnection::copy(args[0], args[1]->getString(), args[2], args[3]->getString());
    return new Void();
}

ConstantSP hdfs_Move(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: move(sourceConn,sourcePath,targetConn,targetPath) ");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != HDFS_CONN_DESC)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "sourceConn should be a hdfsFS handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "sourcePath must be a string scalar.");
    if (args[2]->getType() != DT_RESOURCE || args[2]->getString() != HDFS_CONN_DESC)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "targetConn should be a hdfsFS handle.");
    if (args[3]->getType() != DT_STRING || args[3]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "targetPath must be a string scalar.");

    if(strcmp(args[1]->getString().c_str(), args[3]->getString().c_str()) == 0){
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "sourcePath is same as targetPath.");
    }
    HdfsConnection::move(args[0], args[1]->getString(), args[2], args[3]->getString());
    return new Void();
}

ConstantSP hdfs_Delete(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: delete(conn, path, recursive) ");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != HDFS_CONN_DESC)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "conn should be a hdfsFS conn handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "path must be a string scalar.");
    if (args[2]->getCategory() != INTEGRAL || args[2]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "recursive should be integer.");

    HdfsConnection::deletePath(args[0], args[1]->getString(), args[2]->getLong());
    return new Void();
}

ConstantSP hdfs_Rename(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: rename(conn, sourcePath, targetPath) ");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != HDFS_CONN_DESC)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "conn should be a hdfsFS conn handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "sourcePath must be a string scalar.");
    if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "targetPath must be a string scalar.");

    HdfsConnection::rename(args[0], args[1]->getString(), args[2]->getString());
    return new Void();
}

ConstantSP hdfs_CreateDirectory(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: createDirectory(conn, path) ");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != HDFS_CONN_DESC)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "conn should be a hdfsFS conn handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "path must be a string scalar.");

    HdfsConnection::createDirectory(args[0], args[1]->getString());
    return new Void();
}


ConstantSP hdfs_Chmod(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: chmod(conn, path, mode) ");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != HDFS_CONN_DESC)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "conn should be a hdfsFS conn handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "path must be a string scalar.");
    if (args[2]->getType() != DT_INT || args[2]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "mode must be a integer.");

    HdfsConnection::chmod(args[0], args[1]->getString(), args[2]->getInt());
    return new Void();
}

ConstantSP hdfs_getListDirectory(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: getListDirectory(conn, path) ");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != HDFS_CONN_DESC)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "conn should be a hdfsFS conn handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "path must be a string scalar.");

    return HdfsConnection::getListDirectory(heap, args[0], args[1]->getString());
}

ConstantSP hdfs_listDirectory(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: listDirectory(dirHandle) ");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != HDFS_FILE_DESC)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "dirHandle should be a dirHandle.");
    return HdfsFileInfo::listDirectory(args[0]);
}

ConstantSP hdfs_freeFileInfo(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: freeFileInfo(dirHandle) ");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != HDFS_FILE_DESC)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "dirHandle should be a dirHandle.");
    HdfsFileInfo::freeFileInfo(args[0]);
    return new Void();
}

ConstantSP hdfs_readFile(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: readFile(conn, path, handler) ");
    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != HDFS_CONN_DESC)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "conn should be a hdfsFS conn handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "path must be a string scalar.");
    if (args[2]->getType() != DT_FUNCTIONDEF)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "handler should be a function.");

    return HdfsConnection::readFile(heap, args[0], args[1]->getString(), args[2]);
}

ConstantSP hdfs_writeFile(Heap *heap, vector<ConstantSP> &args)
{
    const auto usage = string("Usage: writeFile(conn, path, table, handler) ");

    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != HDFS_CONN_DESC)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "conn should be a hdfsFS conn handle.");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "path must be a string scalar.");
    if (!args[2]->isTable())
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "table must be a table");
    if (args[3]->getType() != DT_FUNCTIONDEF)
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HDFS_PREFIX + usage + "handler should be a function.");
    HdfsConnection::writeFile(heap, args[0], args[1]->getString(), args[2], args[3]);
    return new Void();
}