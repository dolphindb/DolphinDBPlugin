#include "FormatArrow.h"

#include <vector>
#include <map>
#include <iostream>

#include "Concurrent.h"
#include "ConstantMarshal.h"
#include "SpecialConstant.h"

#define RECORDBATCH_SIZE 8192

template<class T>
static void Destruction(Heap *heap, vector<ConstantSP> &args) {
    T * ptr = (T *) (args[0]->getLong());
    if (ptr != nullptr) {
        delete ptr;
    }
}

typedef std::map<int, std::map<int, ConstantMarshalSP>> MarshalMap;
typedef std::map<int, std::map<int, ConstantUnmarshalSP>> UnmarshalMap;

ConstantSP getSupportedFormats(Heap *heap, vector<ConstantSP> &args) {
    int len = 1;
    VectorSP marshalCol     = Util::createVector(DT_INT, len, len);
    marshalCol->setInt(0, FORMAT_ARROW);        // 010
    VectorSP unmarshalCol   = Util::createVector(DT_INT, len, len);
    unmarshalCol->setInt(0, INT_MIN);           // 010
    
    TableSP supportTable    = Util::createTable({"marshal", "unmarshal"}, {marshalCol, unmarshalCol});
    return supportTable;
}

ConstantSP getConstantMarshal(Heap *heap, vector<ConstantSP> &args) {
    DataOutputStreamSP out  = *reinterpret_cast<DataOutputStreamSP *>(args[0]->getLong());
    MarshalMap* marshalMap = new MarshalMap();
    (*marshalMap)[FORMAT_ARROW][DF_TABLE] = new ArrowTableMarshall(out);
    FunctionDefSP onDestroy = Util::createSystemProcedure("marshal onDestroy()", Destruction<MarshalMap>, 1, 1);
    return Util::createResource((long long) marshalMap, "arrow marshal", onDestroy, heap->currentSession());
}

ConstantSP getConstantUnmarshal(Heap *heap, vector<ConstantSP> &args) {
    DataInputStreamSP in = *reinterpret_cast<DataInputStreamSP *>(args[0]->getLong());
    UnmarshalMap* unmarshalMap = new UnmarshalMap();
    FunctionDefSP onDestroy = Util::createSystemProcedure("marshal onDestroy()", Destruction<UnmarshalMap>, 1, 1);
    return Util::createResource((long long) &unmarshalMap, "arrow unmarshal", onDestroy, heap->currentSession());
}

bool ArrowTableMarshall::start(const ConstantSP& target, bool blocking, IO_ERR& ret){
    return start(0, 0, target, blocking, ret);
}

bool ArrowTableMarshall::start(const char* requestHeader, size_t headerSize, const ConstantSP& target, bool blocking, IO_ERR& ret){
    if(headerSize > 1024){
        ret = INVALIDDATA;
        return false;
    }

    complete_ = false;
    isSchemaSent_ = false;
    isDictionarySent_ = false;
    isRecorderSent_ = false;
    isEndSent_ = false;
    rowsSent_ = 0;
    dictIdsMap_.clear();
    dictionariesSent_ = 0;
    target_ = target;

    buildSchema(target_);
    buildDictIdsMap(target_);
    buildWriter();
    
    if(headerSize > 0){
        memcpy(buf_ , requestHeader, headerSize);
    }
    size_t offset = headerSize;
    short flag = (target->getForm() + 32) <<8;
    flag += ((TableSP)target)->getTableType();
    memcpy(buf_ + offset, (char*)&flag, sizeof(short));
    offset += sizeof(short);
    ret = out_.start(buf_, offset);
    if(ret != OK){
        return false;
    }
    //send table schema
    ret = sendSchema();
    if(ret != OK){
        return false;
    }
    //send symbol base strings, each symbol column sends once
    ret = sendDictionary();
    if(ret != OK){
        return false;
    }
    //send table datas, send RECORDBATCH_SIZE lines once
    ret = sendRecordBatch();
    if(ret != OK){
        return false;
    }
    //send EOF
    ret = sendEnd();
    complete_ = true;
    return ret == OK;
}

void ArrowTableMarshall::buildWriter(){
    auto outputStreamResult = arrow::io::BufferOutputStream::Create();
    if (!outputStreamResult.ok())
        throw RuntimeException("Fail to create BufferOutputStream.");
    stream_ = *outputStreamResult;

    auto writerResult = arrow::ipc::internal::MakePayloadStreamWriter(stream_.get());
    if (!writerResult.ok())
        throw RuntimeException("Fail to MakeStreamWriter.");
    writer_ = *std::move(writerResult);
    auto status = writer_->Start();
    if(!status.ok()){
        throw RuntimeException("Fail to Start writer_.");
    }
}

