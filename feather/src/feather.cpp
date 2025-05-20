#include "feather.h"

#include <vector>
#include <map>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Concurrent.h>
#include <Logger.h>

#include "arrow/ipc/feather.h"
#include "arrow/io/interfaces.h"
#include "arrow/result.h"
#include "arrow/buffer.h"
#include "arrow/table.h"
#include "arrow/chunked_array.h"
#include "arrow/array.h"
#include "arrow/array/data.h"
#include "arrow/status.h"
#include "arrow/io/file.h"
#include "arrow/array/builder_primitive.h"
#include "arrow/array/builder_binary.h"
#include "ddbplugin/PluginLogger.h"
#include "ddbplugin/PluginLoggerImp.h"

using namespace arrow;
using std::cout;
using std::endl;
using std::vector;

template <typename T>
Status getDataBuffer(std::vector<T>& colVec, std::vector<int64_t>& nullVec, std::shared_ptr<ChunkedArray> chunks) {
    int64_t pos = 0;
    for(int j = 0; j < chunks->num_chunks(); j++) {
        std::shared_ptr<arrow::Array> chunk = chunks->chunk(j);
        std::shared_ptr<ArrayData> data = chunk->data();

        nullVec.reserve(nullVec.size() + chunk->null_count());
        const uint8_t* bitmap = chunk->null_bitmap_data();
        if(bitmap != 0) {
            for(int i = 0; i < chunk->length(); i++) {
                unsigned char ch = *(bitmap + (i / 8));
                if(((~ch) & (1 << (i % 8))) != 0) {
                    nullVec.push_back(pos);
                }
                pos++;
            }
        } else {
            pos += chunk->length();
        }
        // k should begin from 1, not 0
        // if the first buffer is not null, data would be mixed
        // primitive array's data locates at buffer 1
        std::shared_ptr<arrow::Buffer> vec = data->buffers[1];
        const T* buf = data->GetValues<T>(1, 0);
        if(buf == nullptr) {
            return Status(StatusCode::Invalid, ". Error occurred when reading from buffer.");
        }
        colVec.insert(colVec.end(), buf, buf + vec->size() / sizeof(T));
    }
    return arrow::Status::OK();
}

