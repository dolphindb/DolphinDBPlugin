#ifndef DOLPHINDBPLUGIN_OPCIMPDOL_H
#define DOLPHINDBPLUGIN_OPCIMPDOL_H

#endif //DOLPHINDBPLUGIN_OPCIMPDOL_H
#include <windows.h>
#include <combaseapi.h>
#include <comcat.h>
#include <vector>
#include <map>
#include <memory>
#include "opcexception.h"
#include "opcda_i.h"
#include "opccomn.h"
#include "opcda.h"
// global id
static unsigned long long global_id = 0;
//
//static unsigned long long global_client_id = 0;
// types
typedef unsigned long long ItemID;
// structs
struct  OPCItemData{
    FILETIME ftTimeStamp;
    WORD wQuality;
    VARIANT vDataValue;
    HRESULT error;


    OPCItemData(HRESULT err);



    OPCItemData(FILETIME time, WORD qual, VARIANT & val, HRESULT err);


    OPCItemData();



    ~OPCItemData();


    void set(OPCITEMSTATE &itemState);


    void set(FILETIME time, WORD qual, VARIANT & val);

    OPCItemData & operator=(OPCItemData &itemData);
};

struct CPropertyDescription{
    /// properties identifier
    DWORD id;

    /// server supplied textual description
    std::string desc;

    /// data type of the property
    VARTYPE type;

    CPropertyDescription(DWORD i, std::string d, VARTYPE t):id(i),desc(d), type(t){};
};
struct SPropertyValue{
    /// the property that this value describes.
    const CPropertyDescription & propDesc;

    /// properties value.
    VARIANT value;

    SPropertyValue(const CPropertyDescription &desc, VARIANT &val);

    ~SPropertyValue();
};



//classes
class OPCClient;
class COPCGroup;
class CAsynchDataCallback;
class CTransaction;
class COPCItem;
class ITransactionComplete;
class IAsynchDataCallback;

// wchar <-> char
wchar_t* T2OLE(const char* s);
char* OLE2T(const wchar_t* s);

// com init and uninit
void coInit();
void coRelease();
// remote get interface
void makeRemoteObject(const std::string& host,const IID requestedClass, const IID requestedInterface, void** interfacePtr);

class OPCClient {
private:
    const std::string _host;
    std::vector<std::string> _serverNameList;
    std::vector<CLSID> _serverCLSIDList;
    IOPCServer* _opcServer;
    IOPCBrowseServerAddressSpace* _iOpcNamespace;
    IOPCItemProperties* _iOpcProperties;

    bool _connected;
    bool _endSubFlag;

public:
    explicit OPCClient(std::string& host, int id): _host(host), threadId_(id) {
        coInit();
        _connected = false;
        group = NULL;
        _endSubFlag = true;
    }
    ~OPCClient() {
        //if(_connected) disconnect();
        coRelease();
    }
    void getServerList(std::vector<std::string>& serverNameList, std::vector<CLSID>& serverCLSIDList);
    void getServerList();
    void connectToOPCServer(string& serverName);
    void disconnect();
    IOPCServer* getServerInterface() {
        _opcServer->AddRef();
        return _opcServer;
    };
    COPCGroup *makeGroup(const std::string & groupName, bool active, unsigned long reqUpdateRate_ms, unsigned long &revisedUpdateRate_ms, float deadBand);
    void getItemNames(std::vector<std::string> & opcItemNames);
    
    bool getConnected(){return _connected;}
    void setSubFlag(bool s){_endSubFlag=s;}
    bool getSubFlag(){return _endSubFlag;}
    int id() { return threadId_; }
public:
    COPCGroup* group;
    int threadId_;
};


/**
* Handles OPC (DCOM) callbacks at the group level. It deals with the receipt of data from asynchronous operations.
* This is a fake COM object.
*/
class CAsynchDataCallback : public IOPCDataCallback
{
private:
    DWORD mRefCount;

    /**
    * group this is a callback for
    */
    COPCGroup &callbacksGroup;


public:
    CAsynchDataCallback(COPCGroup &group);


    ~CAsynchDataCallback(){

    }

    /**
    * Functions associated with IUNKNOWN
    */
    STDMETHODIMP QueryInterface( REFIID iid, LPVOID* ppInterface){
        if ( ppInterface == NULL){
            return E_INVALIDARG;
        }

        if ( iid == IID_IUnknown ){
            *ppInterface = (IUnknown*) this;
        } else if ( iid == IID_IOPCDataCallback){
            *ppInterface = (IOPCDataCallback*) this;
        } else
        {
            *ppInterface = NULL;
            return E_NOINTERFACE;
        }


        AddRef();
        return S_OK;
    }


    STDMETHODIMP_(ULONG) AddRef(){
        return ++mRefCount;
    }


