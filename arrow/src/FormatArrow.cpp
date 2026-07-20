#include "FormatArrow.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <array>
#include <climits>
#include <cmath>
#include <cstring>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "SpecialConstant.h"

#define RECORDBATCH_SIZE 8192
#define HEADER_MAX_SIZE 1024
#define INT128_UNIT_LENGTH 16
#define MONTH_PER_YEAR 12
#define SECOND_PER_MINUTE 60
#define MINUTE_PER_HOUR 60
#define SECOND_PER_HOUR (SECOND_PER_MINUTE * MINUTE_PER_HOUR)
#define DECIMAL128_PRECISION 38

template <class T>
static void Destruction(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    T *ptr = (T *)(args[0]->getLong());  // NOLINT
    if (ptr != nullptr) {
        delete ptr;
    }
}

typedef std::map<int, std::map<int, ConstantMarshalSP>> MarshalMap;
typedef std::map<int, std::map<int, ConstantUnmarshalSP>> UnmarshalMap;

namespace {

constexpr const char *kDdbTypeMetadataKey = "ddbType";
constexpr const char *kDdbExtraMetadataKey = "ddbExtra";
constexpr const char *kDdbTotalRowsMetadataKey = "ddbTotalRows";
constexpr const char *kRetryableArrowReadPrefix = "__ddb_retryable_io__";

arrow::Status makeRetryableArrowReadStatus(IO_ERR ret) {
    return arrow::Status::IOError(
        std::string(kRetryableArrowReadPrefix) +
        std::to_string(static_cast<int>(ret)));
}

bool parseRetryableArrowReadStatus(const arrow::Status &status, IO_ERR &ret) {
    const std::string text = status.ToString();
    const auto pos = text.find(kRetryableArrowReadPrefix);
    if (pos == std::string::npos) {
        return false;
    }
    ret = static_cast<IO_ERR>(std::stoi(
        text.substr(pos + std::strlen(kRetryableArrowReadPrefix))));
    return true;
}

arrow::MemoryPool *arrowMemoryPool() {
    return arrow::system_memory_pool();
}

arrow::ipc::IpcReadOptions makeReadOptions() {
    static_assert(ARROW_VERSION_MAJOR >= 9,
                  "PluginArrow requires Apache Arrow 9.0.0 or newer.");

    auto options = arrow::ipc::IpcReadOptions::Defaults();
    options.max_recursion_depth = arrow::ipc::kMaxNestingDepth;
    options.memory_pool = arrowMemoryPool();
    options.included_fields.clear();
    options.use_threads = true;
    options.ensure_native_endian = true;
    options.pre_buffer_cache_options = arrow::io::CacheOptions::LazyDefaults();
    return options;
}

arrow::ipc::IpcWriteOptions makeWriteOptions() {
    auto options = arrow::ipc::IpcWriteOptions::Defaults();
    options.allow_64bit = false;
    options.max_recursion_depth = arrow::ipc::kMaxNestingDepth;
    options.alignment = 8;
    options.write_legacy_ipc_format = false;
    options.memory_pool = arrowMemoryPool();
    options.codec = nullptr;
    options.use_threads = true;
    options.emit_dictionary_deltas = false;
    options.unify_dictionaries = false;
    options.metadata_version = arrow::ipc::MetadataVersion::V5;
    return options;
}

std::shared_ptr<arrow::KeyValueMetadata> buildFieldMetadata(DATA_TYPE type,
                                                            int extra) {
    return arrow::key_value_metadata(
        {kDdbTypeMetadataKey, kDdbExtraMetadataKey},
        {std::to_string(static_cast<int>(type)), std::to_string(extra)});
}

std::shared_ptr<arrow::Field> buildField(const std::string &name,
                                         const std::shared_ptr<arrow::DataType> &type,
                                         DATA_TYPE ddbType, int extra = 0) {
    return arrow::field(name, type, true, buildFieldMetadata(ddbType, extra));
}

bool parseFieldMetadata(const std::shared_ptr<arrow::Field> &field, DATA_TYPE &type,
                        int &extra) {
    auto metadata = field->metadata();
    if (!metadata || !metadata->Contains(kDdbTypeMetadataKey)) {
        return false;
    }
    auto typeResult = metadata->Get(kDdbTypeMetadataKey);
    auto extraResult = metadata->Get(kDdbExtraMetadataKey);
    if (!typeResult.ok()) {
        return false;
    }
    type = static_cast<DATA_TYPE>(std::stoi(*typeResult));
    extra = (extraResult.ok()) ? std::stoi(*extraResult) : 0;
    return true;
}

INDEX parseSchemaInitialCapacity(const std::shared_ptr<arrow::Schema> &schema,
                                 INDEX fallback) {
    auto metadata = schema ? schema->metadata() : nullptr;
    if (!metadata || !metadata->Contains(kDdbTotalRowsMetadataKey)) {
        return fallback;
    }
    auto totalRowsResult = metadata->Get(kDdbTotalRowsMetadataKey);
    if (!totalRowsResult.ok()) {
        return fallback;
    }
    try {
        long long totalRows = std::stoll(*totalRowsResult);
        if (totalRows < 0 ||
            totalRows > static_cast<long long>(std::numeric_limits<INDEX>::max())) {
            return fallback;
        }
        return static_cast<INDEX>(totalRows);
    } catch (...) {
        return fallback;
    }
}

int128 loadInt128(const uint8_t *data) {
    int128 value = 0;
    std::memcpy(&value, data, sizeof(int128));
    return value;
}

int toVectorLength(int64_t length) {
    if (length < 0 ||
        length > static_cast<int64_t>(std::numeric_limits<int>::max())) {
        throw RuntimeException("Arrow upload batch is too large to fit into a DolphinDB vector.");
    }
    return static_cast<int>(length);
}

void applyExplicitNulls(const VectorSP &column, INDEX start,
                        const std::shared_ptr<arrow::Array> &array) {
    if (array->null_count() == 0) {
        return;
    }
    for (int64_t i = 0; i < array->length(); ++i) {
        if (array->IsNull(i)) {
            column->setNull(start + i);
        }
    }
    column->setNullFlag(true);
}

template <typename T, typename FillFn>
void fillPrimitiveBuffer(T *buffer, int len, const std::shared_ptr<arrow::Array> &array,
                         T nullValue, FillFn fill) {
    if (array->null_count() == 0) {
        for (int i = 0; i < len; ++i) {
            buffer[i] = fill(i);
        }
        return;
    }
    for (int i = 0; i < len; ++i) {
        buffer[i] = array->IsNull(i) ? nullValue : fill(i);
    }
}

template <typename T, typename ConvertFn>
void fillDecimalBuffer(T *buffer, int len,
                       const std::shared_ptr<arrow::Decimal128Array> &array,
                       T nullValue, ConvertFn convert) {
    if (array->null_count() == 0) {
        for (int i = 0; i < len; ++i) {
            buffer[i] = convert(loadInt128(array->GetValue(i)));
        }
        return;
    }
    for (int i = 0; i < len; ++i) {
        buffer[i] = array->IsNull(i)
                        ? nullValue
                        : convert(loadInt128(array->GetValue(i)));
    }
}

template <typename T>
T *prepareColumnBuffer(const VectorSP &column, INDEX start, int len,
                       const char *context) {
    const INDEX targetSize = start + len;
    column->reserve(targetSize);
    column->resize(targetSize);
    void *raw = column->getDataArray();
    if (raw == nullptr) {
        throw RuntimeException(std::string("Failed to access DolphinDB vector buffer for ") +
                               context + ".");
    }
    return static_cast<T *>(raw) + start;
}

template <typename OffsetType>
void fillRawStringVector(std::vector<std::string> &values,
                         const std::shared_ptr<arrow::Array> &array,
                         const OffsetType *offsets, const uint8_t *data) {
    const int len = toVectorLength(array->length());
    values.resize(len);
    if (array->null_count() == 0) {
        for (int i = 0; i < len; ++i) {
            const auto begin = offsets[i];
            const auto end = offsets[i + 1];
            if (begin == end) {
                values[i].clear();
                continue;
            }
            values[i].assign(reinterpret_cast<const char *>(data + begin),
                             static_cast<size_t>(end - begin));
        }
        return;
    }
    for (int i = 0; i < len; ++i) {
        if (array->IsNull(i)) {
            continue;
        }
        const auto begin = offsets[i];
        const auto end = offsets[i + 1];
        if (begin == end) {
            values[i].clear();
            continue;
        }
        values[i].assign(reinterpret_cast<const char *>(data + begin),
                         static_cast<size_t>(end - begin));
    }
}

void extractStringValues(const std::shared_ptr<arrow::Array> &array,
                         std::vector<std::string> &values) {
    switch (array->type_id()) {
    case arrow::Type::STRING: {
        auto typed = std::static_pointer_cast<arrow::StringArray>(array);
        fillRawStringVector(values, array, typed->raw_value_offsets(),
                            typed->value_data() ? typed->value_data()->data() : nullptr);
        return;
    }
    case arrow::Type::LARGE_STRING: {
        auto typed = std::static_pointer_cast<arrow::LargeStringArray>(array);
        fillRawStringVector(values, array, typed->raw_value_offsets(),
                            typed->value_data() ? typed->value_data()->data() : nullptr);
        return;
    }
    default:
        throw RuntimeException("Arrow string columns must use utf8 or large_utf8.");
    }
}

void extractBlobValues(const std::shared_ptr<arrow::Array> &array,
                       std::vector<std::string> &values) {
    if (array->type_id() != arrow::Type::LARGE_BINARY) {
        throw RuntimeException("Arrow blob columns must use large_binary.");
    }
    auto typed = std::static_pointer_cast<arrow::LargeBinaryArray>(array);
    const int len = toVectorLength(array->length());
    values.resize(len);
    for (int i = 0; i < len; ++i) {
        if (array->IsNull(i)) {
            continue;
        }
        auto view = typed->GetView(i);
        if (view.empty()) {
            values[i].clear();
            continue;
        }
        values[i].assign(reinterpret_cast<const char *>(view.data()), view.size());
    }
}

void extractSymbolValues(const std::shared_ptr<arrow::Array> &array,
                         std::vector<std::string> &values) {
    const int len = toVectorLength(array->length());
    switch (array->type_id()) {
    case arrow::Type::DICTIONARY: {
        auto typed = std::static_pointer_cast<arrow::DictionaryArray>(array);
        auto dictionary = typed->dictionary();
        if (dictionary->type_id() == arrow::Type::STRING) {
            auto dictValues = std::static_pointer_cast<arrow::StringArray>(dictionary);
            const int32_t *offsets = dictValues->raw_value_offsets();
            const uint8_t *data =
                dictValues->value_data() ? dictValues->value_data()->data() : nullptr;
            values.resize(len);
            for (int i = 0; i < len; ++i) {
                if (array->IsNull(i)) {
                    continue;
                }
                const int dictIndex = typed->GetValueIndex(i);
                const auto begin = offsets[dictIndex];
                const auto end = offsets[dictIndex + 1];
                values[i].assign(reinterpret_cast<const char *>(data + begin),
                                 static_cast<size_t>(end - begin));
            }
            return;
        }
        if (dictionary->type_id() == arrow::Type::LARGE_STRING) {
            auto dictValues =
                std::static_pointer_cast<arrow::LargeStringArray>(dictionary);
            const int64_t *offsets = dictValues->raw_value_offsets();
            const uint8_t *data =
                dictValues->value_data() ? dictValues->value_data()->data() : nullptr;
            values.resize(len);
            for (int i = 0; i < len; ++i) {
                if (array->IsNull(i)) {
                    continue;
                }
                const int dictIndex = typed->GetValueIndex(i);
                const auto begin = offsets[dictIndex];
                const auto end = offsets[dictIndex + 1];
                values[i].assign(reinterpret_cast<const char *>(data + begin),
                                 static_cast<size_t>(end - begin));
            }
            return;
        }
        throw RuntimeException("Arrow symbol columns must use utf8 or dictionary<int32, utf8>.");
    }
    case arrow::Type::STRING: {
        auto typed = std::static_pointer_cast<arrow::StringArray>(array);
        fillRawStringVector(values, array, typed->raw_value_offsets(),
                            typed->value_data() ? typed->value_data()->data() : nullptr);
        return;
    }
    case arrow::Type::LARGE_STRING: {
        auto typed = std::static_pointer_cast<arrow::LargeStringArray>(array);
        fillRawStringVector(values, array, typed->raw_value_offsets(),
                            typed->value_data() ? typed->value_data()->data() : nullptr);
        return;
    }
    default:
        throw RuntimeException("Arrow symbol columns must use utf8 or dictionary<int32, utf8>.");
    }
}

class ArrowInputStream : public arrow::io::InputStream {
  public:
    explicit ArrowInputStream(DataInputStreamSP in)
        : in_(std::move(in)) {}

