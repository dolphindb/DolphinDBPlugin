#include "parquet_plugin.h"

ConstantSP extractParquetSchema(const ConstantSP &filename)
{
    if (filename->getType() != DT_STRING)
        throw IllegalArgumentException(__FUNCTION__, "The filename and dataset must be a string.");

    ConstantSP schema = ParquetPluginImp::extractParquetSchema(filename->getString());

    return schema;
}

ConstantSP loadParquet(Heap *heap, vector<ConstantSP> &arguments)
{
    ConstantSP filename = arguments[0];

    int rowGroupStart = 0;
    int rowGroupNum = 0;
    ConstantSP schema = ParquetPluginImp::nullSP;
    ConstantSP column = new Void();
    if (filename->getType() != DT_STRING && filename->getType() != DT_RESOURCE )
        throw IllegalArgumentException(__FUNCTION__, "The filename and dataset must be a string.");
    if (arguments.size() >= 2 &&!arguments[1]->isNull() )
    {
        if (!arguments[1]->isTable())
            throw IllegalArgumentException(__FUNCTION__, "schema must be a table containing column names and types.");
        else if(!arguments[1]->isNull())
            schema = arguments[1];
    }
    if (arguments.size() >= 3&&!arguments[2]->isNull())
    {
        if (!arguments[2]->isVector() || arguments[2]->getCategory() != INTEGRAL)
            throw IllegalArgumentException(__FUNCTION__, "column must be a vector");
        else
            column = arguments[2];
    }
    if (arguments.size() >= 4 && !arguments[3]->isNull())
    {
        if (arguments[3]->isScalar() && arguments[3]->getCategory() == INTEGRAL)
        {
            rowGroupStart = arguments[3]->getInt();
            if (rowGroupStart < 0)
                throw IllegalArgumentException(__FUNCTION__, "rowGroupStart must be positive.");
        }
        else
            throw IllegalArgumentException(__FUNCTION__, "rowGroupStart must be an integer.");
    }
    if (arguments.size() >= 5 && !arguments[4]->isNull())
    {
        if (arguments[4]->isScalar() && arguments[4]->getCategory() == INTEGRAL)
        {
            rowGroupNum = arguments[4]->getInt();
            if (rowGroupNum <= 0)
                throw IllegalArgumentException(__FUNCTION__, "rowGroupNum must be positive.");
        }
        else
            throw IllegalArgumentException(__FUNCTION__, "rowGroupNum must be an integer.");
    }
    return ParquetPluginImp::loadParquet(filename->getString(), schema, column, rowGroupStart, rowGroupNum);
}

ConstantSP loadParquetHdfs(Heap *heap, vector<ConstantSP> &arguments)
{
    if(arguments[0]->getType() != DT_RESOURCE || arguments[0]->getString() != "hdfs readFile address")
        throw IllegalArgumentException(__FUNCTION__,"The first arguments should be resource");
    if(arguments[1]->getType() != DT_RESOURCE || arguments[1]->getString() != "hdfs readFile length")
        throw IllegalArgumentException(__FUNCTION__,"The second arguments should be resource");
    
    void *buffer = (void *)arguments[0]->getLong();
    uint64_t *length = (uint64_t *)arguments[1]->getLong();
    return ParquetPluginImp::loadParquetHdfs(buffer, *length);
}

ConstantSP loadParquetEx(Heap *heap, vector<ConstantSP> &arguments)
{
    ConstantSP db = arguments[0];
    ConstantSP tableName = arguments[1];
    ConstantSP partitionColumnNames = arguments[2];
    ConstantSP filename = arguments[3];

    int rowGroupNum = 0;
    int rowGroupStart = 0;
    ConstantSP schema = ParquetPluginImp::nullSP;
    ConstantSP column = new Void();

    if (!db->isDatabase())
        throw IllegalArgumentException(__FUNCTION__, "dbHandle must be a database handle.");
    if (tableName->getType() != DT_STRING)
        throw IllegalArgumentException(__FUNCTION__, "tableName must be a string.");
    if (!partitionColumnNames->isNull() && partitionColumnNames->getType() != DT_STRING)
        throw IllegalArgumentException(__FUNCTION__, "The partition columns must be in string or string vector.");
    if (filename->getType() != DT_STRING)
        throw IllegalArgumentException(__FUNCTION__, "The filename and dataset for h5read must be a string.");
    if (arguments.size() >= 5 && !arguments[4]->isNull())
    {
        if (!arguments[4]->isTable())
            throw IllegalArgumentException(__FUNCTION__, "schema must be a table containing column names and types.");
        else
            schema = arguments[4];
    }
    if (arguments.size() >= 6&&!arguments[5]->isNull())
    {
        if (!arguments[5]->isVector() || arguments[5]->getCategory() != INTEGRAL)
            throw IllegalArgumentException(__FUNCTION__, "column must be a vector");
        else
            column = arguments[5];
    }
    if (arguments.size() >= 7 && !arguments[6]->isNull())
    {
        if (arguments[6]->isScalar() && arguments[6]->getCategory() == INTEGRAL){
            rowGroupStart = arguments[6]->getInt();
            if (rowGroupStart < 0)
                throw IllegalArgumentException(__FUNCTION__, "rowGroupStart must be a non-negative integer.");
        }
        else
            throw IllegalArgumentException(__FUNCTION__, "rowGroupStart must be an integer.");
    }
    if (arguments.size() >= 8 && !arguments[7]->isNull())
    {
        if (arguments[7]->isScalar() && arguments[7]->getCategory() == INTEGRAL){
            rowGroupNum = arguments[7]->getInt();
            if (rowGroupNum <= 0)
                throw IllegalArgumentException(__FUNCTION__, "rowGroupNum must be a non-negative integer.");
        }
        else
            throw IllegalArgumentException(__FUNCTION__, "rowGroupNum must be an integer.");
    }
   
    ConstantSP transform;
    if (arguments.size() >= 9 && !arguments[8]->isNull()) {
        if (arguments[8]->getType() != DT_FUNCTIONDEF)
            throw IllegalArgumentException(__FUNCTION__, "transform must be a function.");
        transform = arguments[8];
    }
    else
        transform = new Void();
    return ParquetPluginImp::loadParquetEx(heap, db, tableName->getString(), partitionColumnNames,
                                           filename->getString(),
                                           schema, column, rowGroupStart, rowGroupNum, transform);
}

ConstantSP parquetDS(Heap *heap, vector<ConstantSP> &arguments)
{
    ConstantSP filename = arguments[0];

    ConstantSP schema = ParquetPluginImp::nullSP;

    if (filename->getType() != DT_STRING)
        throw IllegalArgumentException(__FUNCTION__, "The filename and dataset for h5read must be a string.");
    if (arguments.size() >= 2)
    {
        if (arguments[1]->isNull())
            schema = ParquetPluginImp::nullSP;
        else if (!arguments[1]->isTable())
            throw IllegalArgumentException(__FUNCTION__, "schema must be a table containing column names and types.");
        else
            schema = arguments[1];
    }

    return ParquetPluginImp::parquetDS(filename, schema);
}

ConstantSP saveParquet(Heap *heap, vector<ConstantSP> &arguments)
{
    ConstantSP tb = arguments[0];
    ConstantSP filename = arguments[1];
    if(!tb->isTable())
        throw IllegalArgumentException(__FUNCTION__, "tb must be a table.");
    if(filename->getType() != DT_STRING)
        throw IllegalArgumentException(__FUNCTION__, "The filename must be a string.");
    return ParquetPluginImp::saveParquet(tb, filename->getString());
}

ConstantSP saveParquetHdfs(Heap *heap, vector<ConstantSP>& arguments)
{
    if(arguments.size()!=1 || !arguments[0]->isTable())
        throw IllegalArgumentException(__FUNCTION__, "argument should be a table");
    return ParquetPluginImp::saveParquetHdfs(arguments[0]);
}

