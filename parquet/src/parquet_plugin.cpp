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
    case parquet::LogicalType::Type::INT:
        switch (physical_t)
        {
        case parquet::Type::type::INT32:
            return "INT";
        case parquet::Type::type::INT64:
            return "Long";
        default:
            throw RuntimeException("unsupported data type");
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
    case parquet::ConvertedType::type::INT_16:
    case parquet::ConvertedType::type::INT_32:
    case parquet::ConvertedType::type::UINT_16:
    case parquet::ConvertedType::type::UINT_8:
        return "INT";
    case parquet::ConvertedType::type::TIMESTAMP_MICROS:
        return "NANOTIMESTAMP";
    case parquet::ConvertedType::type::TIMESTAMP_MILLIS:
        return "TIMESTAMP";
    case parquet::ConvertedType::type::DECIMAL:
        return "DOUBLE";
    case parquet::ConvertedType::type::UINT_32:
    case parquet::ConvertedType::type::INT_64:
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
        dolpindb_v[i] = Util::createVector(tb->getColumnType(i), 0);
    }
}
TableSP appendColumnVecToTable(TableSP tb, vector<VectorSP> &colVec)
{
    if (tb->isNull())
        return tb;

    string errMsg;
    INDEX insertedRows = 0;
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
bool convertToDTdate(vector<int> &intValue, parquetTime times_t, vector<long long> &longValue)
{
    tm gmtBuf;
    int buf_size = std::max(intValue.size(), longValue.size());
    switch (times_t)
    {
    case parquetTime::Date:
        return true;
    case parquetTime::TimestampMicros:
        for (int i = 0; i < buf_size; i++)
        {
            time_t ts = longValue[i] / 1000000;
            tm *gmt = gmtime_r(&ts, &gmtBuf);
            intValue[i] = (gmt == nullptr) ? 0 : Util::countDays(gmt->tm_year + 1900, gmt->tm_mon + 1, gmt->tm_mday);
        }
        return true;
    case parquetTime::TimestampMillis:
        for (int i = 0; i < buf_size; i++)
        {
            time_t ts = longValue[i] / 1000;
            tm *gmt = gmtime_r(&ts, &gmtBuf);
            intValue[i] = (gmt == nullptr) ? 0 : Util::countDays(gmt->tm_year + 1900, gmt->tm_mon + 1, gmt->tm_mday);
        }
        return true;
    case parquetTime::TimestampNanos:
        for (int i = 0; i < buf_size; i++)
        {
            time_t ts = longValue[i] / 1000000000;
            tm *gmt = gmtime_r(&ts, &gmtBuf);
            intValue[i] = (gmt == nullptr) ? 0 : Util::countDays(gmt->tm_year + 1900, gmt->tm_mon + 1, gmt->tm_mday);
        }
        return true;
    default:
        return false;
    }
}
bool convertToDTminute(vector<int> &intValue, parquetTime times_t, vector<long long> &longValue)
{
    tm gmtBuf;
    int buf_size = std::max(intValue.size(), longValue.size());
    switch (times_t)
    {
    case parquetTime::Date:
        for (int i = 0; i < buf_size; i++)
        {
            intValue[i] = 0;
        }
        return true;
    case parquetTime::TimestampMicros:
        for (int i = 0; i < buf_size; i++)
        {
            time_t ts = longValue[i] / 1000000;
            tm *gmt = gmtime_r(&ts, &gmtBuf);
            intValue[i] = (gmt == nullptr) ? 0 : gmt->tm_hour * 60 + gmt->tm_min;
        }
        return true;
    case parquetTime::TimestampMillis:
        for (int i = 0; i < buf_size; i++)
        {
            time_t ts = longValue[i] / 1000;
            tm *gmt = gmtime_r(&ts, &gmtBuf);
            intValue[i] = (gmt == nullptr) ? 0 : gmt->tm_hour * 60 + gmt->tm_min;
        }
        return true;
    case parquetTime::TimestampNanos:
        for (int i = 0; i < buf_size; i++)
        {
            time_t ts = longValue[i] / 1000000000;
            tm *gmt = gmtime_r(&ts, &gmtBuf);
            intValue[i] = (gmt == nullptr) ? 0 : gmt->tm_hour * 60 + gmt->tm_min;
        }
        return true;
    case parquetTime::TimeMicros:
        for (int i = 0; i < buf_size; i++)
            intValue[i] = longValue[i] / 1000000 / 60;
        return true;
    case parquetTime::TimeMillis:
        for (int i = 0; i < buf_size; i++)
            intValue[i] = intValue[i] / 1000 / 60;
        return true;
    case parquetTime::TimeNanos:
        for (int i = 0; i < buf_size; i++)
            intValue[i] = longValue[i] / 1000000000 / 60;
        return true;
    default:
        return false;
    }
}
bool convertToDTtime(vector<int> &intValue, parquetTime times_t, vector<long long> &longValue)
{
    tm gmtBuf;
    int buf_size = std::max(intValue.size(), longValue.size());
    switch (times_t)
    {
    case parquetTime::Date:
        for (int i = 0; i < buf_size; i++)
        {
            intValue[i] = 0;
        }
        return true;
    case parquetTime::TimestampMicros:
        for (int i = 0; i < buf_size; i++)
        {
            time_t ts = longValue[i] / 1000000;
            tm *gmt = gmtime_r(&ts, &gmtBuf);
            intValue[i] = (gmt == nullptr) ? 0 : ((gmt->tm_hour * 60 + gmt->tm_min) * 60 + gmt->tm_sec) * 1000 + ((longValue[i]) / 1000) % 1000;
        }
        return true;
    case parquetTime::TimestampMillis:
        for (int i = 0; i < buf_size; i++)
        {
            time_t ts = longValue[i] / 1000;
            tm *gmt = gmtime_r(&ts, &gmtBuf);
            intValue[i] = (gmt == nullptr) ? 0 : ((gmt->tm_hour * 60 + gmt->tm_min) * 60 + gmt->tm_sec) * 1000 + ((longValue[i]) / 1000) % 1000;
        }
        return true;
    case parquetTime::TimestampNanos:
        for (int i = 0; i < buf_size; i++)
        {
            time_t ts = longValue[i] / 1000000000;
            tm *gmt = gmtime_r(&ts, &gmtBuf);
            intValue[i] = (gmt == nullptr) ? 0 : ((gmt->tm_hour * 60 + gmt->tm_min) * 60 + gmt->tm_sec) * 1000 + ((longValue[i]) / 1000) % 1000;
        }
        return true;
    case parquetTime::TimeMicros:
        for (int i = 0; i < buf_size; i++)
            intValue[i] = longValue[i] / 1000;
        return true;
    case parquetTime::TimeMillis:
        return true;
    case parquetTime::TimeNanos:
        for (int i = 0; i < buf_size; i++)
            intValue[i] = longValue[i] / 1000000;
        return true;
    default:
        return false;
    }
}
bool convertToDTnanotime(vector<int> &intValue, parquetTime times_t, vector<long long> &longValue)
{
    tm gmtBuf;
    int buf_size = std::max(intValue.size(), longValue.size());
    switch (times_t)
    {
    case parquetTime::Date:
        for (int i = 0; i < buf_size; i++)
        {
            intValue[i] = 0;
        }
        return true;
    case parquetTime::TimestampMicros:
        for (int i = 0; i < buf_size; i++)
        {
            time_t ts = longValue[i] / 1000000;
            tm *gmt = gmtime_r(&ts, &gmtBuf);
            longValue[i] = (gmt == nullptr) ? 0 : ((gmt->tm_hour * 60 + gmt->tm_min) * 60 + gmt->tm_sec) * 1000000000 + longValue[i] % 1000000 * 1000;
        }
        return true;
    case parquetTime::TimestampMillis:
        for (int i = 0; i < buf_size; i++)
        {
            time_t ts = longValue[i] / 1000;
            tm *gmt = gmtime_r(&ts, &gmtBuf);
            longValue[i] = (gmt == nullptr) ? 0 : ((gmt->tm_hour * 60 + gmt->tm_min) * 60 + gmt->tm_sec) * 1000000000 + longValue[i] % 1000 * 1000000;
        }
        return true;
    case parquetTime::TimestampNanos:
        for (int i = 0; i < buf_size; i++)
        {
            time_t ts = longValue[i] / 1000000000;
            tm *gmt = gmtime_r(&ts, &gmtBuf);
            longValue[i] = (gmt == nullptr) ? 0 : ((gmt->tm_hour * 60 + gmt->tm_min) * 60 + gmt->tm_sec) * 1000000000 + longValue[i] % 1000000000;
        }
        return true;
    case parquetTime::TimeMicros:
        for (int i = 0; i < buf_size; i++)
            intValue[i] = longValue[i] * 1000;
        return true;
    case parquetTime::TimeMillis:
        for (int i = 0; i < buf_size; i++)
            intValue[i] = longValue[i] * 1000000;
        return true;
    case parquetTime::TimeNanos:
        return true;
    default:
        return false;
    }
}
bool convertToDTsecond(vector<int> &intValue, parquetTime times_t, vector<long long> &longValue)
{
    tm gmtBuf;
    int buf_size = std::max(intValue.size(), longValue.size());
    switch (times_t)
    {
    case parquetTime::Date:
        for (int i = 0; i < buf_size; i++)
        {
            intValue[i] = 0;
        }
        return true;
    case parquetTime::TimestampMicros:
        for (int i = 0; i < buf_size; i++)
        {
            time_t ts = longValue[i] / 1000000;
            tm *gmt = gmtime_r(&ts, &gmtBuf);
            intValue[i] = (gmt == nullptr) ? 0 : (gmt->tm_hour * 60 + gmt->tm_min) * 60 + gmt->tm_sec;
        }
        return true;
    case parquetTime::TimestampMillis:
        for (int i = 0; i < buf_size; i++)
        {
            time_t ts = longValue[i] / 1000;
            tm *gmt = gmtime_r(&ts, &gmtBuf);
            intValue[i] = (gmt == nullptr) ? 0 : (gmt->tm_hour * 60 + gmt->tm_min) * 60 + gmt->tm_sec;
        }
        return true;
    case parquetTime::TimestampNanos:
        for (int i = 0; i < buf_size; i++)
        {
            time_t ts = longValue[i] / 1000000000;
            tm *gmt = gmtime_r(&ts, &gmtBuf);
            intValue[i] = (gmt == nullptr) ? 0 : (gmt->tm_hour * 60 + gmt->tm_min) * 60 + gmt->tm_sec;
        }
        return true;
    case parquetTime::TimeMicros:
        for (int i = 0; i < buf_size; i++)
            intValue[i] = longValue[i] / 1000000;
        return true;
    case parquetTime::TimeMillis:
        for (int i = 0; i < buf_size; i++)
            intValue[i] = longValue[i] / 1000;
        return true;
    case parquetTime::TimeNanos:
        for (int i = 0; i < buf_size; i++)
            intValue[i] = longValue[i] / 1000000000;
        return true;
    default:
        return false;
    }
}

bool convertToDTmonth(vector<int> &intValue, parquetTime times_t, vector<long long> &longValue)
{
    tm gmtBuf;
    int buf_size = std::max(intValue.size(), longValue.size());
    switch (times_t)
    {
    case parquetTime::Date:
        for (int i = 0; i < buf_size; i++)
        {
            time_t ts = intValue[i] * 24 * 60 * 60;
            tm *gmt = gmtime_r(&ts, &gmtBuf);
            intValue[i] = (gmt == nullptr) ? 0 : (gmt->tm_year + 1900) * 12 + gmt->tm_mon;
        }
        return true;
    case parquetTime::TimestampMicros:
        for (int i = 0; i < buf_size; i++)
        {
            time_t ts = longValue[i] / 1000000;
            tm *gmt = gmtime_r(&ts, &gmtBuf);
            intValue[i] = (gmt == nullptr) ? 0 : (gmt->tm_year + 1900) * 12 + gmt->tm_mon;
        }
        return true;
    case parquetTime::TimestampMillis:
        for (int i = 0; i < buf_size; i++)
        {
            time_t ts = longValue[i] / 1000;
            tm *gmt = gmtime_r(&ts, &gmtBuf);
            intValue[i] = (gmt == nullptr) ? 0 : (gmt->tm_year + 1900) * 12 + gmt->tm_mon;
        }
        return true;
    case parquetTime::TimestampNanos:
        for (int i = 0; i < buf_size; i++)
        {
            time_t ts = longValue[i] / 1000000000;
            tm *gmt = gmtime_r(&ts, &gmtBuf);
            intValue[i] = (gmt == nullptr) ? 0 : (gmt->tm_year + 1900) * 12 + gmt->tm_mon;
        }
        return true;
    default:
        return false;
    }
}

bool convertToDTdatetime(vector<int> &intValue, parquetTime times_t, vector<long long> &longValue)
{
    int buf_size = std::max(intValue.size(), longValue.size());
    switch (times_t)
    {
    case parquetTime::Date:
        for (int i = 0; i < buf_size; i++)
            intValue[i] = intValue[i] * 24 * 60 * 60;
        return true;
    case parquetTime::TimestampMicros:
        for (int i = 0; i < buf_size; i++)
        {
            intValue[i] = longValue[i] / 1000000;
        }
        return true;
    case parquetTime::TimestampMillis:
        for (int i = 0; i < buf_size; i++)
        {
            intValue[i] = longValue[i] / 1000;
        }
        return true;
    case parquetTime::TimestampNanos:
        for (int i = 0; i < buf_size; i++)
        {
            intValue[i] = longValue[i] / 1000000000;
        }
        return true;
    default:
        return false;
    }
}

bool convertToDTtimestamp(vector<int> &intValue, parquetTime times_t, vector<long long> &longValue)
{
    int buf_size = std::max(intValue.size(), longValue.size());
    switch (times_t)
    {
    case parquetTime::Date:
        for (int i = 0; i < buf_size; i++)
            longValue[i] = intValue[i] * 24 * 60 * 60 * 1000;
        return true;
    case parquetTime::TimestampMicros:
        for (int i = 0; i < buf_size; i++)
        {
            longValue[i] = longValue[i] / 1000;
        }
        return true;
    case parquetTime::TimestampMillis:
        return true;
    case parquetTime::TimestampNanos:
        for (int i = 0; i < buf_size; i++)
        {
            longValue[i] = longValue[i] / 1000000;
        }
        return true;
    default:
        return false;
    }
}

bool convertToDTnanotimestamp(vector<int> &intValue, parquetTime times_t, vector<long long> &longValue)
{
    int buf_size = std::max(intValue.size(), longValue.size());
    switch (times_t)
    {
    case parquetTime::Date:
        for (int i = 0; i < buf_size; i++)
            longValue[i] = intValue[i] * 24 * 60 * 60 * 1000 * 1000000;
        return true;
    case parquetTime::TimestampMicros:
        for (int i = 0; i < buf_size; i++)
        {
            longValue[i] = longValue[i] * 1000;
        }
        return true;
    case parquetTime::TimestampMillis:
        for (int i = 0; i < buf_size; i++)
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

bool convertParquetToDolphindbChar(int col_idx, std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, vector<char> &buffer)
{
    switch (col_descr->physical_type())
    {
    case parquet::Type::BOOLEAN:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:BOOLEAN" + "->" + Util::getDataTypeString(DT_CHAR));
    case parquet::Type::INT32:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:INT32" + "->" + Util::getDataTypeString(DT_CHAR));
    case parquet::Type::INT64:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:INT64" + "->" + Util::getDataTypeString(DT_CHAR));
    case parquet::Type::INT96:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:INT96" + "->" + Util::getDataTypeString(DT_CHAR));
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
        while (flba_reader->HasNext())
        {
            vector<parquet::FixedLenByteArray> value;
            int64_t values_read = 0;
            vector<short> def_level;
            vector<short> rep_level;
            vector<char> char_value;
            int64_t rows_read = flba_reader->ReadBatch(100, def_level.data(), rep_level.data(), value.data(), &values_read);
            if (rows_read == values_read && rows_read != 0)
                memcpy(char_value.data(), value.data(), values_read);
            else
            {
                for (size_t le = 0; le < def_level.size(); le++)
                {
                    if (def_level[le] >= col_descr->max_definition_level())
                    {
                        char_value.push_back(static_cast<char>(*value[le].ptr));
                    }
                    else
                    {
                        char_value.push_back(CHAR_MIN);
                    }
                }
            }
            buffer.insert(buffer.end(), char_value.begin(), char_value.end());
        }
        break;
    }
    case parquet::Type::BYTE_ARRAY:
    {
        parquet::ByteArrayReader *ba_reader = static_cast<parquet::ByteArrayReader *>(column_reader.get());

        while (ba_reader->HasNext())
        {
            vector<parquet::ByteArray> value;
            int64_t values_read;
            vector<short> def_level;
            vector<short> rep_level;
            vector<char> char_value;
            ba_reader->ReadBatch(100, def_level.data(), rep_level.data(), value.data(), &values_read);
            for (size_t le = 0; le < def_level.size(); le++)
            {
                if (value[le].len > 1)
                    throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:ByteArray(length>1)" + "->" + Util::getDataTypeString(DT_CHAR));
                if (def_level[le] >= col_descr->max_definition_level())
                {
                    char_value.push_back(static_cast<char>(*value[le].ptr));
                }
                else
                {
                    char_value.push_back(CHAR_MIN);
                }
            }
            buffer.insert(buffer.end(), char_value.begin(), char_value.end());
        }
        break;
    }
    default:
        throw RuntimeException("unsupported data type");
    }
    return true;
}

bool convertParquetToDolphindbBool(int col_idx, std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, vector<char> &buffer)
{
    switch (col_descr->physical_type())
    {
    case parquet::Type::BOOLEAN:
    {
        parquet::BoolReader *bool_reader = static_cast<parquet::BoolReader *>(column_reader.get());
        while (bool_reader->HasNext())
        {
            bool value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = bool_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                buffer.push_back(static_cast<char>(value));
            }
            else if (rows_read != 0)
            {
                buffer.push_back(CHAR_MIN);
            }
        }
        break;
    }
    case parquet::Type::INT32:
    {
        parquet::Int32Reader *int32_reader = static_cast<parquet::Int32Reader *>(column_reader.get());
        // Read all the rows in the column
        while (int32_reader->HasNext())
        {
            int32_t value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = int32_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                buffer.push_back(static_cast<char>(value != 0));
            }
            else if (rows_read != 0)
            {
                buffer.push_back(CHAR_MIN);
            }
        }
        break;
    }
    case parquet::Type::INT64:
    {
        parquet::Int64Reader *int64_reader = static_cast<parquet::Int64Reader *>(column_reader.get());
        while (int64_reader->HasNext())
        {
            int64_t value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = int64_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                buffer.push_back(static_cast<char>(value != 0));
            }
            else if (rows_read != 0)
            {
                buffer.push_back(CHAR_MIN);
            }
        }
        break;
    }
    case parquet::Type::INT96:
    {
        parquet::Int96Reader *int96_reader = static_cast<parquet::Int96Reader *>(column_reader.get());
        while (int96_reader->HasNext())
        {
            parquet::Int96 value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = int96_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                buffer.push_back(static_cast<char>(value.value[0] != 0 && value.value[1] && value.value[2]));
            }
            else if (rows_read != 0)
            {
                buffer.push_back(CHAR_MIN);
            }
        }
        break;
    }
    case parquet::Type::DOUBLE:
    {
        parquet::DoubleReader *double_reader = static_cast<parquet::DoubleReader *>(column_reader.get());
        while (double_reader->HasNext())
        {
            double value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = double_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                buffer.push_back(static_cast<char>(value != 0));
            }
            else if (rows_read != 0)
            {
                buffer.push_back(CHAR_MIN);
            }
        }
        break;
    }
    case parquet::Type::FLOAT:
    {
        parquet::FloatReader *float_reader = static_cast<parquet::FloatReader *>(column_reader.get());
        while (float_reader->HasNext())
        {
            float value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = float_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                buffer.push_back(static_cast<char>(value != 0));
            }
            else if (rows_read != 0)
            {
                buffer.push_back(CHAR_MIN);
            }
        }
        break;
    }
    case parquet::Type::FIXED_LEN_BYTE_ARRAY:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:FIXED_LEN_BYTE_ARRAY" + "->" + Util::getDataTypeString(DT_BOOL));
    case parquet::Type::BYTE_ARRAY:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:BYTE_ARRAY" + "->" + Util::getDataTypeString(DT_BOOL));

    default:
        throw RuntimeException("unsupported data type");
    }
    return true;
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

