//
// Created by lin on 2021/2/23.
//

#include "PluginHbase.h"
#include "Util.h"
#include "ScalarImp.h"
#include "Logger.h"

#include <protocol/TBinaryProtocol.h>
#include <transport/TTransportUtils.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::hadoop::hbase::thrift;
typedef std::vector<std::string> StrVec;
typedef std::map<std::string,TCell> CellMap;

inline void writeDoubleQuotedString(string& dest, const string& source){
    dest.append(1, '"');
    int len = source.length();
    for(int i=0; i<len; ++i){
        char ch = source[i];
        dest.append(ch=='"' ? 2 : 1, ch);
    }
    dest.append(1, '"');
}

inline long long parseLongInteger(const char* str, int length){
    int cursor = 0;
    while(cursor < length && (str[cursor]<'0' || str[cursor]>'9')) ++cursor;
    if(cursor == length)
        return LLONG_MIN;
    int firstDigitIndex = cursor;
    long long val = str[cursor] - '0';
    while(++cursor < length){
        char ch = str[cursor];
        if(LIKELY(ch>='0' && ch<='9'))
            val = val * 10 + ch - '0';
        else if(ch ==',')
            continue;
        else
            return val;
    }
    return (firstDigitIndex && str[firstDigitIndex - 1]=='-') ? -val : val;
}

inline int parseInteger(const char* str, int length, int nullValue){
    int cursor = 0;
    while(cursor < length && (str[cursor]<'0' || str[cursor]>'9')) ++cursor;
    if(cursor == length)
        return nullValue;
    int firstDigitIndex = cursor;
    int val = str[cursor] - '0';
    while(++cursor < length){
        char ch = str[cursor];
        if(LIKELY(ch>='0' && ch<='9'))
            val = val * 10 + ch - '0';
        else if(ch ==',')
            continue;
        else
            break;
    }
    return (firstDigitIndex && str[firstDigitIndex - 1]=='-') ? -val : val;
}

