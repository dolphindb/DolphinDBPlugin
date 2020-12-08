#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/client_subscriptions.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/plugin/securitypolicy.h>
#include <open62541/plugin/securitypolicy_default.h>
#include <open62541/types.h>
#include <open62541/types_generated_handling.h>
#include <open62541/client_config_default.h>
#include <open62541/network_tcp.h>
#include <open62541/plugin/accesscontrol_default.h>
#include <open62541/plugin/pki_default.h>
#include "CoreConcept.h"
#include "ScalarImp.h"
#include <map>
#include <vector>

extern "C" ConstantSP getOpcUaServerList(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP getOpcUaEndPointList(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP connectOpcUaServer(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP disconnect(const ConstantSP& handle, const ConstantSP& b );
extern "C" ConstantSP readNode(Heap* heap, vector<ConstantSP>& arguments );
extern "C" ConstantSP writeNode(Heap* heap, vector<ConstantSP>& arguments );
extern "C" ConstantSP subscribeNode(Heap* heap, vector<ConstantSP>& arguments );
extern "C" ConstantSP endSub(const ConstantSP& handle, const ConstantSP& b );
extern "C" ConstantSP browseNode(Heap* heap, vector<ConstantSP>& arguments);
extern "C" ConstantSP getSubscriberStat(const ConstantSP &handle, const ConstantSP &b);

static UA_INLINE UA_Int64
UA_DateTime_toUnixTimeStamp(UA_DateTime date) {
    return (date + UA_DateTime_localTimeUtcOffset()- UA_DATETIME_UNIX_EPOCH) / UA_DATETIME_MSEC;
}
static UA_INLINE UA_DateTime
UnixTimeStamp_To_UA_DateTime(UA_Int64 date) {
    return date * UA_DATETIME_MSEC + UA_DATETIME_UNIX_EPOCH - UA_DateTime_localTimeUtcOffset();
}


class DummyOutput: public Output{
public:
    virtual bool timeElapsed(long long nanoSeconds){return true;}
    virtual bool write(const ConstantSP& obj){return true;}
    virtual bool message(const string& msg){return true;}
    virtual void enableIntermediateMessage(bool enabled) {}
    virtual IO_ERR done(){return OK;}
    virtual IO_ERR done(const string& errMsg){return OK;}
    virtual bool start(){return true;}
    virtual bool start(const string& message){return true;}
    virtual IO_ERR writeReady(){return OK;}
    virtual ~DummyOutput(){}
    virtual OUTPUT_TYPE getOutputType() const {return STDOUT;}
    virtual void close() {}
    virtual void setWindow(INDEX index,INDEX size){};
    virtual IO_ERR flush() {return OK;}
};

class OPCUAClient;
class OPCUASub:public Runnable {
public:
    static std::map<UA_UInt32,OPCUASub* > mapSub_;
    OPCUASub(Heap* heap,  UA_Client* client, string endPoint, string clientUri, OPCUAClient* opcuaClient):
        heap_(heap), client_(client), endPoint_(endPoint), clientUri_(clientUri), opcuaClient_(opcuaClient){}
    void setArgs(vector<int>& nsIdx, vector<string>& nodeIdString, ConstantSP& handle){
        nsIdx_ = nsIdx;
	    nodeIdString_ = nodeIdString;
	    handle_ = handle;
    }
    void subs();
    void unSub(){
        /* Delete the subscription */
        subFlag_ = false;
        Util::sleep(1000);
        UA_StatusCode retVal = UA_Client_Subscriptions_deleteSingle(client_, subId_);
        mapSub_.erase(subId_);
        if(retVal!=UA_STATUSCODE_GOOD){
            throw RuntimeException("Could not call unsubscribe service. StatusCode " + string(UA_StatusCode_name(retVal)));
        }
	    UA_Client_run_iterate(client_, 1000);
	
    }
    void run() override {
        while (running_) {
            if(subFlag_){
                UA_StatusCode retVal = UA_Client_run_iterate(client_, 100); 
                if(retVal!=UA_STATUSCODE_GOOD){
                    subFlag_ = false;
                    running_ = false;
                    //throw RuntimeException(string(UA_StatusCode_name(retVal)));

                }
            }
            
        }
    }
    bool getSubed(){return subFlag_;}
    ConstantSP getHandle(){return handle_;}
    Heap* getHeap(){return heap_;}
    std::map<UA_UInt32, string> monMap;
    void setRunning(bool run){running_ = run; }
    string getClientUri(){return clientUri_;}
    void setUser(const string &u){user_ = u;}
    string getUser(){return user_;}
    void addRecP(){recv_++;}
    long long getRecP(){return recv_;}
    string getSubID(){return std::to_string(subId_);};
    string getEndPoint(){return endPoint_;};
    long long getCreateTime(){return createTime_;};
    string getNodeID(){
        string result = "";
        for(size_t i = 0; i < nsIdx_.size();i++){
            result += std::to_string(nsIdx_[i])+":"+nodeIdString_[i]+";";
        }
        return result;
    }
    string getErrorMsg(){ return errorMsg_; }
    void setErrorMsg(string errorMsg){ errorMsg_ = errorMsg; }
    OPCUAClient* getOPCUAClient(){return opcuaClient_;}
private:
    Heap* heap_;
    UA_Client* client_ = nullptr;
    string endPoint_;
    string clientUri_;
    OPCUAClient *opcuaClient_;

    vector<int> nsIdx_;
    vector<string> nodeIdString_;
    ConstantSP handle_ = nullptr;
    UA_UInt32 subId_ = 0;
    bool subFlag_ = false;
    bool running_ = true;
    string user_ = "";
    long long createTime_ = 0;
    string errorMsg_ ="";
    long long recv_ = 0;
    

};

class OPCUAClient{
public:
    OPCUAClient(){
        client_ = UA_Client_new();
    }
    ~OPCUAClient(){
	    endSub();
        UA_Client_delete(client_);
        // delete thread;
        // delete sub;
    }
    void connect(string endPointUrl, string clientUri, string username, string password,UA_MessageSecurityMode securityMode, UA_String securityPolicy,UA_ByteString certificate,UA_ByteString privateKey);
    ConstantSP readNode(vector<int> &nsIdx, vector<string> &nodeIdString, ConstantSP &table);
    ConstantSP writeNode(vector<int> &nsIdx, vector<string> &nodeIdString, ConstantSP &value);
    void browseNode(UA_NodeId object, vector<int> &nameSpace, vector<string> &nodeid);
    ConstantSP browseNode();
    bool getConnected(){
        UA_ClientState state = UA_Client_getState(client_);
        if(state <= UA_ClientState::UA_CLIENTSTATE_CONNECTED)
            return false;
        return true;
    }
    bool getSubed(){
        if(sub_ == nullptr){
            return false;
        }else{
            return sub_->getSubed();
        }
    }
    void subscribe(Heap *heap, vector<int> &nsIdx, vector<string> &nodeIdString, ConstantSP &handle){
	    
	    if(sub_ == nullptr){   
            sub_ = new OPCUASub(heap, client_, connEndPointUrl_, clientUri_, this);
 	        thread_ = new Thread(sub_);
	        //thread->join();
	    }
	    sub_->setArgs(nsIdx, nodeIdString, handle);
        try{
            sub_->subs();
        }
        catch(RuntimeException &e){
            throw e;
        }
        if (!thread_->isRunning()) {
            sub_->setRunning(true);
            thread_->start();
        }
        sub_->setUser(session_->getUser()->getUserId());
    }
    void endSub(){
        if(getSubed()){
            try{
                sub_->setRunning(false);
                sub_->unSub();      
            }
            catch(RuntimeException &e){
                throw e;
            }
        }
    }
    void setSession(const SessionSP& sess){
        session_ = sess;
    }
    SessionSP getSession(){
        return session_;
    }
    bool getSessionStat(){
        return sessionClosed_;
    }
    void setSessionStat(bool flag){
        sessionClosed_ = flag;
    }
private:
    UA_Client *client_;
    OPCUASub *sub_ = nullptr;
    ThreadSP thread_;
    string connEndPointUrl_ = "";
    bool sessionClosed_ = false;
    SessionSP session_;
    string clientUri_;   
};