bool convertParquetToDolphindbInt(int col_idx, std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, vector<int> &buffer, DATA_TYPE times_t)
{
    switch (col_descr->physical_type())
    {
    case parquet::Type::BOOLEAN:
    {
        if (times_t != DT_INT)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:BOOLEAN" + "->" + Util::getDataTypeString(times_t));
        parquet::BoolReader *bool_reader = static_cast<parquet::BoolReader *>(column_reader.get());
        while (bool_reader->HasNext())
        {
            bool value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = bool_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                buffer.push_back(static_cast<int>(value));
            }
            else if (rows_read != 0)
            {
                buffer.push_back(INT32_MIN);
            }
        }
        break;
    }
    case parquet::Type::INT32:
    {
        parquet::Int32Reader *int32_reader = static_cast<parquet::Int32Reader *>(column_reader.get());
        // Read all the rows in the column

        while (int32_reader->HasNext())
        {
            int32_t value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = int32_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                buffer.push_back(value);
            }
            else if (rows_read != 0)
            {
                buffer.push_back(INT32_MIN);
            }
        }
        if(col_descr->logical_type()->is_decimal() || col_descr->converted_type()==parquet::ConvertedType::DECIMAL)
        {
            int scale=std::pow(10, col_descr->type_scale());
            for(auto &x:buffer)
                if(x!=INT32_MIN)
                    x=x/scale;
            break;
        }
        parquetTime parquetT = convertParquetTimes(col_descr->logical_type(), col_descr->converted_type());
        if (parquetT == parquetTime::None)
            break;
        vector<long long> longValue;
        switch (times_t)
        {
        case DT_DATE:
            convertToDTdate(buffer, parquetT, longValue);
            break;
        case DT_MONTH:
            convertToDTmonth(buffer, parquetT, longValue);
            break;
        case DT_TIME:
            convertToDTtime(buffer, parquetT, longValue);
            break;
        case DT_SECOND:
            convertToDTsecond(buffer, parquetT, longValue);
            break;
        case DT_MINUTE:
            convertToDTminute(buffer, parquetT, longValue);
            break;
        case DT_DATETIME:
            convertToDTdatetime(buffer, parquetT, longValue);
            break;
        default:
            break;
        }
        break;
    }
    case parquet::Type::INT64:
    {
        vector<long long> longValue;
        parquet::Int64Reader *int64_reader = static_cast<parquet::Int64Reader *>(column_reader.get());
        while (int64_reader->HasNext())
        {
            int64_t value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = int64_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                longValue.push_back(value);
            }
            else if (rows_read != 0)
            {
                longValue.push_back(INT64_MIN);
            }
        }
        if(col_descr->logical_type()->is_decimal() || col_descr->converted_type()==parquet::ConvertedType::DECIMAL)
        {
            long long scale=std::pow(10,col_descr->type_scale());
            for (auto x : longValue){   
                if (x == INT64_MIN)
                    buffer.push_back(INT32_MIN);
                else if (x/scale >= INT32_MAX)
                    buffer.push_back(INT32_MAX);
                else if (x/scale <= INT32_MIN + 1)
                    buffer.push_back(INT32_MIN + 1);
                else
                    buffer.push_back(static_cast<int>(x/scale));
            }
            break;
        }
        parquetTime parquetT = convertParquetTimes(col_descr->logical_type(), col_descr->converted_type());
        if (parquetT == parquetTime::None || times_t == DT_INT)
        {
            for (auto x : longValue)
            {
                if (x == INT64_MIN)
                    buffer.push_back(INT32_MIN);
                else if (x >= INT32_MAX)
                    buffer.push_back(INT32_MAX);
                else if (x <= INT32_MIN + 1)
                    buffer.push_back(INT32_MIN + 1);
                else
                    buffer.push_back(static_cast<int>(x));
            }
            break;
        }
        buffer.resize(longValue.size());
        switch (times_t)
        {
        case DT_DATE:
            convertToDTdate(buffer, parquetT, longValue);
            break;
        case DT_MONTH:
            convertToDTmonth(buffer, parquetT, longValue);
            break;
        case DT_TIME:
            convertToDTtime(buffer, parquetT, longValue);
            break;
        case DT_SECOND:
            convertToDTsecond(buffer, parquetT, longValue);
            break;
        case DT_MINUTE:
            convertToDTminute(buffer, parquetT, longValue);
            break;
        case DT_DATETIME:
            convertToDTdatetime(buffer, parquetT, longValue);
            break;
        default:
            break;
        }
        break;
    }
    case parquet::Type::INT96:
    {
        vector<long long> longValue;
        parquet::Int96Reader *int96_reader = static_cast<parquet::Int96Reader *>(column_reader.get());
        while (int96_reader->HasNext())
        {
            parquet::Int96 value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = int96_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                if (times_t != DT_INT)
                {
                    long long nanosTimes = parquet::Int96GetNanoSeconds(value);
                    longValue.push_back(nanosTimes);
                }
                else
                    buffer.push_back(static_cast<int>(value.value[2]));
            }
            else if (rows_read != 0)
            {
                if (times_t != DT_INT)
                {
                    longValue.push_back(INT64_MIN);
                }
                else
                    buffer.push_back(INT32_MIN);
            }
        }
        if (times_t != DT_INT)
            buffer.resize(longValue.size());
        switch (times_t)
        {
        case DT_DATE:
            convertToDTdate(buffer, parquetTime::TimestampNanos, longValue);
            break;
        case DT_MONTH:
            convertToDTmonth(buffer, parquetTime::TimestampNanos, longValue);
            break;
        case DT_TIME:
            convertToDTtime(buffer, parquetTime::TimestampNanos, longValue);
            break;
        case DT_SECOND:
            convertToDTsecond(buffer, parquetTime::TimestampNanos, longValue);
            break;
        case DT_MINUTE:
            convertToDTminute(buffer, parquetTime::TimestampNanos, longValue);
            break;
        case DT_DATETIME:
            convertToDTdatetime(buffer, parquetTime::TimestampNanos, longValue);
            break;
        default:
            break;
        }
        break;
    }
    case parquet::Type::DOUBLE:
    {
        if (times_t != DT_INT)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:DOUBLE" + "->" + Util::getDataTypeString(times_t));
        parquet::DoubleReader *double_reader = static_cast<parquet::DoubleReader *>(column_reader.get());
        while (double_reader->HasNext())
        {
            double value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = double_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                if (value >= INT32_MAX)
                    buffer.push_back(INT32_MAX);
                else if (value <= INT32_MIN + 1)
                    buffer.push_back(INT32_MIN + 1);
                else
                {
                    int v = value >= 0 ? (value + 0.5) : (value - 0.5);
                    buffer.push_back(v);
                }
            }
            else if (rows_read != 0)
            {
                buffer.push_back(INT32_MIN);
            }
        }
        break;
    }
    case parquet::Type::FLOAT:
    {
        if (times_t != DT_INT)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:FLOAT" + "->" + Util::getDataTypeString(times_t));
        parquet::FloatReader *float_reader = static_cast<parquet::FloatReader *>(column_reader.get());
        while (float_reader->HasNext())
        {
            float value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = float_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                if (value >= INT32_MAX)
                    buffer.push_back(INT32_MAX);
                else if (value <= INT32_MIN + 1)
                    buffer.push_back(INT32_MIN + 1);
                else
                {
                    int v = value >= 0 ? (value + 0.5) : (value - 0.5);
                    buffer.push_back(v);
                }
            }
            else if (rows_read != 0)
            {
                buffer.push_back(INT32_MIN);
            }
        }
        break;
    }
    case parquet::Type::FIXED_LEN_BYTE_ARRAY:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:FIXED_LEN_BYTE_ARRAY" + "->" + Util::getDataTypeString(DT_INT));
    case parquet::Type::BYTE_ARRAY:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:BYTE_ARRAY" + "->" + Util::getDataTypeString(DT_INT));

    default:
        throw RuntimeException("unsupported data type");
    }
    return true;
}