std::shared_ptr<arrow::Field> getVectorColSchema(TableSP table, int colId, DATA_TYPE colType) {
    switch (colType)
    {
    case DT_BOOL:
        return arrow::field(table->getColumnName(colId), arrow::boolean());
    case DT_CHAR:
        return arrow::field(table->getColumnName(colId), arrow::int8());
    case DT_SHORT:
        return arrow::field(table->getColumnName(colId), arrow::int16());
    case DT_INT:
        return arrow::field(table->getColumnName(colId), arrow::int32());
    case DT_LONG:
        return arrow::field(table->getColumnName(colId), arrow::int64());
    case DT_MONTH:
    case DT_DATE:
        return arrow::field(table->getColumnName(colId), arrow::date32());
    case DT_TIME:
        return arrow::field(table->getColumnName(colId), arrow::time32(arrow::TimeUnit::MILLI));
    case DT_SECOND:
    case DT_MINUTE:
        return arrow::field(table->getColumnName(colId), arrow::time32(arrow::TimeUnit::SECOND));
    case DT_DATETIME:
        return arrow::field(table->getColumnName(colId), arrow::timestamp(arrow::TimeUnit::SECOND));
    case DT_TIMESTAMP:
        return arrow::field(table->getColumnName(colId), arrow::timestamp(arrow::TimeUnit::MILLI));
    case DT_NANOTIME:
        return arrow::field(table->getColumnName(colId), arrow::time64(arrow::TimeUnit::NANO));
    case DT_NANOTIMESTAMP:
        return arrow::field(table->getColumnName(colId), arrow::timestamp(arrow::TimeUnit::NANO));
    case DT_FLOAT:
        return arrow::field(table->getColumnName(colId), arrow::float32());
    case DT_DOUBLE:
        return arrow::field(table->getColumnName(colId), arrow::float64());
    case DT_SYMBOL:
        return arrow::field(table->getColumnName(colId), arrow::dictionary(arrow::int32(), arrow::utf8()));
    case DT_DATEHOUR:
        return arrow::field(table->getColumnName(colId), arrow::timestamp(arrow::TimeUnit::SECOND));
    case DT_IP:
    case DT_STRING:
        return arrow::field(table->getColumnName(colId), arrow::utf8());
    case DT_INT128:
    case DT_UUID:
        return arrow::field(table->getColumnName(colId), arrow::fixed_size_binary(16));
    case DT_BLOB:
        return arrow::field(table->getColumnName(colId), arrow::large_binary());
    case DT_DECIMAL32:
    case DT_DECIMAL64:
        return arrow::field(table->getColumnName(colId), arrow::decimal128(38, table->getColumn(colId)->getExtraParamForType()));
    default:
        throw RuntimeException("Not support this type. " + Util::getDataTypeString(colType));
        break;
    }
}

std::shared_ptr<arrow::Field> getArrayVectorColSchema(TableSP table, int colId, DATA_TYPE colType) {
    switch (int(colType)-ARRAY_TYPE_BASE)
    {
    case DT_BOOL:
        return arrow::field(table->getColumnName(colId), arrow::list(arrow::boolean()));
    case DT_CHAR:
        return arrow::field(table->getColumnName(colId), arrow::list(arrow::int8()));
    case DT_SHORT:
        return arrow::field(table->getColumnName(colId), arrow::list(arrow::int16()));
    case DT_INT:
        return arrow::field(table->getColumnName(colId), arrow::list(arrow::int32()));
    case DT_LONG:
        return arrow::field(table->getColumnName(colId), arrow::list(arrow::int64()));
    case DT_MONTH:
    case DT_DATE:
        return arrow::field(table->getColumnName(colId), arrow::list(arrow::date32()));
    case DT_TIME:
        return arrow::field(table->getColumnName(colId), arrow::list(arrow::time32(arrow::TimeUnit::MILLI)));
    case DT_SECOND:
    case DT_MINUTE:
        return arrow::field(table->getColumnName(colId), arrow::list(arrow::time32(arrow::TimeUnit::SECOND)));
    case DT_DATETIME:
        return arrow::field(table->getColumnName(colId), arrow::list(arrow::timestamp(arrow::TimeUnit::SECOND)));
    case DT_TIMESTAMP:
        return arrow::field(table->getColumnName(colId), arrow::list(arrow::timestamp(arrow::TimeUnit::MILLI)));
    case DT_NANOTIME:
        return arrow::field(table->getColumnName(colId), arrow::list(arrow::time64(arrow::TimeUnit::NANO)));
    case DT_NANOTIMESTAMP:
        return arrow::field(table->getColumnName(colId), arrow::list(arrow::timestamp(arrow::TimeUnit::NANO)));
    case DT_FLOAT:
        return arrow::field(table->getColumnName(colId), arrow::list(arrow::float32()));
    case DT_DOUBLE:
        return arrow::field(table->getColumnName(colId), arrow::list(arrow::float64()));
    case DT_DATEHOUR:
        return arrow::field(table->getColumnName(colId), arrow::list(arrow::timestamp(arrow::TimeUnit::SECOND)));
    case DT_IP:
        return arrow::field(table->getColumnName(colId), arrow::list(arrow::utf8()));
    case DT_INT128:
    case DT_UUID:
        return arrow::field(table->getColumnName(colId), arrow::list(arrow::fixed_size_binary(16)));
    default:
        throw RuntimeException("Not support this type. " + Util::getDataTypeString(colType));
        break;
    }
}

void ArrowTableMarshall::buildSchema(const TableSP& table){
    int numCols = table->columns();
    vector<std::shared_ptr<arrow::Field>> schemaVector(numCols);
    for (int i = 0; i < numCols; ++i) {
        DATA_TYPE dt = table->getColumn(i)->getType();
        if (dt >= ARRAY_TYPE_BASE) {
            schemaVector[i] = getArrayVectorColSchema(table, i, dt);
        }
        else {
            schemaVector[i] = getVectorColSchema(table, i, dt);
        }
	}
	schema_ = arrow::schema(schemaVector);
}

void ArrowTableMarshall::buildDictIdsMap(const TableSP& table){
    int numCols = target_->columns();
    int id = 0;
    for (int i = 0; i < numCols; ++i) {
        if(target_->getColumn(i)->getType() == DT_SYMBOL){
            dictIdsMap_[id++] = i;
        }
    }
}