    arrow::Status Close() override {
        closed_ = true;
        return arrow::Status::OK();
    }

    arrow::Result<int64_t> Tell() const override { return position_; }

    bool closed() const override { return closed_; }

    void beginOpenReplay() {
        if (recordingOpen_) {
            return;
        }
        recordingOpen_ = true;
        replayBuffer_.clear();
        replayOffset_ = 0;
        replayStartPosition_ = position_;
    }

    void rewindOpenReplay() {
        if (!recordingOpen_) {
            return;
        }
        replayOffset_ = 0;
        position_ = replayStartPosition_;
    }

    void finishOpenReplay() {
        recordingOpen_ = false;
        replayBuffer_.clear();
        replayOffset_ = 0;
    }

    arrow::Result<int64_t> Read(int64_t nbytes, void *out) override {
        if (closed_) {
            return arrow::Status::IOError("Arrow input stream is closed.");
        }
        if (nbytes < 0) {
            return arrow::Status::Invalid("Arrow input stream read size is negative.");
        }
        if (nbytes == 0) {
            return int64_t{0};
        }
        if (recordingOpen_) {
            return readWithOpenReplay(nbytes, out);
        }
        const auto targetSize = static_cast<size_t>(nbytes);
        if (pending_.capacity() < targetSize) {
            pending_.reserve(targetSize);
        }
        while (pending_.size() < targetSize) {
            const size_t previousSize = pending_.size();
            pending_.resize(targetSize);
            size_t actualLength = 0;
            IO_ERR ret = in_->readBytes(
                pending_.data() + previousSize, targetSize - previousSize,
                actualLength);
            pending_.resize(previousSize + actualLength);
            if (ret == OK) {
                continue;
            }
            if (ret == NODATA || ret == NOSPACE) {
                return makeRetryableArrowReadStatus(ret);
            }
            pending_.clear();
            return arrow::Status::IOError(
                "Failed to read Arrow IPC bytes from DolphinDB stream with IO "
                "error type " +
                std::to_string(static_cast<int>(ret)) + ".");
        }

        std::memcpy(out, pending_.data(), targetSize);
        if (pending_.size() == targetSize) {
            pending_.clear();
        } else {
            pending_.erase(
                pending_.begin(),
                pending_.begin() +
                    static_cast<std::vector<char>::difference_type>(targetSize));
        }
        position_ += nbytes;
        return nbytes;
    }

    arrow::Result<std::shared_ptr<arrow::Buffer>> Read(int64_t nbytes) override {
        if (nbytes < 0) {
            return arrow::Status::Invalid("Arrow input stream read size is negative.");
        }
        ARROW_ASSIGN_OR_RAISE(auto buffer,
                              arrow::AllocateBuffer(nbytes, arrowMemoryPool()));
        ARROW_ASSIGN_OR_RAISE(int64_t bytesRead,
                              Read(nbytes, buffer->mutable_data()));
        if (bytesRead != nbytes) {
            return arrow::Status::IOError("Failed to read complete Arrow IPC buffer from DolphinDB stream.");
        }
        std::shared_ptr<arrow::Buffer> sharedBuffer(std::move(buffer));
        return sharedBuffer;
    }

  private:
    arrow::Result<int64_t> readWithOpenReplay(int64_t nbytes, void *out) {
        const auto targetSize = static_cast<size_t>(nbytes);
        std::vector<char> result;
        result.reserve(targetSize);

        size_t localReplayOffset = replayOffset_;
        while (result.size() < targetSize) {
            if (localReplayOffset < replayBuffer_.size()) {
                const size_t available = replayBuffer_.size() - localReplayOffset;
                const size_t needed = targetSize - result.size();
                const size_t toCopy = std::min(available, needed);
                result.insert(result.end(), replayBuffer_.begin() +
                                                static_cast<std::vector<char>::difference_type>(
                                                    localReplayOffset),
                              replayBuffer_.begin() +
                                  static_cast<std::vector<char>::difference_type>(
                                      localReplayOffset + toCopy));
                localReplayOffset += toCopy;
                continue;
            }

            std::vector<char> chunk(targetSize - result.size());
            size_t actualLength = 0;
            IO_ERR ret = in_->readBytes(chunk.data(), chunk.size(), actualLength);
            if (actualLength > 0) {
                chunk.resize(actualLength);
                replayBuffer_.insert(replayBuffer_.end(), chunk.begin(), chunk.end());
                result.insert(result.end(), chunk.begin(), chunk.end());
                localReplayOffset += actualLength;
            }
            if (ret == OK) {
                continue;
            }
            if (ret == NODATA || ret == NOSPACE) {
                return makeRetryableArrowReadStatus(ret);
            }
            return arrow::Status::IOError(
                "Failed to read Arrow IPC bytes from DolphinDB stream with IO "
                "error type " +
                std::to_string(static_cast<int>(ret)) + ".");
        }

        std::memcpy(out, result.data(), targetSize);
        replayOffset_ = localReplayOffset;
        position_ += nbytes;
        return nbytes;
    }

  private:
    DataInputStreamSP in_;
    std::vector<char> pending_;
    std::vector<char> replayBuffer_;
    size_t replayOffset_ = 0;
    int64_t replayStartPosition_ = 0;
    bool recordingOpen_ = false;
    int64_t position_ = 0;
    bool closed_ = false;
};

}  // namespace

ConstantSP getSupportedFormats(Heap *heap, vector<ConstantSP> &args) {
    std::ignore = heap;
    std::ignore = args;
    int len = 2;
    VectorSP marshalCol = Util::createVector(DT_INT, len, len);
    marshalCol->setInt(0, FORMAT_ARROW);  // 010
    marshalCol->setInt(1, INT_MIN);
    VectorSP unmarshalCol = Util::createVector(DT_INT, len, len);
    unmarshalCol->setInt(0, FORMAT_ARROW);
    unmarshalCol->setInt(1, FORMAT_ARROW_RETURN_LIMIT);

    TableSP supportTable =
        Util::createTable({"marshal", "unmarshal"}, {marshalCol, unmarshalCol});
    return supportTable;
}

ConstantSP getConstantMarshal(Heap *heap, vector<ConstantSP> &args) {
    DataOutputStreamSP out =
        *reinterpret_cast<DataOutputStreamSP *>(args[0]->getLong());
    MarshalMap *marshalMap = new MarshalMap();
    (*marshalMap)[FORMAT_ARROW][DF_TABLE] = new ArrowTableMarshall(out);
    FunctionDefSP onDestroy = Util::createSystemProcedure(
        "marshal onDestroy()", Destruction<MarshalMap>, 1, 1);
    return Util::createResource((long long)marshalMap, "arrow marshal",
                                onDestroy, heap->currentSession());
}

ConstantSP getConstantUnmarshal(Heap *heap, vector<ConstantSP> &args) {
    DataInputStreamSP in =
        *reinterpret_cast<DataInputStreamSP *>(args[0]->getLong());
    UnmarshalMap *unmarshalMap = new UnmarshalMap();
    (*unmarshalMap)[FORMAT_ARROW][DF_TABLE] =
        new ArrowTableUnmarshall(in, heap->currentSession());
    (*unmarshalMap)[FORMAT_ARROW_RETURN_LIMIT][DF_TABLE] =
        new ArrowTableUnmarshall(in, heap->currentSession());
    FunctionDefSP onDestroy = Util::createSystemProcedure(
        "marshal onDestroy()", Destruction<UnmarshalMap>, 1, 1);
    return Util::createResource((long long)unmarshalMap, "arrow unmarshal",
                                onDestroy, heap->currentSession());
}

bool ArrowTableMarshall::start(const ConstantSP &target, bool blocking,
                               IO_ERR &ret) {
    return start(nullptr, 0, target, blocking, ret);
}

bool ArrowTableMarshall::start(const char *requestHeader, size_t headerSize,
                               const ConstantSP &target, bool blocking,
                               IO_ERR &ret) {
    std::ignore = blocking;
    if (headerSize > HEADER_MAX_SIZE) {
        ret = INVALIDDATA;
        return false;
    }

    complete_ = false;
    isSchemaSent_ = false;
    isDictionarySent_ = false;
    isRecorderSent_ = false;
    isEndSent_ = false;
    pendingPayload_.reset();
    rowsSent_ = 0;
    dictIdsMap_.clear();
    dictionariesSent_ = 0;
    target_ = target;

    buildSchema(target_);
    buildDictIdsMap(target_);
    buildWriter();

    if (headerSize > 0) {
        memcpy(&buf_[0], requestHeader, headerSize);
    }
    size_t offset = headerSize;
    int16_t flag = (target->getForm() + 32) << 8;
    flag += static_cast<int16_t>((TableSP(target))->getTableType());
    memcpy(buf_ + offset, (char *)&flag, sizeof(int16_t));
    offset += sizeof(int16_t);
    ret = out_.start(buf_, offset);
    if (ret != OK) {
        return false;
    }
    // send table schema
    ret = sendSchema();
    if (ret != OK) {
        return false;
    }
    // send symbol base strings, each symbol column sends once
    ret = sendDictionary();
    if (ret != OK) {
        return false;
    }
    // send table datas, send RECORDBATCH_SIZE lines once
    ret = sendRecordBatch();
    if (ret != OK) {
        return false;
    }
    // send EOF
    ret = sendEnd();
    if (ret != OK) {
        return false;
    }
    complete_ = true;
    return true;
}

void ArrowTableMarshall::buildWriter() {
    auto outputStreamResult =
        arrow::io::BufferOutputStream::Create(4096, arrowMemoryPool());
    if (!outputStreamResult.ok()) {
        throw RuntimeException("Fail to create BufferOutputStream.");
    }
    stream_ = *outputStreamResult;

    auto writerResult =
        arrow::ipc::internal::MakePayloadStreamWriter(stream_.get());
    if (!writerResult.ok()) {
        throw RuntimeException("Fail to MakeStreamWriter.");
    }
    writer_ = *std::move(writerResult);
    auto status = writer_->Start();
    if (!status.ok()) {
        throw RuntimeException("Fail to Start writer_.");
    }
}

IO_ERR ArrowTableMarshall::writeArrowPayload(
    const std::shared_ptr<arrow::Buffer> &buf) {
    pendingPayload_ = buf;
    IO_ERR ret = out_.start(reinterpret_cast<const char *>(buf->data()),
                            buf->size());
    if (out_.size() == 0) {
        pendingPayload_.reset();
    }
    return ret;
}

