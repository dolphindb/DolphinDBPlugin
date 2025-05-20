#include "orc_plugin.h"
#include "Exceptions.h"
#include "Types.h"
#include <climits>
#include <exception>
#include "ddbplugin/PluginLogger.h"
#include "ddbplugin/PluginLoggerImp.h"

ConstantSP extractORCSchema(Heap *heap, vector<ConstantSP> &arguments)
{
    ConstantSP filename = arguments[0];
    if (filename->getType() != DT_STRING)
        throw IllegalArgumentException(__FUNCTION__, "The filePath must be a string.");

    ConstantSP schema = ORCPluginImp::extractORCSchema(filename->getString());

    return schema;
}

ConstantSP loadORC(Heap *heap, vector<ConstantSP> &arguments)
{
    string usage{"orc::loadORC(filePath,[schema],[column],[rowStart],[rowNum])"};
    ConstantSP filename = arguments[0];

    int rowStart = 0;
    int rowNum = 0;
    ConstantSP schema = ORCPluginImp::nullSP;
    ConstantSP column = new Void();
    if(filename->getForm() != DF_SCALAR || filename->getType() != DT_STRING)
        throw IllegalArgumentException(__FUNCTION__, usage + "The filePath and dataset must be a string.");
    if(arguments.size() >= 2 && !arguments[1]->isNull())
    {
        if(!arguments[1]->isTable())
            throw IllegalArgumentException(__FUNCTION__, usage + "schema must be a table containing column names and types.");
        else if(!arguments[1]->isNull())
            schema = arguments[1];
    }
    if(arguments.size() >= 3 && !arguments[2]->isNull())
    {
        if(!arguments[2]->isVector() || arguments[2]->getCategory() != INTEGRAL)
            throw IllegalArgumentException(__FUNCTION__, usage + "column must be a integer vector.");
        else
            column = arguments[2];
    }
    if(arguments.size() >= 4 && !arguments[3]->isNull())
    {
        if(arguments[3]->isScalar() && arguments[3]->getCategory() == INTEGRAL)
        {
            rowStart = arguments[3]->getInt();
            if(rowStart < 0)
                throw IllegalArgumentException(__FUNCTION__, usage + "rowStart must be positive.");
        }
        else
            throw IllegalArgumentException(__FUNCTION__, usage + "rowStart must be an integer.");
    }
    if(arguments.size() >= 5 && !arguments[4]->isNull())
    {
        if(arguments[4]->isScalar() && arguments[4]->getCategory() == INTEGRAL)
        {
            rowNum = arguments[4]->getInt();
            if(rowNum < 0)
                throw IllegalArgumentException(__FUNCTION__, usage + "rowNum must be a positive.");
        }
        else
            throw IllegalArgumentException(__FUNCTION__, usage + "rowNum must be an integer.");
    }
    return ORCPluginImp::loadORC(filename->getString(), schema, column, rowStart, rowNum);
}

ConstantSP loadORCHdfs(Heap *heap, vector<ConstantSP> &arguments)
{
    string usage{"orc::loadORCHdfs(address, length)"};
    if(arguments[0]->getType() != DT_RESOURCE || arguments[0]->getString() != "hdfs readFile address")
        throw IllegalArgumentException(__FUNCTION__,"The first arguments should be resource");
    if(arguments[1]->getType() != DT_RESOURCE || arguments[1]->getString() != "hdfs readFile length")
        throw IllegalArgumentException(__FUNCTION__,"The second arguments should be resource");

    void *buffer = (void *)arguments[0]->getLong();
    uint64_t *length = (uint64_t *)arguments[1]->getLong();
    return ORCPluginImp::loadORCFromBuf(buffer,*length);
}

ConstantSP loadORCEx(Heap *heap, vector<ConstantSP> &arguments)
{
    string usage = "orc::loadORCEx(dbHandle,tableName,[partitionColumns],filePath,[schema],[column],[rowStart],[rowNum],[transform])";
    ConstantSP db = arguments[0];
    ConstantSP tableName = arguments[1];
    ConstantSP partitionColumnNames = arguments[2];
    ConstantSP filename = arguments[3];

    int rowStart = 0;
    int rowNum = 0;
    ConstantSP schema = ORCPluginImp::nullSP;
    ConstantSP column = new Void();

    if (!db->isDatabase())
        throw IllegalArgumentException(__FUNCTION__, usage + "dbHandle must be a database handle.");
    if (!tableName->isScalar() || tableName->getType() != DT_STRING)
        throw IllegalArgumentException(__FUNCTION__, usage + "tableName must be a string scalar.");
    if (!partitionColumnNames->isNull() && (partitionColumnNames->getType() != DT_STRING || (partitionColumnNames->getForm() != DF_SCALAR && partitionColumnNames->getForm() != DF_VECTOR)))
        throw IllegalArgumentException(__FUNCTION__, usage + "The partition columns must be in string or string vector.");
    if (!filename->isScalar() || filename->getType() != DT_STRING)
        throw IllegalArgumentException(__FUNCTION__, usage + "The filePath must be a string scalar.");
    if (arguments.size() >= 5 && !arguments[4]->isNull())
    {
        if (!arguments[4]->isTable())
            throw IllegalArgumentException(__FUNCTION__, usage + "schema must be a table containing column names and types.");
        else
            schema = arguments[4];
    }
    if (arguments.size() >= 6 && !arguments[5]->isNull())
    {
        if (!arguments[5]->isVector() || arguments[5]->getCategory() != INTEGRAL)
            throw IllegalArgumentException(__FUNCTION__, usage + "column must be a integer vector");
        else
            column = arguments[5];
    }
    if (arguments.size() >= 7 && !arguments[6]->isNull())
    {
        if (arguments[6]->isScalar() && arguments[6]->getCategory() == INTEGRAL){
            rowStart = arguments[6]->getInt();
            if (rowStart < 0)
                throw IllegalArgumentException(__FUNCTION__, usage + "rowStart must be a non-negative integer.");
        }
        else
            throw IllegalArgumentException(__FUNCTION__, usage + "rowStart must be an integer.");
    }
    if (arguments.size() >= 8 && !arguments[7]->isNull())
    {
        if (arguments[7]->isScalar() && arguments[7]->getCategory() == INTEGRAL){
            rowNum = arguments[7]->getInt();
            if (rowNum <= 0)
                throw IllegalArgumentException(__FUNCTION__, usage + "rowNum must be a non-negative integer.");
        }
        else
            throw IllegalArgumentException(__FUNCTION__, usage + "rowNum must be an integer.");
    }
    ConstantSP transform;
    if (arguments.size() >= 9 && !arguments[8]->isNull()) {
        if (!arguments[8]->isScalar() || arguments[8]->getType() != DT_FUNCTIONDEF)
            throw IllegalArgumentException(__FUNCTION__, usage + "transform must be a function.");
        transform = arguments[8];
    }
    else
        transform = new Void();
    return ORCPluginImp::loadORCEx(heap, db, tableName->getString(), partitionColumnNames,
                                           filename->getString(),
                                           schema, column, rowStart, rowNum, transform);

}

ConstantSP orcDS(Heap *heap, vector<ConstantSP> &arguments)
{
    string usage{"orc::orcDS(filePath,chunkSize,[schema],[skipRows])"};
    ConstantSP filename = arguments[0];
    ConstantSP chunkSize = arguments[1];
    int skipRows = 0;
    ConstantSP schema = ORCPluginImp::nullSP;
    if(!filename->isScalar() || filename->getType() != DT_STRING)
        throw IllegalArgumentException(__FUNCTION__, usage + "filePath must be a string vector.");
    if(chunkSize->isNull() || !chunkSize->isScalar() || chunkSize->getCategory() != INTEGRAL || chunkSize->getInt() <= 0){
        throw IllegalArgumentException(__FUNCTION__, usage + "chunkSize must be a positive integer");
    }
    if(arguments.size() >= 3 && !arguments[2]->isNull())
    {
        if(!arguments[2]->isTable())
            throw IllegalArgumentException(__FUNCTION__, usage + "schema must be a table containing column names and types.");
        else
            schema = arguments[2];
    }
    if(arguments.size() >= 4 && !arguments[3]->isNull())
    {
        if (arguments[3]->isScalar() && arguments[3]->getCategory() == INTEGRAL){
            skipRows = arguments[3]->getInt();
            if (skipRows < 0)
                throw IllegalArgumentException(__FUNCTION__, usage + "skipRows must be a non-negative integer.");
        }
    }
    return ORCPluginImp::orcDS(filename, chunkSize->getInt(), schema, skipRows);
}

ConstantSP saveORC(Heap *heap, vector<ConstantSP> &arguments){

    string usage{"`orc::saveORC(table, filePath)"};
    TableSP table = arguments[0];
    ConstantSP fileName = arguments[1];
    if(table->getForm() != DF_TABLE){
        throw IllegalArgumentException(__FUNCTION__, "table must be a table type.");
    }
    if(!fileName->isScalar() || fileName->getType() != DT_STRING){
        throw IllegalArgumentException(__FUNCTION__, "filePath must be a string scalar.");
    }
    return ORCPluginImp::saveORC(table, fileName->getString());
}

