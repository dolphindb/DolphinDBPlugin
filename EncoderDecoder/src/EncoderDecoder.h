#ifndef ENCODERdECODER_H
#define ENCODERdECODER_H

#include <CoreConcept.h>
#include <Exceptions.h>
#include <Logger.h>
#include <ScalarImp.h>
#include <SysIO.h>
#include <Types.h>
#include <Util.h>
#include <ddbplugin/PluginLogger.h>
#include <iostream>
#include <string>

using ddb::ConstantSP;
using ddb::Heap;
using std::vector;

using ddb::FunctionDefSP;
using ddb::INDEX;
using ddb::TableSP;

extern "C" ConstantSP createJsonDecoder(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP createProtobufDecoder(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP getProtobufSchema(Heap* heap, vector<ConstantSP>& arguments);

namespace ddb {

static string ENCODERDECODER_PREFIX = "[PLUGIN::ENCODERDECODER] ";
class Defer {
public:
    Defer(std::function<void()> code): code(code) {}
    ~Defer() {code(); }
private:
    std::function<void()> code;
};

template <class U>
struct ObjectSizer {
    inline int operator()(const U& obj)
    {
        return 1;
    }
};

template <class U>
struct ObjectUrgency {
    inline bool operator()(const U& obj)
    {
        return false;
    }
};
class GetDataRunnable;
using GetDataRunnableSP = SmartPointer<GetDataRunnable>;
// TODO change to boundBlockingQueue
using GenericBoundedQueueSP = SmartPointer<GenericBoundedQueue<ConstantSP, ObjectSizer<ConstantSP>, ObjectUrgency<ConstantSP>>>;
static int CAPACITY = 1000000;
class BatchProcessor {
public:
    inline BatchProcessor(SessionSP session, ConstantSP handler,
        TableSP dummyTable, FunctionDefSP parser, int workerNum, int batchSize, int timeout);
    inline ~BatchProcessor();
    inline void add(ConstantSP item);
    inline void setParserArgs(VectorSP parserArgs);
    inline void flushTable();

private:
    SessionSP session_;
    ConstantSP handler_;
    FunctionDefSP parser_;
    int workerNum_;
    int batchSize_;
    int timeout_;
    GenericBoundedQueueSP queue_;
    TableSP bufferVec_;
    ThreadSP thread_;
    GetDataRunnableSP runnable_;
};

class GetDataRunnable : public Runnable {
public:
    GetDataRunnable(SessionSP session, BatchProcessor* batchProcessor,
        TableSP bufferVec, GenericBoundedQueueSP queue, int batchSize, int timeout)
        : batchProcessor_(batchProcessor)
        , bufferVec_(bufferVec)
        , queue_(queue)
        , batchSize_(batchSize)
        , timeout_(timeout)
    {
        session_ = session->copy();
        session_->setUser(session->getUser());
    }

    void run()
    {
        try {
            auto lastFlushTime = std::chrono::system_clock::now();
            int gap = 0;
            while (flag_) {
                auto currentTime = std::chrono::system_clock::now();
                gap = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastFlushTime).count();
                try {
                    if (bufferVec_->size() >= batchSize_ || (gap >= timeout_)) {
                        batchProcessor_->flushTable();
                        lastFlushTime = std::chrono::system_clock::now();
                    }
                    ConstantSP item;
                    if (queue_->blockingPop(item, timeout_)) {
                        vector<ConstantSP> values;
                        string errMsg = "";
                        int cols = item->columns();
                        for (int i = 0; i < cols; i++) {
                            values.emplace_back(item->getColumn(i));
                        }
                        INDEX index = bufferVec_->rows();
                        bufferVec_->append(values, index, errMsg);
                        if (errMsg != "") {
                            PLUGIN_LOG_ERR(ENCODERDECODER_PREFIX + " async append: " + errMsg);
                        }
                    }
                } catch (MemoryException& me) {
                    PLUGIN_LOG_ERR(ENCODERDECODER_PREFIX + "out of memory");
                    return;
                } catch (std::bad_alloc& me) {
                    PLUGIN_LOG_ERR(ENCODERDECODER_PREFIX + "out of memory");
                    return;
                } catch (std::exception& e) {
                    PLUGIN_LOG_ERR(ENCODERDECODER_PREFIX + " async append: " + string(e.what()));
                    continue;
                }
            }
            batchProcessor_->flushTable();

        } catch (MemoryException& me) {
            PLUGIN_LOG_ERR(ENCODERDECODER_PREFIX + "out of memory");
            return;
        } catch (std::bad_alloc& me) {
            PLUGIN_LOG_ERR(ENCODERDECODER_PREFIX + "out of memory");
            return;
        } catch(...) {
            PLUGIN_LOG_ERR(ENCODERDECODER_PREFIX + "Async flush thread failed.");
        }
    }

    void setFlag(bool flag)
    {
        flag_ = flag;
    }

private:
    SessionSP session_;
    BatchProcessor* batchProcessor_;
    TableSP bufferVec_;
    GenericBoundedQueueSP queue_;
    int batchSize_;
    int timeout_;
    volatile bool flag_ = true;
};

inline BatchProcessor::BatchProcessor(SessionSP session, ConstantSP handler,
    TableSP dummyTable, FunctionDefSP parser, int workerNum, int batchSize, int timeout)
    : session_(session)
    , handler_(handler)
    , parser_(parser)
    , workerNum_(workerNum)
    , batchSize_(batchSize)
    , timeout_(timeout)
{
    assert(handler->isTable() || handler->getType() == DT_FUNCTIONDEF || handler->getType() == DT_RESOURCE);
    queue_ = new GenericBoundedQueue<ConstantSP, ObjectSizer<ConstantSP>, ObjectUrgency<ConstantSP>>(
        CAPACITY, ObjectSizer<ConstantSP>(), ObjectUrgency<ConstantSP>());
    vector<string> colNames;
    vector<DATA_TYPE> colTypes;
    int cols = dummyTable->columns();
    for (int i = 0; i < cols; ++i) {
        colNames.push_back(dummyTable->getColumnName(i));
        colTypes.push_back(dummyTable->getColumnType(i));
    }
    bufferVec_ = Util::createTable(colNames, colTypes, 0, 1);
    runnable_ = new GetDataRunnable(session_, this, bufferVec_, queue_, batchSize_, timeout_);
    thread_ = new Thread(runnable_);
    thread_->start();
}

inline BatchProcessor::~BatchProcessor()
{
    runnable_->setFlag(false);
    thread_->join();
}

inline void BatchProcessor::add(ConstantSP item)
{
    queue_->push(item);
}

inline void BatchProcessor::flushTable()
{
    if (bufferVec_->size() == 0) {
        return;
    }
    ConstantSP cutSize = Util::createConstant(DT_INT);
    try {
        // cut table manually
        // if bufferVec length < cut:
        //   worker num = 1
        // else if length > cut:
        //   cut every col respectively, and combine them into PLoop args
        ConstantSP splitVec;
        vector<ConstantSP> argsCut;
        auto bufferValue = bufferVec_->values();

        if (bufferVec_->rows() <= workerNum_ || workerNum_ == 1) {
            splitVec = Util::createVector(DT_ANY, 1);
            splitVec->set(0, bufferVec_);
        } else {
            int size = bufferVec_->rows() / workerNum_;
            if (size * workerNum_ < bufferVec_->rows()) {
                size += 1;
            }
            cutSize->setInt(size);
            vector<ConstantSP> cutList(bufferValue->size());
            vector<string> names;
            for (int i = 0; i < bufferValue->size(); ++i) {
                names.push_back(bufferVec_->getColumnName(i));
            }

            for (int i = 0; i < bufferValue->size(); ++i) {
                argsCut = vector<ConstantSP> { bufferValue->get(i), cutSize };
                cutList[i] = session_->getFunctionDef("cut")->call(session_->getHeap().get(), argsCut);
            }

            splitVec = Util::createVector(DT_ANY, cutList[0]->size());

            for (int i = 0; i < cutList[0]->size(); ++i) {
                vector<ConstantSP> cols;
                for (int j = 0; j < bufferValue->size(); ++j) {
                    cols.push_back(cutList[j]->get(i));
                    cols[cols.size()-1]->setTemporary(true);
                }
                auto tmp = Util::createTable(names, cols);
                splitVec->set(i, tmp);
            }
        }

        vector<ConstantSP> argsPloop = { parser_, splitVec };
        ConstantSP parserResult = session_->getFunctionDef("ploop")->call(session_->getHeap().get(), argsPloop);

        if (handler_->isTable()) {
            for (int i = 0; i < parserResult->size(); ++i) {
                vector<ConstantSP> argsAppend = { handler_, parserResult->get(i) };
                session_->getFunctionDef("append!")->call(session_->getHeap().get(), argsAppend);
            }
        } else if (handler_->getType() == DT_FUNCTIONDEF) {
            for (int i = 0; i < parserResult->size(); ++i) {
                vector<ConstantSP> args = { parserResult->get(i) };
                ((FunctionDefSP)handler_)->call(session_->getHeap().get(), args);
            }
        } else if (handler_->getForm() == DF_SYSOBJ) {
            string name = "parseAndHandle";
            Heap* heap = session_->getHeap().get();
            if (handler_->hasMethod(name)) {
                FunctionDefSP func = handler_->getMethod(name);
                for (int i = 0; i < parserResult->size(); ++i) {
                    vector<ConstantSP> args = {handler_, parserResult->get(i)};
                    func->call(heap, args);
                }
            }
            throw RuntimeException("invalid coder handle.");
        }
    } catch (MemoryException& me) {
        throw me;
    } catch (std::bad_alloc& me) {
        throw me;
    } catch (std::exception& e) {
        PLUGIN_LOG_ERR(ENCODERDECODER_PREFIX, " async flush: " + string(e.what()));
    }
    bufferVec_->clear();
}

}
#endif