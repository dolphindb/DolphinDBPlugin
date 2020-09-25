#include"mongodb.h"
#include <Exceptions.h> 
#include <ScalarImp.h>
#include <Util.h>
#include <string>
#include<unordered_map>
#include <bson.h>
#include <mongoc.h>

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
    std::string str() { return "mongodb://"+user_+":"+password_+"@"+host_+":"+std::to_string(port_); }
    std::string host_;
    std::string user_;
    std::string password_;
    std::string db_;
    mongoc_client_t      *mclient=NULL;
    mongoc_database_t    *mdatabase=NULL;
    int port_=27017;
    Mutex mtx_;
    bool initialized_ = false;
    TableSP load(std::string collection,std::string condition,std::string option);
    ~mongoConnection();
};

mongoConnection::~mongoConnection(){
    mongoc_client_destroy(mclient);
    mongoc_database_destroy(mdatabase);
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
        lock.lock();
        if(!isMongoInit){
            mongoc_init ();
        }
        lock.unlock();
    }
    std::string strUri="mongodb://"+user_+":"+password_+"@"+host_+":"+std::to_string(port_);
    mclient = mongoc_client_new (strUri.c_str());
    if(mclient==NULL)throw IllegalArgumentException(__FUNCTION__, "Connect Mongodb faild");
    mdatabase = mongoc_client_get_database (mclient, db_.c_str());
    if(mdatabase==NULL)throw IllegalArgumentException(__FUNCTION__, "Connect Mongodb database faild");
    bson_t *command=BCON_NEW ("ping", BCON_INT32 (1));
    bson_error_t  error;
    bool retval = mongoc_client_command_simple (mclient, db_.c_str(), command, NULL, NULL, &error);
    bson_destroy (command);
    if(!retval){
        mongoc_client_destroy(mclient);
        mongoc_database_destroy(mdatabase);
        string strerro="The connection to the Mongo database failed";
        throw IllegalArgumentException(__FUNCTION__,strerro);
    }
}

static void mongoConnectionOnClose(Heap *heap, vector<ConstantSP> &args) {
    delete (mongoConnection *)(args[0]->getLong());
}

ConstantSP mongodbClose(const ConstantSP &handle, const ConstantSP &b){
    std::string usage = "Usage: close(conn). ";
    mongoConnection *cp = NULL;
    // parse args first
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
    if (args[4]->getType() != DT_STRING || args[4]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "db must be a string");
    }
    std::unique_ptr<mongoConnection> cup(new mongoConnection(args[0]->getString(), args[1]->getInt(), args[2]->getString(), args[3]->getString(), args[4]->getString()));
    const char *fmt = "mongodb connection to [%s]";
    vector<char> descBuf(cup->str().size() + strlen(fmt));
    sprintf(descBuf.data(), fmt, cup->str().c_str());
    FunctionDefSP onClose(Util::createSystemProcedure("mogodb connection onClose()", mongoConnectionOnClose, 1, 1));
    return Util::createResource((long long)cup.release(), descBuf.data(), onClose, heap->currentSession());
}

ConstantSP mongodbLoad(Heap *heap, vector<ConstantSP> &arguments) {
    auto args = getArgs(arguments, 4);
    std::string usage = "Usage: load(connection, condition,option).";
    std::string collection,condition,option;
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, usage + "Collection must be a string");
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
    return safeOp(args[0], [&](mongoConnection *conn) { return conn->load(collection,condition,option); });
}

std::wstring stringToWstring(const std::string &strInput,unsigned int uCodePage){
    #ifdef WINDOWS
    std::wstring strUnicode = L"";
    if (strInput.length() == 0){
        return strUnicode;
    }
    int iLength = ::MultiByteToWideChar(uCodePage, 0, strInput.c_str(), -1, NULL, 0);
    wchar_t* szDest = new wchar_t[iLength + 1];
    memset(szDest, 0, (iLength + 1) * sizeof(wchar_t));

    ::MultiByteToWideChar(uCodePage, 0, strInput.c_str(), -1, (wchar_t*) szDest, iLength);
    strUnicode = szDest;
    delete[] szDest;
    return strUnicode;

    #else
    if (strInput.empty())
    {
        return L"";
    }
    std::string strLocale = setlocale(LC_ALL, "");
    const char* pSrc = strInput.c_str();
    unsigned int iDestSize = mbstowcs(NULL, pSrc, 0) + 1;
    wchar_t* szDest = new wchar_t[iDestSize];
    wmemset(szDest, 0, iDestSize);
    mbstowcs(szDest,pSrc,iDestSize);
    std::wstring wstrResult = szDest;
    delete []szDest;
    setlocale(LC_ALL, strLocale.c_str());
    return wstrResult;
    #endif
}