bool convertParquetToDolphindbShort(int col_idx, std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, vector<short> &buffer)
{
    switch (col_descr->physical_type())
    {
    case parquet::Type::BOOLEAN:
    {
        parquet::BoolReader *bool_reader = static_cast<parquet::BoolReader *>(column_reader.get());
        while (bool_reader->HasNext())
        {
            bool value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = bool_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                buffer.push_back(static_cast<short>(value));
            }
            else if (rows_read != 0)
            {
                buffer.push_back(INT16_MIN);
            }
        }
        break;
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

        while (int32_reader->HasNext())
        {
            int32_t value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = int32_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                if (value >= INT16_MAX)
                    buffer.push_back(INT16_MAX);
                else if (value <= INT16_MIN + 1)
                    buffer.push_back(INT16_MIN + 1);
                else
                    buffer.push_back(static_cast<short>(value));
            }
            else if (rows_read != 0)
            {
                buffer.push_back(INT16_MIN);
            }
        }
        break;
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
        while (int64_reader->HasNext())
        {
            int64_t value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = int64_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                if (value >= INT16_MAX)
                    buffer.push_back(INT16_MAX);
                else if (value <= INT16_MIN + 1)
                    buffer.push_back(INT16_MIN + 1);
                else
                    buffer.push_back(static_cast<int>(value));
            }
            else if (rows_read != 0)
            {
                buffer.push_back(INT16_MIN);
            }
        }
        break;
    }
    case parquet::Type::INT96:
    {
        parquet::Int96Reader *int96_reader = static_cast<parquet::Int96Reader *>(column_reader.get());
        while (int96_reader->HasNext())
        {
            parquet::Int96 value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = int96_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
                if (value.value[2] >= INT16_MAX)
                    buffer.push_back(INT16_MAX);
                else if (static_cast<short>(value.value[2]) <= INT16_MIN + 1)
                    buffer.push_back(INT16_MIN + 1);
                else
                    buffer.push_back(static_cast<short>(value.value[2]));
            else if (rows_read != 0)
                buffer.push_back(INT16_MIN);
        }
        break;
    }
    case parquet::Type::DOUBLE:
    {
        parquet::DoubleReader *double_reader = static_cast<parquet::DoubleReader *>(column_reader.get());
        while (double_reader->HasNext())
        {
            double value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = double_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                if (value >= INT16_MAX)
                    buffer.push_back(INT16_MAX);
                else if (value <= INT16_MIN + 1)
                    buffer.push_back(INT16_MIN + 1);
                else
                {
                    short v = value >= 0 ? (value + 0.5) : (value - 0.5);
                    buffer.push_back(v);
                }
            }
            else if (rows_read != 0)
            {
                buffer.push_back(INT16_MIN);
            }
        }
        break;
    }
    case parquet::Type::FLOAT:
    {
        parquet::FloatReader *float_reader = static_cast<parquet::FloatReader *>(column_reader.get());
        while (float_reader->HasNext())
        {
            float value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = float_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                if (value >= INT16_MAX)
                    buffer.push_back(INT16_MAX);
                else if (value <= INT16_MIN + 1)
                    buffer.push_back(INT16_MIN + 1);
                else
                {
                    short v = value >= 0 ? (value + 0.5) : (value - 0.5);
                    buffer.push_back(v);
                }
            }
            else if (rows_read != 0)
            {
                buffer.push_back(INT16_MIN);
            }
        }
        break;
    }
    case parquet::Type::FIXED_LEN_BYTE_ARRAY:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:INT32" + "->" + Util::getDataTypeString(DT_SHORT));
    case parquet::Type::BYTE_ARRAY:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:INT32" + "->" + Util::getDataTypeString(DT_SHORT));

    default:
        throw RuntimeException("unsupported data type");
    }
    return true;
}