    STDMETHODIMP_(ULONG) Release(){
        --mRefCount;

        if ( mRefCount == 0){
            delete this;
        }
        return mRefCount;
    }

    /**
    * Functions associated with IOPCDataCallback
    */

    STDMETHODIMP OnDataChange(DWORD Transid, OPCHANDLE grphandle, HRESULT masterquality,
                              HRESULT mastererror, DWORD count, OPCHANDLE * clienthandles,
                              VARIANT * values, WORD * quality, FILETIME  * time,
                              HRESULT * errors);


    STDMETHODIMP OnReadComplete(DWORD Transid, OPCHANDLE grphandle,
                                HRESULT masterquality, HRESULT mastererror, DWORD count,
                                OPCHANDLE * clienthandles, VARIANT* values, WORD * quality,
                                FILETIME * time, HRESULT * errors) { return S_OK;}
                                /*
    {
        // TODO this is bad  - server could corrupt address - need to use look up table
        CTransaction & trans = *(CTransaction *)Transid;
        updateOPCData(trans.opcData, count, clienthandles, values,quality,time,errors);
        trans.setCompleted();
        return S_OK;
    }*/


    STDMETHODIMP OnWriteComplete(DWORD Transid, OPCHANDLE grphandle, HRESULT mastererr,
                                 DWORD count, OPCHANDLE * clienthandles, HRESULT * errors) {return S_OK;}
                                 /*
    {
        // TODO this is bad  - server could corrupt address - need to use look up table
        CTransaction & trans = *(CTransaction *)Transid;

        // see page 145 - number of items returned may be less than sent
        for (unsigned i = 0; i < count; i++){
            // TODO this is bad  - server could corrupt address - need to use look up table
            COPCItem * item = (COPCItem *)clienthandles[i];
            trans.setItemError(item, errors[i]); // this records error state - may be good
        }

        trans.setCompleted();
        return S_OK;
    }*/



    STDMETHODIMP OnCancelComplete(DWORD transid, OPCHANDLE grphandle){
        printf("OnCancelComplete: Transid=%ld GrpHandle=%ld\n", transid, grphandle);
        return S_OK;
    }


    /**
    * make OPC item
    */
    static OPCItemData * makeOPCDataItem(VARIANT& value, WORD quality, FILETIME & time, HRESULT error){
        OPCItemData * data = NULL;
        if (FAILED(error)){
            data = new OPCItemData(error);
        } else {
            data = new OPCItemData(time,quality,value,error);
        }
        return data;
    }

    /**
    * Enter the OPC items data that resulted from an operation
    */
    static void updateOPCData(map<ItemID, OPCItemData *> &opcData, DWORD count, OPCHANDLE * clienthandles,
                              VARIANT* values, WORD * quality,FILETIME * time, HRESULT * errors){
        // see page 136 - returned arrays may be out of order
        for (unsigned i = 0; i < count; i++){

            ItemID itemID = clienthandles[i];
            OPCItemData * data = makeOPCDataItem(values[i], quality[i], time[i], errors[i]);
            opcData[itemID] = data;
        }
    }
};

class ITransactionComplete{
public:
    virtual void complete(CTransaction &transaction) = 0;
};

class IAsynchDataCallback
{
public:
    virtual void OnDataChange(COPCGroup & group, map<ItemID, OPCItemData *> & changes) = 0;
};

class COPCGroup
{
private:
    IOPCGroupStateMgt*	iStateManagement;
    IOPCSyncIO*		    iSychIO;
    IOPCAsyncIO2*	    iAsych2IO;
    IOPCItemMgt*		iItemManagement;

    /**
    * Used to keep track of the connection point for the
    * AsynchDataCallback
    */
    IConnectionPoint* iAsynchDataCallbackConnectionPoint;


    /**
    * handle given the group by the server
    */
    DWORD groupHandle;

    /**
    * The server this group belongs to
    */
    OPCClient &opcClient;


    /**
    * Callback for asynch data at the group level
    */
    CAsynchDataCallback* asynchDataCallBackHandler;

    /**
     * Callback for common callback
     */
    CAsynchDataCallback* dataCallbackHandler;

    /**
    * list of OPC items associated with this goup. Not owned (at the moment!)
    */
    std::vector<COPCItem *> items;


    /**
    * Name of the group
    */
    const std::string name;


    /**
    * Handle given to callback by server.
    */
    DWORD callbackHandle;


    /**
    * Users hander to handle asynch data
    * NOT OWNED.
    */
    unique_ptr<IAsynchDataCallback> userAsynchCBHandler;
    CAsynchDataCallback* _CAsynchDataCallback;

    /**
    * Caller owns returned array
    */
    OPCHANDLE * buildServerHandleList(std::vector<COPCItem *>& items);

public:

    map<int,string> mapTag;
    COPCGroup(const std::string & groupName, bool active, unsigned long reqUpdateRate_ms, unsigned long &revisedUpdateRate_ms, float deadBand, OPCClient &server);

