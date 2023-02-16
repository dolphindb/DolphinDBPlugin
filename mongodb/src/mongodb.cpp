#include"mongodb.h"
#include <Exceptions.h> 
#include <ScalarImp.h>
#include <Util.h>
#include <string>
#include<unordered_map>
#include <bson.h>
#include <mongoc.h>
#include "json.hpp"
#include "cvt.h"
using namespace std;

string getBsonString(bson_type_t type){
    switch(type){
        case BSON_TYPE_ARRAY:
            return "BSON ARRAY";
        case BSON_TYPE_BINARY:
            return "BSON BINARY";
        case BSON_TYPE_BOOL:
            return "BSON BOOL";
        case BSON_TYPE_CODE:
            return "BSON CODE";
        case BSON_TYPE_CODEWSCOPE:
            return "BSON CODEWSCOPE";
        case BSON_TYPE_DATE_TIME:
            return "BSON DATE_TIME";
        case BSON_TYPE_DBPOINTER:
            return "BSON DBPOINTER";
        case BSON_TYPE_DECIMAL128:
            return "BSON DECIMAL128";
        case BSON_TYPE_DOCUMENT:
            return "BSON DOCUMENT";
        case BSON_TYPE_DOUBLE:
            return "BSON DOUBLE";
        case BSON_TYPE_EOD:
            return "BSON EOD";
        case BSON_TYPE_INT32:
            return "BSON INT32";
        case BSON_TYPE_INT64:
            return "BSON INT64";
        case BSON_TYPE_MAXKEY:
            return "BSON MAXKEY";
        case BSON_TYPE_MINKEY:
            return "BSON MINKEY";
        case BSON_TYPE_NULL:
            return "BSON NULL";
        case BSON_TYPE_OID:
            return "BSON OID";
        case BSON_TYPE_REGEX:
            return "BSON REGEX";
        case BSON_TYPE_SYMBOL:
            return "BSON SYMBOL";
        case BSON_TYPE_TIMESTAMP:
            return "BSON TIMESTAMP";
        case BSON_TYPE_UNDEFINED:
            return "BSON UNDEFINED";
        case BSON_TYPE_UTF8:
            return "BSON UTF8";
        default:
            return "BSON UNKNOWN";
    }
    return "BSON UNKNOWN";
} 

static bool isMongoInit=false;
static Mutex lock;
ConstantSP messageSP(const std::string &s) {
    auto message = Util::createConstant(DT_STRING);
    message->setString(s);
    return message;
}   

vector<ConstantSP> getArgs(vector<ConstantSP> &args, size_t nMaxArgs) {
    auto ret = vector<ConstantSP>(nMaxArgs);
    for (size_t i = 0; i < nMaxArgs; ++i) {
        if (args.size() >= i + 1)
            ret[i] = args[i];
        else
            ret[i] = Util::createNullConstant(DT_VOID);
    }
    return ret;
}

class mongoConnection{
    public:
    mongoConnection(std::string hostname, int port, std::string username, std::string password, std::string database);
    public:
        std::string str() { return "mongodb://"+user_+":"+password_+"@"+host_+":"+std::to_string(port_)+"/?authSource="+db_; }
    std::string host_;
    std::string user_;
    std::string password_;
    std::string db_;
    mongoc_client_t      *mclient_=NULL;
    mongoc_database_t    *mdatabase_=NULL;
    int port_=27017;
    Mutex mtx_;
    bool initialized_ = false;
    TableSP extractLoad(std::string& collection,std::string& condition,std::string& option,TableSP& schema, bool aggregate);
    TableSP load(std::string collection,std::string condition,std::string option,TableSP schema, bool aggregate);
    VectorSP mongodbGetCollectionNames(string database);
    ~mongoConnection();
};

mongoConnection::~mongoConnection(){
    mongoc_client_destroy(mclient_);
    mongoc_database_destroy(mdatabase_);
}

ConstantSP safeOp(const ConstantSP &arg, std::function<ConstantSP(mongoConnection *)> &&f) {
    if (arg->getType() == DT_RESOURCE&&arg->getLong()!=0) {
        auto conn = (mongoConnection *)(arg->getLong());
        return f(conn);
    } else {
        throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
    }
}


mongoConnection::mongoConnection(std::string hostname, int port, std::string username, std::string password, std::string database)
:host_(hostname),user_(username),password_(password),db_(database),port_(port){
    if(!isMongoInit){
        LockGuard<Mutex> lockGuard(&lock);
        if(!isMongoInit){
            mongoc_init ();
            isMongoInit = true;
        }
    }
    std::string strUri;
    if(user_==""&&password_==""){
            strUri="mongodb://"+host_+":"+std::to_string(port_);
    }
    else{
        if(db_==""){
            db_="admin";
            strUri="mongodb://"+user_+":"+password_+"@"+host_+":"+std::to_string(port_)+"/?authSource="+db_;
        }
        else
        strUri="mongodb://"+user_+":"+password_+"@"+host_+":"+std::to_string(port_)+"/?authSource="+db_;
    }
    mclient_ = mongoc_client_new (strUri.c_str());
    if(mclient_==NULL)throw IllegalArgumentException(__FUNCTION__, "User id or password is incorrect for the given database");
    mdatabase_ = mongoc_client_get_database (mclient_, db_.c_str());
    if(mdatabase_==NULL){
        mongoc_client_destroy(mclient_);
        throw IllegalArgumentException(__FUNCTION__, "User id or password is incorrect for the given database");
    }
    if(user_==""&&password_==""){
        char **test=mongoc_client_get_database_names_with_opts(mclient_,NULL,NULL);
        Defer df1([=](){bson_strfreev(test);});
        if(!test){
            mongoc_client_destroy(mclient_);
            mongoc_database_destroy(mdatabase_);
            throw IllegalArgumentException(__FUNCTION__, "User id or password is incorrect for the given database");
        }
        return;
    }
    bson_error_t  errors;
    bson_t *command=BCON_NEW ("ping", BCON_INT32 (1));
    bool retval = mongoc_client_command_simple (mclient_, db_.c_str(), command, NULL, NULL, &errors);
    bson_destroy (command);
    if(!retval){
        mongoc_client_destroy(mclient_);
        mongoc_database_destroy(mdatabase_);
        std::string strError=errors.message;
        throw IllegalArgumentException(__FUNCTION__, "User id or password is incorrect for the given database");
    }
}

static void mongoConnectionOnClose(Heap *heap, vector<ConstantSP> &args) {
    delete (mongoConnection *)(args[0]->getLong());
}

ConstantSP mongodbClose(const ConstantSP &handle, const ConstantSP &b){
    std::string usage = "Usage: close(conn). ";
    mongoConnection *cp = NULL;
    if (handle->getType() == DT_RESOURCE) {
        cp = (mongoConnection *)(handle->getLong());
    } else {
        throw IllegalArgumentException(__FUNCTION__, "Invalid connection object.");
    }

    if (cp != nullptr) {
        delete cp;
        handle->setLong(0);
        return new Bool(true);
    }
    return new Bool(false);
}

ConstantSP mongodbConnect(Heap *heap, vector<ConstantSP> &args) {
    std::string usage = "Usage: connect(host, port, user, password, db). ";
    // parse args first
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "host must be a string");
    }
    if (args[1]->getType() != DT_INT || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "port must be an integer");
    }
    if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "user must be a string");
    }
    if (args[3]->getType() != DT_STRING || args[3]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "password must be a string");
    }
    string database;
    if(args.size()==5){
        if (args[4]->getType() != DT_STRING || args[4]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, usage + "Db must be a string");
        }
        else database=args[4]->getString();
    }
    std::unique_ptr<mongoConnection> cup(new mongoConnection(args[0]->getString(), args[1]->getInt(), args[2]->getString(), args[3]->getString(), database));
    const char *fmt = "mongodb connection to [%s]";
    vector<char> descBuf(cup->str().size() + strlen(fmt));
    sprintf(descBuf.data(), fmt, cup->str().c_str());
    FunctionDefSP onClose(Util::createSystemProcedure("mogodb connection onClose()", mongoConnectionOnClose, 1, 1));
    return Util::createResource((long long)cup.release(), descBuf.data(), onClose, heap->currentSession());
}

ConstantSP mongodbLoad(Heap *heap, vector<ConstantSP> &arguments) {
    auto args = getArgs(arguments, 4);
    std::string usage = "Usage: load(connection, condition,option, [schema]).";
    std::string collection,condition,option;
    TableSP schema = nullptr;
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "collcetionName must be a string");
    } else {
        collection = args[1]->getString();
    }
    if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "Condition must be a string");
    } else {
        condition = args[2]->getString();
    }
    if (args[3]->getType() != DT_STRING || args[3]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "Option must be a string");
    } else {
        option = args[3]->getString();
    }
    if (arguments.size()==5&&!arguments[4]->isNothing()) {
        if (!arguments[4]->isTable()) {
            throw IllegalArgumentException(__FUNCTION__, usage + "schema must be a table");
        }
        schema = arguments[4];
    }
    return safeOp(args[0], [&](mongoConnection *conn) { return conn->load(collection,condition,option,schema, false); });
}

ConstantSP mongodbAggregate(Heap *heap, vector<ConstantSP> &arguments) {
    auto args = getArgs(arguments, 4);
    std::string usage = "Usage: aggregate(connection, condition,option, [schema]).";
    std::string collection,condition,option;
    TableSP schema = nullptr;
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "collcetionName must be a string");
    } else {
        collection = args[1]->getString();
    }
    if (args[2]->getType() != DT_STRING || args[2]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "Pipeline must be a string");
    } else {
        condition = args[2]->getString();
    }
    if (args[3]->getType() != DT_STRING || args[3]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "Option must be a string");
    } else {
        option = args[3]->getString();
    }
    if (arguments.size()==5&&!arguments[4]->isNothing()) {
        if (!arguments[4]->isTable()) {
            throw IllegalArgumentException(__FUNCTION__, usage + "schema must be a table");
        }
        schema = arguments[4];
    }
    return safeOp(args[0], [&](mongoConnection *conn) { return conn->load(collection,condition,option,schema, true); });
}

void conversionStr(vector<std::string>& colName){
    int len=colName.size();
    for(int i=0;i<len;++i){
        std::u16string tmp= utf8_to_utf16(colName[i]);
        int subLen=tmp.size();
        for(int j=0;j<subLen;++j){
            wchar_t t=tmp[j];
            if(0<=t&&t<128){
                if(!((L'A'<=t&&t<=L'z')||(L'0'<=t&&t<=L'9')||t=='_')){
                    tmp[j]=L'_';
                }
            }
        }
        if(tmp[0]==L'_')tmp=(char16_t)L'c' + tmp;
        colName[i]=utf16_to_utf8(tmp);
    }
}

