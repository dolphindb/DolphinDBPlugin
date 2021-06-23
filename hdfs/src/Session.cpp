//
// Created by ypfan on 2020/12/9.
//

#include "Session.h"

GetData::GetData(Heap *heap, hdfsFS file, std::queue<MyBuffer>* queue, SubConnection* connection)
        :heap_(heap),fs_(file),queue_(queue),connection_(connection){
    session_ = heap->currentSession()->copy();
    session_->setUser(heap->currentSession()->getUser());
    session_->setOutput(new DummyOutput);
}

void GetData::run() {
    hdfsFile file = hdfsOpenFile(fs_,connection_->getPath().c_str(),O_RDONLY,connection_->getBufferSize(),connection_->getReplication(),connection_->getBlockSize());
    if(file == nullptr) {
        LOG_ERR("Error occurred when open the file " + connection_->getPath());
        return;
    }
    MyBuffer buffer = new std::string(connection_->getBufferSize()+6,'0');
    tOffset position = 0;
    tSize size = hdfsPread(fs_,file,position,(void*)(buffer->data()+5),connection_->getBufferSize());

    while(size!=0){
        *(int*)buffer->data() = size;
        (*buffer)[4] = MESSAGE_BODY;
        (*buffer)[size+5] = '\0';
        queue_->push(buffer);
        position+=size;
        buffer = new std::string(connection_->getBufferSize(),'0');
        size = hdfsPread(fs_,file,position,(void*)buffer->data(),connection_->getBufferSize());
    }

    *(int*)buffer->data() = 0;
    (*buffer)[4] = MESSAGE_END;
    queue_->push(buffer);

    if(hdfsCloseFile(fs_,file) == -1)
        LOG_ERR("Error occurred when closing the file");
}

void SubConnection::setBufferSize(int size) {
    bufferSize_ = size;
}

GetDataLine::GetDataLine(Heap *heap, hdfsFS file, std::queue<MyBuffer> *queue, SubConnection *connection, int length)
        :heap_(heap),fs_(file),queue_(queue),connection_(connection),length_(length){
    session_ = heap->currentSession()->copy();
    session_->setUser(heap->currentSession()->getUser());
    session_->setOutput(new DummyOutput);
}

void GetDataLine::run() {
    hdfsFile file = hdfsOpenFile(fs_,connection_->getPath().c_str(),O_RDONLY,connection_->getBufferSize(),connection_->getReplication(),connection_->getBlockSize());
    if(file == nullptr) {
        LOG_ERR("Error occurred when open the file " + connection_->getPath());
        return;
    }
    auto buffer = new string(connection_->getBufferSize(),'0');
    tOffset position = 0;
    tSize size = hdfsPread(fs_,file,position,(void*)(buffer->data()),connection_->getBufferSize());

    MyBuffer bufferPass;
    int ptr;
    int count = 0;
    int bufferPosition = 5;

    while(size!=0){
        position+=size;
        ptr = 0;
        bufferPass = new std::string(connection_->getBufferSize(), '0');
        while(ptr!=size) {
            while(count != length_){
                if(ptr == size)
                    break;
                if((*buffer)[ptr] == '\n')
                    count++;
                (*bufferPass)[bufferPosition++] = (*buffer)[ptr];
                ptr++;
            }
            if(count == length_){
                *(int *) bufferPass->data() = bufferPosition-5;
                (*bufferPass)[4] = MESSAGE_BODY;
                (*bufferPass)[bufferPosition] = '\0';
                queue_->push(bufferPass);
                bufferPass = new std::string(connection_->getBufferSize(), '0');
                count = 0;
                bufferPosition = 5;
            }
        }
        size = hdfsPread(fs_,file,position,(void*)buffer->data(),connection_->getBufferSize());
    }

    *(int*)bufferPass->data() = 0;
    (*bufferPass)[4] = MESSAGE_END;
    queue_->push(bufferPass);

    delete buffer;
    if(hdfsCloseFile(fs_,file) == -1)
        LOG_ERR("Error occurred when closing the file");
}

DealData::DealData(Heap *heap, FunctionDefSP handle, std::queue<MyBuffer>* queue)
        :heap_(heap),handle_(handle),queue_(queue){
    session_ = heap->currentSession()->copy();
    session_->setUser(heap->currentSession()->getUser());
    session_->setOutput(new DummyOutput);
}

void DealData::run() {
    while(true){
        if(!queue_->empty()){
            auto buffer = queue_->front();
            queue_->pop();
            if((*buffer)[4] == MESSAGE_END)
                break;
            auto stringP = Util::createConstant(DT_STRING);
            auto str = buffer->data()+5;
            stringP->setString(string(str));
            vector<ConstantSP> args = {stringP};
            handle_->call(heap_, args);
        }
    }
}

SubConnection::SubConnection(Heap* heap, hdfsFS file, FunctionDefSP handler, string  path, int bufferSize, int replication, long blockSize, int length)
        :file_(file),handler_(handler), heap_(heap),path_(std::move(path)),bufferSize_(bufferSize),replication_(replication),blockSize_(blockSize){
    session_ = heap->currentSession()->copy();
    session_->setUser(heap->currentSession()->getUser());
    queue_ = new std::queue<MyBuffer>();
    if(bufferSize==0)
        bufferSize_ = 1000;

    if(length == 0) {
        SmartPointer<GetData> getData = new GetData(heap_, file_, queue_, this);
        getThread_ = new Thread(getData);
    }
    else {
        SmartPointer<GetDataLine> getData = new GetDataLine(heap_, file_, queue_, this, length);
        getThread_ = new Thread(getData);
    }
    SmartPointer<DealData> dealData = new DealData(heap_,handler_,queue_);
    dealThread_ = new Thread(dealData);
    getThread_->start();
    dealThread_->start();
}