    virtual ~COPCGroup();


    COPCItem * addItem(std::string &itemName, bool active);

    /**
    * returns the number of failed item creates
    * itemsCreated[x] will be null if could not create and will contain error code in corresponding error entry
    */
    int addItems(std::vector<std::string>& itemName, std::vector<COPCItem *>& itemsCreated, std::vector<HRESULT>& errors, bool active);

    void removeItems( DWORD dwCount, OPCHANDLE *phServer);
    /**
    * enable Asynch IO
    */
    void enableAsynch(unique_ptr<IAsynchDataCallback>&& handler);

    /**
    * disable Asych IO
    */
    void disableAsynch();


    /**
    * set the group state values.
    */
    void setState(DWORD reqUpdateRate_ms, DWORD &returnedUpdateRate_ms, float deadBand, BOOL active);



    /**
    * Read set of OPC items synchronously.
    */
    void readSync(std::vector<COPCItem *>& items, map<ItemID, OPCItemData *> &opcData, OPCDATASOURCE source);


    /**
    * Read a defined group of OPC item asynchronously
    */
    CTransaction * readAsync(std::vector<COPCItem *>& items, ITransactionComplete *transactionCB = NULL);


    /**
    * Refresh is an asysnch operation.
    * retreives all active items in the group, which will be stored in the transaction object
    * Transaction object is owned by caller.
    * If group asynch is disabled then this call will not work
    */
    CTransaction * refresh(OPCDATASOURCE source, ITransactionComplete *transactionCB = NULL);



    IOPCSyncIO* getSychIOInterface() {
        iSychIO->AddRef();
        return iSychIO;
    }


    IOPCAsyncIO2* getAsych2IOInterface() {
        iAsych2IO->AddRef();
        return iAsych2IO;
    }


    IOPCItemMgt* &getItemManagementInterface(){
        iItemManagement->AddRef();
        return iItemManagement;
    }

    const std::string & getName() const {
        return name;
    }

    IAsynchDataCallback *getUsrAsynchHandler(){
        return userAsynchCBHandler.get();
    }

    /**
    * returns reaference to the OPC server that this group belongs to.
    */
    OPCClient & getServer(){
        return opcClient;
    }
};

class CTransaction{

    /**
    * Optional transation complete callback - not owned
    */
    ITransactionComplete * completeCallBack;

    // true when the transaction has completed
    bool completed;


    DWORD cancelID;


public:
    /**
    * keyed on OPCitem address (not owned)
    * OPCitem data is owned by the transaction - may be NULL
    */
    map<COPCItem *, OPCItemData *> opcData;


    CTransaction(ITransactionComplete * completeCB = NULL);

    /**
    * Used where the transaction completion will result in data being received.
    */
    CTransaction(std::vector<COPCItem *>&items, ITransactionComplete * completeCB);



    void setItemError(COPCItem *item, HRESULT error);


    void setItemValue(COPCItem *item, FILETIME time, WORD qual, VARIANT & val, HRESULT err);


    /**
    * return Value stored for a given opc item.
    */
    const OPCItemData * getItemValue(COPCItem *item) const;


    /**
    * trigger completion of the transaction.
    */
    void setCompleted();

    bool isCompeleted() const{
        return completed;
    }

    void setCancelId(DWORD id){
        cancelID = id;
    }

    DWORD getCancelId() const{
        return cancelID;
    }
};

class  COPCItem
{
private:
    OPCHANDLE serversItemHandle;
    VARTYPE vtCanonicalDataType;
    DWORD dwAccessRights;

    COPCGroup& group;

    std::string name;
    ItemID _itemID;
protected:
    friend class COPCGroup;
    // used to set data for the OPC item AFTER it has been created in the server.
    void setOPCParams(OPCHANDLE handle, VARTYPE type, DWORD dwAccess);

    // items may only be created by group.
    COPCItem(std::string &itemName, COPCGroup &g);
public:

    virtual ~COPCItem() {};
    void writeSync(VARIANT &data);


    void readSync(OPCItemData& data, OPCDATASOURCE source);


    /**
    * returned transaction object is owned
    */
    CTransaction * readAsynch(ITransactionComplete *transactionCB = NULL);


    /**
    * returned transaction object is owned
    */
    CTransaction * writeAsynch(VARIANT &data, ITransactionComplete *transactionCB = NULL);


    DWORD getAccessRights() const{
        return dwAccessRights;
    }

    OPCHANDLE getHandle() const{
        return serversItemHandle;
    }

    const std::string & getName() const{
        return name;
    }

    ItemID getID() {
        return _itemID;
    }

    void getSupportedProperties(std::vector<CPropertyDescription> &desc);