bool convertParquetToDolphindbLong(int col_idx, std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, vector<long long> &buffer, DATA_TYPE times_t)
{
    switch (col_descr->physical_type())
    {
    case parquet::Type::BOOLEAN:
    {
        parquet::BoolReader *bool_reader = static_cast<parquet::BoolReader *>(column_reader.get());
        while (bool_reader->HasNext())
        {
            bool value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = bool_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                buffer.push_back(static_cast<long long>(value));
            }
            else if (rows_read != 0)
            {
                buffer.push_back(INT64_MIN);
            }
        }
        break;
    }
    case parquet::Type::INT32:
    {
        vector<int> intValue;
        parquet::Int32Reader *int32_reader = static_cast<parquet::Int32Reader *>(column_reader.get());
        // Read all the rows in the column

        while (int32_reader->HasNext())
        {
            int32_t value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = int32_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                intValue.push_back(value);
            }
            else if (rows_read != 0)
            {
                intValue.push_back(INT32_MIN);
            }
        }
        if(col_descr->logical_type()->is_decimal() || col_descr->converted_type()==parquet::ConvertedType::DECIMAL)
        {
            int scale=std::pow(10, col_descr->type_scale());
            for (auto x : intValue)
            {   if (x == INT32_MIN)
                    buffer.push_back(INT64_MIN);
                else
                    buffer.push_back((long long)(x/scale));
            }
            break;
        }
        parquetTime parquetT = convertParquetTimes(col_descr->logical_type(), col_descr->converted_type());
        if (parquetT == parquetTime::None || times_t == DT_LONG)
        {
            for (auto x : intValue)
            {
                if (x == INT32_MIN)
                    buffer.push_back(INT64_MIN);
                else
                    buffer.push_back((long long)x);
            }
            break;
        }
        buffer.resize(intValue.size());
        switch (times_t)
        {
        case DT_NANOTIME:
            convertToDTnanotime(intValue, parquetT, buffer);
            break;
        case DT_NANOTIMESTAMP:
            convertToDTnanotimestamp(intValue, parquetT, buffer);
            break;
        case DT_TIMESTAMP:
            convertToDTtimestamp(intValue, parquetT, buffer);
            break;
        default:
            break;
        }
        break;
    }
    case parquet::Type::INT64:
    {

        parquet::Int64Reader *int64_reader = static_cast<parquet::Int64Reader *>(column_reader.get());
        while (int64_reader->HasNext())
        {
            int64_t value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = int64_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                buffer.push_back(static_cast<long long>(value));
            }
            else if (rows_read != 0)
            {
                buffer.push_back(INT64_MIN);
            }
        }
        if(col_descr->logical_type()->is_decimal() || col_descr->converted_type()==parquet::ConvertedType::DECIMAL)
        {
            long long scale=std::pow(10, col_descr->type_scale());
            for (auto &x : buffer)
                if(x!=INT64_MIN)
                    x=x/scale;
            break;
        }
        parquetTime parquetT = convertParquetTimes(col_descr->logical_type(), col_descr->converted_type());
        if (parquetT == parquetTime::None || times_t == DT_LONG)
        {
            break;
        }
        vector<int> intValue;
        switch (times_t)
        {
        case DT_NANOTIME:
            convertToDTnanotime(intValue, parquetT, buffer);
            break;
        case DT_NANOTIMESTAMP:
            convertToDTnanotimestamp(intValue, parquetT, buffer);
            break;
        case DT_TIMESTAMP:
            convertToDTtimestamp(intValue, parquetT, buffer);
            break;
        default:
            break;
        }
        break;
    }
    case parquet::Type::INT96:
    {
        parquet::Int96Reader *int96_reader = static_cast<parquet::Int96Reader *>(column_reader.get());
        while (int96_reader->HasNext())
        {
            parquet::Int96 value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = int96_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                if (times_t != DT_LONG)
                {
                    long long nanosTimes = parquet::Int96GetNanoSeconds(value);
                    buffer.push_back(nanosTimes);
                }
                else
                {
                    int64_t v = value.value[1];
                    v = v << 32;
                    v += value.value[2];
                    buffer.push_back(static_cast<long long>(v));
                }
            }
            else if (rows_read != 0)
            {
                buffer.push_back(INT64_MIN);
            }
        }
        vector<int> intValue;
        switch (times_t)
        {
        case DT_TIMESTAMP:
            convertToDTtimestamp(intValue, parquetTime::TimestampNanos, buffer);
            break;
        case DT_NANOTIMESTAMP:
            convertToDTnanotimestamp(intValue, parquetTime::TimestampNanos, buffer);
            break;
        case DT_NANOTIME:
            convertToDTnanotime(intValue, parquetTime::TimestampNanos, buffer);
            break;
        default:
            break;
        }
        break;
    }
    case parquet::Type::DOUBLE:
    {
        parquet::DoubleReader *double_reader = static_cast<parquet::DoubleReader *>(column_reader.get());
        while (double_reader->HasNext())
        {
            double value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = double_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                if (value >= INT64_MAX)
                    buffer.push_back(INT64_MAX);
                else if (value <= INT64_MIN + 1)
                    buffer.push_back(INT64_MIN + 1);
                else
                {
                    long long v = value >= 0 ? (value + 0.5) : (value - 0.5);
                    buffer.push_back(v);
                }
            }
            else if (rows_read != 0)
            {
                buffer.push_back(INT64_MIN);
            }
        }
        break;
    }
    case parquet::Type::FLOAT:
    {
        parquet::FloatReader *float_reader = static_cast<parquet::FloatReader *>(column_reader.get());
        while (float_reader->HasNext())
        {
            float value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = float_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                if (value >= INT64_MAX)
                    buffer.push_back(INT64_MAX);
                else if (value <= INT64_MIN + 1)
                    buffer.push_back(INT64_MIN + 1);
                else
                {
                    long long v = value >= 0 ? (value + 0.5) : (value - 0.5);
                    buffer.push_back(v);
                }
            }
            else if (rows_read != 0)
            {
                buffer.push_back(INT64_MIN);
            }
        }
        break;
    }
    case parquet::Type::FIXED_LEN_BYTE_ARRAY:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:FIXED_LEN_BYTE_ARRAY" + "->" + Util::getDataTypeString(DT_LONG));
    case parquet::Type::BYTE_ARRAY:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:BYTE_ARRAY" + "->" + Util::getDataTypeString(DT_LONG));

    default:
        throw RuntimeException("unsupported data type");
    }
    return true;
}

