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
        std::string str() { return "mongodb://"+user_+":"+password_+"@"+host_+":"+std::to_string(port_)+"/?authSource="+db_; }
    std::string host_;
    std::string user_;
    std::string password_;
    std::string db_;
    mongoc_client_t      *mclient=NULL;
    mongoc_database_t    *mdatabase=NULL;
    int port_=27017;
    Mutex mtx_;
    bool initialized_ = false;
    TableSP extractLoad(std::string& collection,std::string& condition,std::string& option,TableSP& schema);
    TableSP load(std::string collection,std::string condition,std::string option,TableSP schema);
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
    mclient = mongoc_client_new (strUri.c_str());
    if(mclient==NULL)throw IllegalArgumentException(__FUNCTION__, "User id or password is incorrect for the given database");
    mdatabase = mongoc_client_get_database (mclient, db_.c_str());
    if(mdatabase==NULL){
        mongoc_client_destroy(mclient);
        throw IllegalArgumentException(__FUNCTION__, "User id or password is incorrect for the given database");
    }
    if(user_==""&&password_==""){
        mongoc_client_destroy(mclient);
        mongoc_database_destroy(mdatabase);
        char **test=mongoc_client_get_database_names_with_opts(mclient,NULL,NULL);
        if(!test)throw IllegalArgumentException(__FUNCTION__, "User id or password is incorrect for the given database");
    }
    bson_error_t  errors;
    bson_t *command=BCON_NEW ("ping", BCON_INT32 (1));
    bool retval = mongoc_client_command_simple (mclient, db_.c_str(), command, NULL, NULL, &errors);
    bson_destroy (command);
    if(!retval){
        mongoc_client_destroy(mclient);
        mongoc_database_destroy(mdatabase);
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
    std::string usage = "Usage: load(connection, condition,option).";
    std::string collection,condition,option;
    TableSP schema = nullptr;
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
    if (arguments.size()==5&&!arguments[4]->isNothing()) {
        if (!arguments[4]->isTable()) {
            throw IllegalArgumentException(__FUNCTION__, usage + "schema must be a table");
        }
        schema = arguments[4];
    }
    return safeOp(args[0], [&](mongoConnection *conn) { return conn->load(collection,condition,option,schema); });
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
                                throw RuntimeException("Mongodb type is not supported");
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
                                        default:throw RuntimeException("unsupported data type");
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
                                        default:throw RuntimeException("unsupported data type");
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
                                        default:throw RuntimeException("unsupported data type");
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
                                        default:throw RuntimeException("unsupported data type");
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
                                        default:throw RuntimeException("unsupported data type");
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
                                        default:throw RuntimeException("unsupported data type");
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
                                        default:throw RuntimeException("unsupported data type");
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
                                        default:throw RuntimeException("unsupported data type");
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
                                        default:throw RuntimeException("unsupported data type");
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
                                        default:throw RuntimeException("unsupported data type");
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
                                        default:throw RuntimeException("unsupported data type");
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
                                        default:throw RuntimeException("unsupported data type");
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
                                        default:throw RuntimeException("unsupported data type");
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
                                        default:throw RuntimeException("unsupported data type");
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
                                        default:throw RuntimeException("unsupported data type");
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
                                        default:throw RuntimeException("unsupported data type");
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
                                        default:throw RuntimeException("unsupported data type");
                                    }
                                    break;
                                }
                                default: {
                                    throw RuntimeException("Data types are not supported");
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
                                throw RuntimeException("Data types are not supported");
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
                        case DT_HANDLE:break;
                        default: {
                            throw RuntimeException("Data types are not supported");
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
                        throw RuntimeException("Data types are not supported");
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
                throw RuntimeException("Data types are not supported");
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
                        throw RuntimeException("Mongodb type is not supported");
                    }
                }
            }
        }
        bson_destroy(nquery);
        mongoc_cursor_destroy(ncursor);
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
}