void realLoad(vector<std::string>& colName,vector<DATA_TYPE>&  colType,vector<ConstantSP>& cols,
    bool schemaEx,int mIndex,
    vector<char*>& buffer,unordered_map<string,vector<string>>& mmap,unordered_map<int,long long>& nullMap,
    bson_t *bsonQuery,bson_t *boption,mongoc_cursor_t *cursor,mongoc_collection_t *mcollection){
    int len=schemaEx?colName.size():0;
    const bson_t* doc;
    long long curNum=mongoc_cursor_get_limit(cursor);
    long long rowSum=0;
    int index=0;
    bool first=true;
    char busf[30];
    while(mongoc_cursor_next(cursor,&doc)){
        rowSum++;
        bson_iter_t biter;
        bson_iter_init(&biter,doc);
        if(!schemaEx&&first){
            first=false;
            while(bson_iter_next(&biter)){
                const char* ss=bson_iter_key(&biter);
                colName.push_back(ss);
                bson_type_t btype=bson_iter_type(&biter);
                switch(btype){
                    case BSON_TYPE_DOUBLE:{
                        colType.push_back(DT_DOUBLE);
                        char* ptr=(char*)malloc(mIndex*sizeof(double));
                        if(ptr == nullptr)    throw std::bad_alloc();
                        buffer.push_back(ptr);
                        double t=bson_iter_double(&biter);
                        *((double*)(buffer[len])+index)=t;
                        break;
                    }
                    case BSON_TYPE_UTF8:{
                        colType.push_back(DT_STRING);
                        char* ptr=NULL;
                        buffer.push_back(ptr);
                        mmap[ss]=vector<string>();
                        mmap[ss].reserve(mIndex);
                        std::string t=bson_iter_utf8(&biter,NULL);
                        mmap[ss].push_back(t);
                        break;
                    }
                    case BSON_TYPE_INT32:{
                        colType.push_back(DT_INT);
                        char* ptr=(char*)malloc(mIndex*sizeof(int));
                        if(ptr == nullptr)    throw std::bad_alloc();
                        buffer.push_back(ptr);
                        int t=bson_iter_int32(&biter);
                        *((int*)(buffer[len])+index)=t;
                        break;
                    }
                    case BSON_TYPE_INT64:{
                        colType.push_back(DT_LONG);
                        char* ptr=(char*)malloc(mIndex*sizeof(long long ));
                        if(ptr == nullptr)    throw std::bad_alloc();
                        buffer.push_back(ptr);
                        long long  t=bson_iter_int64(&biter);
                        *((long long*)(buffer[len])+index)=t;
                        break;
                    }
                    case BSON_TYPE_OID:{
                        colType.push_back(DT_STRING);
                        char* ptr=NULL;
                        buffer.push_back(ptr);
                        mmap[ss]=vector<string>();
                        mmap[ss].reserve(mIndex);
                        char tmp[25];
                        bson_oid_to_string(bson_iter_oid(&biter),tmp);
                        std::string ttmp(tmp);
                        mmap[colName[len]].push_back(ttmp);
                        break;
                    }
                    case BSON_TYPE_BOOL:{
                        colType.push_back(DT_BOOL);
                        char* ptr=(char*)malloc(mIndex*sizeof(bool));
                        if(ptr == nullptr)    throw std::bad_alloc();
                        buffer.push_back(ptr);
                        bool t=bson_iter_bool(&biter);
                        *((char*)(buffer[len])+index)=t;
                        break;
                    }
                    case BSON_TYPE_DATE_TIME:{
                        colType.push_back(DT_TIMESTAMP);
                        char* ptr=(char*)malloc(mIndex*sizeof(long long ));
                        if(ptr == nullptr)    throw std::bad_alloc();
                        buffer.push_back(ptr);
                        long long t=bson_iter_date_time(&biter);
                        *((long long*)(buffer[len])+index)=t;
                        break;
                    }
                    case BSON_TYPE_DECIMAL128:{
                        colType.push_back(DT_DOUBLE);
                        char* ptr=(char*)malloc(mIndex*sizeof(double));
                        if(ptr == nullptr)    throw std::bad_alloc();
                        buffer.push_back(ptr);
                        bson_decimal128_t dec ;
                        bson_iter_decimal128(&biter,&dec);
                        char str[100];
                        bson_decimal128_to_string(&dec,str);
                        std::string ttmp(str);
                        try{
                            double t=std::stod(ttmp);
                            *((double*)(buffer[len])+index)=t;
                        }
                        catch(std::exception& e){
                            *((double*)(buffer[len])+index)=DBL_NMIN;
                        }
                        break;
                    }
                    case BSON_TYPE_SYMBOL:{
                        colType.push_back(DT_STRING);
                        char* ptr=NULL;
                        buffer.push_back(ptr);
                        mmap[ss]=vector<string>();
                        mmap[ss].reserve(mIndex);
                        std::string t=bson_iter_symbol(&biter,NULL);
                        mmap[ss].push_back(t);
                        break;
                    }
                    case BSON_TYPE_NULL:{
                        //NULL Mark
                        colType.push_back(DT_HANDLE);
                        char* ptr=NULL;
                        buffer.push_back(ptr);
                        nullMap[len]=1;
                        break;
                    }
                    default: {
                        throw RuntimeException("The Mongodb value whose key is " + string(ss) + " is of type " + getBsonString(btype) + " and is not supported. ");
                    }
                }
                len++;
            }
            cols.resize(len);
            for(int i=0;i<len;++i){
                if(colType[i]!=DT_HANDLE)
                    cols[i]=Util::createVector(colType[i],0,curNum);
            }
        }   
        else{
            for(int i=0;i<len;++i){
                VectorSP vec=cols[i];
                bool exist=bson_iter_find(&biter,colName[i].c_str());
                if(!exist){
                    bson_iter_init(&biter,doc);
                    exist=bson_iter_find(&biter,colName[i].c_str());
                }
                if(exist){
                    bson_type_t btype=bson_iter_type(&biter);
                    DATA_TYPE dtype=colType[i];
                    if(nullMap.find(i)!=nullMap.end()){
                        long long rNum=nullMap[i];
                        switch(btype){
                            case BSON_TYPE_DOUBLE:{
                                colType[i]=DT_DOUBLE;
                                cols[i]=Util::createVector(DT_DOUBLE,0,rNum);
                                char* ptr=(char*)malloc(mIndex*sizeof(double));
                                if(ptr == nullptr)    throw std::bad_alloc();
                                buffer[i]=ptr;
                                if(rNum<mIndex){
                                    for(long long j=0;j<rNum;++j){
                                        *((double*)ptr+j)=DBL_NMIN;
                                    }
                                }
                                else{
                                    for(long long  j=0;j<mIndex;++j){
                                        *((double*)ptr+j)=DBL_NMIN;
                                    }
                                    long long rwNum=rNum/mIndex;
                                    for(long long j=0;j<rwNum;++j){
                                        vec->appendDouble((double*)ptr,mIndex);
                                    }
                                }
                                *((double*)ptr+rNum%mIndex)=bson_iter_double(&biter);
                                nullMap.erase(i);
                                break;
                            }
                            case BSON_TYPE_UTF8:{
                                colType[i]=DT_STRING;
                                cols[i]=Util::createVector(DT_STRING,0,rNum);
                                mmap[colName[i]]=vector<string>(rNum%mIndex);
                                if(rNum>=mIndex){
                                    vector<string>tmp(mIndex);
                                    long long rwNum=rNum/mIndex;
                                    for(long long j=0;j<rwNum;++j){
                                        vec->appendString(tmp.data(),mIndex);
                                    }
                                }
                                mmap[colName[i]].push_back(bson_iter_utf8(&biter,NULL));
                                nullMap.erase(i);
                                break;
                            }
                            case BSON_TYPE_SYMBOL:{
                                colType[i]=DT_STRING;
                                cols[i]=Util::createVector(DT_STRING,0,rNum);
                                mmap[colName[i]]=vector<string>(rNum%mIndex);
                                if(rNum>=mIndex){
                                    vector<string>tmp(mIndex);
                                    long long rwNum=rNum/mIndex;
                                    for(long long j=0;j<rwNum;++j){
                                        vec->appendString(tmp.data(),mIndex);
                                    }
                                }
                                mmap[colName[i]].push_back(bson_iter_symbol(&biter,NULL));
                                nullMap.erase(i);
                                break;
                            }
                            case BSON_TYPE_INT32:{
                                colType[i]=DT_INT;
                                cols[i]=Util::createVector(DT_INT,0,rNum);
                                char* ptr=(char*)malloc(mIndex*sizeof(int));
                                if(ptr == nullptr)    throw std::bad_alloc();
                                buffer[i]=ptr;
                                if(rNum<mIndex){
                                    for(long long j=0;j<rNum;++j){
                                        *((int*)ptr+j)=INT_MIN;
                                    }
                                }
                                else{
                                    for(long long  j=0;j<mIndex;++j){
                                        *((int*)ptr+j)=INT_MIN;
                                    }
                                    long long rwNum=rNum/mIndex;
                                    for(long long j=0;j<rwNum;++j){
                                        vec->appendInt((int*)ptr,mIndex);
                                    }
                                }
                                *((int*)ptr+rNum%mIndex)=bson_iter_double(&biter);
                                nullMap.erase(i);
                                break;
                            }
                            case BSON_TYPE_INT64:{
                                colType[i]=DT_LONG;
                                cols[i]=Util::createVector(DT_LONG,0,rNum);
                                char* ptr=(char*)malloc(mIndex*sizeof(long long));
                                if(ptr == nullptr)    throw std::bad_alloc();
                                buffer[i]=ptr;
                                if(rNum<mIndex){
                                    for(long long j=0;j<rNum;++j){
                                        *((long long* )ptr+j)=LONG_LONG_MIN;
                                    }
                                }
                                else{
                                    for(long long  j=0;j<mIndex;++j){
                                        *((long long*)ptr+j)=LONG_LONG_MIN;
                                    }
                                    long long rwNum=rNum/mIndex;
                                    for(long long j=0;j<rwNum;++j){
                                        vec->appendLong((long long*)ptr,mIndex);
                                    }
                                }
                                *((long long*)ptr+rNum%mIndex)=bson_iter_int64(&biter);
                                nullMap.erase(i);
                                break;
                            }
                            case BSON_TYPE_OID:{
                                colType[i]=DT_STRING;
                                cols[i]=Util::createVector(DT_STRING,0,rNum);
                                mmap[colName[i]]=vector<string>(rNum%mIndex);
                                if(rNum>=mIndex){
                                    vector<string>tmp(mIndex);
                                    long long rwNum=rNum/mIndex;
                                    for(long long j=0;j<rwNum;++j){
                                        vec->appendString(tmp.data(),mIndex);
                                    }
                                }
                                mmap[colName[i]].push_back(bson_iter_utf8(&biter,NULL));
                                nullMap.erase(i);
                                break;
                            }
                            case BSON_TYPE_BOOL:{
                                colType[i]=DT_BOOL;
                                cols[i]=Util::createVector(DT_BOOL,0,rNum);
                                char* ptr=(char*)malloc(mIndex*sizeof(bool));
                                if(ptr == nullptr)    throw std::bad_alloc();
                                buffer[i]=ptr;
                                if(rNum<mIndex){
                                    for(long long j=0;j<rNum;++j){
                                        *((bool* )ptr+j)=false;
                                    }
                                }
                                else{
                                    for(long long  j=0;j<mIndex;++j){
                                        *((bool*)ptr+j)=false;
                                    }
                                    long long rwNum=rNum/mIndex;
                                    for(long long j=0;j<rwNum;++j){
                                        vec->appendBool((char*)ptr,mIndex);
                                    }
                                }
                                *((bool*)ptr+rNum%mIndex)=bson_iter_bool(&biter);
                                nullMap.erase(i);
                                break;
                            }
                            case BSON_TYPE_DATE_TIME:{
                                colType[i]=DT_TIMESTAMP;
                                cols[i]=Util::createVector(DT_TIMESTAMP,0,rNum);
                                char* ptr=(char*)malloc(mIndex*sizeof(long long));
                                if(ptr == nullptr)    throw std::bad_alloc();
                                buffer[i]=ptr;
                                if(rNum<mIndex){
                                    for(long long j=0;j<rNum;++j){
                                        *((long long* )ptr+j)=LONG_LONG_MIN;
                                    }
                                }
                                else{
                                    for(long long  j=0;j<mIndex;++j){
                                        *((long long*)ptr+j)=LONG_LONG_MIN;
                                    }
                                    long long rwNum=rNum/mIndex;
                                    for(long long j=0;j<rwNum;++j){
                                        vec->appendLong((long long*)ptr,mIndex);
                                    }
                                }
                                *((long long*)ptr+rNum%mIndex)=bson_iter_date_time(&biter);
                                nullMap.erase(i);
                                break;
                            }
                            case BSON_TYPE_DECIMAL128:{
                                colType[i]=DT_DOUBLE;
                                cols[i]=Util::createVector(DT_DOUBLE,0,rNum);
                                char* ptr=(char*)malloc(mIndex*sizeof(double));
                                if(ptr == nullptr)    throw std::bad_alloc();
                                buffer[i]=ptr;
                                if(rNum<mIndex){
                                    for(long long j=0;j<rNum;++j){
                                        *((double*)ptr+j)=DBL_NMIN;
                                    }
                                }
                                else{
                                    for(long long  j=0;j<mIndex;++j){
                                        *((double*)ptr+j)=DBL_NMIN;
                                    }
                                    long long rwNum=rNum/mIndex;
                                    for(long long j=0;j<rwNum;++j){
                                        vec->appendDouble((double*)ptr,mIndex);
                                    }
                                }
                                bson_decimal128_t dec ;
                                bson_iter_decimal128(&biter,&dec);
                                char str[100];
                                bson_decimal128_to_string(&dec,str);
                                std::string ttmp(str);
                                try{
                                    double t=std::stod(ttmp);
                                    *((double*)ptr+rNum%mIndex)=t;
                                }
                                catch(std::exception& e){
                                    *((double*)ptr+rNum%mIndex)=DBL_NMIN;
                                }
                                nullMap.erase(i);
                                break;
                            }
                            case BSON_TYPE_NULL:{
                                nullMap[i]++;
                                break;
                            }
                            default: {
                                throw RuntimeException("The Mongodb value whose key is " + colName[i] + " is of type " + getBsonString(btype) + " and is not supported. ");
                            }
                        }
                    }
                    else{
                        if(schemaEx){
                            switch(dtype){
                                case DT_BOOL:
                                    switch(btype){
                                        case BSON_TYPE_BOOL:{
                                            bool t=bson_iter_bool(&biter);
                                            *((bool*)(buffer[i])+index)=t;
                                            break;
                                        }
                                        case BSON_TYPE_INT32:{
                                            int t=bson_iter_int32(&biter);
                                            *((bool*)(buffer[i])+index)=static_cast<char>(t != 0);
                                            break;
                                        }
                                        case BSON_TYPE_INT64:{
                                            long long t=bson_iter_int64(&biter);
                                            *((bool*)(buffer[i])+index)=static_cast<char>(t != 0);
                                            break;
                                        }
                                        case BSON_TYPE_DOUBLE:{
                                            double t=bson_iter_double(&biter);
                                            *((bool*)(buffer[i])+index)=static_cast<char>(t != 0);
                                            break;
                                        }
                                        case BSON_TYPE_DECIMAL128:{
                                            bson_decimal128_t dec ;
                                            bson_iter_decimal128(&biter,&dec);
                                            char str[100];
                                            bson_decimal128_to_string(&dec,str);
                                            std::string ttmp(str);
                                            try{
                                                double t=std::stod(ttmp);
                                                *((bool*)(buffer[i])+index)=static_cast<char>(t != 0);
                                            }
                                            catch(std::exception& e){
                                                *((bool*)(buffer[i])+index)=false;
                                            }
                                            break;
                                        }
                                        case BSON_TYPE_DATE_TIME:{
                                            throw RuntimeException("Cannot convert from Mongodb date to Dolphindb BOOL");
                                            break;
                                        }
                                        case BSON_TYPE_UTF8:{
                                            std::string str=bson_iter_utf8(&biter,NULL);
                                            if(str=="true"||str=="1")*((bool*)(buffer[i])+index)=true;
                                            else *((bool*)(buffer[i])+index)=false;
                                            break;
                                        }
                                        case BSON_TYPE_SYMBOL:{
                                            std::string str=bson_iter_symbol(&biter,NULL);
                                            if(str=="true"||str=="1")*((bool*)(buffer[i])+index)=true;
                                            else *((bool*)(buffer[i])+index)=false;
                                            break;
                                        }
                                        case BSON_TYPE_NULL:{
                                            *((bool*)(buffer[i])+index)=false;
                                            break;
                                        }
                                        case BSON_TYPE_OID:{
                                            throw RuntimeException("Cannot convert from Mongodb oid to Dolphindb BOOL");
                                        }
                                        default:
                                            throw RuntimeException("The Mongodb value whose key is " + colName[i] + " is of type " + getBsonString(btype) + " and is not supported. ");
                                    }
                                    break;
                                case DT_CHAR:
                                    switch(btype){
                                        case BSON_TYPE_BOOL:{
                                            bool t=bson_iter_bool(&biter);
                                            *((char*)(buffer[i])+index)=t;
                                            break;
                                        }
                                        case BSON_TYPE_INT32:{
                                            int t=bson_iter_int32(&biter);
                                            *((char*)(buffer[i])+index)=*((char*)(&t));
                                            break;
                                        }
                                        case BSON_TYPE_INT64:{
                                            long long t=bson_iter_int64(&biter);
                                            *((char*)(buffer[i])+index)=*((char*)(&t));
                                            break;
                                        }
                                        case BSON_TYPE_DOUBLE:{
                                            double t=bson_iter_double(&biter);
                                            if((t>=0&&t+0.5>INT_MAX)||(t<0&&t-0.5<INT_MIN)){
                                                *((char*)(buffer[i])+index)=0;
                                            }
                                            else{
                                                sprintf(busf,"%.0f",t);
                                                int ret=atoi(busf);
                                                *((char*)(buffer[i])+index)=*((char*)(&ret));
                                            }
                                            break;
                                        }
                                        case BSON_TYPE_DECIMAL128:{
                                            bson_decimal128_t dec ;
                                            bson_iter_decimal128(&biter,&dec);
                                            char str[100];
                                            bson_decimal128_to_string(&dec,str);
                                            std::string ttmp(str);
                                            try{
                                                double t=std::stod(ttmp);
                                                if((t>=0&&t+0.5>INT_MAX)||(t<0&&t-0.5<INT_MIN)){
                                                    *((char*)(buffer[i])+index)=0;
                                                }
                                                else{
                                                    sprintf(busf,"%.0f",t);
                                                    int ret=atoi(busf);
                                                    *((char*)(buffer[i])+index)=*((char*)(&ret));
                                                }
                                            }
                                            catch(std::exception& e){
                                                *((char*)(buffer[i])+index)= CHAR_MIN;
                                            }
                                            break;
                                        }
                                        case BSON_TYPE_DATE_TIME:{
                                            throw RuntimeException("Cannot convert from Mongodb Date to Dolphindb CHAR");
                                            break;
                                        }
                                        case BSON_TYPE_UTF8:{
                                            std::string str=bson_iter_utf8(&biter,NULL);
                                            try{
                                                long long t=std::stoll(str);
                                                *((char*)(buffer[i])+index)=*((char*)(&t));
                                            }
                                            catch(std::exception& e){
                                                *((char*)(buffer[i])+index)= CHAR_MIN;
                                            }
                                            break;
                                        }
                                        case BSON_TYPE_SYMBOL:{
                                            std::string str=bson_iter_symbol(&biter,NULL);
                                            try{
                                                long long t=std::stoll(str);
                                                *((char*)(buffer[i])+index)=*((char*)(&t));
                                            }
                                            catch(std::exception& e){
                                                *((char*)(buffer[i])+index)= CHAR_MIN;
                                            }
                                            break;
                                        }
                                        case BSON_TYPE_NULL:{
                                                *((char*)(buffer[i])+index)=CHAR_MIN;
                                                break;
                                        }
                                        case BSON_TYPE_OID:{
                                            throw RuntimeException("Cannot convert from Mongodb oid to Dolphindb CHAR");
                                        }
                                        default:
                                            throw RuntimeException("The Mongodb value whose key is " + colName[i] + " is of type " + getBsonString(btype) + " and is not supported. ");
                                    }
                                    break;
                                case DT_SHORT:
                                    switch(btype){
                                        case BSON_TYPE_BOOL:{
                                            bool t=bson_iter_bool(&biter);
                                            *((short*)(buffer[i])+index)=t;
                                            break;
                                        }
                                        case BSON_TYPE_INT32:{
                                            int t=bson_iter_int32(&biter);
                                            *((short*)(buffer[i])+index)=*((short*)(&t));
                                            break;
                                        }
                                        case BSON_TYPE_INT64:{
                                            int t=bson_iter_int64(&biter);
                                            *((short*)(buffer[i])+index)=*((short*)(&t));
                                            break;
                                        }
                                        case BSON_TYPE_DOUBLE:{
                                            double t=bson_iter_double(&biter);
                                            if((t>=0&&t+0.5>INT_MAX)||(t<0&&t-0.5<INT_MIN)){
                                                *((short*)(buffer[i])+index)=0;
                                            }
                                            else{
                                                sprintf(busf,"%.0f",t);
                                                int ret=atoi(busf);
                                                *((short*)(buffer[i])+index)=*((short*)(&ret));
                                            }
                                            break;
                                        }
                                        case BSON_TYPE_DECIMAL128:{
                                            bson_decimal128_t dec ;
                                            bson_iter_decimal128(&biter,&dec);
                                            char str[100];
                                            bson_decimal128_to_string(&dec,str);
                                            string ttmp(str);
                                            try{
                                                double t=std::stod(ttmp);
                                                if((t>=0&&t+0.5>INT_MAX)||(t<0&&t-0.5<INT_MIN)){
                                                    *((short*)(buffer[i])+index)=0;
                                                }
                                                else{
                                                    sprintf(busf,"%.0f",t);
                                                    int ret=atoi(busf);
                                                    *((short*)(buffer[i])+index)=*((short*)(&ret));
                                                }
                                            }
                                            catch(std::exception& e){
                                                *((short*)(buffer[i])+index)= SHRT_MIN;
                                            }
                                            break;
                                        }
                                        case BSON_TYPE_DATE_TIME:{
                                            throw RuntimeException("Cannot convert from Mongodb Date to Dolphindb SHORT");
                                            break;
                                        }
                                        case BSON_TYPE_UTF8:{
                                            std::string str=bson_iter_utf8(&biter,NULL);
                                            try{
                                                long long t=std::stoll(str);
                                                *((short*)(buffer[i])+index)=*((short*)(&t));
                                            }
                                            catch(std::exception& e){
                                                *((short*)(buffer[i])+index)= SHRT_MIN;
                                            }
                                            break;
                                        }
                                        case BSON_TYPE_SYMBOL:{
                                            std::string str=bson_iter_symbol(&biter,NULL);
                                            try{
                                                long long t=std::stoll(str);
                                                *((short*)(buffer[i])+index)=*((short*)(&t));
                                            }
                                            catch(std::exception& e){
                                                *((short*)(buffer[i])+index)= SHRT_MIN;
                                            }
                                            break;
                                        }
                                        case BSON_TYPE_NULL:{
                                                *((short*)(buffer[i])+index)=SHRT_MIN;
                                                break;
                                        }
                                        case BSON_TYPE_OID:{
                                            throw RuntimeException("Cannot convert from Mongodb oid to Dolphindb SHORT");
                                        }
                                        default:
                                            throw RuntimeException("The Mongodb value whose key is " + colName[i] + " is of type " + getBsonString(btype) + " and is not supported. ");
                                    }
                                    break;
                                case DT_INT:{
                                    switch(btype){
                                        case BSON_TYPE_BOOL:{
                                            bool t=bson_iter_bool(&biter);
                                            *((int*)(buffer[i])+index)=t;
                                            break;
                                        }
                                        case BSON_TYPE_INT32:{
                                            int t=bson_iter_int32(&biter);
                                            *((int*)(buffer[i])+index)=t;
                                            break;
                                        }
                                        case BSON_TYPE_INT64:{
                                            long long t=bson_iter_int64(&biter);
                                            *((int*)(buffer[i])+index)=*((int*)(&t));
                                            break;
                                        }
                                        case BSON_TYPE_DOUBLE:{
                                            double t=bson_iter_double(&biter);
                                            if((t>=0&&t+0.5>INT_MAX)||(t<0&&t-0.5<INT_MIN)){
                                                *((int*)(buffer[i])+index)=INT_MIN;
                                            }
                                            else{
                                                sprintf(busf,"%.0f",t);
                                                int ret=atoi(busf);
                                                *((int*)(buffer[i])+index)=ret;
                                            }
                                            break;
                                        }
                                        case BSON_TYPE_DECIMAL128:{
                                            bson_decimal128_t dec ;
                                            bson_iter_decimal128(&biter,&dec);
                                            char str[100];
                                            bson_decimal128_to_string(&dec,str);
                                            string ttmp(str);
                                            try{
                                                double t=std::stod(ttmp);
                                                if((t>=0&&t+0.5>INT_MAX)||(t<0&&t-0.5<INT_MIN)){
                                                    *((int*)(buffer[i])+index)=INT_MIN;
                                                }
                                                else{
                                                    sprintf(busf,"%.0f",t);
                                                    int ret=atoi(busf);
                                                    *((int*)(buffer[i])+index)=ret;
                                                }
                                            }
                                            catch(std::exception& e){
                                                *((int*)(buffer[i])+index)=INT_MIN;
                                            }
                                            break;
                                        }
                                        case BSON_TYPE_DATE_TIME:{
                                            throw RuntimeException("Cannot convert from Mongodb Date to Dolphindb INT");
                                            break;
                                        }
                                        case BSON_TYPE_UTF8:{
                                            std::string str=bson_iter_utf8(&biter,NULL);
                                            try{
                                                long long t=std::stoll(str);
                                                *((int*)(buffer[i])+index)=*((int*)(&t));
                                            }
                                            catch(std::exception& e){
                                                *((int*)(buffer[i])+index)=INT_MIN;
                                            }
                                            break;
                                        }
                                        case BSON_TYPE_SYMBOL:{
                                            std::string str=bson_iter_symbol(&biter,NULL);
                                            try{
                                                long long t=std::stoll(str);
                                                *((int*)(buffer[i])+index)=*((int*)(&t));
                                            }
                                            catch(std::exception& e){
                                                *((int*)(buffer[i])+index)=INT_MIN;
                                            }
                                        }
                                        case BSON_TYPE_NULL:{
                                                *((int*)(buffer[i])+index)=INT_MIN;
                                                break;
                                        }
                                        case BSON_TYPE_OID:{
                                            throw RuntimeException("Cannot convert from Mongodb oid to Dolphindb INT");
                                        }
                                        default:
                                            throw RuntimeException("The Mongodb value whose key is " + colName[i] + " is of type " + getBsonString(btype) + " and is not supported. ");
                                    }
                                    break;
                                }
                                case DT_DATETIME:{
                                    switch(btype){
                                        case BSON_TYPE_BOOL:{
                                            throw RuntimeException("Cannot convert from Mongodb Bool to Dolphindb DATETIME");
                                            break;
                                        }
                                        case BSON_TYPE_INT32:{
                                            throw RuntimeException("Cannot convert from Mongodb Int32 to Dolphindb DATETIME");
                                            break;
                                        }
                                        case BSON_TYPE_INT64:{
                                            throw RuntimeException("Cannot convert from Mongodb Int64 to Dolphindb DATETIME");
                                            break;
                                        }
                                        case BSON_TYPE_DOUBLE:{
                                            throw RuntimeException("Cannot convert from Mongodb Double to Dolphindb DATETIME");
                                            break;
                                        }
                                        case BSON_TYPE_DECIMAL128:{
                                            throw RuntimeException("Cannot convert from Mongodb Decimal128 to Dolphindb DATETIME");
                                            break;
                                        }
                                        case BSON_TYPE_DATE_TIME:{
                                            long long t=bson_iter_date_time(&biter);
                                            *((long long*)(buffer[i])+index)=t/1000;
                                            break;
                                        }
                                        case BSON_TYPE_UTF8:{
                                            throw RuntimeException("Cannot convert from Mongodb Sring to Dolphindb DATETIME");
                                            break;
                                        }
                                        case BSON_TYPE_SYMBOL:{
                                            throw RuntimeException("Cannot convert from Mongodb Symbol to Dolphindb DATETIME");
                                            break;
                                        }
                                        case BSON_TYPE_NULL:{
                                                *((long long*)(buffer[i])+index)=LONG_LONG_MIN;
                                                break;
                                        }
                                        case BSON_TYPE_OID:{
                                            throw RuntimeException("Cannot convert from Mongodb oid to Dolphindb DATETIME");
                                        }
                                        default:
                                            throw RuntimeException("The Mongodb value whose key is " + colName[i] + " is of type " + getBsonString(btype) + " and is not supported. ");
                                    }
                                    break;
                                }
                                case DT_TIMESTAMP:{
                                    switch(btype){
                                        case BSON_TYPE_BOOL:{
                                            throw RuntimeException("Cannot convert from Mongodb Bool to Dolphindb TIMESTAMP");
                                            break;
                                        }
                                        case BSON_TYPE_INT32:{
                                            throw RuntimeException("Cannot convert from Mongodb Int32 to Dolphindb TIMESTAMP");
                                            break;
                                        }
                                        case BSON_TYPE_INT64:{
                                            throw RuntimeException("Cannot convert from Mongodb Int64 to Dolphindb TIMESTAMP");
                                            break;
                                        }
                                        case BSON_TYPE_DOUBLE:{
                                            throw RuntimeException("Cannot convert from Mongodb Double to Dolphindb TIMESTAMP");
                                            break;
                                        }
                                        case BSON_TYPE_DECIMAL128:{
                                            throw RuntimeException("Cannot convert from Mongodb Decimal128 to Dolphindb TIMESTAMP");
                                            break;
                                        }
                                        case BSON_TYPE_DATE_TIME:{
                                            long long t=bson_iter_date_time(&biter);
                                            *((long long*)(buffer[i])+index)=t;
                                            break;
                                        }
                                        case BSON_TYPE_UTF8:{
                                            throw RuntimeException("Cannot convert from Mongodb Sring to Dolphindb TIMESTAMP");
                                            break;
                                        }
                                        case BSON_TYPE_SYMBOL:{
                                            throw RuntimeException("Cannot convert from Mongodb Symbol to Dolphindb DATETIME");
                                            break;
                                        }
                                        case BSON_TYPE_NULL:{
                                                *((long long*)(buffer[i])+index)=LONG_LONG_MIN;
                                                break;
                                        }
                                        case BSON_TYPE_OID:{
                                            throw RuntimeException("Cannot convert from Mongodb oid to Dolphindb TIMESTAMP");
                                        }
                                        default:
                                            throw RuntimeException("The Mongodb value whose key is " + colName[i] + " is of type " + getBsonString(btype) + " and is not supported. ");
                                    }
                                    break;
                                }
                                case DT_NANOTIME:{
                                    switch(btype){
                                        case BSON_TYPE_BOOL:{
                                            throw RuntimeException("Cannot convert from Mongodb Bool to Dolphindb NANOTIME");
                                            break;
                                        }
                                        case BSON_TYPE_INT32:{
                                            throw RuntimeException("Cannot convert from Mongodb Int32 to Dolphindb NANOTIME");
                                            break;
                                        }
                                        case BSON_TYPE_INT64:{
                                            throw RuntimeException("Cannot convert from Mongodb Int64 to Dolphindb NANOTIME");
                                            break;
                                        }
                                        case BSON_TYPE_DOUBLE:{
                                            throw RuntimeException("Cannot convert from Mongodb Double to Dolphindb NANOTIME");
                                            break;
                                        }
                                        case BSON_TYPE_DECIMAL128:{
                                            throw RuntimeException("Cannot convert from Mongodb Decimal128 to Dolphindb NANOTIME");
                                            break;
                                        }
                                        case BSON_TYPE_DATE_TIME:{
                                            long long t=bson_iter_date_time(&biter);
                                            long long c=(t%86400000)*1000000;
                                            *((long long*)(buffer[i])+index)=c;
                                            break;
                                        }
                                        case BSON_TYPE_UTF8:{
                                            throw RuntimeException("Cannot convert from Mongodb Sring to Dolphindb NANOTIME");
                                            break;
                                        }
                                        case BSON_TYPE_SYMBOL:{
                                            throw RuntimeException("Cannot convert from Mongodb Symbol to Dolphindb NANOTIME");
                                            break;
                                        }
                                        case BSON_TYPE_NULL:{
                                                *((long long*)(buffer[i])+index)=LONG_LONG_MIN;
                                                break;
                                        }
                                        case BSON_TYPE_OID:{
                                            throw RuntimeException("Cannot convert from Mongodb oid to Dolphindb NANOTIME");
                                        }
                                        default:
                                            throw RuntimeException("The Mongodb value whose key is " + colName[i] + " is of type " + getBsonString(btype) + " and is not supported. ");
                                    }
                                    break;
                                }
                                case DT_NANOTIMESTAMP:{
                                    switch(btype){
                                        case BSON_TYPE_BOOL:{
                                            throw RuntimeException("Cannot convert from Mongodb Bool to Dolphindb NANOTIME");
                                            break;
                                        }
                                        case BSON_TYPE_INT32:{
                                            throw RuntimeException("Cannot convert from Mongodb Int32 to Dolphindb NANOTIMESTAMP");
                                            break;
                                        }
                                        case BSON_TYPE_INT64:{
                                            throw RuntimeException("Cannot convert from Mongodb Int64 to Dolphindb NANOTIMESTAMP");
                                            break;
                                        }
                                        case BSON_TYPE_DOUBLE:{
                                            throw RuntimeException("Cannot convert from Mongodb Double to Dolphindb NANOTIMESTAMP");
                                            break;
                                        }
                                        case BSON_TYPE_DECIMAL128:{
                                            throw RuntimeException("Cannot convert from Mongodb Decimal128 to Dolphindb NANOTIMESTAMP");
                                            break;
                                        }
                                        case BSON_TYPE_DATE_TIME:{
                                            long long t=bson_iter_date_time(&biter);
                                            long long c=t*1000000;
                                            *((long long*)(buffer[i])+index)=c;
                                            break;
                                        }
                                        case BSON_TYPE_UTF8:{
                                            throw RuntimeException("Cannot convert from Mongodb Sring to Dolphindb NANOTIMESTAMP");
                                            break;
                                        }
                                        case BSON_TYPE_SYMBOL:{
                                            throw RuntimeException("Cannot convert from Mongodb Symbol to Dolphindb NANOTIMESTAMP");
                                            break;
                                        }
                                        case BSON_TYPE_NULL:{
                                                *((long long*)(buffer[i])+index)=LONG_LONG_MIN;
                                                break;
                                        }
                                        case BSON_TYPE_OID:{
                                            throw RuntimeException("Cannot convert from Mongodb oid to Dolphindb NANOTIMESTAMP");
                                        }
                                        default:
                                            throw RuntimeException("The Mongodb value whose key is " + colName[i] + " is of type " + getBsonString(btype) + " and is not supported. ");
                                    }
                                    break;
                                }
                                case DT_LONG:{
                                    switch(btype){
                                        case BSON_TYPE_BOOL:{
                                            bool t=bson_iter_bool(&biter);
                                            *((long long*)(buffer[i])+index)=t;
                                            break;
                                        }
                                        case BSON_TYPE_INT32:{
                                            int t=bson_iter_int32(&biter);
                                            *((long long*)(buffer[i])+index)=t;
                                            break;
                                        }
                                        case BSON_TYPE_INT64:{
                                            long long t=bson_iter_int64(&biter);
                                                *((long long*)(buffer[i])+index)=t;
                                            break;
                                        }
                                        case BSON_TYPE_DOUBLE:{
                                            double t=bson_iter_double(&biter);
                                            if((t>=0&&t+0.5>LONG_LONG_MAX)||(t<0&&t-0.5<LONG_LONG_MIN)){
                                                *((long long*)(buffer[i])+index)=LONG_LONG_MIN;
                                            }
                                            else{
                                                sprintf(busf,"%.0f",t);
                                                long long ret=atoll(busf);
                                                *((long long*)(buffer[i])+index)=ret;
                                            }
                                            break;
                                        }
                                        case BSON_TYPE_DECIMAL128:{
                                            bson_decimal128_t dec ;
                                            bson_iter_decimal128(&biter,&dec);
                                            char str[100];
                                            bson_decimal128_to_string(&dec,str);
                                            std::string ttmp(str);
                                            double t=std::stod(ttmp);
                                            try{
                                                if((t>=0&&t+0.5>LONG_LONG_MAX)||(t<0&&t-0.5<LONG_LONG_MIN)){
                                                    *((long long*)(buffer[i])+index)=LONG_LONG_MIN;
                                                }
                                                else{
                                                    sprintf(busf,"%.0f",t);
                                                    long long ret=atoll(busf);
                                                    *((long long*)(buffer[i])+index)=ret;
                                                }
                                            }
                                            catch(std::exception& e){
                                                *((long long*)(buffer[i])+index)=LONG_LONG_MIN;
                                            }
                                            break;
                                        }
                                        case BSON_TYPE_DATE_TIME:{
                                            throw RuntimeException("Cannot convert from Mongodb Date to Dolphindb LONG");
                                            break;
                                        }
                                        case BSON_TYPE_UTF8:{
                                            std::string str=bson_iter_utf8(&biter,NULL);
                                            try{
                                                long long t=std::stoll(str);
                                                *((long long*)(buffer[i])+index)=t;
                                            }
                                            catch(std::exception& e){
                                                *((long long*)(buffer[i])+index)=LONG_LONG_MIN;
                                            }
                                            break;
                                        }
                                        case BSON_TYPE_SYMBOL:{
                                            std::string str=bson_iter_symbol(&biter,NULL);
                                            try{
                                                long long t=std::stoll(str);
                                                *((long long*)(buffer[i])+index)=t;
                                            }
                                            catch(std::exception& e){
                                                *((long long*)(buffer[i])+index)=LONG_LONG_MIN;
                                            }
                                            break;
                                        }
                                        case BSON_TYPE_NULL:{
                                                *((long long*)(buffer[i])+index)=LONG_LONG_MIN;
                                                break;
                                        }
                                        case BSON_TYPE_OID:{
                                            throw RuntimeException("Cannot convert from Mongodb oid to Dolphindb LONG");
                                        }
                                        default:
                                            throw RuntimeException("The Mongodb value whose key is " + colName[i] + " is of type " + getBsonString(btype) + " and is not supported. ");
                                    }
                                    break;
                                }
                                case DT_DATE:{
                                    switch(btype){
                                        case BSON_TYPE_BOOL:{
                                            throw RuntimeException("Cannot convert from Mongodb Bool to Dolphindb NANOTIME");
                                            break;
                                        }
                                        case BSON_TYPE_INT32:{
                                            throw RuntimeException("Cannot convert from Mongodb Int32 to Dolphindb DATE");
                                            break;
                                        }
                                        case BSON_TYPE_INT64:{
                                            throw RuntimeException("Cannot convert from Mongodb Int64 to Dolphindb DATE");
                                            break;
                                        }
                                        case BSON_TYPE_DOUBLE:{
                                            throw RuntimeException("Cannot convert from Mongodb Double to Dolphindb DATE");
                                            break;
                                        }
                                        case BSON_TYPE_DECIMAL128:{
                                            throw RuntimeException("Cannot convert from Mongodb Decimal128 to Dolphindb DATE");
                                            break;
                                        }
                                        case BSON_TYPE_DATE_TIME:{
                                            long long t=bson_iter_date_time(&biter);
                                            *((int*)(buffer[i])+index)=t/86400000;
                                            break;
                                        }
                                        case BSON_TYPE_UTF8:{
                                            throw RuntimeException("Cannot convert from Mongodb Sring to Dolphindb DATE");
                                            break;
                                        }
                                        case BSON_TYPE_SYMBOL:{
                                            throw RuntimeException("Cannot convert from Mongodb Symbol to Dolphindb DATE");
                                            break;
                                        }
                                        case BSON_TYPE_NULL:{
                                                *((int*)(buffer[i])+index)=INT_MIN;
                                                break;
                                        }
                                        case BSON_TYPE_OID:{
                                            throw RuntimeException("Cannot convert from Mongodb oid to Dolphindb DATE");
                                        }
                                        default:
                                            throw RuntimeException("The Mongodb value whose key is " + colName[i] + " is of type " + getBsonString(btype) + " and is not supported. ");
                                    }
                                    break;
                                }
                                case DT_MONTH:{
                                    switch(btype){
                                        case BSON_TYPE_BOOL:{
                                            throw RuntimeException("Cannot convert from Mongodb Bool to Dolphindb NANOTIME");
                                            break;
                                        }
                                        case BSON_TYPE_INT32:{
                                            throw RuntimeException("Cannot convert from Mongodb Int32 to Dolphindb MONTH");
                                            break;
                                        }
                                        case BSON_TYPE_INT64:{
                                            throw RuntimeException("Cannot convert from Mongodb Int64 to Dolphindb MONTH");
                                            break;
                                        }
                                        case BSON_TYPE_DOUBLE:{
                                            throw RuntimeException("Cannot convert from Mongodb Double to Dolphindb MONTH");
                                            break;
                                        }
                                        case BSON_TYPE_DECIMAL128:{
                                            throw RuntimeException("Cannot convert from Mongodb Decimal128 to Dolphindb MONTH");
                                            break;
                                        }
                                        case BSON_TYPE_DATE_TIME:{
                                            long long t=bson_iter_date_time(&biter);
                                            int year,month,day;
                                            int c=t/86400000;
                                            Util::parseDate(c,year,month,day);
                                            *((int*)(buffer[i])+index)=year*12+month-1;
                                            break;
                                        }
                                        case BSON_TYPE_UTF8:{
                                            throw RuntimeException("Cannot convert from Mongodb Sring to Dolphindb MONTH");
                                            break;
                                        }
                                        case BSON_TYPE_SYMBOL:{
                                            throw RuntimeException("Cannot convert from Mongodb Symbol to Dolphindb MONTH");
                                            break;
                                        }
                                        case BSON_TYPE_NULL:{
                                                *((int*)(buffer[i])+index)=INT_MIN;
                                                break;
                                        }
                                        case BSON_TYPE_OID:{
                                            throw RuntimeException("Cannot convert from Mongodb oid to Dolphindb MONTH");
                                        }
                                        default:
                                            throw RuntimeException("The Mongodb value whose key is " + colName[i] + " is of type " + getBsonString(btype) + " and is not supported. ");
                                    }
                                    break;
                                }
                                case DT_TIME:{
                                    switch(btype){
                                        case BSON_TYPE_BOOL:{
                                            throw RuntimeException("Cannot convert from Mongodb Bool to Dolphindb TIME");
                                            break;
                                        }
                                        case BSON_TYPE_INT32:{
                                            throw RuntimeException("Cannot convert from Mongodb Int32 to Dolphindb TIME");
                                            break;
                                        }
                                        case BSON_TYPE_INT64:{
                                            throw RuntimeException("Cannot convert from Mongodb Int64 to Dolphindb TIME");
                                            break;
                                        }
                                        case BSON_TYPE_DOUBLE:{
                                            throw RuntimeException("Cannot convert from Mongodb Double to Dolphindb TIME");
                                            break;
                                        }
                                        case BSON_TYPE_DECIMAL128:{
                                            throw RuntimeException("Cannot convert from Mongodb Decimal128 to Dolphindb TIME");
                                            break;
                                        }
                                        case BSON_TYPE_DATE_TIME:{
                                            long long t=bson_iter_date_time(&biter);
                                            int c=t%86400000;
                                            *((int*)(buffer[i])+index)=c;
                                            break;
                                        }
                                        case BSON_TYPE_UTF8:{
                                            throw RuntimeException("Cannot convert from Mongodb Sring to Dolphindb TIME");
                                            break;
                                        }
                                        case BSON_TYPE_SYMBOL:{
                                            throw RuntimeException("Cannot convert from Mongodb Symbol to Dolphindb TIME");
                                            break;
                                        }
                                        case BSON_TYPE_NULL:{
                                                *((int*)(buffer[i])+index)=INT_MIN;
                                                break;
                                        }
                                        case BSON_TYPE_OID:{
                                            throw RuntimeException("Cannot convert from Mongodb oid to Dolphindb TIME");
                                        }
                                        default:
                                            throw RuntimeException("The Mongodb value whose key is " + colName[i] + " is of type " + getBsonString(btype) + " and is not supported. ");
                                    }
                                    break;
                                }
                                case DT_MINUTE:{
                                    switch(btype){
                                        case BSON_TYPE_BOOL:{
                                            throw RuntimeException("Cannot convert from Mongodb Bool to Dolphindb MINUTE");
                                            break;
                                        }
                                        case BSON_TYPE_INT32:{
                                            throw RuntimeException("Cannot convert from Mongodb Int32 to Dolphindb MINUTE");
                                            break;
                                        }
                                        case BSON_TYPE_INT64:{
                                            throw RuntimeException("Cannot convert from Mongodb Int64 to Dolphindb MINUTE");
                                            break;
                                        }
                                        case BSON_TYPE_DOUBLE:{
                                            throw RuntimeException("Cannot convert from Mongodb Double to Dolphindb MINUTE");
                                            break;
                                        }
                                        case BSON_TYPE_DECIMAL128:{
                                            throw RuntimeException("Cannot convert from Mongodb Decimal128 to Dolphindb MINUTE");
                                            break;
                                        }
                                        case BSON_TYPE_DATE_TIME:{
                                            long long t=bson_iter_date_time(&biter);
                                            int c=t%86400000/60/1000;
                                            *((int*)(buffer[i])+index)=c;
                                            break;
                                        }
                                        case BSON_TYPE_UTF8:{
                                            throw RuntimeException("Cannot convert from Mongodb Sring to Dolphindb MINUTE");
                                            break;
                                        }
                                        case BSON_TYPE_SYMBOL:{
                                            throw RuntimeException("Cannot convert from Mongodb Symbol to Dolphindb MINUTE");
                                            break;
                                        }
                                        case BSON_TYPE_NULL:{
                                                *((int*)(buffer[i])+index)=INT_MIN;
                                                break;
                                        }
                                        case BSON_TYPE_OID:{
                                            throw RuntimeException("Cannot convert from Mongodb oid to Dolphindb MINUTE");
                                        }
                                        default:
                                            throw RuntimeException("The Mongodb value whose key is " + colName[i] + " is of type " + getBsonString(btype) + " and is not supported. ");
                                    }
                                    break;
                                }
                                case DT_FLOAT:{
                                    switch(btype){
                                        case BSON_TYPE_BOOL:{
                                            bool t=bson_iter_bool(&biter);
                                            *((float*)(buffer[i])+index)=t;
                                            break;
                                        }
                                        case BSON_TYPE_INT32:{
                                            int t=bson_iter_int32(&biter);
                                            *((float*)(buffer[i])+index)=t;
                                            break;
                                        }
                                        case BSON_TYPE_INT64:{
                                            long long t=bson_iter_int64(&biter);
                                            *((float*)(buffer[i])+index)=static_cast<float>(t);
                                            break;
                                        }
                                        case BSON_TYPE_DOUBLE:{
                                            double t=bson_iter_double(&biter);
                                            *((float*)(buffer[i])+index)=static_cast<float>(t);
                                            break;
                                        }
                                        case BSON_TYPE_DECIMAL128:{
                                            bson_decimal128_t dec ;
                                            bson_iter_decimal128(&biter,&dec);
                                            char str[100];
                                            bson_decimal128_to_string(&dec,str);
                                            std::string ttmp(str);
                                            try{
                                                double t=std::stod(ttmp);
                                                *((float*)(buffer[i])+index)=static_cast<float>(t);
                                            }
                                            catch(std::exception& e){
                                                *((float*)(buffer[i])+index)=FLT_NMIN;
                                            }
                                            break;
                                        }
                                        case BSON_TYPE_DATE_TIME:{
                                            throw RuntimeException("Cannot convert from Mongodb Date to Dolphindb FLOAT");
                                            break;
                                        }
                                        case BSON_TYPE_UTF8:{
                                            std::string str=bson_iter_utf8(&biter,NULL);
                                            try{
                                                float t=std::stof(str);
                                                *((float*)(buffer[i])+index)=t;
                                            }
                                            catch(std::exception& e){
                                                *((float*)(buffer[i])+index)=FLT_NMIN;
                                            }
                                            break;
                                        }
                                        case BSON_TYPE_SYMBOL:{
                                            std::string str=bson_iter_symbol(&biter,NULL);
                                            try{
                                                float t=std::stof(str);
                                                *((float*)(buffer[i])+index)=t;
                                            }
                                            catch(std::exception& e){
                                                *((float*)(buffer[i])+index)=FLT_NMIN;
                                            }
                                            break;
                                        }
                                        case BSON_TYPE_NULL:{
                                                *((float*)(buffer[i])+index)=FLT_NMIN;
                                                break;
                                        }
                                        case BSON_TYPE_OID:{
                                            throw RuntimeException("Cannot convert from Mongodb oid to Dolphindb FLOAT");
                                        }
                                        default:
                                            throw RuntimeException("The Mongodb value whose key is " + colName[i] + " is of type " + getBsonString(btype) + " and is not supported. ");
                                    }
                                    break;
                                }
                                case DT_DOUBLE:{
                                    switch(btype){
                                        case BSON_TYPE_BOOL:{
                                            bool t=bson_iter_bool(&biter);
                                            *((double*)(buffer[i])+index)=t;
                                            break;
                                        }
                                        case BSON_TYPE_INT32:{
                                            int t=bson_iter_int32(&biter);
                                            *((double*)(buffer[i])+index)=t;
                                            break;
                                        }
                                        case BSON_TYPE_INT64:{
                                            long long t=bson_iter_int64(&biter);
                                            *((double*)(buffer[i])+index)=t;
                                            break;
                                        }
                                        case BSON_TYPE_DOUBLE:{
                                            double t=bson_iter_double(&biter);
                                            *((double*)(buffer[i])+index)=t ;
                                            break;
                                        }
                                        case BSON_TYPE_DECIMAL128:{
                                            bson_decimal128_t dec ;
                                            bson_iter_decimal128(&biter,&dec);
                                            char str[100];
                                            bson_decimal128_to_string(&dec,str);
                                            std::string ttmp(str);
                                            try{
                                                double t=std::stod(ttmp);
                                                *((double*)(buffer[i])+index)=t ;
                                            }
                                            catch(std::exception& e){
                                                *((double*)(buffer[i])+index)=DBL_NMIN;
                                            }
                                            break;
                                        }
                                        case BSON_TYPE_DATE_TIME:{
                                            throw RuntimeException("Cannot convert from Mongodb Date to Dolphindb FLOAT");
                                            break;
                                        }
                                        case BSON_TYPE_UTF8:{
                                            std::string str=bson_iter_utf8(&biter,NULL);
                                            try{
                                                double t=std::stod(str);
                                                *((double*)(buffer[i])+index)=t ;
                                            }
                                            catch(std::exception& e){
                                                *((double*)(buffer[i])+index)=DBL_NMIN;
                                            }
                                            break;
                                        }
                                        case BSON_TYPE_SYMBOL:{
                                            std::string str=bson_iter_symbol(&biter,NULL);
                                            try{
                                                double t=std::stod(str);
                                                *((double*)(buffer[i])+index)=t ;
                                            }
                                            catch(std::exception& e){
                                                *((double*)(buffer[i])+index)=DBL_NMIN;
                                            }
                                            break;
                                        }
                                        case BSON_TYPE_NULL:{
                                                *((double*)(buffer[i])+index)=DBL_NMIN;
                                                break;
                                        }
                                        case BSON_TYPE_OID:{
                                            throw RuntimeException("Cannot convert from Mongodb oid to Dolphindb DOUBLE");
                                        }
                                        default:
                                            throw RuntimeException("The Mongodb value whose key is " + colName[i] + " is of type " + getBsonString(btype) + " and is not supported. ");
                                    }
                                    break;
                                }
                                case DT_SYMBOL:{
                                    switch(btype){
                                        case BSON_TYPE_BOOL:{
                                            bool t=bson_iter_bool(&biter);
                                            if(t)
                                                mmap[colName[i]].push_back("1");
                                            else 
                                                mmap[colName[i]].push_back("0");
                                            break;
                                        }
                                        case BSON_TYPE_INT32:{
                                            int t=bson_iter_int32(&biter);
                                            mmap[colName[i]].push_back(std::to_string(t));
                                            break;
                                        }
                                        case BSON_TYPE_INT64:{
                                            long long t=bson_iter_int64(&biter);
                                            mmap[colName[i]].push_back(std::to_string(t));
                                            break;
                                        }
                                        case BSON_TYPE_DOUBLE:{
                                            double t=bson_iter_double(&biter);
                                            string strt=std::to_string(t);
                                            int index=strt.size()-1;
                                            if(strt.find('.')!=-1){
                                                while(strt[index]=='0'){
                                                    --index;
                                                }
                                            }
                                            strt=strt.substr(0,index+1);
                                            mmap[colName[i]].push_back(strt);
                                            break;
                                        }
                                        case BSON_TYPE_DECIMAL128:{
                                            bson_decimal128_t dec ;
                                            bson_iter_decimal128(&biter,&dec);
                                            char str[100];
                                            bson_decimal128_to_string(&dec,str);
                                            std::string ttmp(str);
                                            try{
                                                double t=std::stod(ttmp);
                                                string strt=std::to_string(t);
                                                int index=strt.size()-1;
                                                if(strt.find('.')!=-1){
                                                    while(strt[index]=='0'){
                                                        --index;
                                                    }
                                                }
                                                strt=strt.substr(0,index+1);
                                                mmap[colName[i]].push_back(strt);
                                            }
                                            catch(std::exception& e){
                                                mmap[colName[i]].push_back("");
                                            }
                                            break;
                                        }
                                        case BSON_TYPE_DATE_TIME:{
                                            throw RuntimeException("Cannot convert from Mongodb Date to Dolphindb SYMBOL");
                                            break;
                                        }
                                        case BSON_TYPE_UTF8:{
                                            std::string str=bson_iter_utf8(&biter,NULL);
                                            mmap[colName[i]].push_back(str);
                                            break;
                                        }
                                        case BSON_TYPE_SYMBOL:{
                                            std::string str=bson_iter_symbol(&biter,NULL);
                                            mmap[colName[i]].push_back(str);
                                            break;
                                        }
                                        case BSON_TYPE_OID:{
                                            const bson_oid_t * oid=bson_iter_oid(&biter);
                                            char str[25];
                                            bson_oid_to_string(oid,str);
                                            mmap[colName[i]].push_back(str);
                                            break;
                                        }
                                        case BSON_TYPE_NULL:{
                                            mmap[colName[i]].push_back("");
                                            break;
                                        }
                                        default:
                                            throw RuntimeException("The Mongodb value whose key is " + colName[i] + " is of type " + getBsonString(btype) + " and is not supported. ");
                                    }
                                    break;
                                }
                                case DT_STRING:{
                                    switch(btype){
                                        case BSON_TYPE_BOOL:{
                                            bool t=bson_iter_bool(&biter);
                                            if(t)
                                                mmap[colName[i]].push_back("1");
                                            else 
                                                mmap[colName[i]].push_back("0");
                                            break;
                                        }
                                        case BSON_TYPE_INT32:{
                                            int t=bson_iter_int32(&biter);
                                            mmap[colName[i]].push_back(std::to_string(t));
                                            break;
                                        }
                                        case BSON_TYPE_INT64:{
                                            long long t=bson_iter_int64(&biter);
                                            mmap[colName[i]].push_back(std::to_string(t));
                                            break;
                                        }
                                        case BSON_TYPE_DOUBLE:{
                                            double t=bson_iter_double(&biter);
                                            string strt=std::to_string(t);
                                            int index=strt.size()-1;
                                            if(strt.find('.')!=-1){
                                                while(strt[index]=='0'){
                                                    --index;
                                                }
                                            }
                                            strt=strt.substr(0,index+1);
                                            mmap[colName[i]].push_back(strt);
                                            break;
                                        }
                                        case BSON_TYPE_DECIMAL128:{
                                            bson_decimal128_t dec ;
                                            bson_iter_decimal128(&biter,&dec);
                                            char str[100];
                                            bson_decimal128_to_string(&dec,str);
                                            std::string ttmp(str);
                                            try{
                                                double t=std::stod(ttmp);
                                                string strt=std::to_string(t);
                                                int index=strt.size()-1;
                                                if(strt.find('.')!=-1){
                                                    while(strt[index]=='0'){
                                                        --index;
                                                    }
                                                }
                                                strt=strt.substr(0,index+1);
                                                mmap[colName[i]].push_back(strt);
                                            }
                                            catch(std::exception& e){
                                                mmap[colName[i]].push_back("");
                                            }
                                            break;
                                        }
                                        case BSON_TYPE_DATE_TIME:{
                                            throw RuntimeException("Cannot convert from Mongodb Date to Dolphindb SYMBOL");
                                            break;
                                        }
                                        case BSON_TYPE_UTF8:{
                                            std::string str=bson_iter_utf8(&biter,NULL);
                                            mmap[colName[i]].push_back(str);
                                            break;
                                        }
                                        case BSON_TYPE_SYMBOL:{
                                            std::string str=bson_iter_symbol(&biter,NULL);
                                            mmap[colName[i]].push_back(str);
                                            break;
                                        }
                                        case BSON_TYPE_OID:{
                                            const bson_oid_t * oid=bson_iter_oid(&biter);
                                            char str[25];
                                            bson_oid_to_string(oid,str);
                                            mmap[colName[i]].push_back(str);
                                            break;
                                        }
                                        case BSON_TYPE_NULL:{
                                            mmap[colName[i]].push_back("");
                                            break;
                                        }
                                        default:
                                            throw RuntimeException("The Mongodb value whose key is " + colName[i] + " is of type " + getBsonString(btype) + " and is not supported. ");
                                    }
                                    break;
                                }
                                default: {
                                    throw RuntimeException("The dolphindb type " + Util::getDataTypeString(dtype) + " is not supported");
                                }
                            }
                        }
                        else{
                            switch(dtype){
                            case DT_BOOL:
                                if(btype==BSON_TYPE_BOOL){
                                    bool t=bson_iter_bool(&biter);
                                    *((bool*)(buffer[i])+index)=t;
                                }
                                else *((bool*)(buffer[i])+index)=false;
                                break;
                            case DT_CHAR:
                                break;
                            case DT_SHORT:
                                break;
                            case DT_INT:
                                if(btype==BSON_TYPE_INT32){
                                    int t=bson_iter_int32(&biter);
                                    *((int*)(buffer[i])+index)=t;
                                }
                                else *((int*)(buffer[i])+index)=INT_MIN;
                                break;
                            case DT_DATETIME:
                                break;
                            case DT_TIMESTAMP:{
                                if(btype==BSON_TYPE_DATE_TIME){
                                    long long t=bson_iter_date_time(&biter);
                                    *((long long*)(buffer[i])+index)=t;
                                }
                                else *((long long*)(buffer[i])+index)=LONG_LONG_MIN;
                                break;
                            }
                            case DT_NANOTIME:
                            break;
                            case DT_NANOTIMESTAMP:
                            break;
                            case DT_LONG:
                                if(btype==BSON_TYPE_INT64){
                                    long long t=bson_iter_int64(&biter);
                                    *((long long*)(buffer[i])+index)=t;
                                }
                                else if(btype==BSON_TYPE_INT32){
                                    int t=bson_iter_int64(&biter);
                                    *((long long*)(buffer[i])+index)=(long long)t;
                                }
                                else *((long long*)(buffer[i])+index)=LONG_LONG_MIN;
                                break;
                            case DT_DATE:
                            break;
                            case DT_MONTH:
                            break;
                            case DT_TIME:
                            break;
                            case DT_MINUTE:
                            break;
                            case DT_SECOND:
                                break;
                            case DT_FLOAT:
                                break;
                            case DT_DOUBLE:
                                if(btype==BSON_TYPE_DOUBLE){
                                    double t=bson_iter_double(&biter);
                                    *((double*)(buffer[i])+index)=t;
                                }
                                else if(btype==BSON_TYPE_DECIMAL128){
                                    bson_decimal128_t dec ;
                                    bson_iter_decimal128(&biter,&dec);
                                    char str[100];
                                    bson_decimal128_to_string(&dec,str);
                                    std::string ttmp(str);
                                    double t=std::stod(ttmp);
                                    *((double*)(buffer[i])+index)=t;
                                }
                                else *((double*)(buffer[i])+index)=DBL_NMIN;
                                break;
                            case DT_SYMBOL:
                            break;
                            case DT_STRING:
                                if(btype==BSON_TYPE_UTF8){
                                    std::string t=bson_iter_utf8(&biter,NULL);
                                    mmap[colName[i]].push_back(t);
                                }
                                else if(btype==BSON_TYPE_SYMBOL){
                                    std::string t=bson_iter_symbol(&biter,NULL);
                                    mmap[colName[i]].push_back(t);
                                }
                                else if(btype==BSON_TYPE_OID){
                                    char tmp[25];
                                    bson_oid_to_string(bson_iter_oid(&biter),tmp);
                                    std::string ttmp(tmp);
                                    mmap[colName[i]].push_back(ttmp);
                                }
                                else mmap[colName[i]].push_back("");
                                break;
                            case DT_HANDLE:
                                break;
                            default: {
                                throw RuntimeException("Type " + Util::getDataTypeString(dtype) + " are not supported");
                            }
                        }
                    }
                    }
                }
                else{
                    DATA_TYPE type=colType[i];
                    switch(type){
                        case DT_BOOL:
                            *((bool*)(buffer[i])+index)=false;
                            break;
                        case DT_CHAR:
                            *((char*)(buffer[i])+index)=CHAR_MIN;
                            break;
                        case DT_SHORT:
                            *((bool*)(buffer[i])+index)=SHRT_MIN;
                            break;
                        case DT_INT:
                            *((int*)(buffer[i])+index)=INT_MIN;
                            break;
                        case DT_DATETIME:
                        case DT_TIMESTAMP:
                        case DT_NANOTIME:
                        case DT_NANOTIMESTAMP:
                        case DT_LONG:
                            *((long long*)(buffer[i])+index)=LONG_LONG_MIN;
                            break;
                        case DT_DATE:
                        case DT_MONTH:
                        case DT_TIME:
                        case DT_MINUTE:
                        case DT_SECOND:
                            *((int*)(buffer[i])+index)=INT_MIN;
                            break;
                        case DT_FLOAT:
                            *((float*)(buffer[i])+index)=FLT_NMIN;
                            break;
                        case DT_DOUBLE:
                            *((double*)(buffer[i])+index)=DBL_NMIN;
                            break;
                        case DT_SYMBOL:
                        case DT_STRING:
                            mmap[colName[i]].push_back(string());
                            break;
                        case DT_HANDLE:
                                nullMap[i]++;
                                break;
                        default: {
                            throw RuntimeException("Type " + Util::getDataTypeString(type) + " are not supported");
                        }
                    }
                }
            }
        }
        ++index;
        if(index==mIndex){
            for(int i=0;i<len;++i){
                DATA_TYPE type=colType[i];
                VectorSP vec=cols[i];
                char* colBuffer=buffer[i];
                switch(type){
                    case DT_BOOL:
                        vec->appendBool((char*)colBuffer,mIndex);
                        break;
                    case DT_CHAR:
                        vec->appendChar((char*)colBuffer, mIndex);
                        break;
                    case DT_SHORT:
                        vec->appendShort((short *)colBuffer, mIndex);
                        break;
                    case DT_INT:
                        vec->appendInt((int *)colBuffer, mIndex);
                        break;
                    case DT_DATETIME:
                    case DT_TIMESTAMP:
                    case DT_NANOTIME:
                    case DT_NANOTIMESTAMP:
                    case DT_LONG:
                        vec->appendLong((long long *)colBuffer, mIndex);
                        break;
                    case DT_DATE:
                    case DT_MONTH:
                    case DT_TIME:
                    case DT_MINUTE:
                    case DT_SECOND:
                        vec->appendInt((int *)colBuffer, mIndex);
                        break;
                    case DT_FLOAT:
                        vec->appendFloat((float *)colBuffer, mIndex);
                        break;
                    case DT_DOUBLE:
                        vec->appendDouble((double *)colBuffer, mIndex);
                        break;
                    case DT_SYMBOL:
                    case DT_STRING:
                        vec->appendString(mmap[colName[i]].data(), mIndex);
                        mmap[colName[i]].clear();
                        break;
                    case DT_HANDLE:break;
                    default: {
                        throw RuntimeException("The dolphindb type " + Util::getDataTypeString(type) + " is not supported");
                    }
                }
            }
            index=0;
        }
    }
    for(int i=0;i<len;++i){
        DATA_TYPE type=colType[i];
        VectorSP vec=cols[i];
        char* colBuffer=buffer[i];
        switch(type){
            case DT_BOOL:
                vec->appendBool((char*)colBuffer,index);
                break;
            case DT_CHAR:
                vec->appendChar((char*)colBuffer, index);
                break;
            case DT_SHORT:
                vec->appendShort((short *)colBuffer, index);
                break;
            case DT_INT:
                vec->appendInt((int *)colBuffer, index);
                break;
            case DT_DATETIME:
            case DT_TIMESTAMP:
            case DT_NANOTIME:
            case DT_NANOTIMESTAMP:
            case DT_LONG:
                vec->appendLong((long long *)colBuffer, index);
                break;
            case DT_DATE:
            case DT_MONTH:
            case DT_TIME:
            case DT_MINUTE:
            case DT_SECOND:
                vec->appendInt((int *)colBuffer, index);
                break;
            case DT_FLOAT:
                vec->appendFloat((float *)colBuffer, index);
                break;
            case DT_DOUBLE:
                vec->appendDouble((double *)colBuffer, index);
                break;
            case DT_SYMBOL:
            case DT_STRING:
                vec->appendString(mmap[colName[i]].data(), index);
                mmap[colName[i]].clear();
                break;
            case DT_HANDLE:
                break;
            default: {
                throw RuntimeException("The dolphindb type " + Util::getDataTypeString(type) + " is not supported");
            }
        }
    }
    // for(int i=0;i<len;++i){
    //     if(buffer[i]!=NULL)free(buffer[i]);
    // }
    for(auto a:nullMap){
        double tmp[mIndex];
        for(int i=0;i<mIndex;++i)tmp[i]=DBL_NMIN;
        colType[a.first]=DT_DOUBLE;
        cols[a.first]=Util::createVector(DT_DOUBLE,0,rowSum);
        VectorSP vec=cols[a.first];
        long long rnum=rowSum/mIndex;
        for(long long i=0;i<rnum;++i){
            vec->appendDouble((double*)tmp,mIndex);
        }
        rnum=len%mIndex;
        vec->appendDouble((double*)tmp,rowSum%mIndex);
    }
    if(len==0){ 
        const char* ct="{}";
        bson_t* nquery=bson_new_from_json((const uint8_t*)ct,-1,NULL);
        if(nquery==NULL){
            string strTmp="The BSON structure doesn't fit:";
            throw IllegalArgumentException(__FUNCTION__,strTmp + ct);
        }
        Defer dfQuery([=](){bson_destroy(nquery);});

        const char* optionStr = "{\"limit\":1}";
        bson_t* noption = bson_new_from_json((const uint8_t*)optionStr,-1,NULL);
        if(nquery==NULL){
            string strTmp="The BSON structure doesn't fit:";
            throw IllegalArgumentException(__FUNCTION__,strTmp + optionStr);
        }
        Defer dfOption([=](){bson_destroy(noption);});

        mongoc_cursor_t* ncursor=mongoc_collection_find_with_opts (mcollection,nquery,noption,NULL);
        Defer dfCursor([=](){mongoc_cursor_destroy(ncursor);});

        const bson_t *ndoc;
        if(mongoc_cursor_next(ncursor,&ndoc)){
            bson_iter_t biter;
            bson_iter_init(&biter,ndoc);
            while(bson_iter_next(&biter)){
                const char* ss=bson_iter_key(&biter);
                colName.push_back(ss);
                ++len;
                bson_type_t btype=bson_iter_type(&biter);
                switch(btype){
                    case BSON_TYPE_DOUBLE:{
                        colType.push_back(DT_DOUBLE);
                        cols.push_back(Util::createVector(DT_DOUBLE,0,0));
                        break;
                    }
                    case BSON_TYPE_UTF8:{
                        colType.push_back(DT_STRING);
                        cols.push_back(Util::createVector(DT_STRING,0,0));
                        break;
                    }
                    case BSON_TYPE_INT32:{
                        colType.push_back(DT_INT);
                        cols.push_back(Util::createVector(DT_INT,0,0));
                        break;
                    }
                    case BSON_TYPE_INT64:{
                        colType.push_back(DT_LONG);
                        cols.push_back(Util::createVector(DT_LONG,0,0));
                        break;
                    }
                    case BSON_TYPE_OID:{
                        colType.push_back(DT_STRING);
                        cols.push_back(Util::createVector(DT_STRING,0,0));
                        break;
                    }
                    case BSON_TYPE_BOOL:{
                        colType.push_back(DT_BOOL);
                        cols.push_back(Util::createVector(DT_BOOL,0,0));
                        break;
                    }
                    case BSON_TYPE_DATE_TIME:{
                        colType.push_back(DT_TIMESTAMP);
                        cols.push_back(Util::createVector(DT_TIMESTAMP,0,0));
                        break;
                    }
                    case BSON_TYPE_DECIMAL128:{
                        colType.push_back(DT_DOUBLE);
                        cols.push_back(Util::createVector(DT_DOUBLE,0,0));
                        break;
                    }
                    case BSON_TYPE_SYMBOL:{
                        colType.push_back(DT_STRING);
                        cols.push_back(Util::createVector(DT_STRING,0,0));
                        break;
                    }
                    case BSON_TYPE_NULL:{
                        colType.push_back(DT_BOOL);
                        cols.push_back(Util::createVector(DT_BOOL,0,0));
                        break;
                    }
                    default: {
                        throw RuntimeException("The Mongodb value whose key is " + string(ss) + " is of type " + getBsonString(btype) + " and is not supported. ");
                    }
                }
            }
        }
        if(len==0){
            colName.push_back("c_id");
            cols.push_back(Util::createVector(DT_STRING,0,0));
        }
    }
    conversionStr(colName);
    for(int i=0;i<len;++i){
        for(int j=0;j<i;++j){
            if(colName[i]==colName[j])throw RuntimeException("Column names cannot be the same");
        }
    }
    bson_error_t queryError;
    if(mongoc_cursor_error(cursor, &queryError)){
        throw IllegalArgumentException(__FUNCTION__, string(queryError.message));
    }
}