bool isDecimal(const parquet::ColumnDescriptor *col_descr)
{
    return col_descr->logical_type()->is_decimal() || (col_descr->converted_type()==parquet::ConvertedType::DECIMAL);
}

bool convertParquetToDolphindbFloat(int col_idx, std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, vector<float> &buffer)
{
    switch (col_descr->physical_type())
    {
    case parquet::Type::BOOLEAN:
    {
        parquet::BoolReader *bool_reader = static_cast<parquet::BoolReader *>(column_reader.get());
        while (bool_reader->HasNext())
        {
            bool value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = bool_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                buffer.push_back(static_cast<float>(value));
            }
            else if (rows_read != 0)
            {
                buffer.push_back(FLT_MIN);
            }
        }
        break;
    }
    case parquet::Type::INT32:
    {
        parquet::Int32Reader *int32_reader = static_cast<parquet::Int32Reader *>(column_reader.get());
        // Read all the rows in the column
        bool Decimal=isDecimal(col_descr);
        int scale=std::pow(10, col_descr->type_scale());
        while (int32_reader->HasNext())
        {
            int32_t value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = int32_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                if(Decimal)
                {
                    double v=((double)value)/scale;
                    buffer.push_back(static_cast<float>(v));
                }
                buffer.push_back(static_cast<float>(value));
            }
            else if (rows_read != 0)
            {
                buffer.push_back(FLT_MIN);
            }
        }
        break;
    }
    case parquet::Type::INT64:
    {
        long long scale=std::pow(10, col_descr->type_scale());
        bool Decimal=isDecimal(col_descr);
        parquet::Int64Reader *int64_reader = static_cast<parquet::Int64Reader *>(column_reader.get());
        while (int64_reader->HasNext())
        {
            int64_t value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = int64_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                if(Decimal)
                {
                    double v=((double)value)/scale;
                    buffer.push_back(static_cast<float>(v));
                }
                else 
                    buffer.push_back(static_cast<float>(value));
            }
            else if (rows_read != 0)
            {
                buffer.push_back(FLT_MIN);
            }
        }
        
        break;
    }
    case parquet::Type::INT96:
    {
        parquet::Int96Reader *int96_reader = static_cast<parquet::Int96Reader *>(column_reader.get());
        while (int96_reader->HasNext())
        {
            parquet::Int96 value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = int96_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                int64_t v = value.value[1];
                v = v << 32;
                v += value.value[2];
                buffer.push_back(static_cast<float>(v));
            }
            else if (rows_read != 0)
            {
                buffer.push_back(FLT_MIN);
            }
        }
        break;
    }
    case parquet::Type::DOUBLE:
    {
        parquet::DoubleReader *double_reader = static_cast<parquet::DoubleReader *>(column_reader.get());
        while (double_reader->HasNext())
        {
            double value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = double_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                buffer.push_back(static_cast<float>(value));
            }
            else if (rows_read != 0)
            {
                buffer.push_back(FLT_MIN);
            }
        }
        break;
    }
    case parquet::Type::FLOAT:
    {
        parquet::FloatReader *float_reader = static_cast<parquet::FloatReader *>(column_reader.get());
        while (float_reader->HasNext())
        {
            float value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = float_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                buffer.push_back(value);
            }
            else if (rows_read != 0)
            {
                buffer.push_back(FLT_MIN);
            }
        }
        break;
    }
    case parquet::Type::FIXED_LEN_BYTE_ARRAY:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:FIXED_LEN_BYTE_ARRAY" + "->" + Util::getDataTypeString(DT_FLOAT));
    case parquet::Type::BYTE_ARRAY:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:BYTE_ARRAY" + "->" + Util::getDataTypeString(DT_FLOAT));

    default:
        throw RuntimeException("unsupported data type");
    }
    return true;
}