std::shared_ptr<arrow::Field> getVectorColSchema(const TableSP &table,
                                                 int colId, DATA_TYPE colType) {
    const std::string &name = table->getColumnName(colId);
    int extra = table->getColumn(colId)->getExtraParamForType();
    switch (colType) {
    case DT_BOOL:
        return buildField(name, arrow::boolean(), colType);
    case DT_CHAR:
        return buildField(name, arrow::int8(), colType);
    case DT_SHORT:
        return buildField(name, arrow::int16(), colType);
    case DT_INT:
        return buildField(name, arrow::int32(), colType);
    case DT_LONG:
        return buildField(name, arrow::int64(), colType);
    case DT_MONTH:
    case DT_DATE:
        return buildField(name, arrow::date32(), colType);
    case DT_TIME:
        return buildField(name, arrow::time32(arrow::TimeUnit::MILLI), colType);
    case DT_SECOND:
    case DT_MINUTE:
        return buildField(name, arrow::time32(arrow::TimeUnit::SECOND), colType);
    case DT_DATETIME:
        return buildField(name, arrow::timestamp(arrow::TimeUnit::SECOND), colType);
    case DT_TIMESTAMP:
        return buildField(name, arrow::timestamp(arrow::TimeUnit::MILLI), colType);
    case DT_NANOTIME:
        return buildField(name, arrow::time64(arrow::TimeUnit::NANO), colType);
    case DT_NANOTIMESTAMP:
        return buildField(name, arrow::timestamp(arrow::TimeUnit::NANO), colType);
    case DT_FLOAT:
        return buildField(name, arrow::float32(), colType);
    case DT_DOUBLE:
        return buildField(name, arrow::float64(), colType);
    case DT_SYMBOL:
        return buildField(name, arrow::dictionary(arrow::int32(), arrow::utf8()),
                          colType);
    case DT_DATEHOUR:
        return buildField(name, arrow::timestamp(arrow::TimeUnit::SECOND), colType);
    case DT_IP:
    case DT_STRING:
        return buildField(name, arrow::utf8(), colType);
    case DT_INT128:
    case DT_UUID:
        return buildField(name, arrow::fixed_size_binary(INT128_UNIT_LENGTH),
                          colType);
    case DT_BLOB:
        return buildField(name, arrow::large_binary(), colType);
    case DT_DECIMAL32:
    case DT_DECIMAL64:
    case DT_DECIMAL128:
        return buildField(name,
                          arrow::decimal128(DECIMAL128_PRECISION, extra), colType,
                          extra);
    default:
        throw RuntimeException("Not support this type. " +
                               Util::getDataTypeString(colType));
        break;
    }
}

std::shared_ptr<arrow::Field> getArrayVectorColSchema(const TableSP &table,
                                                      int colId,
                                                      DATA_TYPE colType) {
    const std::string &name = table->getColumnName(colId);
    int extra = table->getColumn(colId)->getExtraParamForType();
    switch (static_cast<int>(colType) - ARRAY_TYPE_BASE) {
    case DT_BOOL:
        return buildField(name, arrow::list(arrow::boolean()), colType);
    case DT_CHAR:
        return buildField(name, arrow::list(arrow::int8()), colType);
    case DT_SHORT:
        return buildField(name, arrow::list(arrow::int16()), colType);
    case DT_INT:
        return buildField(name, arrow::list(arrow::int32()), colType);
    case DT_LONG:
        return buildField(name, arrow::list(arrow::int64()), colType);
    case DT_MONTH:
    case DT_DATE:
        return buildField(name, arrow::list(arrow::date32()), colType);
    case DT_TIME:
        return buildField(name, arrow::list(arrow::time32(arrow::TimeUnit::MILLI)),
                          colType);
    case DT_SECOND:
    case DT_MINUTE:
        return buildField(name,
                          arrow::list(arrow::time32(arrow::TimeUnit::SECOND)),
                          colType);
    case DT_DATETIME:
        return buildField(name,
                          arrow::list(arrow::timestamp(arrow::TimeUnit::SECOND)),
                          colType);
    case DT_TIMESTAMP:
        return buildField(name,
                          arrow::list(arrow::timestamp(arrow::TimeUnit::MILLI)),
                          colType);
    case DT_NANOTIME:
        return buildField(name, arrow::list(arrow::time64(arrow::TimeUnit::NANO)),
                          colType);
    case DT_NANOTIMESTAMP:
        return buildField(name,
                          arrow::list(arrow::timestamp(arrow::TimeUnit::NANO)),
                          colType);
    case DT_FLOAT:
        return buildField(name, arrow::list(arrow::float32()), colType);
    case DT_DOUBLE:
        return buildField(name, arrow::list(arrow::float64()), colType);
    case DT_DATEHOUR:
        return buildField(name,
                          arrow::list(arrow::timestamp(arrow::TimeUnit::SECOND)),
                          colType);
    case DT_IP:
        return buildField(name, arrow::list(arrow::utf8()), colType);
    case DT_INT128:
    case DT_UUID:
        return buildField(name,
                          arrow::list(arrow::fixed_size_binary(INT128_UNIT_LENGTH)),
                          colType, extra);
    default:
        throw RuntimeException("Not support this type. " +
                               Util::getDataTypeString(colType));
        break;
    }
}

void ArrowTableMarshall::buildSchema(const TableSP &table) {
    int numCols = table->columns();
    vector<std::shared_ptr<arrow::Field>> schemaVector(numCols);
    for (int i = 0; i < numCols; ++i) {
        DATA_TYPE dataType = table->getColumn(i)->getType();
        if (dataType >= ARRAY_TYPE_BASE) {
            schemaVector[i] = getArrayVectorColSchema(table, i, dataType);
        } else {
            schemaVector[i] = getVectorColSchema(table, i, dataType);
        }
    }
    schema_ = arrow::schema(schemaVector);
}

void ArrowTableMarshall::buildDictIdsMap(const TableSP &table) {
    std::ignore = table;
    int numCols = target_->columns();
    int dictId = 0;
    for (int i = 0; i < numCols; ++i) {
        if (target_->getColumn(i)->getType() == DT_SYMBOL) {
            dictIdsMap_[dictId++] = i;
        }
    }
}

IO_ERR ArrowTableMarshall::sendDictionary() {
    for (auto iter = dictIdsMap_.find(dictionariesSent_);
         iter != dictIdsMap_.end(); ++iter) {
        SymbolBaseSP base = target_->getColumn(iter->second)->getSymbolBase();
        if (base.isNull()) {
            throw RuntimeException("column symbol Base is Null " +
                                   std::to_string(iter->second));
        }
        arrow::StringBuilder builder;
        auto result = builder.Reserve(base->size());
        if (!result.ok()) {
            throw RuntimeException("Fail to alloc memory in sendDictionary");
        }
        for (int j = 0; j < base->size(); ++j) {
            result = builder.Append(base->getSymbol(j).getString());
            if (!result.ok()) {
                throw RuntimeException(
                    "Fail to alloc memory in sendDictionary");
            }
        }
        auto arrayResult = builder.Finish();
        if (!arrayResult.ok()) {
            throw RuntimeException("Fail to Finish builder in sendDictionary");
        }
        auto array = *arrayResult;

        arrow::ipc::IpcPayload payload;
        auto status = arrow::ipc::GetDictionaryPayload(
            iter->first, array, makeWriteOptions(),
            &payload);
        if (!status.ok()) {
            throw RuntimeException(
                "Fail to GetDictionaryPayload in sendDictionary");
        }
        result = stream_->Reset();
        if (!result.ok()) {
            throw RuntimeException("Fail to reset stream in sendDictionary");
        }
        status = writer_->WritePayload(payload);
        if (!status.ok()) {
            throw RuntimeException(
                "Fail to GetDictionaryPayload in sendDictionary");
        }

        auto bufferResult = stream_->Finish();
        if (!bufferResult.ok()) {
            throw RuntimeException("Fail to get buffer from outputStream.");
        }
        auto buf = *bufferResult;
        IO_ERR ret = OK;
        ++dictionariesSent_;
        ret = writeArrowPayload(buf);
        if (ret != OK) {
            return ret;
        }
    }
    isDictionarySent_ = true;
    return OK;
}