inline int parseInteger(const char* str){
    int sign = 1;
    if(*str=='-'){
        sign = -1;
        ++str;
    }
    else if(*str=='+')
        ++str;
    int val = 0;
    while(*str != 0){
        char ch = *str++;
        if(LIKELY(ch>='0' && ch<='9'))
            val = val * 10 + ch - '0';
        else
            return sign*val;
    }
    return sign*val;
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
bool parsePartialDate(const string &str, bool containDelimitor, int& part1, int& part2)
{
    if(str.length()<3)
        return false;
    unsigned start=0;
    if(Util::isLetter(str[0])){
        part1=parseEnglishMonth(str[0],str[1],str[2]);
        if(part1==0)
            return false;
        start=containDelimitor?4:3;
    }
    else{
        part1=str[0]-'0';
        if(Util::isDigit(str[1])){
            part1=part1*10+str[1]-'0';
            start=containDelimitor?3:2;
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
    if(str.length()<6)
        return false;
    int year,month;
    if(str.length()==6){
        char * pEnd;
        int tem = std::strtol(str.c_str(), &pEnd, 10);
        if(*pEnd == '\0') {
            month = tem % 100;
            if(month > 12)
                return false;
            year = tem / 100;
            intVal=year*12+month-1;
            return true;
        }else{
            return false;
        }
    }
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
    if(str.length() != 12 && str.length() != 9)
        return false;
    int hour, minute, second, millisecond;
    if(str.length() == 9) {
        char * pEnd;
        long long tem = std::strtoll(str.c_str(), &pEnd, 10);
        if(*pEnd == '\0') {
            millisecond = tem % 1000;
            tem /= 1000;
            second = tem % 100;
            tem /= 100;
            minute = tem % 100;
            hour = tem / 100;
            intVal=((hour*60+minute)*60+second)*1000 + millisecond;
            return true;
        }else{
            return false;
        }
    }
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
    if(str.length()<6)
        return false;
    int hour,minute,second;
    if(str.length() == 6){
        char * pEnd;
        int tem = std::strtol(str.c_str(), &pEnd, 10);
        if(*pEnd == '\0') {
            second = tem % 100;
            tem /= 100;
            minute = tem % 100;
            hour = tem / 100;
        }else{
            return false;
        }
    }
    else if(str.length() == 7){
        hour=(str[0]-'0');
        minute=(str[2]-'0')*10+str[3]-'0';
        second=(str[5]-'0')*10+str[6]-'0';
    }
    else{
        hour=(str[0]-'0')*10+str[1]-'0';
        minute=(str[3]-'0')*10+str[4]-'0';
        second=(str[6]-'0')*10+str[7]-'0';
    }
    if(hour>=24|| minute>=60 || second>=60)
        return false;
    intVal=(hour*60+minute)*60+second;
    return true;
}
bool minuteParser(const string &str, int &intVal){
    intVal=INT_MIN;
    int len = str.length();
    if(len != 4 && len != 5)
        return false;
    int hour,minute;
    if(len == 4){
        char * pEnd;
        int tem = std::strtol(str.c_str(), &pEnd, 10);
        if(*pEnd == '\0') {
            minute = tem % 100;
            hour = tem / 100;
            intVal=hour*60+minute;
            return true;
        }else{
            return false;
        }
    }
    hour=(str[0]-'0')*10+str[1]-'0';
    minute=(str[3]-'0')*10+str[4]-'0';
    if(hour>=24 || minute>=60)
        return false;
    intVal=hour*60+minute;
    return true;
}
bool datetimeParser(const string &str, int &intVal){
    intVal=INT_MIN;
    int len = str.length();
    if(len != 19 && len != 14)
        return false;
    int hour,minute,second;
    if(len == 14) {
        char * pEnd;
        long long tem = std::strtoll(str.c_str(), &pEnd, 10);
        if(*pEnd == '\0') {
            second = tem % 100;
            tem = tem / 100;
            minute = tem % 100;
            tem = tem / 100;
            hour = tem % 100;
            dateParser(str.substr(0, 8), intVal);
        }
    } else {
        int start=str.length()-8;
        while(start>=0 && (str[start]!=' ' && str[start]!='T')) --start;
        if(start<0)
            return false;
        int end=start-1;
        while(end>=0 && (str[end]==' ' || str[end]=='T')) --end;
        if(end<0)
            return false;
        dateParser(str.substr(0, end+1), intVal);
        string t = str.substr(start + 1);
        hour=(t[0]-'0')*10+t[1]-'0';
        minute=(t[3]-'0')*10+t[4]-'0';
        second=(t[6]-'0')*10+t[7]-'0';
    }
    if(intVal==INT_MIN)
        return false;
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
    int len = str.length();
    if(len != 23 && len != 29)
        return false;
    if(len == 23) {
        char * pEnd;
        long long tem = std::strtoll(str.c_str() + 8, &pEnd, 10);
        if(*pEnd == '\0') {
            int hour, minute, second, nanosecond;
            nanosecond = tem % 1000000000ll;
            tem = tem / 1000000000ll;
            second = tem % 100;
            tem = tem / 100;
            minute = tem % 100;
            tem = tem / 100;
            hour = tem % 100;
            int intVal;
            dateParser(str.substr(0, 8), intVal);
            longVal = intVal * 86400000000000ll + ((hour * 60 + minute) * 60 + second) * 1000000000ll + nanosecond;
            return true;
        }else{
            return false;
        }
    }
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
    int len = str.length();
    if(len != 17 && len != 23)
        return false;
    if(len == 17) {
        char * pEnd;
        long long tem = std::strtoll(str.c_str(), &pEnd, 10);
        if(*pEnd == '\0') {
            int hour,minute,second,millisecond;
            millisecond = tem % 1000;
            tem = tem / 1000;
            second = tem % 100;
            tem = tem / 100;
            minute = tem % 100;
            tem = tem / 100;
            hour = tem % 100;
            int intVal;
            dateParser(str.substr(0, 8), intVal);
            longVal=intVal*86400000ll+((hour*60+minute)*60+second)*1000+millisecond;
            return true;
        }else{
            return false;
        }
    }

    int start=str.length()-12;
    while(start>=0 && (str[start]!=' ' && str[start]!='T')) --start;
    if(start<0)
        return false;
    int intVal;
    dateParser(str.substr(0, 10), intVal);
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
    else
        return false;
    longVal=intVal*86400000ll+((hour*60+minute)*60+second)*1000+millisecond;
    return true;
}

static void
printRow(const std::vector<TRowResult> &rowResult)
{
    for (size_t i = 0; i < rowResult.size(); i++) {
        std::cout << "row: " << rowResult[i].row << ", cols: ";
        for (CellMap::const_iterator it = rowResult[i].columns.begin();
             it != rowResult[i].columns.end(); ++it) {
            std::cout << it->first << " => " << it->second.value << "; ";
        }
        std::cout << std::endl;
    }
}

static void hbaseConnectionOnClose(Heap *heap, vector<ConstantSP> &args) {
    HbaseConnect *conn = reinterpret_cast<HbaseConnect *>(args[0]->getLong());
    if(conn){
        conn->close();
        delete conn;
        args[0]->setLong(0);
    }
}

ConstantSP hbaseConnect(Heap *heap, vector<ConstantSP> &args){
    std::string usage = "Usage: connect(host, port, [isFramed], [timeout]). ";
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "host must be a string!");
    }
    if (args[1]->getType() != DT_INT || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "port must be an integer!");
    }
    bool isFramed = false;
    if(args.size() == 3){
        if (args[2]->getType() != DT_BOOL || args[2]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, usage + "isFramed must be a bool!");
        }
        isFramed = args[2]->getBool();
    }
    int timeout = 5000; //default is 5000ms
    if(args.size() == 4){
        if(args[3]->getType() != DT_INT || args[3]->getForm() != DF_SCALAR){
            throw IllegalArgumentException(__FUNCTION__ , usage + "timeout must be an integer!");
        }
        timeout = args[3]->getInt();
    }
    std::unique_ptr<HbaseConnect> conn(new HbaseConnect(args[0]->getString(), args[1]->getInt(), isFramed, timeout));
    FunctionDefSP onClose(Util::createSystemProcedure("hbase connection onClose()", hbaseConnectionOnClose, 1, 1));
    return Util::createResource((long long)conn.release(), "hbase connection", onClose, heap->currentSession());
}