namespace ORCPluginImp
{

ConstantSP nullSP = Util::createNullConstant(DT_VOID);
const string STR_MIN = "";

std::string getColumnType(const orc::Type *subtype)
{
    switch(subtype->getKind())
    {
    case orc::TypeKind::BOOLEAN:
        return "BOOL";
    case orc::TypeKind::BYTE:
        return "CHAR";
    case orc::TypeKind::SHORT:
        return "SHORT";
    case orc::TypeKind::INT:
        return "INT";
    case orc::TypeKind::LONG:
        return "LONG";
    case orc::TypeKind::FLOAT:
        return "FLOAT";
    case orc::TypeKind::DOUBLE:
        return "DOUBLE";
    case orc::TypeKind::STRING:
    case orc::TypeKind::CHAR:
    case orc::TypeKind::VARCHAR:
        return "STRING";
    case orc::TypeKind::TIMESTAMP:
        return "NANOTIMESTAMP";
    case orc::TypeKind::DATE:
        return "DATE";
    default:
        throw RuntimeException(ORC_PREFIX + "unsupported data type");
    }
}
bool getSchemaCol(const orc::Type &schema_descr, const ConstantSP &col_idx, vector<ConstantSP> &dolphindbCol)
{
    if(dolphindbCol.size() != 2)
        return false;
    int col_num = col_idx->size();
    if(col_num == 0)
        return false;
    dolphindbCol[0] = Util::createVector(DT_STRING, col_num, col_num);
    dolphindbCol[1] = Util::createVector(DT_STRING, col_num, col_num);
    for (int i = 0; i < col_num; ++i)
    {
        string name = schema_descr.getFieldName(col_idx->getInt(i));
        if(!Util::isVariableCandidate(name))
            // throw RuntimeException(ORC_PREFIX + "column name " + name + " is invalid.");
            name = "Z" + name;
        string type = getColumnType(schema_descr.getSubtype(col_idx->getInt(i)));
        ConstantSP col_name = new String(name);
        ConstantSP col_type = new String(type);

        dolphindbCol[0]->set(i, col_name);
        dolphindbCol[1]->set(i, col_type);
    }
    return true;
}
TableSP extractORCSchema(const string &filename)
{
    ORC_UNIQUE_PTR<orc::Reader> reader = orc::createReader(
        orc::readLocalFile(filename),
        orc::ReaderOptions()
    );
    const orc::Type &schema_descr = reader->getType();
    int col_num = schema_descr.getSubtypeCount();
    vector<ConstantSP> cols(2);
    ConstantSP col_idx = Util::createIndexVector(0, col_num);
    if(!getSchemaCol(schema_descr, col_idx, cols))
        throw RuntimeException(ORC_PREFIX + "get schema failed");
    vector<string> colNames(2);
    colNames[0] = "name";
    colNames[1] = "type";
    return Util::createTable(colNames, cols);
}
void createNewVectorSP(vector<VectorSP> &dolphindb_v, const TableSP &tb)
{
    int col_num = dolphindb_v.size();
    if(col_num != tb->columns())
        throw RuntimeException(ORC_PREFIX + "schema and column are not match.");
    for(int i = 0; i < col_num; i++)
    {
        dolphindb_v[i] = Util::createVector(tb->getColumnType(i), 0);
    }
}

TableSP appendColumnVecToTable(TableSP tb, vector<VectorSP> &colVec)
{
    if(tb->isNull())
        return tb;
    string errMsg;
    INDEX insertedRows = 0;
    vector<ConstantSP> cols(colVec.size());
    int col_num = colVec.size();
    for(int i = 0; i < col_num; i++)
    {
        cols[i] = {colVec[i]};
    }
    if(!tb->append(cols, insertedRows, errMsg))
        throw TableRuntimeException(ORC_PREFIX + errMsg);
    return tb;
}
bool convertToDTnanotime(orc::TypeKind type, vector<long long> &buffer)
{
    tm gmtBuf;
    switch(type)
    {
    case orc::TypeKind::TIMESTAMP:
        for(auto &x : buffer)
        {
            if(x == INT64_MIN) {
                continue;
            }
            time_t ts = x / 1000000000;
            tm *gmt = gmtime_r(&ts, &gmtBuf);
            x = (gmt == nullptr) ? 0 : ((gmt->tm_hour * 60 + gmt->tm_min) * 60 + gmt->tm_sec) * 1000000000 + x % 1000000000;
        }
        return true;
    case orc::TypeKind::DATE:
        return true;
    default:
        return false;
    }
}
bool convertToDTnanotimestamp(orc::TypeKind type, vector<long long> &buffer)
{
    switch(type)
    {
    case orc::TypeKind::TIMESTAMP:
        return true;
    case orc::TypeKind::DATE:
        for(auto &x : buffer) {
            if(x == INT64_MIN) {
                continue;
            }
            x = x * 24 * 60 * 60 * 1000 * 1000000;
        }
        return true;
    default:
        return false;
    }
}
bool convertToDTtimestamp(orc::TypeKind type, vector<long long> &buffer)
{
    switch(type)
    {
    case orc::TypeKind::TIMESTAMP:
        for(auto &x : buffer) {
            if(x == INT64_MIN) {
                continue;
            }
            x = x / 1000000;
        }
        return true;
    case orc::TypeKind::DATE:
        for(auto &x : buffer) {
            if(x == INT64_MIN) {
                continue;
            }
            x = x * 24 * 60 * 60 * 1000;
        }
        return true;
    default:
        return false;
    }
}
bool convertToDTdate(orc::TypeKind type, vector<int> &buffer)
{
    tm gmtBuf;
    switch(type)
    {
    case orc::TypeKind::TIMESTAMP:
        for(auto &x : buffer)
        {
            if(x == INT32_MIN) {
                continue;
            }
            time_t ts = x / 1000;
            tm *gmt = gmtime_r(&ts, &gmtBuf);
            x = (gmt == nullptr) ? 0 : Util::countDays(gmt->tm_year + 1900, gmt->tm_mon + 1, gmt->tm_mday);
        }
        return true;
    case orc::TypeKind::DATE:
        return true;
    default:
        return false;
    }
}
bool convertToDTmonth(orc::TypeKind type, vector<int> &buffer)
{
    tm gmtBuf;
    switch(type)
    {
    case orc::TypeKind::TIMESTAMP:
        for(auto &x : buffer)
        {
            if(x == INT32_MIN) {
                continue;
            }
            time_t ts = x / 1000;
            tm *gmt = gmtime_r(&ts, &gmtBuf);
            x = (gmt == nullptr) ? 0 : (gmt->tm_year + 1900) * 12 + gmt->tm_mon;
        }
        return true;
    case orc::TypeKind::DATE:
        for(auto &x : buffer)
        {
            if(x == INT32_MIN) {
                continue;
            }
            time_t ts = x * 24 * 60 * 60;
            tm *gmt = gmtime_r(&ts, &gmtBuf);
            x = (gmt == nullptr) ? 0 : (gmt->tm_year + 1900) * 12 + gmt->tm_mon;
        }
        return true;
    default:
        return false;
    }
}
bool convertToDTtime(orc::TypeKind type, vector<int> &buffer)
{
    tm gmtBuf;
    switch(type)
    {
    case orc::TypeKind::TIMESTAMP:
        for(auto &x : buffer)
        {
            if(x == INT32_MIN) {
                continue;
            }
            time_t ts = x / 1000;
            tm *gmt = gmtime_r(&ts, &gmtBuf);
            x = (gmt == nullptr) ? 0 : ((gmt->tm_hour * 60 + gmt->tm_min) * 60 + gmt->tm_sec) * 1000 + (x / 1000) % 1000;
        }
        return true;
    case orc::TypeKind::DATE:
        for(auto &x : buffer)
        {
            x = 0;
        }
        return true;
    default:
        return false;
    }
}
bool convertToDTsecond(orc::TypeKind type, vector<int> &buffer)
{
    tm gmtBuf;
    switch(type)
    {
    case orc::TypeKind::TIMESTAMP:
        for(auto &x : buffer)
        {
            if(x == INT32_MIN) {
                continue;
            }
            time_t ts = x / 1000;
            tm *gmt = gmtime_r(&ts, &gmtBuf);
            x = (gmt == nullptr) ? 0 : (gmt->tm_hour * 60 + gmt->tm_min) * 60 + gmt->tm_sec;
        }
        return true;
    case orc::TypeKind::DATE:
        for(auto &x : buffer)
        {
            x = 0;
        }
        return true;
    default:
        return false;
    }
}
bool convertToDTminute(orc::TypeKind type, vector<int> &buffer)
{
    tm gmtBuf;
    switch(type)
    {
    case orc::TypeKind::TIMESTAMP:
        for(auto &x : buffer)
        {
            if(x == INT32_MIN) {
                continue;
            }
            time_t ts = x / 1000;
            tm *gmt = gmtime_r(&ts, &gmtBuf);
            x = (gmt == nullptr) ? 0 : gmt->tm_hour * 60 + gmt->tm_min;
        }
        return true;
    case orc::TypeKind::DATE:
        for(auto &x : buffer)
        {
            x = 0;
        }
        return true;
    default:
        return false;
    }
}
bool convertToDTdatetime(orc::TypeKind type, vector<int> &buffer)
{
    switch(type)
    {
    case orc::TypeKind::TIMESTAMP:
        for(auto &x : buffer)
        {
            if(x == INT32_MIN) {
                continue;
            }
            x = x / 1000;
        }
        return true;
    case orc::TypeKind::DATE:
        for(auto &x : buffer)
        {
            if(x == INT32_MIN) {
                continue;
            }
            x = x * 24 * 60 * 60;
        }
        return true;
    default:
        return false;
    }
}

int parseEnglishMonth(char first, char second, char third){
	if(first=='J' || first=='j'){
		if(second=='a' || second=='A')
			return 1;
		else if(third=='N' || third=='n')
			return 6;
		else
			return 7;
	}
	else if(first=='F' || first=='f')
		return 2;
	else if(first=='M' || first=='m'){
		if(third=='R' || third=='r')
			return 3;
		else
			return 5;
	}
	else if(first=='A' || first=='a'){
		if(second=='P' || second=='p')
			return 4;
		else
			return 8;
	}
	else if(first=='S' || first=='s')
		return 9;
	else if(first=='O' || first=='o')
		return 10;
	else if(first=='N' || first=='n')
		return 11;
	else if(first=='D' || first=='d')
		return 12;
	else
		return 0;
}
bool parsePartialDate(const string &str, bool containDelimiter, int& part1, int& part2)
{
	if(str.length()<3)
		return false;
	unsigned start=0;
	if(Util::isLetter(str[0])){
		part1=parseEnglishMonth(str[0],str[1],str[2]);
		if(part1==0)
			return false;
		start=containDelimiter?4:3;
	}
	else{
		part1=str[0]-'0';
		if(Util::isDigit(str[1])){
			part1=part1*10+str[1]-'0';
			start=containDelimiter?3:2;
		}
		else
			start=2;
	}

	part2=0;
	while(start<str.length())
		part2=part2*10+str[start++]-'0';
	return true;
}
bool dateParser(const string &str, int &intVal)
{
	intVal=INT_MIN;
	if(str.length()<6)
		return false;
	int year=0,month=0,day=0;
    //year in the first
    year=(str[0]-'0')*10+str[1]-'0';
    if(Util::isDateDelimitor(str[2])){
        //date=yy-m-d yy-mm-dd
        if(year<20)
            year+=2000;
        else
            year+=1900;
        parsePartialDate(str.substr(3), true, month, day);
    }
    else if(str.length()==6){
        //date=yymmdd
        if(year<20)
            year+=2000;
        else
            year+=1900;
        month=(str[2]-'0')*10+str[3]-'0';
        day=(str[4]-'0')*10+str[5]-'0';
    }
    else{
        year=year*100+(str[2]-'0')*10+str[3]-'0';
        if(Util::isDateDelimitor(str[4]))
            parsePartialDate(str.substr(5),true,month,day);
        else
            parsePartialDate(str.substr(4),false,month,day);
    }
	intVal = Util::countDays(year,month,day);
    return true;
}
bool monthParser(const string &str, int &intVal){
	intVal=INT_MIN;
	if(str.length()<7)
		return false;
	int year,month;
	year=(str[0]-'0')*1000+(str[1]-'0')*100+(str[2]-'0')*10+str[3]-'0';
	if(str[4]!='.')
		return false;
	month=(str[5]-'0')*10+str[6]-'0';
	if(month>12)
		return false;
	intVal=year*12+month-1;
    return true;
}
bool timeParser(const string &str, int &intVal){
	intVal=INT_MIN;
	if(str.length() != 12)
		return false;
	int hour, minute, second, millisecond=0;
	hour=(str[0]-'0')*10+str[1]-'0';
	minute=(str[3]-'0')*10+str[4]-'0';
	second=(str[6]-'0')*10+str[7]-'0';
	if(hour>=24|| minute>=60 || second>=60)
		return false;
	millisecond = stoi(str.substr(9));
	intVal=((hour*60+minute)*60+second)*1000 + millisecond;
    return true;
}
bool secondParser(const string &str, int &intVal){
	intVal=INT_MIN;
	if(str.length()<7)
		return false;
	int hour,minute,second;
	if(str.length() == 8){
		hour=(str[0]-'0')*10+str[1]-'0';
		minute=(str[3]-'0')*10+str[4]-'0';
		second=(str[6]-'0')*10+str[7]-'0';
	}
	else{
		hour=(str[0]-'0');
		minute=(str[2]-'0')*10+str[3]-'0';
		second=(str[5]-'0')*10+str[6]-'0';
	}
	if(hour>=24|| minute>=60 || second>=60)
		return false;
	intVal=(hour*60+minute)*60+second;
    return true;
}
bool minuteParser(const string &str, int &intVal){
	intVal=INT_MIN;
	if(str.length()<5)
		return false;
	int hour,minute;
	hour=(str[0]-'0')*10+str[1]-'0';
	minute=(str[3]-'0')*10+str[4]-'0';
	if(hour>=24 || minute>=60)
		return false;
	intVal=hour*60+minute;
    return true;
}
bool datetimeParser(const string &str, int &intVal){
	intVal=INT_MIN;
	if(str.length()<15)
		return false;
	int start=str.length()-8;
	while(start>=0 && (str[start]!=' ' && str[start]!='T')) --start;
	if(start<0)
		return false;
	int end=start-1;
	while(end>=0 && (str[end]==' ' || str[end]=='T')) --end;
	if(end<0)
		return false;
    dateParser(str.substr(0, end+1), intVal);
	if(intVal==INT_MIN)
		return false;

	int hour,minute,second;
    string t = str.substr(start + 1);
	hour=(t[0]-'0')*10+t[1]-'0';
	minute=(t[3]-'0')*10+t[4]-'0';
	second=(t[6]-'0')*10+t[7]-'0';
	if(hour>=24|| minute>=60 || second>=60){
		intVal=INT_MIN;
		return false;
	}
	intVal=intVal*86400+(hour*60+minute)*60+second;
    return true;
}
bool nanotimeParser(const string &str, long long &longVal){
	longVal = LLONG_MIN;
	if(str.length() == 15 && str[2] != ':'){
		int hour, minute, second, nanosecond=0;
		hour=(str[0]-'0')*10+str[1]-'0';
		minute=(str[2]-'0')*10+str[3]-'0';
		second=(str[4]-'0')*10+str[5]-'0';
		for(int i=6; i<15; ++i)
			nanosecond = nanosecond * 10 + str[i] - '0';
		if(hour>=24|| minute>=60 || second>=60)
			return false;
		longVal=((hour*60+minute)*60+second)*1000000000ll + nanosecond;
	}
	else if(str.length() == 15 || str.length() == 18){
		int hour, minute, second, nanosecond=0;
		hour=(str[0]-'0')*10+str[1]-'0';
		minute=(str[3]-'0')*10+str[4]-'0';
		second=(str[6]-'0')*10+str[7]-'0';
		if(hour>=24|| minute>=60 || second>=60)
			return false;

        nanosecond = stoi(str.substr(9));
		if(str.length() == 15)
			nanosecond *= 1000;
		longVal=((hour*60+minute)*60+second)*1000000000ll + nanosecond;
	}
    return true;
}
bool nanotimestampParser(const string &str, long long &longVal){
	longVal=LLONG_MIN;
	if(str.length()<22)
		return false;

	int dateLen = str.length() - 16;
	if(str[dateLen]!=' ' && str[dateLen] != 'T'){
		dateLen -= 3;
		if(str[dateLen]!=' ' && str[dateLen] != 'T')
			return false;
	}
    int intVal;
    dateParser(str.substr(0, dateLen), intVal);
	if(intVal==INT_MIN){
		longVal=LLONG_MIN;
		return false;
	}

	int hour, minute, second, nanosecond=0;
    string t = str.substr(dateLen + 1);
	hour=(t[0]-'0')*10+t[1]-'0';
	minute=(t[3]-'0')*10+t[4]-'0';
	second=(t[6]-'0')*10+t[7]-'0';
	if(hour>=24|| minute>=60 || second>=60){
		longVal=LLONG_MIN;
		return false;
	}
    nanosecond = stoi(t.substr(9));
	if(t.length() - dateLen == 16)
		nanosecond *= 1000;
	longVal=intVal*86400000000000ll+((hour*60+minute)*60+second)*1000000000ll + nanosecond;
    return true;
}
bool timestampParser(const string &str, long long &longVal){
	longVal=LLONG_MIN;
	if(str.length()<15)
		return false;
	int start=str.length()-8;
	while(start>=0 && (str[start]!=' ' && str[start]!='T')) --start;
	if(start<0)
		return false;
	int end=start-1;
	while(end>=0 && (str[end]==' ' || str[end]=='T')) --end;
	if(end<0)
		return false;
    int intVal;
    dateParser(str.substr(0, end + 1), intVal);
	if(intVal==INT_MIN){
		longVal=LLONG_MIN;
		return false;
	}

	int hour,minute,second,millisecond=0;
    string t = str.substr(start + 1);
	hour=(t[0]-'0')*10+t[1]-'0';
	minute=(t[3]-'0')*10+t[4]-'0';
	second=(t[6]-'0')*10+t[7]-'0';
	if(hour>=24|| minute>=60 || second>=60){
		longVal=LLONG_MIN;
		return false;
	}
	if(t[8]=='.')
        millisecond = stoi(t.substr(9));
	longVal=intVal*86400000ll+((hour*60+minute)*60+second)*1000+millisecond;
    return true;
}

bool convertORCToDolphindbBool(int col_idx, orc::StructVectorBatch *root, orc::TypeKind type, vector<char> &buffer)
{
    switch(type)
    {
    case orc::TypeKind::BOOLEAN:
    case orc::TypeKind::BYTE:
    case orc::TypeKind::SHORT:
    case orc::TypeKind::INT:
    case orc::TypeKind::LONG:
    {
        orc::LongVectorBatch *col_batch = dynamic_cast<orc::LongVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(CHAR_MIN);
            }
            else
            {
                int64_t value = col_batch->data[r];
                buffer.push_back(static_cast<char>(value != 0));
            }
        }
        break;
    }
    case orc::TypeKind::FLOAT:
    case orc::TypeKind::DOUBLE:
    {
        orc::DoubleVectorBatch *col_batch = dynamic_cast<orc::DoubleVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(CHAR_MIN);
            }
            else
            {
                double value = col_batch->data[r];
                buffer.push_back(static_cast<char>(value != 0));
            }
        }
        break;
    }
    case orc::TypeKind::STRING:
    case orc::TypeKind::CHAR:
    case orc::TypeKind::VARCHAR:
    {
        orc::StringVectorBatch *col_batch = dynamic_cast<orc::StringVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(CHAR_MIN);
            }
            else
            {
                int64_t len = col_batch->length[r];
                char *data = col_batch->data[r];
                string str(data, len);
                if(str == "true")
                    buffer.push_back(1);
                else if(str == "false")
                    buffer.push_back(0);
                else
                    throw RuntimeException(ORC_PREFIX + "value of boolean type must be \"true\" or \"false\".");
            }
        }
        break;
    }
    case orc::TypeKind::TIMESTAMP:
        throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::TIMESTAMP->" + Util::getDataTypeString(DT_BOOL));
    case orc::TypeKind::DATE:
        throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::DATE->" + Util::getDataTypeString(DT_BOOL));
    default:
        throw RuntimeException(ORC_PREFIX + "unsupported data type.");
    }
    return true;
}
bool convertORCToDolphindbChar(int col_idx, orc::StructVectorBatch *root, orc::TypeKind type, vector<char> &buffer)
{
    switch(type)
    {
    case orc::TypeKind::BOOLEAN:
        throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::BOOLEAN->" + Util::getDataTypeString(DT_CHAR));
    case orc::TypeKind::BYTE:
    {
        orc::LongVectorBatch *col_batch = dynamic_cast<orc::LongVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(CHAR_MIN);
            }
            else
            {
                int64_t value = col_batch->data[r];
                buffer.push_back(static_cast<char>(value));
            }
        }
        break;
    }
    case orc::TypeKind::SHORT:
        throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::SHORT->" + Util::getDataTypeString(DT_CHAR));
    case orc::TypeKind::INT:
        throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::INT->" + Util::getDataTypeString(DT_CHAR));
    case orc::TypeKind::LONG:
        throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::LONG->" + Util::getDataTypeString(DT_CHAR));
    case orc::TypeKind::FLOAT:
        throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::FLOAT->" + Util::getDataTypeString(DT_CHAR));
    case orc::TypeKind::DOUBLE:
        throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::DOUBLE->" + Util::getDataTypeString(DT_CHAR));
    case orc::TypeKind::STRING:
    {
        orc::StringVectorBatch *col_batch = dynamic_cast<orc::StringVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            int64_t len = col_batch->length[r];
            if(len > 1)
                throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::STRING(length>1)->" + Util::getDataTypeString(DT_CHAR));
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(CHAR_MIN);
            }
            else
            {
                char value = col_batch->data[r][0];
                buffer.push_back(value);
            }
        }
        break;
    }
    case orc::TypeKind::CHAR:
    {
        orc::StringVectorBatch *col_batch = dynamic_cast<orc::StringVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            int64_t len = col_batch->length[r];
            if(len > 1)
                throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::CHAR(length>1)->" + Util::getDataTypeString(DT_CHAR));
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(CHAR_MIN);
            }
            else
            {
                char value = col_batch->data[r][0];
                buffer.push_back(value);
            }
        }
        break;
    }
    case orc::TypeKind::VARCHAR:
    {
        orc::StringVectorBatch *col_batch = dynamic_cast<orc::StringVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            int64_t len = col_batch->length[r];
            if(len > 1)
                throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::VARCHAR(length>1)->" + Util::getDataTypeString(DT_CHAR));
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(CHAR_MIN);
            }
            else
            {
                char value = col_batch->data[r][0];
                buffer.push_back(value);
            }
        }
        break;
    }
    case orc::TypeKind::TIMESTAMP:
        throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::TIMESTAMP->" + Util::getDataTypeString(DT_CHAR));
    case orc::TypeKind::DATE:
        throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::DATE->" + Util::getDataTypeString(DT_CHAR));
    default:
        throw RuntimeException(ORC_PREFIX + "unsupported data type.");
    }
    return true;
}
bool convertORCToDolphindbInt(int col_idx, orc::StructVectorBatch *root, orc::TypeKind type, vector<int> &buffer, DATA_TYPE times_t)
{
    switch(type)
    {
    case orc::TypeKind::BOOLEAN:
    {
        if(times_t != DT_INT)
            throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::BOOLEAN->" + Util::getDataTypeString(times_t));
        orc::LongVectorBatch *col_batch = dynamic_cast<orc::LongVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(INT32_MIN);
            }
            else
            {
                int64_t value = col_batch->data[r];
                buffer.push_back(static_cast<int>(value));
            }

        }
        break;
    }
    case orc::TypeKind::BYTE:
    {
        if(times_t != DT_INT)
            throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::BYTE->" + Util::getDataTypeString(times_t));
        orc::LongVectorBatch *col_batch = dynamic_cast<orc::LongVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(INT32_MIN);
            }
            else
            {
                int64_t value = col_batch->data[r];
                buffer.push_back(static_cast<int>(value));
            }
        }
        break;
    }
    case orc::TypeKind::SHORT:
    {
        if(times_t != DT_INT)
            throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::SHORT->" + Util::getDataTypeString(times_t));
        orc::LongVectorBatch *col_batch = dynamic_cast<orc::LongVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(INT32_MIN);
            }
            else
            {
                int64_t value = col_batch->data[r];
                buffer.push_back(static_cast<int>(value));
            }
        }
        break;
    }
    case orc::TypeKind::INT:
    {
        orc::LongVectorBatch *col_batch = dynamic_cast<orc::LongVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(INT32_MIN);
            }
            else
            {
                int64_t value = col_batch->data[r];
                buffer.push_back(static_cast<int>(value));
            }
        }
        break;
    }
    case orc::TypeKind::LONG:
    {
        orc::LongVectorBatch *col_batch = dynamic_cast<orc::LongVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            int64_t value = col_batch->data[r];
            if(col_batch->hasNulls && !col_batch->notNull[r])
                buffer.push_back(INT32_MIN);
            else if(value == INT64_MIN)
                buffer.push_back(INT32_MIN);
            else if(value >= INT32_MAX)
                buffer.push_back(INT32_MAX);
            else if(value <= INT32_MIN + 1)
                buffer.push_back(INT32_MIN + 1);
            else
                buffer.push_back(static_cast<int>(value));
        }
        break;
    }
    case orc::TypeKind::FLOAT:
    {
        if(times_t != DT_INT)
            throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::FLOAT->" + Util::getDataTypeString(times_t));
        orc::DoubleVectorBatch *col_batch = dynamic_cast<orc::DoubleVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            double value = col_batch->data[r];
            if(col_batch->hasNulls && !col_batch->notNull[r])
                buffer.push_back(INT32_MIN);
            else if(value >= INT32_MAX)
                buffer.push_back(INT32_MAX);
            else if(value <= INT32_MIN + 1)
                buffer.push_back(INT32_MIN + 1);
            else
            {
                int v = value >= 0 ? (value + 0.5) : (value - 0.5);
                buffer.push_back(v);
            }
        }
        break;
    }
    case orc::TypeKind::DOUBLE:
    {
        if(times_t != DT_INT)
            throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::DOUBLE->" + Util::getDataTypeString(times_t));
        orc::DoubleVectorBatch *col_batch = dynamic_cast<orc::DoubleVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            double value = col_batch->data[r];
            if(col_batch->hasNulls && !col_batch->notNull[r])
                buffer.push_back(INT32_MIN);
            else if(value >= INT32_MAX)
                buffer.push_back(INT32_MAX);
            else if(value <= INT32_MIN + 1)
                buffer.push_back(INT32_MIN + 1);
            else
            {
                int v = value >= 0 ? (value + 0.5) : (value - 0.5);
                buffer.push_back(v);
            }
        }
        break;
    }
    case orc::TypeKind::STRING:
    case orc::TypeKind::CHAR:
    case orc::TypeKind::VARCHAR:
    {
        orc::StringVectorBatch *col_batch = dynamic_cast<orc::StringVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(INT32_MIN);
                continue;
            }
            int64_t len = col_batch->length[r];
            char *data = col_batch->data[r];
            std::string str(data, len);
            int intVal;
            switch(times_t)
            {
            case DT_INT:
                try {
                    intVal = stoi(str);
                } catch (exception& e) {
                    throw RuntimeException(ORC_PREFIX + "\"" + str + "\" can not be transformed can not be transformed to INT data.");
                }
                break;
            case DT_DATE:
                dateParser(str, intVal);
                break;
            case DT_MONTH:
                monthParser(str, intVal);
                break;
            case DT_TIME:
                timeParser(str, intVal);
                break;
            case DT_SECOND:
                secondParser(str, intVal);
                break;
            case DT_MINUTE:
                minuteParser(str, intVal);
                break;
            case DT_DATETIME:
                datetimeParser(str, intVal);
                break;
            default:
                break;
            }
            buffer.push_back(intVal);
        }

        break;
    }
    case orc::TypeKind::TIMESTAMP:
    {
        orc::TimestampVectorBatch *col_batch = dynamic_cast<orc::TimestampVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(INT32_MIN);
            }
            else
            {
                int64_t data = col_batch->data[r];
                int64_t nanosecond = col_batch->nanoseconds[r];
                int timestamp = data * 1000 + nanosecond / 1000000;
                buffer.push_back(static_cast<int>(timestamp));
            }
        }
        switch(times_t)
        {
        case DT_DATE:
            convertToDTdate(type, buffer);
            break;
        case DT_MONTH:
            convertToDTmonth(type, buffer);
            break;
        case DT_TIME:
            convertToDTtime(type, buffer);
            break;
        case DT_SECOND:
            convertToDTsecond(type, buffer);
            break;
        case DT_MINUTE:
            convertToDTminute(type, buffer);
            break;
        case DT_DATETIME:
            convertToDTdatetime(type, buffer);
            break;
        default:
            break;
        }
        break;
    }
    case orc::TypeKind::DATE:
    {
        orc::LongVectorBatch *col_batch = dynamic_cast<orc::LongVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(INT32_MIN);
            }
            else
            {
                int64_t data = col_batch->data[r];
                buffer.push_back(static_cast<int>(data));
            }
        }
        switch(times_t)
        {
        case DT_DATE:
            convertToDTdate(type, buffer);
            break;
        case DT_MONTH:
            convertToDTmonth(type, buffer);
            break;
        case DT_TIME:
            convertToDTtime(type, buffer);
            break;
        case DT_SECOND:
            convertToDTsecond(type, buffer);
            break;
        case DT_MINUTE:
            convertToDTminute(type, buffer);
            break;
        case DT_DATETIME:
            convertToDTdatetime(type, buffer);
            break;
        default:
            break;
        }
        break;
    }
    default:
        throw RuntimeException(ORC_PREFIX + "unsupported data type.");
    }
    return true;
}
bool convertORCToDolphindbLong(int col_idx, orc::StructVectorBatch *root, orc::TypeKind type, vector<long long> &buffer, DATA_TYPE times_t)
{
    switch(type)
    {
    case orc::TypeKind::BOOLEAN:
    case orc::TypeKind::BYTE:
    case orc::TypeKind::SHORT:
    case orc::TypeKind::INT:
    {
        orc::LongVectorBatch *col_batch = dynamic_cast<orc::LongVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            int64_t value = col_batch->data[r];
            if(col_batch->hasNulls && !col_batch->notNull[r])
                buffer.push_back(INT64_MIN);
            else if(value == INT32_MIN)
                buffer.push_back(INT64_MIN);
            else
                buffer.push_back(static_cast<long long>(value));
        }
        break;
    }
    case orc::TypeKind::LONG:
    {
        orc::LongVectorBatch *col_batch = dynamic_cast<orc::LongVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(INT64_MIN);
            }
            else
            {
                int64_t value = col_batch->data[r];
                buffer.push_back(static_cast<long long>(value));
            }
        }
        break;
    }
    case orc::TypeKind::FLOAT:
    case orc::TypeKind::DOUBLE:
    {
        orc::DoubleVectorBatch *col_batch = dynamic_cast<orc::DoubleVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            double value = col_batch->data[r];
            if(col_batch->hasNulls && !col_batch->notNull[r])
                buffer.push_back(INT64_MIN);
            else if(value >= static_cast<double>(INT64_MAX))
                buffer.push_back(INT64_MAX);
            else if(value <= static_cast<double>(INT64_MIN + 1))
                buffer.push_back(INT64_MIN + 1);
            else
            {
                long long v = value >= 0 ? (value + 0.5) : (value - 0.5);
                buffer.push_back(v);
            }
        }
        break;
    }
    case orc::TypeKind::STRING:
    case orc::TypeKind::CHAR:
    case orc::TypeKind::VARCHAR:
    {
        orc::StringVectorBatch *col_batch = dynamic_cast<orc::StringVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
                buffer.push_back(INT64_MIN);
            int64_t len = col_batch->length[r];
            char *data = col_batch->data[r];
            const string str(data, len);
            long long longVal;
            switch(times_t)
            {
            case DT_LONG:
                try {
                    longVal = stoll(str);
                } catch (exception& e) {
                    throw RuntimeException(ORC_PREFIX + "\"" + str + "\" can not be transformed can not be transformed to LONG data.");
                }
                break;
            case DT_NANOTIME:
                nanotimeParser(str, longVal);
                break;
            case DT_NANOTIMESTAMP:
                nanotimestampParser(str, longVal);
                break;
            case DT_TIMESTAMP:
                timestampParser(str, longVal);
                break;
            default:
                break;
            }
            buffer.push_back(longVal);
        }
        break;
    }
    case orc::TypeKind::TIMESTAMP:
    {
        orc::TimestampVectorBatch *col_batch = dynamic_cast<orc::TimestampVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(INT64_MIN);
            }
            else
            {
                int64_t data = col_batch->data[r];
                int64_t nanoseconds = col_batch->nanoseconds[r];
                long long timestamp = data * 1000000000 + nanoseconds;
                buffer.push_back(timestamp);
            }
        }
        switch(times_t)
        {
        case DT_NANOTIME:
            convertToDTnanotime(type, buffer);
            break;
        case DT_NANOTIMESTAMP:
            convertToDTnanotimestamp(type, buffer);
            break;
        case DT_TIMESTAMP:
            convertToDTtimestamp(type, buffer);
            break;
        default:
            break;
        }
        break;
    }
    case orc::TypeKind::DATE:
    {
        orc::LongVectorBatch *col_batch = dynamic_cast<orc::LongVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(INT64_MIN);
            }
            else
            {
                int64_t data = col_batch->data[r];
                buffer.push_back(static_cast<long long>(data));
            }
        }
        switch(times_t)
        {
        case DT_NANOTIME:
            convertToDTnanotime(type, buffer);
            break;
        case DT_NANOTIMESTAMP:
            convertToDTnanotimestamp(type, buffer);
            break;
        case DT_TIMESTAMP:
            convertToDTtimestamp(type, buffer);
            break;
        default:
            break;
        }
        break;
    }
    default:
        throw RuntimeException(ORC_PREFIX + "unsupported data type.");
    }
    return true;
}
bool convertORCToDolphindbShort(int col_idx, orc::StructVectorBatch *root, orc::TypeKind type, vector<short> &buffer)
{
    switch(type)
    {
    case orc::TypeKind::BOOLEAN:
    case orc::TypeKind::BYTE:
    case orc::TypeKind::SHORT:
    {
        orc::LongVectorBatch *col_batch = dynamic_cast<orc::LongVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(INT16_MIN);
            }
            else
            {
                int64_t value = col_batch->data[r];
                buffer.push_back(static_cast<short>(value));
            }
        }
        break;
    }
    case orc::TypeKind::INT:
    case orc::TypeKind::LONG:
    {
        orc::LongVectorBatch *col_batch = dynamic_cast<orc::LongVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            int64_t value = col_batch->data[r];
            if(col_batch->hasNulls && !col_batch->notNull[r])
                buffer.push_back(INT16_MIN);
            else if(value >= INT16_MAX)
                buffer.push_back(INT16_MAX);
            else if(value <= INT16_MIN + 1)
                buffer.push_back(INT16_MIN + 1);
            else
                buffer.push_back(static_cast<short>(value));
        }
        break;
    }
    case orc::TypeKind::FLOAT:
    case orc::TypeKind::DOUBLE:
    {
        orc::DoubleVectorBatch *col_batch = dynamic_cast<orc::DoubleVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            double value = col_batch->data[r];
            if(col_batch->hasNulls && !col_batch->notNull[r])
                buffer.push_back(INT16_MIN);
            else if(value >= INT16_MAX)
                buffer.push_back(INT16_MAX);
            else if(value <= INT16_MIN + 1)
                buffer.push_back(INT16_MIN + 1);
            else
            {
                short v = value >= 0 ? (value + 0.5) : (value - 0.5);
                buffer.push_back(v);
            }
        }
        break;
    }
    case orc::TypeKind::STRING:
    case orc::TypeKind::CHAR:
    case orc::TypeKind::VARCHAR:
    {
        orc::StringVectorBatch *col_batch = dynamic_cast<orc::StringVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            int64_t len = col_batch->length[r];
            char *data = col_batch->data[r];
            string str(data, len);
            if(col_batch->hasNulls && !col_batch->notNull[r])
                buffer.push_back(INT16_MIN);
            else
                buffer.push_back(static_cast<short>(stoi(str)));
        }
        break;
    }
    case orc::TypeKind::TIMESTAMP:
        throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::TIMESTAMP->" + Util::getDataTypeString(DT_SHORT));
    case orc::TypeKind::DATE:
        throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::DATE->" + Util::getDataTypeString(DT_SHORT));
    default:
        throw RuntimeException(ORC_PREFIX + "unsupported data type.");
    }
    return true;
}
bool convertORCToDolphindbFloat(int col_idx, orc::StructVectorBatch *root, orc::TypeKind type, vector<float> &buffer)
{
    switch(type)
    {
    case orc::TypeKind::BOOLEAN:
    case orc::TypeKind::BYTE:
    case orc::TypeKind::SHORT:
    case orc::TypeKind::INT:
    case orc::TypeKind::LONG:
    {
        orc::LongVectorBatch *col_batch = dynamic_cast<orc::LongVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(FLT_MIN);
            }
            else
            {
                int64_t value = col_batch->data[r];
                buffer.push_back(static_cast<float>(value));
            }
        }
        break;
    }
    case orc::TypeKind::FLOAT:
    case orc::TypeKind::DOUBLE:
    {
        orc::DoubleVectorBatch *col_batch = dynamic_cast<orc::DoubleVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(FLT_MIN);
            }
            else
            {
                double value = col_batch->data[r];
                buffer.push_back(static_cast<float>(value));
            }
        }
        break;
    }
    case orc::TypeKind::STRING:
    case orc::TypeKind::CHAR:
    case orc::TypeKind::VARCHAR:
    {
        orc::StringVectorBatch *col_batch = dynamic_cast<orc::StringVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            int64_t len = col_batch->length[r];
            char *data = col_batch->data[r];
            string str(data, len);
            if(col_batch->hasNulls && !col_batch->notNull[r])
                buffer.push_back(FLT_MIN);
            else {
                try {
                    buffer.push_back(std::stof(str));
                } catch (exception& e) {
                    throw RuntimeException(ORC_PREFIX + "\"" + str + "\" can not be transformed can not be transformed to FLOAT data.");
                }
            }
        }
        break;
    }
    case orc::TypeKind::TIMESTAMP:
        throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::TIMESTAMP->" + Util::getDataTypeString(DT_FLOAT));
    case orc::TypeKind::DATE:
        throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::DATE->" + Util::getDataTypeString(DT_FLOAT));
    default:
        throw RuntimeException(ORC_PREFIX + "unsupported data type.");
    }
    return true;
}
bool convertORCToDolphindbDouble(int col_idx, orc::StructVectorBatch *root, orc::TypeKind type, vector<double> &buffer)
{
    switch(type)
    {
    case orc::TypeKind::BOOLEAN:
    case orc::TypeKind::BYTE:
    case orc::TypeKind::SHORT:
    case orc::TypeKind::INT:
    case orc::TypeKind::LONG:
    {
        orc::LongVectorBatch *col_batch = dynamic_cast<orc::LongVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(DBL_MIN);
            }
            else
            {
                int64_t value = col_batch->data[r];
                buffer.push_back(static_cast<double>(value));
            }
        }
        break;
    }
    case orc::TypeKind::FLOAT:
    case orc::TypeKind::DOUBLE:
    {
        orc::DoubleVectorBatch *col_batch = dynamic_cast<orc::DoubleVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(DBL_MIN);
            }
            else
            {
                double value = col_batch->data[r];
                buffer.push_back(value);
            }
        }
        break;
    }
    case orc::TypeKind::STRING:
    case orc::TypeKind::CHAR:
    case orc::TypeKind::VARCHAR:
    {
        orc::StringVectorBatch *col_batch = dynamic_cast<orc::StringVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            int64_t len = col_batch->length[r];
            char *data = col_batch->data[r];
            string str(data, len);
            if(col_batch->hasNulls && !col_batch->notNull[r])
                buffer.push_back(DBL_MIN);
            else {
                try {
                    buffer.push_back(std::stod(str));
                } catch (exception& e) {
                    throw RuntimeException(ORC_PREFIX + "\"" + str + "\" can not be transformed can not be transformed to DOUBLE data.");
                }
            }
        }
        break;
    }
    case orc::TypeKind::TIMESTAMP:
        throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::TIMESTAMP->" + Util::getDataTypeString(DT_DOUBLE));
    case orc::TypeKind::DATE:
        throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::DATE->" + Util::getDataTypeString(DT_DOUBLE));
    default:
        throw RuntimeException(ORC_PREFIX + "unsupported data type.");
    }
    return true;
}
bool convertORCToDolphindbString(int col_idx, orc::StructVectorBatch *root, orc::TypeKind type, vector<string> &buffer, DATA_TYPE string_t)
{
    switch(type)
    {
    case orc::TypeKind::BOOLEAN:
    {
        if(string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc:BOOLEAN->" + Util::getDataTypeString(string_t));
        orc::LongVectorBatch *col_batch = dynamic_cast<orc::LongVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            int64_t value = col_batch->data[r];
            if(col_batch->hasNulls && !col_batch->notNull[r])
                buffer.push_back(STR_MIN);
            else if(value)
                buffer.push_back("true");
            else
                buffer.push_back("false");
        }
        break;
    }
    case orc::TypeKind::BYTE:
    {
        if(string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc:BYTE->" + Util::getDataTypeString(string_t));
        orc::LongVectorBatch *col_batch = dynamic_cast<orc::LongVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(STR_MIN);
            }
            else
            {
                int64_t value = col_batch->data[r];
                buffer.push_back(std::to_string(value));
            }
        }
        break;
    }
    case orc::TypeKind::SHORT:
    {
        if(string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc:SHORT->" + Util::getDataTypeString(string_t));
        orc::LongVectorBatch *col_batch = dynamic_cast<orc::LongVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(STR_MIN);
            }
            else
            {
                int64_t value = col_batch->data[r];
                buffer.push_back(std::to_string(value));
            }
        }
        break;
    }
    case orc::TypeKind::INT:
    {
        if(string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc:INT->" + Util::getDataTypeString(string_t));
        orc::LongVectorBatch *col_batch = dynamic_cast<orc::LongVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(STR_MIN);
            }
            else
            {
                int64_t value = col_batch->data[r];
                buffer.push_back(std::to_string(value));
            }
        }
        break;
    }
    case orc::TypeKind::LONG:
    {
        if(string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc:LONG->" + Util::getDataTypeString(string_t));
        orc::LongVectorBatch *col_batch = dynamic_cast<orc::LongVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(STR_MIN);
            }
            else
            {
                int64_t value = col_batch->data[r];
                buffer.push_back(std::to_string(value));
            }
        }
        break;
    }
    case orc::TypeKind::FLOAT:
    {
        if(string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc:FLOAT->" + Util::getDataTypeString(string_t));
        orc::DoubleVectorBatch *col_batch = dynamic_cast<orc::DoubleVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(STR_MIN);
            }
            else
            {
                double value = col_batch->data[r];
                std::ostringstream out;
                out << value;
                buffer.push_back(out.str());
            }
        }
        break;
    }
    case orc::TypeKind::DOUBLE:
    {
        if(string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc:DOUBLE->" + Util::getDataTypeString(string_t));
        orc::DoubleVectorBatch *col_batch = dynamic_cast<orc::DoubleVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(STR_MIN);
            }
            else
            {
                double value = col_batch->data[r];
                std::ostringstream out;
                out << value;
                buffer.push_back(out.str());
            }
        }
        break;
    }
    case orc::TypeKind::STRING:
    {
        if(string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc:STRING->" + Util::getDataTypeString(string_t));
        orc::StringVectorBatch *col_batch = dynamic_cast<orc::StringVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(STR_MIN);
            }
            else
            {
                int64_t len = col_batch->length[r];
                char *data = col_batch->data[r];
                buffer.push_back(std::string(data, len));
            }
        }
        break;
    }
    case orc::TypeKind::CHAR:
    {
        if(string_t == DT_INT128)
            throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::CHAR->" + Util::getDataTypeString(string_t));
        orc::StringVectorBatch *col_batch = dynamic_cast<orc::StringVectorBatch*>(root->fields[col_idx]);
        if(string_t == DT_UUID && col_batch->numElements > 0 && col_batch->length[0] != 16)
            throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::CHAR, length NOT 16->" + Util::getDataTypeString(string_t));
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(STR_MIN);
            }
            else
            {
                int64_t len = col_batch->length[r];
                char *data = col_batch->data[r];
                buffer.push_back(std::string(data, len));
            }
        }
        break;
    }
    case orc::TypeKind::VARCHAR:
    {
        if(string_t == DT_UUID || string_t == DT_INT128)
            throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc:VARCHAR->" + Util::getDataTypeString(string_t));
        orc::StringVectorBatch *col_batch = dynamic_cast<orc::StringVectorBatch*>(root->fields[col_idx]);
        for(uint64_t r = 0; r < col_batch->numElements; ++r)
        {
            if(col_batch->hasNulls && !col_batch->notNull[r])
            {
                buffer.push_back(STR_MIN);
            }
            else
            {
                int64_t len = col_batch->length[r];
                char *data = col_batch->data[r];
                buffer.push_back(std::string(data, len));
            }
        }
        break;
    }
    case orc::TypeKind::TIMESTAMP:
        throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::TIMESTAMP->" + Util::getDataTypeString(string_t));
    case orc::TypeKind::DATE:
        throw RuntimeException(ORC_PREFIX + "incompatible type in column " + std::to_string(col_idx) + " orc::DATE->" + Util::getDataTypeString(string_t));
    default:
        throw RuntimeException(ORC_PREFIX + "unsupported data type.");
    }
    return true;
}