void MarshalVectorColumn(
    std::shared_ptr<arrow::RecordBatchBuilder> &batch_builder,
    const VectorSP &column, int columnId, int real_size, int rowsSent,
    char *pvalidBuffer, DATA_TYPE colType) {
    arrow::Status result;
    switch (colType) {
    case DT_BOOL: {
        auto *builder = dynamic_cast<arrow::BooleanBuilder *>(
            batch_builder->GetField(columnId));
        std::vector<char> buffer(real_size);
        const char *pbuf =
            column->getBoolConst(rowsSent, real_size, buffer.data());
        result = builder->AppendValues(
            reinterpret_cast<const uint8_t *>(pbuf), real_size,
            reinterpret_cast<uint8_t *>(pvalidBuffer));
        break;
    }
    case DT_CHAR: {
        auto *builder = dynamic_cast<arrow::Int8Builder *>(
            batch_builder->GetField(columnId));
        std::vector<char> buffer(real_size);
        const char *pbuf =
            column->getCharConst(rowsSent, real_size, buffer.data());
        result = builder->AppendValues(
            reinterpret_cast<const int8_t *>(pbuf), real_size,
            reinterpret_cast<uint8_t *>(pvalidBuffer));
        break;
    }
    case DT_SHORT: {
        auto *builder = dynamic_cast<arrow::Int16Builder *>(
            batch_builder->GetField(columnId));
        std::vector<short> buffer(real_size);
        const short *pbuf =
            column->getShortConst(rowsSent, real_size, buffer.data());
        result = builder->AppendValues(
            pbuf, real_size, reinterpret_cast<uint8_t *>(pvalidBuffer));
        break;
    }
    case DT_INT: {
        auto *builder = dynamic_cast<arrow::Int32Builder *>(
            batch_builder->GetField(columnId));
        std::vector<int> buffer(real_size);
        const int *pbuf =
            column->getIntConst(rowsSent, real_size, buffer.data());
        result = builder->AppendValues(
            pbuf, real_size, reinterpret_cast<uint8_t *>(pvalidBuffer));
        break;
    }
    case DT_LONG: {
        auto *builder = dynamic_cast<arrow::Int64Builder *>(
            batch_builder->GetField(columnId));
        std::vector<long long> buffer(real_size);
        const long long *pbuf =
            column->getLongConst(rowsSent, real_size, buffer.data());
#if defined(__linux__)
        result = builder->AppendValues(
            reinterpret_cast<const int64_t *>(pbuf), real_size,
            reinterpret_cast<uint8_t *>(pvalidBuffer));
#elif defined(_WIN32)
        result = builder->AppendValues(
            pbuf, real_size, reinterpret_cast<uint8_t *>(pvalidBuffer));
#endif
        break;
    }
    case DT_MONTH: {
        auto *builder = dynamic_cast<arrow::Date32Builder *>(
            batch_builder->GetField(columnId));
        std::vector<int> buffer(real_size);
        std::vector<int> tbuf(real_size);
        const int *pbuf =
            column->getIntConst(rowsSent, real_size, buffer.data());
        for (INDEX ni = 0; ni < real_size; ++ni) {
            tbuf[ni] = Util::countDays(pbuf[ni] / MONTH_PER_YEAR,
                                       pbuf[ni] % MONTH_PER_YEAR + 1, 1);
        }
        result = builder->AppendValues(
            tbuf.data(), real_size, reinterpret_cast<uint8_t *>(pvalidBuffer));
        break;
    }
    case DT_DATE: {
        auto *builder = dynamic_cast<arrow::Date32Builder *>(
            batch_builder->GetField(columnId));
        std::vector<int> buffer(real_size);
        const int *pbuf =
            column->getIntConst(rowsSent, real_size, buffer.data());
        result = builder->AppendValues(
            pbuf, real_size, reinterpret_cast<uint8_t *>(pvalidBuffer));
        break;
    }
    case DT_TIME:
    case DT_SECOND: {
        auto *builder = dynamic_cast<arrow::Time32Builder *>(
            batch_builder->GetField(columnId));
        std::vector<int> buffer(real_size);
        const int *pbuf =
            column->getIntConst(rowsSent, real_size, buffer.data());
        result = builder->AppendValues(
            pbuf, real_size, reinterpret_cast<uint8_t *>(pvalidBuffer));
        break;
    }
    case DT_MINUTE: {
        auto *builder = dynamic_cast<arrow::Time32Builder *>(
            batch_builder->GetField(columnId));
        std::vector<int> intBuffer(real_size);
        std::vector<int> buffer(real_size);
        const int *pbuf =
            column->getIntConst(rowsSent, real_size, intBuffer.data());
        std::transform(pbuf, pbuf + real_size, buffer.begin(),
                       [](int val) { return val * SECOND_PER_MINUTE; });
        result =
            builder->AppendValues(buffer.data(), real_size,
                                  reinterpret_cast<uint8_t *>(pvalidBuffer));
        break;
    }
    case DT_DATETIME: {
        auto *builder = dynamic_cast<arrow::TimestampBuilder *>(
            batch_builder->GetField(columnId));
        std::vector<int> intBuffer(real_size);
        std::vector<int64_t> longBuffer(real_size);
        const int *pbuf =
            column->getIntConst(rowsSent, real_size, intBuffer.data());
        std::copy(pbuf, pbuf + real_size, longBuffer.begin());
        result =
            builder->AppendValues(longBuffer.data(), real_size,
                                  reinterpret_cast<uint8_t *>(pvalidBuffer));
        break;
    }
    case DT_TIMESTAMP:
    case DT_NANOTIMESTAMP: {
        auto *builder = dynamic_cast<arrow::TimestampBuilder *>(
            batch_builder->GetField(columnId));
        std::vector<long long> buffer(real_size);
        const long long *pbuf =
            column->getLongConst(rowsSent, real_size, buffer.data());
#if defined(__linux__)
        result = builder->AppendValues(
            reinterpret_cast<const int64_t *>(pbuf), real_size,
            reinterpret_cast<uint8_t *>(pvalidBuffer));
#elif defined(_WIN32)
        result = builder->AppendValues(
            pbuf, real_size, reinterpret_cast<uint8_t *>(pvalidBuffer));
#endif
        break;
    }
    case DT_SYMBOL: {
        auto *builder = dynamic_cast<arrow::StringDictionaryBuilder *>(
            batch_builder->GetField(columnId));
        std::vector<int> buffer(real_size);
        std::vector<int64_t> longBuffer(real_size);
        const int *pbuf =
            column->getIntConst(rowsSent, real_size, buffer.data());
        std::transform(pbuf, pbuf + real_size, longBuffer.begin(),
                       [](int val) { return static_cast<int64_t>(val); });
        result =
            builder->AppendIndices(longBuffer.data(), real_size,
                                   reinterpret_cast<uint8_t *>(pvalidBuffer));
        break;
    }
    case DT_NANOTIME: {
        auto *builder = dynamic_cast<arrow::Time64Builder *>(
            batch_builder->GetField(columnId));
        std::vector<long long> buffer(real_size);
        const long long *pbuf =
            column->getLongConst(rowsSent, real_size, buffer.data());
#if defined(__linux__)
        result = builder->AppendValues(
            reinterpret_cast<const int64_t *>(pbuf), real_size,
            reinterpret_cast<uint8_t *>(pvalidBuffer));
#elif defined(_WIN32)
        result = builder->AppendValues(
            pbuf, real_size, reinterpret_cast<uint8_t *>(pvalidBuffer));
#endif
        break;
    }
    case DT_DATEHOUR: {
        auto *builder = dynamic_cast<arrow::TimestampBuilder *>(
            batch_builder->GetField(columnId));
        std::vector<int> buffer(real_size);
        std::vector<int64_t> tbuf(real_size);
        const int *pbuf =
            column->getIntConst(rowsSent, real_size, buffer.data());
        std::transform(pbuf, pbuf + real_size, tbuf.begin(), [](int val) {
            return static_cast<int64_t>(val) *
                   static_cast<int64_t>(SECOND_PER_HOUR);
        });
        result = builder->AppendValues(
            tbuf.data(), real_size, reinterpret_cast<uint8_t *>(pvalidBuffer));
        break;
    }
    case DT_FLOAT: {
        auto *builder = dynamic_cast<arrow::FloatBuilder *>(
            batch_builder->GetField(columnId));
        std::vector<float> buffer(real_size);
        const float *pbuf =
            column->getFloatConst(rowsSent, real_size, buffer.data());
        if (pbuf != buffer.data()) {
            std::copy(pbuf, pbuf + real_size, buffer.begin());
        }
        for (int i = 0; i < real_size; ++i) {
            if (pvalidBuffer[i] != 0 && buffer[i] == FLT_NMIN) {
                buffer[i] = std::numeric_limits<float>::quiet_NaN();
            }
        }
        result = builder->AppendValues(
            buffer.data(), real_size, reinterpret_cast<uint8_t *>(pvalidBuffer));
        break;
    }
    case DT_DOUBLE: {
        auto *builder = dynamic_cast<arrow::DoubleBuilder *>(
            batch_builder->GetField(columnId));
        std::vector<double> buffer(real_size);
        const double *pbuf =
            column->getDoubleConst(rowsSent, real_size, buffer.data());
        if (pbuf != buffer.data()) {
            std::copy(pbuf, pbuf + real_size, buffer.begin());
        }
        for (int i = 0; i < real_size; ++i) {
            if (pvalidBuffer[i] != 0 && buffer[i] == DBL_NMIN) {
                buffer[i] = std::numeric_limits<double>::quiet_NaN();
            }
        }
        result = builder->AppendValues(
            buffer.data(), real_size, reinterpret_cast<uint8_t *>(pvalidBuffer));
        break;
    }
    case DT_STRING: {
        auto *builder = dynamic_cast<arrow::StringBuilder *>(
            batch_builder->GetField(columnId));
        std::vector<char *> buffer(real_size);
        char **pbuf =
            column->getStringConst(rowsSent, real_size, buffer.data());
        result =
            builder->AppendValues(const_cast<const char **>(pbuf), real_size,
                                  reinterpret_cast<uint8_t *>(pvalidBuffer));
        break;
    }
    case DT_BLOB: {
        auto *builder = dynamic_cast<arrow::LargeBinaryBuilder *>(
            batch_builder->GetField(columnId));
        std::vector<char *> buffer(real_size);
        char **pbuf =
            column->getStringConst(rowsSent, real_size, buffer.data());
        result =
            builder->AppendValues(const_cast<const char **>(pbuf), real_size,
                                  reinterpret_cast<uint8_t *>(pvalidBuffer));
        break;
    }
    case DT_INT128:
    case DT_UUID: {
        auto *builder = dynamic_cast<arrow::FixedSizeBinaryBuilder *>(
            batch_builder->GetField(columnId));
        size_t allocSize = static_cast<size_t>(real_size) * INT128_UNIT_LENGTH;
        std::vector<unsigned char> buffer(allocSize);
        std::vector<unsigned char> vbuffer(allocSize);
        const unsigned char *pbuf = column->getBinaryConst(
            rowsSent, real_size, INT128_UNIT_LENGTH, buffer.data());
        unsigned char *vbuf = vbuffer.data();
        for (size_t i = 0; i < static_cast<size_t>(real_size); ++i) {
            std::reverse_copy(pbuf + i * INT128_UNIT_LENGTH,
                              pbuf + (i + 1) * INT128_UNIT_LENGTH,
                              vbuf + i * INT128_UNIT_LENGTH);
        }
        result = builder->AppendValues(
            reinterpret_cast<const uint8_t *>(vbuf), real_size,
            reinterpret_cast<uint8_t *>(pvalidBuffer));
        break;
    }
    case DT_IP: {
        auto *builder = dynamic_cast<arrow::StringBuilder *>(
            batch_builder->GetField(columnId));
        size_t allocSize = static_cast<size_t>(real_size) * INT128_UNIT_LENGTH;
        std::vector<unsigned char> buffer(allocSize);
        const unsigned char *pbuf = column->getBinaryConst(
            rowsSent, real_size, INT128_UNIT_LENGTH, buffer.data());
        std::vector<std::string> ips(real_size);
        for (size_t ni = 0; ni < static_cast<size_t>(real_size); ++ni) {
            ips[ni] = IPAddr::toString(pbuf + ni * INT128_UNIT_LENGTH);
        }
        result = builder->AppendValues(
            ips, reinterpret_cast<uint8_t *>(pvalidBuffer));
        break;
    }
    case DT_DECIMAL32: {
        auto *builder = dynamic_cast<arrow::Decimal128Builder *>(
            batch_builder->GetField(columnId));
        std::vector<int> intBuffer(real_size);
        std::vector<__int128_t> buffer(real_size);
        int scale = column->getExtraParamForType();
        const int *pbuf = column->getDecimal32Const(rowsSent, real_size, scale,
                                                    intBuffer.data());
        std::transform(pbuf, pbuf + real_size, buffer.begin(),
                       [](int val) { return static_cast<__int128_t>(val); });
        result = builder->AppendValues(
            reinterpret_cast<uint8_t *>(buffer.data()), real_size,
            reinterpret_cast<uint8_t *>(pvalidBuffer));
        break;
    }
    case DT_DECIMAL64: {
        auto *builder = dynamic_cast<arrow::Decimal128Builder *>(
            batch_builder->GetField(columnId));
        std::vector<long long> longBuffer(real_size);
        std::vector<__int128_t> buffer(real_size);
        int scale = column->getExtraParamForType();
        const long long *pbuf = column->getDecimal64Const(
            rowsSent, real_size, scale, longBuffer.data());
        std::transform(
            pbuf, pbuf + real_size, buffer.begin(),
            [](long long val) { return static_cast<__int128_t>(val); });
        result = builder->AppendValues(
            reinterpret_cast<uint8_t *>(buffer.data()), real_size,
            reinterpret_cast<uint8_t *>(pvalidBuffer));
        break;
    }
    case DT_DECIMAL128: {
        auto *builder = dynamic_cast<arrow::Decimal128Builder *>(
            batch_builder->GetField(columnId));
        std::vector<int128> valueBuffer(real_size);
        std::vector<__int128_t> buffer(real_size);
        int scale = column->getExtraParamForType();
        const int128 *pbuf = column->getDecimal128Const(rowsSent, real_size, scale,
                                                        valueBuffer.data());
        std::copy(pbuf, pbuf + real_size, buffer.begin());
        result = builder->AppendValues(
            reinterpret_cast<uint8_t *>(buffer.data()), real_size,
            reinterpret_cast<uint8_t *>(pvalidBuffer));
        break;
    }
    default: {
        throw RuntimeException("Not Support This Type " +
                               Util::getDataTypeString(colType));
        break;
    }
    }
    if (!result.ok()) {
        throw RuntimeException("Failed to append values.");
    }
}

