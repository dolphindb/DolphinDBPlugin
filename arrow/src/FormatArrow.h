#ifndef ARROW_FORMAT_H
#define ARROW_FORMAT_H

#include <map>

#include "ddbplugin/CommonInterface.h"
#include <CoreConcept.h>
#include <ConstantMarshal.h>
#include <Exceptions.h>
#include <FlatHashmap.h>
#include <ScalarImp.h>
#include <SysIO.h>
#include <Util.h>
#include <Types.h>

#include "arrow/api.h"
#include "arrow/ipc/api.h"
#include "arrow/io/api.h"

extern "C" ConstantSP getSupportedFormats(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP getConstantMarshal(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP getConstantUnmarshal(Heap *heap, vector<ConstantSP> &args);

enum DATA_FORMAT {
    FORMAT_ARROW=2
};

class ArrowTableMarshall: public ConstantMarshalImp {
public:
    ArrowTableMarshall(const DataOutputStreamSP& out)
        : ConstantMarshalImp(out), isSchemaSent_(false), isDictionarySent_(false), isRecorderSent_(false), isEndSent_(false)
        , schema_(nullptr), stream_(nullptr), writer_(nullptr), rowsSent_(0), dictionariesSent_(0) {}
	virtual ~ArrowTableMarshall() {}

	virtual bool start(const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual bool start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret);
	virtual bool resume(IO_ERR& ret);
	virtual void reset() { target_.clear(); }

private:
    void buildSchema(const TableSP& table);
    void buildDictIdsMap(const TableSP& table);
    void buildWriter();

    IO_ERR sendDictionary();
    IO_ERR sendSchema();
    IO_ERR sendRecordBatch();
    IO_ERR sendEnd();

private:
    bool isSchemaSent_;
    bool isDictionarySent_;
    bool isRecorderSent_;
    bool isEndSent_;
    std::shared_ptr<arrow::Schema> schema_;
    std::shared_ptr<arrow::io::BufferOutputStream> stream_;
    std::shared_ptr<arrow::ipc::internal::IpcPayloadWriter> writer_;
    INDEX rowsSent_;
    std::map<int, int> dictIdsMap_;                                 //key->dictionary Id, value->the index of symbol column
    int dictionariesSent_;
};



#endif      // ARROW_FORMAT_H