ConstantSP loadORC(const string &filename, const ConstantSP &schema, const ConstantSP &column, int rowStart, int rowNum)
{
    ORC_UNIQUE_PTR<orc::InputStream> inStream = orc::readLocalFile(filename);
    orc::ReaderOptions readerOptions;
    ORC_UNIQUE_PTR<orc::Reader> reader = orc::createReader(
        std::move(inStream),
        readerOptions
    );
    const orc::Type &schema_descr = reader->getType();
    int col_num = schema_descr.getSubtypeCount();
    VectorSP columnToRead;
    if(column->isNull()){
        columnToRead = Util::createIndexVector(0, col_num);
    }else{
        columnToRead = column;
        int maxIndex = columnToRead->max()->getInt();
        int minIndex = columnToRead->min()->getInt();
        if(maxIndex >= col_num){
            throw IllegalArgumentException("loadOrc", "Invalid column index " + std::to_string(maxIndex) + " to load.");
        }
        if(minIndex < 0){
            throw IllegalArgumentException("loadOrc", "Invalid column index " + std::to_string(minIndex) + " to load.");
        }
    }
    int totalRow = reader->getNumberOfRows();
    if(rowStart >= totalRow)
        throw RuntimeException(ORC_PREFIX + "stripeStart to read is out of range.");
    int rowCount;
    if(rowNum == 0)
    {
        rowCount = totalRow - rowStart;
    }
    else
    {
        rowCount = rowNum;
        if(rowStart + rowCount > totalRow)
            rowCount = totalRow - rowStart;
    }
    ConstantSP init_schema = schema;
    if(schema->isNull())
    {
        vector<ConstantSP> cols(2);
        if(!getSchemaCol(schema_descr, columnToRead, cols))
            throw RuntimeException(ORC_PREFIX + "get schema failed.");
        vector<string> colNames(2);
        colNames[0] = "name";
        colNames[1] = "type";
        init_schema = Util::createTable(colNames, cols);
    }
    return loadORC(reader.get(), init_schema, columnToRead, rowStart, rowCount);
}