namespace ParquetPluginImp
{

const string STR_MIN="";
ConstantSP nullSP = Util::createNullConstant(DT_VOID);
void ParquetReadOnlyFile::open(const std::string &file_name)
{
    close();
    try
    {
        std::unique_ptr<parquet::ParquetFileReader> parquet_reader = parquet::ParquetFileReader::OpenFile(file_name, false);

        _parquet_reader = std::move(parquet_reader);
    }
    catch (parquet::ParquetStatusException e)
    {
        throw IOException(e.what());
    }
}

void ParquetReadOnlyFile::open(std::shared_ptr<arrow::io::RandomAccessFile> source)
{
    close();
    try
    {
        std::unique_ptr<parquet::ParquetFileReader> parquet_reader = parquet::ParquetFileReader::Open(source);

        _parquet_reader = std::move(parquet_reader);
    }
    catch (parquet::ParquetStatusException e)
    {
        throw IOException(e.what());
    }
}

std::shared_ptr<parquet::RowGroupReader> ParquetReadOnlyFile::rowReader(int i)
{
    if (i >= fileMetadataReader()->num_row_groups())
        return nullptr;
    else
        return _parquet_reader->RowGroup(i);
}

void ParquetReadOnlyFile::close()
{
    if (_parquet_reader != nullptr)
        _parquet_reader->Close();
}

std::string getDafaultColumnType(parquet::Type::type physical_t)
{
    switch (physical_t)
    {
    case parquet::Type::type::BOOLEAN:
        return "BOOL";
    case parquet::Type::type::INT32:
        return "INT";
    case parquet::Type::type::INT64:
        return "LONG";
    case parquet::Type::type::FLOAT:
        return "FLOAT";
    case parquet::Type::type::DOUBLE:
        return "DOUBLE";
    case parquet::Type::type::BYTE_ARRAY:
        return "STRING";
    case parquet::Type::type::FIXED_LEN_BYTE_ARRAY:
        return "STRING";
    case parquet::Type::type::INT96:
        return "NANOTIMESTAMP";
    default:
        throw RuntimeException("unsupported data type");
    }
}

std::string getLayoutColumnType(std::shared_ptr<const parquet::LogicalType> &logical_t, parquet::Type::type physical_t, parquet::SortOrder::type sort_order)
{
    if (logical_t->is_none()||logical_t->is_invalid())
    {
        return getDafaultColumnType(physical_t);
    }
    parquet::LogicalType::Type::type logi_t = logical_t->type();
    switch (logi_t)
    {
    case parquet::LogicalType::Type::INT:{
        const parquet::IntLogicalType* intLogical_t = static_cast<const parquet::IntLogicalType*>(logical_t.get());
        int bitWidth = intLogical_t->bit_width();
        if(bitWidth == 8){
            if(sort_order == parquet::SortOrder::UNSIGNED)
                return "SHORT";
            else
                return "CHAR";
        }
        else if(bitWidth == 16){
            if(sort_order == parquet::SortOrder::UNSIGNED)
                return "INT";
            else
                return "SHORT";
        }
        else if(bitWidth == 32){
            if(sort_order == parquet::SortOrder::UNSIGNED)
                return "LONG";
            else
                return "INT";
        }
        else if(bitWidth == 64){
                return "LONG";
        }
        else{
            switch (physical_t)
            {
            case parquet::Type::type::INT32:
                return "INT";
            case parquet::Type::type::INT64:
                return "LONG";
            default:
                throw RuntimeException("unsupported data type");
            }
        }
    }
    case parquet::LogicalType::Type::STRING:
    {
        if (physical_t != parquet::Type::BYTE_ARRAY)
            throw RuntimeException("unsupported data type");
        return "STRING";
    }

    case parquet::LogicalType::Type::ENUM:
    {
        if (physical_t != parquet::Type::BYTE_ARRAY)
            throw RuntimeException("unsupported data type");
        return "SYMBOL";
    }
    case parquet::LogicalType::Type::DATE:
    {
        if (physical_t != parquet::Type::INT32)
            throw RuntimeException("unsupported data type");
        return "DATE";
    }
    case parquet::LogicalType::Type::TIME:
    {
        parquet::LogicalType::TimeUnit::unit timeUnit = (std::dynamic_pointer_cast<const parquet::TimeLogicalType>(logical_t))->time_unit();
        if (timeUnit==parquet::LogicalType::TimeUnit::unit::UNKNOWN)
            throw RuntimeException("unknown TIME precision");
        else if (timeUnit == parquet::LogicalType::TimeUnit::unit::MILLIS)
            return "TIME";
        else
            return "NANOTIME";
    }
    case parquet::LogicalType::Type::TIMESTAMP:{
        parquet::LogicalType::TimeUnit::unit timeUnit = (std::dynamic_pointer_cast<const parquet::TimestampLogicalType>(logical_t))->time_unit();
        if (physical_t != parquet::Type::INT64)
            throw RuntimeException("unsupported data type");
        if (timeUnit==parquet::LogicalType::TimeUnit::unit::UNKNOWN)
            throw RuntimeException("unknown TIMESTAMP precision");
        else if(timeUnit==parquet::LogicalType::TimeUnit::unit::MILLIS)
            return "TIMESTAMP";
        else
            return "NANOTIMESTAMP";
    }
    case parquet::LogicalType::Type::UUID:
        throw RuntimeException("unsupported data type");
    case parquet::LogicalType::Type::DECIMAL:
        return "DOUBLE";
    default:
        throw RuntimeException("unsupported data type");
    }
    return "";
}

std::string getLayoutColumnType(parquet::ConvertedType::type converted_t, parquet::Type::type physical_t, parquet::SortOrder::type sort_order)
{

    switch (converted_t)
    {
    case parquet::ConvertedType::type::NONE:
    case parquet::ConvertedType::type::UNDEFINED:
        return getDafaultColumnType(physical_t);
    case parquet::ConvertedType::type::INT_8:
        return "CHAR";
    case parquet::ConvertedType::type::INT_16:
    case parquet::ConvertedType::type::UINT_8:
        return "SHORT";
    case parquet::ConvertedType::type::INT_32:
    case parquet::ConvertedType::type::UINT_16:
        return "INT";
    case parquet::ConvertedType::type::TIMESTAMP_MICROS:
        return "NANOTIMESTAMP";
    case parquet::ConvertedType::type::TIMESTAMP_MILLIS:
        return "TIMESTAMP";
    case parquet::ConvertedType::type::DECIMAL:
        return "DOUBLE";
    case parquet::ConvertedType::type::UINT_32:
    case parquet::ConvertedType::type::INT_64:
    case parquet::ConvertedType::type::UINT_64:
        return "LONG";
    case parquet::ConvertedType::type::TIME_MICROS:
        return "NANOTIME";
    case parquet::ConvertedType::type::TIME_MILLIS:
        return "TIME";
    case parquet::ConvertedType::type::DATE:
        return "DATE";
    case parquet::ConvertedType::type::ENUM:
        return "SYMBOL";
    case parquet::ConvertedType::type::UTF8:
        return "STRING";
    default:
        throw RuntimeException("unsupported data type");
    }
    return "";
}

bool getSchemaCol(const parquet::SchemaDescriptor *schema_descr, const ConstantSP &col_idx, vector<ConstantSP> &dolpindbCol)
{
    if (dolpindbCol.size() != 2)
        return false;
    int col_num = col_idx->size();
    if (col_num == 0)
        return false;
    dolpindbCol[0] = Util::createVector(DT_STRING, col_num, col_num);
    dolpindbCol[1] = Util::createVector(DT_STRING, col_num, col_num);
    for (int i = 0; i < col_num; i++)
    {
        const parquet::ColumnDescriptor *col = schema_descr->Column(col_idx->getInt(i));
        if (col->max_repetition_level() != 0)
            throw RuntimeException("not support parquet repeated field yet.");
        string path = col->path()->ToDotString();
        string name = path;
        auto lt = col->converted_type();
        auto la = col->logical_type();
        auto lp = col->physical_type();
        string type;
        type = (la && la->is_valid()) ? getLayoutColumnType(la, lp, col->sort_order()) : getLayoutColumnType(lt, lp, col->sort_order());
        ConstantSP col_name = new String(name);
        ConstantSP col_type = new String(type);

        dolpindbCol[0]->set(i, col_name);
        dolpindbCol[1]->set(i, col_type);
    }
    return true;
}

TableSP extractParquetSchema(const string &filename)
{
    ParquetReadOnlyFile file(filename);
    std::shared_ptr<parquet::FileMetaData> file_metadata = file.fileMetadataReader();
    const parquet::SchemaDescriptor *s = file_metadata->schema();
    int col_num = s->num_columns();
    vector<ConstantSP> cols(2);
    ConstantSP col_idx = Util::createIndexVector(0,col_num);
    if (!getSchemaCol(s, col_idx, cols))
        throw RuntimeException("get schema failed");
    vector<string> colNames(2);
    colNames[0] = "name";
    colNames[1] = "type";
    return Util::createTable(colNames, cols);
}

void createNewVectorSP(vector<VectorSP> &dolpindb_v, const TableSP &tb)
{
    int col_num = dolpindb_v.size();
    for (int i = 0; i < col_num; i++)
    {
        if(tb->getColumnType(i) == DT_SYMBOL)
            dolpindb_v[i] = Util::createVector(DT_STRING, 0);
        else
            dolpindb_v[i] = Util::createVector(tb->getColumnType(i), 0);
    }
}

TableSP appendColumnVecToTable(TableSP tb, vector<VectorSP> &colVec, INDEX& insertedRows)
{
    if (tb->isNull())
        return tb;

    string errMsg;
    vector<ConstantSP> cols(colVec.size());
    int col_num = colVec.size();
    for (int i = 0; i < col_num; i++)
    {
        cols[i] = {colVec[i]};
    }
    if (!tb->append(cols, insertedRows, errMsg))
        throw TableRuntimeException(errMsg);
    return tb;
}

template<typename T>
bool convertToDTDate(vector<T> &intValue, parquetTime times_t, int bufSize)
{
    long long ratio;
    switch (times_t)
    {
    case parquetTime::Date:
        return true;
    case parquetTime::TimestampMicros:
        ratio = Util::getTemporalConversionRatio(DT_TIMESTAMP,DT_DATE);
        ratio = (- ratio) * 1000;
        break;
    case parquetTime::TimestampMillis:
        ratio = Util::getTemporalConversionRatio(DT_TIMESTAMP,DT_DATE);
        break;
    case parquetTime::TimestampNanos:        
        ratio = Util::getTemporalConversionRatio(DT_NANOTIMESTAMP,DT_DATE);
        break;
    default:
        return false;
    }
    for (int i = 0; i < bufSize; i++)
    {
        intValue[i] /= ratio;
    }
    return true;
}

template<typename T>
bool convertToDTMinute(vector<T> &intValue, parquetTime times_t, int bufSize)
{
    long long ratio;
    switch (times_t)
    {
    case parquetTime::Date:
        return false;
    case parquetTime::TimestampMicros:
        ratio = Util::getTemporalConversionRatio(DT_TIME, DT_MINUTE);
        ratio = (-ratio) * 1000;
        for (int i = 0; i < bufSize; i++)
        {
            intValue[i] = intValue[i] % 86400000000 / ratio;
        }
        return true;
    case parquetTime::TimestampMillis:
        ratio = -Util::getTemporalConversionRatio(DT_TIME, DT_MINUTE);
        for (int i = 0; i < bufSize; i++)
        {
            intValue[i] = intValue[i] % 86400000 / ratio;
        }
        return true;
    case parquetTime::TimestampNanos:
        ratio = -Util::getTemporalConversionRatio(DT_NANOTIME, DT_MINUTE);
        for (int i = 0; i < bufSize; i++)
        {
            intValue[i] = intValue[i] % 86400000000000 / ratio;
        }
        return true;
    case parquetTime::TimeMicros:
        ratio = -Util::getTemporalConversionRatio(DT_TIME, DT_MINUTE);
        ratio *= 1000; 
        for (int i = 0; i < bufSize; i++)
        {
            intValue[i] = intValue[i] / ratio;
        }
        return true;
    case parquetTime::TimeMillis:
        ratio = -Util::getTemporalConversionRatio(DT_TIME, DT_MINUTE);
        for (int i = 0; i < bufSize; i++)
        {
            intValue[i] = intValue[i] / ratio;
        }
        return true;
    case parquetTime::TimeNanos:
        ratio = -Util::getTemporalConversionRatio(DT_NANOTIME, DT_MINUTE);
        for (int i = 0; i < bufSize; i++)
        {
            intValue[i] = intValue[i] / ratio;
        }
        return true;
    default:
        return false;
    }
}

template<typename T>
bool convertToDTTime(vector<T> &intValue, parquetTime times_t, int bufSize)
{
    switch (times_t)
    {
    case parquetTime::Date:
        return false;
    case parquetTime::TimestampMicros:
        for (int i = 0; i < bufSize; i++)
        {
            intValue[i] = intValue[i] % 86400000000 / 1000;
        }
        return true;
    case parquetTime::TimestampMillis:
        for (int i = 0; i < bufSize; i++)
        {
            intValue[i] = intValue[i] % 86400000;
        }
        return true;
    case parquetTime::TimestampNanos:
        for (int i = 0; i < bufSize; i++)
        {
            intValue[i] = intValue[i] % 86400000000000 / 1000000;
        }
        return true;
    case parquetTime::TimeMicros:
        for (int i = 0; i < bufSize; i++)
            intValue[i] = intValue[i] / 1000;
        return true;
    case parquetTime::TimeMillis:
        return true;
    case parquetTime::TimeNanos:
        for (int i = 0; i < bufSize; i++)
            intValue[i] = intValue[i] / 1000000;
        return true;
    default:
        return false;
    }
}

template<typename T>
bool convertToDTSecond(vector<T> &intValue, parquetTime times_t, int bufSize)
{
    switch (times_t)
    {
    case parquetTime::Date:
        return false;
    case parquetTime::TimestampMicros:
        for (int i = 0; i < bufSize; i++)
        {
            intValue[i] = intValue[i] % 86400000000 / 1000000;
        }
        return true;
    case parquetTime::TimestampMillis:
        for (int i = 0; i < bufSize; i++)
        {
            intValue[i] = intValue[i] % 86400000 / 1000;
        }
        return true;
    case parquetTime::TimestampNanos:
        for (int i = 0; i < bufSize; i++)
        {
            intValue[i] = intValue[i] % 86400000000000 / 1000000000;
        }
        return true;
    case parquetTime::TimeMicros:
        for (int i = 0; i < bufSize; i++)
            intValue[i] = intValue[i] / 1000000;
        return true;
    case parquetTime::TimeMillis:
        for (int i = 0; i < bufSize; i++)
            intValue[i] = intValue[i] / 1000;
        return true;
    case parquetTime::TimeNanos:
        for (int i = 0; i < bufSize; i++)
            intValue[i] = intValue[i] / 1000000000;
        return true;
    default:
        return false;
    }
}

template<typename T>
bool convertToDTMonth(vector<T> &intValue, parquetTime times_t, int bufSize)
{
    int year, month, day;
    long long ratio;
    switch (times_t)
    {
    case parquetTime::Date:
        for (int i = 0; i < bufSize; i++)
        {
			Util::parseDate(intValue[i], year, month, day);
			intValue[i] = year*12+month-1;
        }
        return true;
    case parquetTime::TimestampMicros:
        ratio = Util::getTemporalConversionRatio(DT_TIMESTAMP,DT_DATE);
        ratio = (- ratio) * 1000;
        for (int i = 0; i < bufSize; i++)
        {
			Util::parseDate(intValue[i] / ratio, year, month, day);
			intValue[i] = year*12+month-1;
        }
        return true;
    case parquetTime::TimestampMillis:
        ratio = -Util::getTemporalConversionRatio(DT_TIMESTAMP,DT_DATE);
        for (int i = 0; i < bufSize; i++)
        {
			Util::parseDate(intValue[i] / ratio, year, month, day);
			intValue[i] = year*12+month-1;
        }
        return true;
    case parquetTime::TimestampNanos:
        ratio = -Util::getTemporalConversionRatio(DT_NANOTIMESTAMP,DT_DATE);
        for (int i = 0; i < bufSize; i++)
        {
			Util::parseDate(intValue[i] / ratio, year, month, day);
			intValue[i] = year*12+month-1;
        }
        return true;
    default:
        return false;
    }
}

template<typename T>
bool convertToDTDatetime(vector<T> &intValue, parquetTime times_t, int bufSize)
{
    switch (times_t)
    {
    case parquetTime::Date:
        for (int i = 0; i < bufSize; i++)
            intValue[i] = intValue[i] * 24 * 60 * 60;
        return true;
    case parquetTime::TimestampMicros:
        for (int i = 0; i < bufSize; i++)
        {
            intValue[i] = intValue[i] / 1000000;
        }
        return true;
    case parquetTime::TimestampMillis:
        for (int i = 0; i < bufSize; i++)
        {
            intValue[i] = intValue[i] / 1000;
        }
        return true;
    case parquetTime::TimestampNanos:
        for (int i = 0; i < bufSize; i++)
        {
            intValue[i] = intValue[i] / 1000000000;
        }
        return true;
    default:
        return false;
    }
}

bool convertToDTNanotime(vector<int> &intValue, parquetTime times_t, vector<long long> &longValue, int bufSize)
{
    switch (times_t)
    {
    case parquetTime::Date:
        return false;
    case parquetTime::TimestampMicros:
        for (int i = 0; i < bufSize; i++)
        {
            longValue[i] = longValue[i] % 86400000000 * 1000;
        }
        return true;
    case parquetTime::TimestampMillis:
        for (int i = 0; i < bufSize; i++)
        {
            longValue[i] = longValue[i] % 86400000 * 1000000;
        }
        return true;
    case parquetTime::TimestampNanos:
        for (int i = 0; i < bufSize; i++)
        {
            longValue[i] = longValue[i] % 86400000000000;
        }
    case parquetTime::TimeMicros:
        for (int i = 0; i < bufSize; i++)
            longValue[i] = longValue[i] * 1000;
        return true;
    case parquetTime::TimeMillis:
        for (int i = 0; i < bufSize; i++)
            longValue[i] = intValue[i] * 1000000;
        return true;
    case parquetTime::TimeNanos:
        return true;
    default:
        return false;
    }
}

bool convertToDTTimestamp(vector<int> &intValue, parquetTime times_t, vector<long long> &longValue, int bufSize)
{
    switch (times_t)
    {
    case parquetTime::Date:
        for (int i = 0; i < bufSize; i++)
            longValue[i] = intValue[i] * 24 * 60 * 60 * 1000;
        return true;
    case parquetTime::TimestampMicros:
        for (int i = 0; i < bufSize; i++)
        {
            longValue[i] = longValue[i] / 1000;
        }
        return true;
    case parquetTime::TimestampMillis:
        return true;
    case parquetTime::TimestampNanos:
        for (int i = 0; i < bufSize; i++)
        {
            longValue[i] = longValue[i] / 1000000;
        }
        return true;
    default:
        return false;
    }
}

bool convertToDTNanotimestamp(vector<int> &intValue, parquetTime times_t, vector<long long> &longValue, int bufSize)
{
    switch (times_t)
    {
    case parquetTime::Date:
        for (int i = 0; i < bufSize; i++)
            longValue[i] = intValue[i] * 24 * 60 * 60 * 1000 * 1000000;
        return true;
    case parquetTime::TimestampMicros:
        for (int i = 0; i < bufSize; i++)
        {
            longValue[i] = longValue[i] * 1000;
        }
        return true;
    case parquetTime::TimestampMillis:
        for (int i = 0; i < bufSize; i++)
        {
            longValue[i] = longValue[i] * 1000000;
        }
        return true;
    case parquetTime::TimestampNanos:
        return true;
    default:
        return false;
    }
}

parquetTime convertParquetTimes(const std::shared_ptr<const parquet::LogicalType> &logical_t, parquet::ConvertedType::type converted_t)
{
    if (logical_t)
    {
        switch (logical_t->type())
        {
        case parquet::LogicalType::Type::DATE:
            return parquetTime::Date;
        case parquet::LogicalType::Type::TIME:
        {
            parquet::LogicalType::TimeUnit::unit timeUnit = (std::dynamic_pointer_cast<const parquet::TimeLogicalType>(logical_t))->time_unit();
            if (timeUnit==parquet::LogicalType::TimeUnit::unit::UNKNOWN)
                throw RuntimeException("unknown TIMESTAMP precision");
            else if (timeUnit == parquet::LogicalType::TimeUnit::unit::MICROS)
                return parquetTime::TimeMicros;
            else if (timeUnit == parquet::LogicalType::TimeUnit::unit::MILLIS)
                return parquetTime::TimeMillis;
            else
                return parquetTime::TimeNanos;
        }
        case parquet::LogicalType::Type::TIMESTAMP:
        {
            parquet::LogicalType::TimeUnit::unit timeUnit = (std::dynamic_pointer_cast<const parquet::TimestampLogicalType>(logical_t))->time_unit();
            if (timeUnit==parquet::LogicalType::TimeUnit::unit::UNKNOWN)
                throw RuntimeException("unknown TIMESTAMP precision");
            else if (timeUnit==parquet::LogicalType::TimeUnit::unit::MICROS)
                return parquetTime::TimestampMicros;
            else if (timeUnit==parquet::LogicalType::TimeUnit::unit::MILLIS)
                return parquetTime::TimestampMillis;
            else
                return parquetTime::TimestampNanos;
        }
        default:
            return parquetTime::None;
        }
    }
    else if (converted_t)
    {
        switch (converted_t)
        {
        case parquet::ConvertedType::DATE:
            return parquetTime::Date;
        case parquet::ConvertedType::TIME_MILLIS:
            return parquetTime::TimeMillis;
        case parquet::ConvertedType::TIME_MICROS:
            return parquetTime::TimeMicros;
        case parquet::ConvertedType::TIMESTAMP_MILLIS:
            return parquetTime::TimestampMillis;
        case parquet::ConvertedType::TIMESTAMP_MICROS:
            return parquetTime::TimestampMicros;
        default:
            return parquetTime::None;
        }
    }
    return parquetTime::None;
}

int convertParquetToDolphindbChar(int col_idx, std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, char* buffer, size_t batchSize, bool& containNull)
{
    int64_t values_read = 0;
    vector<short> def_level(batchSize);
    vector<short> rep_level(batchSize);
    int64_t rows_read = 0;
    switch (col_descr->physical_type())
    {
    case parquet::Type::BOOLEAN:
    {
        parquet::BoolReader *bool_reader = static_cast<parquet::BoolReader *>(column_reader.get());
        vector<char> bool_value(batchSize);
        while(bool_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += bool_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (bool*)bool_value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        if (rows_read == values_read && rows_read != 0)
        {
            memcpy(buffer, bool_value.data(), rows_read * sizeof(char));
        }
        else if (rows_read != 0)
        {
            int index = 0;
            for (int64_t le = 0; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    buffer[le] = bool_value[index++];
                }
                else
                {
                    buffer[le] = CHAR_MIN;
                }
            }
        }
        return rows_read;
    }
    case parquet::Type::INT32:
    {
        parquet::Int32Reader *int32_reader = static_cast<parquet::Int32Reader *>(column_reader.get());
        // Read all the rows in the column
        vector<int32_t> value(batchSize);
        while(int32_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int32_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (int32_t*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }             
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = value[index++];
            }
            else
            {
                buffer[le] = CHAR_MIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::INT64:
    {
        parquet::Int64Reader *int64_reader = static_cast<parquet::Int64Reader *>(column_reader.get());
        vector<int64_t> value(batchSize);
        while(int64_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int64_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (int64_t*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }      
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = value[index++];
            }
            else
            {
                buffer[le] = CHAR_MIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::INT96:
    {
        parquet::Int96Reader *int96_reader = static_cast<parquet::Int96Reader *>(column_reader.get());
        vector<parquet::Int96> value(batchSize);
        while(int96_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int96_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (parquet::Int96*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }  
        containNull = rows_read != values_read;            
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = value[index++].value[2];
            }
            else
            {
                buffer[le] = CHAR_MIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::DOUBLE:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:DOUBLE" + "->" + Util::getDataTypeString(DT_CHAR));
    case parquet::Type::FLOAT:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:FLOAT" + "->" + Util::getDataTypeString(DT_CHAR));
    case parquet::Type::FIXED_LEN_BYTE_ARRAY:
    {
        int FIXED_LENGTH = col_descr->type_length();
        if (FIXED_LENGTH > 1 || FIXED_LENGTH <= 0)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:FIXED_LEN_BYTE_ARRAY(fixed length=" + std::to_string(FIXED_LENGTH) + ") ->" + Util::getDataTypeString(DT_CHAR));
        parquet::FixedLenByteArrayReader *flba_reader = static_cast<parquet::FixedLenByteArrayReader *>(column_reader.get());
        vector<parquet::FixedLenByteArray> value(batchSize);
        while(flba_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            int64_t le = rows_read;
            rows_read += flba_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (parquet::FixedLenByteArray*)value.data(), &valuesRead);
            int index = 0;
            for (; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    buffer[le] = static_cast<char>(*value[index++].ptr);
                }
                else
                {
                    containNull = true;
                    buffer[le] = CHAR_MIN;
                }
            }
        }
        return rows_read;
    }
    case parquet::Type::BYTE_ARRAY:
    {
        parquet::ByteArrayReader *ba_reader = static_cast<parquet::ByteArrayReader *>(column_reader.get());
        vector<parquet::ByteArray> value(batchSize);
        while(ba_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            int64_t le = rows_read;
            rows_read += ba_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (parquet::ByteArray*)value.data(), &valuesRead);
            values_read += valuesRead;
            int index = 0;
            for (; le < rows_read; le++)
            {
                if (value[index].len > 1)
                    throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:ByteArray(length>1)" + "->" + Util::getDataTypeString(DT_CHAR));
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    buffer[le] = static_cast<char>(*value[index++].ptr);
                }
                else
                {
                    containNull = true;
                    buffer[le] = CHAR_MIN;
                }
            }
        }              
        return rows_read;
    }
    default:
        throw RuntimeException("unsupported data type");
    }
    return 0;
}

int convertParquetToDolphindbBool(int col_idx, std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, char* buffer, size_t batchSize, bool& containNull)
{
    int64_t values_read = 0;
    vector<short> def_level(batchSize);
    vector<short> rep_level(batchSize);
    int64_t rows_read = 0;
    switch (col_descr->physical_type())
    {
    case parquet::Type::BOOLEAN:
    {
        parquet::BoolReader *bool_reader = static_cast<parquet::BoolReader *>(column_reader.get());
        vector<char> bool_value(batchSize);
        while(bool_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += bool_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (bool*)bool_value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        if (rows_read == values_read && rows_read != 0)
        {
            memcpy(buffer, bool_value.data(), rows_read * sizeof(char));
        }
        else if (rows_read != 0)
        {
            int index = 0;
            for (int64_t le = 0; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    buffer[le] = bool_value[index++];
                }
                else
                {
                    buffer[le] = CHAR_MIN;
                }
            }
        }
        return rows_read;
    }
    case parquet::Type::INT32:
    {
        parquet::Int32Reader *int32_reader = static_cast<parquet::Int32Reader *>(column_reader.get());
        // Read all the rows in the column
        vector<int32_t> value(batchSize);
        while(int32_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int32_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (int32_t*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }             
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = (value[index++] != 0);
            }
            else
            {
                buffer[le] = CHAR_MIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::INT64:
    {
        parquet::Int64Reader *int64_reader = static_cast<parquet::Int64Reader *>(column_reader.get());
        vector<int64_t> value(batchSize);
        while(int64_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int64_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (int64_t*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }      
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = (value[index++] != 0);
            }
            else
            {
                buffer[le] = CHAR_MIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::INT96:
    {
        parquet::Int96Reader *int96_reader = static_cast<parquet::Int96Reader *>(column_reader.get());
        vector<parquet::Int96> value(batchSize);
        while(int96_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int96_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (parquet::Int96*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }  
        containNull = rows_read != values_read;            
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = (value[index].value[0] != 0 && value[index].value[1] && value[index].value[2]);
                index ++;
            }
            else
            {
                buffer[le] = CHAR_MIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::DOUBLE:
    {
        parquet::DoubleReader *double_reader = static_cast<parquet::DoubleReader *>(column_reader.get());
        vector<double> value(batchSize);
        while(double_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += double_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (double*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }       
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = (value[index++] != 0);
            }
            else
            {
                buffer[le] = CHAR_MIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::FLOAT:
    {
        parquet::FloatReader *float_reader = static_cast<parquet::FloatReader *>(column_reader.get());
        vector<float> value(batchSize);
        while(float_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += float_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (float*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = (value[index++] != 0);
            }
            else
            {
                buffer[le] = CHAR_MIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::FIXED_LEN_BYTE_ARRAY:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:FIXED_LEN_BYTE_ARRAY" + "->" + Util::getDataTypeString(DT_BOOL));
    case parquet::Type::BYTE_ARRAY:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:BYTE_ARRAY" + "->" + Util::getDataTypeString(DT_BOOL));

    default:
        throw RuntimeException("unsupported data type");
    }
    return 0;
}

int convertParquetToDolphindbInt(int col_idx, std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, int* buffer, DATA_TYPE times_t, size_t batchSize, bool& containNull)
{
    int64_t values_read = 0;
    vector<short> def_level(batchSize);
    vector<short> rep_level(batchSize);
    int64_t rows_read = 0;
    switch (col_descr->physical_type())
    {
    case parquet::Type::BOOLEAN:
    {
        if (times_t != DT_INT)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:BOOLEAN" + "->" + Util::getDataTypeString(times_t));
        parquet::BoolReader *bool_reader = static_cast<parquet::BoolReader *>(column_reader.get());
        vector<char> value(batchSize);
        while(bool_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += bool_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (bool*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = value[index++];
            }
            else
            {
                buffer[le] = INT_MIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::INT32:
    {
        parquet::Int32Reader *int32_reader = static_cast<parquet::Int32Reader *>(column_reader.get());
        // Read all the rows in the column
        vector<int32_t> value(batchSize);
        while(int32_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int32_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (int32_t*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        if(col_descr->logical_type()->is_decimal() || col_descr->converted_type()==parquet::ConvertedType::DECIMAL)
        {
            int scale=std::pow(10, col_descr->type_scale());
            for(int i = 0; i < values_read; i++)
                value[i]=value[i]/scale;
        }
        else{
            parquetTime parquetT = convertParquetTimes(col_descr->logical_type(), col_descr->converted_type());
            if (parquetT != parquetTime::None){
                switch (times_t)
                {
                case DT_DATE:
                    convertToDTDate(value, parquetT, values_read);
                    break;
                case DT_MONTH:
                    convertToDTMonth(value, parquetT, values_read);
                    break;
                case DT_TIME:
                    convertToDTTime(value, parquetT, values_read);
                    break;
                case DT_SECOND:
                    convertToDTSecond(value, parquetT, values_read);
                    break;
                case DT_MINUTE:
                    convertToDTMinute(value, parquetT, values_read);
                    break;
                case DT_DATETIME:
                    convertToDTDatetime(value, parquetT, values_read);
                    break;
                default:
                    break;
                }
            }
        }
        int index = 0;
        if (rows_read == values_read && rows_read != 0)
        {
            memcpy(buffer, value.data(), sizeof(int) * rows_read);
        }
        else{
            for (int64_t le = 0; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    buffer[le] = value[index++];
                }
                else
                {
                    buffer[le] = INT_MIN;
                }
            }
        }
        return rows_read;
    }
    case parquet::Type::INT64:
    {
        vector<int64_t> value(batchSize);
        parquet::Int64Reader *int64_reader = static_cast<parquet::Int64Reader *>(column_reader.get());
        while(int64_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int64_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (int64_t*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        if(col_descr->logical_type()->is_decimal() || col_descr->converted_type()==parquet::ConvertedType::DECIMAL)
        {
            long long scale=std::pow(10,col_descr->type_scale());
            for(int i = 0; i < values_read; i++){
                value[i]=value[i]/scale;
            }
        }else{
            parquetTime parquetT = convertParquetTimes(col_descr->logical_type(), col_descr->converted_type());
            if (parquetT != parquetTime::None && times_t != DT_INT)
            {
                switch (times_t)
                {
                case DT_DATE:
                    convertToDTDate(value, parquetT, values_read);
                    break;
                case DT_MONTH:
                    convertToDTMonth(value, parquetT, values_read);
                    break;
                case DT_TIME:
                    convertToDTTime(value, parquetT, values_read);
                    break;
                case DT_SECOND:
                    convertToDTSecond(value, parquetT, values_read);
                    break;
                case DT_MINUTE:
                    convertToDTMinute(value, parquetT, values_read);
                    break;
                case DT_DATETIME:
                    convertToDTDatetime(value, parquetT, values_read);
                    break;
                default:
                    break;
                }
            }
        }
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = value[index++];
            }
            else
            {
                buffer[le] = INT_MIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::INT96:
    {
        vector<long long> longValue;
        parquet::Int96Reader *int96_reader = static_cast<parquet::Int96Reader *>(column_reader.get());
        vector<parquet::Int96> value(batchSize);
        while(int96_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int96_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (parquet::Int96*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        if(times_t == DT_INT){
            int index = 0;
            for (int64_t le = 0; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    buffer[le] = value[index++].value[2];
                }
                else
                {
                    buffer[le] = INT_MIN;
                }
            }
        }
        else{
            longValue.resize(values_read);
            for(int i = 0; i <values_read; i++ ){
                longValue[i] = parquet::Int96GetNanoSeconds(value[i]);
            }
            switch (times_t)
            {
            case DT_DATE:
                convertToDTDate(longValue, parquetTime::TimestampNanos, values_read);
                break;
            case DT_MONTH:
                convertToDTMonth(longValue, parquetTime::TimestampNanos, values_read);
                break;
            case DT_TIME:
                convertToDTTime(longValue, parquetTime::TimestampNanos, values_read);
                break;
            case DT_SECOND:
                convertToDTSecond(longValue, parquetTime::TimestampNanos, values_read);
                break;
            case DT_MINUTE:
                convertToDTMinute(longValue, parquetTime::TimestampNanos, values_read);
                break;
            case DT_DATETIME:
                convertToDTDatetime(longValue, parquetTime::TimestampNanos, values_read);
                break;
            default:
                break;
            }
            int index = 0;
            for (int64_t le = 0; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    buffer[le] = longValue[index++];
                }
                else
                {
                    buffer[le] = INT_MIN;
                }
            }
        }
        return rows_read;
    }
    case parquet::Type::DOUBLE:
    {
        if (times_t != DT_INT)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:DOUBLE" + "->" + Util::getDataTypeString(times_t));
        parquet::DoubleReader *double_reader = static_cast<parquet::DoubleReader *>(column_reader.get());
        vector<double> value(batchSize);
        while(double_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += double_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (double*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }       
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = value[index++];
            }
            else
            {
                buffer[le] = INT_MIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::FLOAT:
    {
        if (times_t != DT_INT)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:FLOAT" + "->" + Util::getDataTypeString(times_t));
        parquet::FloatReader *float_reader = static_cast<parquet::FloatReader *>(column_reader.get());
        vector<float> value(batchSize);
        while(float_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += float_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (float*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = value[index++];
            }
            else
            {
                buffer[le] = INT_MIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::FIXED_LEN_BYTE_ARRAY:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:FIXED_LEN_BYTE_ARRAY" + "->" + Util::getDataTypeString(DT_INT));
    case parquet::Type::BYTE_ARRAY:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:BYTE_ARRAY" + "->" + Util::getDataTypeString(DT_INT));

    default:
        throw RuntimeException("unsupported data type");
    }
    return 0;
}

int convertParquetToDolphindbShort(int col_idx, std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, short* buffer, size_t batchSize, bool& containNull)
{
    int64_t values_read = 0;
    vector<short> def_level(batchSize);
    vector<short> rep_level(batchSize);
    int64_t rows_read = 0;
    switch (col_descr->physical_type())
    {
    case parquet::Type::BOOLEAN:
    {
        parquet::BoolReader *bool_reader = static_cast<parquet::BoolReader *>(column_reader.get());
        vector<char> value(batchSize);
        while(bool_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += bool_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (bool*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = value[index++];
            }
            else
            {
                buffer[le] = SHRT_MIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::INT32:
    {
        if (col_descr->logical_type() && col_descr->logical_type()->is_valid())
        {
            switch (col_descr->logical_type()->type())
            {
            case parquet::LogicalType::Type::INT:
            case parquet::LogicalType::Type::DATE:
            case parquet::LogicalType::Type::TIME:
                break;
            default:
                throw RuntimeException("unsupported data type");
            }
        }
        if (col_descr->converted_type() && col_descr->converted_type() != parquet::ConvertedType::type::NONE)
        {
            switch (col_descr->converted_type())
            {
            case parquet::ConvertedType::INT_16:
            case parquet::ConvertedType::TIME_MICROS:
            case parquet::ConvertedType::UINT_16:
            case parquet::ConvertedType::UINT_8:
            case parquet::ConvertedType::INT_8:
            case parquet::ConvertedType::INT_32:
            case parquet::ConvertedType::UINT_32:
                break;
            default:
                throw RuntimeException("unsupported data type");
            }
        }
        parquet::Int32Reader *int32_reader = static_cast<parquet::Int32Reader *>(column_reader.get());
        // Read all the rows in the column
        vector<int32_t> value(batchSize);
        while(int32_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int32_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (int32_t*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = value[index++];
            }
            else
            {
                buffer[le] = SHRT_MIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::INT64:
    {
        if (col_descr->logical_type() && col_descr->logical_type()->is_valid())
        {
            switch (col_descr->logical_type()->type())
            {
            case parquet::LogicalType::Type::INT:
            case parquet::LogicalType::Type::TIMESTAMP:
            case parquet::LogicalType::Type::TIME:
                break;
            default:
                throw RuntimeException("unsupported data type");
            }
        }
        if (col_descr->converted_type() && col_descr->converted_type() != parquet::ConvertedType::type::NONE)
        {
            switch (col_descr->converted_type())
            {
            case parquet::ConvertedType::INT_64:
            case parquet::ConvertedType::TIME_MILLIS:
            case parquet::ConvertedType::TIMESTAMP_MICROS:
            case parquet::ConvertedType::TIMESTAMP_MILLIS:
            case parquet::ConvertedType::UINT_64:
                break;
            default:
                throw RuntimeException("unsupported data type");
            }
        }
        parquet::Int64Reader *int64_reader = static_cast<parquet::Int64Reader *>(column_reader.get());
        vector<int64_t> value(batchSize);
        while(int64_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int64_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (int64_t*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = value[index++];
            }
            else
            {
                buffer[le] = SHRT_MIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::INT96:
    {
        parquet::Int96Reader *int96_reader = static_cast<parquet::Int96Reader *>(column_reader.get());
        vector<parquet::Int96> value(batchSize);
        while(int96_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int96_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (parquet::Int96*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = value[index++].value[2];
            }
            else
            {
                buffer[le] = SHRT_MIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::DOUBLE:
    {
        parquet::DoubleReader *double_reader = static_cast<parquet::DoubleReader *>(column_reader.get());
        vector<double> value(batchSize);
        while(double_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += double_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (double*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }       
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = value[index++];
            }
            else
            {
                buffer[le] = SHRT_MIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::FLOAT:
    {
        parquet::FloatReader *float_reader = static_cast<parquet::FloatReader *>(column_reader.get());
        vector<float> value(batchSize);
        while(float_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += float_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (float*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = value[index++];
            }
            else
            {
                buffer[le] = SHRT_MIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::FIXED_LEN_BYTE_ARRAY:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:INT32" + "->" + Util::getDataTypeString(DT_SHORT));
    case parquet::Type::BYTE_ARRAY:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:INT32" + "->" + Util::getDataTypeString(DT_SHORT));

    default:
        throw RuntimeException("unsupported data type");
    }
    return 0;
}

int convertParquetToDolphindbLong(int col_idx, std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, long long* buffer, DATA_TYPE times_t, size_t batchSize, bool& containNull)
{
    int64_t values_read = 0;
    vector<short> def_level(batchSize);
    vector<short> rep_level(batchSize);
    int64_t rows_read = 0;
    switch (col_descr->physical_type())
    {
    case parquet::Type::BOOLEAN:
    {
        parquet::BoolReader *bool_reader = static_cast<parquet::BoolReader *>(column_reader.get());
        vector<char> value(batchSize);
        while(bool_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += bool_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (bool*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = value[index++];
            }
            else
            {
                buffer[le] = LLONG_MIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::INT32:
    {
        parquet::Int32Reader *int32_reader = static_cast<parquet::Int32Reader *>(column_reader.get());
        // Read all the rows in the column

        vector<int32_t> value(batchSize);
        while(int32_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int32_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (int32_t*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }
        containNull = rows_read != values_read;
        if(col_descr->logical_type()->is_decimal() || col_descr->converted_type()==parquet::ConvertedType::DECIMAL)
        {
            int scale=std::pow(10, col_descr->type_scale());
            for(int i = 0; i < values_read; i++)
                value[i]=value[i]/scale;
        }
        parquetTime parquetT = convertParquetTimes(col_descr->logical_type(), col_descr->converted_type());
        if (parquetT != parquetTime::None && times_t != DT_LONG)
        {
            vector<long long> longV;
            switch (times_t)
            {
            case DT_NANOTIME:
                convertToDTNanotime(value, parquetT, longV, values_read);
                break;
            case DT_NANOTIMESTAMP:
                convertToDTNanotimestamp(value, parquetT, longV, values_read);
                break;
            case DT_TIMESTAMP:
                convertToDTTimestamp(value, parquetT, longV, values_read);
                break;
            default:
                break;
            }
            int index = 0;
            for (int64_t le = 0; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    buffer[le] = longV[index++];
                }
                else
                {
                    buffer[le] = LLONG_MIN;
                }
            }
            return rows_read;
        }
         int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = value[index++];
            }
            else
            {
                buffer[le] = LLONG_MIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::INT64:
    {

        parquet::Int64Reader *int64_reader = static_cast<parquet::Int64Reader *>(column_reader.get());
        vector<long long> value(batchSize);
        while(int64_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int64_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (int64_t*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }
        containNull = rows_read != values_read;
        if(col_descr->logical_type()->is_decimal() || col_descr->converted_type()==parquet::ConvertedType::DECIMAL)
        {
            int scale=std::pow(10, col_descr->type_scale());
            for(int i = 0; i < values_read; i++)
                value[i]=value[i]/scale;
        }
        else{
            parquetTime parquetT = convertParquetTimes(col_descr->logical_type(), col_descr->converted_type());
            if (parquetT != parquetTime::None){
                vector<int> intValue(values_read);
                switch (times_t)
                {
                case DT_NANOTIME:
                    convertToDTNanotime(intValue, parquetT, value, values_read);
                    break;
                case DT_NANOTIMESTAMP:
                    convertToDTNanotimestamp(intValue, parquetT, value, values_read);
                    break;
                case DT_TIMESTAMP:
                    convertToDTTimestamp(intValue, parquetT, value, values_read);
                    break;
                default:
                    break;
                }
            }
        }
        int index = 0;
        if(col_descr->sort_order() == parquet::SortOrder::UNSIGNED)
        {
            for (size_t i = 0; i < value.size(); i++)
                if(value[i] < 0)
                    value[i] = LLONG_MIN;
        }
        if (rows_read == values_read && rows_read != 0)
        {
            memcpy(buffer, value.data(), sizeof(long long) * rows_read);
        }
        else{
            for (int64_t le = 0; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    buffer[le] = value[index++];
                }
                else
                {
                    buffer[le] = LLONG_MIN;
                }
            }
        }
        return rows_read;
    }
    case parquet::Type::INT96:
    {
        vector<long long> longValue;
        parquet::Int96Reader *int96_reader = static_cast<parquet::Int96Reader *>(column_reader.get());
        vector<parquet::Int96> value(batchSize);
        while(int96_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int96_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (parquet::Int96*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }
        containNull = rows_read != values_read;
        if(times_t == DT_LONG){
            int index = 0;
            for (int64_t le = 0; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    int64_t v = value[index].value[1];
                    v = v << 32;
                    v += value[index++].value[2];
                    buffer[le] = v;
                }
                else
                {
                    buffer[le] = LLONG_MIN;
                }
            }
        }
        else{
            longValue.resize(values_read);
            for(int i = 0; i <values_read; i++ ){
                longValue[i] = parquet::Int96GetNanoSeconds(value[i]);
            }
            vector<int> intValue;
            switch (times_t)
            {
            case DT_TIMESTAMP:
                convertToDTTimestamp(intValue, parquetTime::TimestampNanos, longValue, values_read);
                break;
            case DT_NANOTIMESTAMP:
                convertToDTNanotimestamp(intValue, parquetTime::TimestampNanos, longValue, values_read);
                break;
            case DT_NANOTIME:
                convertToDTNanotime(intValue, parquetTime::TimestampNanos, longValue, values_read);
                break;
            default:
                break;
            }
            int index = 0;
            for (int64_t le = 0; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    buffer[le] = longValue[index++];
                }
                else
                {
                    buffer[le] = LLONG_MIN;
                }
            }
        }
        return rows_read;
    }
    case parquet::Type::DOUBLE:
    {
        if (times_t != DT_LONG)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:DOUBLE" + "->" + Util::getDataTypeString(times_t));
        parquet::DoubleReader *double_reader = static_cast<parquet::DoubleReader *>(column_reader.get());
        vector<double> value(batchSize);
        while(double_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += double_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (double*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }       
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = value[index++];
            }
            else
            {
                buffer[le] = LLONG_MIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::FLOAT:
    {
        if (times_t != DT_LONG)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:FLOAT" + "->" + Util::getDataTypeString(times_t));
        parquet::FloatReader *float_reader = static_cast<parquet::FloatReader *>(column_reader.get());
        vector<float> value(batchSize);
        while(float_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += float_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (float*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }       
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = value[index++];
            }
            else
            {
                buffer[le] = LLONG_MIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::FIXED_LEN_BYTE_ARRAY:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:FIXED_LEN_BYTE_ARRAY" + "->" + Util::getDataTypeString(DT_LONG));
    case parquet::Type::BYTE_ARRAY:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:BYTE_ARRAY" + "->" + Util::getDataTypeString(DT_LONG));

    default:
        throw RuntimeException("unsupported data type");
    }
    return 0;
}

int convertParquetToDolphindbFloat(int col_idx, std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, float* buffer, size_t batchSize, bool& containNull)
{
    int64_t values_read = 0;
    vector<short> def_level(batchSize);
    vector<short> rep_level(batchSize);
    int64_t rows_read = 0;
    switch (col_descr->physical_type())
    {
    case parquet::Type::BOOLEAN:
    {
        parquet::BoolReader *bool_reader = static_cast<parquet::BoolReader *>(column_reader.get());
        vector<char> value(batchSize);
        while(bool_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += bool_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (bool*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }       
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = value[index++];
            }
            else
            {
                buffer[le] = FLT_NMIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::INT32:
    {
        parquet::Int32Reader *int32_reader = static_cast<parquet::Int32Reader *>(column_reader.get());
        // Read all the rows in the column
        bool Decimal=isDecimal(col_descr);
        int scale=std::pow(10, col_descr->type_scale());
        vector<int32_t> value(batchSize);
        while(int32_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int32_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (int32_t*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }
        containNull = rows_read != values_read;              
        int index = 0;
        if(Decimal){
            for (int64_t le = 0; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    buffer[le] = ((double)value[index++])/scale;
                }
                else
                {
                    buffer[le] = FLT_NMIN;
                }
            }

        }else{
            for (int64_t le = 0; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    buffer[le] = value[index++];
                }
                else
                {
                    buffer[le] = FLT_NMIN;
                }
            }
        }
        return rows_read;
    }
    case parquet::Type::INT64:
    {
        long long scale=std::pow(10, col_descr->type_scale());
        bool Decimal=isDecimal(col_descr);
        parquet::Int64Reader *int64_reader = static_cast<parquet::Int64Reader *>(column_reader.get());
        vector<int64_t> value(batchSize);
        while(int64_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int64_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (int64_t*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        int index = 0;
        if(Decimal){
            for (int64_t le = 0; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    buffer[le] = ((double)value[index++])/scale;
                }
                else
                {
                    buffer[le] = FLT_NMIN;
                }
            }

        }else{
            for (int64_t le = 0; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    buffer[le] = value[index++];
                }
                else
                {
                    buffer[le] = FLT_NMIN;
                }
            }
        }
        return rows_read;
    }
    case parquet::Type::INT96:
    {
        parquet::Int96Reader *int96_reader = static_cast<parquet::Int96Reader *>(column_reader.get());
        vector<parquet::Int96> value(batchSize);
        while(int96_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int96_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (parquet::Int96*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                int64_t v = value[index].value[1];
                v = v << 32;
                v += value[index++].value[2];
                buffer[le] = v;
            }
            else
            {
                buffer[le] = FLT_NMIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::DOUBLE:
    {
        parquet::DoubleReader *double_reader = static_cast<parquet::DoubleReader *>(column_reader.get());
        vector<double> value(batchSize);
        while(double_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += double_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (double*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = value[index++];
            }
            else
            {
                buffer[le] = FLT_NMIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::FLOAT:
    {
        parquet::FloatReader *float_reader = static_cast<parquet::FloatReader *>(column_reader.get());
        vector<float> value(batchSize);
        while(float_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += float_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (float*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        int index = 0;
        if(rows_read == values_read){
            memcpy(buffer, value.data(), sizeof(float) * rows_read);
        }
        else{
            for (int64_t le = 0; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    buffer[le] = value[index++];
                }
                else
                {
                    buffer[le] = FLT_NMIN;
                }
            }
        }
        return rows_read;
    }
    case parquet::Type::FIXED_LEN_BYTE_ARRAY:
    {
        int fixed_length = col_descr->type_length();
        parquet::FixedLenByteArrayReader *flba_reader = static_cast<parquet::FixedLenByteArrayReader *>(column_reader.get());
        vector<parquet::FixedLenByteArray> value(batchSize);
        while(flba_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            int64_t le = rows_read;
            rows_read += flba_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (parquet::FixedLenByteArray*)value.data(), &valuesRead);
            int index = 0;
            for (; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    arrow::Result<arrow::Decimal128> t = arrow::Decimal128::FromBigEndian(value[index++].ptr, fixed_length);
                    if(!t.ok()){
                        throw RuntimeException(t.status().ToString());
                    }
                    buffer[le] = (*t).ToFloat(col_descr->type_scale());
                }
                else
                {
                    buffer[le] = FLT_NMIN;
                }
            }
        }
        return rows_read;
    }
    case parquet::Type::BYTE_ARRAY:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:BYTE_ARRAY" + "->" + Util::getDataTypeString(DT_FLOAT));

    default:
        throw RuntimeException("unsupported data type");
    }
    return 0;
}

int convertParquetToDolphindbDouble(int col_idx, std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, double* buffer, size_t batchSize, bool& containNull)
{
    int64_t values_read = 0;
    vector<short> def_level(batchSize);
    vector<short> rep_level(batchSize);
    int64_t rows_read = 0;
    switch (col_descr->physical_type())
    {
    case parquet::Type::BOOLEAN:
    {
        parquet::BoolReader *bool_reader = static_cast<parquet::BoolReader *>(column_reader.get());
        vector<char> value(batchSize);
        while(bool_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += bool_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (bool*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = value[index++];
            }
            else
            {
                buffer[le] = DBL_NMIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::INT32:
    {
        parquet::Int32Reader *int32_reader = static_cast<parquet::Int32Reader *>(column_reader.get());
        // Read all the rows in the column
        bool Decimal=isDecimal(col_descr);
        int scale=std::pow(10, col_descr->type_scale());
        vector<int32_t> value(batchSize);
        while(int32_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int32_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (int32_t*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        int index = 0;
        if(Decimal){
            for (int64_t le = 0; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    buffer[le] = ((double)value[index++])/scale;
                }
                else
                {
                    buffer[le] = DBL_NMIN;
                }
            }

        }else{
            for (int64_t le = 0; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    buffer[le] = value[index++];
                }
                else
                {
                    buffer[le] = DBL_NMIN;
                }
            }
        }
        return rows_read;
    }
    case parquet::Type::INT64:
    {
        long long scale=std::pow(10, col_descr->type_scale());
        bool Decimal=isDecimal(col_descr);
        parquet::Int64Reader *int64_reader = static_cast<parquet::Int64Reader *>(column_reader.get());
        vector<int64_t> value(batchSize);
        while(int64_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int64_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (int64_t*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        int index = 0;
        if(Decimal){
            for (int64_t le = 0; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    buffer[le] = ((double)value[index++])/scale;
                }
                else
                {
                    buffer[le] = DBL_NMIN;
                }
            }

        }else{
            for (int64_t le = 0; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    buffer[le] = value[index++];
                }
                else
                {
                    buffer[le] = DBL_NMIN;
                }
            }
        }
        return rows_read;
    }
    case parquet::Type::INT96:
    {
        parquet::Int96Reader *int96_reader = static_cast<parquet::Int96Reader *>(column_reader.get());
        vector<parquet::Int96> value(batchSize);
        while(int96_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int96_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (parquet::Int96*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                int64_t v = value[index].value[1];
                v = v << 32;
                v += value[index++].value[2];
                buffer[le] = v;
            }
            else
            {
                buffer[le] = DBL_NMIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::DOUBLE:
    {
        parquet::DoubleReader *double_reader = static_cast<parquet::DoubleReader *>(column_reader.get());
        vector<double> value(batchSize);
        while(double_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += double_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (double*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        int index = 0;
        if(rows_read == values_read){
            memcpy(buffer, value.data(), sizeof(double) * rows_read);
        }
        else{
            for (int64_t le = 0; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    buffer[le] = value[index++];
                }
                else
                {
                    buffer[le] = DBL_NMIN;
                }
            }
        }
        return rows_read;
    }
    case parquet::Type::FLOAT:
    {
        parquet::FloatReader *float_reader = static_cast<parquet::FloatReader *>(column_reader.get());
        vector<float> value(batchSize);
        while(float_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += float_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (float*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        int index = 0;
        if(rows_read == values_read){
            for (int64_t le = 0; le < rows_read; le++)
            {
                buffer[le] = value[le];
            }        
        }
        else{
            for (int64_t le = 0; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    buffer[le] = value[index++];
                }
                else
                {
                    buffer[le] = FLT_NMIN;
                }
            }
        }
        return rows_read;
    }
    case parquet::Type::FIXED_LEN_BYTE_ARRAY:
    {
        int fixed_length = col_descr->type_length();
        parquet::FixedLenByteArrayReader *flba_reader = static_cast<parquet::FixedLenByteArrayReader *>(column_reader.get());
        vector<parquet::FixedLenByteArray> value(batchSize);
        while(flba_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            int64_t le = rows_read;
            rows_read += flba_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (parquet::FixedLenByteArray*)value.data(), &valuesRead);
            int index = 0;
            for (; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    arrow::Result<arrow::Decimal128> t = arrow::Decimal128::FromBigEndian(value[index++].ptr, fixed_length);
                    if(!t.ok()){
                        throw RuntimeException(t.status().ToString());
                    }
                    buffer[le] = (*t).ToDouble(col_descr->type_scale());
                }
                else
                {
                    buffer[le] = DBL_NMIN;
                }
            }
        }
        return rows_read;
    }
    case parquet::Type::BYTE_ARRAY:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:BYTE_ARRAY" + "->" + Util::getDataTypeString(DT_DOUBLE));

    default:
        throw RuntimeException("unsupported data type");
    }
    return 0;
}

int convertParquetToDolphindbString(int col_idx, std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, vector<string> &buffer, DATA_TYPE string_t, size_t batchSize, bool& containNull)
{
    int64_t values_read = 0;
    vector<short> def_level(batchSize);
    vector<short> rep_level(batchSize);
    int64_t rows_read = 0;
    switch (col_descr->physical_type())
    {
    case parquet::Type::BOOLEAN:
    {
        if (string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:BOOLEAN" + "->" + Util::getDataTypeString(string_t));
        parquet::BoolReader *bool_reader = static_cast<parquet::BoolReader *>(column_reader.get());
        vector<char> value(batchSize);
        while(bool_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += bool_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (bool*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = value[index++]? "true":"false";
            }
            else
            {
                buffer[le] = "";
            }
        }
        return rows_read;
    }
    case parquet::Type::INT32:
    {
        if (string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:INT32" + "->" + Util::getDataTypeString(string_t));
        parquet::Int32Reader *int32_reader = static_cast<parquet::Int32Reader *>(column_reader.get());
        vector<int32_t> value(batchSize);
        while(int32_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int32_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (int32_t*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = std::to_string(value[index++]);
            }
            else
            {
                buffer[le] = "";
            }
        }
        return rows_read;
    }
    case parquet::Type::INT64:
    {
        if (string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:INT32" + "->" + Util::getDataTypeString(string_t));
        parquet::Int64Reader *int64_reader = static_cast<parquet::Int64Reader *>(column_reader.get());
        vector<int64_t> value(batchSize);
        while(int64_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int64_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (int64_t*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = std::to_string(value[index++]);
            }
            else
            {
                buffer[le] = "";
            }
        }
        return rows_read;
    }
    case parquet::Type::INT96:
    {
        if (string_t == DT_UUID)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:INT96" + "->" + Util::getDataTypeString(string_t));
        parquet::Int96Reader *int96_reader = static_cast<parquet::Int96Reader *>(column_reader.get());
        vector<parquet::Int96> value(batchSize);
        while(int96_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int96_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (parquet::Int96*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        int index = 0;
        if(string_t == DT_INT128){
            for (int64_t le = 0; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    char bytes[16];
                    for (int i = 0; i < 3; i++)
                    {
                        bytes[i * 4] = (char)((value[index].value[i] >> 24) & 0xFF);
                        bytes[i * 4 + 1] = (char)((value[index].value[i] >> 16) & 0xFF);
                        bytes[i * 4 + 2] = (char)((value[index].value[i] >> 8) & 0xFF);
                        bytes[i * 4 + 3] = (char)(value[index].value[i] & 0xFF);
                    }
                    buffer[le] = string(bytes);
                    index++;
                }
                else
                {
                    buffer[le] = "";
                }
            }
        }else{
            for (int64_t le = 0; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    buffer[le] = parquet::Int96ToString(value[index++]);
                }
                else
                {
                    buffer[le] = "";
                }
            }
        }
    }
    case parquet::Type::DOUBLE:
    {
        if (string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:INT32" + "->" + Util::getDataTypeString(string_t));
        parquet::DoubleReader *double_reader = static_cast<parquet::DoubleReader *>(column_reader.get());
        vector<double> value(batchSize);
        while(double_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += double_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (double*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }       
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = std::to_string(value[index++]);
            }
            else
            {
                buffer[le] = "";
            }
        }
        return rows_read;
    }
    case parquet::Type::FLOAT:
    {
        if (string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:INT32" + "->" + Util::getDataTypeString(string_t));
        parquet::FloatReader *float_reader = static_cast<parquet::FloatReader *>(column_reader.get());
        vector<float> value(batchSize);
        while(float_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += float_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (float*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }              
        containNull = rows_read != values_read;
        int index = 0;
        for (int64_t le = 0; le < rows_read; le++)
        {
            if (def_level[le] >= col_descr->max_definition_level())
            {
                buffer[le] = std::to_string(value[index++]);
            }
            else
            {
                buffer[le] = "";
            }
        }
        return rows_read;
    }
    case parquet::Type::FIXED_LEN_BYTE_ARRAY:
    {
        if (string_t == DT_INT128)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:FIXED_LEN_BYTE_ARRAY" + "->" + Util::getDataTypeString(string_t));
        int fixed_length = col_descr->type_length();
        if (string_t == DT_UUID && fixed_length != 16)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:FIXED_LEN_BYTE_ARRAY, length NOT 16" + "->" + Util::getDataTypeString(string_t));
        
        parquet::FixedLenByteArrayReader *flba_reader = static_cast<parquet::FixedLenByteArrayReader *>(column_reader.get());
        vector<parquet::FixedLenByteArray> value(batchSize);
        while(flba_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            int64_t le = rows_read;
            rows_read += flba_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (parquet::FixedLenByteArray*)value.data(), &valuesRead);
            int index = 0;
            for (; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    buffer[le] =std::string(reinterpret_cast<const char *>(value[index++].ptr), fixed_length);

                }
                else
                {
                    buffer[le] = "";
                    containNull = true;
                }
            }
        }
        return rows_read;
    }
    case parquet::Type::BYTE_ARRAY:
    {
        if (string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:BYTE_ARRAY" + "->" + Util::getDataTypeString(string_t));
        parquet::ByteArrayReader *ba_reader = static_cast<parquet::ByteArrayReader *>(column_reader.get());
        vector<parquet::ByteArray> value(batchSize);
        while(ba_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            int64_t le = rows_read;
            rows_read += ba_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (parquet::ByteArray*)value.data(), &valuesRead);
            int index = 0;
            for (; le < rows_read; le++)
            {
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    buffer[le] =parquet::ByteArrayToString(value[index++]);

                }
                else
                {
                    buffer[le] = "";
                    containNull = true;
                }
            }
        }
        return rows_read;
    }
    default:
        throw RuntimeException("unsupported data type");
    }
    return 0;
}

ConstantSP loadParquet(const string &filename, const ConstantSP &schema, const ConstantSP &column, const int rowGroupStart, const int rowGroupNum)
{
    ParquetReadOnlyFile file(filename);
    std::shared_ptr<parquet::FileMetaData> file_metadata = file.fileMetadataReader();
    const parquet::SchemaDescriptor *s = file_metadata->schema();
    int col_num = s->num_columns();
    VectorSP columnToRead;
    if(column->isNull()){
        columnToRead = Util::createIndexVector(0, col_num);
    }else{
        columnToRead = column;
        int maxIndex = columnToRead->max()->getInt();
        int minIndex = columnToRead->min()->getInt();
        if(maxIndex >= col_num){
            throw IllegalArgumentException("loadParquet", "Invalid column index " + std::to_string(maxIndex) + " to load");
        }
        if(minIndex < 0){
            throw IllegalArgumentException("loadParquet", "Invalid column index " + std::to_string(minIndex) + " to load");        
        }
    }
    int row_group_num = file_metadata->num_row_groups();
    if (rowGroupStart >= row_group_num)
        throw RuntimeException("rowGroupStart to read is out of range.");
    int rowGroupEnd;
    if (rowGroupNum == 0)
    {
        rowGroupEnd = row_group_num;
    }
    else
    {
        rowGroupEnd = rowGroupStart + rowGroupNum;
        if (rowGroupEnd >= row_group_num)
            rowGroupEnd = row_group_num;
    }
    ConstantSP init_schema = schema;
    if (schema->isNull())
    {
        vector<ConstantSP> cols(2);
        if (!getSchemaCol(s, columnToRead, cols))
            throw RuntimeException("get schema failed");
        vector<string> colNames(2);
        colNames[0] = "name";
        colNames[1] = "type";
        init_schema = Util::createTable(colNames, cols);
    }
    return loadParquet(&file, init_schema, columnToRead, rowGroupStart, rowGroupEnd);
}

ConstantSP loadParquetHdfs(void *buffer, int64_t len)
{
    std::shared_ptr<arrow::io::BufferReader> buf = std::make_shared<arrow::io::BufferReader>((uint8_t *)buffer, len);
    int rowGroupStart = 0;
    int rowGroupNum = 0;
    ConstantSP schema = ParquetPluginImp::nullSP;
    ConstantSP column = new Void();
    ParquetReadOnlyFile file;
    file.open(buf);
    std::shared_ptr<parquet::FileMetaData> file_metadata = file.fileMetadataReader();
    const parquet::SchemaDescriptor *s = file_metadata->schema();
    int col_num = s->num_columns();
    VectorSP columnToRead;
    if(column->isNull()){
        columnToRead = Util::createIndexVector(0, col_num);
    }else{
        columnToRead = column;
        int maxIndex = columnToRead->max()->getInt();
        int minIndex = columnToRead->min()->getInt();
        if(maxIndex >= col_num){
            throw IllegalArgumentException("loadParquet", "Invalid column index " + std::to_string(maxIndex) + " to load");
        }
        if(minIndex < 0){
            throw IllegalArgumentException("loadParquet", "Invalid column index " + std::to_string(minIndex) + " to load");        
        }
    }
    int row_group_num = file_metadata->num_row_groups();
    if (rowGroupStart >= row_group_num)
        throw RuntimeException("rowGroupStart to read is out of range.");
    int rowGroupEnd;
    if (rowGroupNum == 0)
    {
        rowGroupEnd = row_group_num;
    }
    else
    {
        rowGroupEnd = rowGroupStart + rowGroupNum;
        if (rowGroupEnd >= row_group_num)
            rowGroupEnd = row_group_num;
    }
    ConstantSP init_schema = schema;
    if (schema->isNull())
    {
        vector<ConstantSP> cols(2);
        if (!getSchemaCol(s, columnToRead, cols))
            throw RuntimeException("get schema failed");
        vector<string> colNames(2);
        colNames[0] = "name";
        colNames[1] = "type";
        init_schema = Util::createTable(colNames, cols);
    }
    return loadParquet(&file, init_schema, columnToRead, rowGroupStart, rowGroupEnd);
}

ConstantSP loadParquet(ParquetReadOnlyFile *file, const ConstantSP &schema, const ConstantSP &column, const int rowGroupStart, const int rowGroupEnd)
{
    TableSP tableWithSchema = DBFileIO::createEmptyTableFromSchema(schema);
    int col_num = column->size();
    char buf[1024 * 64 * 8];
    long long rows_read = 0;
    for (int row = rowGroupStart; row < rowGroupEnd; row++)
    {
        std::shared_ptr<parquet::RowGroupReader> row_reader = file->rowReader(row);
        if (row_reader == nullptr)
            throw RuntimeException("Read parquet file failed.");
        size_t totalRows = row_reader->metadata()->num_rows();

        size_t startRow = 0;
        size_t batchRow = totalRows > 1024 * 64 ? 1024 * 64 : totalRows;
        vector<std::shared_ptr<parquet::ColumnReader>> colReaders;
        for (int i = 0; i < col_num; i++){
            int col_idx = column->getInt(i);
            std::shared_ptr<parquet::ColumnReader> column_reader = row_reader->Column(col_idx);
            colReaders.push_back(column_reader);
        }
        INDEX insertRows = 1;
        while(startRow < totalRows && insertRows != 0){
            vector<VectorSP> dolphindbCol(col_num);
            createNewVectorSP(dolphindbCol, tableWithSchema);
            for (int i = 0; i < col_num; i++)
            {
                std::shared_ptr<parquet::ColumnReader> column_reader = colReaders[i];
                const parquet::ColumnDescriptor *col_descr = column_reader->descr();
                if (col_descr->max_repetition_level() != 0)
                    throw RuntimeException("not support parquet repeated field yet.");
                DATA_TYPE dolphin_t = dolphindbCol[i]->getType();
                bool containNull;
                switch (dolphin_t)
                {
                case DT_BOOL:
                {
                    rows_read = convertParquetToDolphindbBool(i, column_reader, col_descr, buf, batchRow, containNull);
                    dolphindbCol[i]->appendChar(buf, rows_read);
                    dolphindbCol[i]->setNullFlag(containNull);
                    break;
                }
                case DT_CHAR:
                {
                    rows_read = convertParquetToDolphindbChar(i, column_reader, col_descr, buf, batchRow, containNull);
                    dolphindbCol[i]->appendChar(buf, rows_read);
                    dolphindbCol[i]->setNullFlag(containNull);
                    break;
                }
                case DT_DATE:
                case DT_MONTH:
                case DT_TIME:
                case DT_SECOND:
                case DT_MINUTE:
                case DT_DATETIME:
                case DT_INT:
                {
                    rows_read = convertParquetToDolphindbInt(i, column_reader, col_descr, (int*)buf, dolphin_t, batchRow, containNull);
                    dolphindbCol[i]->appendInt((int*)buf, rows_read);
                    dolphindbCol[i]->setNullFlag(containNull);
                    break;
                }
                case DT_LONG:
                case DT_NANOTIME:
                case DT_NANOTIMESTAMP:
                case DT_TIMESTAMP:
                {
                    rows_read = convertParquetToDolphindbLong(i, column_reader, col_descr, (long long*)buf, dolphin_t, batchRow, containNull);
                    dolphindbCol[i]->appendLong((long long *)buf, rows_read);
                    dolphindbCol[i]->setNullFlag(containNull);
                    break;
                }
                case DT_SHORT:
                {
                    rows_read = convertParquetToDolphindbShort(i, column_reader, col_descr, (short*)buf, batchRow, containNull);
                    dolphindbCol[i]->appendShort((short *)buf, rows_read);
                    dolphindbCol[i]->setNullFlag(containNull);
                    break;
                }
                case DT_FLOAT:
                {
                    rows_read = convertParquetToDolphindbFloat(i, column_reader, col_descr, (float*)buf, batchRow, containNull);
                    dolphindbCol[i]->appendFloat((float *)buf, rows_read);
                    dolphindbCol[i]->setNullFlag(containNull);
                    break;
                }
                case DT_DOUBLE:
                {
                    rows_read = convertParquetToDolphindbDouble(i, column_reader, col_descr, (double*)buf, batchRow, containNull);
                    dolphindbCol[i]->appendDouble((double *)buf, rows_read);
                    dolphindbCol[i]->setNullFlag(containNull);
                    break;
                }
                case DT_INT128:
                case DT_UUID:
                case DT_STRING:
                case DT_SYMBOL:
                {
                    vector<string> buffer(batchRow);
                    rows_read =  convertParquetToDolphindbString(i, column_reader, col_descr, buffer, dolphin_t, batchRow, containNull);
                    dolphindbCol[i]->appendString(buffer.data(), rows_read);
                    dolphindbCol[i]->setNullFlag(containNull);
                    break;
                }
                default:
                    throw RuntimeException("unsupported data type.");
                    break;
                }
            }
            tableWithSchema = appendColumnVecToTable(tableWithSchema, dolphindbCol, insertRows);
            startRow += insertRows;
        }
    }
    return tableWithSchema;
}

ConstantSP loadFromParquetToDatabase(Heap *heap, vector<ConstantSP> &arguments)
{
    ParquetReadOnlyFile *file = (ParquetReadOnlyFile *)(arguments[0]->getLong());
    TableSP schema = static_cast<TableSP>(arguments[1]);
    VectorSP columnArg = arguments[2];
    int rowGroup = arguments[3]->getInt();
    int rowGroupEnd = arguments[4]->getInt();
    SystemHandleSP db = static_cast<SystemHandleSP>(arguments[5]);
    string tableName = arguments[6]->getString();

    bool diskSeqMode = !db->getDatabaseDir().empty() &&
                       db->getDomain()->getPartitionType() == SEQ;
    TableSP loadedTable = loadParquet(file, schema, columnArg, rowGroup, rowGroupEnd);
    FunctionDefSP transform = (FunctionDefSP)arguments[8];
    if(!transform.isNull() && !transform->isNull()){
        vector<ConstantSP> arg={loadedTable};
        loadedTable = transform->call(heap, arg);
    }
    if (diskSeqMode)
    {
        string id = db->getDomain()->getPartition(arguments[7]->getInt())->getPath();
        string directory = db->getDatabaseDir() + "/" + id;
        if (!DBFileIO::saveBasicTable(heap->currentSession(), directory, loadedTable.get(), tableName, NULL, true, 1, false))
            throw RuntimeException("Failed to save the table to directory " + directory);
        return new Long(loadedTable->rows());
    }
    else
        return loadedTable;
}

ConstantSP savePartition(Heap *heap, vector<ConstantSP> &arguments){
    SystemHandleSP db = arguments[0];
    ConstantSP tb = arguments[1];
    ConstantSP tbInMemory = arguments[2];
    string dbPath = db->getDatabaseDir();
    FunctionDefSP append = heap->currentSession()->getFunctionDef("append!");
    vector<ConstantSP> appendArgs = {tb, tbInMemory};
    append->call(heap, appendArgs);
    return new Void();
}

ConstantSP loadParquetEx(Heap *heap, const SystemHandleSP &db, const string &tableName, const ConstantSP &partitionColumns,
                         const string &filename, const TableSP &schema,
                         const ConstantSP &column, const int rowGroupStart, const int rowGroupNum, const ConstantSP &transform)
{
    ParquetReadOnlyFile *f = new ParquetReadOnlyFile(filename);
    std::shared_ptr<parquet::FileMetaData> file_metadata = f->fileMetadataReader();
    const parquet::SchemaDescriptor *s = file_metadata->schema();
    int col_num = s->num_columns();
    VectorSP columnToRead;
    if(column->isNull()){
        columnToRead = Util::createIndexVector(0, col_num);
    }else{
        columnToRead = column;
        int maxIndex = columnToRead->max()->getInt();
        int minIndex = columnToRead->min()->getInt();
        if(maxIndex >= col_num){
            throw IllegalArgumentException("loadParquetEx", "Invalid column index " + std::to_string(maxIndex) + " to load");
        }
        if(minIndex < 0){
            throw IllegalArgumentException("loadParquetEx", "Invalid column index " + std::to_string(minIndex) + " to load");        
        }
    }
    TableSP convertedSchema;

    if (schema->isNull())
    {
        vector<ConstantSP> cols_d(2);
        if (!getSchemaCol(s, columnToRead, cols_d))
            throw RuntimeException("get schema failed");
        vector<string> colNames(2);
        colNames[0] = "name";
        colNames[1] = "type";
        convertedSchema = Util::createTable(colNames, cols_d);
    }
    else
    {
        convertedSchema = schema;
    }

    vector<DistributedCallSP> tasks = generateParquetTasks(heap, f, convertedSchema, columnToRead, rowGroupStart, rowGroupNum, db, tableName, transform);
    int partitions = tasks.size();
    string owner = heap->currentSession()->getUser()->getUserId();
    DomainSP domain = db->getDomain();
    bool seqDomain = domain->getPartitionType() == SEQ;
    bool inMemory = db->getDatabaseDir().empty();
    ConstantSP tableName_ = new String(tableName);
    if (seqDomain)
    {
        StaticStageExecutor executor(false, false, false);
        executor.execute(heap, tasks);
        for (int i = 0; i < partitions; i++)
        {
            const string &errMsg = tasks[i]->getErrorMessage();
            if (!errMsg.empty())
                throw RuntimeException(errMsg);
        }   
        if (inMemory)
        {
            ConstantSP tmpTables = Util::createVector(DT_ANY, partitions);
            for (int i = 0; i < partitions; i++)
                tmpTables->set(i, tasks[i]->getResultObject());
            ConstantSP partitionNames = new String("");
            return generateInMemoryParitionedTable(heap, db, tmpTables, partitionNames);
        }
        else
        {
            vector<int> partitionColumnIndices(1, -1);
            vector<int> baseIds;
            int baseId = -1;
            string tableFile = db->getDatabaseDir() + "/" + tableName + ".tbl";
            vector<ColumnDesc> cols;
            int columns = convertedSchema->rows();
            for (int i = 0; i < columns; ++i)
            {
                string name = convertedSchema->getColumn(0)->getString(i);
                DATA_TYPE type = Util::getDataType(convertedSchema->getColumn(1)->getString(i));
                int extra = type == DT_SYMBOL ? baseId : -1;
                cols.push_back(ColumnDesc(name, type, extra));
            }

            if (!DBFileIO::saveTableHeader(owner, cols, partitionColumnIndices, 0, tableFile, NULL))
                throw IOException("Failed to save table header " + tableFile);
            if (!DBFileIO::saveDatabase(db.get()))
                throw IOException("Failed to save database " + db->getDatabaseDir());
            db->getDomain()->addTable(tableName, owner, cols, partitionColumnIndices);
            vector<ConstantSP> loadTableArgs = {db, tableName_};
            return heap->currentSession()->getFunctionDef("loadTable")->call(heap, loadTableArgs);
        }
    }
    else
    {
        string dbPath = db->getDatabaseDir();
        vector<ConstantSP> existsTableArgs = {new String(dbPath), tableName_};
        bool existsTable = heap->currentSession()->getFunctionDef("existsTable")->call(heap, existsTableArgs)->getBool();
        ConstantSP result;

        if (existsTable)
        {
            vector<ConstantSP> loadTableArgs = {db, tableName_};
            result = heap->currentSession()->getFunctionDef("loadTable")->call(heap, loadTableArgs);
        }
        else
        {
            ConstantSP dummyTable = DBFileIO::createEmptyTableFromSchema(convertedSchema);
            vector<ConstantSP> createTableArgs = {db, dummyTable, tableName_, partitionColumns};
            result = heap->currentSession()->getFunctionDef("createPartitionedTable")->call(heap, createTableArgs);
        }
        vector<FunctionDefSP> functors;
        FunctionDefSP func = Util::createSystemFunction("savePartition",&savePartition, 3, 3, false);
        vector<ConstantSP> args(2);
        args[0] = db;
        args[1] = result;
        functors.push_back(Util::createPartialFunction(func, args));
        //int parallel = 10;
        PipelineStageExecutor executor(functors, false, 4, 2);
        executor.execute(heap, tasks);

        for(int i=0; i<partitions; ++i){
            if(!tasks[i]->getErrorMessage().empty()){
                string errMsg;
                errMsg = tasks[i]->getErrorMessage();
                throw RuntimeException(errMsg);
            }
        }
        if (!inMemory) {
            vector<ConstantSP> loadTableArgs = {db, tableName_};
            result = heap->currentSession()->getFunctionDef("loadTable")->call(heap, loadTableArgs);
        }
        return result;
    }
}

TableSP generateInMemoryParitionedTable(Heap *heap, const SystemHandleSP &db,
                                        const ConstantSP &tables, const ConstantSP &partitionNames)
{
    FunctionDefSP createPartitionedTable = heap->currentSession()->getFunctionDef("createPartitionedTable");
    ConstantSP emptyString = new String("");
    vector<ConstantSP> args{db, tables, emptyString, partitionNames};
    return createPartitionedTable->call(heap, args);
}

void getParquetReadOnlyFile(Heap *heap, vector<ConstantSP> &arguments)
{
    ParquetReadOnlyFile *file = reinterpret_cast<ParquetReadOnlyFile *>(arguments[0]->getLong());
    if(file!=nullptr)
    {
        delete file;
    }
}

vector<DistributedCallSP> generateParquetTasks(Heap *heap, ParquetReadOnlyFile *file,
                                               const TableSP &schema, const ConstantSP &column, const int rowGroupStart, const int rowGroupNum,
                                               const SystemHandleSP &db, const string &tableName, const ConstantSP &transform)
{
    int maxRowGroupNum = file->fileMetadataReader()->num_row_groups();
    if(rowGroupStart>=maxRowGroupNum){
        throw RuntimeException("rowGroupStart to read is out of range.");
    }
    int partitions = maxRowGroupNum-rowGroupStart;
    if (rowGroupNum != 0)
        partitions = std::min((int)rowGroupNum, partitions);

    DomainSP domain = db->getDomain();

    if (domain->getPartitionType() == SEQ)
    {
        if (domain->getPartitionCount() <= 1)
            throw IOException("The database must have at least two partitions.");
        else if (partitions != domain->getPartitionCount())
            throw IOException("The database's number of partitions must match the number of row group in parquet file.");
    }

    vector<DistributedCallSP> tasks;
    ConstantSP _tableName = new String(tableName);
    const char *fmt = "Read parquet file";
    FunctionDefSP getParquetFileHead(Util::createSystemProcedure("getParquetReadOnlyFile", getParquetReadOnlyFile, 1, 1));
    ConstantSP parquetFile = Util::createResource(reinterpret_cast<long long>(file), fmt, getParquetFileHead, heap->currentSession());
    FunctionDefSP func = Util::createSystemFunction("loadFromParquetToDatabase", loadFromParquetToDatabase, 9, 9, false);
    //ConstantSP parquetFile=new Long((long long)file);
    for (int i = 0; i < partitions; i++)
    {
        ConstantSP rowGroupNo = new Long(rowGroupStart+i);
        ConstantSP rowGroupEnd = new Long(rowGroupStart+i+1);
        ConstantSP id = new Int(i);
        vector<ConstantSP> args{parquetFile, schema, column, rowGroupNo, rowGroupEnd, db, _tableName, id, transform};
        ObjectSP call = Util::createRegularFunctionCall(func, args);
        //ConstantSP t = func->call(heap,args);
        DistributedCallSP task = new DistributedCall(call, true);
        tasks.push_back(task);
    }
    return tasks;
}

int getRowGroup(const string &filename)
{
    ParquetReadOnlyFile f(filename);
    return f.fileMetadataReader()->num_row_groups();
}

ConstantSP parquetDS(const ConstantSP &filename, const ConstantSP &schema)
{
    int rowGroupNum = getRowGroup(filename->getString());
    ConstantSP dataSources = Util::createVector(DT_ANY, rowGroupNum);
    ConstantSP column = ParquetPluginImp::nullSP;
    FunctionDefSP _loadParquet = Util::createSystemFunction("loadParquet", ::loadParquet, 1, 5, false);
    for (int i = 0; i < rowGroupNum; i++)
    {
        ConstantSP _rowNumStart = new Int(i);
        ConstantSP _rowGroup = new Int(1);
        vector<ConstantSP> args{filename, schema, column, _rowNumStart, _rowGroup};
        ObjectSP code = Util::createRegularFunctionCall(_loadParquet, args);
        ConstantSP ds = new DataSource(code);
        dataSources->set(i, ds);
    }
    return dataSources;
}

const int16_t ONE = 1;
const int16_t ZERO = 0;

ConstantSP saveParquet(ConstantSP &table, const string &filename)
{
    std::shared_ptr<arrow::io::FileOutputStream> outfile;
    PARQUET_ASSIGN_OR_THROW(
        outfile,
        arrow::io::FileOutputStream::Open(filename)
    );
    
    std::shared_ptr<parquet::schema::GroupNode> schema = getParquetSchemaFromDolphindb(table);
    parquet::WriterProperties::Builder builder;
    std::unique_ptr<parquet::ParquetFileWriter> writer = parquet::ParquetFileWriter::Open(
        outfile,
        schema,
        builder.build()
    );

    int colNum = table->columns();
    parquet::RowGroupWriter *rowGroupWriter = writer->AppendRowGroup();
    for(int i = 0; i < colNum; ++i)
    {
        parquet::ColumnWriter *columnWriter = rowGroupWriter->NextColumn();
        VectorSP dolphinCol = table->getColumn(i);
        if(dolphinCol->size() == 0){
            continue;
        }
        ConstantSP front = dolphinCol->get(0);
        switch(front->getType())
        {
        case DT_BOOL:
        {
            parquet::BoolWriter *parquetCol = dynamic_cast<parquet::BoolWriter*>(columnWriter);
            writeBoolToParquet(parquetCol, dolphinCol);
            break;
        }
        case DT_CHAR:
        {
            parquet::Int32Writer *parquetCol = dynamic_cast<parquet::Int32Writer*>(columnWriter);
            writeIntToParquet(parquetCol, dolphinCol);
            break;
        }
        case DT_SHORT:
        case DT_INT:
        case DT_DATE:
        case DT_TIME:
        {
            parquet::Int32Writer *parquetCol = dynamic_cast<parquet::Int32Writer*>(columnWriter);
            writeIntToParquet(parquetCol, dolphinCol);
            break;
        }
        case DT_LONG:
        case DT_TIMESTAMP:
        case DT_NANOTIME:
        case DT_NANOTIMESTAMP:
        {
            parquet::Int64Writer *parquetCol = dynamic_cast<parquet::Int64Writer*>(columnWriter);
            writeLongToParquet(parquetCol, dolphinCol);
            break;
        }
        case DT_MONTH:
        {
            parquet::Int32Writer *parquetCol = dynamic_cast<parquet::Int32Writer*>(columnWriter);
            auto transform = [](int v){
                using months = std::chrono::duration<int, std::ratio<2629746> >;
                using days = std::chrono::duration<int, std::ratio<86400> >;
                v = std::chrono::duration_cast<days>(months(v)).count();
                v = Util::getMonthStart(v - 719514);
                return v;
            };
            writeIntToParquet(parquetCol, dolphinCol, transform);
            break;
        }
        case DT_MINUTE:
        {
            parquet::Int32Writer *parquetCol = dynamic_cast<parquet::Int32Writer*>(columnWriter);
            auto transform = [](int v){
                return v * 60 * 1000;
            };
            writeIntToParquet(parquetCol, dolphinCol, transform);
            break;
        }
        case DT_SECOND:
        {
            parquet::Int32Writer *parquetCol = dynamic_cast<parquet::Int32Writer*>(columnWriter);
            auto transform = [](int v){
                return v * 1000;
            };
            writeIntToParquet(parquetCol, dolphinCol, transform);
            break;
        }
        case DT_DATETIME:
        {
            parquet::Int64Writer *parquetCol = dynamic_cast<parquet::Int64Writer*>(columnWriter);
            auto transform = [](long long v){
                return v * 1000;
            };
            writeLongToParquet(parquetCol, dolphinCol, transform);
            break;
        }
        case DT_FLOAT:
        {
            parquet::FloatWriter *parquetCol = dynamic_cast<parquet::FloatWriter*>(columnWriter);
            writeFloatToParquet(parquetCol, dolphinCol);
            break;
        }
        case DT_DOUBLE:
        {
            parquet::DoubleWriter *parquetCol = dynamic_cast<parquet::DoubleWriter*>(columnWriter);
            writeDoubleToParquet(parquetCol, dolphinCol);
            break;
        }
        case DT_SYMBOL:
        case DT_STRING:
        {
            parquet::ByteArrayWriter *parquetCol = dynamic_cast<parquet::ByteArrayWriter*>(columnWriter);
            writeStringToParquet(parquetCol, dolphinCol);
            break;
        }
        default:
            throw RuntimeException("unsupported type.");
        }
    }

    rowGroupWriter->Close();
    writer->Close();
    outfile->Close();
    return new Void();
}

ConstantSP saveParquetHdfs(ConstantSP &table)
{
    std::shared_ptr<arrow::io::BufferOutputStream> outfile;
    PARQUET_ASSIGN_OR_THROW(
        outfile,
        arrow::io::BufferOutputStream::Create()
    );
    std::shared_ptr<parquet::schema::GroupNode> schema = getParquetSchemaFromDolphindb(table);
    parquet::WriterProperties::Builder builder;
    std::unique_ptr<parquet::ParquetFileWriter> writer = parquet::ParquetFileWriter::Open(
        outfile,
        schema,
        builder.build()
    );

    int colNum = table->columns();
    parquet::RowGroupWriter *rowGroupWriter = writer->AppendRowGroup();
    for(int i = 0; i < colNum; ++i)
    {
        parquet::ColumnWriter *columnWriter = rowGroupWriter->NextColumn();
        VectorSP dolphinCol = table->getColumn(i);
        if(dolphinCol->size() == 0){
            continue;
        }
        ConstantSP front = dolphinCol->get(0);
        switch(front->getType())
        {
        case DT_BOOL:
        {
            parquet::BoolWriter *parquetCol = dynamic_cast<parquet::BoolWriter*>(columnWriter);
            writeBoolToParquet(parquetCol, dolphinCol);
            break;
        }
        case DT_CHAR:
        {
            parquet::Int32Writer *parquetCol = dynamic_cast<parquet::Int32Writer*>(columnWriter);
            writeIntToParquet(parquetCol, dolphinCol);
            break;
        }
        case DT_SHORT:
        case DT_INT:
        case DT_DATE:
        case DT_TIME:
        {
            parquet::Int32Writer *parquetCol = dynamic_cast<parquet::Int32Writer*>(columnWriter);
            writeIntToParquet(parquetCol, dolphinCol);
            break;
        }
        case DT_LONG:
        case DT_TIMESTAMP:
        case DT_NANOTIME:
        case DT_NANOTIMESTAMP:
        {
            parquet::Int64Writer *parquetCol = dynamic_cast<parquet::Int64Writer*>(columnWriter);
            writeLongToParquet(parquetCol, dolphinCol);
            break;
        }
        case DT_MONTH:
        {
            parquet::Int32Writer *parquetCol = dynamic_cast<parquet::Int32Writer*>(columnWriter);
            auto transform = [](int v){
                using months = std::chrono::duration<int, std::ratio<2629746> >;
                using days = std::chrono::duration<int, std::ratio<86400> >;
                v = std::chrono::duration_cast<days>(months(v)).count();
                v = Util::getMonthStart(v - 719514);
                return v;
            };
            writeIntToParquet(parquetCol, dolphinCol, transform);
            break;
        }
        case DT_MINUTE:
        {
            parquet::Int32Writer *parquetCol = dynamic_cast<parquet::Int32Writer*>(columnWriter);
            auto transform = [](int v){
                return v * 60 * 1000;
            };
            writeIntToParquet(parquetCol, dolphinCol, transform);
            break;
        }
        case DT_SECOND:
        {
            parquet::Int32Writer *parquetCol = dynamic_cast<parquet::Int32Writer*>(columnWriter);
            auto transform = [](int v){
                return v * 1000;
            };
            writeIntToParquet(parquetCol, dolphinCol, transform);
            break;
        }
        case DT_DATETIME:
        {
            parquet::Int64Writer *parquetCol = dynamic_cast<parquet::Int64Writer*>(columnWriter);
            auto transform = [](long long v){
                return v * 1000;
            };
            writeLongToParquet(parquetCol, dolphinCol, transform);
            break;
        }
        case DT_FLOAT:
        {
            parquet::FloatWriter *parquetCol = dynamic_cast<parquet::FloatWriter*>(columnWriter);
            writeFloatToParquet(parquetCol, dolphinCol);
            break;
        }
        case DT_DOUBLE:
        {
            parquet::DoubleWriter *parquetCol = dynamic_cast<parquet::DoubleWriter*>(columnWriter);
            writeDoubleToParquet(parquetCol, dolphinCol);
            break;
        }
        case DT_SYMBOL:
        case DT_STRING:
        {
            parquet::ByteArrayWriter *parquetCol = dynamic_cast<parquet::ByteArrayWriter*>(columnWriter);
            writeStringToParquet(parquetCol, dolphinCol);
            break;
        }
        default:
            throw RuntimeException("unsupported type.");
        }
    }

    rowGroupWriter->Close();
    writer->Close();
    std::shared_ptr<arrow::Buffer> fileBuf;
    PARQUET_ASSIGN_OR_THROW(
        fileBuf,
        outfile->Finish()
    );
    uint64_t *len = new uint64_t;
    *len = fileBuf->size();
    char *buffer = new char[*len];
    memcpy((void *)buffer, (void *)fileBuf->data(), *len);
    void *vBuf = (void *)buffer;
    VectorSP res = Util::createVector(DT_LONG, 2, 2);
    res->setLong(0, (long long)vBuf);
    res->setLong(1, (long long)len);
    return res;
}

std::shared_ptr<parquet::schema::GroupNode> getParquetSchemaFromDolphindb(const TableSP &table){
    int colNum = table->columns();
    parquet::schema::NodeVector types;
    for(int i = 0; i < colNum; ++i)
    {
        switch(table->getColumnType(i))
        {
        case DT_BOOL:
            types.push_back(
                parquet::schema::PrimitiveNode::Make(
                    table->getColumnName(i),
                    parquet::Repetition::type::OPTIONAL,
                    nullptr,
                    parquet::Type::type::BOOLEAN
                )
            );
            break;
        case DT_CHAR:
            types.push_back(
                parquet::schema::PrimitiveNode::Make(
                    table->getColumnName(i),
                    parquet::Repetition::type::OPTIONAL,
                    parquet::LogicalType::Int(8, true),
                    parquet::Type::type::INT32
                )
            );
            break;
        case DT_SHORT:
            types.push_back(
                parquet::schema::PrimitiveNode::Make(
                    table->getColumnName(i),
                    parquet::Repetition::type::OPTIONAL,
                    parquet::LogicalType::Int(16, true),
                    parquet::Type::type::INT32
                )
            );
            break;
        case DT_INT:
            types.push_back(
                parquet::schema::PrimitiveNode::Make(
                    table->getColumnName(i),
                    parquet::Repetition::type::OPTIONAL,
                    parquet::LogicalType::Int(32, true),
                    parquet::Type::type::INT32
                )
            );
            break;
        case DT_LONG:
            types.push_back(
                parquet::schema::PrimitiveNode::Make(
                    table->getColumnName(i),
                    parquet::Repetition::type::OPTIONAL,
                    parquet::LogicalType::Int(64, true),
                    parquet::Type::type::INT64
                )
            );
            break;
        case DT_MONTH:
        case DT_DATE:
            types.push_back(
                parquet::schema::PrimitiveNode::Make(
                    table->getColumnName(i),
                    parquet::Repetition::type::OPTIONAL,
                    parquet::LogicalType::Date(),
                    parquet::Type::type::INT32
                )
            );
            break;
        case DT_TIME:
        case DT_MINUTE:
        case DT_SECOND:
            types.push_back(
                parquet::schema::PrimitiveNode::Make(
                    table->getColumnName(i),
                    parquet::Repetition::type::OPTIONAL,
                    parquet::LogicalType::Time(false, parquet::LogicalType::TimeUnit::unit::MILLIS),
                    parquet::Type::type::INT32
                )
            );
            break;
        case DT_DATETIME:
        case DT_TIMESTAMP:
            types.push_back(
                parquet::schema::PrimitiveNode::Make(
                    table->getColumnName(i),
                    parquet::Repetition::type::OPTIONAL,
                    parquet::LogicalType::Timestamp(false, parquet::LogicalType::TimeUnit::unit::MILLIS),
                    parquet::Type::type::INT64
                )
            );
            break;
        case DT_NANOTIME:
            types.push_back(
                parquet::schema::PrimitiveNode::Make(
                    table->getColumnName(i),
                    parquet::Repetition::type::OPTIONAL,
                    parquet::LogicalType::Time(false, parquet::LogicalType::TimeUnit::unit::NANOS),
                    parquet::Type::type::INT64
                )
            );
            break;
        case DT_NANOTIMESTAMP:
            types.push_back(
                parquet::schema::PrimitiveNode::Make(
                    table->getColumnName(i),
                    parquet::Repetition::type::OPTIONAL,
                    parquet::LogicalType::Timestamp(false, parquet::LogicalType::TimeUnit::unit::NANOS),
                    parquet::Type::type::INT64
                )
            );
            break;
        case DT_FLOAT:
            types.push_back(
                parquet::schema::PrimitiveNode::Make(
                    table->getColumnName(i),
                    parquet::Repetition::type::OPTIONAL,
                    nullptr,
                    parquet::Type::type::FLOAT
                )
            );
            break;
        case DT_DOUBLE:
            types.push_back(
                parquet::schema::PrimitiveNode::Make(
                    table->getColumnName(i),
                    parquet::Repetition::type::OPTIONAL,
                    nullptr,
                    parquet::Type::type::DOUBLE
                )
            );
            break;
        case DT_SYMBOL:
        case DT_STRING:
            types.push_back(
                parquet::schema::PrimitiveNode::Make(
                    table->getColumnName(i),
                    parquet::Repetition::type::OPTIONAL,
                    parquet::LogicalType::String(),
                    parquet::Type::type::BYTE_ARRAY
                )
            );
            break;
        default:
            throw RuntimeException("unsupported type.");
        }
    }
    return std::dynamic_pointer_cast<parquet::schema::GroupNode>(
        parquet::schema::GroupNode::Make(
            table->getName(),
            parquet::Repetition::type::REQUIRED,
            types,
            nullptr
        )
    );
}

void writeBoolToParquet(parquet::BoolWriter *parquetCol, const VectorSP &dolphinCol){
    for(int j = 0; j < dolphinCol->size(); ++j){
        ConstantSP value = dolphinCol->get(j);
        if(value->isNull()){
            parquetCol->WriteBatch(1, &ZERO, &ZERO, nullptr);
        }
        else{
            bool v = value->getBool();
            parquetCol->WriteBatch(1, &ONE, &ZERO, &v);
        }
    }
}

void writeIntToParquet(parquet::Int32Writer *parquetCol, const VectorSP &dolphinCol, int (*transform)(int)){
    vector<int> buffer;
    for(int j = 0; j < dolphinCol->size(); ++j){
        ConstantSP value = dolphinCol->get(j);
        if(value->isNull()){
            if(!buffer.empty()){
                vector<int16_t> ONEs(buffer.size(), ONE);
                vector<int16_t> ZEROs(buffer.size(), ZERO);
                parquetCol->WriteBatch(buffer.size(), ONEs.data(), ZEROs.data(), buffer.data());
                buffer.clear();
            }
            parquetCol->WriteBatch(1, &ZERO, &ZERO, nullptr);
        }
        else{
            buffer.push_back(transform(value->getInt()));
        }
    }
    if(!buffer.empty()){
        vector<int16_t> ONEs(buffer.size(), ONE);
        vector<int16_t> ZEROs(buffer.size(), ZERO);
        parquetCol->WriteBatch(buffer.size(), ONEs.data(), ZEROs.data(), buffer.data());
    }
}

void writeLongToParquet(parquet::Int64Writer *parquetCol, const VectorSP &dolphinCol, long long (*transform)(long long)){
    vector<int64_t> buffer;
    for(int j = 0; j < dolphinCol->size(); ++j){
        ConstantSP value = dolphinCol->get(j);
        if(value->isNull()){
            if(!buffer.empty()){
                vector<int16_t> ONEs(buffer.size(), ONE);
                vector<int16_t> ZEROs(buffer.size(), ZERO);
                parquetCol->WriteBatch(buffer.size(), ONEs.data(), ZEROs.data(), buffer.data());
                buffer.clear();
            }
            parquetCol->WriteBatch(1, &ZERO, &ZERO, nullptr);
        }
        else{
            buffer.push_back(transform(value->getLong()));
        }
    }
    if(!buffer.empty()){
        vector<int16_t> ONEs(buffer.size(), ONE);
        vector<int16_t> ZEROs(buffer.size(), ZERO);
        parquetCol->WriteBatch(buffer.size(), ONEs.data(), ZEROs.data(), buffer.data());
    }
}

void writeFloatToParquet(parquet::FloatWriter *parquetCol, const VectorSP &dolphinCol){
    vector<float> buffer;
    for(int j = 0; j < dolphinCol->size(); ++j){
        ConstantSP value = dolphinCol->get(j);
        if(value->isNull()){
            if(!buffer.empty()){
                vector<int16_t> ONEs(buffer.size(), ONE);
                vector<int16_t> ZEROs(buffer.size(), ZERO);
                parquetCol->WriteBatch(buffer.size(), ONEs.data(), ZEROs.data(), buffer.data());
                buffer.clear();
            }
            parquetCol->WriteBatch(1, &ZERO, &ZERO, nullptr);
        }
        else{
            buffer.push_back(value->getFloat());
        }
    }
    if(!buffer.empty()){
        vector<int16_t> ONEs(buffer.size(), ONE);
        vector<int16_t> ZEROs(buffer.size(), ZERO);
        parquetCol->WriteBatch(buffer.size(), ONEs.data(), ZEROs.data(), buffer.data());
    }
}

void writeDoubleToParquet(parquet::DoubleWriter *parquetCol, const VectorSP &dolphinCol){
    vector<double> buffer;
    for(int j = 0; j < dolphinCol->size(); ++j){
        ConstantSP value = dolphinCol->get(j);
        if(value->isNull()){
            if(!buffer.empty()){
                vector<int16_t> ONEs(buffer.size(), ONE);
                vector<int16_t> ZEROs(buffer.size(), ZERO);
                parquetCol->WriteBatch(buffer.size(), ONEs.data(), ZEROs.data(), buffer.data());
                buffer.clear();
            }
            parquetCol->WriteBatch(1, &ZERO, &ZERO, nullptr);
        }
        else{
            buffer.push_back(value->getDouble());
        }
    }
    if(!buffer.empty()){
        vector<int16_t> ONEs(buffer.size(), ONE);
        vector<int16_t> ZEROs(buffer.size(), ZERO);
        parquetCol->WriteBatch(buffer.size(), ONEs.data(), ZEROs.data(), buffer.data());
    }
}

void writeStringToParquet(parquet::ByteArrayWriter *parquetCol, const VectorSP &dolphinCol){
    for(int j = 0; j < dolphinCol->size(); ++j){
        ConstantSP value = dolphinCol->get(j);
        if(value->isNull()){
            parquetCol->WriteBatch(1, &ZERO, &ZERO, nullptr);
        }else{
            string v = value->getString();
            parquet::ByteArray str;
            str.ptr = reinterpret_cast<const uint8_t*>(v.data());
            str.len = static_cast<uint32_t>(v.size());
            parquetCol->WriteBatch(1, &ONE, &ZERO, &str);
        }
    }
}

} // namespace ParquetPluginImp