bool convertParquetToDolphindbDouble(int col_idx, std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, vector<double> &buffer)
{
    switch (col_descr->physical_type())
    {
    case parquet::Type::BOOLEAN:
    {
        parquet::BoolReader *bool_reader = static_cast<parquet::BoolReader *>(column_reader.get());
        while (bool_reader->HasNext())
        {
            bool value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = bool_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                buffer.push_back(static_cast<double>(value));
            }
            else if (rows_read != 0)
            {
                buffer.push_back(DBL_MIN);
            }
        }
        break;
    }
    case parquet::Type::INT32:
    {
        int scale=std::pow(10, col_descr->type_scale());
        bool Decimal=isDecimal(col_descr);
        parquet::Int32Reader *int32_reader = static_cast<parquet::Int32Reader *>(column_reader.get());
        // Read all the rows in the column

        while (int32_reader->HasNext())
        {
            int32_t value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = int32_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                if(Decimal)
                {
                    double v=((double)value)/scale;
                    buffer.push_back(v);
                }
                else
                    buffer.push_back(static_cast<double>(value));
            }
            else if (rows_read != 0)
            {
                buffer.push_back(DBL_MIN);
            }
        }
        break;
    }
    case parquet::Type::INT64:
    {
        long long scale=std::pow(10, col_descr->type_scale());
        bool Decimal=isDecimal(col_descr);
        parquet::Int64Reader *int64_reader = static_cast<parquet::Int64Reader *>(column_reader.get());
        while (int64_reader->HasNext())
        {
            int64_t value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = int64_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                if(Decimal)
                {
                    double v=((double)value)/scale;
                    buffer.push_back(v);
                }
                else
                    buffer.push_back(static_cast<double>(value));
            }
            else if (rows_read != 0)
            {
                buffer.push_back(DBL_MIN);
            }
        }
        break;
    }
    case parquet::Type::INT96:
    {
        parquet::Int96Reader *int96_reader = static_cast<parquet::Int96Reader *>(column_reader.get());
        while (int96_reader->HasNext())
        {
            parquet::Int96 value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = int96_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                int64_t v = value.value[1];
                v = v << 32;
                v += value.value[2];
                buffer.push_back(static_cast<double>(v));
            }
            else if (rows_read != 0)
            {
                buffer.push_back(DBL_MIN);
            }
        }
        break;
    }
    case parquet::Type::DOUBLE:
    {
        parquet::DoubleReader *double_reader = static_cast<parquet::DoubleReader *>(column_reader.get());
        while (double_reader->HasNext())
        {
            double value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = double_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                buffer.push_back((double)value);
            }
            else if (rows_read != 0)
            {
                buffer.push_back(DBL_MIN);
            }
        }
        break;
    }
    case parquet::Type::FLOAT:
    {
        parquet::FloatReader *float_reader = static_cast<parquet::FloatReader *>(column_reader.get());
        while (float_reader->HasNext())
        {
            float value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = float_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                buffer.push_back((double)value);
            }
            else if (rows_read != 0)
            {
                buffer.push_back(DBL_MIN);
            }
        }
        break;
    }
    case parquet::Type::FIXED_LEN_BYTE_ARRAY:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:FIXED_LEN_BYTE_ARRAY" + "->" + Util::getDataTypeString(DT_DOUBLE));
    case parquet::Type::BYTE_ARRAY:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:BYTE_ARRAY" + "->" + Util::getDataTypeString(DT_DOUBLE));

    default:
        throw RuntimeException("unsupported data type");
    }
    return true;
}