class HdfsInputStream : public orc::InputStream
{
    public:
        HdfsInputStream(void *buf, uint64_t len):buffer(buf),length(len){message = "hdfs input stream";}
        uint64_t getLength() const override
        {
            return length;
        }
        uint64_t getNaturalReadSize() const override
        {
            return 128 * 1024;
        }

        void read(void* buf,uint64_t len,uint64_t offset) override
        {
            char *p = (char *)buffer+offset;
            memcpy(buf, (void *)p, len);
        }
        const std::string& getName() const override
        {
            return message;
        }

    private:
        void *buffer;
        string message;
        uint64_t length;
};

ConstantSP loadORCFromBuf(void *buffer, uint64_t length)
{
    ORC_UNIQUE_PTR<orc::InputStream> inStream = std::unique_ptr<orc::InputStream>(new HdfsInputStream(buffer,length));
    int rowStart = 0;
    int rowNum = 0;
    ConstantSP schema = ORCPluginImp::nullSP;
    ConstantSP column = new Void();
    orc::ReaderOptions readerOptions;
    ORC_UNIQUE_PTR<orc::Reader> reader = orc::createReader(
        std::move(inStream),
        readerOptions
    );
    const orc::Type &schema_descr = reader->getType();
    int col_num = schema_descr.getSubtypeCount();
    VectorSP columnToRead;
    if(column->isNull()){
        columnToRead = Util::createIndexVector(0, col_num);
    }else{
        columnToRead = column;
        int maxIndex = columnToRead->max()->getInt();
        int minIndex = columnToRead->min()->getInt();
        if(maxIndex >= col_num){
            throw IllegalArgumentException("loadOrc", "Invalid column index " + std::to_string(maxIndex) + " to load.");
        }
        if(minIndex < 0){
            throw IllegalArgumentException("loadOrc", "Invalid column index " + std::to_string(minIndex) + " to load.");
        }
    }
    int totalRow = reader->getNumberOfRows();
    if(rowStart >= totalRow)
        throw RuntimeException(ORC_PREFIX + "stripeStart to read is out of range.");
    int rowCount;
    if(rowNum == 0)
    {
        rowCount = totalRow - rowStart;
    }
    else
    {
        rowCount = rowNum;
        if(rowStart + rowCount > totalRow)
            rowCount = totalRow - rowStart;
    }
    ConstantSP init_schema = schema;
    if(schema->isNull())
    {
        vector<ConstantSP> cols(2);
        if(!getSchemaCol(schema_descr, columnToRead, cols))
            throw RuntimeException(ORC_PREFIX + "get schema failed.");
        vector<string> colNames(2);
        colNames[0] = "name";
        colNames[1] = "type";
        init_schema = Util::createTable(colNames, cols);
    }
    return loadORC(reader.get(), init_schema, columnToRead, rowStart, rowCount);
}