std::string wstringToString(const std::wstring &wstrInput,unsigned int uCodePage){
    #ifdef WINDOWS
    std::string strAnsi = "";
    if (wstrInput.length() == 0){
        return strAnsi;
    }
    int iLength = ::WideCharToMultiByte(uCodePage, 0, wstrInput.c_str(), -1, NULL, 0,NULL, NULL);
    char* szDest = new char[iLength + 1];
    memset((void*) szDest, 0, (iLength + 1) * sizeof(char));
    ::WideCharToMultiByte(uCodePage, 0, wstrInput.c_str(), -1, szDest, iLength, NULL,NULL);
    strAnsi = szDest;
    delete[] szDest;
    return strAnsi;

    #else
    std::string strLocale = setlocale(LC_ALL, "");
    const wchar_t* pSrc = wstrInput.c_str();
    unsigned int iDestSize = wcstombs(NULL, pSrc, 0) + 1;
    char *szDest = new char[iDestSize];
    memset(szDest,0,iDestSize);
    wcstombs(szDest,pSrc,iDestSize);
    std::string strResult = szDest;
    delete []szDest;
    setlocale(LC_ALL, strLocale.c_str());
    return strResult;
    #endif
}

void conversionStr(vector<std::string>& colName){
    int len=colName.size();
    for(int i=0;i<len;++i){
        #ifdef WINDOWS
        std::wstring tmp=stringToWstring(colName[i],CP_UTF8);
        #else 
        std::wstring tmp=stringToWstring(colName[i],0);
        #endif
        int subLen=tmp.size();
        for(int j=0;j<subLen;++j){
            wchar_t t=tmp[j];
            if(0<=t&&t<128){
                if(!((L'A'<=t&&t<=L'z')||(L'0'<=t&&t<=L'9')||t=='_')){
                    tmp[j]=L'_';
                }
            }
        }
        if(tmp[0]==L'_')tmp=L'c'+tmp;
        #ifdef WINDOWS
        colName[i]=wstringToString(tmp,CP_UTF8);
        #else 
        colName[i]=wstringToString(tmp,0);
        #endif
    }
}
TableSP mongoConnection::load(std::string collection,std::string condition,std::string option){
    int len=0;
    int mIndex=1024;
    std::string strUri="mongodb://"+user_+":"+password_+"@"+host_+":"+std::to_string(port_);
    mtx_.lock();
    mongoc_collection_t* mcollection = mongoc_client_get_collection (mclient, db_.c_str(),collection.c_str());
    if(mcollection==NULL){
        mtx_.unlock();
        throw IllegalArgumentException(__FUNCTION__, "Connect Mongodb collection faild");
    }
    const char * str=condition.c_str();
    bson_t* bsonQuery=bson_new_from_json((const uint8_t*)str,-1,NULL);
    if(bsonQuery==NULL){
        mtx_.unlock();
        string strTmp="This BSON structure doesn't fit:";
        throw IllegalArgumentException(__FUNCTION__,strTmp +str);
    }
    str=option.c_str();
    bson_t* boption=bson_new_from_json((const uint8_t*)str,-1,NULL);
    if(boption==NULL){
        mtx_.unlock();
        string strTmp="This BSON structure doesn't fit:";
        throw IllegalArgumentException(__FUNCTION__, strTmp+str);
    }
        mongoc_cursor_t* cursor=mongoc_collection_find_with_opts (mcollection,bsonQuery,boption,NULL);
    long long curNum=mongoc_cursor_get_limit(cursor);
    vector<std::string> colName;
    vector<DATA_TYPE>  colType;
    vector<ConstantSP> cols;
    vector<char*> buffer;
    unordered_map<string,vector<string>> mmap;
    unordered_map<int,long long> nullMap;
    
    const bson_t* doc;
    int index=0;
    bool first=true;
    int rowSum=0;
    while(mongoc_cursor_next(cursor,&doc)){
        rowSum++;
        bson_iter_t biter;
        bson_iter_init(&biter,doc);
        if(first){
            while(bson_iter_next(&biter)){
                const char* ss=bson_iter_key(&biter);
                colName.push_back(ss);
                bson_type_t btype=bson_iter_type(&biter);
                switch(btype){
                    case BSON_TYPE_DOUBLE:{
                        colType.push_back(DT_DOUBLE);
                        char* ptr=(char*)malloc(mIndex*sizeof(double));
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
                        buffer.push_back(ptr);
                        int t=bson_iter_int32(&biter);
                        *((int*)(buffer[len])+index)=t;
                        break;
                    }
                    case BSON_TYPE_INT64:{
                        colType.push_back(DT_LONG);
                        char* ptr=(char*)malloc(mIndex*sizeof(long long ));
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
                        buffer.push_back(ptr);
                        bool t=bson_iter_bool(&biter);
                        *((char*)(buffer[len])+index)=t;
                        break;
                    }
                    case BSON_TYPE_DATE_TIME:{
                        colType.push_back(DT_TIMESTAMP);
                        char* ptr=(char*)malloc(mIndex*sizeof(long long ));
                        buffer.push_back(ptr);
                        long long t=bson_iter_date_time(&biter);
                        *((long long*)(buffer[len])+index)=t;
                        break;
                    }
                    case BSON_TYPE_DECIMAL128:{
                        colType.push_back(DT_DOUBLE);
                        char* ptr=(char*)malloc(mIndex*sizeof(double));
                        buffer.push_back(ptr);
                        bson_decimal128_t dec ;
                        bson_iter_decimal128(&biter,&dec);
                        char str[100];
                        bson_decimal128_to_string(&dec,str);
                        std::string ttmp(str);
                        double t=std::stod(ttmp);
                        *((double*)(buffer[len])+index)=t;
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
                        mtx_.unlock();
                        throw IllegalArgumentException(__FUNCTION__, "Mongodb type is not supported");
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
                                double t=std::stod(ttmp);
                                *((double*)ptr+rNum%mIndex)=t;
                                nullMap.erase(i);
                                break;
                            }
                            case BSON_TYPE_NULL:{
                                nullMap[i]++;
                                break;
                            }
                            default: {
                                mtx_.unlock();
                                throw IllegalArgumentException(__FUNCTION__, "Mongodb type is not supported");
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
                                mtx_.unlock();
                                throw IllegalArgumentException(__FUNCTION__, "Data types are not supported");
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
                        case DT_HANDLE:break;
                        default: {
                            mtx_.unlock();
                            throw IllegalArgumentException(__FUNCTION__, "Data types are not supported");
                        }
                    }
                }
            }
        }
        first=false;
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
                        mtx_.unlock();
                        throw IllegalArgumentException(__FUNCTION__, "Data types are not supported");
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
                mtx_.unlock();
                throw IllegalArgumentException(__FUNCTION__, "Data types are not supported");
            }
        }
     }
     for(int i=0;i<len;++i){
         if(buffer[i]!=NULL)free(buffer[i]);
     }
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
        mongoc_cursor_t* ncursor=mongoc_collection_find_with_opts (mcollection,nquery,boption,NULL);
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
                        mtx_.unlock();
                        throw IllegalArgumentException(__FUNCTION__, "Mongodb type is not supported");
                    }
                }
            }
        }
        bson_destroy(nquery);
        mongoc_cursor_destroy(ncursor);
        if(len==0){
            mtx_.unlock();
            colName.push_back("c_id");
            cols.push_back(Util::createVector(DT_STRING,0,0));
            TableSP ret=Util::createTable(colName,cols);
            return ret; 
        }
    }
    bson_destroy(bsonQuery);
    bson_destroy(boption);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy (mcollection);
    //mongoc_cleanup ();
    mtx_.unlock();
    conversionStr(colName);
    for(int i=0;i<len;++i){
        for(int j=0;j<i;++j){
            if(colName[i]==colName[j])throw IllegalArgumentException(__FUNCTION__, "Column names cannot be the same");
        }
    }
    TableSP ret=Util::createTable(colName,cols);
    return ret;
}