TableSP mongoConnection::extractLoad(std::string &collection,std::string &condition,std::string &option,TableSP &schema, bool aggregate){
    bool schemaEx=false;
    vector<std::string> colName;
    vector<DATA_TYPE>  colType;
    vector<ConstantSP> cols;
    vector<char*> buffer;
    Defer dfBuffer([&](){
        size_t size = buffer.size();
        for(int i = 0; i < size; ++i){
            if(buffer[i] != nullptr)
                free(buffer[i]);
        }
        });
    unordered_map<string,vector<string>> mmap;
    unordered_map<int,long long> nullMap;
    int len=0;
    int mIndex=1024;
    //extract schema
    if(!schema.isNull()&&!schema->isNull()){
        schemaEx=true;
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
        len=vecName->size();
        for(int i=0;i<len;++i){
            colName.push_back(vecName->getString(i));
        }
        cols.resize(len);
        for(int i=0;i<len;++i){
            string sType=vecType->getString(i);
            std::transform(sType.begin(),sType.end(),sType.begin(),::toupper);
            if(sType=="BOOL"){
                colType.push_back(DT_BOOL);
                cols[i]=Util::createVector(DT_BOOL,0);
                char* ptr=(char*)malloc(mIndex*sizeof(bool));
                if(ptr == nullptr)    throw std::bad_alloc();
                buffer.push_back(ptr);
            }
            else if(sType=="CHAR"){
                colType.push_back(DT_CHAR);
                cols[i]=Util::createVector(DT_CHAR,0);
                char* ptr=(char*)malloc(mIndex*sizeof(char));
                if(ptr == nullptr)    throw std::bad_alloc();
                buffer.push_back(ptr);
            }
           else if(sType=="SHORT"){
                colType.push_back(DT_SHORT);
                cols[i]=Util::createVector(DT_SHORT,0);
                char* ptr=(char*)malloc(mIndex*sizeof(short));
                if(ptr == nullptr)    throw std::bad_alloc();
                buffer.push_back(ptr);
            }
            else if(sType=="INT"){
                colType.push_back(DT_INT);
                cols[i]=Util::createVector(DT_INT,0);
                char* ptr=(char*)malloc(mIndex*sizeof(int));
                if(ptr == nullptr)    throw std::bad_alloc();
                buffer.push_back(ptr);
            }
            else if(sType=="LONG"){
                colType.push_back(DT_LONG);
                cols[i]=Util::createVector(DT_LONG,0);
                char* ptr=(char*)malloc(mIndex*sizeof(long long));
                if(ptr == nullptr)    throw std::bad_alloc();
                buffer.push_back(ptr);
            }
            else if(sType=="DATE"){
                colType.push_back(DT_DATE);
                cols[i]=Util::createVector(DT_DATE,0);
                char* ptr=(char*)malloc(mIndex*sizeof(int));
                if(ptr == nullptr)    throw std::bad_alloc();
                buffer.push_back(ptr);
            }
            else if(sType=="MONTH"){
                colType.push_back(DT_MONTH);
                cols[i]=Util::createVector(DT_MONTH,0);
                char* ptr=(char*)malloc(mIndex*sizeof(int));
                if(ptr == nullptr)    throw std::bad_alloc();
                buffer.push_back(ptr);
            }
            else if(sType=="TIME"){
                colType.push_back(DT_TIME);
                cols[i]=Util::createVector(DT_TIME,0);
                char* ptr=(char*)malloc(mIndex*sizeof(int));
                if(ptr == nullptr)    throw std::bad_alloc();
                buffer.push_back(ptr);
            }
            else if(sType=="MINUTE"){
                colType.push_back(DT_MINUTE);
                cols[i]=Util::createVector(DT_MINUTE,0);
                char* ptr=(char*)malloc(mIndex*sizeof(int));
                if(ptr == nullptr)    throw std::bad_alloc();
                buffer.push_back(ptr);
            }
            else if(sType=="SECOND"){
                colType.push_back(DT_SECOND);
                cols[i]=Util::createVector(DT_SECOND,0);
                char* ptr=(char*)malloc(mIndex*sizeof(int));
                if(ptr == nullptr)    throw std::bad_alloc();
                buffer.push_back(ptr);
            }
            else if(sType=="DATETIME"){
                colType.push_back(DT_DATETIME);
                cols[i]=Util::createVector(DT_DATETIME,0);
                char* ptr=(char*)malloc(mIndex*sizeof(long long));
                if(ptr == nullptr)    throw std::bad_alloc();
                buffer.push_back(ptr);
            }
            else if(sType=="TIMESTAMP"){
                colType.push_back(DT_TIMESTAMP);
                cols[i]=Util::createVector(DT_TIMESTAMP,0);
                char* ptr=(char*)malloc(mIndex*sizeof(long long));
                if(ptr == nullptr)    throw std::bad_alloc();
                buffer.push_back(ptr);
            }
            else if(sType=="NANOTIME"){
                colType.push_back(DT_NANOTIME);
                cols[i]=Util::createVector(DT_NANOTIME,0);
                char* ptr=(char*)malloc(mIndex*sizeof(long long));
                if(ptr == nullptr)    throw std::bad_alloc();
                buffer.push_back(ptr);
            }
            else if(sType=="NANOTIMESTAMP"){
                colType.push_back(DT_NANOTIMESTAMP);
                cols[i]=Util::createVector(DT_NANOTIMESTAMP,0);
                char* ptr=(char*)malloc(mIndex*sizeof(long long));
                if(ptr == nullptr)    throw std::bad_alloc();
                buffer.push_back(ptr);
            }
            else if(sType=="FLOAT"){
                colType.push_back(DT_FLOAT);
                cols[i]=Util::createVector(DT_FLOAT,0);
                char* ptr=(char*)malloc(mIndex*sizeof(float));
                if(ptr == nullptr)    throw std::bad_alloc();
                buffer.push_back(ptr);
            }
            else if(sType=="DOUBLE"){
                colType.push_back(DT_DOUBLE);
                cols[i]=Util::createVector(DT_DOUBLE,0);
                char* ptr=(char*)malloc(mIndex*sizeof(double));
                if(ptr == nullptr)    throw std::bad_alloc();
                buffer.push_back(ptr);
            }
            else if(sType=="SYMBOL"){
                colType.push_back(DT_SYMBOL);
                cols[i]=Util::createVector(DT_SYMBOL,0);
                char* ptr=NULL;
                buffer.push_back(ptr);
                mmap[colName[i]]=std::vector<std::string>();
            }
            else if(sType=="STRING"){
                colType.push_back(DT_STRING);
                cols[i]=Util::createVector(DT_STRING,0);
                char* ptr=NULL;
                buffer.push_back(ptr);
                mmap[colName[i]]=std::vector<std::string>();
            }
            else{
                throw IllegalArgumentException(__FUNCTION__, "The DolphinDB "+vecType->getString(i)+" type does not support conversions. ");
            }
        }
    }
    
    //find collection for database
    int dbFIndex=collection.find_first_of(':');
    string dbTmp=db_;
    mongoc_database_t *dbPtr=mdatabase_;
    if(dbFIndex!=-1){
        dbTmp=collection.substr(0,dbFIndex);
        collection=collection.substr(dbFIndex+1);
    }
    if(dbTmp==""){
        throw IllegalArgumentException(__FUNCTION__, "There is no database name");
    }
    if(db_!=dbTmp){
        dbPtr= mongoc_client_get_database (mclient_, dbTmp.c_str());
        if(dbPtr==NULL){
            throw IllegalArgumentException(__FUNCTION__, "Database name have error");
        }
        Defer df([=](){mongoc_database_destroy(dbPtr);});
        char **collectionList=mongoc_database_get_collection_names_with_opts(dbPtr,NULL,NULL);
        Defer df1([=](){bson_strfreev(collectionList);});
        if(collectionList==NULL){
            throw IllegalArgumentException(__FUNCTION__, "Not authorized on database\""+dbTmp+"\" to query data");
        }
        bool flag=false;
        for (int i = 0; collectionList[i]; i++){
            if(collection==collectionList[i]){
                flag=true;
                break;
            }
        }
        if(!flag){
            throw IllegalArgumentException(__FUNCTION__, "The collection does not exist on the given database");
        }
    }
    else{
        char **collectionList=mongoc_database_get_collection_names_with_opts(mdatabase_,NULL,NULL);
        Defer df1([=](){bson_strfreev(collectionList);});
        if(collectionList==NULL){
            throw IllegalArgumentException(__FUNCTION__, "The collection does not exist on the given database");
        }
        bool flag=false;
        for (int i = 0; collectionList[i]; i++){
            if(collection==collectionList[i]){
                flag=true;
                break;
            }
        }
        if(!flag){
            throw IllegalArgumentException(__FUNCTION__, "The collection does not exist on the given database");
        }
    }
    
    mongoc_collection_t* mcollection = mongoc_client_get_collection (mclient_, dbTmp.c_str(),collection.c_str());
    if(mcollection==NULL){
        throw IllegalArgumentException(__FUNCTION__, "Connect Mongodb collection faild");
    }
    Defer df([=](){ mongoc_collection_destroy(mcollection);});
    const char * str=condition.c_str();
    bson_t* bsonQuery=bson_new_from_json((const uint8_t*)str,-1,NULL);
    if(bsonQuery==NULL){
        string strTmp="The BSON structure doesn't fit:";
        throw IllegalArgumentException(__FUNCTION__,strTmp +str);
    }
    Defer df2([=](){bson_destroy(bsonQuery);});
    str=option.c_str();
    bson_t* boption=bson_new_from_json((const uint8_t*)str,-1,NULL);
    if(boption==NULL){
        string strTmp="The BSON structure doesn't fit:";
        throw IllegalArgumentException(__FUNCTION__, strTmp+str);
    }
    Defer df3([=](){bson_destroy(boption);});
    mongoc_cursor_t* cursor;
    if(aggregate)
        cursor = mongoc_collection_aggregate(mcollection, MONGOC_QUERY_NONE, bsonQuery, boption, NULL);
    else
        cursor = mongoc_collection_find_with_opts (mcollection,bsonQuery,boption,NULL);

    Defer df4([=](){mongoc_cursor_destroy(cursor);});
    realLoad(colName,colType,cols,schemaEx,mIndex,buffer,mmap,nullMap,bsonQuery,boption,cursor,mcollection);
    TableSP ret=Util::createTable(colName,cols);
    return ret;
}