Status transCol(ConstantSP& retCol, std::shared_ptr<ChunkedArray> chunks, std::shared_ptr<DataType> type) {
    long long length = chunks->length();
    DATA_TYPE dolphinType = convertArrowToDolphinDB(type);
    if(dolphinType == DT_VOID) {
        return Status(StatusCode::TypeError, "unsupported type ");
    }
    retCol = Util::createVector(dolphinType, 0, length);
    switch(dolphinType) {
    case DT_BOOL:
    {
        std::vector<int64_t> nullVec;
        std::vector<char> colVec(length, 0);

        int64_t pos = 0;
        for(int j = 0; j < chunks->num_chunks(); j++) {
            std::shared_ptr<arrow::Array> chunk = chunks->chunk(j);
            std::shared_ptr<ArrayData> data = chunk->data();

            nullVec.reserve(nullVec.size() + chunk->null_count());
            const uint8_t* bitmap = chunk->null_bitmap_data();

            std::shared_ptr<arrow::Buffer> vec = data->buffers[1];
            const uint8_t* buf = data->GetValues<uint8_t>(1, 0);
            if(buf == nullptr) {
                return Status(StatusCode::Invalid, ". Error occurred when reading from buffer.");
            }
            for(int i = 0; i < chunk->length(); i++) {
                unsigned char ch = *(buf + (i / 8));
                if(((ch) & (1 << (i % 8))) != 0) {
                    colVec[pos] = 1;
                }
                if(bitmap != 0) {
                    unsigned char ch = *(bitmap + (i / 8));
                    if(((~ch) & (1 << (i % 8))) != 0) {
                        colVec[pos] = 128;
                    }
                }
                pos++;
            }
        }

        ((VectorSP)retCol)->appendBool((const char*)(colVec.data()), length);
        break;

    }
    case DT_CHAR:
    {
        std::vector<unsigned char> colVec;
        std::vector<int64_t> nullVec;
        ARROW_RETURN_NOT_OK(getDataBuffer<unsigned char>(colVec, nullVec, chunks));
        for(int i = 0; i < int(nullVec.size()); i++) {
            *reinterpret_cast<unsigned char*>(&colVec[nullVec[i]]) = 0x80;
        }
        ((VectorSP)retCol)->appendChar((const char*)(colVec.data()), length);
        break;
    }
    case DT_SHORT:
    {
        if(type->id() == arrow::Type::type::UINT8) {
            std::vector<unsigned char> dataVec;
            std::vector<int64_t> nullVec;
            ARROW_RETURN_NOT_OK(getDataBuffer<unsigned char>(dataVec, nullVec, chunks));
            std::vector<short> colVec;
            colVec.reserve(dataVec.size());
            for(uint32_t i = 0; i < dataVec.size(); i++) {
                colVec.push_back(dataVec[i]);
            }
            for(int i = 0; i < int(nullVec.size()); i++) {
                *reinterpret_cast<unsigned short*>(&colVec[nullVec[i]]) = 0x8000;
            }
            ((VectorSP)retCol)->appendShort((const short*)(colVec.data()), length);
            break;
        }
        std::vector<short> colVec;
        std::vector<int64_t> nullVec;
        ARROW_RETURN_NOT_OK(getDataBuffer<short>(colVec, nullVec, chunks));
        for(int i = 0; i < int(nullVec.size()); i++) {
            *reinterpret_cast<unsigned short*>(&colVec[nullVec[i]]) = 0x8000;
        }
        ((VectorSP)retCol)->appendShort((const short*)(colVec.data()), length);
        break;
    }
    case DT_INT:
    {
        if(type->id() == arrow::Type::type::UINT16) {
            std::vector<unsigned short> dataVec;
            std::vector<int64_t> nullVec;
            ARROW_RETURN_NOT_OK(getDataBuffer<unsigned short>(dataVec, nullVec, chunks));
            std::vector<int> colVec;
            colVec.reserve(dataVec.size());
            for(uint32_t i = 0; i < dataVec.size(); i++) {
                colVec.push_back(dataVec[i]);
            }
            for(int i = 0; i < int(nullVec.size()); i++) {
                *reinterpret_cast<unsigned int*>(&colVec[nullVec[i]]) = 0x80000000;
            }
            ((VectorSP)retCol)->appendInt((const int*)(colVec.data()), length);
            break;
        }
        std::vector<int> colVec;
        std::vector<int64_t> nullVec;
        ARROW_RETURN_NOT_OK(getDataBuffer<int>(colVec, nullVec, chunks));
        for(int i = 0; i < int(nullVec.size()); i++) {
            *reinterpret_cast<unsigned int*>(&colVec[nullVec[i]]) = 0x80000000;
        }
        ((VectorSP)retCol)->appendInt((const int*)(colVec.data()), length);
        break;
    }
    case DT_LONG:
    {
        if(type->id() == arrow::Type::type::UINT32) {
            std::vector<unsigned int> dataVec;
            std::vector<int64_t> nullVec;
            ARROW_RETURN_NOT_OK(getDataBuffer<uint32_t>(dataVec, nullVec, chunks));
            std::vector<long long> colVec;
            colVec.reserve(dataVec.size());
            for(uint32_t i = 0; i < dataVec.size(); i++) {
                colVec.push_back(dataVec[i]);
            }
            for(int i = 0; i < int(nullVec.size()); i++) {
                *reinterpret_cast<unsigned long long*>(&colVec[nullVec[i]]) = 0x8000000000000000;
            }
            ((VectorSP)retCol)->appendLong((const long long*)(colVec.data()), length);
            break;
        } else if(type->id() == arrow::Type::type::UINT64) {
            std::vector<uint64_t> dataVec;
            std::vector<int64_t> nullVec;
            ARROW_RETURN_NOT_OK(getDataBuffer<uint64_t>(dataVec, nullVec, chunks));
            std::vector<long long> colVec;
            colVec.reserve(dataVec.size());
            for(uint32_t i = 0; i < dataVec.size(); i++) {
                if(dataVec[i] > 9223372036854775807ULL) {
                    *reinterpret_cast<unsigned long long*>(&colVec[i]) = 0x8000000000000000;
                }
                colVec[i] = dataVec[i];
            }
            for(int i = 0; i < int(nullVec.size()); i++) {
                *reinterpret_cast<unsigned long long*>(&colVec[nullVec[i]]) = 0x8000000000000000;
            }
            ((VectorSP)retCol)->appendLong((const long long*)(colVec.data()), length);
            break;
        }
        std::vector<long long> colVec;
        std::vector<int64_t> nullVec;
        ARROW_RETURN_NOT_OK(getDataBuffer<long long>(colVec, nullVec, chunks));
        for(int i = 0; i < int(nullVec.size()); i++) {
            *reinterpret_cast<unsigned long long*>(&colVec[nullVec[i]]) = 0x8000000000000000;
        }
        ((VectorSP)retCol)->appendLong((const long long*)(colVec.data()), length);
        break;
    }
    case DT_FLOAT:
    {
        std::vector<float> colVec;
        std::vector<int64_t> nullVec;
        ARROW_RETURN_NOT_OK(getDataBuffer<float>(colVec, nullVec, chunks));
        for(int i = 0; i < int(nullVec.size()); i++) {
            *reinterpret_cast<unsigned int*>(&colVec[nullVec[i]]) = 0xff7fffff;
        }
        unsigned int nan = 0;
        for(int i = 23; i < 31; i++) {
            unsigned int k = 1;
            nan |= (k<<i);
        }
        for(int i = 0; i < int(colVec.size()); i++) {
            unsigned int val = *reinterpret_cast<unsigned int*>(&colVec[i]);
            if((val & nan) == nan) {
                *reinterpret_cast<unsigned int*>(&colVec[i]) = 0xff7fffff;
            }
        }
        ((VectorSP)retCol)->appendFloat((const float*)(colVec.data()), length);
        break;
    }
    case DT_DOUBLE:
    {

        std::vector<double> colVec;
        std::vector<int64_t> nullVec;
        ARROW_RETURN_NOT_OK(getDataBuffer<double>(colVec, nullVec, chunks));

        for(int i = 0; i < int(nullVec.size()); i++) {
            *reinterpret_cast<unsigned long long*>(&colVec[nullVec[i]]) = 0xffefffffffffffff;
        }

        unsigned long long nan = 0;
        for(int i = 52; i < 63; i++) {
            unsigned long long k = 1;
            nan |= (k<<i);
        }

        for(int i = 0; i < int(colVec.size()); i++) {
            unsigned long long val = *reinterpret_cast<unsigned long long*>(&colVec[i]);
            if((val & nan) == nan) {
                *reinterpret_cast<unsigned long long*>(&colVec[i]) = 0xffefffffffffffff;
            }
        }
        ((VectorSP)retCol)->appendDouble((const double*)(colVec.data()), length);

        break;
    }
    case DT_STRING:
    {
        int pos = 0;
        std::vector<string> colVec(length);
        for(int j = 0; j < chunks->num_chunks(); j++) {
            int64_t arrayLength = chunks->chunk(j)->length();
            const StringType::offset_type* offsets = std::static_pointer_cast<arrow::StringArray>(chunks->chunk(j))->raw_value_offsets();
            char * rawPtr = (char *)std::static_pointer_cast<arrow::StringArray>(chunks->chunk(j))->raw_data();
            for(int i = 0; i < arrayLength; i++) {
                int strLen = offsets[i+1] - offsets[i];
                colVec[pos].resize(strLen);
                if(strLen > 0) {
                    memcpy(&colVec[pos][0], rawPtr + offsets[i], strLen);
                }

                pos++;
            }
        }

        ((VectorSP)retCol)->appendString(colVec.data(), length);
        break;
    }
    case DT_DATE:
    {
        std::vector<int> colVec;
        std::vector<int64_t> nullVec;
        ARROW_RETURN_NOT_OK(getDataBuffer<int>(colVec, nullVec, chunks));
        for(int i = 0; i < int(nullVec.size()); i++) {
            *reinterpret_cast<unsigned int*>(&colVec[nullVec[i]]) = 0x80000000;
        }
        ((VectorSP)retCol)->appendInt((const int*)(colVec.data()), length);
        break;
    }
    case DT_TIMESTAMP:
    {
        std::vector<long long> colVec;
        std::vector<int64_t> nullVec;
        ARROW_RETURN_NOT_OK(getDataBuffer<long long>(colVec, nullVec, chunks));
        for(int i = 0; i < int(nullVec.size()); i++) {
            *reinterpret_cast<unsigned long long*>(&colVec[nullVec[i]]) = 0x8000000000000000;
        }
        ((VectorSP)retCol)->appendLong((const long long*)(colVec.data()), length);
        break;
    }
    case DT_NANOTIMESTAMP:
    {
        std::vector<long long> colVec;
        std::vector<int64_t> nullVec;
        ARROW_RETURN_NOT_OK(getDataBuffer<long long>(colVec, nullVec, chunks));
        for(int i = 0; i < int(nullVec.size()); i++) {
            *reinterpret_cast<unsigned long long*>(&colVec[nullVec[i]]) = 0x8000000000000000;
        }
        ((VectorSP)retCol)->appendLong((const long long*)(colVec.data()), length);
        break;
    }
    case DT_SECOND:
    {
        std::vector<int> colVec;
        std::vector<int64_t> nullVec;
        ARROW_RETURN_NOT_OK(getDataBuffer<int>(colVec, nullVec, chunks));
        for(int i = 0; i < int(nullVec.size()); i++) {
            *reinterpret_cast<unsigned int*>(&colVec[nullVec[i]]) = 0x80000000;
        }
        ((VectorSP)retCol)->appendInt((const int*)(colVec.data()), length);
        break;
    }
    case DT_TIME:
    {
        std::vector<int> colVec;
        std::vector<int64_t> nullVec;
        ARROW_RETURN_NOT_OK(getDataBuffer<int>(colVec, nullVec, chunks));
        for(int i = 0; i < int(nullVec.size()); i++) {
            *reinterpret_cast<unsigned int*>(&colVec[nullVec[i]]) = 0x80000000;
        }
        ((VectorSP)retCol)->appendInt((const int*)(colVec.data()), length);
        break;
    }
    case DT_NANOTIME:
    {
        std::vector<long long> colVec;
        std::vector<int64_t> nullVec;
        ARROW_RETURN_NOT_OK(getDataBuffer<long long>(colVec, nullVec, chunks));
        for(int i = 0; i < int(nullVec.size()); i++) {
            *reinterpret_cast<unsigned long long*>(&colVec[nullVec[i]]) = 0x8000000000000000;
        }
        ((VectorSP)retCol)->appendLong((const long long*)(colVec.data()), length);
        break;
    }
    default:
        return Status(StatusCode::TypeError, "unsupported type ");
    };

    retCol->setNullFlag(retCol->hasNull());
    return Status::OK();
}

