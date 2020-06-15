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


static UA_INLINE UA_Int64
UA_DateTime_toUnixTimeStamp(UA_DateTime date) {
    return (date + UA_DateTime_localTimeUtcOffset()- UA_DATETIME_UNIX_EPOCH) / UA_DATETIME_MSEC;
}
static UA_INLINE UA_DateTime
UnixTimeStamp_To_UA_DateTime(UA_Int64 date) {
    return date * UA_DATETIME_MSEC + UA_DATETIME_UNIX_EPOCH - UA_DateTime_localTimeUtcOffset();
}

class OPCUASub:public Runnable {
public:
    static std::map<UA_UInt32,OPCUASub* > mapSub;
    OPCUASub(Heap* _heap,  UA_Client* _client, string _endPoint):heap(_heap), client(_client), endPoint(_endPoint){}
    void setArgs(vector<int>& _nsIdx, vector<string>& _nodeIdString, ConstantSP& _handle){
        nsIdx = _nsIdx;
	    nodeIdString = _nodeIdString;
	    handle = _handle;
    }
    void subs();
    void unSub(){
        /* Delete the subscription */
	    subFlag = false;
        running = false;
        UA_StatusCode retVal = UA_Client_Subscriptions_deleteSingle(client, subId);
        mapSub.erase(subId);
        if(retVal!=UA_STATUSCODE_GOOD){
            throw RuntimeException("Could not call unsubscribe service. StatusCode " + string(UA_StatusCode_name(retVal)));
        }
	    UA_Client_run_iterate(client, 1000);
	
    }
    void run() override {
        while (running) {
            if(subFlag){
                UA_StatusCode retVal = UA_Client_run_iterate(client, 100); 
                if(retVal!=UA_STATUSCODE_GOOD){
                    subFlag = false;
                    running = false;
                    //throw RuntimeException(string(UA_StatusCode_name(retVal)));

                }
            }
            
        }
    }
    ConstantSP getHandle(){return handle;}
    Heap* getHeap(){return heap;}
    std::map<UA_UInt32, string> monMap;
    void setRunning(bool _run){running = _run; }
private:
    Heap* heap;
    UA_Client* client = nullptr;
    vector<int> nsIdx;
    vector<string> nodeIdString;
    ConstantSP handle;
    UA_UInt32 subId;
    bool subFlag = false;
    string endPoint;
    bool running = true;
    
};

class OPCUAClient{
public:
    OPCUAClient(){
        client = UA_Client_new();
    }
    ~OPCUAClient(){
	    endSub();
        UA_Client_delete(client);
    }
    void connect(string endPointUrl, string clientUri, string username, string password,UA_MessageSecurityMode securityMode, UA_String securityPolicy,UA_ByteString certificate,UA_ByteString privateKey);
    ConstantSP readNode(vector<int> &nsIdx, vector<string> &nodeIdString, ConstantSP &table);
    ConstantSP writeNode(vector<int> &nsIdx, vector<string> &nodeIdString, ConstantSP &value);
    void browseNode(UA_NodeId object, vector<int> &nameSpace, vector<string> &nodeid);
    ConstantSP browseNode();
    bool getConnected(){
        UA_ClientState state = UA_Client_getState(client);
        if(state <= UA_ClientState::UA_CLIENTSTATE_CONNECTED)
            return false;
        return true;
    }
    bool getSubed(){
        return subFlag;
    }
    void subscribe(Heap *heap, vector<int> &nsIdx, vector<string> &nodeIdString, ConstantSP &handle){
	    
	    if(sub == nullptr){   
            sub = new OPCUASub(heap, client, connEndPointUrl);
 	        thread = new Thread(sub);
	        //thread->join();
	    }
	    sub->setArgs(nsIdx, nodeIdString, handle);
        try{
            sub->subs();
        }
        catch(RuntimeException e){
            throw e;
        }
        if (!thread->isRunning()) {
            sub->setRunning(true);
            thread->start();
        }
        subFlag = true;
    }
    void endSub(){
        if(subFlag){
            try{
                sub->unSub();      
            }
            catch(RuntimeException e){
                return;
            }
        }
	    subFlag = false;
    }

private:
UA_Client *client;
OPCUASub *sub = nullptr;
ThreadSP thread;
bool subFlag = false;
string connEndPointUrl = "";
};