IO_ERR ArrowTableMarshall::sendDictionary(){
    for(auto iter = dictIdsMap_.find(dictionariesSent_); iter != dictIdsMap_.end(); ++iter){
        SymbolBaseSP base = target_->getColumn(iter->second)->getSymbolBase();
        if(base.isNull()){
            throw RuntimeException("column symbol Base is Null " + std::to_string(iter->second));
        }
        arrow::StringBuilder builder;
        builder.Reserve(base->size());
        for(int j = 0; j < base->size(); ++j){
            builder.Append(base->getSymbol(j).getString());
        }
        auto arrayResult = builder.Finish();
        if(!arrayResult.ok()){
            throw RuntimeException("Fail to Finish builder in sendDictionary");
        }
        auto array = *arrayResult;

        arrow::ipc::IpcPayload payload;
        auto status = arrow::ipc::GetDictionaryPayload(iter->first, array, arrow::ipc::IpcWriteOptions::Defaults(), &payload);
        if(!status.ok()){
            throw RuntimeException("Fail to GetDictionaryPayload in sendDictionary");
        }
        stream_->Reset();
        status = writer_->WritePayload(payload);
        if(!status.ok()){
            throw RuntimeException("Fail to GetDictionaryPayload in sendDictionary");
        }

        auto bufferResult = stream_->Finish();
        if (!bufferResult.ok())
            throw RuntimeException("Fail to get buffer from outputStream.");
        auto buf = *bufferResult;
        IO_ERR ret = OK;
        ++dictionariesSent_;
        if ((ret = out_.start((char *)buf->data(), buf->size())) != OK)
            return ret;
    }
    isDictionarySent_ = true;
    return OK;
}

