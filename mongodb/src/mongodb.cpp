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

TableSP mongoConnection::load(std::string collection,std::string condition,std::string option){
    int len=0;
    int mIndex=1024;
    std::string strUri="mongodb://"+user_+":"+password_+"@"+host_+":"+std::to_string(port_);
    mtx_.lock();
    mongoc_collection_t* mcollection = mongoc_client_get_collection (mclient, db_.c_str(),collection.c_str());
    if(mcollection==NULL)throw IllegalArgumentException(__FUNCTION__, "Connect Mongodb collection faild");
    const char * str=condition.c_str();
    bson_t* bsonQuery=bson_new_from_json((const uint8_t*)str,-1,NULL);
    if(bsonQuery==NULL)throw IllegalArgumentException(__FUNCTION__, str);
    str=option.c_str();
    bson_t* boption=bson_new_from_json((const uint8_t*)str,-1,NULL);
    if(boption==NULL)throw IllegalArgumentException(__FUNCTION__, "The BSON query condition is incorrect");
        mongoc_cursor_t* cursor=mongoc_collection_find_with_opts (mcollection,bsonQuery,boption,NULL);
    long long curNum=mongoc_cursor_get_limit(cursor);
    vector<std::string> colName;
    vector<DATA_TYPE>  colType;
    vector<ConstantSP> cols;
    vector<char*> buffer;
    unordered_map<string,vector<string>> mmap;
    
    const bson_t* doc;
    int index=0;
    bool first=true;
    while(mongoc_cursor_next(cursor,&doc)){
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
                    default: throw IllegalArgumentException(__FUNCTION__, "Mongodb type is not supported");
                }
                len++;
            }
            cols.resize(len);
            for(int i=0;i<len;++i)cols[i]=Util::createVector(colType[i],0,curNum);
        }
        else{
            for(int i=0;i<len;++i){
                if(bson_iter_find(&biter,colName[i].c_str())){
                    bson_type_t btype=bson_iter_type(&biter);
                    switch(btype){
                        case BSON_TYPE_DOUBLE:{
                            double t=bson_iter_double(&biter);
                            *((double*)(buffer[i])+index)=t;
                            break;
                        }
                        case BSON_TYPE_UTF8:{
                            std::string t=bson_iter_utf8(&biter,NULL);
                            mmap[colName[i]].push_back(t);
                            break;
                        }
                        case BSON_TYPE_INT32:{
                            int t=bson_iter_int32(&biter);
                            *((int*)(buffer[i])+index)=t;
                            break;
                        }
                        case BSON_TYPE_INT64:{
                            long long t=bson_iter_int64(&biter);
                            *((long long*)(buffer[i])+index)=t;
                            break;
                        }
                        case BSON_TYPE_OID:{
                            char tmp[25];
                            bson_oid_to_string(bson_iter_oid(&biter),tmp);
                            std::string ttmp(tmp);
                            mmap[colName[i]].push_back(ttmp);
                            break;
                        }
                        case BSON_TYPE_BOOL:{
                            bool t=bson_iter_bool(&biter);
                            *((bool*)(buffer[i])+index)=t;
                            break;
                        }
                        case BSON_TYPE_DATE_TIME:{
                            long long t=bson_iter_date_time(&biter);
                            *((long long*)(buffer[i])+index)=t;
                            break;
                        }
                        case BSON_TYPE_DECIMAL128:{
                            bson_decimal128_t dec ;
                            bson_iter_decimal128(&biter,&dec);
                            char str[100];
                            bson_decimal128_to_string(&dec,str);
                            std::string ttmp(str);
                            double t=std::stod(ttmp);
                            *((double*)(buffer[i])+index)=t;
                            break;
                        }
                        case BSON_TYPE_SYMBOL:{
                            std::string t=bson_iter_symbol(&biter,NULL);
                            mmap[colName[i]].push_back(t);
                            break;
                        }
                        default: throw IllegalArgumentException(__FUNCTION__, "Mongodb type is not supported");
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
                            *((float*)(buffer[i])+index)=FLT_MIN;
                            break;
                        case DT_DOUBLE:
                            *((float*)(buffer[i])+index)=DBL_MIN;
                            break;
                        case DT_SYMBOL:
                        case DT_STRING:
                            mmap[colName[i]].push_back(string());
                            break;
                        default: throw IllegalArgumentException(__FUNCTION__, "Data types are not supported");
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
                    default: throw IllegalArgumentException(__FUNCTION__, "Data types are not supported");
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
            default: throw IllegalArgumentException(__FUNCTION__, "Data types are not supported");
        }
     }
     for(int i=0;i<len;++i){
         if(buffer[i]!=NULL)free(buffer[i]);
     }
    bson_destroy(bsonQuery);
    bson_destroy(boption);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy (mcollection);
    //mongoc_cleanup ();
    mtx_.unlock();
    if(len==0)throw IllegalArgumentException(__FUNCTION__, "Failed to parse MongoDB data, check user, database, collection, and query");
    TableSP ret=Util::createTable(colName,cols);
    return ret;
}