TableSP mongoConnection::load(std::string collection,std::string condition,std::string option,TableSP schema, bool aggregate){
    LockGuard<Mutex> LockGuard(&mtx_);
    TableSP ret;
    ret=extractLoad(collection,condition,option,schema, aggregate);
    return ret;
}

VectorSP mongoConnection::mongodbGetCollectionNames(string database){
    vector<string> collectionNames;
    if(database == "")
        database = "admin";
    if(db_!=database){
        mongoc_database_t* dbPtr= mongoc_client_get_database (mclient_, database.c_str());
        if(dbPtr==NULL){
            throw IllegalArgumentException(__FUNCTION__, "Database name have error");
        }
        Defer df([=](){mongoc_database_destroy(dbPtr);});
        char **collectionList=mongoc_database_get_collection_names_with_opts(dbPtr,NULL,NULL);
        if(collectionList==NULL){
            throw IllegalArgumentException(__FUNCTION__, "Not authorized on database\""+database+"\" to query data");
        }
        for (int i = 0; collectionList[i]; i++){
            collectionNames.emplace_back(collectionList[i]);
        }
        bson_strfreev(collectionList);
    }
    else{
        char **collectionList=mongoc_database_get_collection_names_with_opts(mdatabase_,NULL,NULL);
        if(collectionList==NULL){
            throw IllegalArgumentException(__FUNCTION__, "The collection does not exist on the given database");
        }
        for (int i = 0; collectionList[i]; i++){
            collectionNames.emplace_back(collectionList[i]);
        }
        bson_strfreev(collectionList);
    }
    int size = collectionNames.size();
    VectorSP ret = Util::createVector(DT_STRING, 0, size);
    ret->appendString(collectionNames.data(), size);
    return ret;
}