class GetColRunnable: public Runnable {
    public:
        GetColRunnable(std::vector<ConstantSP>& cols, std::shared_ptr<ChunkedArray> chunks, std::shared_ptr<DataType> type, int index, Status& status): cols_(cols), chunks_(chunks), type_(type), index_(index), status_(status){
        }
    void run() {
        try {
            status_ = transCol(cols_[index_], chunks_, type_);
        } catch (std::exception &e) {
            string errMsg = e.what();
            status_ = Status(StatusCode::SerializationError, errMsg);
            PLUGIN_LOG_ERR(errMsg);
        } catch (...) {
            string errMsg = "Error occurs when get cols data.";
            status_ = Status(StatusCode::UnknownError, errMsg);
            PLUGIN_LOG_ERR(errMsg);
        }
    }
    private:
    std::vector<ConstantSP>& cols_;
    std::shared_ptr<ChunkedArray> chunks_;
    std::shared_ptr<DataType> type_;
    int index_;
    Status& status_;
};

using GetColRunnableSP = SmartPointer<GetColRunnable>;

Status loadFromFeather(string filePath, VectorSP columnToRead, TableSP& table) {

    ARROW_ASSIGN_OR_RAISE(const std::shared_ptr<io::MemoryMappedFile> mapFile, io::MemoryMappedFile::Open(filePath, io::FileMode::type::READ));
    ARROW_ASSIGN_OR_RAISE(long long fileLen, mapFile->GetSize());
    ARROW_ASSIGN_OR_RAISE(std::shared_ptr<arrow::Buffer> buf, mapFile->Read(fileLen));
    ARROW_ASSIGN_OR_RAISE(const std::shared_ptr<io::RandomAccessFile> file,arrow::Buffer::GetReader(buf));
    ARROW_ASSIGN_OR_RAISE(std::shared_ptr<ipc::feather::Reader> reader,ipc::feather::Reader::Open(file));

    // schema
    std::shared_ptr<Schema> schema = reader->schema();
    std::vector<string> colNames;
    std::vector<ConstantSP> cols;
    std::vector<std::shared_ptr<Field>> fields = schema->fields();
    std::shared_ptr<arrow::Table> outWhole;
    int colNum;

    std::vector<int> indexMap;
    if(columnToRead.isNull()) {
        int num_fields = schema->num_fields();
        colNames.resize(num_fields);
        cols.resize(num_fields);

        std::vector<std::shared_ptr<Field>> fields = schema->fields();
        for(int i = 0; i < num_fields; i++) {
            colNames[i] = fields[i]->name();
        }
        colNum = colNames.size();
        ARROW_RETURN_NOT_OK(reader->Read(&outWhole));
    } else {
        int num_fields = columnToRead->size();
        if(num_fields > schema->num_fields()) {
            throw IllegalArgumentException("load", "Too many indices to load");
        } else if(num_fields == 0) {
            throw IllegalArgumentException("load", "Too few indices to load");
        }

        colNames.resize(num_fields);
        cols.resize(num_fields);
        std::vector<int> colIndices(num_fields);
        std::map<string, int> colMap;

        std::vector<std::shared_ptr<Field>> fields = schema->fields();
        for(int i = 0; i < int(fields.size()); i++) {
            colMap[fields[i]->name()] = i;
        }
        for(int i = 0; i < num_fields; i++) {
            colNames[i] = columnToRead->getString(i);
            if(colMap.find(colNames[i]) == colMap.end()) {
                throw RuntimeException("Unrecognized column name " + colNames[i]);
            }
            colIndices[i] = colMap[colNames[i]];
        }
        colNum = colNames.size();
        vector<int> colSort = colIndices;
        sort(colSort.begin(), colSort.end());
        auto end = std::unique(colSort.begin(), colSort.end());
        if((unsigned int)(end-colSort.begin()) != colSort.size()) {
            return Status(StatusCode::IndexError, "The column names cannot be duplicate");
        }
        indexMap.resize(colNum);
        for(int i = 0; i < colNum; i++) {
            indexMap[i] = std::find(colSort.begin(), colSort.end(), colIndices[i]) - colSort.begin();
        }
        ARROW_RETURN_NOT_OK(reader->Read(colSort, &outWhole));
    }

    std::vector<std::shared_ptr<ChunkedArray>> columns = outWhole->columns();
    vector<ThreadSP> colThreads(colNum);
    vector<arrow::Status> statList(colNum);

    for(int i = 0; i < colNum; i++) {
        std::shared_ptr<DataType> type = columns[i]->type();
        GetColRunnableSP runnable = new GetColRunnable(cols, columns[i], type, i, statList[i]);
        ThreadSP thread = new Thread(runnable);
        colThreads[i] = thread;
        if(!thread->isStarted()) {
            thread->start();
        }
    }
    for(auto &thread: colThreads) {
        thread->join();
    }
    for(int i = 0; i < colNum; i++) {
        auto &stat = statList[i];
        if(!stat.ok()) {
            return Status(stat.code(), stat.message() + "in column " + colNames[i] + ".");
        }
    }

    if(columnToRead.isNull()) {
        table = Util::createTable(colNames, cols);
    } else {
        std::vector<ConstantSP> colsReMap(colNum);
        for(int i = 0; i < colNum; i++) {
            colsReMap[i] = cols[indexMap[i]];
        }
        table = Util::createTable(colNames, colsReMap);
    }
    return Status::OK();
}


