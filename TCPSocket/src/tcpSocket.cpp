#include "tcpSocket.h"

class SocketWrapper{
public:
    SocketWrapper(const SocketWrapper& wrapper) = delete;
    SocketWrapper& operator=(const SocketWrapper& wrapper) = delete;
    SocketWrapper(const SocketSP& socket){
        socket_ = socket;
    }
    ConstantSP read(int maxLength){
        LockGuard<Mutex> _(&lock_);
        char dataBuffer[10240];
        vector<char> dataBufferVec;
        char* dataPtr = dataBuffer;
        if(maxLength > 10240){
            dataBufferVec.resize(maxLength);
            dataPtr = dataBufferVec.data();
        }
        size_t readLength;
        IO_ERR ret = socket_->read(dataPtr, maxLength, readLength);
        if(ret != OK){
            throw RuntimeException(PLUGIN_TCP_PREFIX + "failed to read from socket with IO error type " + std::to_string(ret));
        }
        return new String(DolphinString(dataPtr, readLength), true);
    }
    void write(const string& data){
        LockGuard<Mutex> _(&lock_);
        const char* dataPtr = data.c_str();
        int totalSize = data.size();
        int offset = 0;
        size_t actualSize;
        while(offset < totalSize){
            IO_ERR ret = socket_->write(dataPtr + offset, totalSize - offset, actualSize);
            if(ret != OK){
                throw RuntimeException(PLUGIN_TCP_PREFIX + "failed to write to socket with IO error type " + std::to_string(ret));
            }
            offset += actualSize;
        }
    }
    void close(){
        LockGuard<Mutex> _(&lock_);
        IO_ERR ret = socket_->close();
        if(ret != OK){
            throw RuntimeException(PLUGIN_TCP_PREFIX + "failed to close the socket with IO error type " + std::to_string(ret));
        }
    }
private:
    Mutex lock_;
    SocketSP socket_;
};
typedef SmartPointer<SocketWrapper> SocketWrapperSP;
dolphindb::ResourceMap<SocketWrapper> TCP_SOCKET_MAP(PLUGIN_TCP_PREFIX, "tcp socket");

static void tcpOnClose(Heap *heap, vector<ConstantSP> &args){
    TCP_SOCKET_MAP.safeRemoveWithoutException(args[0]);
}

ConstantSP tcpConnect(Heap *heap, vector<ConstantSP> &args){
    if(args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR)
        throw RuntimeException(PLUGIN_TCP_PREFIX + "ip must be a string scalar. ");
    if(args[1]->getType() != DT_INT || args[1]->getForm() != DF_SCALAR)
        throw RuntimeException(PLUGIN_TCP_PREFIX + "port must be a int scalar. ");
    string ip = args[0]->getString();
    int port = args[1]->getInt();
    SocketSP socket = new Socket(ip, port, true);
    IO_ERR ret = socket->connect();
    if(ret != OK){
        throw RuntimeException(PLUGIN_TCP_PREFIX + "failed to connect from socket with IO error type " + std::to_string(ret));
    }
    SmartPointer<SocketWrapper> wrapper = new SocketWrapper(socket);
    FunctionDefSP onClose(Util::createSystemProcedure("tcp onClose()", tcpOnClose, 1, 1));
    ConstantSP handle = Util::createResource(reinterpret_cast<long long>(wrapper.get()), "tcp socket connect to " + ip + ":" + std::to_string(port) + 
       + " " + Uuid(true).getString(), onClose, heap->currentSession());
    TCP_SOCKET_MAP.safeAdd(handle, wrapper);
    return handle;
}


ConstantSP tcpRead(Heap *heap, vector<ConstantSP> &args){
    int size = 10240;
    if(args.size() > 1){
        if(args[1]->getType() != DT_INT || args[1]->getForm() != DF_SCALAR)
            throw RuntimeException(PLUGIN_TCP_PREFIX + "size must be a int scalar. ");
        size = args[1]->getInt();
    }
    SocketWrapperSP wrapper = TCP_SOCKET_MAP.safeGet(args[0]);
    return wrapper->read(size);
}

ConstantSP tcpWrite(Heap *heap, vector<ConstantSP> &args){
    if((args[1]->getType() != DT_STRING && args[1]->getType() != DT_BLOB) || args[1]->getForm() != DF_SCALAR)
        throw RuntimeException(PLUGIN_TCP_PREFIX + "data must be a blob scalar or a string scalar. ");
    SocketWrapperSP wrapper = TCP_SOCKET_MAP.safeGet(args[0]);
    string data = args[1]->getString();
    wrapper->write(data);
    return new Bool(true);
}

ConstantSP tcpClose(Heap *heap, vector<ConstantSP> &args){
    SocketWrapperSP wrapper = TCP_SOCKET_MAP.safeGet(args[0]);
    wrapper->close();
    TCP_SOCKET_MAP.safeRemove(args[0]);
    return new Bool(true);
}