void MarshalArrayVectorColumn(
    std::shared_ptr<arrow::RecordBatchBuilder> &batch_builder,
    const VectorSP &column, int columnId, int real_size, int rowsSent,
    char *pvalidBuffer, DATA_TYPE colType) {
    std::ignore = pvalidBuffer;
    auto *builder =
        dynamic_cast<arrow::ListBuilder *>(batch_builder->GetField(columnId));
    std::vector<INDEX> indbuf(real_size);
    std::vector<INDEX> offset(real_size + 1);

    VectorSP indexVector =
        (static_cast<ObjectPtr<FastArrayVector>>(column))->getSourceIndex();
    VectorSP valueVector =
        (static_cast<ObjectPtr<FastArrayVector>>(column))->getSourceValue();

    const INDEX *pindbuf =
        indexVector->getIndexConst(rowsSent, real_size, indbuf.data());
    offset[0] = 0;
    INDEX valueStart = rowsSent == 0 ? 0 : indexVector->getIndex(rowsSent - 1);
    for (int i = 0; i < real_size; ++i) {
        offset[i + 1] = pindbuf[i] - valueStart;
    }

    int real_len = offset[real_size];

    std::vector<char> validBuffer(real_len);
    valueVector->isValid(valueStart, real_len, validBuffer.data());

    auto result = builder->AppendValues(
        reinterpret_cast<arrow::ListType::offset_type *>(offset.data()),
        real_size);
    if (!result.ok()) {
        throw RuntimeException("Failed to append values.");
    }

    switch (colType) {
    case DT_BOOL: {
        auto *subBuilder =
            dynamic_cast<arrow::BooleanBuilder *>(builder->value_builder());
        std::vector<char> valueBuffer(real_len);
        const char *pbuf =
            valueVector->getBoolConst(valueStart, real_len, valueBuffer.data());
        result = subBuilder->AppendValues(
            reinterpret_cast<const uint8_t *>(pbuf), real_len,
            reinterpret_cast<uint8_t *>(validBuffer.data()));
        break;
    }
    case DT_CHAR: {
        auto *subBuilder =
            dynamic_cast<arrow::Int8Builder *>(builder->value_builder());
        std::vector<char> valueBuffer(real_len);
        const char *pbuf =
            valueVector->getCharConst(valueStart, real_len, valueBuffer.data());
        result = subBuilder->AppendValues(
            reinterpret_cast<const int8_t *>(pbuf), real_len,
            reinterpret_cast<uint8_t *>(validBuffer.data()));
        break;
    }
    case DT_SHORT: {
        auto *subBuilder =
            dynamic_cast<arrow::Int16Builder *>(builder->value_builder());
        std::vector<short> valueBuffer(real_len);
        const short *pbuf =
            valueVector->getShortConst(valueStart, real_len, valueBuffer.data());
        result = subBuilder->AppendValues(
            pbuf, real_len, reinterpret_cast<uint8_t *>(validBuffer.data()));
        break;
    }
    case DT_INT: {
        auto *subBuilder =
            dynamic_cast<arrow::Int32Builder *>(builder->value_builder());
        std::vector<int> valueBuffer(real_len);
        const int *pbuf =
            valueVector->getIntConst(valueStart, real_len, valueBuffer.data());
        result = subBuilder->AppendValues(
            pbuf, real_len, reinterpret_cast<uint8_t *>(validBuffer.data()));
        break;
    }
    case DT_LONG: {
        auto *subBuilder =
            dynamic_cast<arrow::Int64Builder *>(builder->value_builder());
        std::vector<long long> valueBuffer(real_len);
        const long long *pbuf =
            valueVector->getLongConst(valueStart, real_len, valueBuffer.data());
#if defined(__linux__)
        result = subBuilder->AppendValues(
            reinterpret_cast<const int64_t *>(pbuf), real_len,
            reinterpret_cast<uint8_t *>(validBuffer.data()));
#elif defined(_WIN32)
        result = subBuilder->AppendValues(
            pbuf, real_len, reinterpret_cast<uint8_t *>(validBuffer.data()));
#endif
        break;
    }
    case DT_MONTH: {
        auto *subBuilder =
            dynamic_cast<arrow::Date32Builder *>(builder->value_builder());
        std::vector<int> intBuffer(real_len);
        std::vector<int> valueBuffer(real_len);
        const int *pbuf =
            valueVector->getIntConst(valueStart, real_len, intBuffer.data());
        std::transform(pbuf, pbuf + real_len, valueBuffer.begin(), [](int val) {
            return Util::countDays(val / MONTH_PER_YEAR,
                                   val % MONTH_PER_YEAR + 1, 1);
        });
        result = subBuilder->AppendValues(
            valueBuffer.data(), real_len,
            reinterpret_cast<uint8_t *>(validBuffer.data()));
        break;
    }
    case DT_DATE: {
        auto *subBuilder =
            dynamic_cast<arrow::Date32Builder *>(builder->value_builder());
        std::vector<int> valueBuffer(real_len);
        const int *pbuf =
            valueVector->getIntConst(valueStart, real_len, valueBuffer.data());
        result = subBuilder->AppendValues(
            pbuf, real_len, reinterpret_cast<uint8_t *>(validBuffer.data()));
        break;
    }
    case DT_TIME:
    case DT_SECOND: {
        auto *subBuilder =
            dynamic_cast<arrow::Time32Builder *>(builder->value_builder());
        std::vector<int> valueBuffer(real_len);
        const int *pbuf =
            valueVector->getIntConst(valueStart, real_len, valueBuffer.data());
        result = subBuilder->AppendValues(
            pbuf, real_len, reinterpret_cast<uint8_t *>(validBuffer.data()));
        break;
    }
    case DT_MINUTE: {
        auto *subBuilder =
            dynamic_cast<arrow::Time32Builder *>(builder->value_builder());
        std::vector<int> intBuffer(real_len);
        std::vector<int> valueBuffer(real_len);
        const int *pbuf =
            valueVector->getIntConst(valueStart, real_len, intBuffer.data());
        std::transform(pbuf, pbuf + real_len, valueBuffer.begin(),
                       [](int val) { return SECOND_PER_MINUTE * val; });
        result = subBuilder->AppendValues(
            valueBuffer.data(), real_len,
            reinterpret_cast<uint8_t *>(validBuffer.data()));
        break;
    }
    case DT_DATETIME: {
        auto *subBuilder =
            dynamic_cast<arrow::TimestampBuilder *>(builder->value_builder());
        std::vector<int> intBuffer(real_len);
        std::vector<int64_t> valueBuffer(real_len);
        const int *pbuf =
            valueVector->getIntConst(valueStart, real_len, intBuffer.data());
        std::transform(pbuf, pbuf + real_len, valueBuffer.begin(),
                       [](int val) { return static_cast<int64_t>(val); });
        result = subBuilder->AppendValues(
            valueBuffer.data(), real_len,
            reinterpret_cast<uint8_t *>(validBuffer.data()));
        break;
    }
    case DT_TIMESTAMP:
    case DT_NANOTIMESTAMP: {
        auto *subBuilder =
            dynamic_cast<arrow::TimestampBuilder *>(builder->value_builder());
        std::vector<long long> valueBuffer(real_len);
        const long long *pbuf =
            valueVector->getLongConst(valueStart, real_len, valueBuffer.data());
#if defined(__linux__)
        result = subBuilder->AppendValues(
            reinterpret_cast<const int64_t *>(pbuf), real_len,
            reinterpret_cast<uint8_t *>(validBuffer.data()));
#elif defined(_WIN32)
        result = subBuilder->AppendValues(
            pbuf, real_len, reinterpret_cast<uint8_t *>(validBuffer.data()));
#endif
        break;
    }
    case DT_NANOTIME: {
        auto *subBuilder =
            dynamic_cast<arrow::Time64Builder *>(builder->value_builder());
        std::vector<long long> valueBuffer(real_len);
        const long long *pbuf =
            valueVector->getLongConst(valueStart, real_len, valueBuffer.data());
#if defined(__linux__)
        result = subBuilder->AppendValues(
            reinterpret_cast<const int64_t *>(pbuf), real_len,
            reinterpret_cast<uint8_t *>(validBuffer.data()));
#elif defined(_WIN32)
        result = subBuilder->AppendValues(
            pbuf, real_len, reinterpret_cast<uint8_t *>(validBuffer.data()));
#endif
        break;
    }
    case DT_DATEHOUR: {
        auto *subBuilder =
            dynamic_cast<arrow::TimestampBuilder *>(builder->value_builder());
        std::vector<int> intBuffer(real_len);
        std::vector<int64_t> valueBuffer(real_len);
        const int *pbuf =
            valueVector->getIntConst(valueStart, real_len, intBuffer.data());
        std::transform(pbuf, pbuf + real_len, valueBuffer.begin(), [](int val) {
            return static_cast<int64_t>(val) *
                   static_cast<int64_t>(SECOND_PER_HOUR);
        });
        result = subBuilder->AppendValues(
            valueBuffer.data(), real_len,
            reinterpret_cast<uint8_t *>(validBuffer.data()));
        break;
    }
    case DT_FLOAT: {
        auto *subBuilder =
            dynamic_cast<arrow::FloatBuilder *>(builder->value_builder());
        std::vector<float> valueBuffer(real_len);
        const float *pbuf =
            valueVector->getFloatConst(valueStart, real_len, valueBuffer.data());
        result = subBuilder->AppendValues(
            pbuf, real_len, reinterpret_cast<uint8_t *>(validBuffer.data()));
        break;
    }
    case DT_DOUBLE: {
        auto *subBuilder =
            dynamic_cast<arrow::DoubleBuilder *>(builder->value_builder());
        std::vector<double> valueBuffer(real_len);
        const double *pbuf =
            valueVector->getDoubleConst(valueStart, real_len, valueBuffer.data());
        result = subBuilder->AppendValues(
            pbuf, real_len, reinterpret_cast<uint8_t *>(validBuffer.data()));
        break;
    }
    case DT_INT128:
    case DT_UUID: {
        auto *subBuilder = dynamic_cast<arrow::FixedSizeBinaryBuilder *>(
            builder->value_builder());
        size_t allocSize = static_cast<size_t>(real_len) * INT128_UNIT_LENGTH;
        std::vector<unsigned char> valueBuffer(allocSize);
        std::vector<unsigned char> vbuffer(allocSize);
        const unsigned char *pbuf = valueVector->getBinaryConst(
            valueStart, real_len, INT128_UNIT_LENGTH, valueBuffer.data());
        unsigned char *vbuf = vbuffer.data();
        for (size_t i = 0; i < static_cast<size_t>(real_len); ++i) {
            std::reverse_copy(pbuf + i * INT128_UNIT_LENGTH,
                              pbuf + (i + 1) * INT128_UNIT_LENGTH,
                              vbuf + i * INT128_UNIT_LENGTH);
        }
        result = subBuilder->AppendValues(
            reinterpret_cast<const uint8_t *>(vbuf), real_len,
            reinterpret_cast<uint8_t *>(validBuffer.data()));
        break;
    }
    case DT_IP: {
        auto *subBuilder =
            dynamic_cast<arrow::StringBuilder *>(builder->value_builder());
        size_t allocSize = static_cast<size_t>(real_len) * INT128_UNIT_LENGTH;
        std::vector<unsigned char> valueBuffer(allocSize);
        const unsigned char *pbuf = valueVector->getBinaryConst(
            valueStart, real_len, INT128_UNIT_LENGTH, valueBuffer.data());
        std::vector<std::string> ips(real_len);
        for (size_t ni = 0; ni < static_cast<size_t>(real_len); ++ni) {
            ips[ni] = IPAddr::toString(pbuf + ni * INT128_UNIT_LENGTH);
        }
        result = subBuilder->AppendValues(
            ips, reinterpret_cast<uint8_t *>(validBuffer.data()));
        break;
    }
    default: {
        throw RuntimeException(
            "Not Support This Type " +
            Util::getDataTypeString(DATA_TYPE(colType + ARRAY_TYPE_BASE)));
        break;
    }
    }
    if (!result.ok()) {
        throw RuntimeException("Failed to append values.");
    }
}

IO_ERR ArrowTableMarshall::sendRecordBatch() {
    int numCols = target_->columns();
    int vecLen = target_->size();
    std::vector<char> validBuffer(RECORDBATCH_SIZE);
    char *pvalidBuffer = validBuffer.data();
    while (rowsSent_ < vecLen) {
        int real_size = std::min(RECORDBATCH_SIZE, vecLen - rowsSent_);
        auto recordBatchRuilderResult =
            arrow::RecordBatchBuilder::Make(schema_, arrowMemoryPool(), real_size);
        if (!recordBatchRuilderResult.ok()) {
            throw RuntimeException("Fail to create Record Batch Builder");
        }

        std::shared_ptr<arrow::RecordBatchBuilder> batch_builder =
            *std::move(recordBatchRuilderResult);
        for (int i = 0; i < numCols; ++i) {
            VectorSP column = target_->getColumn(i);
            DATA_TYPE dataType = column->getType();
            column->isValid(rowsSent_, real_size, pvalidBuffer);
            if (dataType >= ARRAY_TYPE_BASE) {
                MarshalArrayVectorColumn(batch_builder, column, i, real_size,
                                         rowsSent_, pvalidBuffer,
                                         DATA_TYPE(dataType - ARRAY_TYPE_BASE));
            } else {
                MarshalVectorColumn(batch_builder, column, i, real_size,
                                    rowsSent_, pvalidBuffer, dataType);
            }
        }

        std::shared_ptr<arrow::RecordBatch> batch;
        auto flushResult = batch_builder->Flush();
        if (!flushResult.ok()) {
            throw RuntimeException("batch_builder flush Fail");
        }
        batch = *flushResult;

        auto result = stream_->Reset();
        if (!result.ok()) {
            throw RuntimeException("Failed to reset stream.");
        }

        arrow::ipc::IpcPayload payload;
        auto status = arrow::ipc::GetRecordBatchPayload(
            *batch, makeWriteOptions(), &payload);
        if (!status.ok()) {
            throw RuntimeException("GetRecordBatchPayload Fail");
        }

        status = writer_->WritePayload(payload);
        if (!status.ok()) {
            throw RuntimeException("WritePayload Fail");
        }
        auto bufferResult = stream_->Finish();
        if (!bufferResult.ok()) {
            throw RuntimeException("Fail to get buffer from outputStream.");
        }
        auto buf = *bufferResult;
        rowsSent_ += real_size;
        IO_ERR ret = writeArrowPayload(buf);
        if (ret != OK) {
            return ret;
        }
    }
    isRecorderSent_ = true;
    return OK;
}