bool convertParquetToDolphindbString(int col_idx, std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, vector<string> &buffer, DATA_TYPE string_t)
{
    switch (col_descr->physical_type())
    {
    case parquet::Type::BOOLEAN:
    {
        if (string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:BOOLEAN" + "->" + Util::getDataTypeString(string_t));
        parquet::BoolReader *bool_reader = static_cast<parquet::BoolReader *>(column_reader.get());
        while (bool_reader->HasNext())
        {
            bool value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = bool_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                if (value)
                    buffer.push_back("true");
                else
                    buffer.push_back("false");
            }
            else if (rows_read != 0)
            {

                buffer.push_back(STR_MIN);
            }
        }
        break;
    }
    case parquet::Type::INT32:
    {
        if (string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:INT32" + "->" + Util::getDataTypeString(string_t));
        parquet::Int32Reader *int32_reader = static_cast<parquet::Int32Reader *>(column_reader.get());
        // Read all the rows in the column

        while (int32_reader->HasNext())
        {
            int32_t value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = int32_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                buffer.push_back(std::to_string(value));
            }
            else if (rows_read != 0)
            {
                buffer.push_back(STR_MIN);
            }
        }

        break;
    }
    case parquet::Type::INT64:
    {
        if (string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:INT64" + "->" + Util::getDataTypeString(string_t));
        parquet::Int64Reader *int64_reader = static_cast<parquet::Int64Reader *>(column_reader.get());
        while (int64_reader->HasNext())
        {
            int64_t value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = int64_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                buffer.push_back(std::to_string(value));
            }
            else if (rows_read != 0)
            {
                buffer.push_back(STR_MIN);
            }
        }
        break;
    }
    case parquet::Type::INT96:
    {
        if (string_t == DT_UUID)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:INT96" + "->" + Util::getDataTypeString(string_t));
        parquet::Int96Reader *int96_reader = static_cast<parquet::Int96Reader *>(column_reader.get());
        while (int96_reader->HasNext())
        {
            parquet::Int96 value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = int96_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                if (string_t == DT_INT128)
                {
                    char bytes[16];
                    for (int i = 0; i < 3; i++)
                    {
                        bytes[i * 4] = (char)((value.value[i] >> 24) & 0xFF);
                        bytes[i * 4 + 1] = (char)((value.value[i] >> 16) & 0xFF);
                        bytes[i * 4 + 2] = (char)((value.value[i] >> 8) & 0xFF);
                        bytes[i * 4 + 3] = (char)(value.value[i] & 0xFF);
                    }
                    buffer.push_back(string(bytes));
                }
                else
                {
                    buffer.push_back(parquet::Int96ToString(value));
                }
            }
            else if (rows_read != 0)
            {
                buffer.push_back(STR_MIN);
            }
        }
        break;
    }
    case parquet::Type::DOUBLE:
    {
        if (string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:DOUBLE" + "->" + Util::getDataTypeString(string_t));
        parquet::DoubleReader *double_reader = static_cast<parquet::DoubleReader *>(column_reader.get());
        while (double_reader->HasNext())
        {
            double value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = double_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                std::ostringstream out;
                out << value;
                buffer.push_back(out.str());
            }
            else if (rows_read != 0)
            {
                buffer.push_back(STR_MIN);
            }
        }
        break;
    }
    case parquet::Type::FLOAT:
    {
        if (string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:FLOAT" + "->" + Util::getDataTypeString(string_t));
        parquet::FloatReader *float_reader = static_cast<parquet::FloatReader *>(column_reader.get());
        while (float_reader->HasNext())
        {
            float value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = float_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                std::ostringstream out;
                out << value;
                buffer.push_back(out.str());
            }
            else if (rows_read != 0)
            {
                buffer.push_back(STR_MIN);
            }
        }
        break;
    }
    case parquet::Type::FIXED_LEN_BYTE_ARRAY:
    {
        if (string_t == DT_INT128)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:FIXED_LEN_BYTE_ARRAY" + "->" + Util::getDataTypeString(string_t));
        int fixed_length = col_descr->type_length();
        if (string_t == DT_UUID && fixed_length != 16)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:FIXED_LEN_BYTE_ARRAY, length NOT 16" + "->" + Util::getDataTypeString(string_t));
        parquet::FixedLenByteArrayReader *flba_reader = static_cast<parquet::FixedLenByteArrayReader *>(column_reader.get());
        while (flba_reader->HasNext())
        {
            parquet::FixedLenByteArray value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = flba_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                buffer.push_back(std::string(reinterpret_cast<const char *>(value.ptr), fixed_length));
            }
            else if (rows_read != 0)
            {
                buffer.push_back(STR_MIN);
            }
        }

        break;
    }
    case parquet::Type::BYTE_ARRAY:
    {
        if (string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:BYTE_ARRAY" + "->" + Util::getDataTypeString(string_t));
        parquet::ByteArrayReader *ba_reader = static_cast<parquet::ByteArrayReader *>(column_reader.get());
        int i =0;
        while (ba_reader->HasNext())
        {
            parquet::ByteArray value;
            int64_t values_read;
            short def_level;
            short rep_level;
            int64_t rows_read = ba_reader->ReadBatch(1, &def_level, &rep_level, &value, &values_read);
            if (rows_read == values_read && rows_read != 0)
            {
                buffer.push_back(parquet::ByteArrayToString(value));
            }
            else if (rows_read != 0)
            {
                buffer.push_back(STR_MIN);
            }
            i++;
        }
        break;
    }
    default:
        throw RuntimeException("unsupported data type");
    }
    return true;
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