ConstantSP showTables(Heap *heap, vector<ConstantSP> &args) {
    std::string usage = "Usage: showTables(hbaseConnection). ";
    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "hbase connection") {
        throw IllegalArgumentException(__FUNCTION__, usage + "hbaseConnection must be a hbase connection!");
    }
    HbaseConnect *conn = reinterpret_cast<HbaseConnect *>(args[0]->getLong());
    return conn->showTables();
}

ConstantSP load(Heap *heap, vector<ConstantSP> &args){
    std::string usage = "Usage: load(hbaseConnection, tableName, [schema]). ";
    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "hbase connection") {
        throw IllegalArgumentException(__FUNCTION__, usage + "hbaseConnection must be a hbase connection!");
    }
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "tableName must be a string!");
    }
    HbaseConnect *conn = reinterpret_cast<HbaseConnect *>(args[0]->getLong());
    if(args.size() == 3) {
        if (args[2]->getForm() != DF_TABLE ) {
            throw IllegalArgumentException(__FUNCTION__, usage + "schema must be a table!");
        }
        return conn->load(args[1]->getString(), args[2]);
    }
    return conn->load(args[1]->getString());
}

ConstantSP getRow(Heap *heap, vector<ConstantSP> &args){
    std::string usage = "Usage: getRow(hbaseConnection, tableName, rowKey, [columnName]). ";
    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "hbase connection") {
        throw IllegalArgumentException(__FUNCTION__, usage + "hbaseConnection must be a hbase connection!");
    }
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "tableName must be a string!");
    }
    if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "row must be a string!");
    }
    HbaseConnect *conn = reinterpret_cast<HbaseConnect *>(args[0]->getLong());
    if (args.size() == 4) {
        std::vector<std::string> columnNames;
        if(args[3]->getType() != DT_STRING && (args[3]->getForm() != DF_SCALAR || args[3]->getForm() != DF_VECTOR))
            throw IllegalArgumentException(__FUNCTION__, usage + "columnName must be a string or string vector!");
        if(args[3]->getForm() == DF_SCALAR){
            columnNames.emplace_back(args[3]->getString());
        }else{
            int columnSize = args[3]->size();
            for(int i = 0; i < columnSize; ++i){
                columnNames.emplace_back(args[3]->getString(i));
            }
        }
        return conn->getRow(args[1]->getString(), args[2]->getString(), columnNames);
    }
    return conn->getRow(args[1]->getString(), args[2]->getString());
}