ConstantSP loadORC(orc::Reader *reader, const ConstantSP &schema, const ConstantSP &column, int rowStart, int rowNum)
{
    const orc::Type &schema_descr = reader->getType();
    TableSP tableWithSchema = DBFileIO::createEmptyTableFromSchema(schema);
    int col_num = column->size();
    vector<VectorSP> dolphindbCol(col_num);
    createNewVectorSP(dolphindbCol, tableWithSchema);
    orc::RowReaderOptions rowReaderOptions;
    ORC_UNIQUE_PTR<orc::RowReader> rowReader = reader->createRowReader(rowReaderOptions);
    rowReader->seekToRow(rowStart);
    // ORC_UNIQUE_PTR<orc::ColumnVectorBatch> batch = rowReader->createRowBatch(rowNum);
    int batchSize = std::min(1000000, rowNum);
    ORC_UNIQUE_PTR<orc::ColumnVectorBatch> batch = rowReader->createRowBatch(batchSize);
    int hasRead = 0;
    while(rowReader->next(*batch))
    {
        orc::StructVectorBatch *root = dynamic_cast<orc::StructVectorBatch*>(batch.get());
        for(int i = 0; i < col_num; i++)
        {
            int col_idx = column->getInt(i);
            const orc::Type *subtype = schema_descr.getSubtype(col_idx);
            orc::TypeKind type = subtype->getKind();
            DATA_TYPE dolphin_t = dolphindbCol[i]->getType();
            switch(dolphin_t)
            {
            case DT_BOOL:
            {
                vector<char> buffer;
                convertORCToDolphindbBool(col_idx, root, type, buffer);
                // dolphindbCol[i]->appendChar(buffer.data(), buffer.size());
                for(auto &flt : buffer){
                    ConstantSP v = new Bool(flt);
                    if(flt == CHAR_MIN){
                        v->setNull();
                    }
                    dolphindbCol[i]->append(v);
                }
                break;
            }
            case DT_CHAR:
            {
                vector<char> buffer;
                convertORCToDolphindbChar(col_idx, root, type, buffer);
                // dolphindbCol[i]->appendChar(buffer.data(), buffer.size());
                for(auto &flt : buffer){
                    ConstantSP v = new Char(flt);
                    if(flt == CHAR_MIN){
                        v->setNull();
                    }
                    dolphindbCol[i]->append(v);
                }
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
                convertORCToDolphindbInt(col_idx, root, type, buffer, dolphin_t);
                // dolphindbCol[i]->appendInt(buffer.data(), buffer.size());
                for(auto &flt : buffer){
                    ConstantSP v = new Int(flt);
                    if(flt == INT32_MIN){
                        v->setNull();
                    }
                    dolphindbCol[i]->append(v);
                }
                break;
            }
            case DT_LONG:
            case DT_NANOTIME:
            case DT_NANOTIMESTAMP:
            case DT_TIMESTAMP:
            {
                vector<long long> buffer;
                convertORCToDolphindbLong(col_idx, root, type, buffer, dolphin_t);
                // dolphindbCol[i]->appendLong(buffer.data(), buffer.size());
                for(auto &flt : buffer){
                    ConstantSP v = new Long(flt);
                    if(flt == INT64_MIN){
                        v->setNull();
                    }
                    dolphindbCol[i]->append(v);
                }
                break;
            }
            case DT_SHORT:
            {
                vector<short> buffer;
                convertORCToDolphindbShort(col_idx, root, type, buffer);
                // dolphindbCol[i]->appendShort(buffer.data(), buffer.size());
                for(auto &flt : buffer){
                    ConstantSP v = new Short(flt);
                    if(flt == INT16_MIN){
                        v->setNull();
                    }
                    dolphindbCol[i]->append(v);
                }
                break;
            }
            case DT_FLOAT:
            {
                vector<float> buffer;
                convertORCToDolphindbFloat(col_idx, root, type, buffer);
                // dolphindbCol[i]->appendFloat(buffer.data(), buffer.size());
                for(auto &flt : buffer){
                    ConstantSP v = new Float(flt);
                    if(flt == FLT_MIN){
                        v->setNull();
                    }
                    dolphindbCol[i]->append(v);
                }
                break;
            }
            case DT_DOUBLE:
            {
                vector<double> buffer;
                convertORCToDolphindbDouble(col_idx, root, type, buffer);
                // dolphindbCol[i]->appendDouble(buffer.data(), buffer.size());
                for(auto &dbl : buffer){
                    ConstantSP v = new Double(dbl);
                    if(dbl == DBL_MIN){
                        v->setNull();
                    }
                    dolphindbCol[i]->append(v);
                }
                break;
            }
            case DT_INT128:
            case DT_UUID:
            case DT_STRING:
            case DT_SYMBOL:
            {
                vector<string> buffer;
                convertORCToDolphindbString(col_idx, root, type, buffer, dolphin_t);
                // dolphindbCol[i]->appendString(buffer.data(), buffer.size());
                // dolphindbCol[i] = Util::createVector(dolphin_t, buffer.size(), buffer.size());
                for(auto &str : buffer){
                    ConstantSP v = new String(str);
                    dolphindbCol[i]->append(v);
                }
                break;
            }
            default:
                throw RuntimeException(ORC_PREFIX + "unsupported data type.");
                break;
            }
        }
        hasRead += root->numElements;
        batchSize = std::min(1024, rowNum - hasRead);
        batch = rowReader->createRowBatch(batchSize);
    }
    return appendColumnVecToTable(tableWithSchema, dolphindbCol);
}

