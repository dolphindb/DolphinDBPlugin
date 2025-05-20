#include "HdfsWrapper.h"
#include "ddbplugin/Plugin.h"

namespace hdfsPlugin {

static void mockOnClose(Heap *heap, vector<ConstantSP> &args) {}

string getErrorMsgWithPrefix(const string &operation) {
    string errMsg = PLUGIN_HDFS_PREFIX + operation + " failed";
    if (errno != 0) {
        string reason = strerror(errno);
        errMsg += " due to " + reason;
    }
    return errMsg;
}

template <typename T>
static void hdfsOnClose(Heap *heap, vector<ConstantSP> &args) {
    T *ptr = (T *)args[0]->getLong();
    if (ptr != nullptr) {
        delete []ptr;
        args[0]->setLong(0);
    }
}

static short encodeMode(int mode) {
    if (mode < 0 || mode > 777) {
        throw RuntimeException(PLUGIN_HDFS_PREFIX + "Invalid mode \"" + std::to_string(mode) + "\".");
    }
    short m1 = mode % 10;
    mode /= 10;
    short m2 = mode % 10;
    mode /= 10;
    short m3 = mode % 10;
    if (m1 > 7 || m2 > 7 || m3 > 7) {
        throw RuntimeException(PLUGIN_HDFS_PREFIX + "Invalid mode \"" + std::to_string(m3) + std::to_string(m2) +
                               std::to_string(m1) + "\".");
    }
    return m3 * 64 + m2 * 8 + m1;
}

static short decodeMode(short mode) {
    if (mode < 0 || mode > 511) {
        PLUGIN_LOG_WARN(PLUGIN_HDFS_PREFIX + "Invalid mode \"" + std::to_string(mode) + "\".");
    }
    short m1 = mode % 8;
    mode /= 8;
    short m2 = mode % 8;
    mode /= 8;
    short m3 = mode % 8;

    return m3 * 100 + m2 * 10 + m1;
}

HdfsFileInfo::HdfsFileInfo(Heap *heap, hdfsFileInfo *info, int size)
    : Resource(0, HDFS_FILE_DESC, Util::createSystemProcedure("hdfs fileInfo onClose", mockOnClose, 1, 1),
               heap->currentSession()),
      size_(size),
      info_(info) {}

HdfsFileInfo::~HdfsFileInfo() {
    if (info_) {
        hdfsFreeFileInfo(info_, size_);
        info_ = nullptr;
    }
}

ConstantSP HdfsFileInfo::listDirectory(HdfsFileInfoSP info) {
    info->checkValid();
    ConstantSP kindVec = Util::createVector(DT_STRING, info->size_);
    ConstantSP nameVec = Util::createVector(DT_STRING, info->size_);
    ConstantSP lastModVec = Util::createVector(DT_DATETIME, info->size_);
    ConstantSP sizeVec = Util::createVector(DT_LONG, info->size_);
    ConstantSP repVec = Util::createVector(DT_SHORT, info->size_);
    ConstantSP blockSizeVec = Util::createVector(DT_LONG, info->size_);
    ConstantSP ownerVec = Util::createVector(DT_STRING, info->size_);
    ConstantSP groupVec = Util::createVector(DT_STRING, info->size_);
    ConstantSP permissionVec = Util::createVector(DT_SHORT, info->size_);
    ConstantSP lastAccessVec = Util::createVector(DT_DATETIME, info->size_);
    for (int i = 0; i < info->size_; i++) {
        kindVec->setString(i, info->info_[i].mKind == kObjectKindFile ? "F" : "D");
        nameVec->setString(i, info->info_[i].mName);
        lastModVec->setInt(i, info->info_[i].mLastMod);
        sizeVec->setLong(i, info->info_[i].mSize);
        repVec->setShort(i, info->info_[i].mReplication);
        blockSizeVec->setLong(i, info->info_[i].mBlockSize);
        ownerVec->setString(i, info->info_[i].mOwner);
        groupVec->setString(i, info->info_[i].mGroup);
        permissionVec->setShort(i, decodeMode(info->info_[i].mPermissions));
        lastAccessVec->setInt(i, info->info_[i].mLastAccess);
    }
    vector<string> colNames = {"mKind",      "mName",  "mLastMod", "mSize",        "mReplication",
                               "mBlockSize", "mOwner", "mGroup",   "mPermissions", "mLastAccess"};
    vector<ConstantSP> cols = {kindVec,      nameVec,  lastModVec, sizeVec,       repVec,
                               blockSizeVec, ownerVec, groupVec,   permissionVec, lastAccessVec};

    return Util::createTable(colNames, cols);
}

void HdfsFileInfo::freeFileInfo(HdfsFileInfoSP info) {
    info->checkValid();
    info->freeFileInfo();
}

void HdfsFileInfo::checkValid() {
    if (!info_) {
        throw RuntimeException(PLUGIN_HDFS_PREFIX + "hdfsFileInfo already freed.");
    }
}

void HdfsFileInfo::freeFileInfo() {
    if (info_) {
        hdfsFreeFileInfo(info_, size_);
        info_ = nullptr;
    }
}

HdfsConnection::HdfsConnection(Heap *heap, hdfs_internal *fs)
    : Resource(0, HDFS_CONN_DESC, Util::createSystemProcedure("hdfs conn onClose", mockOnClose, 1, 1),
               heap->currentSession()),
      fs_(fs) {}

HdfsConnection::~HdfsConnection() {
    if (fs_ && hdfsDisconnect(fs_) == -1) {
        PLUGIN_LOG_WARN(getErrorMsgWithPrefix("hdfs conn destruction"));
    }
}

void HdfsConnection::disconnect(HdfsConnectionSP conn) {
    LockGuard<Mutex> lock(&conn->mutex_);
    conn->checkValid();
    if (hdfsDisconnect(conn->fs_) == -1) {
        conn->fs_ = 0;
        throw RuntimeException(getErrorMsgWithPrefix("disconnecting"));
    }
    conn->fs_ = 0;
}

void HdfsConnection::exists(HdfsConnectionSP conn, const string &path) {
    LockGuard<Mutex> lock(&conn->mutex_);
    conn->checkValid();
    if (hdfsExists(conn->fs_, path.c_str()) == -1) {
        throw RuntimeException(getErrorMsgWithPrefix("checking existence"));
    }
}

void HdfsConnection::copy(HdfsConnectionSP srcConn, const string &src, HdfsConnectionSP dstConn, const string &dst) {
    auto locks = tryLock(srcConn, dstConn);
    srcConn->checkValid();
    dstConn->checkValid();
    if (hdfsCopy(srcConn->fs_, src.c_str(), dstConn->fs_, dst.c_str()) == -1) {
        throw RuntimeException(getErrorMsgWithPrefix("copying"));
    }
}

void HdfsConnection::move(HdfsConnectionSP srcConn, const string &src, HdfsConnectionSP dstConn, const string &dst) {
    auto locks = tryLock(srcConn, dstConn);
    srcConn->checkValid();
    dstConn->checkValid();
    if (hdfsMove(srcConn->fs_, src.c_str(), dstConn->fs_, dst.c_str()) == -1) {
        throw RuntimeException(getErrorMsgWithPrefix("moving"));
    }
}

void HdfsConnection::deletePath(HdfsConnectionSP conn, const string &path, int recursive) {
    LockGuard<Mutex> lock(&conn->mutex_);
    conn->checkValid();
    if (hdfsDelete(conn->fs_, path.c_str(), recursive) == -1) {
        throw RuntimeException(getErrorMsgWithPrefix("deleting"));
    }
}

void HdfsConnection::rename(HdfsConnectionSP conn, const string &oldPath, const string &newPath) {
    LockGuard<Mutex> lock(&conn->mutex_);
    conn->checkValid();
    if (hdfsRename(conn->fs_, oldPath.c_str(), newPath.c_str()) == -1) {
        throw RuntimeException(getErrorMsgWithPrefix("renaming"));
    }
}

void HdfsConnection::createDirectory(HdfsConnectionSP conn, const string &path) {
    LockGuard<Mutex> lock(&conn->mutex_);
    conn->checkValid();
    if (conn->fs_ && hdfsCreateDirectory(conn->fs_, path.c_str()) == -1) {
        throw RuntimeException(getErrorMsgWithPrefix("creating directory"));
    }
}

void HdfsConnection::chmod(HdfsConnectionSP conn, const string &path, int mode) {
    LockGuard<Mutex> lock(&conn->mutex_);
    conn->checkValid();
    if (hdfsChmod(conn->fs_, path.c_str(), encodeMode(mode)) == -1) {
        throw RuntimeException(getErrorMsgWithPrefix("changing ownership"));
    }
}

HdfsFileInfoSP HdfsConnection::getListDirectory(Heap *heap, HdfsConnectionSP conn, const string &path) {
    LockGuard<Mutex> lock(&conn->mutex_);
    conn->checkValid();
    int size = 0;
    auto info = hdfsListDirectory(conn->fs_, path.c_str(), &size);
    if (info == nullptr) {
        throw RuntimeException(getErrorMsgWithPrefix("getting directory information"));
    }
    return new HdfsFileInfo(heap, info, size);
}

ConstantSP HdfsConnection::readFile(Heap *heap, HdfsConnectionSP conn, const string &path, FunctionDefSP handler) {
    LockGuard<Mutex> lock(&conn->mutex_);
    conn->checkValid();
    hdfsFile pFile = hdfsOpenFile(conn->fs_, path.c_str(), O_RDONLY, 0, 0, 0);
    if (pFile == nullptr) {
        throw RuntimeException(getErrorMsgWithPrefix("opening file"));
    }
    ddb::PluginDefer defer([&](){
        if (hdfsCloseFile(conn->fs_, pFile) == -1) {
            PLUGIN_LOG_WARN(getErrorMsgWithPrefix("closing file handle"));
        }
    });
    int size = 0;
    auto info = hdfsListDirectory(conn->fs_, path.c_str(), &size);
    if (info == nullptr || size != 1) {
        throw RuntimeException(getErrorMsgWithPrefix("getting file information"));
    }

    int64_t mSize = info[0].mSize;
    int64_t *bufSize = new int64_t[1];
    bufSize[0] = mSize;
    char *buffer = new char[mSize];

    int readBytes;
    int64_t readOffset = 0;
    while (readOffset < mSize) {
        readBytes = hdfsPread(conn->fs_, pFile, 0, buffer + readOffset, mSize);
        readOffset += readBytes;
        if (readBytes == -1) {
            throw RuntimeException(getErrorMsgWithPrefix("reading file data"));
        }
    }
    FunctionDefSP onCloseChar(Util::createSystemProcedure("hdfs readFile onClose()", hdfsOnClose<char>, 1, 1));
    FunctionDefSP onCloseLong(Util::createSystemProcedure("hdfs readFile onClose()", hdfsOnClose<int64_t>, 1, 1));
    vector<ConstantSP> handlerArgs(2);
    handlerArgs[0] =
        Util::createResource((long long)buffer, "hdfs readFile address", onCloseChar, heap);
    handlerArgs[1] =
        Util::createResource((long long)bufSize, "hdfs readFile length", onCloseLong, heap);
    return handler->call(heap, handlerArgs);
}

void HdfsConnection::writeFile(Heap *heap, HdfsConnectionSP conn, const string &path, TableSP table,
                               FunctionDefSP handler) {
    LockGuard<Mutex> lock(&conn->mutex_);
    conn->checkValid();
    hdfsFile pFile = hdfsOpenFile(conn->fs_, path.c_str(), O_WRONLY | O_CREAT, 0, 0, 0);
    if (pFile == nullptr) {
        throw RuntimeException(getErrorMsgWithPrefix("opening file"));
    }
    ddb::PluginDefer defer([&](){
        if (hdfsCloseFile(conn->fs_, pFile) == -1) {
            PLUGIN_LOG_WARN(getErrorMsgWithPrefix("closing file handle"));
        }
    });
    vector<ConstantSP> handlerArgs(1);
    handlerArgs[0] = table;
    VectorSP vsp = handler->call(heap, handlerArgs);
    if (vsp->size() != 2 || !vsp->isNumber() || vsp->hasNull()) {
        throw RuntimeException(PLUGIN_HDFS_PREFIX + "invalid return value of handler");
    }
    void *buffer = (void *)vsp->getLong(0);
    int64_t length = ((int64_t *)vsp->getLong(1))[0];
    tSize writeRes = hdfsWrite(conn->fs_, pFile, buffer, length);
    if (hdfsFlush(conn->fs_, pFile)) {
        throw RuntimeException(getErrorMsgWithPrefix("flushing writeFile"));
    }
    if (writeRes == -1) {
        throw RuntimeException(PLUGIN_HDFS_PREFIX + "error occurred while writing data to hdfs");
    }
}

void HdfsConnection::checkValid() {
    if (!fs_) {
        throw RuntimeException(PLUGIN_HDFS_PREFIX + "hdfs connection already disconnected.");
    }
}

vector<TryLockGuard<Mutex>> HdfsConnection::tryLock(HdfsConnectionSP srcConn, HdfsConnectionSP dstConn) {
    while (true) {
        TryLockGuard<Mutex> srcLock(&srcConn->mutex_);
        if (!srcLock.isLocked()) {
            continue;
        }
        TryLockGuard<Mutex> dstLock(&dstConn->mutex_);
        if (!dstLock.isLocked()) {
            continue;
        }
        return {std::move(srcLock), std::move(dstLock)};
    }
}

}  // namespace hdfsPlugin