    /**
    * retreive the OPC item properties for the descriptors passed. Any data previously existing in propsRead will be destroyed.
    */
    //void getProperties(const std::vector<CPropertyDescription> &propsToRead, ATL::CAutoPtrArray<SPropertyValue> &propsRead);
};

void FileTimeToMs( FILETIME ft, unsigned long long& t ) ;
void MsToFileTime( FILETIME & ft, unsigned long long t );

void log_item(OPCItemData& data) ;

enum opc_vt {
    OPC_STRING = 1,
    OPC_DOUBLE = 2,
    OPC_LONG = 3,
    OPC_ARRAY_STRING = 4,
    OPC_ARRAY_DOUBLE = 5,
    OPC_TIME = 6
};

class ValWithType {
public:
    long long int data_long_long; // 1
    string data_string; // 2
    double data_double; // 3
    vector<string> data_array_string; // 4
    vector<double> data_array_double; // 5
    unsigned long long data_time; // 6
    opc_vt vt;
public:
    explicit ValWithType(OPCItemData& data) {

        switch (data.vDataValue.vt) {
            case VT_CY:
                // money type
                data_double = data.vDataValue.cyVal.int64 / 10000.0;
                vt = OPC_DOUBLE;
                break;
            case VT_UI1:
            case VT_UI2:
                // quality
                data_long_long = data.vDataValue.uiVal;
                vt = OPC_LONG;
                break;
            case VT_UI4:
                data_long_long = data.vDataValue.uintVal;
                vt = OPC_LONG;
                break;
            case VT_DATE:
                // time
                SYSTEMTIME t;
                FILETIME t2;
                VariantTimeToSystemTime(data.vDataValue.date, &t);
                SystemTimeToFileTime(&t, &t2);
                FileTimeToMs(t2, data_time);
                data_time /= 1000;
                vt = OPC_TIME;
                break;
            case VT_R4:
                // real4
                data_double = data.vDataValue.fltVal;
                vt = OPC_DOUBLE;
                break;
            case VT_ARRAY | VT_R8: {
                long lbound, ubound;
                SafeArrayGetLBound(data.vDataValue.parray, 1, &lbound);
                SafeArrayGetUBound(data.vDataValue.parray, 1, &ubound);
                int len = ubound-lbound + 1;
                for (int i = 0; i < len; i++) {
                    if(((double *) data.vDataValue.parray->pvData) == 0) {
                        continue;
                    }
                    data_array_double.push_back(((double *) data.vDataValue.parray->pvData)[i]);
                }
                vt = OPC_ARRAY_DOUBLE;
                break;
            }
            case VT_BSTR:
                data_string = OLE2T(data.vDataValue.bstrVal);
                vt = OPC_STRING;
                break;
            case VT_BSTR | VT_ARRAY: {
                long lbound, ubound;
                SafeArrayGetLBound(data.vDataValue.parray, 1, &lbound);
                SafeArrayGetUBound(data.vDataValue.parray, 1, &ubound);
                int len = ubound-lbound + 1;
                for (int i = 0; i < len; i++) {
                    if(((BSTR *) data.vDataValue.parray->pvData)[i] == 0) {
                        continue;
                    }
                    data_array_string.push_back(OLE2T(((BSTR *) data.vDataValue.parray->pvData)[i]));
                }
                vt = OPC_ARRAY_STRING;
                break;
            }
            case VT_BOOL:
                data_long_long = data.vDataValue.boolVal;
                vt = OPC_LONG;
                break;
            case VT_I1:
                // char
                data_long_long = (char)data.vDataValue.iVal;
                vt = OPC_LONG;
                break;
            case VT_I2:
                // short
                data_long_long = data.vDataValue.iVal;
                vt = OPC_LONG;
                break;
            case VT_I4:
                data_long_long = data.vDataValue.intVal;
                vt = OPC_LONG;
                break;
            case VT_R8:
                data_double = data.vDataValue.dblVal;
                vt = OPC_DOUBLE;
                break;
            default:
                throw OPCException("unsupported now");
        }
    }
    void log() {
        switch(vt){
            case OPC_LONG:
                cout << "val: " << data_long_long << endl;
                break;
            case OPC_STRING:

                cout << "val: " << data_string << endl;
                break;
            case OPC_DOUBLE:

                cout << "val: " << data_double << endl;
                break;
            case OPC_TIME:

                cout << "val: " << data_time << endl;
                break;
            case OPC_ARRAY_DOUBLE:
                cout << "arr: " << endl;
                for(int i = 0; i < (int)data_array_double.size();i++) {
                    cout << "val" << i << ": " << data_array_double[i] << endl;
                }
                break;
            case OPC_ARRAY_STRING:
                for(int i = 0; i < (int)data_array_string.size();i++) {
                    cout << "val" << i << ": " << data_array_string[i] << endl;
                }
                break;
        }
    }
} ;