void getORCReader(Heap *heap, vector<ConstantSP> &arguments)
{
    orc::Reader *file = reinterpret_cast<orc::Reader*>(arguments[0]->getLong());
    if(file != nullptr)
    {
        delete file;
    }
}

ConstantSP loadFromORCToDatabase(Heap *heap, vector<ConstantSP> &arguments)
{
    orc::Reader *reader = (orc::Reader *)(arguments[0]->getLong());
    TableSP schema = static_cast<TableSP>(arguments[1]);
    VectorSP columnArg = arguments[2];
    int rowStart = arguments[3]->getInt();
    int rowNum = arguments[4]->getInt();
    SystemHandleSP db = static_cast<SystemHandleSP>(arguments[5]);
    string tableName = arguments[6]->getString();

    bool diskSeqMode = !db->getDatabaseDir().empty() &&
                       db->getDomain()->getPartitionType() == SEQ;
    TableSP loadedTable = loadORC(reader, schema, columnArg, rowStart, rowNum);
    FunctionDefSP transform = (FunctionDefSP)arguments[8];
    if(!transform.isNull() && !transform->isNull()){
        vector<ConstantSP> arg = {loadedTable};
        loadedTable = transform->call(heap, arg);
    }
    if(diskSeqMode)
    {
        string id = db->getDomain()->getPartition(arguments[7]->getInt())->getPath();
        string directory = db->getDatabaseDir() + "/" + id;
        if(!DBFileIO::saveBasicTable(heap->currentSession(), directory, loadedTable.get(), tableName, NULL, true, 1, false))
            throw RuntimeException(ORC_PREFIX + "Failed to save the table to directory " + directory);
        return new Long(loadedTable->rows());
    }
    else
        return loadedTable;
}

