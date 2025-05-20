#ifndef HDFS_WRAPPER_H
#define HDFS_WRAPPER_H

#include "CoreConcept.h"
#include "ScalarImp.h"
#include "ddbplugin/PluginLogger.h"
#include "hdfs.h"

namespace hdfsPlugin {

static const string HDFS_CONN_DESC = "hdfsFS connection";
static const string HDFS_FILE_DESC = "FileInfo connection";
static const string PLUGIN_HDFS_PREFIX = "[PLUGIN::HDFS]: ";

string getErrorMsgWithPrefix(const string &operation);

class HdfsFileInfo;
using HdfsFileInfoSP = SmartPointer<HdfsFileInfo>;
class HdfsConnection;
using HdfsConnectionSP = SmartPointer<HdfsConnection>;

class HdfsFileInfo : public Resource {
  public:
    HdfsFileInfo(Heap *heap, hdfsFileInfo *info, int size);
    ~HdfsFileInfo();

    static ConstantSP listDirectory(HdfsFileInfoSP info);
    static void freeFileInfo(HdfsFileInfoSP info);

  private:
    void checkValid();
    void freeFileInfo();

  private:
    int size_ = 0;
    hdfsFileInfo *info_;
    Mutex mutex_;
};

class HdfsConnection : public Resource {
  public:
    HdfsConnection(Heap *heap, hdfs_internal *fs);
    ~HdfsConnection();

    static void disconnect(HdfsConnectionSP conn);
    static void exists(HdfsConnectionSP conn, const string &path);
    static void copy(HdfsConnectionSP srcConn, const string &src, HdfsConnectionSP dstConn, const string &dst);
    static void move(HdfsConnectionSP srcConn, const string &src, HdfsConnectionSP dstConn, const string &dst);
    static void deletePath(HdfsConnectionSP conn, const string &path, int recursive);
    static void rename(HdfsConnectionSP conn, const string &oldPath, const string &newPath);
    static void createDirectory(HdfsConnectionSP conn, const string &path);
    static void chmod(HdfsConnectionSP conn, const string &path, int mode);
    static HdfsFileInfoSP getListDirectory(Heap *heap, HdfsConnectionSP conn, const string &path);
    static ConstantSP readFile(Heap *heap, HdfsConnectionSP conn, const string &path, FunctionDefSP handler);
    static void writeFile(Heap *heap, HdfsConnectionSP conn, const string &path, TableSP table, FunctionDefSP handler);

  private:
    void checkValid();
    static vector<TryLockGuard<Mutex>> tryLock(HdfsConnectionSP srcConn, HdfsConnectionSP dstConn);

  private:
    Mutex mutex_;
    hdfs_internal *fs_;
};

}  // namespace hdfsPlugin

#endif