ConstantSP deleteTable(Heap *heap, vector<ConstantSP> &args){
    std::string usage = "Usage: deleteTable(hbaseConnection, tableName). ";
    if (args[0]->getType() != DT_RESOURCE || args[0]->getString() != "hbase connection") {
        throw IllegalArgumentException(__FUNCTION__, usage + "hbaseConnection must be a hbase connection!");
    }
    if (args[1]->getType() != DT_STRING || (args[1]->getForm() != DF_SCALAR && args[1]->getForm() != DF_VECTOR)){
        throw IllegalArgumentException(__FUNCTION__, usage + "tableName must be a string!");
    }
    HbaseConnect *conn = reinterpret_cast<HbaseConnect *>(args[0]->getLong());
    vector<string> tablenames;
    if (args[1]->getForm() == DF_VECTOR){
        for(int i = 0; i < args[1]->size(); ++i) {
            conn->deleteTable(args[1]->getString(i));
        }
    }else{
        conn->deleteTable(args[1]->getString());
    }
    return new Void();
}

HbaseConnect::HbaseConnect(const string &hostname, const int port, bool isFramed, int timeout):host_(hostname), port_(port){
    using namespace apache::thrift::transport;
    socket_ = std::make_shared<TSocket>(hostname, port);
    socket_->setConnTimeout(timeout);
    if (isFramed) {
        transport_ = std::make_shared<TFramedTransport>(socket_);
    } else {
        transport_ = std::make_shared<TBufferedTransport>(socket_);
    }
    protocol_ = std::make_shared<TBinaryProtocol>(transport_);
    client_ = std::make_shared<HbaseClient>(protocol_);
    try{
        transport_->open();
    }catch(const TException &tx) {
        throw RuntimeException(string("HBase: ") + tx.what());
    }
}

ConstantSP HbaseConnect::showTables() {
    LockGuard<Mutex> lk(&mtx_);
    StrVec tables;
    try{
        client_->getTableNames(tables);
    }catch (TException &tx){
        throw RuntimeException(string("HBase getTableNames error: ") + tx.what());
    }
    VectorSP ret = Util::createVector(DT_STRING, 0, tables.size());
    ret->appendString(tables.data(), tables.size());
    return ret;
}

ConstantSP HbaseConnect::load(const std::string &tableName) {
    LockGuard<Mutex> lk(&mtx_);
    bool found = false;
    StrVec tables;
    try{
        client_->getTableNames(tables);
    }catch (TException &tx){
        throw RuntimeException(string("HBase getTableNames error: ") + tx.what());
    }
    const std::map<Text, Text>  dummyAttributes;
    StrVec columnNames;
    int scanner;
    for (int i = 0; i < tables.size(); ++i) {
        if (tableName == tables[i]) {
            TableSP result;
            try{
                scanner = client_->scannerOpen(tableName, "", columnNames, dummyAttributes);
            }catch (TException &tx){
                throw RuntimeException(string("HBase scannerOpen error: ") + tx.what());
            }
            vector<string> columnNames= {"row"};
            bool first = true;
            try {
                while (true) {
                    std::vector<TRowResult> value;
                    client_->scannerGetList(value, scanner, 1024);
                    if (value.size() == 0){
                        if(first){
                            return new Void();
                        }
                        break;
                    }
                    vector<ConstantSP> columns;
                    for (size_t i = 0; i < value.size(); i++) {
                        std::vector<ConstantSP> dataToAppend;
                        if(first){
                            columns.emplace_back(new String(value[i].row));
                        }else{
                            dataToAppend.emplace_back(new String(value[i].row));
                        }

                        for (CellMap::const_iterator it = value[i].columns.begin();
                             it != value[i].columns.end(); ++it) {
                            if(first){
                                columnNames.emplace_back(it->first);
                                columns.emplace_back(new String(it->second.value));
                            }else{
                                dataToAppend.emplace_back(new String(it->second.value));
                            }
                        }
                        if(first) {
                            result = Util::createTable(columnNames, columns);
                            first = false;
                        }else{
                            INDEX insertedRows;
                            std::string errMsg;
                            bool success = result->append(dataToAppend, insertedRows, errMsg);
                            if (!success){
                                std::cerr << errMsg << std::endl;
                                LOG_ERR(errMsg);
                            }
                        }
                    }
                    found = true;
                }
            } catch (const IOError &ioe) {
                std::cerr << "FATAL: Scanner raised IOError" << std::endl;
                LOG_ERR(string("FATAL: Scanner raised IOError") + ioe.what());
            }
            client_->scannerClose(scanner);
            return result;
        }
    }
    if(!found){
        throw RuntimeException("Table " + tableName + " is not found!");
    }
}

