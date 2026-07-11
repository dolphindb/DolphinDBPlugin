// SPDX-License-Identifier: Apache-2.0
// Copyright © 2025-2025 DolphinDB, Inc.
#pragma once

#include "DolphinDBEverything.h"
#include "ddbplugin/CommonInterface.h"

#include <ConstantMarshal.h>
#include <CoreConcept.h>
#include <Exceptions.h>
#include <FlatHashmap.h>
#include <ScalarImp.h>
#include <SysIO.h>
#include <Types.h>
#include <Util.h>

#include <map>
#include <deque>
#include <string>
#include <utility>
#include <vector>

#if defined(_MSC_VER)
#pragma warning(push)
#elif defined(__clang__)
#pragma clang diagnostic push
// Too many to fix
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wredundant-move"
#else  // gcc
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#if __GNUC__ >= 9
#pragma GCC diagnostic ignored "-Wredundant-move"
#endif
#endif

#include "arrow/api.h"
#include "arrow/io/api.h"
#include "arrow/ipc/api.h"
#include "arrow/util/config.h"

#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__clang__)
#pragma clang diagnostic pop
#else  // gcc
#pragma GCC diagnostic pop
#endif

namespace ddb {

extern "C" ConstantSP getSupportedFormats(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP getConstantMarshal(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP getConstantUnmarshal(Heap *heap,
                                           vector<ConstantSP> &args);

enum DATA_FORMAT {
    FORMAT_ARROW = 2,  // 0 1 0
    FORMAT_ARROW_RETURN_LIMIT = 6,  // 0 1 1
};

class ArrowTableMarshall : public ConstantMarshalImp {
  public:
    explicit ArrowTableMarshall(const DataOutputStreamSP &out)
        : ConstantMarshalImp(out),
          schema_(nullptr),
          stream_(nullptr),
          writer_(nullptr),
          rowsSent_(0),
          dictionariesSent_(0) {}
    ~ArrowTableMarshall() override = default;

    bool start(const ConstantSP &target, bool blocking, IO_ERR &ret) override;
    bool start(const char *requestHeader, size_t headerSize,
               const ConstantSP &target, bool blocking, IO_ERR &ret) override;
    bool resume(IO_ERR &ret) override;
    void reset() override { target_.clear(); }

  private:
    void buildSchema(const TableSP &table);
    void buildDictIdsMap(const TableSP &table);
    void buildWriter();

    IO_ERR writeArrowPayload(const std::shared_ptr<arrow::Buffer> &buf);
    IO_ERR sendDictionary();
    IO_ERR sendSchema();
    IO_ERR sendRecordBatch();
    IO_ERR sendEnd();

    bool isSchemaSent_ = false;
    bool isDictionarySent_ = false;
    bool isRecorderSent_ = false;
    bool isEndSent_ = false;
    std::shared_ptr<arrow::Schema> schema_;
    std::shared_ptr<arrow::io::BufferOutputStream> stream_;
    std::shared_ptr<arrow::ipc::internal::IpcPayloadWriter> writer_;
    std::shared_ptr<arrow::Buffer> pendingPayload_;
    INDEX rowsSent_ = -1;
    std::map<int, int>
        dictIdsMap_;  // key->dictionary Id, value->the index of symbol column
    int dictionariesSent_ = -1;
    unsigned long long endMarker_ = 0xFFFFFFFF00000000ULL;
};

class ArrowTableUnmarshall : public ConstantUnmarshalImp {
  public:
    ArrowTableUnmarshall(const DataInputStreamSP &in, Session *session);
    ~ArrowTableUnmarshall() override = default;

    bool start(short flag, bool blocking, IO_ERR &ret) override;
    bool resume(IO_ERR &ret) override;
    void reset() override;

  private:
    struct ColumnTypeInfo {
        DATA_TYPE type = DT_VOID;
        int extra = 0;
    };

    bool consume(IO_ERR &ret);
    void ensureSchemaInitialized(const std::shared_ptr<arrow::Schema> &schema,
                                 INDEX initialCapacity = 0);
    void appendRecordBatch(const std::shared_ptr<arrow::RecordBatch> &batch);
    void appendColumn(const VectorSP &column,
                      const std::shared_ptr<arrow::Array> &array,
                      const ColumnTypeInfo &typeInfo) const;
    void appendListColumn(const VectorSP &column,
                          const std::shared_ptr<arrow::Array> &array,
                          const ColumnTypeInfo &typeInfo) const;
    void appendPrimitiveColumn(const VectorSP &column,
                               const std::shared_ptr<arrow::Array> &array,
                               const ColumnTypeInfo &typeInfo) const;
    ConstantSP convertPrimitiveColumn(const std::shared_ptr<arrow::Array> &array,
                                      const ColumnTypeInfo &typeInfo) const;
    ConstantSP convertListValue(const std::shared_ptr<arrow::Array> &values,
                                int64_t offset, int64_t length,
                                const ColumnTypeInfo &typeInfo) const;
    ColumnTypeInfo inferColumnType(const std::shared_ptr<arrow::Field> &field) const;

  private:
    std::shared_ptr<arrow::io::InputStream> stream_;
    std::shared_ptr<arrow::ipc::RecordBatchStreamReader> reader_;
    std::vector<std::string> columnNames_;
    std::vector<ColumnTypeInfo> columnTypes_;
    std::vector<ConstantSP> columns_;
    bool complete_ = false;
};

}  // namespace ddb