vector<DistributedCallSP> generateORCTasks(Heap* heap, const orc::Reader *reader, const TableSP &schema, const ConstantSP &column, const int rowStart, const int rowNum,
                                          const SystemHandleSP &db, const string &tableName, const ConstantSP &transform)
{
    int maxRowNum = reader->getNumberOfRows();
    if(rowStart >= maxRowNum){
        throw RuntimeException(ORC_PREFIX + "rowStart to read is out of range.");
    }
    int partitions = floor((maxRowNum - rowStart) / 1000000.0);
    if(rowNum != 0){
        partitions = std::min(partitions, int(floor(rowNum / 1000000.0)));
    }
    DomainSP domain = db->getDomain();

    if(domain->getPartitionType() == SEQ)
    {
        if(domain->getPartitionCount() <= 1)
            throw IOException(ORC_PREFIX + "The database must have at least two partitions.");
        partitions = domain->getPartitionCount();
    }
    partitions = partitions == 0 ? 1: partitions;

    vector<DistributedCallSP> tasks;
    ConstantSP _tableName = new String(tableName);
    const char *fmt = "Read orc file";
    FunctionDefSP getOrcFileHead(Util::createSystemProcedure("getOrcReader", getORCReader, 1, 1));
    ConstantSP orcReader = Util::createResource(reinterpret_cast<long long>(reader), fmt, getOrcFileHead, heap->currentSession());
    FunctionDefSP func = Util::createSystemFunction("loadFromOrcToDatabase", loadFromORCToDatabase, 9, 9, false);
    int rowPerPartition = floor(reader->getNumberOfRows() * 1.0 / partitions);
    ConstantSP rowCount = new Long(rowPerPartition);
    for(int i = 0; i < partitions; i++)
    {
        ConstantSP rowStart = new Long(i * rowPerPartition);
        ConstantSP id = new Int(i);
        if(i == partitions-1) {
            rowCount->setLong(reader->getNumberOfRows() - i * rowCount->getLong());
        }
        vector<ConstantSP> args{orcReader, schema, column, rowStart, rowCount, db, _tableName, id, transform};
        ObjectSP call = Util::createRegularFunctionCall(func, args);
        DistributedCallSP task = new DistributedCall(call, true);
        tasks.push_back(task);
    }
    return tasks;
}

TableSP generateInMemoryPartitionedTable(Heap *heap, const SystemHandleSP &db,
                                        const ConstantSP &tables, const ConstantSP &partitionNames)
{
    FunctionDefSP createPartitionedTable = heap->currentSession()->getFunctionDef("createPartitionedTable");
    ConstantSP emptyString = new String("");
    vector<ConstantSP> args{db, tables, emptyString, partitionNames};
    return createPartitionedTable->call(heap, args);
}

ConstantSP generatePartition(Heap *heap, vector<ConstantSP> &arguments)
{
    SystemHandleSP db = arguments[0];
    ConstantSP tb = arguments[1];
    ConstantSP tbInMemory = arguments[2];
    string dbPath = db->getDatabaseDir();
    FunctionDefSP append = heap->currentSession()->getFunctionDef("append!");
    vector<ConstantSP> appendArgs = {tb, tbInMemory};
    append->call(heap, appendArgs);
    return new Void();
}