ConstantSP loadFeather(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string("Usage: load(filePath, [columns]). ");

    if(args[0]->getType() != DT_STRING){
        throw IllegalArgumentException(__FUNCTION__, usage + "The parameter filePath must be a string.");
    }
    VectorSP columnToRead = SmartPointer<Vector>(0);
    if(args.size()==2 && !args[1]->isNull()) {
        if(!args[1]->isVector() || args[1]->getCategory() != LITERAL) {
            throw IllegalArgumentException(__FUNCTION__, usage + "The parameter columns must be a string vector.");
        }
        columnToRead = args[1];
    }
    std::string filePath = args[0]->getString();
    TableSP table;
    Status status = loadFromFeather(filePath, columnToRead, table);
    if (!status.ok()) {
        throw RuntimeException(status.ToString());
    }
    return table;
}

Status getSchema(string filePath, TableSP& table) {
    ARROW_ASSIGN_OR_RAISE(const std::shared_ptr<io::MemoryMappedFile> mapFile, io::MemoryMappedFile::Open(filePath, io::FileMode::type::READ));
    ARROW_ASSIGN_OR_RAISE(long long fileLen, mapFile->GetSize());
    ARROW_ASSIGN_OR_RAISE(std::shared_ptr<arrow::Buffer> buf, mapFile->Read(fileLen));
    ARROW_ASSIGN_OR_RAISE(const std::shared_ptr<io::RandomAccessFile> file,arrow::Buffer::GetReader(buf));
    ARROW_ASSIGN_OR_RAISE(std::shared_ptr<ipc::feather::Reader> reader,ipc::feather::Reader::Open(file));

    // schema
    std::shared_ptr<Schema> schema = reader->schema();
    std::vector<std::shared_ptr<Field>> fields = schema->fields();
    std::vector<string> names;
    std::vector<string> types;
    std::vector<string> ddbTypes;
    int length = fields.size();
    for(int i = 0; i < length; i++) {
        names.push_back((fields[i]->name()));
        types.push_back((fields[i]->type()->ToString()));
        ddbTypes.push_back(Util::getDataTypeString(convertArrowToDolphinDB(fields[i]->type())));
    }

    std::vector<string> colNames{"name", "type", "DolphinDBType"};
    std::vector<ConstantSP> cols(3);
    cols[0] = Util::createVector(DT_SYMBOL, 0, length, true);
    cols[1] = Util::createVector(DT_SYMBOL, 0, length, true);
    cols[2] = Util::createVector(DT_SYMBOL, 0, length, true);
    ((VectorSP)cols[0])->appendString((names.data()), length);
    ((VectorSP)cols[1])->appendString((types.data()), length);
    ((VectorSP)cols[2])->appendString((ddbTypes.data()), length);

    table = Util::createTable(colNames, cols);
    return Status::OK();
}