ConstantSP mongodbGetCollections(Heap *heap, vector<ConstantSP> &arguments){
    string dataBase;
    if (arguments.size() > 1)
    {
        if (arguments[1]->getType() != DT_STRING || arguments[1]->getForm() != DF_SCALAR)
            throw RuntimeException("database must be a string scalar. ");
        dataBase = arguments[1]->getString();
    }
    return safeOp(arguments[0], [&](mongoConnection *conn) { return conn->mongodbGetCollectionNames(dataBase); });
}


using namespace nlohmann;

int ARRAY_VECTOR_TYPE_BASE = 64;

ConstantSP mongodbParseJson(Heap *heap, vector<ConstantSP> &arguments)
{
    if(arguments[0]->getForm() != DF_VECTOR || arguments[0]->getType() != DT_STRING){
        throw RuntimeException("str must be a string vector. ");
    }
    if(arguments[1]->getForm() != DF_VECTOR || arguments[1]->getType() != DT_STRING)
        throw RuntimeException("origin colNames must be a string vector. ");
    if(arguments[2]->getForm() != DF_VECTOR || arguments[2]->getType() != DT_STRING)
        throw RuntimeException("convert colNames must be a string vector. ");
    if(arguments[3]->getForm() != DF_VECTOR || arguments[3]->getType() != DT_INT)
        throw RuntimeException("types must be a int vector. ");
    vector<string> originCol, convertCol;
    DictionarySP originVec = arguments[1];
    DictionarySP convertVec = arguments[2];
    if(originVec->size() != convertVec->size())
        throw RuntimeException("origin colNames must be equal than convert colNames. ");

    VectorSP vec = arguments[0];
    int rows = vec->size();

    //get colName 
    int colSize = originVec->size();
    for(int i = 0; i < colSize; ++i){
        originCol.emplace_back(originVec->getString(i));
        convertCol.push_back(convertVec->getString(i));
    }

    //create output vectors
    VectorSP types = arguments[3];
    int maxIndex = 1024;
    if(types->size() != colSize)
        throw RuntimeException("types size must be equal than origin colNames. ");
    vector<ConstantSP> cols;
    vector<vector<char>> dataBuffer;
    vector<vector<string>> dataStringBuffer;
    vector<vector<INDEX>> arrayDataIndex;
    vector<vector<char>> arrayDataBuffer;
    cols.resize(colSize);
    dataStringBuffer.resize(colSize);
    dataBuffer.resize(colSize);
    arrayDataIndex.resize(colSize);
    vector<DATA_TYPE> colTypes;
    for (int i = 0; i < colSize; ++i)
    {
        DATA_TYPE type = (DATA_TYPE)types->getInt(i);
        if ((int)type < ARRAY_VECTOR_TYPE_BASE)
        {
            cols[i] = Util::createVector(type, 0, rows);
            switch (type)
            {
            case DT_BOOL:
                dataBuffer[i].resize(rows * sizeof(bool));
                break;
            case DT_INT:
                dataBuffer[i].resize(rows * sizeof(int));
                break;
            case DT_FLOAT:
                dataBuffer[i].resize(rows * sizeof(float));
                break;
            case DT_DOUBLE:
                dataBuffer[i].resize(rows * sizeof(double));
                break;
            case DT_STRING:
                dataStringBuffer[i].resize(rows * sizeof(char *));
                break;
            case DT_SYMBOL:
                dataStringBuffer[i].resize(rows * sizeof(char *));
                break;

            default:
                throw RuntimeException("The dolphindb type " + Util::getDataTypeString(type) + " is not supported");
            }
        }
        else
        {
            //cols[i] = InternalUtil::createArrayVector((DATA_TYPE)type, 0);
            switch (type - ARRAY_VECTOR_TYPE_BASE)
            {
            case DT_BOOL:
                dataBuffer[i].resize(rows * sizeof(bool));
                break;
            case DT_INT:
                dataBuffer[i].resize(rows * sizeof(int));
                break;
            case DT_FLOAT:
                dataBuffer[i].resize(rows * sizeof(float));
                break;
            case DT_DOUBLE:
                dataBuffer[i].resize(rows * sizeof(double));
                break;

            default:
                throw RuntimeException("The dolphindb type " + Util::getDataTypeString(type) + " is not supported");
            }
        }
        colTypes.push_back(type);
    }


    vector<string> originData;
    originData.resize(rows);
    char *buffer[maxIndex];
    int times = rows / maxIndex + 1;
    for (int timeIndex = 0; timeIndex < times; ++timeIndex)
    {
        // cout<<times<<endl;
        int subSize = min(maxIndex, rows - maxIndex * timeIndex);
        // cout<<subSize<<endl;
        char **ptr = vec->getStringConst(maxIndex * timeIndex, subSize, buffer);
        for (int subRowIndex = 0; subRowIndex < subSize; ++subRowIndex)
        {
            originData[subRowIndex] = ptr[subRowIndex];
        }
        for (int subRowIndex = 0; subRowIndex < subSize; ++subRowIndex)
        {
            json parsedObj = json::parse(originData[subRowIndex]);
            for (int colIndex = 0; colIndex < colSize; ++colIndex)
            {

                json col = parsedObj[originCol[colIndex]];
                // if (col.type() == json::value_t::array)
                // {
                //     if (col.size() != 1)
                //         throw RuntimeException("col " + originCol[colIndex] + " index " + to_string(subRowIndex + timeIndex * maxIndex) + " json array size must be one.");
                //     col = col[0];
                // }
                // cout<<col.type_name()<<endl;
                try
                {
                    if ((int)colTypes[colIndex] < ARRAY_VECTOR_TYPE_BASE)
                    {
                        int dataOffect = subRowIndex + timeIndex * maxIndex;
                        switch (colTypes[colIndex])
                        {
                        case DT_BOOL:
                            if (col.is_null())
                                ((char *)dataBuffer[colIndex].data())[dataOffect] = CHAR_MIN;
                            else
                                ((bool *)dataBuffer[colIndex].data())[dataOffect] = col.get<bool>();
                            break;
                        case DT_INT:
                            if (col.is_null())
                                ((int *)dataBuffer[colIndex].data())[dataOffect] = INT_MIN;
                            else
                                ((int *)dataBuffer[colIndex].data())[dataOffect] = col.get<int>();
                            break;
                        case DT_FLOAT:
                            if (col.is_null())
                                ((float *)dataBuffer[colIndex].data())[dataOffect] = FLT_NMIN;
                            else
                                ((float *)dataBuffer[colIndex].data())[dataOffect] = col.get<int>();
                            break;
                        case DT_DOUBLE:
                            if (col.is_null())
                                ((double *)dataBuffer[colIndex].data())[dataOffect] = DBL_NMIN;
                            else
                                ((double *)dataBuffer[colIndex].data())[dataOffect] = col.get<double>();
                            break;
                        case DT_STRING:
                            if (col.is_null())
                                dataStringBuffer[colIndex][dataOffect] = "";
                            else
                                dataStringBuffer[colIndex][dataOffect] = col.get<string>();
                            break;
                        case DT_SYMBOL:
                            if (col.is_null())
                                dataStringBuffer[colIndex][dataOffect] = "";
                            else
                                dataStringBuffer[colIndex][dataOffect] = col.get<string>();
                            break;

                        default:
                            throw RuntimeException("The dolphindb type " + Util::getDataTypeString(colTypes[colIndex]) + "is not supported to convert");
                        }
                    }
                    else
                    {
                        int size = col.size();
                        vector<INDEX>& colDataArrayIndex = arrayDataIndex[colIndex];
                        int preIndex = colDataArrayIndex.size() == 0 ? 0 : colDataArrayIndex[colDataArrayIndex.size() - 1];
                        if (col.type() == json::value_t::array && col.type() == json::value_t::null)
                            throw RuntimeException("The col " + originCol[colIndex] + " json data must be array. ");
                        switch (colTypes[colIndex] - ARRAY_VECTOR_TYPE_BASE)
                        {
                        case DT_BOOL:
                            if((preIndex + size + 1) * sizeof(bool) > dataBuffer[colIndex].size())
                                dataBuffer[colIndex].resize(max(dataBuffer[colIndex].size(), (preIndex + size + 1) * sizeof(bool)) * 2);
                            if (col.is_null() || col.size() == 0){
                                ((char *)dataBuffer[colIndex].data())[preIndex] = CHAR_MIN;
                                arrayDataIndex[colIndex].push_back(preIndex + 1);
                            }
                            else{
                                int index = 0;
                                for(auto it = col.begin(); it < col.end(); ++it, ++index){
                                    if(it->is_null())
                                        ((char *)dataBuffer[colIndex].data())[preIndex + index] = CHAR_MIN;
                                    else
                                        ((bool *)dataBuffer[colIndex].data())[preIndex + index] = it->get<bool>();
                                }
                                arrayDataIndex[colIndex].push_back(preIndex + size);
                            }
                            break;
                        case DT_INT:
                            if((preIndex + size + 1) * sizeof(int) > dataBuffer[colIndex].size())
                                dataBuffer[colIndex].resize(max(dataBuffer[colIndex].size(), (preIndex + size + 1) * sizeof(int)) * 2);
                            if (col.is_null() || col.size() == 0){
                                ((int *)dataBuffer[colIndex].data())[preIndex] = INT_MIN;
                                arrayDataIndex[colIndex].push_back(preIndex + 1);
                            }
                            else{
                                int index = 0;
                                for(auto it = col.begin(); it < col.end(); ++it, ++index){
                                    if(it->is_null())
                                        ((int *)dataBuffer[colIndex].data())[preIndex + index] = INT_MIN;
                                    else
                                        ((int *)dataBuffer[colIndex].data())[preIndex + index] = it->get<int>();
                                }
                                arrayDataIndex[colIndex].push_back(preIndex + size);
                            }
                            break;
                        case DT_FLOAT:
                            if((preIndex + size + 1) * sizeof(float) > dataBuffer[colIndex].size())
                                dataBuffer[colIndex].resize(max(dataBuffer[colIndex].size(), (preIndex + size + 1) * sizeof(float)) * 2);
                            if (col.is_null() || col.size() == 0){
                                ((float *)dataBuffer[colIndex].data())[preIndex] = FLT_NMIN;
                                arrayDataIndex[colIndex].push_back(preIndex + 1);
                            }
                            else{
                                int index = 0;
                                for(auto it = col.begin(); it < col.end(); ++it, ++index){
                                    if(it->is_null())
                                        ((float *)dataBuffer[colIndex].data())[preIndex + index] = FLT_NMIN;
                                    else
                                        ((float *)dataBuffer[colIndex].data())[preIndex + index] = it->get<float>();
                                }
                                arrayDataIndex[colIndex].push_back(preIndex + size);
                            }
                            break;
                        case DT_DOUBLE:
                            if((preIndex + size + 1) * sizeof(double) > dataBuffer[colIndex].size())
                                dataBuffer[colIndex].resize(max(dataBuffer[colIndex].size(), (preIndex + size + 1) * sizeof(double)) * 2);
                            if (col.is_null() || col.size() == 0){
                                ((double *)dataBuffer[colIndex].data())[preIndex] = DBL_NMIN;
                                arrayDataIndex[colIndex].push_back(preIndex + 1);
                            }
                            else{
                                int index = 0;
                                for(auto it = col.begin(); it < col.end(); ++it, ++index){
                                    if(it->is_null())
                                        ((double *)dataBuffer[colIndex].data())[preIndex + index] = DBL_NMIN;
                                    else
                                        ((double *)dataBuffer[colIndex].data())[preIndex + index] = it->get<double>();
                                }
                                arrayDataIndex[colIndex].push_back(preIndex + size);
                            }
                            break;

                        default:
                            throw RuntimeException("The dolphindb type " + Util::getDataTypeString(colTypes[colIndex]) + "is not supported to convert");
                        }
                    }
                }
                catch (exception &e)
                {
                    throw RuntimeException("col " + originCol[colIndex] + " " + e.what());
                }
            }
        }
    }


        for (int colIndex = 0; colIndex < colSize; ++colIndex)
        {
            VectorSP& vec = (VectorSP&)cols[colIndex];
            if ((int)colTypes[colIndex] < ARRAY_VECTOR_TYPE_BASE)
            {
                switch (colTypes[colIndex])
                {
                case DT_BOOL:
                    vec->appendBool((char *)dataBuffer[colIndex].data(), rows);
                    break;
                case DT_INT:
                    vec->appendInt((int *)dataBuffer[colIndex].data(), rows);
                    break;
                case DT_FLOAT:
                    vec->appendFloat((float *)dataBuffer[colIndex].data(), rows);
                    break;
                case DT_DOUBLE:
                    vec->appendDouble((double *)dataBuffer[colIndex].data(), rows);
                    break;
                case DT_STRING:
                    vec->appendString((string *)dataStringBuffer[colIndex].data(), rows);
                    break;
                case DT_SYMBOL:
                    vec->appendString((string *)dataStringBuffer[colIndex].data(), rows);
                    break;

                default:
                    throw RuntimeException("The dolphindb type " + Util::getDataTypeString(colTypes[colIndex]) + "is not supported to append");
                }
            }else{
                int indexVecSize = arrayDataIndex[colIndex].size();
                VectorSP indexVec = Util::createVector(DT_INDEX, indexVecSize, indexVecSize);
                int totalRows = arrayDataIndex[colIndex].size() == 0 ? 0 : arrayDataIndex[colIndex][arrayDataIndex[colIndex].size() - 1];
                VectorSP vecValue = Util::createVector((DATA_TYPE)(colTypes[colIndex] - 64), 0, totalRows);
                indexVec->setIndex(0, indexVecSize, arrayDataIndex[colIndex].data());
                switch (colTypes[colIndex] - ARRAY_VECTOR_TYPE_BASE)
                {
                case DT_BOOL:
                    vecValue->appendBool((char *)dataBuffer[colIndex].data(), totalRows);
                    break;
                case DT_INT:
                    vecValue->appendInt((int *)dataBuffer[colIndex].data(), totalRows);
                    break;
                case DT_FLOAT:
                    vecValue->appendFloat((float *)dataBuffer[colIndex].data(), totalRows);
                    break;
                case DT_DOUBLE:
                    vecValue->appendDouble((double *)dataBuffer[colIndex].data(), totalRows);
                    break;
                default:
                    throw RuntimeException("The dolphindb type " + Util::getDataTypeString(colTypes[colIndex]) + "is not supported to append");
                }
                vector<ConstantSP> args{indexVec, vecValue};
                try{
                vec = heap->currentSession()->getFunctionDef("arrayVector")->call(heap, args);
                }catch(exception &e){
                    throw RuntimeException("Col " + originCol[colIndex] + " data fail to create arrrayVector." + e.what());
                }
            }
        }

    return Util::createTable(convertCol, cols);
}