ConstantSP HbaseConnect::load(const std::string &tableName, TableSP schema) {
    LockGuard<Mutex> lk(&mtx_);
    bool found = false;
    VectorSP vecName=schema->getColumn("name");
    if(vecName==nullptr){
        throw IllegalArgumentException(__FUNCTION__, "There is no column \"name\" in schema table");
    }
    if(vecName->getType()!=DT_STRING){
        throw IllegalArgumentException(__FUNCTION__, "The schema table column \"name\" type must be STRING");
    }
    VectorSP vecType=schema->getColumn("type");
    if(vecType==nullptr){
        throw IllegalArgumentException(__FUNCTION__, "There is no column \"type\" in schema table");
    }
    if(vecType->getType()!=DT_STRING){
        throw IllegalArgumentException(__FUNCTION__, "The schema table column \"type\" type must be STRING");
    }
    if(vecName->size()!=vecType->size()){
        throw IllegalArgumentException(__FUNCTION__, "The schema table column \"name\" and \"type\" size are not equal");
    }
    int colNums = vecName->size();
    StrVec colNames{"row"};
    StrVec columnNames;
    vector<ConstantSP> cols;
    vector<DATA_TYPE>  colType;
    colType.emplace_back(DT_STRING);
    cols.resize(colNums + 1);
    cols[0] = Util::createVector(DT_STRING, 0);
    for(int i = 1; i < colNums + 1; ++i){
        colNames.emplace_back(vecName->getString(i-1));
        columnNames.emplace_back(vecName->getString(i-1));
        string sType=vecType->getString(i-1);
        std::transform(sType.begin(),sType.end(),sType.begin(),::toupper);
        if(sType=="BOOL"){
            colType.push_back(DT_BOOL);
            cols[i]=Util::createVector(DT_BOOL,0);
        }
        else if(sType=="CHAR"){
            colType.push_back(DT_CHAR);
            cols[i]=Util::createVector(DT_CHAR,0);
        }
        else if(sType=="SHORT"){
            colType.push_back(DT_SHORT);
            cols[i]=Util::createVector(DT_SHORT,0);
        }
        else if(sType=="INT"){
            colType.push_back(DT_INT);
            cols[i]=Util::createVector(DT_INT,0);
        }
        else if(sType=="LONG"){
            colType.push_back(DT_LONG);
            cols[i]=Util::createVector(DT_LONG,0);
        }
        else if(sType=="DATE"){
            colType.push_back(DT_DATE);
            cols[i]=Util::createVector(DT_DATE,0);
        }
        else if(sType=="MONTH"){
            colType.push_back(DT_MONTH);
            cols[i]=Util::createVector(DT_MONTH,0);
        }
        else if(sType=="TIME"){
            colType.push_back(DT_TIME);
            cols[i]=Util::createVector(DT_TIME,0);
        }
        else if(sType=="MINUTE"){
            colType.push_back(DT_MINUTE);
            cols[i]=Util::createVector(DT_MINUTE,0);
        }
        else if(sType=="SECOND"){
            colType.push_back(DT_SECOND);
            cols[i]=Util::createVector(DT_SECOND,0);
        }
        else if(sType=="DATETIME"){
            colType.push_back(DT_DATETIME);
            cols[i]=Util::createVector(DT_DATETIME,0);
        }
        else if(sType=="TIMESTAMP"){
            colType.push_back(DT_TIMESTAMP);
            cols[i]=Util::createVector(DT_TIMESTAMP,0);
        }
        else if(sType=="NANOTIME"){
            colType.push_back(DT_NANOTIME);
            cols[i]=Util::createVector(DT_NANOTIME,0);
        }
        else if(sType=="NANOTIMESTAMP"){
            colType.push_back(DT_NANOTIMESTAMP);
            cols[i]=Util::createVector(DT_NANOTIMESTAMP,0);
        }
        else if(sType=="FLOAT"){
            colType.push_back(DT_FLOAT);
            cols[i]=Util::createVector(DT_FLOAT,0);
        }
        else if(sType=="DOUBLE"){
            colType.push_back(DT_DOUBLE);
            cols[i]=Util::createVector(DT_DOUBLE,0);
        }
        else if(sType=="SYMBOL"){
            colType.push_back(DT_SYMBOL);
            cols[i]=Util::createVector(DT_SYMBOL,0);
        }
        else if(sType=="STRING"){
            colType.push_back(DT_STRING);
            cols[i]=Util::createVector(DT_STRING,0);
        }
        else{
            throw IllegalArgumentException(__FUNCTION__, "The Type "+sType+" is not supported");
        }
    }
    TableSP result = Util::createTable(colNames, colType, 0, 10);
    StrVec tables;
    try{
        client_->getTableNames(tables);
    }catch (TException &tx){
        throw RuntimeException(string("HBase getTableNames error: ") + tx.what());
    }
    const std::map<Text, Text>  dummyAttributes;
    int scanner;
    StrVec emptyVec;
    for(int i = 0; i < tables.size(); ++i){
        if(tableName == tables[i]) {
            try{
                scanner = client_->scannerOpen(tableName, "", emptyVec, dummyAttributes);
                //scanner = client_->scannerOpen(tableName, "", columnNames, dummyAttributes);
            }catch (TException &tx){
                throw RuntimeException(string("HBase scannerOpen error: ") + tx.what());
            }
            while (true) {
                std::vector<TRowResult> value;
                try{
                    client_->scannerGetList(value, scanner, 1024);
                }catch (TException &tx){
                    throw RuntimeException(string("HBase scannerGetList error: ") + tx.what());
                }
                if (value.size() == 0)
                    break;
                for (size_t i = 0; i < value.size(); i++) {
                    std::vector<ConstantSP> dataToAppend;
                    dataToAppend.emplace_back(new String(value[i].row));
                    for(int j = 1; j < colNums + 1; ++j){
                        auto cell = value[i].columns[colNames[j]];
                        if (cell.value == "") {
                            dataToAppend.emplace_back(new Void());
                            continue;
                        }
                        switch(colType[j]){
                            case DT_BOOL:{
                                string tem(cell.value);
                                std::transform(tem.begin(),tem.end(),tem.begin(),::toupper);
                                if(tem == "TRUE" || tem == "1"){
                                    dataToAppend.emplace_back(new Bool(1));
                                    break;
                                }else if(tem == "FALSE" || tem == "0"){
                                    dataToAppend.emplace_back(new Bool(0));
                                    break;
                                }else{
                                    dataToAppend.emplace_back(new Void());
                                    break;
                                }
                            }
                            case DT_CHAR:{
                                dataToAppend.emplace_back(new Char(cell.value[0]));
                                break;
                            }
                            case DT_SHORT:{
                                char *pEnd;
                                int tem = std::strtol(cell.value.c_str(), &pEnd, 10);
                                if(pEnd == cell.value.c_str()) {
                                    dataToAppend.emplace_back(new Void());
                                }else{
                                    dataToAppend.emplace_back(new Short(tem));
                                }
                                break;
                            }
                            case DT_INT:{
                                char *pEnd;
                                int tem = std::strtol(cell.value.c_str(), &pEnd, 10);
                                if(pEnd == cell.value.c_str()) {
                                    dataToAppend.emplace_back(new Void());
                                }else{
                                    dataToAppend.emplace_back(new Int(tem));
                                }
                                break;
                            }
                            case DT_LONG:{
                                char *pEnd;
                                long long tem = std::strtoll(cell.value.c_str(), &pEnd, 10);
                                if(pEnd == cell.value.c_str()) {
                                    dataToAppend.emplace_back(new Void());
                                }else{
                                    dataToAppend.emplace_back(new Long(tem));
                                }
                                break;
                            }
                            case DT_FLOAT:{
                                char *pEnd;
                                float tem = std::strtof(cell.value.c_str(), &pEnd);
                                if(pEnd == cell.value.c_str()) {
                                    dataToAppend.emplace_back(new Void());
                                }else{
                                    dataToAppend.emplace_back(new Float(tem));
                                }
                                break;
                            }
                            case DT_DOUBLE:{
                                char *pEnd;
                                double tem = std::strtod(cell.value.c_str(), &pEnd);
                                if(pEnd == cell.value.c_str()) {
                                    dataToAppend.emplace_back(new Void());
                                }else{
                                    dataToAppend.emplace_back(new Double(tem));
                                }
                                break;
                            }
                            case DT_SYMBOL:
                            case DT_STRING:{
                                dataToAppend.emplace_back(new String(cell.value));
                                break;
                            }
                            case DT_TIMESTAMP:{
                                long long tem;
                                if(timestampParser(cell.value, tem)){
                                    dataToAppend.emplace_back(new Timestamp(tem));
                                }else{
                                    dataToAppend.emplace_back(new Void());
                                }
                                break;
                            }
                            case DT_NANOTIME:{
                                long long tem;
                                if(nanotimeParser(cell.value, tem)){
                                    dataToAppend.emplace_back(new NanoTime(tem));
                                }else{
                                    dataToAppend.emplace_back(new Void());
                                }
                                break;
                            }
                            case DT_NANOTIMESTAMP:{
                                long long tem;
                                if(nanotimestampParser(cell.value, tem)){
                                    dataToAppend.emplace_back(new NanoTimestamp(tem));
                                }else{
                                    dataToAppend.emplace_back(new Void());
                                }
                                break;
                            }
                            case DT_DATETIME:{
                                int tem;
                                if(datetimeParser(cell.value, tem)){
                                    dataToAppend.emplace_back(new DateTime(tem));
                                }else{
                                    dataToAppend.emplace_back(new Void());
                                }
                                break;
                            }
                            case DT_MINUTE:{
                                int tem;
                                if(minuteParser(cell.value, tem)){
                                    dataToAppend.emplace_back(new Minute(tem));
                                }else{
                                    dataToAppend.emplace_back(new Void());
                                }
                                break;
                            }
                            case DT_SECOND:{
                                int tem;
                                if(secondParser(cell.value, tem)){
                                    dataToAppend.emplace_back(new Second(tem));
                                }else{
                                    dataToAppend.emplace_back(new Void());
                                }
                                break;
                            }
                            case DT_TIME:{
                                int tem;
                                if(timeParser(cell.value, tem)){
                                    dataToAppend.emplace_back(new Time(tem));
                                }else{
                                    dataToAppend.emplace_back(new Void());
                                }
                                break;
                            }
                            case DT_MONTH:{
                                int tem;
                                if(monthParser(cell.value, tem)){
                                    dataToAppend.emplace_back(new Month(tem));
                                }else{
                                    dataToAppend.emplace_back(new Void());
                                }
                                break;
                            }
                            case DT_DATE:{
                                int tem;
                                if(dateParser(cell.value, tem)){
                                    dataToAppend.emplace_back(new Date(tem));
                                }else{
                                    dataToAppend.emplace_back(new Void());
                                }
                                break;
                            }
                            default:{
                                client_->scannerClose(scanner);
                                throw IllegalArgumentException(__FUNCTION__, "The Type "+ vecType->getString(j-1) + " is not supported");
                            }
                        }
                    }
                    INDEX insertedRows;
                    std::string errMsg;
                    bool success = result->append(dataToAppend, insertedRows, errMsg);
                    if (!success){
                        client_->scannerClose(scanner);
                        throw RuntimeException("Error when append table: " + errMsg);
                    }
                }
            }
            found = true;
            client_->scannerClose(scanner);
            return result;
        }
    }
    if(!found){
        throw RuntimeException("Table " + tableName + " is not found!");
    }
}