ConstantSP schemaFeather(Heap *heap, vector<ConstantSP> &args){
    const auto usage = string("Usage: schema(filePath). ");

    if(args[0]->getType() != DT_STRING){
        throw IllegalArgumentException(__FUNCTION__, usage + "The parameter filePath must be a string.");
    }
    std::string filePath = args[0]->getString();
    TableSP table;
    Status stat = getSchema(filePath, table);
    if(!stat.ok()) {
        throw RuntimeException(stat.ToString());
    }

    return table;
}

Status transCol(std::shared_ptr<arrow::Array>& dest, VectorSP dolphinCol, DATA_TYPE type) {
    int length = dolphinCol->size();
    std::shared_ptr<arrow::Array> array;
    switch(type) {
    case DT_BOOL:
    {
        std::vector<uint8_t> vecBool(length);
        char * buf = (char*)vecBool.data();
        dolphinCol->getBool(0, length, buf);

        std::vector<bool> isValid(length, true);
        std::vector<bool> bools(length);
        for(int i = 0; i < length; i++) {
            if(vecBool[i] == 128) {
                isValid[i] = false;
            } else if(vecBool[i] == 1) {
                bools[i] = true;
            } else {
                bools[i] = false;
            }
        }

        arrow::BooleanBuilder builder;
        ARROW_RETURN_NOT_OK(builder.Resize(length));
        ARROW_RETURN_NOT_OK(builder.AppendValues(bools, isValid));
        ARROW_RETURN_NOT_OK(builder.Finish(&array));
        break;
    }
    case DT_CHAR:
    {
        std::vector<uint8_t> vecSrc(length);
        char * buf = (char*)vecSrc.data();
        dolphinCol->getChar(0, length, (buf));

        std::vector<bool> isValid(length, true);
        if(dolphinCol->hasNull()) {
            for(int i = 0; i < length; i++) {
                if(*reinterpret_cast<char*>(&vecSrc[i]) == -128) {
                    isValid[i] = false;
                }
            }
        }

        arrow::NumericBuilder<arrow::UInt8Type> builder;
        ARROW_RETURN_NOT_OK(builder.Resize(length));
        ARROW_RETURN_NOT_OK(builder.AppendValues(vecSrc, isValid));
        ARROW_RETURN_NOT_OK(builder.Finish(&array));
        break;
    }
    case DT_SHORT:
    {
        std::vector<int16_t> vecSrc(length);
        short * buf = (short*)vecSrc.data();
        dolphinCol->getShort(0, length, (buf));
        std::vector<bool> isValid(length, true);
        if(dolphinCol->hasNull()) {
            for(int i = 0; i < length; i++) {
                if(vecSrc[i] == -32768) {
                    isValid[i] = false;
                }
            }
        }

        arrow::NumericBuilder<arrow::Int16Type> builder;
        ARROW_RETURN_NOT_OK(builder.Resize(length));
        ARROW_RETURN_NOT_OK(builder.AppendValues(vecSrc, isValid));
        ARROW_RETURN_NOT_OK(builder.Finish(&array));
        break;
    }
    case DT_INT:
    {
        std::vector<int32_t> vecSrc(length);
        int * buf = (int*)vecSrc.data();
        dolphinCol->getInt(0, length, (buf));
        std::vector<bool> isValid(length, true);
        if(dolphinCol->hasNull()) {
            for(int i = 0; i < length; i++) {
                if(vecSrc[i] == -2147483648) {
                    isValid[i] = false;
                }
            }
        }
        arrow::NumericBuilder<arrow::Int32Type> builder;
        ARROW_RETURN_NOT_OK(builder.Resize(length));
        ARROW_RETURN_NOT_OK(builder.AppendValues(vecSrc, isValid));
        ARROW_RETURN_NOT_OK(builder.Finish(&array));
        break;
    }
    case DT_LONG:
    {
        std::vector<int64_t> vecSrc(length);
        long long * buf = (long long*)vecSrc.data();
        dolphinCol->getLong(0, length, (buf));
        std::vector<bool> isValid(length, true);
        if(dolphinCol->hasNull()) {
            for(int i = 0; i < length; i++) {
                if((unsigned long long)vecSrc[i] == -9223372036854775808ULL) {
                    isValid[i] = false;
                }
            }
        }
        arrow::NumericBuilder<arrow::Int64Type> builder;
        ARROW_RETURN_NOT_OK(builder.Resize(length));
        ARROW_RETURN_NOT_OK(builder.AppendValues(vecSrc, isValid));
        ARROW_RETURN_NOT_OK(builder.Finish(&array));
        break;
    }
    case DT_DATE:
    {
        std::vector<int32_t> vecSrc(length);
        int * buf = (int*)vecSrc.data();
        dolphinCol->getInt(0, length, (buf));
        std::vector<bool> isValid(length, true);
        if(dolphinCol->hasNull()) {
            for(int i = 0; i < length; i++) {
                if(vecSrc[i] == -2147483648) {
                    isValid[i] = false;
                }
            }
        }
        arrow::NumericBuilder<arrow::Date32Type> builder;
        ARROW_RETURN_NOT_OK(builder.Resize(length));
        ARROW_RETURN_NOT_OK(builder.AppendValues(vecSrc, isValid));
        ARROW_RETURN_NOT_OK(builder.Finish(&array));
        break;
    }
    case DT_TIME:
    {
        std::vector<int32_t> vecSrc(length);
        int * buf = (int*)vecSrc.data();
        dolphinCol->getInt(0, length, (buf));
        std::vector<bool> isValid(length, true);
        if(dolphinCol->hasNull()) {
            for(int i = 0; i < length; i++) {
                if(vecSrc[i] == -2147483648) {
                    isValid[i] = false;
                }
            }
        }
        auto builder = arrow::NumericBuilder<arrow::Time32Type>(convertDolphinDBToArrow(DT_TIME), default_memory_pool());
        ARROW_RETURN_NOT_OK(builder.Resize(length));
        ARROW_RETURN_NOT_OK(builder.AppendValues(vecSrc, isValid));
        ARROW_RETURN_NOT_OK(builder.Finish(&array));
        break;
    }
    case DT_SECOND:
    {
        std::vector<int32_t> vecSrc(length);
        int * buf = (int*)vecSrc.data();
        dolphinCol->getInt(0, length, (buf));
        std::vector<bool> isValid(length, true);
        if(dolphinCol->hasNull()) {
            for(int i = 0; i < length; i++) {
                if(vecSrc[i] == -2147483648) {
                    isValid[i] = false;
                }
            }
        }
        auto builder = arrow::NumericBuilder<arrow::Time32Type>(convertDolphinDBToArrow(DT_SECOND), default_memory_pool());
        ARROW_RETURN_NOT_OK(builder.Resize(length));
        ARROW_RETURN_NOT_OK(builder.AppendValues(vecSrc, isValid));
        ARROW_RETURN_NOT_OK(builder.Finish(&array));
        break;
    }
    case DT_TIMESTAMP:
    {
        std::vector<int64_t> vecSrc(length);
        long long * buf = (long long*)vecSrc.data();
        dolphinCol->getLong(0, length, (buf));
        std::vector<bool> isValid(length, true);
        if(dolphinCol->hasNull()) {
            for(int i = 0; i < length; i++) {
                if((unsigned long long)vecSrc[i] == -9223372036854775808ULL) {
                    isValid[i] = false;
                }
            }
        }
        auto builder = arrow::NumericBuilder<arrow::TimestampType>(convertDolphinDBToArrow(DT_TIMESTAMP), default_memory_pool());
        ARROW_RETURN_NOT_OK(builder.Resize(length));
        ARROW_RETURN_NOT_OK(builder.AppendValues(vecSrc, isValid));
        ARROW_RETURN_NOT_OK(builder.Finish(&array));
        break;
    }
    case DT_NANOTIME:
    {
        std::vector<int64_t> vecSrc(length);
        long long * buf = (long long*)vecSrc.data();
        dolphinCol->getLong(0, length, (buf));
        std::vector<bool> isValid(length, true);
        if(dolphinCol->hasNull()) {
            for(int i = 0; i < length; i++) {
                if((unsigned long long)vecSrc[i] == -9223372036854775808ULL) {
                    isValid[i] = false;
                }
            }
        }
        auto builder = arrow::NumericBuilder<arrow::Time64Type>(convertDolphinDBToArrow(DT_NANOTIME), default_memory_pool());
        ARROW_RETURN_NOT_OK(builder.Resize(length));
        ARROW_RETURN_NOT_OK(builder.AppendValues(vecSrc, isValid));
        ARROW_RETURN_NOT_OK(builder.Finish(&array));
        break;
    }
    case DT_NANOTIMESTAMP:
    {
        std::vector<int64_t> vecSrc(length);
        long long * buf = (long long*)vecSrc.data();
        dolphinCol->getLong(0, length, (buf));
        std::vector<bool> isValid(length, true);
        if(dolphinCol->hasNull()) {
            for(int i = 0; i < length; i++) {
                if((unsigned long long)vecSrc[i] == -9223372036854775808ULL) {
                    isValid[i] = false;
                }
            }
        }
        auto builder = arrow::NumericBuilder<arrow::TimestampType>(convertDolphinDBToArrow(DT_NANOTIMESTAMP), default_memory_pool());
        ARROW_RETURN_NOT_OK(builder.Resize(length));
        ARROW_RETURN_NOT_OK(builder.AppendValues(vecSrc, isValid));
        ARROW_RETURN_NOT_OK(builder.Finish(&array));
        break;
    }
    case DT_FLOAT:
    {
        std::vector<float> vecSrc(length);
        float * buf = (float*)vecSrc.data();
        dolphinCol->getFloat(0, length, (buf));


        std::vector<bool> isValid(length, true);
        if(dolphinCol->hasNull()) {
            for(int i = 0; i < length; i++) {
                if(*reinterpret_cast<unsigned int*>(&vecSrc[i]) == 0xff7fffff) {
                    isValid[i] = false;
                    // *reinterpret_cast<unsigned int*>(&vecSrc[i]) = 0x7fc00000;
                }
            }
        }

        arrow::NumericBuilder<arrow::FloatType> builder;
        ARROW_RETURN_NOT_OK(builder.Resize(length));
        ARROW_RETURN_NOT_OK(builder.AppendValues(vecSrc, isValid));
        ARROW_RETURN_NOT_OK(builder.Finish(&array));
        break;
    }
    case DT_DOUBLE:
    {
        std::vector<double> vecSrc(length);
        double * buf = vecSrc.data();
        dolphinCol->getDouble(0, length, buf);

        std::vector<bool> isValid(length, true);
        if(dolphinCol->hasNull()) {
            for(int i = 0; i < length; i++) {
                if(*reinterpret_cast<unsigned long long*>(&vecSrc[i]) == 0xffefffffffffffff) {
                    isValid[i] = false;
                    // *reinterpret_cast<unsigned long long*>(&vecSrc[i]) = 0x7ff8000000000000;
                }
            }
        }

        arrow::NumericBuilder<arrow::DoubleType> builder;
        ARROW_RETURN_NOT_OK(builder.Resize(length));
        ARROW_RETURN_NOT_OK(builder.AppendValues(vecSrc, isValid));
        ARROW_RETURN_NOT_OK(builder.Finish(&array));
        break;
    }
    case DT_STRING:
    {
        // arrow::StringBuilder builder;
        // ARROW_RETURN_NOT_OK(builder.Resize(length));
        // DolphinString* buf[65535];
        // INDEX start = 0;
        // while(start < length){
        //     int count = std::min(length - start, 65535);
        //     std::vector<string> vec(count);
        //     DolphinString** pbuf = dolphinCol->getStringConst(start, count, buf);
        //     for (int rowIndex = 0; rowIndex < count; rowIndex++) {
        //         vec[rowIndex] = pbuf[rowIndex]->getString();
        //     }
        //     start += count;
        //     ARROW_RETURN_NOT_OK(builder.AppendValues(vec));
        // }
        // ARROW_RETURN_NOT_OK(builder.Finish(&array));

        vector<char*> vecSrc(length);
        ((ConstantSP)dolphinCol)->getString(0, length, vecSrc.data());
        arrow::StringBuilder builder;
        ARROW_RETURN_NOT_OK(builder.Resize(length));
        ARROW_RETURN_NOT_OK(builder.AppendValues((const char**)vecSrc.data(), length));
        ARROW_RETURN_NOT_OK(builder.Finish(&array));
        break;
    }
    case DT_SYMBOL:
    {
        vector<char*> vecSrc(length);
        ((ConstantSP)dolphinCol)->getString(0, length, vecSrc.data());
        arrow::StringBuilder builder;
        ARROW_RETURN_NOT_OK(builder.Resize(length));
        ARROW_RETURN_NOT_OK(builder.AppendValues((const char**)vecSrc.data(), length));
        ARROW_RETURN_NOT_OK(builder.Finish(&array));
        break;
    }
    default:
        break;
    }
    dest = array;
    return Status::OK();
}