IO_ERR ArrowTableMarshall::sendSchema() {
    auto result = stream_->Reset();
    if (!result.ok()) {
        throw RuntimeException("Failed to reset stream.");
    }
    arrow::ipc::IpcPayload payload;
    arrow::ipc::DictionaryFieldMapper mapper(*schema_);
    auto status = arrow::ipc::GetSchemaPayload(
        *schema_, makeWriteOptions(), mapper, &payload);
    if (!status.ok()) {
        throw RuntimeException("Fail to GetSchemaPayload.");
    }
    status = writer_->WritePayload(payload);
    if (!status.ok()) {
        throw RuntimeException("Fail to WritePayload.");
    }

    auto bufferResult = stream_->Finish();
    if (!bufferResult.ok()) {
        throw RuntimeException("Fail to Finish stream_.");
    }
    auto buf = *bufferResult;
    isSchemaSent_ = true;

    return writeArrowPayload(buf);
}

IO_ERR ArrowTableMarshall::sendEnd() {
    isEndSent_ = true;
    return out_.start(reinterpret_cast<const char *>(&endMarker_),
                      sizeof(endMarker_));
}

bool ArrowTableMarshall::resume(IO_ERR &ret) {
    if (complete_) {
        return (ret = out_.getDataOutputStream()->flush()) == OK;
    }
    if (out_.size() > 0) {
        ret = out_.resume();
        if (out_.size() == 0) {
            pendingPayload_.reset();
        }
        if (ret != OK) {
            return false;
        }
    }
    if (!isSchemaSent_) {
        ret = sendSchema();
        if (ret != OK) {
            return false;
        }
    }
    if (!isDictionarySent_) {
        ret = sendDictionary();
        if (ret != OK) {
            return false;
        }
    }
    if (!isRecorderSent_) {
        ret = sendRecordBatch();
        if (ret != OK) {
            return false;
        }
    }
    if (!isEndSent_) {
        ret = sendEnd();
        if (ret != OK) {
            return false;
        }
    }
    complete_ = true;
    return (ret = out_.getDataOutputStream()->flush()) == OK;
}

ArrowTableUnmarshall::ArrowTableUnmarshall(const DataInputStreamSP &in,
                                           Session *session)
    : ConstantUnmarshalImp(in, session) {}

bool ArrowTableUnmarshall::start(short flag, bool blocking, IO_ERR &ret) {
    std::ignore = blocking;
    reset();
    DATA_FORM form = static_cast<DATA_FORM>((flag >> 8) & ~32);
    if (form != DF_TABLE) {
        ret = INVALIDDATA;
        return false;
    }
    stream_ = std::make_shared<ArrowInputStream>(in_);
    return consume(ret);
}

bool ArrowTableUnmarshall::resume(IO_ERR &ret) { return consume(ret); }

void ArrowTableUnmarshall::reset() {
    obj_.clear();
    reader_.reset();
    stream_.reset();
    columnNames_.clear();
    columnTypes_.clear();
    columns_.clear();
    complete_ = false;
}

bool ArrowTableUnmarshall::consume(IO_ERR &ret) {
    if (complete_) {
        ret = OK;
        return true;
    }
    if (!reader_) {
        auto inputStream = std::static_pointer_cast<ArrowInputStream>(stream_);
        inputStream->beginOpenReplay();
        auto readerResult = arrow::ipc::RecordBatchStreamReader::Open(
            stream_, makeReadOptions());
        if (!readerResult.ok()) {
            if (parseRetryableArrowReadStatus(readerResult.status(), ret)) {
                inputStream->rewindOpenReplay();
                return false;
            }
            inputStream->finishOpenReplay();
            throw RuntimeException("Failed to open Arrow IPC stream: " +
                                   readerResult.status().ToString());
        }
        reader_ = *readerResult;
        inputStream->finishOpenReplay();
    }
    while (true) {
        auto batchResult = reader_->Next();
        if (!batchResult.ok()) {
            if (parseRetryableArrowReadStatus(batchResult.status(), ret)) {
                return false;
            }
            throw RuntimeException("Failed to decode Arrow IPC stream: " +
                                   batchResult.status().ToString());
        }
        auto batch = *batchResult;
        if (!batch) {
            auto schema = reader_->schema();
            if (!schema) {
                throw RuntimeException("Arrow IPC stream missing schema.");
            }
            ensureSchemaInitialized(schema, parseSchemaInitialCapacity(schema, 0));
            obj_ = Util::createTable(columnNames_, columns_);
            complete_ = true;
            ret = OK;
            return true;
        }
        appendRecordBatch(batch);
    }
}

void ArrowTableUnmarshall::ensureSchemaInitialized(
    const std::shared_ptr<arrow::Schema> &schema, INDEX initialCapacity) {
    if (!columns_.empty()) {
        return;
    }
    int numFields = schema->num_fields();
    columnNames_.reserve(numFields);
    columnTypes_.reserve(numFields);
    columns_.reserve(numFields);
    for (int i = 0; i < numFields; ++i) {
        auto field = schema->field(i);
        ColumnTypeInfo info = inferColumnType(field);
        columnNames_.push_back(field->name());
        columnTypes_.push_back(info);
        if (info.type >= ARRAY_TYPE_BASE) {
            columns_.push_back(
                Util::createArrayVector(info.type, 0, 0, initialCapacity, 0, true,
                                        info.extra));
        } else if (info.type == DT_STRING) {
            columns_.push_back(Util::createStringNoInitVector(0, initialCapacity));
        } else {
            columns_.push_back(Util::createVector(info.type, 0, initialCapacity, true,
                                                  info.extra));
        }
    }
}

void ArrowTableUnmarshall::appendRecordBatch(
    const std::shared_ptr<arrow::RecordBatch> &batch) {
    ensureSchemaInitialized(
        batch->schema(),
        parseSchemaInitialCapacity(batch->schema(), batch->num_rows()));
    for (int i = 0; i < batch->num_columns(); ++i) {
        VectorSP column = columns_[i];
        appendColumn(column, batch->column(i), columnTypes_[i]);
    }
}

void ArrowTableUnmarshall::appendColumn(
    const VectorSP &column, const std::shared_ptr<arrow::Array> &array,
    const ColumnTypeInfo &typeInfo) const {
    if (typeInfo.type >= ARRAY_TYPE_BASE) {
        appendListColumn(column, array, typeInfo);
        return;
    }
    appendPrimitiveColumn(column, array, typeInfo);
}

void ArrowTableUnmarshall::appendListColumn(
    const VectorSP &column, const std::shared_ptr<arrow::Array> &array,
    const ColumnTypeInfo &typeInfo) const {
    if (array->length() == 0) {
        return;
    }
    auto listArray = std::dynamic_pointer_cast<arrow::ListArray>(array);
    if (!listArray) {
        throw RuntimeException("Only Arrow list columns are supported for array vectors.");
    }
    DATA_TYPE valueType =
        static_cast<DATA_TYPE>(static_cast<int>(typeInfo.type) - ARRAY_TYPE_BASE);
    ColumnTypeInfo valueInfo(valueType, typeInfo.extra);
    INDEX valueCount = 0;
    for (int64_t i = 0; i < listArray->length(); ++i) {
        valueCount += listArray->IsNull(i) ? 1 : listArray->value_length(i);
    }
    VectorSP index = Util::createVector(DT_INDEX, listArray->length(),
                                        listArray->length());
    INDEX *indexBuffer = prepareColumnBuffer<INDEX>(
        index, 0, toVectorLength(listArray->length()), "Arrow list batch offsets");
    VectorSP values = Util::createVector(valueType, 0, valueCount, true,
                                         typeInfo.extra);
    ConstantSP sourceValues = convertPrimitiveColumn(listArray->values(), valueInfo);
    INDEX cumulativeLength = 0;
    ConstantSP nullValue = Util::createNullConstant(valueType, typeInfo.extra);
    for (int64_t i = 0; i < listArray->length(); ++i) {
        if (listArray->IsNull(i)) {
            if (!values->append(nullValue, 0, 1)) {
                throw RuntimeException("Failed to append Arrow list null row.");
            }
            ++cumulativeLength;
        } else {
            int64_t offset = listArray->value_offset(i);
            int64_t length = listArray->value_length(i);
            if (length > 0 &&
                !values->append(sourceValues, offset, toVectorLength(length))) {
                throw RuntimeException("Failed to append Arrow list values.");
            }
            cumulativeLength += toVectorLength(length);
        }
        indexBuffer[i] = cumulativeLength;
    }
    VectorSP batchColumn = new FastArrayVector(index, values);
    if (!column->append(batchColumn)) {
        throw RuntimeException("Failed to append Arrow list rows to array vector column.");
    }
}

ConstantSP ArrowTableUnmarshall::convertListValue(
    const std::shared_ptr<arrow::Array> &values, int64_t offset, int64_t length,
    const ColumnTypeInfo &typeInfo) const {
    auto slice = values->Slice(offset, length);
    return convertPrimitiveColumn(slice, typeInfo);
}