void MarshalVectorColumn(std::shared_ptr<arrow::RecordBatchBuilder> &batch_builder, VectorSP column, int columnId, int real_size, int rowsSent, char *pvalidBuffer, DATA_TYPE colType) {
    switch (colType)
    {
    case DT_BOOL: {
        auto builder = static_cast<arrow::BooleanBuilder*>(batch_builder->GetField(columnId));
        std::unique_ptr<char[]> buffer(new char[real_size]);
        const char *pbuf = column->getBoolConst(rowsSent, real_size, buffer.get());
        builder->AppendValues(reinterpret_cast<const uint8_t*>(pbuf), real_size, reinterpret_cast<uint8_t*>(pvalidBuffer));
        break;
    }
    case DT_CHAR: {
        auto builder = static_cast<arrow::Int8Builder*>(batch_builder->GetField(columnId));
        std::unique_ptr<char[]> buffer(new char[real_size]);
        const char *pbuf = column->getCharConst(rowsSent, real_size, buffer.get());
        builder->AppendValues(reinterpret_cast<const int8_t*>(pbuf), real_size, reinterpret_cast<uint8_t*>(pvalidBuffer));
        break;
    }
    case DT_SHORT: {
        auto builder = static_cast<arrow::Int16Builder*>(batch_builder->GetField(columnId));
        std::unique_ptr<short[]> buffer(new short[real_size]);
        const short *pbuf = column->getShortConst(rowsSent, real_size, buffer.get());
        builder->AppendValues(pbuf, real_size, reinterpret_cast<uint8_t*>(pvalidBuffer));
        break;
    }
    case DT_INT: {
        auto builder = static_cast<arrow::Int32Builder*>(batch_builder->GetField(columnId));
        std::unique_ptr<int[]> buffer(new int[real_size]);
        const int *pbuf = column->getIntConst(rowsSent, real_size, buffer.get());
        builder->AppendValues(pbuf, real_size, reinterpret_cast<uint8_t*>(pvalidBuffer));
        break;
    }
    case DT_LONG: {
        auto builder = static_cast<arrow::Int64Builder*>(batch_builder->GetField(columnId));
        std::unique_ptr<long long[]> buffer(new long long[real_size]);
        const long long *pbuf = column->getLongConst(rowsSent, real_size, buffer.get());
#if defined(__linux__)
        builder->AppendValues(reinterpret_cast<const int64_t*>(pbuf), real_size, reinterpret_cast<uint8_t*>(pvalidBuffer));
#elif defined(_WIN32)
        builder->AppendValues(pbuf, real_size, reinterpret_cast<uint8_t*>(pvalidBuffer));
#endif
        break;
    }
    case DT_MONTH: {
        auto builder = static_cast<arrow::Date32Builder*>(batch_builder->GetField(columnId));
        std::unique_ptr<int[]> buffer(new int[real_size]);
        std::unique_ptr<int[]> tbuf(new int[real_size]);
        const int *pbuf = column->getIntConst(rowsSent, real_size, buffer.get());
        for(INDEX ni=0;ni<real_size;++ni)
            tbuf[ni] = Util::countDays(pbuf[ni]/12, pbuf[ni]%12+1, 1);
        builder->AppendValues(tbuf.get(), real_size, reinterpret_cast<uint8_t*>(pvalidBuffer));
        break;
    }
    case DT_DATE: {
        auto builder = static_cast<arrow::Date32Builder*>(batch_builder->GetField(columnId));
        std::unique_ptr<int[]> buffer(new int[real_size]);
        const int *pbuf = column->getIntConst(rowsSent, real_size, buffer.get());
        builder->AppendValues(pbuf, real_size, reinterpret_cast<uint8_t*>(pvalidBuffer));
        break;
    }
    case DT_TIME:
    case DT_SECOND: {
        auto builder = static_cast<arrow::Time32Builder*>(batch_builder->GetField(columnId));
        std::unique_ptr<int[]> buffer(new int[real_size]);
        const int *pbuf = column->getIntConst(rowsSent, real_size, buffer.get());
        builder->AppendValues(pbuf, real_size, reinterpret_cast<uint8_t*>(pvalidBuffer));
        break;
    }
    case DT_MINUTE: {
        auto builder = static_cast<arrow::Time32Builder*>(batch_builder->GetField(columnId));
        std::unique_ptr<int[]> intBuffer(new int[real_size]);
        std::unique_ptr<int[]> buffer(new int[real_size]);
        const int *pbuf = column->getIntConst(rowsSent, real_size, intBuffer.get());
        for(INDEX ni=0;ni<real_size;++ni) 
            buffer[ni] = 60 * pbuf[ni];
        builder->AppendValues(buffer.get(), real_size, reinterpret_cast<uint8_t*>(pvalidBuffer));
        break;
    }
    case DT_DATETIME: {
        auto builder = static_cast<arrow::TimestampBuilder*>(batch_builder->GetField(columnId));
        std::unique_ptr<int[]> intBuffer(new int[real_size]);
        std::unique_ptr<int64_t[]> longBuffer(new int64_t[real_size]);
        const int *pbuf = column->getIntConst(rowsSent, real_size, intBuffer.get());
        for(INDEX ni=0;ni<real_size;++ni)
            longBuffer[ni] = pbuf[ni];
        builder->AppendValues(longBuffer.get(), real_size, reinterpret_cast<uint8_t*>(pvalidBuffer));
        break;
    }
    case DT_TIMESTAMP:
    case DT_NANOTIMESTAMP: {
        auto builder = static_cast<arrow::TimestampBuilder*>(batch_builder->GetField(columnId));
        std::unique_ptr<long long[]> buffer(new long long[real_size]);
        const long long *pbuf = column->getLongConst(rowsSent, real_size, buffer.get());
#if defined(__linux__)
        builder->AppendValues(reinterpret_cast<const int64_t*>(pbuf), real_size, reinterpret_cast<uint8_t*>(pvalidBuffer));
#elif defined(_WIN32)
        builder->AppendValues(pbuf, real_size, reinterpret_cast<uint8_t*>(pvalidBuffer));
#endif
        break;
    }
    case DT_SYMBOL: {
        auto builder = static_cast<arrow::StringDictionaryBuilder*>(batch_builder->GetField(columnId));
        std::unique_ptr<int[]> buffer(new int[real_size]);
        std::unique_ptr<int64_t[]> longBuffer(new int64_t[real_size]);
        const int *pbuf = column->getIntConst(rowsSent, real_size, buffer.get());
        for(INDEX ni=0;ni<real_size;++ni) 
            longBuffer[ni] = pbuf[ni];
        builder->AppendIndices(longBuffer.get(), real_size, reinterpret_cast<uint8_t*>(pvalidBuffer));
        break;
    }
    case DT_NANOTIME: {
        auto builder = static_cast<arrow::Time64Builder*>(batch_builder->GetField(columnId));
        std::unique_ptr<long long[]> buffer(new long long[real_size]);
        const long long *pbuf = column->getLongConst(rowsSent, real_size, buffer.get());
#if defined(__linux__)
        builder->AppendValues(reinterpret_cast<const int64_t*>(pbuf), real_size, reinterpret_cast<uint8_t*>(pvalidBuffer));
#elif defined(_WIN32)
        builder->AppendValues(pbuf, real_size, reinterpret_cast<uint8_t*>(pvalidBuffer));
#endif
        break;
    }
    case DT_DATEHOUR: {
        auto builder = static_cast<arrow::TimestampBuilder*>(batch_builder->GetField(columnId));
        std::unique_ptr<int[]> buffer(new int[real_size]);
        std::unique_ptr<int64_t[]> tbuf(new int64_t[real_size]);
        const int *pbuf = column->getIntConst(rowsSent, real_size, buffer.get());
        for(INDEX ni=0;ni<real_size;++ni)
            tbuf[ni] = pbuf[ni] * 3600LL;
        builder->AppendValues(tbuf.get(), real_size, reinterpret_cast<uint8_t*>(pvalidBuffer));
        break;
    }
    case DT_FLOAT: {
        auto builder = static_cast<arrow::FloatBuilder*>(batch_builder->GetField(columnId));
        std::unique_ptr<float[]> buffer(new float[real_size]);
        const float *pbuf = column->getFloatConst(rowsSent, real_size, buffer.get());
        builder->AppendValues(pbuf, real_size, reinterpret_cast<uint8_t*>(pvalidBuffer));
        break;
    }
    case DT_DOUBLE: {
        auto builder = static_cast<arrow::DoubleBuilder*>(batch_builder->GetField(columnId));
        std::unique_ptr<double[]> buffer(new double[real_size]);
        const double *pbuf = column->getDoubleConst(rowsSent, real_size, buffer.get());
        builder->AppendValues(pbuf, real_size, reinterpret_cast<uint8_t*>(pvalidBuffer));
        break;
    }
    case DT_STRING: {
        auto builder = static_cast<arrow::StringBuilder*>(batch_builder->GetField(columnId));
        std::unique_ptr<char*[]> buffer(new char*[real_size]);
        char** pbuf = column->getStringConst(rowsSent, real_size, buffer.get());
        builder->AppendValues(const_cast<const char**>(pbuf), real_size, reinterpret_cast<uint8_t*>(pvalidBuffer));
        break;
    }
    case DT_BLOB: {
        auto builder = static_cast<arrow::LargeBinaryBuilder*>(batch_builder->GetField(columnId));
        std::unique_ptr<char*[]> buffer(new char*[real_size]);
        char** pbuf = column->getStringConst(rowsSent, real_size, buffer.get());
        builder->AppendValues(const_cast<const char**>(pbuf), real_size, reinterpret_cast<uint8_t*>(pvalidBuffer));
        break;
    }
    case DT_INT128:
    case DT_UUID: {
        auto builder = static_cast<arrow::FixedSizeBinaryBuilder*>(batch_builder->GetField(columnId));
        std::unique_ptr<unsigned char[]> buffer(new unsigned char[real_size * 16]);
        std::unique_ptr<unsigned char[]> vbuffer(new unsigned char[real_size * 16]);
        const unsigned char* pbuf = column->getBinaryConst(rowsSent, real_size, 16, buffer.get());
        unsigned char* vbuf = vbuffer.get();
        for (int i = 0; i < real_size; ++i) {
            vbuf[i*16 + 0 ] = pbuf[i*16 + 15];
            vbuf[i*16 + 1 ] = pbuf[i*16 + 14];
            vbuf[i*16 + 2 ] = pbuf[i*16 + 13];
            vbuf[i*16 + 3 ] = pbuf[i*16 + 12];
            vbuf[i*16 + 4 ] = pbuf[i*16 + 11];
            vbuf[i*16 + 5 ] = pbuf[i*16 + 10];
            vbuf[i*16 + 6 ] = pbuf[i*16 + 9 ];
            vbuf[i*16 + 7 ] = pbuf[i*16 + 8 ];
            vbuf[i*16 + 8 ] = pbuf[i*16 + 7 ];
            vbuf[i*16 + 9 ] = pbuf[i*16 + 6 ];
            vbuf[i*16 + 10] = pbuf[i*16 + 5 ];
            vbuf[i*16 + 11] = pbuf[i*16 + 4 ];
            vbuf[i*16 + 12] = pbuf[i*16 + 3 ];
            vbuf[i*16 + 13] = pbuf[i*16 + 2 ];
            vbuf[i*16 + 14] = pbuf[i*16 + 1 ];
            vbuf[i*16 + 15] = pbuf[i*16 + 0 ];
        }
        builder->AppendValues(reinterpret_cast<const uint8_t*>(vbuf), real_size, reinterpret_cast<uint8_t*>(pvalidBuffer));
        break;
    }
    case DT_IP: {
        auto builder = static_cast<arrow::StringBuilder*>(batch_builder->GetField(columnId));
        std::unique_ptr<unsigned char[]> buffer(new unsigned char[real_size * 16]);
        const unsigned char* pbuf = column->getBinaryConst(rowsSent, real_size, 16, buffer.get());
        std::vector<std::string> ips(real_size);
        for(INDEX ni=0;ni<real_size;++ni)
            ips[ni] = IPAddr::toString(pbuf + ni * 16);
        builder->AppendValues(ips, reinterpret_cast<uint8_t*>(pvalidBuffer));
        break;
    }
    case DT_DECIMAL32: {
        auto builder = static_cast<arrow::Decimal128Builder*>(batch_builder->GetField(columnId));
        std::unique_ptr<int[]> intBuffer(new int[real_size]);
        std::unique_ptr<__int128_t[]> buffer(new __int128_t[real_size]);
        int scale = column->getExtraParamForType();
        const int *pbuf = column->getDecimal32Const(rowsSent, real_size, scale, intBuffer.get());
        for(INDEX ni=0;ni<real_size;++ni)
            buffer[ni] = static_cast<__int128_t>(pbuf[ni]);
        builder->AppendValues(reinterpret_cast<uint8_t*>(buffer.get()), real_size, reinterpret_cast<uint8_t*>(pvalidBuffer));
        break;
    }
    case DT_DECIMAL64: {
        auto builder = static_cast<arrow::Decimal128Builder*>(batch_builder->GetField(columnId));
        std::unique_ptr<long long[]> longBuffer(new long long[real_size]);
        std::unique_ptr<__int128_t[]> buffer(new __int128_t[real_size]);
        int scale = column->getExtraParamForType();
        const long long *pbuf = column->getDecimal64Const(rowsSent, real_size, scale, longBuffer.get());
        for(INDEX ni=0;ni<real_size;++ni)
            buffer[ni] = static_cast<__int128_t>(pbuf[ni]);
        builder->AppendValues(reinterpret_cast<uint8_t*>(buffer.get()), real_size, reinterpret_cast<uint8_t*>(pvalidBuffer));
        break;
    }
    default: {
        throw RuntimeException("Not Support This Type " + Util::getDataTypeString(colType));
        break;
    }
    }
}

