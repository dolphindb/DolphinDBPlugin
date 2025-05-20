#include "parquet_plugin.h"
#include "ddbplugin/PluginLoggerImp.h"
#include "ddbplugin/Plugin.h"
#include "SpecialConstant.h"

ConstantSP extractParquetSchema(Heap *heap, vector<ConstantSP> &arguments)
{
    ConstantSP filename = arguments[0];
    if (filename.isNull() || filename->getType() != DT_STRING || filename->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, "The filename and dataset must be a string scalar.");

    ConstantSP schema = ParquetPluginImp::extractParquetSchema(filename->getString());

    return schema;
}

ConstantSP loadParquet(Heap *heap, vector<ConstantSP> &arguments)
{
	if (arguments.empty())
		throw IllegalArgumentException(__FUNCTION__, "Arguments can't be empty.");
    ConstantSP filename = arguments[0];

    int rowGroupStart = 0;
    int rowGroupNum = 0;
    ConstantSP schema = ParquetPluginImp::nullSP;
    ConstantSP column = new Void();
    if (!(filename->getType() == DT_STRING && filename->getForm() == DF_SCALAR) && filename->getType() != DT_RESOURCE )
        throw IllegalArgumentException(__FUNCTION__, "The filename and dataset must be a string scalar.");
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
            throw IllegalArgumentException(__FUNCTION__, "columnsToLoad must be a integral vector");
        else
            column = arguments[2];
    }
    if (arguments.size() >= 4 && !arguments[3]->isNull())
    {
        if (arguments[3]->isScalar() && arguments[3]->getCategory() == INTEGRAL)
        {
            rowGroupStart = arguments[3]->getInt();
            if (rowGroupStart < 0)
                throw IllegalArgumentException(__FUNCTION__, "startRowGroup must be positive.");
        }
        else
            throw IllegalArgumentException(__FUNCTION__, "startRowGroup must be an integer.");
    }
    if (arguments.size() >= 5 && !arguments[4]->isNull())
    {
        if (arguments[4]->isScalar() && arguments[4]->getCategory() == INTEGRAL)
        {
            rowGroupNum = arguments[4]->getInt();
            if (rowGroupNum <= 0)
                throw IllegalArgumentException(__FUNCTION__, "rowGroupNum must be a positive integer.");
        }
        else
            throw IllegalArgumentException(__FUNCTION__, "rowGroupNum must be an integer.");
    }
    return ParquetPluginImp::loadParquetByFileName(heap, filename->getString(), schema, column, rowGroupStart, rowGroupNum);
}

ConstantSP loadParquetHdfs(Heap *heap, vector<ConstantSP> &arguments)
{
    if(arguments[0]->getType() != DT_RESOURCE || arguments[0]->getString() != "hdfs readFile address")
        throw IllegalArgumentException(__FUNCTION__,"The first arguments should be resource");
    if(arguments[1]->getType() != DT_RESOURCE || arguments[1]->getString() != "hdfs readFile length")
        throw IllegalArgumentException(__FUNCTION__,"The second arguments should be resource");

    void *buffer = (void *)arguments[0]->getLong();
	if (!buffer)
		throw IllegalArgumentException(__FUNCTION__,"Buffer address can't be null.");
    uint64_t *length = (uint64_t *)arguments[1]->getLong();
	if (!length)
		throw IllegalArgumentException(__FUNCTION__,"Length address can't be null.");
    return ParquetPluginImp::loadParquetHdfs(buffer, *length);
}

ConstantSP loadParquetEx(Heap *heap, vector<ConstantSP> &arguments)
{
	if (arguments.size() < 4)
        throw IllegalArgumentException(__FUNCTION__, "Arguments size can't less than four.");
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
        throw IllegalArgumentException(__FUNCTION__, "The partitionColumns columns must be in string or string vector.");
    if (!(filename->getType() == DT_STRING && filename->getForm() == DF_SCALAR))
        throw IllegalArgumentException(__FUNCTION__, "The filename must be a string scalar.");
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
            throw IllegalArgumentException(__FUNCTION__, "columnsToLoad must be a integral vector");
        else
            column = arguments[5];
    }
    if (arguments.size() >= 7 && !arguments[6]->isNull())
    {
        if (arguments[6]->isScalar() && arguments[6]->getCategory() == INTEGRAL){
            rowGroupStart = arguments[6]->getInt();
            if (rowGroupStart < 0)
                throw IllegalArgumentException(__FUNCTION__, "startRowGroup must be a non-negative integer.");
        }
        else
            throw IllegalArgumentException(__FUNCTION__, "startRowGroup must be an integer.");
    }
    if (arguments.size() >= 8 && !arguments[7]->isNull())
    {
        if (arguments[7]->isScalar() && arguments[7]->getCategory() == INTEGRAL){
            rowGroupNum = arguments[7]->getInt();
            if (rowGroupNum <= 0)
                throw IllegalArgumentException(__FUNCTION__, "rowGroupNum must be a positive integer.");
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
	if (arguments.empty())
		throw IllegalArgumentException(__FUNCTION__, "Arguments can't be empty.");
    ConstantSP filename = arguments[0];

    ConstantSP schema = ParquetPluginImp::nullSP;

    if (filename.isNull() || !(filename->getType() == DT_STRING && filename->getForm() == DF_SCALAR))
        throw IllegalArgumentException(__FUNCTION__, "The filename must be a string scalar.");
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
	if (arguments.size() < 2)
		throw IllegalArgumentException(__FUNCTION__, "Arguments can't less than two.");
    ConstantSP tb = arguments[0];
    ConstantSP filename = arguments[1];
    if(tb.isNull() || !tb->isTable())
        throw IllegalArgumentException(__FUNCTION__, "table must be a table.");
    if(filename.isNull() || filename->getType() != DT_STRING || filename->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, "The filename must be a string scalars.");
    string compression;
    if(arguments.size() >= 3 && !arguments[2]->isNothing()){
        if(arguments[2]->getType() != DT_STRING || arguments[2]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, "The compressMethod must be a string scalar.");
        compression = arguments[2]->getString();
        if(ParquetPluginImp::COMPRESSION_SET.count(compression) == 0)
            throw IllegalArgumentException(__FUNCTION__, "The compressMethod type only supports snappy, gzip, zstd.");
    }
    return ParquetPluginImp::saveParquet(tb, filename->getString(), compression);
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
std::atomic<int> READ_THREAD_NUM(1);
void ParquetReadOnlyFile::open(const std::string &file_name)
{
    close();
    try
    {
        std::unique_ptr<parquet::ParquetFileReader> parquet_reader = parquet::ParquetFileReader::OpenFile(file_name, false);

        _parquet_reader = std::move(parquet_reader);
    }
    catch (parquet::ParquetStatusException& e)
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
    catch (parquet::ParquetStatusException& e)
    {
        throw IOException(e.what());
    }
}

std::shared_ptr<parquet::RowGroupReader> ParquetReadOnlyFile::rowReader(int i)
{
    if (i >= fileMetadataReader()->num_row_groups() || (_parquet_reader.get() == nullptr))
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
        throw RuntimeException("unsupported parquet physical type " + std::to_string(physical_t));
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
                throw RuntimeException("unsupported data type" + std::to_string(physical_t));
            }
        }
    }
    case parquet::LogicalType::Type::STRING:
    {
        if (physical_t != parquet::Type::BYTE_ARRAY)
            throw RuntimeException("unsupported logical type STRING when physical type is not BYTE_ARRAY");
        return "STRING";
    }

    case parquet::LogicalType::Type::ENUM:
    {
        if (physical_t != parquet::Type::BYTE_ARRAY)
            throw RuntimeException("unsupported logical type ENUM when physical type is not BYTE_ARRAY");
        return "SYMBOL";
    }
    case parquet::LogicalType::Type::DATE:
    {
        if (physical_t != parquet::Type::INT32)
            throw RuntimeException("unsupported logical type DATE when physical type is not INT32");
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
            throw RuntimeException("unsupported logical type TIMESTAMP when physical type is not INT64");
        if (timeUnit==parquet::LogicalType::TimeUnit::unit::UNKNOWN)
            throw RuntimeException("unknown TIMESTAMP precision");
        else if(timeUnit==parquet::LogicalType::TimeUnit::unit::MILLIS)
            return "TIMESTAMP";
        else
            return "NANOTIMESTAMP";
    }
    case parquet::LogicalType::Type::UUID:
        throw RuntimeException("unsupported logical type UUID");
    case parquet::LogicalType::Type::DECIMAL:
        return "DOUBLE";
    case parquet::LogicalType::Type::NIL:{
        switch (physical_t){
            case parquet::Type::type::BOOLEAN:
                return "BOOL";
            case parquet::Type::type::INT32:
                return "INT";
            case parquet::Type::type::INT64:
            case parquet::Type::type::INT96:
                return "LONG";
            case parquet::Type::type::FLOAT:
                return "FLOAT";
            case parquet::Type::type::DOUBLE:
                return "DOUBLE";
            case parquet::Type::type::BYTE_ARRAY:
                return "STRING";
            case parquet::Type::type::FIXED_LEN_BYTE_ARRAY:
                return "STRING";
            default:
                throw RuntimeException("unsupported logical type NIL when physical type is " + std::to_string(physical_t));
        }
    }
    default:
        throw RuntimeException("unsupported logical type " + std::to_string(logi_t));
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
        throw RuntimeException("unsupported parquet converted type " + std::to_string(converted_t));
    }
    return "";
}