void ArrowTableUnmarshall::appendPrimitiveColumn(
    const VectorSP &column, const std::shared_ptr<arrow::Array> &array,
    const ColumnTypeInfo &typeInfo) const {
    const int len = toVectorLength(array->length());
    if (len == 0) {
        return;
    }
    const INDEX start = column->size();

    switch (typeInfo.type) {
    case DT_BOOL: {
        auto typed = std::static_pointer_cast<arrow::BooleanArray>(array);
        char *buffer =
            prepareColumnBuffer<char>(column, start, len, "Arrow bool column");
        fillPrimitiveBuffer<char>(buffer, len, array, CHAR_MIN,
                                  [&](int i) { return typed->Value(i) ? 1 : 0; });
        if (array->null_count() > 0) {
            column->setNullFlag(true);
        }
        return;
    }
    case DT_CHAR: {
        auto typed = std::static_pointer_cast<arrow::Int8Array>(array);
        char *buffer =
            prepareColumnBuffer<char>(column, start, len, "Arrow char column");
        if (array->null_count() == 0) {
            std::memcpy(buffer, typed->raw_values(), static_cast<size_t>(len));
        } else {
            fillPrimitiveBuffer<char>(buffer, len, array, CHAR_MIN,
                                      [&](int i) {
                                          return static_cast<char>(typed->raw_values()[i]);
                                      });
        }
        if (array->null_count() > 0) {
            column->setNullFlag(true);
        }
        return;
    }
    case DT_SHORT: {
        auto typed = std::static_pointer_cast<arrow::Int16Array>(array);
        short *buffer =
            prepareColumnBuffer<short>(column, start, len, "Arrow short column");
        if (array->null_count() == 0) {
            std::memcpy(buffer, typed->raw_values(),
                        static_cast<size_t>(len) * sizeof(short));
        } else {
            fillPrimitiveBuffer<short>(buffer, len, array, SHRT_MIN,
                                       [&](int i) {
                                           return static_cast<short>(typed->raw_values()[i]);
                                       });
        }
        if (array->null_count() > 0) {
            column->setNullFlag(true);
        }
        return;
    }
    case DT_INT:
    case DT_MONTH:
    case DT_DATE:
    case DT_TIME:
    case DT_SECOND:
    case DT_MINUTE:
    case DT_DATETIME:
    case DT_DATEHOUR: {
        int *buffer =
            prepareColumnBuffer<int>(column, start, len, "Arrow int column");
        switch (typeInfo.type) {
        case DT_INT: {
            auto typed = std::static_pointer_cast<arrow::Int32Array>(array);
            if (array->null_count() == 0) {
                std::memcpy(buffer, typed->raw_values(),
                            static_cast<size_t>(len) * sizeof(int));
            } else {
                fillPrimitiveBuffer<int>(buffer, len, array, INT_MIN,
                                         [&](int i) { return typed->raw_values()[i]; });
            }
            break;
        }
        case DT_MONTH: {
            auto typed = std::static_pointer_cast<arrow::Date32Array>(array);
            fillPrimitiveBuffer<int>(buffer, len, array, INT_MIN, [&](int i) {
                int year, month, day;
                Util::parseDate(typed->raw_values()[i], year, month, day);
                return year * MONTH_PER_YEAR + month - 1;
            });
            break;
        }
        case DT_DATE: {
            auto typed = std::static_pointer_cast<arrow::Date32Array>(array);
            if (array->null_count() == 0) {
                std::memcpy(buffer, typed->raw_values(),
                            static_cast<size_t>(len) * sizeof(int));
            } else {
                fillPrimitiveBuffer<int>(buffer, len, array, INT_MIN,
                                         [&](int i) { return typed->raw_values()[i]; });
            }
            break;
        }
        case DT_TIME:
        case DT_SECOND: {
            auto typed = std::static_pointer_cast<arrow::Time32Array>(array);
            if (array->null_count() == 0) {
                std::memcpy(buffer, typed->raw_values(),
                            static_cast<size_t>(len) * sizeof(int));
            } else {
                fillPrimitiveBuffer<int>(buffer, len, array, INT_MIN,
                                         [&](int i) { return typed->raw_values()[i]; });
            }
            break;
        }
        case DT_MINUTE: {
            auto typed = std::static_pointer_cast<arrow::Time32Array>(array);
            fillPrimitiveBuffer<int>(buffer, len, array, INT_MIN, [&](int i) {
                return typed->raw_values()[i] / SECOND_PER_MINUTE;
            });
            break;
        }
        case DT_DATETIME: {
            auto typed = std::static_pointer_cast<arrow::TimestampArray>(array);
            fillPrimitiveBuffer<int>(buffer, len, array, INT_MIN, [&](int i) {
                return static_cast<int>(typed->raw_values()[i]);
            });
            break;
        }
        case DT_DATEHOUR: {
            auto typed = std::static_pointer_cast<arrow::TimestampArray>(array);
            fillPrimitiveBuffer<int>(buffer, len, array, INT_MIN, [&](int i) {
                return static_cast<int>(typed->raw_values()[i] / SECOND_PER_HOUR);
            });
            break;
        }
        default:
            break;
        }
        if (array->null_count() > 0) {
            column->setNullFlag(true);
        }
        return;
    }
    case DT_LONG:
    case DT_TIMESTAMP:
    case DT_NANOTIME:
    case DT_NANOTIMESTAMP: {
        long long *buffer =
            prepareColumnBuffer<long long>(column, start, len, "Arrow long column");
        switch (typeInfo.type) {
        case DT_LONG: {
            auto typed = std::static_pointer_cast<arrow::Int64Array>(array);
            if (array->null_count() == 0) {
                std::memcpy(buffer, typed->raw_values(),
                            static_cast<size_t>(len) * sizeof(long long));
            } else {
                fillPrimitiveBuffer<long long>(
                    buffer, len, array, LLONG_MIN,
                    [&](int i) { return static_cast<long long>(typed->raw_values()[i]); });
            }
            break;
        }
        case DT_TIMESTAMP:
        case DT_NANOTIMESTAMP: {
            auto typed = std::static_pointer_cast<arrow::TimestampArray>(array);
            if (array->null_count() == 0) {
                std::memcpy(buffer, typed->raw_values(),
                            static_cast<size_t>(len) * sizeof(long long));
            } else {
                fillPrimitiveBuffer<long long>(
                    buffer, len, array, LLONG_MIN,
                    [&](int i) { return static_cast<long long>(typed->raw_values()[i]); });
            }
            break;
        }
        case DT_NANOTIME: {
            auto typed = std::static_pointer_cast<arrow::Time64Array>(array);
            if (array->null_count() == 0) {
                std::memcpy(buffer, typed->raw_values(),
                            static_cast<size_t>(len) * sizeof(long long));
            } else {
                fillPrimitiveBuffer<long long>(
                    buffer, len, array, LLONG_MIN,
                    [&](int i) { return static_cast<long long>(typed->raw_values()[i]); });
            }
            break;
        }
        default:
            break;
        }
        if (array->null_count() > 0) {
            column->setNullFlag(true);
        }
        return;
    }
    case DT_FLOAT: {
        auto typed = std::static_pointer_cast<arrow::FloatArray>(array);
        float *buffer =
            prepareColumnBuffer<float>(column, start, len, "Arrow float column");
        bool hasNull = array->null_count() > 0;
        for (int i = 0; i < len; ++i) {
            if (array->IsNull(i)) {
                buffer[i] = FLT_NMIN;
                continue;
            }
            const float value = typed->Value(i);
            if (std::isnan(value)) {
                buffer[i] = FLT_NMIN;
            } else {
                buffer[i] = value;
            }
        }
        if (hasNull) {
            column->setNullFlag(true);
        }
        return;
    }
    case DT_DOUBLE: {
        auto typed = std::static_pointer_cast<arrow::DoubleArray>(array);
        double *buffer =
            prepareColumnBuffer<double>(column, start, len, "Arrow double column");
        bool hasNull = array->null_count() > 0;
        for (int i = 0; i < len; ++i) {
            if (array->IsNull(i)) {
                buffer[i] = DBL_NMIN;
                continue;
            }
            const double value = typed->Value(i);
            if (std::isnan(value)) {
                buffer[i] = DBL_NMIN;
            } else {
                buffer[i] = value;
            }
        }
        if (hasNull) {
            column->setNullFlag(true);
        }
        return;
    }
    case DT_SYMBOL: {
        std::vector<std::string> values(len);
        extractSymbolValues(array, values);
        if (!column->appendString(values.data(), len)) {
            throw RuntimeException("Failed to append Arrow symbol column.");
        }
        applyExplicitNulls(column, start, array);
        return;
    }
    case DT_STRING: {
        std::vector<std::string> values(len);
        extractStringValues(array, values);
        if (!column->appendString(values.data(), len)) {
            throw RuntimeException("Failed to append Arrow string column.");
        }
        applyExplicitNulls(column, start, array);
        return;
    }
    case DT_BLOB: {
        std::vector<std::string> values(len);
        extractBlobValues(array, values);
        if (!column->appendString(values.data(), len)) {
            throw RuntimeException("Failed to append Arrow blob column.");
        }
        applyExplicitNulls(column, start, array);
        return;
    }
    case DT_UUID:
    case DT_INT128: {
        auto typed = std::static_pointer_cast<arrow::FixedSizeBinaryArray>(array);
        column->reserve(start + len);
        column->resize(start + len);
        auto *buffer = static_cast<unsigned char *>(column->getDataArray());
        if (buffer == nullptr) {
            throw RuntimeException("Failed to access DolphinDB vector buffer for Arrow int128 column.");
        }
        buffer += static_cast<size_t>(start) * INT128_UNIT_LENGTH;
        for (int i = 0; i < len; ++i) {
            if (array->IsNull(i)) {
                continue;
            }
            const uint8_t *raw = typed->GetValue(i);
            std::reverse_copy(raw, raw + INT128_UNIT_LENGTH,
                              buffer +
                                  static_cast<size_t>(i) * INT128_UNIT_LENGTH);
        }
        applyExplicitNulls(column, start, array);
        return;
    }
    case DT_IP: {
        auto typed = std::static_pointer_cast<arrow::StringArray>(array);
        column->reserve(start + len);
        column->resize(start + len);
        auto *buffer = static_cast<unsigned char *>(column->getDataArray());
        if (buffer == nullptr) {
            throw RuntimeException("Failed to access DolphinDB vector buffer for Arrow IP column.");
        }
        buffer += static_cast<size_t>(start) * INT128_UNIT_LENGTH;
        for (int i = 0; i < len; ++i) {
            if (array->IsNull(i)) {
                continue;
            }
            auto view = typed->GetView(i);
            if (!IPAddr::parseIPAddr(view.data(), view.size(),
                                     buffer +
                                         static_cast<size_t>(i) * INT128_UNIT_LENGTH)) {
                throw RuntimeException("Failed to parse Arrow utf8 value as DolphinDB IP.");
            }
        }
        applyExplicitNulls(column, start, array);
        return;
    }
    case DT_DECIMAL32:
    case DT_DECIMAL64:
    case DT_DECIMAL128: {
        auto typed = std::static_pointer_cast<arrow::Decimal128Array>(array);
        switch (typeInfo.type) {
        case DT_DECIMAL32: {
            int *buffer = prepareColumnBuffer<int>(
                column, start, len, "Arrow decimal32 column");
            fillDecimalBuffer<int>(buffer, len, typed, INT_MIN,
                                   [](int128 value) {
                                       return static_cast<int>(value);
                                   });
            if (!column->setDecimal32(start, len, typeInfo.extra, buffer)) {
                throw RuntimeException("Failed to append Arrow decimal32 column.");
            }
            break;
        }
        case DT_DECIMAL64: {
            long long *buffer = prepareColumnBuffer<long long>(
                column, start, len, "Arrow decimal64 column");
            fillDecimalBuffer<long long>(buffer, len, typed, LLONG_MIN,
                                         [](int128 value) {
                                             return static_cast<long long>(value);
                                         });
            if (!column->setDecimal64(start, len, typeInfo.extra, buffer)) {
                throw RuntimeException("Failed to append Arrow decimal64 column.");
            }
            break;
        }
        case DT_DECIMAL128: {
            int128 *buffer = prepareColumnBuffer<int128>(
                column, start, len, "Arrow decimal128 column");
            fillDecimalBuffer<int128>(buffer, len, typed, INT128_MIN,
                                      [](int128 value) { return value; });
            if (!column->setDecimal128(start, len, typeInfo.extra, buffer)) {
                throw RuntimeException("Failed to append Arrow decimal128 column.");
            }
            break;
        }
        default:
            break;
        }
        if (array->null_count() > 0) {
            column->setNullFlag(true);
        }
        return;
    }
    default:
        throw RuntimeException("Unsupported Arrow upload type " +
                               Util::getDataTypeString(typeInfo.type));
    }
}