ConstantSP HbaseConnect::getRow(const std::string &tableName, const std::string &rowKey) {
    LockGuard<Mutex> lk(&mtx_);
    bool found = false;
    try{
        StrVec tables;
        client_->getTableNames(tables);
        for (StrVec::const_iterator it = tables.begin(); it != tables.end(); ++it) {
            if (tableName == *it) {
                std::vector<TRowResult> rowResult;
                const std::map<Text, Text> dummyAttributes;
                client_->getRow(rowResult, tableName, rowKey, dummyAttributes);
                vector<string> columnNames= {"row"};
                vector<ConstantSP> columns;
                for (size_t i = 0; i < rowResult.size(); i++) {
                    columns.emplace_back(new String(rowResult[i].row));
                    for (CellMap::const_iterator it = rowResult[i].columns.begin();
                         it != rowResult[i].columns.end(); ++it) {
                        columnNames.emplace_back(it->first);
                        columns.emplace_back(new String(it->second.value));
                    }
                }
                return Util::createTable(columnNames, columns);
            }
        }
    }catch(const TException &tx) {
        throw RuntimeException(string("HBase: ") + tx.what());
    }
    if(!found){
        throw RuntimeException("Table " + tableName + " is not found!");
    }
}

ConstantSP HbaseConnect::getRow(const std::string &tableName, const std::string &rowKey,
                                const std::vector<std::string>& columnNames) {
    LockGuard<Mutex> lk(&mtx_);
    bool found = false;
    try{
        StrVec tables;
        client_->getTableNames(tables);
        for (StrVec::const_iterator it = tables.begin(); it != tables.end(); ++it) {
            if (tableName == *it) {
                std::vector<TRowResult> rowResult;
                const std::map<Text, Text>  dummyAttributes;
                client_->getRowWithColumns(rowResult, tableName, rowKey, columnNames, dummyAttributes);
                vector<string> columnNames= {"row"};
                vector<ConstantSP> columns;
                for (size_t i = 0; i < rowResult.size(); i++) {
                    columns.emplace_back(new String(rowResult[i].row));
                    for (CellMap::const_iterator it = rowResult[i].columns.begin();
                         it != rowResult[i].columns.end(); ++it) {
                        columnNames.emplace_back(it->first);
                        columns.emplace_back(new String(it->second.value));
                    }
                }
                return Util::createTable(columnNames, columns);
            }
        }
    }catch(const TException &tx) {
        throw RuntimeException(string("HBase: ") + tx.what());
    }
    if(!found){
        throw RuntimeException("Table " + tableName + " is not found!");
    }
}

void HbaseConnect::deleteTable(const std::string &tableName) {
    LockGuard<Mutex> lk(&mtx_);
    bool found = false;
    try{
        StrVec tables;
        client_->getTableNames(tables);
        for (StrVec::const_iterator it = tables.begin(); it != tables.end(); ++it) {
            if (tableName == *it) {
                if (client_->isTableEnabled(*it)) {
                    client_->disableTable(*it);
                }
                client_->deleteTable(*it);
                found = true;
                break;
            }
        }
    }catch(const TException &tx) {
        throw RuntimeException(string("HBase: ") + tx.what());
    }
    if(!found){
        throw RuntimeException("Table " + tableName + " is not found!");
    }
}

void HbaseConnect::close() {
    LockGuard<Mutex> lk(&mtx_);
    try{
        transport_->close();
        socket_->close();
    }catch(const TException &tx) {
        throw RuntimeException(string("HBase: ") + tx.what());
    }
}