class TransColRunnable: public Runnable {
    public:
        TransColRunnable(std::vector<std::shared_ptr<arrow::Array>>& columns, VectorSP dolphinCol, DATA_TYPE type, int index, Status& status): columns_(columns), dolphinCol_(dolphinCol), type_(type), index_(index), status_(status){
        }
    void run() {
        try {
            status_ = transCol(columns_[index_], dolphinCol_, type_);
        } catch (std::exception &e) {
            string errMsg = e.what();
            status_ = Status(StatusCode::SerializationError, errMsg);
            PLUGIN_LOG_ERR(errMsg);
        } catch (...) {
            string errMsg = "Error occurs when trans cols data.";
            status_ = Status(StatusCode::UnknownError, errMsg);
            PLUGIN_LOG_ERR(errMsg);
        }
    }
    private:
    std::vector<std::shared_ptr<arrow::Array>>& columns_;
    VectorSP dolphinCol_;
    DATA_TYPE type_;
    int index_;
    Status& status_;
};

using TransColRunnableSP = SmartPointer<TransColRunnable>;

// vector<std::shared_ptr<io::FileOutputStream>> fileVec;
// Mutex fileVecMutex;

arrow::Status saveToFeather(const ConstantSP& table, const ConstantSP& filename, ipc::feather::WriteProperties& properties) {
    // make schema
    int colNum = table->columns();
    std::shared_ptr<Schema> schema;
    std::vector<std::shared_ptr<Field>> fields(colNum);
    for(int i = 0; i < colNum; i++) {
        std::shared_ptr<arrow::DataType> type = convertDolphinDBToArrow(((TableSP)table)->getColumnType(i));
        if(type->id() == Type::type::NA) {
            return Status(StatusCode::TypeError, Util::getDataTypeString(((TableSP)table)->getColumnType(i)) + ". Unsupported type in column " + ((TableSP)table)->getColumnName(i) + ".");
        }
        fields[i] = arrow::field(((TableSP)table)->getColumnName(i), type);
    }
    schema = arrow::schema(fields);

    // make columns
    std::vector<std::shared_ptr<arrow::Array>> columns(colNum);
    vector<ThreadSP> colThreads(colNum);
    vector<arrow::Status> statList(colNum);
    for(int i = 0; i < colNum; i++) {
        VectorSP dolphinCol = table->getColumn(i);
        DATA_TYPE type = ((TableSP)table)->getColumnType(i);
        TransColRunnableSP runnable = new TransColRunnable(columns, dolphinCol, type, i, statList[i]);
        ThreadSP thread = new Thread(runnable);
        colThreads[i] = thread;
        if(!thread->isStarted()) {
            thread->start();
        }
    }

    for(auto thread: colThreads) {
        thread->join();
    }
    for(int i = 0; i < colNum; i++) {
        auto stat = statList[i];
        if(!stat.ok()) {
            return arrow::Status(stat.code(), stat.message() + " in Column " + ((TableSP)table)->getColumnName(i) + ".");
        }
    }

    std::shared_ptr<arrow::Table> arrowTable = arrow::Table::Make(schema, columns);
    ARROW_ASSIGN_OR_RAISE(auto file, io::FileOutputStream::Open(filename->getString(), false));
    ARROW_RETURN_NOT_OK(ipc::feather::WriteTable(*arrowTable, file.get(), properties));

    // LockGuard<Mutex> guard(&fileVecMutex);
    // fileVec.push_back(file);

    return arrow::Status::OK();
}