bool getSchemaCol(const parquet::SchemaDescriptor *schema_descr, const ConstantSP &col_idx, vector<ConstantSP> &dolpindbCol)
{
    if (dolpindbCol.size() != 2){
        return false;
    }
	if (col_idx.isNull()){
		throw RuntimeException("col_idx can't be null.");
    }
    int col_num = col_idx->size();
    if (col_num == 0)
        return false;
    dolpindbCol[0] = Util::createVector(DT_STRING, col_num, col_num);
    dolpindbCol[1] = Util::createVector(DT_STRING, col_num, col_num);
	if (dolpindbCol[0].isNull() || dolpindbCol[1].isNull())
		throw RuntimeException("createVector failed.");
    for (int i = 0; i < col_num; i++)
    {
        const parquet::ColumnDescriptor *col = schema_descr->Column(col_idx->getInt(i));
        if (!col)
            throw RuntimeException("columnsToLoad index out of range");
        string path = col->path()->ToDotString();
        string name = path;
        auto lt = col->converted_type();
        auto la = col->logical_type();
        auto lp = col->physical_type();
        string type;
        type = (la && la->is_valid()) ? getLayoutColumnType(la, lp, col->sort_order()) : getLayoutColumnType(lt, lp, col->sort_order());
        if (col->max_repetition_level() != 0){
            type = type + "[]";
        }
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
	if (!file_metadata.get())
		throw RuntimeException("fileMetadataReader failed");
    const parquet::SchemaDescriptor *s = file_metadata->schema();
	if (!s)
		throw RuntimeException("get schema failed");
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

void createNewVectorSP(vector<ConstantSP> &dolpindb_v, const TableSP &tb, int totalRows)
{
	if (tb.isNull())
		throw RuntimeException("table can't be null");
    int col_num = dolpindb_v.size();
    if(tb->columns() != col_num)
        throw RuntimeException("the size of column schema not match");
    for (int i = 0; i < col_num; i++)
    {
        DATA_TYPE dt = tb->getColumnType(i);
        if(dt >= 64){
            dt = static_cast<DATA_TYPE>(dt - 64);
        }
        if(dt == DT_SYMBOL)
            dolpindb_v[i] = Util::createVector(DT_STRING, totalRows);
        else
            dolpindb_v[i] = Util::createVector(dt, totalRows);
    }
}

TableSP appendColumnVecToTable(TableSP tb, vector<VectorSP> &colVec, INDEX& insertedRows)
{
    if (tb.isNull() || tb->isNull())
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
	if (!ratio)
		throw RuntimeException("Dividend ratio can't be zero.");
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
		if (!ratio)
			throw RuntimeException("Dividend ratio can't be zero.");
        for (int i = 0; i < bufSize; i++)
        {
            intValue[i] = intValue[i] % 86400000000 / ratio;
        }
        return true;
    case parquetTime::TimestampMillis:
        ratio = -Util::getTemporalConversionRatio(DT_TIME, DT_MINUTE);
		if (!ratio)
			throw RuntimeException("Dividend ratio can't be zero.");
        for (int i = 0; i < bufSize; i++)
        {
            intValue[i] = intValue[i] % 86400000 / ratio;
        }
        return true;
    case parquetTime::TimestampNanos:
        ratio = -Util::getTemporalConversionRatio(DT_NANOTIME, DT_MINUTE);
		if (!ratio)
			throw RuntimeException("Dividend ratio can't be zero.");
        for (int i = 0; i < bufSize; i++)
        {
            intValue[i] = intValue[i] % 86400000000000 / ratio;
        }
        return true;
    case parquetTime::TimeMicros:
        ratio = -Util::getTemporalConversionRatio(DT_TIME, DT_MINUTE);
        ratio *= 1000;
		if (!ratio)
			throw RuntimeException("Dividend ratio can't be zero.");
        for (int i = 0; i < bufSize; i++)
        {
            intValue[i] = intValue[i] / ratio;
        }
        return true;
    case parquetTime::TimeMillis:
        ratio = -Util::getTemporalConversionRatio(DT_TIME, DT_MINUTE);
		if (!ratio)
			throw RuntimeException("Dividend ratio can't be zero.");
        for (int i = 0; i < bufSize; i++)
        {
            intValue[i] = intValue[i] / ratio;
        }
        return true;
    case parquetTime::TimeNanos:
        ratio = -Util::getTemporalConversionRatio(DT_NANOTIME, DT_MINUTE);
		if (!ratio)
			throw RuntimeException("Dividend ratio can't be zero.");
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
	if (!logical_t.get())
		throw RuntimeException("logical_t can't be null");
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

template <typename ReaderType, typename StoreType>
int64_t ReadBatchFunc(ReaderType* reader, size_t batchSize, short* def_level, short* rep_level, StoreType* value, int64_t& values_read){
    return reader->ReadBatch(batchSize, def_level, rep_level, value, &values_read);
}
template<typename ValueType, typename StoreType>
struct ValueGetter {
    inline static ValueType getValue(StoreType& value, int scale) {
        return static_cast<ValueType>(value) / scale;
    }
    inline static ValueType getValueNoScale(StoreType& value) {
        return value;
    }
};

template<typename ValueType>
struct ValueGetter<ValueType, parquet::Int96> {
    inline static ValueType getValue(parquet::Int96& value, int scale) {
        int128 v = value.value[2] + (int128(value.value[1]) << 32) + (int128(value.value[0]) << 64);
        return v / scale;
    }
    inline static ValueType getValueNoScale(parquet::Int96& value) {
        int128 v = value.value[2] + (int128(value.value[1]) << 32) + (int128(value.value[0]) << 64);
        return v;
    }
};

template<typename StoreType>
struct ValueGetter<std::string, StoreType> {
    inline static std::string getValue(StoreType& value, int scale) {
        return std::to_string(value / scale);
    }
    inline static std::string getValueNoScale(StoreType& value) {
        return std::to_string(value);
    }
};

template<typename ValueType, typename StoreType, typename DDBStoreType>
struct SetVectorData{
    inline static void set(DDBStoreType* buffer, int len, StoreType* value, int scale, vector<short>& def_level, bool containNull, const parquet::ColumnDescriptor *col_descr){
        int index = 0;
        if(scale != 1){
            for(int i = 0; i < len; ++i){
                if (def_level[i] >= col_descr->max_definition_level())
                {
                    buffer[i] = ValueGetter<ValueType, StoreType>::getValue(value[index++], scale);
                }
                else
                {
                    buffer[i] = dolphindb::getNullValue<DDBStoreType>();
                }
            }
        }else{
            for(int i = 0; i < len; ++i){
                if (def_level[i] >= col_descr->max_definition_level())
                {
                    buffer[i] = ValueGetter<ValueType, StoreType>::getValueNoScale(value[index++]);
                }
                else
                {
                    buffer[i] = dolphindb::getNullValue<DDBStoreType>();
                }
            }
        }
    }
};

template<typename ValueType, typename DDBStoreType>
struct SetVectorData<ValueType, ValueType, DDBStoreType>{
    inline static void set(DDBStoreType* buffer, int len, ValueType* value, int scale, vector<short>& def_level, bool containNull, const parquet::ColumnDescriptor *col_descr){
        if(scale != 1){
            for(int i = 0; i < len; i++){
                value[i] = value[i] / scale;
            }
        }
        if(!containNull){
            memcpy(buffer, value, len * sizeof(ValueType));
        }else{
            int index = 0;
            for(int i = 0; i < len; i++){
                if (def_level[i] >= col_descr->max_definition_level())
                {
                    buffer[i] = value[index++];
                }
                else
                {
                    buffer[i] = dolphindb::getNullValue<DDBStoreType>();
                }
            }
        }
    }
};

void getIndex(vector<short> rep_level, int readCount, IndexArgs& indexArgs){
    if(indexArgs.isArray_){
        for(int i = 0; i < readCount; ++i){
            if(rep_level[i] == 0 && !(indexArgs.index_.size() == 0 && indexArgs.cumSum_ == 0)){
                indexArgs.lastIndex_ += indexArgs.cumSum_;
                indexArgs.index_.push_back(indexArgs.lastIndex_);
                indexArgs.cumSum_ = 1;
            }else{
                indexArgs.cumSum_++;
            }
        }
    }
}

template<typename StoreType, typename DDBStoreType, typename ValueType = DDBStoreType, typename ReaderType>
int convertParquetToDolphindbInternal(int col_idx, ReaderType* column_reader, const parquet::ColumnDescriptor *col_descr, DDBStoreType* buffer, size_t batchSize, bool& containNull, IndexArgs& indexArgs)
{
    int64_t values_read = 0;
    vector<short> def_level(batchSize);
    vector<short> rep_level(batchSize);
    int64_t rows_read = 0;
    StoreType* value = new StoreType[batchSize];
    dolphindb::PluginDefer defer([&](){delete[] value;});
    while(column_reader->HasNext() && rows_read < (int64_t)batchSize){
        int64_t valuesRead = 0;
        rows_read += ReadBatchFunc(column_reader, batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, value + values_read, valuesRead);
        values_read += valuesRead;
    }
    int scale = 1;
    if(col_descr->logical_type()->is_decimal() || col_descr->converted_type()==parquet::ConvertedType::DECIMAL)
    {
        scale=std::pow(10, col_descr->type_scale());
        if (!scale)
            throw RuntimeException("Dividend scale can't be zero.");
    }
    containNull = containNull | (rows_read != values_read);
    SetVectorData<ValueType, StoreType, DDBStoreType>::set(buffer, rows_read, value, scale, def_level, containNull, col_descr);
    getIndex(rep_level, rows_read, indexArgs);
    return rows_read;
}

int convertParquetToDolphindbChar(int col_idx, std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, char* buffer, size_t batchSize, bool& containNull, IndexArgs& indexArgs)
{
    int64_t values_read = 0;
    vector<short> def_level(batchSize);
    vector<short> rep_level(batchSize);
    int64_t rows_read = 0;
    switch (col_descr->physical_type())
    {
    case parquet::Type::BOOLEAN:
    {
        return convertParquetToDolphindbInternal<bool>(col_idx, static_cast<parquet::BoolReader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::INT32:
    {
        return convertParquetToDolphindbInternal<int>(col_idx, static_cast<parquet::Int32Reader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::INT64:
    {
        return convertParquetToDolphindbInternal<int64_t>(col_idx, static_cast<parquet::Int64Reader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::INT96:
    {
        return convertParquetToDolphindbInternal<parquet::Int96>(col_idx, static_cast<parquet::Int96Reader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
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
		if (!flba_reader)
			throw RuntimeException("flba_reader can't be null");
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
        getIndex(rep_level, rows_read, indexArgs);
        return rows_read;
    }
    case parquet::Type::BYTE_ARRAY:
    {
        parquet::ByteArrayReader *ba_reader = static_cast<parquet::ByteArrayReader *>(column_reader.get());
		if (!ba_reader)
			throw RuntimeException("ba_reader can't be null");
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
        getIndex(rep_level, rows_read, indexArgs);
        return rows_read;
    }
    default:
        throw RuntimeException("unsupported data type");
    }
    return 0;
}

int convertParquetToDolphindbBool(int col_idx, std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, char* buffer, size_t batchSize, bool& containNull, IndexArgs& indexArgs)
{
	if (!col_descr)
		throw RuntimeException("col_descr can't be null");
    switch (col_descr->physical_type())
    {
    case parquet::Type::BOOLEAN:
    {
        return convertParquetToDolphindbInternal<bool, char, bool>(col_idx, static_cast<parquet::BoolReader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::INT32:
    {
        return convertParquetToDolphindbInternal<int, char, bool>(col_idx, static_cast<parquet::Int32Reader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::INT64:
    {
        return convertParquetToDolphindbInternal<int64_t, char, bool>(col_idx, static_cast<parquet::Int64Reader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::INT96:
    {
        return convertParquetToDolphindbInternal<parquet::Int96, char, bool>(col_idx, static_cast<parquet::Int96Reader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::DOUBLE:
    {
        return convertParquetToDolphindbInternal<double, char, bool>(col_idx, static_cast<parquet::DoubleReader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::FLOAT:
    {
        return convertParquetToDolphindbInternal<float, char, bool>(col_idx, static_cast<parquet::FloatReader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
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

int convertParquetToDolphindbInt(int col_idx, std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, int* buffer, DATA_TYPE times_t, size_t batchSize, bool& containNull, IndexArgs& indexArgs)
{
    int64_t values_read = 0;
    vector<short> def_level(batchSize);
    vector<short> rep_level(batchSize);
    int64_t rows_read = 0;
	if (!col_descr)
		throw RuntimeException("col_descr can't be null");
    switch (col_descr->physical_type())
    {
    case parquet::Type::BOOLEAN:
    {
        if (times_t != DT_INT)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:BOOLEAN" + "->" + Util::getDataTypeString(times_t));
        return convertParquetToDolphindbInternal<bool>(col_idx, (parquet::BoolReader *)(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::INT32:
    {
        parquet::Int32Reader *int32_reader = static_cast<parquet::Int32Reader *>(column_reader.get());
		if (!int32_reader)
			throw RuntimeException("int32_reader can't be null");
        // Read all the rows in the column
        vector<int32_t> value(batchSize);
        while(int32_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int32_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (int32_t*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }
        getIndex(rep_level, rows_read, indexArgs);
        containNull = rows_read != values_read;
        if(col_descr->logical_type()->is_decimal() || col_descr->converted_type()==parquet::ConvertedType::DECIMAL)
        {
            int scale=std::pow(10, col_descr->type_scale());
			if (!scale)
				throw RuntimeException("Dividend scale can't be zero.");
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
                    containNull = true;
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
		if (!int64_reader)
			throw RuntimeException("int64_reader can't be null");
        while(int64_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int64_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (int64_t*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }
        getIndex(rep_level, rows_read, indexArgs);
        containNull = rows_read != values_read;
        if(col_descr->logical_type()->is_decimal() || col_descr->converted_type()==parquet::ConvertedType::DECIMAL)
        {
            long long scale=std::pow(10,col_descr->type_scale());
			if (!scale)
				throw RuntimeException("Dividend scale can't be zero.");
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
                containNull = true;
                buffer[le] = INT_MIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::INT96:
    {
        vector<long long> longValue;
        parquet::Int96Reader *int96_reader = static_cast<parquet::Int96Reader *>(column_reader.get());
		if (!int96_reader)
			throw RuntimeException("int96_reader can't be null");
        vector<parquet::Int96> value(batchSize);
        while(int96_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int96_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (parquet::Int96*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }
        getIndex(rep_level, rows_read, indexArgs);
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
                    containNull = true;
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
                    containNull = true;
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
        return convertParquetToDolphindbInternal<double>(col_idx, static_cast<parquet::DoubleReader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::FLOAT:
    {
        if (times_t != DT_INT)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:FLOAT" + "->" + Util::getDataTypeString(times_t));
        return convertParquetToDolphindbInternal<float>(col_idx, static_cast<parquet::FloatReader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
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

int convertParquetToDolphindbShort(int col_idx, std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, short* buffer, size_t batchSize, bool& containNull, IndexArgs& indexArgs)
{
    vector<short> def_level(batchSize);
    vector<short> rep_level(batchSize);
	if (!col_descr)
		throw RuntimeException("col_descr can't be null");
    switch (col_descr->physical_type())
    {
    case parquet::Type::BOOLEAN:
    {
        return convertParquetToDolphindbInternal<bool>(col_idx, static_cast<parquet::BoolReader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
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
                throw RuntimeException("uncompatible type in column " + std::to_string(col_idx)  + " Parquet:" + col_descr->logical_type()->ToString() + "->" + Util::getDataTypeString(DT_SHORT));
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
                throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " Parquet:" + std::to_string(col_descr->converted_type()) + "->" + Util::getDataTypeString(DT_SHORT));
            }
        }
        return convertParquetToDolphindbInternal<int>(col_idx, static_cast<parquet::Int32Reader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
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
                throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " Parquet:" + col_descr->logical_type()->ToString() + "->" + Util::getDataTypeString(DT_SHORT));
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
                throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " Parquet:" + std::to_string(col_descr->converted_type()) + "->" + Util::getDataTypeString(DT_SHORT));
            }
        }
        return convertParquetToDolphindbInternal<int64_t>(col_idx, static_cast<parquet::Int64Reader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::INT96:
    {
        return convertParquetToDolphindbInternal<parquet::Int96>(col_idx, static_cast<parquet::Int96Reader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::DOUBLE:
    {
        return convertParquetToDolphindbInternal<double>(col_idx, static_cast<parquet::DoubleReader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::FLOAT:
    {
        return convertParquetToDolphindbInternal<float>(col_idx, static_cast<parquet::FloatReader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
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

int convertParquetToDolphindbLong(int col_idx, std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, long long* buffer, DATA_TYPE times_t, size_t batchSize, bool& containNull, IndexArgs& indexArgs)
{
    int64_t values_read = 0;
    vector<short> def_level(batchSize);
    vector<short> rep_level(batchSize);
    int64_t rows_read = 0;
	if (!col_descr)
		throw RuntimeException("col_descr can't be null");
    switch (col_descr->physical_type())
    {
    case parquet::Type::BOOLEAN:
    {
        return convertParquetToDolphindbInternal<bool>(col_idx, static_cast<parquet::BoolReader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::INT32:
    {
        parquet::Int32Reader *int32_reader = static_cast<parquet::Int32Reader *>(column_reader.get());
		if (!int32_reader)
			throw RuntimeException("int32_reader can't be null");
        // Read all the rows in the column

        vector<int32_t> value(batchSize);
        while(int32_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int32_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (int32_t*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }
        getIndex(rep_level, rows_read, indexArgs);
        containNull = rows_read != values_read;
        if(col_descr->logical_type()->is_decimal() || col_descr->converted_type()==parquet::ConvertedType::DECIMAL)
        {
            int scale=std::pow(10, col_descr->type_scale());
			if (!scale)
				throw RuntimeException("Dividend scale can't be zero.");
            for(int i = 0; i < values_read; i++)
                value[i]=value[i]/scale;
        }
        parquetTime parquetT = convertParquetTimes(col_descr->logical_type(), col_descr->converted_type());
        if (parquetT != parquetTime::None && times_t != DT_LONG)
        {
            vector<long long> longV(values_read);
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
                    containNull = true;
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
                containNull = true;
                buffer[le] = LLONG_MIN;
            }
        }
        return rows_read;
    }
    case parquet::Type::INT64:
    {

        parquet::Int64Reader *int64_reader = static_cast<parquet::Int64Reader *>(column_reader.get());
		if (!int64_reader)
			throw RuntimeException("int64_reader can't be null");
        vector<long long> value(batchSize);
        while(int64_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int64_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (int64_t*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }
        getIndex(rep_level, rows_read, indexArgs);
        containNull = rows_read != values_read;
        if(col_descr->logical_type()->is_decimal() || col_descr->converted_type()==parquet::ConvertedType::DECIMAL)
        {
            int scale=std::pow(10, col_descr->type_scale());
			if (!scale)
				throw RuntimeException("Dividend scale can't be zero.");
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
                if(value[i] < 0){
                    value[i] = LLONG_MIN;
                    containNull = true;
                }
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
                    containNull = true;
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
		if (!int96_reader)
			throw RuntimeException("int96_reader can't be null");
        vector<parquet::Int96> value(batchSize);
        while(int96_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int96_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (parquet::Int96*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }
        getIndex(rep_level, rows_read, indexArgs);
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
                    containNull = true;
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
                    containNull = true;
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
        return convertParquetToDolphindbInternal<double>(col_idx, static_cast<parquet::DoubleReader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::FLOAT:
    {
        if (times_t != DT_LONG)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:FLOAT" + "->" + Util::getDataTypeString(times_t));
        return convertParquetToDolphindbInternal<float>(col_idx, static_cast<parquet::FloatReader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
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

int convertParquetToDolphindbFloat(int col_idx, std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, float* buffer, size_t batchSize, bool& containNull, IndexArgs& indexArgs)
{
    vector<short> def_level(batchSize);
    vector<short> rep_level(batchSize);
    int64_t rows_read = 0;
	if (!col_descr)
		throw RuntimeException("col_descr can't be null");
    switch (col_descr->physical_type())
    {
    case parquet::Type::BOOLEAN:
    {
        return convertParquetToDolphindbInternal<bool>(col_idx, static_cast<parquet::BoolReader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::INT32:
    {
        return convertParquetToDolphindbInternal<int>(col_idx, static_cast<parquet::Int32Reader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::INT64:
    {
        return convertParquetToDolphindbInternal<int64_t>(col_idx, static_cast<parquet::Int64Reader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::INT96:
    {
        return convertParquetToDolphindbInternal<parquet::Int96>(col_idx, static_cast<parquet::Int96Reader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::DOUBLE:
    {
        return convertParquetToDolphindbInternal<double>(col_idx, static_cast<parquet::DoubleReader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::FLOAT:
    {
        return convertParquetToDolphindbInternal<float>(col_idx, static_cast<parquet::FloatReader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::FIXED_LEN_BYTE_ARRAY:
    {
        int fixed_length = col_descr->type_length();
        parquet::FixedLenByteArrayReader *flba_reader = static_cast<parquet::FixedLenByteArrayReader *>(column_reader.get());
		if (!flba_reader)
			throw RuntimeException("flba_reader can't be null");
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
                    containNull = true;
                }
            }
        }
        getIndex(rep_level, rows_read, indexArgs);
        return rows_read;
    }
    case parquet::Type::BYTE_ARRAY:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:BYTE_ARRAY" + "->" + Util::getDataTypeString(DT_FLOAT));

    default:
        throw RuntimeException("unsupported data type");
    }
    return 0;
}

int convertParquetToDolphindbDouble(int col_idx, std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, double* buffer, size_t batchSize, bool& containNull, IndexArgs& indexArgs)
{
    vector<short> def_level(batchSize);
    vector<short> rep_level(batchSize);
    int64_t rows_read = 0;
	if (!col_descr)
		throw RuntimeException("col_descr can't be null");
    switch (col_descr->physical_type())
    {
    case parquet::Type::BOOLEAN:
    {
        return convertParquetToDolphindbInternal<bool>(col_idx, static_cast<parquet::BoolReader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::INT32:
    {
        return convertParquetToDolphindbInternal<int>(col_idx, static_cast<parquet::Int32Reader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::INT64:
    {
        return convertParquetToDolphindbInternal<int64_t>(col_idx, static_cast<parquet::Int64Reader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::INT96:
    {
        return convertParquetToDolphindbInternal<parquet::Int96>(col_idx, static_cast<parquet::Int96Reader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::DOUBLE:
    {
        return convertParquetToDolphindbInternal<double>(col_idx, static_cast<parquet::DoubleReader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::FLOAT:
    {
        return convertParquetToDolphindbInternal<float>(col_idx, static_cast<parquet::FloatReader *>(column_reader.get()), col_descr, buffer, batchSize, containNull, indexArgs);
    }
    case parquet::Type::FIXED_LEN_BYTE_ARRAY:
    {
        int fixed_length = col_descr->type_length();
        parquet::FixedLenByteArrayReader *flba_reader = static_cast<parquet::FixedLenByteArrayReader *>(column_reader.get());
		if (!flba_reader)
			throw RuntimeException("flba_reader can't be null");
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
                    containNull = true;
                    buffer[le] = DBL_NMIN;
                }
            }
        }
        getIndex(rep_level, rows_read, indexArgs);
        return rows_read;
    }
    case parquet::Type::BYTE_ARRAY:
        throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:BYTE_ARRAY" + "->" + Util::getDataTypeString(DT_DOUBLE));

    default:
        throw RuntimeException("unsupported data type");
    }
    return 0;
}

int convertParquetToDolphindbString(int col_idx, std::shared_ptr<parquet::ColumnReader> column_reader, const parquet::ColumnDescriptor *col_descr, vector<string> &buffer, DATA_TYPE string_t, size_t batchSize, bool& containNull, IndexArgs& indexArgs)
{
    int64_t values_read = 0;
    vector<short> def_level(batchSize);
    vector<short> rep_level(batchSize);
    int64_t rows_read = 0;
	if (!col_descr)
		throw RuntimeException("col_descr can't be null");
    switch(col_descr->physical_type()){
        case parquet::Type::BOOLEAN:
        case parquet::Type::INT32:
        case parquet::Type::INT64:
        case parquet::Type::DOUBLE:
        case parquet::Type::FLOAT:{
            if(string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:" + parquet::TypeToString(col_descr->physical_type()) + "->" + Util::getDataTypeString(string_t));
        }
        default: break;
    }
    switch (col_descr->physical_type())
    {
    case parquet::Type::BOOLEAN:
    {
        if (string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:BOOLEAN" + "->" + Util::getDataTypeString(string_t));
        return convertParquetToDolphindbInternal<bool>(col_idx, static_cast<parquet::BoolReader *>(column_reader.get()), col_descr, buffer.data(), batchSize, containNull, indexArgs);
    }
    case parquet::Type::INT32:
    {
        if (string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:INT32" + "->" + Util::getDataTypeString(string_t));
        return convertParquetToDolphindbInternal<int>(col_idx, static_cast<parquet::Int32Reader *>(column_reader.get()), col_descr, buffer.data(), batchSize, containNull, indexArgs);
    }
    case parquet::Type::INT64:
    {
        if (string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:INT64" + "->" + Util::getDataTypeString(string_t));
        return convertParquetToDolphindbInternal<int64_t>(col_idx, static_cast<parquet::Int64Reader *>(column_reader.get()), col_descr, buffer.data(), batchSize, containNull, indexArgs);
    }
    case parquet::Type::INT96:
    {
        if (string_t == DT_UUID)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:INT96" + "->" + Util::getDataTypeString(string_t));
        parquet::Int96Reader *int96_reader = static_cast<parquet::Int96Reader *>(column_reader.get());
		if (!int96_reader)
			throw RuntimeException("int96_reader can't be null");
        vector<parquet::Int96> value(batchSize);
        while(int96_reader->HasNext() && rows_read < (int64_t)batchSize){
            int64_t valuesRead = 0;
            rows_read += int96_reader->ReadBatch(batchSize - rows_read, def_level.data() + rows_read, rep_level.data() + rows_read, (parquet::Int96*)value.data() + values_read, &valuesRead);
            values_read += valuesRead;
        }
        getIndex(rep_level, rows_read, indexArgs);
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
                    containNull = true;
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
                    containNull = true;
                    buffer[le] = "";
                }
            }
        }
    }
    case parquet::Type::DOUBLE:
    {
        if (string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:DOUBLE" + "->" + Util::getDataTypeString(string_t));
        return convertParquetToDolphindbInternal<double>(col_idx, static_cast<parquet::DoubleReader *>(column_reader.get()), col_descr, buffer.data(), batchSize, containNull, indexArgs);
    }
    case parquet::Type::FLOAT:
    {
        if (string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:FLOAT" + "->" + Util::getDataTypeString(string_t));
        return convertParquetToDolphindbInternal<float>(col_idx, static_cast<parquet::FloatReader *>(column_reader.get()), col_descr, buffer.data(), batchSize, containNull, indexArgs);
    }
    case parquet::Type::FIXED_LEN_BYTE_ARRAY:
    {
        if (string_t == DT_INT128)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:FIXED_LEN_BYTE_ARRAY" + "->" + Util::getDataTypeString(string_t));
        int fixed_length = col_descr->type_length();
        if (string_t == DT_UUID && fixed_length != 16)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:FIXED_LEN_BYTE_ARRAY, length NOT 16" + "->" + Util::getDataTypeString(string_t));

        parquet::FixedLenByteArrayReader *flba_reader = static_cast<parquet::FixedLenByteArrayReader *>(column_reader.get());
		if (!flba_reader)
			throw RuntimeException("flba_reader can't be null");
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
        getIndex(rep_level, rows_read, indexArgs);
        return rows_read;
    }
    case parquet::Type::BYTE_ARRAY:
    {
        if (string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException("uncompatible type in column " + std::to_string(col_idx) + " " + "Parquet:BYTE_ARRAY" + "->" + Util::getDataTypeString(string_t));
        parquet::ByteArrayReader *ba_reader = static_cast<parquet::ByteArrayReader *>(column_reader.get());
		if (!ba_reader)
			throw RuntimeException("ba_reader can't be null");
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
        getIndex(rep_level, rows_read, indexArgs);
        return rows_read;
    }
    default:
        throw RuntimeException("unsupported data type");
    }
    return 0;
}

ConstantSP loadParquetByFileName(Heap* heap, const string &filename, const ConstantSP &schema, const ConstantSP &column, const int rowGroupStart, const int rowGroupNum)
{
    ParquetReadOnlyFile file(filename);
    std::shared_ptr<parquet::FileMetaData> file_metadata = file.fileMetadataReader();
	if (!file_metadata.get())
		throw RuntimeException("fileMetadataReader failed");
    const parquet::SchemaDescriptor *s = file_metadata->schema();
	if (!s)
		throw RuntimeException("get schema failed");
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
    return loadParquetByFilePtr(&file, heap, filename, init_schema, columnToRead, rowGroupStart, rowGroupEnd);
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
	if (!file_metadata.get())
		throw RuntimeException("fileMetadataReader failed");
    const parquet::SchemaDescriptor *s = file_metadata->schema();
	if (!s)
		throw RuntimeException("get schema failed");
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
    return loadParquetByFilePtr(&file, nullptr, "", init_schema, columnToRead, rowGroupStart, rowGroupEnd);
}

ConstantSP loadParquetByFilePtr(ParquetReadOnlyFile *file, Heap *heap, string fileName, const ConstantSP &schema, const ConstantSP &column, const int rowGroupStart, const int rowGroupEnd)
{
	if (!file)
		throw RuntimeException("file can't be null");
    TableSP tableWithSchema = DBFileIO::createEmptyTableFromSchema(schema);
    int col_num = column->size();
    
    size_t totalRows = 0;
    for (int row = rowGroupStart; row < rowGroupEnd; row++){
        std::shared_ptr<parquet::RowGroupReader> row_reader = file->rowReader(row);
        if (row_reader.get() == nullptr)
            throw RuntimeException("Read parquet file failed.");
        totalRows += row_reader->metadata()->num_rows();
    }

    vector<ConstantSP> dolphindbCol(col_num);
    vector<ConstantSP> indexDolphindbCol(col_num);
    createNewVectorSP(dolphindbCol, tableWithSchema, totalRows);
    int columns = column->size();
    for(int i = 0; i < columns; ++i){
        indexDolphindbCol[i] = Util::createVector(DT_INDEX, 0, totalRows);
    }

    size_t batchRow = 1024 * 64;

    int readThreadNum = READ_THREAD_NUM;
    if(fileName.empty() || readThreadNum == 1){
        vector<int> writeOffset(col_num, 0);
        int rowCount = 0;
        for(int row = rowGroupStart; row < rowGroupEnd; ++row){
            for(int i = 0; i < col_num; ++i){
                loadParquetColumn(file, batchRow, dolphindbCol[i], row, i, column->getInt(i), writeOffset[i], rowCount, indexDolphindbCol[i]);
                writeOffset[i] += rowCount;
            }
        }
    }else{
        FunctionDefSP func = Util::createSystemFunction("pipeLoadParquet", loadParquetPloopFunc, 2, 2, false);
        ConstantSP fileNameArgs = new String(fileName);
        ConstantSP batchRowArgs = new Int(batchRow);
        vector<VectorSP> dolphindbCols;
        ConstantSP rowGroupStartArgs = new Int(rowGroupStart);
        vector<int> arrowIndexArgs;
        ConstantSP rowGroupEndArgsArgs = new Int(rowGroupEnd);
        VectorSP dolphinIndexArgs = Util::createVector(DT_INT, col_num);
        for (int i = 0; i < col_num; ++i) {
            dolphinIndexArgs->setInt(i, i);
            arrowIndexArgs.push_back(column->getInt(i));
            dolphindbCols.push_back(dolphindbCol[i]);
        }
        SmartPointer<ParquetPloopArgs> ploopArgs = new ParquetPloopArgs(fileName, batchRow, dolphindbCols, rowGroupStart, arrowIndexArgs, rowGroupEnd, indexDolphindbCol);
        vector<ConstantSP> partialArgs{ploopArgs};
        FunctionDefSP partialFunction = Util::createPartialFunction(func, partialArgs);
        int threadCount = readThreadNum == 0 ? col_num : readThreadNum; 
        int cutSize = col_num / threadCount;
        if(cutSize * threadCount < col_num) cutSize++;
        vector<ConstantSP> cutArgs{dolphinIndexArgs, new Int(cutSize)};
        dolphinIndexArgs = heap->currentSession()->getFunctionDef("cut")->call(heap, cutArgs);
        vector<ConstantSP> ploopArg{partialFunction, dolphinIndexArgs};
        heap->currentSession()->getFunctionDef("ploop")->call(heap, ploopArg);
    }
    vector<std::string> colNames;
    for(int i = 0; i < col_num; ++i){
        dolphindbCol[i]->setTemporary(true);
        colNames.push_back(tableWithSchema->getColumnName(i));
        if(tableWithSchema->getColumnType(i) >= 64){
            if(indexDolphindbCol[i]->rows() == 0){
                if(dolphindbCol[i]->rows() != 0){
                    throw RuntimeException("the size of value must be 0 when the size of index is 0. ");
                }
            }else{
                if(dolphindbCol[i]->rows() != indexDolphindbCol[i]->getIndex(indexDolphindbCol[i]->rows() - 1)){
                    throw RuntimeException("index value is not suitable for value. ");
                }
            }
            dolphindbCol[i] = new FastArrayVector(indexDolphindbCol[i], dolphindbCol[i]);
        }
    }
    return Util::createTable(colNames, dolphindbCol);
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
    string fileName = arguments[9]->getString();

    bool diskSeqMode = !db->getDatabaseDir().empty() &&
                       db->getDomain()->getPartitionType() == SEQ;
    TableSP loadedTable = loadParquetByFilePtr(file, heap, fileName, schema, columnArg, rowGroup, rowGroupEnd);
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
	if (arguments.size() < 3)
		throw RuntimeException("Arguments size can't less than three.");
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
	if (!file_metadata.get())
		throw RuntimeException("fileMetadataReader failed");
    const parquet::SchemaDescriptor *s = file_metadata->schema();
	if (!s)
		throw RuntimeException("get schema failed");
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

    vector<DistributedCallSP> tasks = generateParquetTasks(heap, f, convertedSchema, columnToRead, rowGroupStart, rowGroupNum, db, tableName, transform, filename);
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
            string physicalIndex = tableName;
            if (!DBFileIO::saveTableHeader(owner, physicalIndex, cols, partitionColumnIndices, 0, tableFile, NULL))
                throw IOException("Failed to save table header " + tableFile);
            if (!DBFileIO::saveDatabase(db.get()))
                throw IOException("Failed to save database " + db->getDatabaseDir());
            db->getDomain()->addTable(tableName, owner, physicalIndex, cols, partitionColumnIndices);
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
                                               const SystemHandleSP &db, const string &tableName, const ConstantSP &transform, const string& fileName)
{
	if (!file)
		throw RuntimeException("file can't be null");
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
    FunctionDefSP func = Util::createSystemFunction("loadFromParquetToDatabase", loadFromParquetToDatabase, 10, 10, false);
    ConstantSP fileNameArg = new String(fileName);
    //ConstantSP parquetFile=new Long((long long)file);
    for (int i = 0; i < partitions; i++)
    {
        ConstantSP rowGroupNo = new Long(rowGroupStart+i);
        ConstantSP rowGroupEnd = new Long(rowGroupStart+i+1);
        ConstantSP id = new Int(i);
        vector<ConstantSP> args{parquetFile, schema, column, rowGroupNo, rowGroupEnd, db, _tableName, id, transform, fileNameArg};
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

template<typename DDBStoreType>
void getDDBData(const VectorSP& valueCol, int offset, int size, vector<DDBStoreType>& buffer);
template<>
void getDDBData<char>(const VectorSP& valueCol, int offset, int size, vector<char>& buffer){
    valueCol->getChar(offset, size, buffer.data());
}
template<>
void getDDBData<int>(const VectorSP& valueCol, int offset, int size, vector<int>& buffer){
    valueCol->getInt(offset, size, buffer.data());
}
template<>
void getDDBData<long long>(const VectorSP& valueCol, int offset, int size, vector<long long>& buffer){
    valueCol->getLong(offset, size, buffer.data());
}
template<>
void getDDBData<float>(const VectorSP& valueCol, int offset, int size, vector<float>& buffer){
    valueCol->getFloat(offset, size, buffer.data());
}
template<>
void getDDBData<double>(const VectorSP& valueCol, int offset, int size, vector<double>& buffer){
    valueCol->getDouble(offset, size, buffer.data());
}
template<>
void getDDBData<string>(const VectorSP& valueCol, int offset, int size, vector<string>& buffer){
    vector<DolphinString*> storeBuffer;
    storeBuffer.resize(size);
    DolphinString** constPtr = valueCol->getStringConst(offset, size, storeBuffer.data());
    for(int i = 0; i < size; ++i){
        buffer[i] = (*(constPtr[i])).getString();
    }
}

template<typename ParquetType, typename DDBStoreType>
inline ParquetType convertToParquet(const DDBStoreType& value){
    return value;
}
template<>
inline parquet::ByteArray convertToParquet<parquet::ByteArray, string>(const string& value){
    parquet::ByteArray byteArray;
    byteArray.len = value.size();
    byteArray.ptr = reinterpret_cast<const uint8_t*>(value.c_str());
    return byteArray;
}


template<typename WriterType, typename ParquetType, typename DDBStoreType>
void writeToParquet(WriterType* writer, const VectorSP& valueCol, const VectorSP& IndexCol, const std::function<void(vector<DDBStoreType>&, size_t)>& transfrom){
    vector<DDBStoreType> ddbBuffer(Util::BUF_SIZE);
    vector<short> defLevel(Util::BUF_SIZE);
    vector<short> repLevel(Util::BUF_SIZE);
    vector<INDEX> IndexBuffer;
    ParquetType* buffer = new ParquetType[Util::BUF_SIZE];
    dolphindb::PluginDefer defer([&](){delete[] buffer;});
    
    const INDEX* indexPtr = nullptr;
    INDEX indexOffset = 0;
    bool isArrayVector = !IndexCol.isNull();
    if(isArrayVector){
        int rows = IndexCol->rows();
        IndexBuffer.resize(IndexCol->rows());
        indexPtr = IndexCol->getIndexConst(0, rows, IndexBuffer.data());
    }
    int valueRows = valueCol->rows();
    INDEX totalOffset = 0;
    while(totalOffset < valueRows){
        int size = std::min(valueRows - totalOffset, Util::BUF_SIZE);
        getDDBData(valueCol, totalOffset, size, ddbBuffer);
        int valueOffset = 0;
        if(transfrom){
            transfrom(ddbBuffer, size);
        }
        for(int i = 0; i < size; ++i){
            if(ddbBuffer[i] == dolphindb::getNullValue<DDBStoreType>()){
                defLevel[i] = ZERO;
            }else{
                defLevel[i] = ONE;
                buffer[valueOffset++] = convertToParquet<ParquetType, DDBStoreType>(ddbBuffer[i]);
            }
            if(isArrayVector){
                if(totalOffset + i == 0){
                    repLevel[i] = ZERO;
                }
                else if(indexOffset < static_cast<INDEX>(IndexBuffer.size()) && indexPtr[indexOffset] == totalOffset + i){
                    repLevel[i] = ZERO;
                    indexOffset++;
                }else{
                    repLevel[i] = ONE;
                }
            }
        } 
        writer->WriteBatch(size, defLevel.data(), repLevel.data(), buffer);
        totalOffset += size;
    }
}

void writeToParquetRowGroup(parquet::RowGroupWriter *rowGroupWriter, ConstantSP &table){
    int colNum = table->columns();
    for(int i = 0; i < colNum; ++i)
    {
        parquet::ColumnWriter *columnWriter = rowGroupWriter->NextColumn();
        VectorSP dolphinCol = table->getColumn(i);
        VectorSP IndexCol;
        if(dolphinCol->getType() >= 64){
            IndexCol = dynamic_cast<FastArrayVector*>(dolphinCol.get())->getSourceIndex();
            dolphinCol = dynamic_cast<FastArrayVector*>(dolphinCol.get())->getSourceValue();
        }
        if(dolphinCol->size() == 0){
            continue;
        }
        switch(dolphinCol->getType())
        {
        case DT_BOOL:
        {
            parquet::BoolWriter *parquetCol = dynamic_cast<parquet::BoolWriter*>(columnWriter);
            writeToParquet<parquet::BoolWriter, bool, char>(parquetCol, dolphinCol, IndexCol, nullptr);
            break;
        }
        case DT_CHAR:
        {
            parquet::Int32Writer *parquetCol = dynamic_cast<parquet::Int32Writer*>(columnWriter);
            writeToParquet<parquet::Int32Writer, int, char>(parquetCol, dolphinCol, IndexCol, nullptr);
            break;
        }
        case DT_SHORT:
        case DT_INT:
        case DT_DATE:
        case DT_TIME:
        {
            parquet::Int32Writer *parquetCol = dynamic_cast<parquet::Int32Writer*>(columnWriter);
            writeToParquet<parquet::Int32Writer, int, int>(parquetCol, dolphinCol, IndexCol, nullptr);
            break;
        }
        case DT_LONG:
        case DT_TIMESTAMP:
        case DT_NANOTIME:
        case DT_NANOTIMESTAMP:
        {
            parquet::Int64Writer *parquetCol = dynamic_cast<parquet::Int64Writer*>(columnWriter);
            writeToParquet<parquet::Int64Writer, int64_t, long long>(parquetCol, dolphinCol, IndexCol, nullptr);
            break;
        }
        case DT_MONTH:
        {
            parquet::Int32Writer *parquetCol = dynamic_cast<parquet::Int32Writer*>(columnWriter);
            auto transform = [](vector<int>& data, size_t size){
                for(size_t i = 0; i < size; ++i){
                    if(data[i] == dolphindb::getNullValue<int>()){
                        continue;
                    }
                    using months = std::chrono::duration<int, std::ratio<2629746> >;
                    using days = std::chrono::duration<int, std::ratio<86400> >;
                    data[i] = std::chrono::duration_cast<days>(months(data[i])).count();
                    data[i] = Util::getMonthStart(data[i] - 719514);
                }
            };
            writeToParquet<parquet::Int32Writer, int, int>(parquetCol, dolphinCol, IndexCol, transform);
            break;
        }
        case DT_MINUTE:
        {
            parquet::Int32Writer *parquetCol = dynamic_cast<parquet::Int32Writer*>(columnWriter);
            auto transform = [](vector<int>& data, size_t size){
                for(size_t i = 0; i < size; ++i){
                    if(data[i] == dolphindb::getNullValue<int>()){
                        continue;
                    }
                    data[i] = data[i] * 60 * 1000;
                }
            };
            writeToParquet<parquet::Int32Writer, int, int>(parquetCol, dolphinCol, IndexCol, transform);
            break;
        }
        case DT_SECOND:
        {
            parquet::Int32Writer *parquetCol = dynamic_cast<parquet::Int32Writer*>(columnWriter);
            auto transform = [](vector<int>& data, size_t size){
                for(size_t i = 0; i < size; ++i){
                    if(data[i] == dolphindb::getNullValue<int>()){
                        continue;
                    }
                    data[i] = data[i] * 1000;
                }
            };
            writeToParquet<parquet::Int32Writer, int, int>(parquetCol, dolphinCol, IndexCol, transform);
            break;
        }
        case DT_DATETIME:
        {
            parquet::Int64Writer *parquetCol = dynamic_cast<parquet::Int64Writer*>(columnWriter);
            auto transform = [](vector<long long>& data, size_t size){
                for(size_t i = 0; i < size; ++i){
                    if(data[i] == dolphindb::getNullValue<long long>()){
                        continue;
                    }
                    data[i] = data[i] * 1000;
                }
            };
            writeToParquet<parquet::Int64Writer, int64_t, long long>(parquetCol, dolphinCol, IndexCol, transform);
            break;
        }
        case DT_FLOAT:
        {
            parquet::FloatWriter *parquetCol = dynamic_cast<parquet::FloatWriter*>(columnWriter);
            writeToParquet<parquet::FloatWriter, float, float>(parquetCol, dolphinCol, IndexCol, nullptr);
            break;
        }
        case DT_DOUBLE:
        {
            parquet::DoubleWriter *parquetCol = dynamic_cast<parquet::DoubleWriter*>(columnWriter);
            writeToParquet<parquet::DoubleWriter, double, double>(parquetCol, dolphinCol, IndexCol, nullptr);
            break;
        }
        case DT_SYMBOL:
        case DT_STRING:
        {
            parquet::ByteArrayWriter *parquetCol = dynamic_cast<parquet::ByteArrayWriter*>(columnWriter);
            writeToParquet<parquet::ByteArrayWriter, parquet::ByteArray, string>(parquetCol, dolphinCol, IndexCol, nullptr);
            break;
        }
        default:
            throw RuntimeException("unsupported type: " + Util::getDataTypeString(dolphinCol->getType()));
        }
    }
}

ConstantSP saveParquet(ConstantSP &table, const string &filename, const string &compression)
{
    std::shared_ptr<arrow::io::FileOutputStream> outfile;
    PARQUET_ASSIGN_OR_THROW(
        outfile,
        arrow::io::FileOutputStream::Open(filename)
    );

    std::shared_ptr<parquet::schema::GroupNode> schema = getParquetSchemaFromDolphindb(table);
    parquet::WriterProperties::Builder builder;
    if (compression == "snappy") {
        builder.compression(parquet::Compression::SNAPPY);
    } else if (compression == "gzip") {
        builder.compression(parquet::Compression::GZIP);
    } else if (compression == "zstd") {
        builder.compression(parquet::Compression::ZSTD);
    }
    std::unique_ptr<parquet::ParquetFileWriter> writer = parquet::ParquetFileWriter::Open(
        outfile,
        schema,
        builder.build()
    );
    parquet::RowGroupWriter *rowGroupWriter = writer->AppendRowGroup();
    writeToParquetRowGroup(rowGroupWriter, table);
    rowGroupWriter->Close();
    writer->Close();
    outfile->Close();
    return new Void();
}

ConstantSP saveParquetHdfs(ConstantSP &table)
{
	if (table.isNull())
		throw RuntimeException("table can't be null");
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

    parquet::RowGroupWriter *rowGroupWriter = writer->AppendRowGroup();
    writeToParquetRowGroup(rowGroupWriter, table);
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
        DATA_TYPE type = table->getColumnType(i);
        bool isArray = type >= 64;
        parquet::Repetition::type parquetType = isArray ? parquet::Repetition::type::REPEATED : parquet::Repetition::type::OPTIONAL;
        type = isArray ? static_cast<DATA_TYPE>(type - 64) : type;
        switch(type)
        {
        case DT_BOOL:
            types.push_back(
                parquet::schema::PrimitiveNode::Make(
                    table->getColumnName(i),
                    parquetType,
                    nullptr,
                    parquet::Type::type::BOOLEAN
                )
            );
            break;
        case DT_CHAR:
            types.push_back(
                parquet::schema::PrimitiveNode::Make(
                    table->getColumnName(i),
                    parquetType,
                    parquet::LogicalType::Int(8, true),
                    parquet::Type::type::INT32
                )
            );
            break;
        case DT_SHORT:
            types.push_back(
                parquet::schema::PrimitiveNode::Make(
                    table->getColumnName(i),
                    parquetType,
                    parquet::LogicalType::Int(16, true),
                    parquet::Type::type::INT32
                )
            );
            break;
        case DT_INT:
            types.push_back(
                parquet::schema::PrimitiveNode::Make(
                    table->getColumnName(i),
                    parquetType,
                    parquet::LogicalType::Int(32, true),
                    parquet::Type::type::INT32
                )
            );
            break;
        case DT_LONG:
            types.push_back(
                parquet::schema::PrimitiveNode::Make(
                    table->getColumnName(i),
                    parquetType,
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
                    parquetType,
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
                    parquetType,
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
                    parquetType,
                    parquet::LogicalType::Timestamp(false, parquet::LogicalType::TimeUnit::unit::MILLIS),
                    parquet::Type::type::INT64
                )
            );
            break;
        case DT_NANOTIME:
            types.push_back(
                parquet::schema::PrimitiveNode::Make(
                    table->getColumnName(i),
                    parquetType,
                    parquet::LogicalType::Time(false, parquet::LogicalType::TimeUnit::unit::NANOS),
                    parquet::Type::type::INT64
                )
            );
            break;
        case DT_NANOTIMESTAMP:
            types.push_back(
                parquet::schema::PrimitiveNode::Make(
                    table->getColumnName(i),
                    parquetType,
                    parquet::LogicalType::Timestamp(false, parquet::LogicalType::TimeUnit::unit::NANOS),
                    parquet::Type::type::INT64
                )
            );
            break;
        case DT_FLOAT:
            types.push_back(
                parquet::schema::PrimitiveNode::Make(
                    table->getColumnName(i),
                    parquetType,
                    nullptr,
                    parquet::Type::type::FLOAT
                )
            );
            break;
        case DT_DOUBLE:
            types.push_back(
                parquet::schema::PrimitiveNode::Make(
                    table->getColumnName(i),
                    parquetType,
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
                    parquetType,
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

ConstantSP loadParquetColumn(ParquetReadOnlyFile *file, int batchRow, const VectorSP &dolphindbCol, int row,
                             int dolphinIndex, int arrowIndex, int offsetStart, int& totalRows, ConstantSP& indexCol) {

    DATA_TYPE dolphin_t = dolphindbCol->getType();
    char buf[batchRow * 8];
    long long rows_read = -1;
    std::shared_ptr<parquet::RowGroupReader> row_reader = file->rowReader(row);
    if (row_reader.get() == nullptr) throw RuntimeException("Read parquet file failed.");
    std::shared_ptr<parquet::ColumnReader> column_reader = row_reader->Column(arrowIndex);
    const parquet::ColumnDescriptor *col_descr = column_reader->descr();

    bool containNull = false;
    int offsetWrite = offsetStart;
    int capicity = row_reader->metadata()->num_rows();

    IndexArgs indexargs;
    indexargs.lastIndex_ = indexCol->rows() == 0 ? 0 : indexCol->getIndex(indexCol->rows() - 1);
    indexargs.isArray_ = col_descr->max_repetition_level() != 0;
    indexargs.index_.reserve(capicity);
    totalRows = 0;
    while (column_reader->HasNext()) {
        if(indexargs.isArray_){
            dolphindbCol->resize(offsetWrite + batchRow);
        }
        switch (dolphin_t) {
            case DT_BOOL: {
                char *ptr = dolphindbCol->getBoolBuffer(offsetWrite, batchRow, buf);
                rows_read = convertParquetToDolphindbBool(dolphinIndex, column_reader, col_descr, ptr,
                                                          batchRow, containNull, indexargs);
                dolphindbCol->setBool(offsetWrite, rows_read, ptr);
                dolphindbCol->setNullFlag(containNull);
                break;
            }
            case DT_CHAR: {
                char *ptr = dolphindbCol->getCharBuffer(offsetWrite, batchRow, buf);
                rows_read = convertParquetToDolphindbChar(dolphinIndex, column_reader, col_descr, ptr,
                                                          batchRow, containNull, indexargs);
                dolphindbCol->setChar(offsetWrite, rows_read, ptr);
                dolphindbCol->setNullFlag(containNull);
                break;
            }
            case DT_DATE:
            case DT_MONTH:
            case DT_TIME:
            case DT_SECOND:
            case DT_MINUTE:
            case DT_DATETIME:
            case DT_INT: {
                int *ptr = dolphindbCol->getIntBuffer(offsetWrite, batchRow, (int *)buf);
                rows_read = convertParquetToDolphindbInt(dolphinIndex, column_reader, col_descr, ptr,
                                                         dolphin_t, batchRow, containNull, indexargs);
                dolphindbCol->setInt(offsetWrite, rows_read, ptr);
                dolphindbCol->setNullFlag(containNull);
                break;
            }
            case DT_LONG:
            case DT_NANOTIME:
            case DT_NANOTIMESTAMP:
            case DT_TIMESTAMP: {
                long long *ptr = dolphindbCol->getLongBuffer(offsetWrite, batchRow, (long long *)buf);
                rows_read = convertParquetToDolphindbLong(dolphinIndex, column_reader, col_descr, ptr,
                                                          dolphin_t, batchRow, containNull, indexargs);
                dolphindbCol->setLong(offsetWrite, rows_read, ptr);
                dolphindbCol->setNullFlag(containNull);
                break;
            }
            case DT_SHORT: {
                short *ptr = dolphindbCol->getShortBuffer(offsetWrite, batchRow, (short *)buf);
                rows_read = convertParquetToDolphindbShort(dolphinIndex, column_reader, col_descr, ptr,
                                                           batchRow, containNull, indexargs);
                dolphindbCol->setShort(offsetWrite, rows_read, ptr);
                dolphindbCol->setNullFlag(containNull);
                break;
            }
            case DT_FLOAT: {
                float *ptr = dolphindbCol->getFloatBuffer(offsetWrite, batchRow, (float *)buf);
                rows_read = convertParquetToDolphindbFloat(dolphinIndex, column_reader, col_descr, ptr,
                                                           batchRow, containNull, indexargs);
                dolphindbCol->setFloat(offsetWrite, rows_read, ptr);
                dolphindbCol->setNullFlag(containNull);
                break;
            }
            case DT_DOUBLE: {
                double *ptr = dolphindbCol->getDoubleBuffer(offsetWrite, batchRow, (double *)buf);
                rows_read = convertParquetToDolphindbDouble(dolphinIndex, column_reader, col_descr, ptr,
                                                            batchRow, containNull, indexargs);
                dolphindbCol->setDouble(offsetWrite, rows_read, ptr);
                dolphindbCol->setNullFlag(containNull);
                break;
            }
            case DT_INT128:
            case DT_UUID:
            case DT_STRING:
            case DT_SYMBOL: {
                vector<string> buffer(batchRow);
                rows_read = convertParquetToDolphindbString(dolphinIndex, column_reader, col_descr, buffer,
                                                            dolphin_t, batchRow, containNull, indexargs);
                dolphindbCol->setString(offsetWrite, rows_read, buffer.data());
                dolphindbCol->setNullFlag(containNull);
                break;
            }
            default: throw RuntimeException("unsupported data type.");
        }
        if(indexargs.isArray_){
            dolphindbCol->resize(offsetWrite + rows_read);
        }
        offsetWrite += rows_read;
        totalRows += rows_read;
    }
    if(indexargs.cumSum_ != 0){
        indexargs.index_.push_back(indexargs.lastIndex_ + indexargs.cumSum_);
    }
    reinterpret_cast<Vector*>(indexCol.get())->appendIndex(indexargs.index_.data(), indexargs.index_.size());
    return new Bool(true);
}

ConstantSP loadParquetPloopFunc(Heap *heap, vector<ConstantSP> &arguments) {

    SmartPointer<ParquetPloopArgs> ploopArgs = arguments[0];

    string fileName = ploopArgs->getFileName();
    int batchRow = ploopArgs->getBatchRow();
    const vector<VectorSP>& dolphindbColVec = ploopArgs->getDolphindbColVec();
    vector<ConstantSP>& indexCol = ploopArgs->getDolphindbIndexColVec();
    int rowGroupStart = ploopArgs->getRowGroupStart();
    const vector<int>& arrowIndexVec = ploopArgs->getArrowIndexVec();
    ParquetReadOnlyFile file(fileName);
    int rowGroupEnd = ploopArgs->getRowGroupEnd();
    if(arguments[1]->getType() != DT_INT || (arguments[1]->getForm() != DF_VECTOR && arguments[1]->getForm() != DF_SCALAR))
        throw RuntimeException("the dolphinIndexVec for loadParquetPloopFunc must be an int or an int vector");
    ConstantSP dolphinIndexVec = arguments[1];
    int colSize = dolphinIndexVec->rows();
    int dolphindbColVecSize = dolphindbColVec.size();
    int arrowIndexVecSize = arrowIndexVec.size();
    for(int i = 0; i < colSize; ++i){
        int offsetWrite = 0;
        int rowCount = 0;
        int dolphinIndex = dolphinIndexVec->getInt(i);
        if(dolphinIndex >= dolphindbColVecSize)
            throw RuntimeException("the dolphinIndex must be less than the size of dolphindbColVec");
        if(dolphinIndex >= arrowIndexVecSize)
            throw RuntimeException("the dolphinIndex must be less than the size of arrowIndexVec");
        for (int row = rowGroupStart; row < rowGroupEnd; row++){
            loadParquetColumn(&file, batchRow, dolphindbColVec[dolphinIndex], row, dolphinIndex, arrowIndexVec[dolphinIndex], offsetWrite, rowCount, indexCol[dolphinIndex]);
            offsetWrite += rowCount;
        }
    }
    return new Bool(true);
}

} // namespace ParquetPluginImp

ConstantSP setReadThreadNum(Heap *heap, vector<ConstantSP>& arguments){
    string usage("parquet::setReadThreadNum(num) ");
    if(arguments[0]->getType() != DT_INT || arguments[0]->getForm() != DF_SCALAR)
        throw IllegalArgumentException("parquet::setReadThreadNum", usage + "num must be an int scalar. ");
    int threadNum = arguments[0]->getInt();
    if(threadNum < 0)
        throw IllegalArgumentException("parquet::setReadThreadNum", usage + "num can not be less than 0 ");
    ParquetPluginImp::READ_THREAD_NUM = threadNum;
    return new Bool(true);
}
ConstantSP getReadThreadNum(Heap *heap, vector<ConstantSP>& arguments){
    int readThreadNum = ParquetPluginImp::READ_THREAD_NUM;
    return new Int(readThreadNum);
}