ConstantSP loadParquet(ParquetReadOnlyFile *file, const ConstantSP &schema, const ConstantSP &column, const int rowGroupStart, const int rowGroupEnd)
{
    TableSP tableWithSchema = DBFileIO::createEmptyTableFromSchema(schema);
    int col_num = column->size();
    vector<VectorSP> dolphindbCol(col_num);
    createNewVectorSP(dolphindbCol, tableWithSchema);
    for (int row = rowGroupStart; row < rowGroupEnd; row++)
    {
        std::shared_ptr<parquet::RowGroupReader> row_reader = file->rowReader(row);
        if (row_reader == nullptr)
            throw RuntimeException("Read parquet file failed.");
        
        for (int i = 0; i < col_num; i++)
        {
            int col_idx = column->getInt(i);
            std::shared_ptr<parquet::ColumnReader> column_reader = row_reader->Column(col_idx);
            const parquet::ColumnDescriptor *col_descr = column_reader->descr();
            if (col_descr->max_repetition_level() != 0)
                throw RuntimeException("not support parquet repeated field yet.");
            DATA_TYPE dolphin_t = dolphindbCol[i]->getType();
            switch (dolphin_t)
            {
            case DT_BOOL:
            {
                vector<char> buffer;
                convertParquetToDolphindbBool(i, column_reader, col_descr, buffer);
                dolphindbCol[i]->appendChar(buffer.data(), buffer.size());
                break;
            }
            case DT_CHAR:
            {
                vector<char> buffer;
                convertParquetToDolphindbChar(i, column_reader, col_descr, buffer);
                dolphindbCol[i]->appendChar(buffer.data(), buffer.size());
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
                vector<int> buffer;
                convertParquetToDolphindbInt(i, column_reader, col_descr, buffer, dolphin_t);
                dolphindbCol[i]->appendInt(buffer.data(), static_cast<int>(buffer.size()));
                break;
            }
            case DT_LONG:
            case DT_NANOTIME:
            case DT_NANOTIMESTAMP:
            case DT_TIMESTAMP:
            {
                vector<long long> buffer;
                convertParquetToDolphindbLong(i, column_reader, col_descr, buffer, dolphin_t);
                dolphindbCol[i]->appendLong(buffer.data(), static_cast<int>(buffer.size()));
                break;
            }
            case DT_SHORT:
            {
                vector<short> buffer;
                convertParquetToDolphindbShort(i, column_reader, col_descr, buffer);
                dolphindbCol[i]->appendShort(buffer.data(), buffer.size());
                break;
            }
            case DT_FLOAT:
            {
                vector<float> buffer;
                convertParquetToDolphindbFloat(i, column_reader, col_descr, buffer);
                dolphindbCol[i]->appendFloat(buffer.data(), buffer.size());
                break;
            }
            case DT_DOUBLE:
            {
                vector<double> buffer;
                convertParquetToDolphindbDouble(i, column_reader, col_descr, buffer);
                dolphindbCol[i]->appendDouble(buffer.data(), buffer.size());
                break;
            }
            case DT_INT128:
            case DT_UUID:
            case DT_STRING:
            case DT_SYMBOL:
            {
                vector<string> buffer;
                convertParquetToDolphindbString(i, column_reader, col_descr, buffer, dolphin_t);
                dolphindbCol[i]->appendString(buffer.data(), buffer.size());
                break;
            }
            default:
                throw RuntimeException("unsupported data type.");
                break;
            }
        }
    }
    return appendColumnVecToTable(tableWithSchema, dolphindbCol);
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

} // namespace ParquetPluginImp