void MarshalArrayVectorColumn(std::shared_ptr<arrow::RecordBatchBuilder> &batch_builder, VectorSP column, int columnId, int real_size, int rowsSent, char *pvalidBuffer, DATA_TYPE colType) {
    auto builder = static_cast<arrow::ListBuilder*>(batch_builder->GetField(columnId));
    std::unique_ptr<INDEX[]> indbuf(new INDEX[real_size]);
    std::unique_ptr<INDEX[]> offset(new INDEX[real_size+1]);

    VectorSP indexVector = ((SmartPointer<FastArrayVector>)column)->getSourceIndex();
    VectorSP valueVector = ((SmartPointer<FastArrayVector>)column)->getSourceValue();
    
    const INDEX *pindbuf = indexVector->getIndexConst(rowsSent, real_size, indbuf.get());
    memcpy(offset.get()+1, pindbuf, real_size*sizeof(INDEX));
    offset[0] = 0;
    int lastv = rowsSent == 0 ? 0 : indexVector->getIndex(rowsSent-1);
    if (lastv != 0) {
        for (int i = 1; i <= real_size; ++i) {
            offset[i] = offset[i] - lastv;
        }
    }

    int real_len = offset[real_size] - offset[0];
    
    std::unique_ptr<char[]> validBuffer(new char[real_len]);
    valueVector->isValid(offset[0], real_len, validBuffer.get());
    
    builder->AppendValues(reinterpret_cast<arrow::ListType::offset_type*>(offset.get()), real_size);

    switch (colType)
    {
    case DT_BOOL: {
        arrow::BooleanBuilder* subBuilder = static_cast<arrow::BooleanBuilder*>(builder->value_builder());
        std::unique_ptr<char[]> valueBuffer(new char[real_len]);
        const char *pbuf = valueVector->getBoolConst(offset[0], real_len, valueBuffer.get());
        subBuilder->AppendValues(reinterpret_cast<const uint8_t*>(pbuf), real_len, reinterpret_cast<uint8_t*>(validBuffer.get()));
        break;
    }
    case DT_CHAR: {
        arrow::Int8Builder* subBuilder = static_cast<arrow::Int8Builder*>(builder->value_builder());
        std::unique_ptr<char[]> valueBuffer(new char[real_len]);
        const char *pbuf = valueVector->getCharConst(offset[0], real_len, valueBuffer.get());
        subBuilder->AppendValues(reinterpret_cast<const int8_t*>(pbuf), real_len, reinterpret_cast<uint8_t*>(validBuffer.get()));
        break;
    }
    case DT_SHORT: {
        arrow::Int16Builder* subBuilder = static_cast<arrow::Int16Builder*>(builder->value_builder());
        std::unique_ptr<short[]> valueBuffer(new short[real_len]);
        const short *pbuf = valueVector->getShortConst(offset[0], real_len, valueBuffer.get());
        subBuilder->AppendValues(pbuf, real_len, reinterpret_cast<uint8_t*>(validBuffer.get()));
        break;
    }
    case DT_INT: {
        arrow::Int32Builder* subBuilder = static_cast<arrow::Int32Builder*>(builder->value_builder());
        std::unique_ptr<int[]> valueBuffer(new int[real_len]);
        const int *pbuf = valueVector->getIntConst(offset[0], real_len, valueBuffer.get());
        subBuilder->AppendValues(pbuf, real_len, reinterpret_cast<uint8_t*>(validBuffer.get()));
        break;
    }
    case DT_LONG: {
        arrow::Int64Builder* subBuilder = static_cast<arrow::Int64Builder*>(builder->value_builder());
        std::unique_ptr<long long[]> valueBuffer(new long long[real_len]);
        const long long *pbuf = valueVector->getLongConst(offset[0], real_len, valueBuffer.get());
#if defined(__linux__)
        subBuilder->AppendValues(reinterpret_cast<const int64_t*>(pbuf), real_len, reinterpret_cast<uint8_t*>(validBuffer.get()));
#elif defined(_WIN32)
        subBuilder->AppendValues(pbuf, real_len, reinterpret_cast<uint8_t*>(validBuffer.get()));
#endif
        break;
    }
    case DT_MONTH: {
        arrow::Date32Builder* subBuilder = static_cast<arrow::Date32Builder*>(builder->value_builder());
        std::unique_ptr<int[]> intBuffer(new int[real_len]);
        std::unique_ptr<int[]> valueBuffer(new int[real_len]);
        const int *pbuf = valueVector->getIntConst(offset[0], real_len, intBuffer.get());
        for(INDEX ni=0;ni<real_len;++ni)
            valueBuffer[ni] = Util::countDays(pbuf[ni]/12, pbuf[ni]%12+1, 1);
        subBuilder->AppendValues(valueBuffer.get(), real_len, reinterpret_cast<uint8_t*>(validBuffer.get()));
        break;
    }
    case DT_DATE: {
        arrow::Date32Builder* subBuilder = static_cast<arrow::Date32Builder*>(builder->value_builder());
        std::unique_ptr<int[]> valueBuffer(new int[real_len]);
        const int *pbuf = valueVector->getIntConst(offset[0], real_len, valueBuffer.get());
        subBuilder->AppendValues(pbuf, real_len, reinterpret_cast<uint8_t*>(validBuffer.get()));
        break;
    }
    case DT_TIME:
    case DT_SECOND: {
        arrow::Time32Builder* subBuilder = static_cast<arrow::Time32Builder*>(builder->value_builder());
        std::unique_ptr<int[]> valueBuffer(new int[real_len]);
        const int *pbuf = valueVector->getIntConst(offset[0], real_len, valueBuffer.get());
        subBuilder->AppendValues(pbuf, real_len, reinterpret_cast<uint8_t*>(validBuffer.get()));
        break;
    }
    case DT_MINUTE: {
        arrow::Time32Builder* subBuilder = static_cast<arrow::Time32Builder*>(builder->value_builder());
        std::unique_ptr<int[]> intBuffer(new int[real_len]);
        std::unique_ptr<int[]> valueBuffer(new int[real_len]);
        const int *pbuf = valueVector->getIntConst(offset[0], real_len, intBuffer.get());
        for(INDEX ni=0;ni<real_len;++ni)
            valueBuffer[ni] = 60 * pbuf[ni];
        subBuilder->AppendValues(valueBuffer.get(), real_len, reinterpret_cast<uint8_t*>(validBuffer.get()));
        break;
    }
    case DT_DATETIME: {
        arrow::TimestampBuilder* subBuilder = static_cast<arrow::TimestampBuilder*>(builder->value_builder());
        std::unique_ptr<int[]> intBuffer(new int[real_len]);
        std::unique_ptr<int64_t[]> valueBuffer(new int64_t[real_len]);
        const int *pbuf = valueVector->getIntConst(offset[0], real_len, intBuffer.get());
        for(INDEX ni=0;ni<real_len;++ni)
            valueBuffer[ni] = static_cast<int64_t>(pbuf[ni]);
        subBuilder->AppendValues(valueBuffer.get(), real_len, reinterpret_cast<uint8_t*>(validBuffer.get()));
        break;
    }
    case DT_TIMESTAMP:
    case DT_NANOTIMESTAMP: {
        arrow::TimestampBuilder* subBuilder = static_cast<arrow::TimestampBuilder*>(builder->value_builder());
        std::unique_ptr<long long[]> valueBuffer(new long long[real_len]);
        const long long *pbuf = valueVector->getLongConst(offset[0], real_len, valueBuffer.get());
#if defined(__linux__)
        subBuilder->AppendValues(reinterpret_cast<const int64_t*>(pbuf), real_len, reinterpret_cast<uint8_t*>(validBuffer.get()));
#elif defined(_WIN32)
        subBuilder->AppendValues(pbuf, real_len, reinterpret_cast<uint8_t*>(validBuffer.get()));
#endif
        break;
    }
    case DT_NANOTIME: {
        arrow::Time64Builder* subBuilder = static_cast<arrow::Time64Builder*>(builder->value_builder());
        std::unique_ptr<long long[]> valueBuffer(new long long[real_len]);
        const long long *pbuf = valueVector->getLongConst(offset[0], real_len, valueBuffer.get());
#if defined(__linux__)
        subBuilder->AppendValues(reinterpret_cast<const int64_t*>(pbuf), real_len, reinterpret_cast<uint8_t*>(validBuffer.get()));
#elif defined(_WIN32)
        subBuilder->AppendValues(pbuf, real_len, reinterpret_cast<uint8_t*>(validBuffer.get()));
#endif
        break;
    }
    case DT_DATEHOUR: {
        arrow::TimestampBuilder* subBuilder = static_cast<arrow::TimestampBuilder*>(builder->value_builder());
        std::unique_ptr<int[]> intBuffer(new int[real_len]);
        std::unique_ptr<int64_t[]> valueBuffer(new int64_t[real_len]);
        const int *pbuf = valueVector->getIntConst(offset[0], real_len, intBuffer.get());
        for(INDEX ni=0;ni<real_len;++ni)
            valueBuffer[ni] = pbuf[ni] * 3600LL;
        subBuilder->AppendValues(valueBuffer.get(), real_len, reinterpret_cast<uint8_t*>(validBuffer.get()));
        break;
    }
    case DT_FLOAT: {
        arrow::FloatBuilder* subBuilder = static_cast<arrow::FloatBuilder*>(builder->value_builder());
        std::unique_ptr<float[]> valueBuffer(new float[real_len]);
        const float *pbuf = valueVector->getFloatConst(offset[0], real_len, valueBuffer.get());
        subBuilder->AppendValues(pbuf, real_len, reinterpret_cast<uint8_t*>(validBuffer.get()));
        break;
    }
    case DT_DOUBLE: {
        arrow::DoubleBuilder* subBuilder = static_cast<arrow::DoubleBuilder*>(builder->value_builder());
        std::unique_ptr<double[]> valueBuffer(new double[real_len]);
        const double *pbuf = valueVector->getDoubleConst(offset[0], real_len, valueBuffer.get());
        subBuilder->AppendValues(pbuf, real_len, reinterpret_cast<uint8_t*>(validBuffer.get()));
        break;
    }
    case DT_INT128:
    case DT_UUID: {
        arrow::FixedSizeBinaryBuilder* subBuilder = static_cast<arrow::FixedSizeBinaryBuilder*>(builder->value_builder());
        std::unique_ptr<unsigned char[]> valueBuffer(new unsigned char[real_len * 16]);
        std::unique_ptr<unsigned char[]> vbuffer(new unsigned char[real_len * 16]);
        const unsigned char *pbuf = valueVector->getBinaryConst(offset[0], real_len, 16, valueBuffer.get());
        unsigned char* vbuf = vbuffer.get();
        for (int i = 0; i < real_len; ++i) {
            vbuf[i*16 + 0 ] = pbuf[i*16 + 15];
            vbuf[i*16 + 1 ] = pbuf[i*16 + 14];
            vbuf[i*16 + 2 ] = pbuf[i*16 + 13];
            vbuf[i*16 + 3 ] = pbuf[i*16 + 12];
            vbuf[i*16 + 4 ] = pbuf[i*16 + 11];
            vbuf[i*16 + 5 ] = pbuf[i*16 + 10];
            vbuf[i*16 + 6 ] = pbuf[i*16 + 9 ];
            vbuf[i*16 + 7 ] = pbuf[i*16 + 8 ];
            vbuf[i*16 + 8 ] = pbuf[i*16 + 7 ];
            vbuf[i*16 + 9 ] = pbuf[i*16 + 6 ];
            vbuf[i*16 + 10] = pbuf[i*16 + 5 ];
            vbuf[i*16 + 11] = pbuf[i*16 + 4 ];
            vbuf[i*16 + 12] = pbuf[i*16 + 3 ];
            vbuf[i*16 + 13] = pbuf[i*16 + 2 ];
            vbuf[i*16 + 14] = pbuf[i*16 + 1 ];
            vbuf[i*16 + 15] = pbuf[i*16 + 0 ];
        }
        subBuilder->AppendValues(reinterpret_cast<const uint8_t*>(vbuf), real_len, reinterpret_cast<uint8_t*>(validBuffer.get()));
        break;
    }
    case DT_IP: {
        arrow::StringBuilder* subBuilder = static_cast<arrow::StringBuilder*>(builder->value_builder());
        std::unique_ptr<unsigned char[]> valueBuffer(new unsigned char[real_len * 16]);
        const unsigned char *pbuf = valueVector->getBinaryConst(offset[0], real_len, 16, valueBuffer.get());
        std::vector<std::string> ips(real_len);
        for(INDEX ni=0;ni<real_len;++ni)
            ips[ni] = IPAddr::toString(pbuf + ni*16);
        subBuilder->AppendValues(ips, reinterpret_cast<uint8_t*>(validBuffer.get()));
        break;
    }
    default: {
        throw RuntimeException("Not Support This Type " + Util::getDataTypeString(DATA_TYPE(colType+ARRAY_TYPE_BASE)));
        break;
    }
    }
}