TableSP mongoConnection::extractLoad(std::string &collection,std::string &condition,std::string &option,TableSP &schema){
    bool schemaEx=false;
    vector<std::string> colName;
    vector<DATA_TYPE>  colType;
    vector<ConstantSP> cols;
    vector<char*> buffer;
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
                buffer.push_back(ptr);
            }
            else if(sType=="CHAR"){
                colType.push_back(DT_CHAR);
                cols[i]=Util::createVector(DT_CHAR,0);
                char* ptr=(char*)malloc(mIndex*sizeof(char));
                buffer.push_back(ptr);
            }
           else if(sType=="SHORT"){
                colType.push_back(DT_SHORT);
                cols[i]=Util::createVector(DT_SHORT,0);
                char* ptr=(char*)malloc(mIndex*sizeof(short));
                buffer.push_back(ptr);
            }
            else if(sType=="INT"){
                colType.push_back(DT_INT);
                cols[i]=Util::createVector(DT_INT,0);
                char* ptr=(char*)malloc(mIndex*sizeof(int));
                buffer.push_back(ptr);
            }
            else if(sType=="LONG"){
                colType.push_back(DT_LONG);
                cols[i]=Util::createVector(DT_LONG,0);
                char* ptr=(char*)malloc(mIndex*sizeof(long long));
                buffer.push_back(ptr);
            }
            else if(sType=="DATE"){
                colType.push_back(DT_DATE);
                cols[i]=Util::createVector(DT_DATE,0);
                char* ptr=(char*)malloc(mIndex*sizeof(int));
                buffer.push_back(ptr);
            }
            else if(sType=="MONTH"){
                colType.push_back(DT_MONTH);
                cols[i]=Util::createVector(DT_MONTH,0);
                char* ptr=(char*)malloc(mIndex*sizeof(int));
                buffer.push_back(ptr);
            }
            else if(sType=="TIME"){
                colType.push_back(DT_TIME);
                cols[i]=Util::createVector(DT_TIME,0);
                char* ptr=(char*)malloc(mIndex*sizeof(int));
                buffer.push_back(ptr);
            }
            else if(sType=="MINUTE"){
                colType.push_back(DT_MINUTE);
                cols[i]=Util::createVector(DT_MINUTE,0);
                char* ptr=(char*)malloc(mIndex*sizeof(int));
                buffer.push_back(ptr);
            }
            else if(sType=="SECOND"){
                colType.push_back(DT_SECOND);
                cols[i]=Util::createVector(DT_SECOND,0);
                char* ptr=(char*)malloc(mIndex*sizeof(int));
                buffer.push_back(ptr);
            }
            else if(sType=="DATETIME"){
                colType.push_back(DT_DATETIME);
                cols[i]=Util::createVector(DT_DATETIME,0);
                char* ptr=(char*)malloc(mIndex*sizeof(long long));
                buffer.push_back(ptr);
            }
            else if(sType=="TIMESTAMP"){
                colType.push_back(DT_TIMESTAMP);
                cols[i]=Util::createVector(DT_TIMESTAMP,0);
                char* ptr=(char*)malloc(mIndex*sizeof(long long));
                buffer.push_back(ptr);
            }
            else if(sType=="NANOTIME"){
                colType.push_back(DT_NANOTIME);
                cols[i]=Util::createVector(DT_NANOTIME,0);
                char* ptr=(char*)malloc(mIndex*sizeof(long long));
                buffer.push_back(ptr);
            }
            else if(sType=="NANOTIMESTAMP"){
                colType.push_back(DT_NANOTIMESTAMP);
                cols[i]=Util::createVector(DT_NANOTIMESTAMP,0);
                char* ptr=(char*)malloc(mIndex*sizeof(long long));
                buffer.push_back(ptr);
            }
            else if(sType=="FLOAT"){
                colType.push_back(DT_FLOAT);
                cols[i]=Util::createVector(DT_FLOAT,0);
                char* ptr=(char*)malloc(mIndex*sizeof(float));
                buffer.push_back(ptr);
            }
            else if(sType=="DOUBLE"){
                colType.push_back(DT_DOUBLE);
                cols[i]=Util::createVector(DT_DOUBLE,0);
                char* ptr=(char*)malloc(mIndex*sizeof(double));
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
                throw IllegalArgumentException(__FUNCTION__, "The Type "+vecType->getString(i)+" is not supported");
            }
        }
    }
    
    //find collection for database
    int dbFIndex=collection.find_first_of(':');
    string dbTmp=db_;
    mongoc_database_t *dbPtr=mdatabase;
    if(dbFIndex!=-1){
        dbTmp=collection.substr(0,dbFIndex);
        collection=collection.substr(dbFIndex+1);
    }
    if(dbTmp==""){
        throw IllegalArgumentException(__FUNCTION__, "There is no database name");
    }
    if(db_!=dbTmp){
        dbPtr= mongoc_client_get_database (mclient, dbTmp.c_str());
        if(dbPtr==NULL){
            throw IllegalArgumentException(__FUNCTION__, "Database name have error");
        }
        char **collectionList=mongoc_database_get_collection_names_with_opts(dbPtr,NULL,NULL);
        if(collectionList==NULL){
            mongoc_database_destroy(dbPtr);
            throw IllegalArgumentException(__FUNCTION__, "Not authorized on database\""+dbTmp+"\" to query data");
        }
        bool flag=false;
        for (int i = 0; collectionList[i]; i++){
            if(collection==collectionList[i]){
                flag=true;
                break;
            }
        }
        mongoc_database_destroy(dbPtr);
        if(!flag){
            throw IllegalArgumentException(__FUNCTION__, "The collection does not exist on the given database");
        }
    }
    else{
        char **collectionList=mongoc_database_get_collection_names_with_opts(mdatabase,NULL,NULL);
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
    
    mongoc_collection_t* mcollection = mongoc_client_get_collection (mclient, dbTmp.c_str(),collection.c_str());
    if(mcollection==NULL){
        throw IllegalArgumentException(__FUNCTION__, "Connect Mongodb collection faild");
    }
    const char * str=condition.c_str();
    bson_t* bsonQuery=bson_new_from_json((const uint8_t*)str,-1,NULL);
    if(bsonQuery==NULL){
        mongoc_collection_destroy(mcollection);
        string strTmp="This BSON structure doesn't fit:";
        throw IllegalArgumentException(__FUNCTION__,strTmp +str);
    }
    str=option.c_str();
    bson_t* boption=bson_new_from_json((const uint8_t*)str,-1,NULL);
    if(boption==NULL){
        mongoc_collection_destroy(mcollection);
        bson_destroy(bsonQuery);
        string strTmp="This BSON structure doesn't fit:";
        throw IllegalArgumentException(__FUNCTION__, strTmp+str);
    }
    mongoc_cursor_t* cursor=mongoc_collection_find_with_opts (mcollection,bsonQuery,boption,NULL);

    try{
       realLoad(colName,colType,cols,schemaEx,mIndex,buffer,mmap,nullMap,bsonQuery,boption,cursor,mcollection);
    }
    catch(IllegalArgumentException& e){
        bson_destroy(bsonQuery);
        bson_destroy(boption);
        mongoc_cursor_destroy(cursor);
        mongoc_collection_destroy (mcollection);
        throw e;
    }
    catch(RuntimeException& e){
        bson_destroy(bsonQuery);
        bson_destroy(boption);
        mongoc_cursor_destroy(cursor);
        mongoc_collection_destroy (mcollection);
        throw e;
    }

    TableSP ret=Util::createTable(colName,cols);
    bson_destroy(bsonQuery);
    bson_destroy(boption);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy (mcollection);
    //mongoc_cleanup ();
    
    return ret;
}
TableSP mongoConnection::load(std::string collection,std::string condition,std::string option,TableSP schema){
    mtx_.lock();
    TableSP ret;
    try{
        ret=extractLoad(collection,condition,option,schema);
    }
    catch(IllegalArgumentException& e){
        mtx_.unlock();
        throw e;
    }
    catch(RuntimeException& e){
        mtx_.unlock();
        throw e;
    }
    mtx_.unlock();
    return ret;
}