ConstantSP ArrowTableUnmarshall::convertPrimitiveColumn(
    const std::shared_ptr<arrow::Array> &array,
    const ColumnTypeInfo &typeInfo) const {
    const int len = toVectorLength(array->length());
    VectorSP result =
        Util::createVector(typeInfo.type, len, len, true, typeInfo.extra);
    if (len == 0) {
        return result;
    }

    switch (typeInfo.type) {
    case DT_BOOL: {
        auto typed = std::static_pointer_cast<arrow::BooleanArray>(array);
        std::vector<char> buffer(len);
        fillPrimitiveBuffer<char>(buffer.data(), len, array, CHAR_MIN,
                                  [&](int i) { return typed->Value(i) ? 1 : 0; });
        if (!result->setBool(0, len, buffer.data())) {
            throw RuntimeException("Failed to set Arrow bool vector.");
        }
        break;
    }
    case DT_CHAR: {
        auto typed = std::static_pointer_cast<arrow::Int8Array>(array);
        std::vector<char> buffer(len);
        fillPrimitiveBuffer<char>(buffer.data(), len, array, CHAR_MIN,
                                  [&](int i) {
                                      return static_cast<char>(typed->Value(i));
                                  });
        if (!result->setChar(0, len, buffer.data())) {
            throw RuntimeException("Failed to set Arrow char vector.");
        }
        break;
    }
    case DT_SHORT: {
        auto typed = std::static_pointer_cast<arrow::Int16Array>(array);
        std::vector<short> buffer(len);
        fillPrimitiveBuffer<short>(buffer.data(), len, array, SHRT_MIN,
                                   [&](int i) {
                                       return static_cast<short>(typed->Value(i));
                                   });
        if (!result->setShort(0, len, buffer.data())) {
            throw RuntimeException("Failed to set Arrow short vector.");
        }
        break;
    }
    case DT_INT:
    case DT_MONTH:
    case DT_DATE:
    case DT_TIME:
    case DT_SECOND:
    case DT_MINUTE:
    case DT_DATETIME:
    case DT_DATEHOUR: {
        std::vector<int> buffer(len);
        switch (typeInfo.type) {
        case DT_INT: {
            auto typed = std::static_pointer_cast<arrow::Int32Array>(array);
            fillPrimitiveBuffer<int>(buffer.data(), len, array, INT_MIN,
                                     [&](int i) { return typed->Value(i); });
            break;
        }
        case DT_MONTH: {
            auto typed = std::static_pointer_cast<arrow::Date32Array>(array);
            fillPrimitiveBuffer<int>(buffer.data(), len, array, INT_MIN, [&](int i) {
                int year, month, day;
                Util::parseDate(typed->Value(i), year, month, day);
                return year * MONTH_PER_YEAR + month - 1;
            });
            break;
        }
        case DT_DATE: {
            auto typed = std::static_pointer_cast<arrow::Date32Array>(array);
            fillPrimitiveBuffer<int>(buffer.data(), len, array, INT_MIN,
                                     [&](int i) { return typed->Value(i); });
            break;
        }
        case DT_TIME:
        case DT_SECOND: {
            auto typed = std::static_pointer_cast<arrow::Time32Array>(array);
            fillPrimitiveBuffer<int>(buffer.data(), len, array, INT_MIN,
                                     [&](int i) { return typed->Value(i); });
            break;
        }
        case DT_MINUTE: {
            auto typed = std::static_pointer_cast<arrow::Time32Array>(array);
            fillPrimitiveBuffer<int>(buffer.data(), len, array, INT_MIN,
                                     [&](int i) {
                                         return typed->Value(i) / SECOND_PER_MINUTE;
                                     });
            break;
        }
        case DT_DATETIME: {
            auto typed = std::static_pointer_cast<arrow::TimestampArray>(array);
            fillPrimitiveBuffer<int>(buffer.data(), len, array, INT_MIN,
                                     [&](int i) {
                                         return static_cast<int>(typed->Value(i));
                                     });
            break;
        }
        case DT_DATEHOUR: {
            auto typed = std::static_pointer_cast<arrow::TimestampArray>(array);
            fillPrimitiveBuffer<int>(buffer.data(), len, array, INT_MIN,
                                     [&](int i) {
                                         return static_cast<int>(
                                             typed->Value(i) / SECOND_PER_HOUR);
                                     });
            break;
        }
        default:
            break;
        }
        if (!result->setInt(0, len, buffer.data())) {
            throw RuntimeException("Failed to set Arrow int vector.");
        }
        break;
    }
    case DT_LONG:
    case DT_TIMESTAMP:
    case DT_NANOTIME:
    case DT_NANOTIMESTAMP: {
        std::vector<long long> buffer(len);
        switch (typeInfo.type) {
        case DT_LONG: {
            auto typed = std::static_pointer_cast<arrow::Int64Array>(array);
            fillPrimitiveBuffer<long long>(
                buffer.data(), len, array, LLONG_MIN,
                [&](int i) { return static_cast<long long>(typed->Value(i)); });
            break;
        }
        case DT_TIMESTAMP:
        case DT_NANOTIMESTAMP: {
            auto typed = std::static_pointer_cast<arrow::TimestampArray>(array);
            fillPrimitiveBuffer<long long>(
                buffer.data(), len, array, LLONG_MIN,
                [&](int i) { return static_cast<long long>(typed->Value(i)); });
            break;
        }
        case DT_NANOTIME: {
            auto typed = std::static_pointer_cast<arrow::Time64Array>(array);
            fillPrimitiveBuffer<long long>(
                buffer.data(), len, array, LLONG_MIN,
                [&](int i) { return static_cast<long long>(typed->Value(i)); });
            break;
        }
        default:
            break;
        }
        if (!result->setLong(0, len, buffer.data())) {
            throw RuntimeException("Failed to set Arrow long vector.");
        }
        break;
    }
    case DT_FLOAT: {
        auto typed = std::static_pointer_cast<arrow::FloatArray>(array);
        std::vector<float> buffer(len);
        fillPrimitiveBuffer<float>(buffer.data(), len, array, FLT_NMIN,
                                   [&](int i) { return typed->Value(i); });
        if (!result->setFloat(0, len, buffer.data())) {
            throw RuntimeException("Failed to set Arrow float vector.");
        }
        break;
    }
    case DT_DOUBLE: {
        auto typed = std::static_pointer_cast<arrow::DoubleArray>(array);
        std::vector<double> buffer(len);
        fillPrimitiveBuffer<double>(buffer.data(), len, array, DBL_NMIN,
                                    [&](int i) { return typed->Value(i); });
        if (!result->setDouble(0, len, buffer.data())) {
            throw RuntimeException("Failed to set Arrow double vector.");
        }
        break;
    }
    case DT_SYMBOL: {
        std::vector<std::string> values(len);
        extractSymbolValues(array, values);
        if (!result->setString(0, len, values.data())) {
            throw RuntimeException("Failed to set Arrow symbol vector.");
        }
        applyExplicitNulls(result, 0, array);
        return result;
    }
    case DT_STRING: {
        std::vector<std::string> values(len);
        extractStringValues(array, values);
        if (!result->setString(0, len, values.data())) {
            throw RuntimeException("Failed to set Arrow string vector.");
        }
        applyExplicitNulls(result, 0, array);
        return result;
    }
    case DT_BLOB: {
        std::vector<std::string> values(len);
        extractBlobValues(array, values);
        if (!result->setString(0, len, values.data())) {
            throw RuntimeException("Failed to set Arrow blob vector.");
        }
        applyExplicitNulls(result, 0, array);
        return result;
    }
    case DT_UUID:
    case DT_INT128: {
        auto typed = std::static_pointer_cast<arrow::FixedSizeBinaryArray>(array);
        std::vector<unsigned char> buffer(static_cast<size_t>(len) * INT128_UNIT_LENGTH);
        for (int i = 0; i < len; ++i) {
            if (array->IsNull(i)) {
                continue;
            }
            const uint8_t *raw = typed->GetValue(i);
            std::reverse_copy(raw, raw + INT128_UNIT_LENGTH,
                              buffer.data() +
                                  static_cast<size_t>(i) * INT128_UNIT_LENGTH);
        }
        if (!result->setBinary(0, len, INT128_UNIT_LENGTH, buffer.data())) {
            throw RuntimeException("Failed to set Arrow int128 vector.");
        }
        applyExplicitNulls(result, 0, array);
        return result;
    }
    case DT_IP: {
        auto typed = std::static_pointer_cast<arrow::StringArray>(array);
        std::vector<unsigned char> buffer(static_cast<size_t>(len) * INT128_UNIT_LENGTH);
        for (int i = 0; i < len; ++i) {
            if (array->IsNull(i)) {
                continue;
            }
            auto view = typed->GetView(i);
            if (!IPAddr::parseIPAddr(view.data(), view.size(),
                                     buffer.data() +
                                         static_cast<size_t>(i) * INT128_UNIT_LENGTH)) {
                throw RuntimeException("Failed to parse Arrow utf8 value as DolphinDB IP.");
            }
        }
        if (!result->setBinary(0, len, INT128_UNIT_LENGTH, buffer.data())) {
            throw RuntimeException("Failed to set Arrow IP vector.");
        }
        applyExplicitNulls(result, 0, array);
        return result;
    }
    case DT_DECIMAL32: {
        auto typed = std::static_pointer_cast<arrow::Decimal128Array>(array);
        std::vector<int> buffer(len);
        fillDecimalBuffer<int>(buffer.data(), len, typed, INT_MIN,
                               [](int128 value) { return static_cast<int>(value); });
        if (!result->setDecimal32(0, len, typeInfo.extra, buffer.data())) {
            throw RuntimeException("Failed to set Arrow decimal32 vector.");
        }
        break;
    }
    case DT_DECIMAL64: {
        auto typed = std::static_pointer_cast<arrow::Decimal128Array>(array);
        std::vector<long long> buffer(len);
        fillDecimalBuffer<long long>(buffer.data(), len, typed, LLONG_MIN,
                                     [](int128 value) {
                                         return static_cast<long long>(value);
                                     });
        if (!result->setDecimal64(0, len, typeInfo.extra, buffer.data())) {
            throw RuntimeException("Failed to set Arrow decimal64 vector.");
        }
        break;
    }
    case DT_DECIMAL128: {
        auto typed = std::static_pointer_cast<arrow::Decimal128Array>(array);
        std::vector<int128> buffer(len);
        fillDecimalBuffer<int128>(buffer.data(), len, typed, INT128_MIN,
                                  [](int128 value) { return value; });
        if (!result->setDecimal128(0, len, typeInfo.extra, buffer.data())) {
            throw RuntimeException("Failed to set Arrow decimal128 vector.");
        }
        break;
    }
    default:
        throw RuntimeException("Unsupported Arrow upload type " +
                               Util::getDataTypeString(typeInfo.type));
    }

    if (array->null_count() > 0) {
        result->setNullFlag(true);
    }
    return result;
}

ArrowTableUnmarshall::ColumnTypeInfo ArrowTableUnmarshall::inferColumnType(
    const std::shared_ptr<arrow::Field> &field) const {
    DATA_TYPE metadataType;
    int metadataExtra = 0;
    if (parseFieldMetadata(field, metadataType, metadataExtra)) {
        return {metadataType, metadataExtra};
    }

    auto type = field->type();
    switch (type->id()) {
    case arrow::Type::BOOL:
        return {DT_BOOL, 0};
    case arrow::Type::INT8:
        return {DT_CHAR, 0};
    case arrow::Type::INT16:
        return {DT_SHORT, 0};
    case arrow::Type::INT32:
        return {DT_INT, 0};
    case arrow::Type::INT64:
        return {DT_LONG, 0};
    case arrow::Type::DATE32:
        return {DT_DATE, 0};
    case arrow::Type::TIME32: {
        auto timeType = std::static_pointer_cast<arrow::Time32Type>(type);
        if (timeType->unit() == arrow::TimeUnit::SECOND) {
            return {DT_SECOND, 0};
        }
        if (timeType->unit() == arrow::TimeUnit::MILLI) {
            return {DT_TIME, 0};
        }
        break;
    }
    case arrow::Type::TIME64: {
        auto timeType = std::static_pointer_cast<arrow::Time64Type>(type);
        if (timeType->unit() == arrow::TimeUnit::NANO) {
            return {DT_NANOTIME, 0};
        }
        break;
    }
    case arrow::Type::TIMESTAMP: {
        auto timestampType = std::static_pointer_cast<arrow::TimestampType>(type);
        if (timestampType->unit() == arrow::TimeUnit::SECOND) {
            return {DT_DATETIME, 0};
        }
        if (timestampType->unit() == arrow::TimeUnit::MILLI) {
            return {DT_TIMESTAMP, 0};
        }
        if (timestampType->unit() == arrow::TimeUnit::NANO) {
            return {DT_NANOTIMESTAMP, 0};
        }
        break;
    }
    case arrow::Type::FLOAT:
        return {DT_FLOAT, 0};
    case arrow::Type::DOUBLE:
        return {DT_DOUBLE, 0};
    case arrow::Type::DICTIONARY: {
        auto dictType = std::static_pointer_cast<arrow::DictionaryType>(type);
        if (dictType->index_type()->id() != arrow::Type::INT32) {
            throw RuntimeException("Only dictionary<int32, utf8> is supported.");
        }
        if (dictType->value_type()->id() != arrow::Type::STRING &&
            dictType->value_type()->id() != arrow::Type::LARGE_STRING) {
            throw RuntimeException("Only dictionary<int32, utf8> is supported.");
        }
        return {DT_SYMBOL, 0};
    }
    case arrow::Type::STRING:
    case arrow::Type::LARGE_STRING:
        return {DT_STRING, 0};
    case arrow::Type::FIXED_SIZE_BINARY: {
        auto binaryType = std::static_pointer_cast<arrow::FixedSizeBinaryType>(type);
        if (binaryType->byte_width() != INT128_UNIT_LENGTH) {
            throw RuntimeException("Only fixed_size_binary(16) is supported.");
        }
        return {DT_INT128, 0};
    }
    case arrow::Type::LARGE_BINARY:
        return {DT_BLOB, 0};
    case arrow::Type::DECIMAL128: {
        auto decimalType = std::static_pointer_cast<arrow::Decimal128Type>(type);
        return {DT_DECIMAL128, decimalType->scale()};
    }
    case arrow::Type::LIST: {
        auto listType = std::static_pointer_cast<arrow::ListType>(type);
        auto valueInfo = inferColumnType(listType->value_field());
        if (valueInfo.type == DT_SYMBOL || valueInfo.type == DT_STRING ||
            valueInfo.type == DT_BLOB || valueInfo.type == DT_DECIMAL32 ||
            valueInfo.type == DT_DECIMAL64 || valueInfo.type == DT_DECIMAL128) {
            throw RuntimeException("This Arrow list type is not supported.");
        }
        return {static_cast<DATA_TYPE>(valueInfo.type + ARRAY_TYPE_BASE),
                valueInfo.extra};
    }
    default:
        break;
    }
    throw RuntimeException("Unsupported Arrow upload type: " + type->ToString());
}