IO_ERR ArrowTableMarshall::sendRecordBatch(){
    int numCols = target_->columns();
    int vecLen = target_->size();
    std::shared_ptr<char> validBuffer(new char[RECORDBATCH_SIZE], [](char *p) { delete[] p; });
    char *pvalidBuffer = validBuffer.get();
    while(rowsSent_ < vecLen) {
        int real_size = std::min(RECORDBATCH_SIZE, vecLen - rowsSent_);
        auto recordBatchRuilderResult = arrow::RecordBatchBuilder::Make(schema_, arrow::default_memory_pool(), real_size);
        if (!recordBatchRuilderResult.ok()) {
            throw RuntimeException("Fail to create Record Batch Builder");
        }

        std::shared_ptr<arrow::RecordBatchBuilder> batch_builder = *std::move(recordBatchRuilderResult);
        for (int i = 0; i < numCols; ++i) {
            VectorSP column = target_->getColumn(i);
            DATA_TYPE dt = column->getType();
            column->isValid(rowsSent_, real_size, pvalidBuffer);
            if (dt>=ARRAY_TYPE_BASE) {
                MarshalArrayVectorColumn(batch_builder, column, i, real_size, rowsSent_, pvalidBuffer, DATA_TYPE(dt-ARRAY_TYPE_BASE));
            } 
            else {
                MarshalVectorColumn(batch_builder, column, i, real_size, rowsSent_, pvalidBuffer, dt);
            }
        }

        std::shared_ptr<arrow::RecordBatch> batch;
        auto flushResult = batch_builder->Flush();
        if(!flushResult.ok()){
            throw RuntimeException("batch_builder flush Fail");
        }
        batch = *flushResult;

        stream_->Reset();

        arrow::ipc::IpcPayload payload;
        auto status = arrow::ipc::GetRecordBatchPayload(*batch, arrow::ipc::IpcWriteOptions::Defaults(), &payload);
        if(!status.ok()){
            throw RuntimeException("GetRecordBatchPayload Fail");
        }

        status = writer_->WritePayload(payload);
        if (!status.ok()){
            throw RuntimeException("WritePayload Fail");
        }
        auto bufferResult = stream_->Finish();
        if (!bufferResult.ok())
            throw RuntimeException("Fail to get buffer from outputStream.");
        auto buf = *bufferResult;
        rowsSent_ += real_size;
        IO_ERR ret = OK;
        if ((ret = out_.start((char *)buf->data(), buf->size())) != OK)
            return ret;
    }
    isRecorderSent_ = true;
    return OK;
}