ConstantSP loadORCEx(Heap *heap, const SystemHandleSP &db, const string &tableName, const ConstantSP &partitionColumns,
                         const string &filename, const TableSP &schema, const ConstantSP &column, const int rowStart, const int rowNum, const ConstantSP &transform)
{
    ORC_UNIQUE_PTR<orc::InputStream> inStream = orc::readLocalFile(filename);
    orc::ReaderOptions readerOptions;
    ORC_UNIQUE_PTR<orc::Reader> reader = orc::createReader(std::move(inStream), readerOptions);
    const orc::Type &schema_descr = reader->getType();
    int col_num = schema_descr.getSubtypeCount();
    VectorSP columnToRead;
    if(column->isNull()){
        columnToRead = Util::createIndexVector(0, col_num);
    }else{
        columnToRead = column;
        int maxIndex = columnToRead->max()->getInt();
        int minIndex = columnToRead->min()->getInt();
        if(maxIndex >= col_num){
            throw IllegalArgumentException("loadOrcEx", "Invalid column index " + std::to_string(maxIndex) + " to load.");
        }
        if(minIndex < 0){
            throw IllegalArgumentException("loadOrcEx", "Invalid column index " + std::to_string(minIndex) + " to load.");
        }
    }
    TableSP convertedSchema;

    if(schema->isNull())
    {
        vector<ConstantSP> cols_d(2);
        if(!getSchemaCol(schema_descr, columnToRead, cols_d))
            throw RuntimeException(ORC_PREFIX + "get schema failed");
        vector<string> colNames(2);
        colNames[0] = "name";
        colNames[1] = "type";
        convertedSchema = Util::createTable(colNames, cols_d);
    }
    else
    {
        convertedSchema = schema;
    }

    vector<DistributedCallSP> tasks = generateORCTasks(heap, reader.release(), convertedSchema, columnToRead, rowStart, rowNum, db, tableName, transform);
    int partitions = tasks.size();
    string owner = heap->currentSession()->getUser()->getUserId();
    DomainSP domain = db->getDomain();
    bool seqDomain = domain->getPartitionType() == SEQ;
    bool inMemory = db->getDatabaseDir().empty();
    ConstantSP tableName_ = new String(tableName);
    if(seqDomain)
    {
        StaticStageExecutor executor(false, false, false);
        executor.execute(heap, tasks);
        for(int i = 0; i < partitions; i++)
        {
            const string &errMsg = tasks[i]->getErrorMessage();
            if(!errMsg.empty())
                throw RuntimeException(ORC_PREFIX + errMsg);
        }
        if(inMemory)
        {
            ConstantSP tmpTables = Util::createVector(DT_ANY, partitions);
            for(int i = 0; i < partitions; i++)
                tmpTables->set(i, tasks[i]->getResultObject());
            ConstantSP partitionNames = new String("");
            return generateInMemoryPartitionedTable(heap, db, tmpTables, partitionNames);
        }
        else
        {
            vector<int> partitionColumnIndices(1, -1);
            vector<int> baseIds;
            int baseId = -1;
            string tableFile = db->getDatabaseDir() + "/" + tableName + ".tbl";
            vector<ColumnDesc> cols;
            int columns = convertedSchema->rows();
            for(int i = 0; i < columns; ++i)
            {
                string name = convertedSchema->getColumn(0)->getString(i);
                DATA_TYPE type = Util::getDataType(convertedSchema->getColumn(1)->getString(i));
                int extra = type == DT_SYMBOL ? baseId : -1;
                cols.push_back(ColumnDesc(name, type, extra));
            }

            string physicalIndex = tableName;
            if(!DBFileIO::saveTableHeader(owner, physicalIndex, cols, partitionColumnIndices, 0, tableFile, NULL))
                throw IOException(ORC_PREFIX + "Failed to save table header " + tableFile);
            if(!DBFileIO::saveDatabase(db.get()))
                throw IOException(ORC_PREFIX + "Failed to save database " + db->getDatabaseDir());
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

        if(existsTable)
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
        FunctionDefSP func = Util::createSystemFunction("savePartition", &generatePartition, 3, 3, false);
        vector<ConstantSP> args(2);
        args[0] = db;
        args[1] = result;
        functors.push_back(Util::createPartialFunction(func, args));
        PipelineStageExecutor executor(functors, false, 4, 2);
        executor.execute(heap, tasks);
        for(int i = 0; i < partitions; ++i)
        {
            if(!tasks[i]->getErrorMessage().empty()){
                string errMsg;
                errMsg = tasks[i]->getErrorMessage();
                throw RuntimeException(ORC_PREFIX + errMsg);
            }
        }
        if(!inMemory)
        {
            vector<ConstantSP> loadTableArgs = {db, tableName_};
            result = heap->currentSession()->getFunctionDef("loadTable")->call(heap, loadTableArgs);
        }
        return result;
    }
}

int getRowNum(const string &str)
{
    ORC_UNIQUE_PTR<orc::InputStream> inStream = orc::readLocalFile(str);
    orc::ReaderOptions options;
    ORC_UNIQUE_PTR<orc::Reader> reader = orc::createReader(
        move(inStream),
        options
    );
    return reader->getNumberOfRows();
}

ConstantSP orcDS(const ConstantSP &filename, int chunkSize, const ConstantSP &schema, int skipRows)
{
    int rowNum = getRowNum(filename->getString());
    int rowRest = std::max(rowNum - skipRows, 0);
    ConstantSP dataSources = Util::createVector(DT_ANY, ceil(1.0 * rowRest / chunkSize));
    ConstantSP column = ORCPluginImp::nullSP;
    FunctionDefSP _loadOrc = Util::createSystemFunction("loadOrc", ::loadORC, 1, 5, false);
    int i = 0;
    int rowStart = skipRows;
    while(rowStart < rowNum){
        ConstantSP _rowStart = new Int(rowStart);
        ConstantSP _chunkSize = new Int(chunkSize);
        vector<ConstantSP> args{filename, schema, column, _rowStart, _chunkSize};
        ObjectSP code = Util::createRegularFunctionCall(_loadOrc, args);
        ConstantSP ds = new DataSource(code);
        dataSources->set(i++, ds);
        rowStart += chunkSize;
    }
    return dataSources;
}

string getORCSchema(const TableSP &table){
    string schema = "";
    for(int i = 0; i < table->columns(); ++i){
        string name = table->getColumnName(i);
        string type;
        switch(table->getColumnType(i)){
        case DT_BOOL:
            type = "boolean";
            break;
        case DT_CHAR:
            type = "tinyint";
            break;
        case DT_SHORT:
            type = "smallint";
            break;
        case DT_INT:
            type = "int";
            break;
        case DT_LONG:
            type = "bigint";
            break;
        case DT_DATE:
        case DT_MONTH:
            type = "date";
            break;
        case DT_TIME:
        case DT_MINUTE:
        case DT_SECOND:
        case DT_DATETIME:
        case DT_TIMESTAMP:
        case DT_NANOTIME:
        case DT_NANOTIMESTAMP:
            type = "timestamp";
            break;
        case DT_FLOAT:
            type = "float";
            break;
        case DT_DOUBLE:
            type = "double";
            break;
        case DT_STRING:
        case DT_SYMBOL:
            type = "string";
            break;
        default:
            throw RuntimeException(ORC_PREFIX + "unsupported type.");
        }
        if(schema == ""){
            schema = name + ":" + type;
        }
        else{
            schema += "," + name + ":" + type;
        }
    }
    return "struct<" + schema + ">";
}

static unsigned saveORCBatch(orc::StructVectorBatch *root, const TableSP table, const unsigned offset, const unsigned count){
    for(int i = 0; i < table->columns(); ++i){
        VectorSP dolphinCol = table->getColumn(i);
        root->fields[i]->hasNulls = true;
        switch(table->getColumnType(i)){
        case DT_BOOL:
        {
            orc::LongVectorBatch *orcCol = dynamic_cast<orc::LongVectorBatch*>(root->fields[i]);
            for(unsigned j = 0; j < count; ++j){
                ConstantSP v = dolphinCol->get(offset + j);
                if(v->isNull()){
                    orcCol->notNull[j] = false;
                }
                else{
                    orcCol->notNull[j] = true;
                    orcCol->data[j] = v->getBool();
                }
            }
            break;
        }
        case DT_CHAR:
        {
            orc::LongVectorBatch *orcCol = dynamic_cast<orc::LongVectorBatch*>(root->fields[i]);
            for(unsigned j = 0; j < count; ++j){
                ConstantSP v = dolphinCol->get(offset + j);
                if(v->isNull()){
                    orcCol->notNull[j] = false;
                }
                else{
                    orcCol->notNull[j] = true;
                    orcCol->data[j] = v->getChar();
                }
            }
            break;
        }
        case DT_SHORT:
        {
            orc::LongVectorBatch *orcCol = dynamic_cast<orc::LongVectorBatch*>(root->fields[i]);
            for(unsigned j = 0; j < count; ++j){
                ConstantSP v = dolphinCol->get(offset + j);
                if(v->isNull()){
                    orcCol->notNull[j] = false;
                }
                else{
                    orcCol->notNull[j] = true;
                    orcCol->data[j] = v->getShort();
                }
            }
            break;
        }
        case DT_INT:
        {
            orc::LongVectorBatch *orcCol = dynamic_cast<orc::LongVectorBatch*>(root->fields[i]);
            for(unsigned j = 0; j < count; ++j){
                ConstantSP v = dolphinCol->get(offset + j);
                if(v->isNull()){
                    orcCol->notNull[j] = false;
                }
                else{
                    orcCol->notNull[j] = true;
                    orcCol->data[j] = v->getInt();
                }

            }
            break;
        }
        case DT_LONG:
        {
            orc::LongVectorBatch *orcCol = dynamic_cast<orc::LongVectorBatch*>(root->fields[i]);
            for(unsigned j = 0; j < count; ++j){
                ConstantSP v = dolphinCol->get(offset + j);
                if(v->isNull()){
                    orcCol->notNull[j] = false;
                }
                else{
                    orcCol->notNull[j] = true;
                    orcCol->data[j] = v->getLong();
                }

            }
            break;
        }
        case DT_DATE:
        {
            orc::LongVectorBatch *orcCol = dynamic_cast<orc::LongVectorBatch*>(root->fields[i]);
            for(unsigned j = 0; j < count; ++j){
                ConstantSP v = dolphinCol->get(offset + j);
                if(v->isNull()){
                    orcCol->notNull[j] = false;
                }
                else{
                    orcCol->notNull[j] = true;
                    orcCol->data[j] = v->getInt();
                }

            }
            break;
        }
        case DT_MONTH:
        {
            orc::LongVectorBatch *orcCol = dynamic_cast<orc::LongVectorBatch*>(root->fields[i]);
            using months = std::chrono::duration<int, std::ratio<2629746> >;
            using days = std::chrono::duration<int, std::ratio<86400> >;
            for(unsigned j = 0; j < count; ++j){
                ConstantSP v = dolphinCol->get(offset + j);
                if(v->isNull()){
                    orcCol->notNull[j] = false;
                }else{
                    orcCol->notNull[j] = true;
                    int m = v->getInt();
                    m = std::chrono::duration_cast<days>(months(m)).count();
                    m = Util::getMonthStart(m - 719514);
                    orcCol->data[j] = m;
                }

            }
            break;
        }
        case DT_TIME:
        {
            orc::TimestampVectorBatch *orcCol = dynamic_cast<orc::TimestampVectorBatch*>(root->fields[i]);
            for(unsigned j = 0; j < count; ++j){
                ConstantSP v = dolphinCol->get(offset + j);
                if(v->isNull()){
                    orcCol->notNull[j] = false;
                }
                else{
                    orcCol->notNull[j] = true;
                    int millis = v->getInt();
                    orcCol->data[j] = millis / 1000;
                    orcCol->nanoseconds[j] = (millis % 1000) * 1000000;
                }
            }
            break;
        }
        case DT_MINUTE:
        {
            orc::TimestampVectorBatch *orcCol = dynamic_cast<orc::TimestampVectorBatch*>(root->fields[i]);
            for(unsigned j = 0; j < count; ++j){
                ConstantSP v = dolphinCol->get(offset + j);
                if(v->isNull()){
                    orcCol->notNull[j] = false;
                }
                else{
                    orcCol->notNull[j] = true;
                    orcCol->data[j] = v->getInt() * 60;
                    orcCol->nanoseconds[j] = 0;
                }
            }
            break;
        }
        case DT_SECOND:
        case DT_DATETIME:
        {
            orc::TimestampVectorBatch *orcCol = dynamic_cast<orc::TimestampVectorBatch*>(root->fields[i]);
            for(unsigned j = 0; j < count; ++j){
                ConstantSP v = dolphinCol->get(offset + j);
                if(v->isNull()){
                    orcCol->notNull[j] = false;
                }
                else{
                    orcCol->notNull[j] = true;
                    orcCol->data[j] = v->getInt();
                    orcCol->nanoseconds[j] = 0;
                }

            }
            break;
        }
        case DT_TIMESTAMP:
        {
            orc::TimestampVectorBatch *orcCol = dynamic_cast<orc::TimestampVectorBatch*>(root->fields[i]);
            for(unsigned j = 0; j < count; ++j){
                ConstantSP v = dolphinCol->get(offset + j);
                if(v->isNull()){
                    orcCol->notNull[j] = false;
                }
                else{
                    orcCol->notNull[j] = true;
                    long long millis = v->getLong();
                    orcCol->data[j] = millis / 1000;
                    orcCol->nanoseconds[j] = (millis % 1000) * 1000000;
                }

            }
            break;
        }
        case DT_NANOTIME:
        case DT_NANOTIMESTAMP:
        {
            orc::TimestampVectorBatch *orcCol = dynamic_cast<orc::TimestampVectorBatch*>(root->fields[i]);
            for(unsigned j = 0; j < count; ++j){
                ConstantSP v = dolphinCol->get(offset + j);
                if(v->isNull()){
                    orcCol->notNull[j] = false;
                }
                else{
                    orcCol->notNull[j] = true;
                    long long nanos = v->getLong();
                    orcCol->data[j] = nanos / 1000000000;
                    orcCol->nanoseconds[j] = nanos % 1000000000;
                }
            }
            break;
        }
        case DT_FLOAT:
        {
            orc::DoubleVectorBatch *orcCol = dynamic_cast<orc::DoubleVectorBatch*>(root->fields[i]);
            for(unsigned j = 0; j < count; ++j){
                ConstantSP v = dolphinCol->get(offset + j);
                if(v->isNull()){
                    orcCol->notNull[j] = false;
                }
                else{
                    orcCol->notNull[j] = true;
                    orcCol->data[j] = v->getFloat();
                }

            }
            break;
        }
        case DT_DOUBLE:
        {
            orc::DoubleVectorBatch *orcCol = dynamic_cast<orc::DoubleVectorBatch*>(root->fields[i]);
            for(unsigned j = 0; j < count; ++j){
                ConstantSP v = dolphinCol->get(offset + j);
                if(v->isNull()){
                    orcCol->notNull[j] = false;
                }
                else{
                    orcCol->notNull[j] = true;
                    orcCol->data[j] = v->getDouble();
                }
            }
            break;
        }
        case DT_STRING:
        case DT_SYMBOL:
        {
            orc::StringVectorBatch *orcCol = dynamic_cast<orc::StringVectorBatch*>(root->fields[i]);
            for(unsigned j = 0; j < count; ++j){
                ConstantSP v = dolphinCol->get(offset + j);
                if(v->isNull()){
                    orcCol->notNull[j] = false;
                }
                else{
                    orcCol->notNull[j] = true;
                    string str = v->getString();
                    orcCol->length[j] = str.length();
                    orcCol->data[j] = new char[str.length() + 1];
                    strcpy(orcCol->data[j], str.c_str());
                }
            }
            break;
        }
        default:
            throw RuntimeException(ORC_PREFIX + "unsupported type.");
        }
        root->fields[i]->numElements = count;
    }
    return count;
}

ConstantSP saveORC(const TableSP &table, const string &fileName){
    ORC_UNIQUE_PTR<orc::OutputStream> outStream = orc::writeLocalFile(fileName);
    ORC_UNIQUE_PTR<orc::Type> schema(orc::Type::buildTypeFromString(getORCSchema(table)));
    orc::WriterOptions writerOptions;
    ORC_UNIQUE_PTR<orc::Writer> writer = orc::createWriter(*schema, outStream.get(), writerOptions);

    // uint64_t batchSize = table->getColumn(0)->size();
    uint64_t batchSize = 1024, rowCount = table->getColumn(0)->size();
    ORC_UNIQUE_PTR<orc::ColumnVectorBatch> batch = writer->createRowBatch(batchSize);

    uint64_t rows = 0;
    while(rows < rowCount){
        orc::StructVectorBatch *root = dynamic_cast<orc::StructVectorBatch*>(batch.get());
        unsigned wrote = saveORCBatch(root, table, rows, std::min(batchSize, rowCount - rows));
        root->numElements = wrote;
        writer->add(*batch);
        rows += wrote;
    }
    writer->close();
    return new Void();
}

} // namespace ORCPluginImp