ConstantSP saveFeather(Heap *heap, vector<ConstantSP> &args){

    const auto usage = string("Usage: save(table, filePath, [compressMethod], [compressionLevel]). ");
    ConstantSP table = args[0];
    ConstantSP filename = args[1];
    if(!table->isTable()){
        throw IllegalArgumentException(__FUNCTION__, usage + "The parameter table must be a table.");
    }
    if(filename->getType() != DT_STRING){
        throw IllegalArgumentException(__FUNCTION__, usage + "The filePath must be a string.");
    }

    if(args.size()>=3) {
        if(args[2]->getType() != DT_STRING) {
            throw IllegalArgumentException(__FUNCTION__, usage + "The compressMethod must be a string.");
        }
        arrow::Status status;
        ipc::feather::WriteProperties properties;
        string compression = args[2]->getString();
        std::transform(compression.begin(),compression.end(),compression.begin(),::tolower);
        if(compression == "uncompressed") {
            properties.compression = Compression::type::UNCOMPRESSED;
        } else if(compression == "lz4") {
            properties.compression = Compression::type::LZ4_FRAME;
        } else if(compression == "zstd") {
            properties.compression = Compression::type::ZSTD;
        } else {
            throw IllegalArgumentException(__FUNCTION__, usage + "Unsupported compressMethod.");
        }

        if(args.size()==4) {
            if(args[3]->getType() != DT_INT) {
                throw IllegalArgumentException(__FUNCTION__, usage + "The parameter compressionLevel must be an INT.");
            }
            if(compression != "zstd") {
                throw IllegalArgumentException(__FUNCTION__, usage + "Only the compressMethod zstd supports compressionLevel.");
            }
            int level = args[3]->getInt();
            properties.compression_level = level;
        }
        status = saveToFeather(table, filename, properties);
        if (!status.ok()) {
            throw RuntimeException(status.ToString());
        }
    } else {
        auto properties = ipc::feather::WriteProperties::Defaults();
        arrow::Status status = saveToFeather(table, filename, properties);

        if (!status.ok()) {
            throw RuntimeException(status.ToString());
        }
    }

    return new Void();
}