IO_ERR ArrowTableMarshall::sendSchema(){
    stream_->Reset();
    arrow::ipc::IpcPayload payload;
    arrow::ipc::DictionaryFieldMapper mapper(*schema_);
    auto status = arrow::ipc::GetSchemaPayload(*schema_, arrow::ipc::IpcWriteOptions::Defaults(), mapper, &payload);
    if(!status.ok()){
        throw RuntimeException("Fail to GetSchemaPayload.");
    }
    status = writer_->WritePayload(payload);
    if(!status.ok()){
        throw RuntimeException("Fail to WritePayload.");
    }

    auto bufferResult = stream_->Finish();
    if (!bufferResult.ok())
        throw RuntimeException("Fail to Finish stream_.");
    auto buf = *bufferResult;
    isSchemaSent_ = true;

    return out_.start((char *)buf->data(), buf->size());
}

IO_ERR ArrowTableMarshall::sendEnd(){
    isEndSent_ = true;
    unsigned long long end = 0xFFFFFFFF00000000;
    return out_.start((char *)(&end), sizeof(end));
}

bool ArrowTableMarshall::resume(IO_ERR& ret) {
    if(complete_){
        return (ret = out_.getDataOutputStream()->flush()) == OK;
    }
    if(out_.size() > 0){
        if((ret = out_.resume()) != OK) {
            return false;
        }
    }
    if(!isSchemaSent_){
        ret = sendSchema();
        if(ret != OK){
            return false;
        }
    }
    if(!isDictionarySent_){
        ret = sendDictionary();
        if(ret != OK){
            return false;
        }
    }
    if(!isRecorderSent_){
        ret = sendRecordBatch();
        if(ret != OK){
            return false;
        }
    }
    if(!isEndSent_){
        ret = sendEnd();
        if(ret != OK){
            return false;
        }
    }
    return true;
}
