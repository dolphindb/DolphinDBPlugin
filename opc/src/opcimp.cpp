#include <iostream>
#include <vector>
#include "opcimp.h"
#include <ctime>
using namespace std;
wchar_t* T2OLE(const char* s) {
    size_t len = strlen(s) + 1;
    size_t converted = 0;
    wchar_t* ret = (wchar_t*)malloc(sizeof(wchar_t) * len);
    mbstowcs_s(&converted, ret, len, s, _TRUNCATE);
    return ret;
}

char* OLE2T(const wchar_t* s) {
    size_t len = wcslen(s) + 1;
    size_t converted = 0;
    char* ret = (char*)malloc(len);
    wcstombs_s(&converted, ret, len, s, _TRUNCATE);
    return ret;
}

void makeRemoteObject(const string& host,const IID requestedClass, const IID requestedInterface, void** interfacePtr) {
    COAUTHINFO athn;
    ZeroMemory(&athn, sizeof(COAUTHINFO));
    // Set up the NULL security information
    athn.dwAuthnLevel = RPC_C_AUTHN_LEVEL_CONNECT;
    //athn.dwAuthnLevel = RPC_C_AUTHN_LEVEL_NONE;
    athn.dwAuthnSvc = RPC_C_AUTHN_WINNT;
    athn.dwAuthzSvc = RPC_C_AUTHZ_NONE;
    athn.dwCapabilities = EOAC_NONE;
    athn.dwImpersonationLevel = RPC_C_IMP_LEVEL_IMPERSONATE;
    athn.pAuthIdentityData = NULL;
    athn.pwszServerPrincName = NULL;


    COSERVERINFO remoteServerInfo;
    ZeroMemory(&remoteServerInfo, sizeof(COSERVERINFO));
    remoteServerInfo.pAuthInfo = &athn;
    remoteServerInfo.pwszName = T2OLE(host.c_str());

    MULTI_QI reqInterface;
    reqInterface.pIID = &requestedInterface;
    reqInterface.pItf = NULL;
    reqInterface.hr = S_OK;

    HRESULT result = CoCreateInstanceEx(requestedClass,NULL, CLSCTX_REMOTE_SERVER,
                                        &remoteServerInfo, 1, &reqInterface);

    if (FAILED(result))
    {
        throw OPCException("Failed to get remote interface");
    }
    *interfacePtr = reqInterface.pItf; // avoid ref counter getting incremented again
}

void coInit() {
    CoInitialize(nullptr);
    CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
}

void coRelease() {
    CoUninitialize();
}

void OPCClient::getServerList() {
    getServerList(_serverNameList, _serverCLSIDList);
}

void OPCClient::getServerList(std::vector<std::string> &serverNameList, std::vector<CLSID> &serverCLSIDList) {
    //create com instance
    IOPCServerList *pCatInfo = NULL;
    HRESULT res;
    makeRemoteObject(_host, CLSID_OpcServerList, IID_IOPCServerList, (void**)&pCatInfo);

    IEnumCLSID *pEnumCLSID = NULL;
    CATID pcatidImpl[1];
    pcatidImpl[0]=(CATID)IID_CATID_OPCDAServer20;

    //Now enumerate the classes i.e. COM objects of this type.
    res = pCatInfo->EnumClassesOfCategories (1, pcatidImpl, 1, pcatidImpl , &pEnumCLSID);

    if(FAILED(res)) {
        coRelease();
        throw OPCException("fail enum");
    }

    // get names and progIDs
    GUID glist;
    ULONG actual;
    std::vector<std::string> vec;
    while( (res = pEnumCLSID->Next( 1, &glist, &actual ))==S_OK ) {
        WCHAR* progID;
        WCHAR* userType;
        HRESULT res = pCatInfo->GetClassDetails(glist, &progID, &userType);/*ProgIDFromCLSID(glist, &progID)*/
        if(FAILED(res)) {
            coRelease();
            throw OPCException("get class details fail");
        }
        cout << std::string(OLE2T(progID)) << endl;
        printf("CLSID: {%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}\n",
               glist.Data1, glist.Data2, glist.Data3,
               glist.Data4[0], glist.Data4[1], glist.Data4[2], glist.Data4[3],
               glist.Data4[4], glist.Data4[5], glist.Data4[6], glist.Data4[7]);
        serverNameList.push_back(std::string(OLE2T(progID)));
        serverCLSIDList.push_back(glist);
    }
    pCatInfo->Release();
    pEnumCLSID->Release();
}

void OPCClient::connectToOPCServer(string& serverName) {
    // first step, get serve CLSID
    try {
      getServerList();
    }
    catch(exception& ex){
      cout<<"Fail to get server list:"<<string(ex.what())<<endl;
      throw OPCException("Failed to get the server list");
    }
    // find
    CLSID cid;
    bool flag = false;
    for(int i = 0, size = _serverNameList.size();i < size;i++) {
        if(_serverNameList[i] == serverName) {
            cid = _serverCLSIDList[i];
            flag = true;
        }
    }
    if(!flag) {
        throw OPCException("can not find server");
    }
    // try get IUnknown interface
    IUnknown* iOPCServer;

    makeRemoteObject(_host, cid, IID_IUnknown, (void**)&iOPCServer);
    iOPCServer->QueryInterface(IID_IOPCServer, (void**)&_opcServer);
    iOPCServer->Release();
    // release
    HRESULT res = _opcServer->QueryInterface(IID_IOPCBrowseServerAddressSpace, (void**)&_iOpcNamespace);
    if (FAILED(res)){
        throw OPCException("Failed to obtain IID_IOPCBrowseServerAddressSpace interface");
    }

    res = _opcServer->QueryInterface(IID_IOPCItemProperties, (void**)&_iOpcProperties);
    if (FAILED(res)){
        throw OPCException("Failed to obtain IID_IOPCItemProperties interface");
    }

    _connected = true;
    _endSubFlag = true;
}

void OPCClient::disconnect() {
    // release opcserver
    //_opcServer->Release();
    _opcServer->Release();
    _iOpcNamespace->Release();
    _iOpcProperties->Release();

    _opcServer = NULL;
    if(group != NULL)
        delete group;
    _endSubFlag = true;
    _connected = false;
}

COPCGroup *OPCClient::makeGroup(const std::string & groupName, bool active, unsigned long reqUpdateRate_ms, unsigned long &revisedUpdateRate_ms, float deadBand){

    group = new COPCGroup(groupName, active, reqUpdateRate_ms, revisedUpdateRate_ms, deadBand, *this);
    return group;
}

COPCGroup::COPCGroup(const std::string & groupName, bool active, unsigned long reqUpdateRate_ms, unsigned long &revisedUpdateRate_ms, float deadBand, OPCClient &client):
        opcClient(client),
        asynchDataCallBackHandler(NULL),
        name(groupName)
{
    WCHAR* wideName = T2OLE(groupName.c_str());


    HRESULT result = opcClient.getServerInterface()->AddGroup(wideName, active, reqUpdateRate_ms, 0, 0, &deadBand,
                                                              0, &groupHandle, &revisedUpdateRate_ms, IID_IOPCGroupStateMgt, (LPUNKNOWN*)&iStateManagement);
    if (FAILED(result))
    {
        throw OPCException("Failed to Add group");
    }

    result = iStateManagement->QueryInterface(IID_IOPCSyncIO, (void**)&iSychIO);
    if (FAILED(result)){
        throw OPCException("Failed to get IID_IOPCSyncIO");
    }

    result = iStateManagement->QueryInterface(IID_IOPCAsyncIO2, (void**)&iAsych2IO);
    if (FAILED(result)){
        throw OPCException("Failed to get IID_IOPCAsyncIO2");
    }

    result = iStateManagement->QueryInterface(IID_IOPCItemMgt, (void**)&iItemManagement);
    if (FAILED(result)){
        throw OPCException("Failed to get IID_IOPCItemMgt");
    }
}

COPCGroup::~COPCGroup()
{
    opcClient.getServerInterface()->RemoveGroup(groupHandle, FALSE);
}

OPCHANDLE * COPCGroup::buildServerHandleList(std::vector<COPCItem *>& items){
    OPCHANDLE *handles = new OPCHANDLE[items.size()];
    for (unsigned i = 0; i < items.size(); i++){
        if (items[i]==NULL){
            delete []handles;
            throw OPCException("Item is NULL");
        }
        handles[i] = items[i]->getHandle();
    }
    return handles;
}


void COPCGroup::readSync(std::vector<COPCItem *>& items,  map<ItemID, OPCItemData *>& opcData, OPCDATASOURCE source){
    OPCHANDLE *serverHandles = buildServerHandleList(items);
    HRESULT *itemResult;
    OPCITEMSTATE *itemState;
    DWORD noItems = (DWORD)items.size();

    HRESULT	result = iSychIO->Read(source, noItems, serverHandles, &itemState, &itemResult);
    if (FAILED(result)){
        delete []serverHandles;
        throw OPCException("Read failed");
    }

    for (unsigned i = 0; i < noItems; i++){
        ItemID itemId = itemState[i].hClient;
        //cout << i << endl;
        //cout << itemId << endl;
        //cout << itemState[i].ftTimeStamp.dwHighDateTime << ":" << itemState[i].ftTimeStamp.dwLowDateTime << ":" << itemState[i].wQuality << ":" << itemState[i].vDataValue.intVal << endl;
        OPCItemData * data = CAsynchDataCallback::makeOPCDataItem(itemState[i].vDataValue, itemState[i].wQuality, itemState[i].ftTimeStamp, itemResult[i]);
        opcData[itemId] = data;
    }

    delete []serverHandles;
    CoTaskMemFree(itemState);
    CoTaskMemFree(itemResult);
    //delete itemResult;
    //delete itemState;
}

COPCItem::COPCItem(std::string &itemName, COPCGroup &g):
        group(g), name(itemName){
    _itemID = global_id + 1;
    global_id += 1;
}

int COPCGroup::addItems(std::vector<std::string>& itemName, std::vector<COPCItem *>& itemsCreated, std::vector<HRESULT>& errors, bool active){
    itemsCreated.resize(itemName.size());
    errors.resize(itemName.size());
    OPCITEMDEF *itemDef = new OPCITEMDEF[itemName.size()];
    unsigned i = 0;
    for (; i < itemName.size(); i++){
        itemsCreated[i] = new COPCItem(itemName[i],*this);
        itemDef[i].szItemID = T2OLE(itemName[i].c_str());
        itemDef[i].szAccessPath = NULL;//wideName;
        itemDef[i].bActive = active;
        itemDef[i].hClient = itemsCreated[i]->getID();
        itemDef[i].dwBlobSize = 0;
        itemDef[i].pBlob = NULL;
        itemDef[i].vtRequestedDataType = VT_EMPTY;
    }

    HRESULT *itemResult;
    OPCITEMRESULT *itemDetails;
    DWORD noItems = (DWORD)itemName.size();

    HRESULT	result = getItemManagementInterface()->AddItems(noItems, itemDef, &itemDetails, &itemResult);
    delete[] itemDef;
    if (FAILED(result)){
        throw OPCException("Failed to add items");
    }

    int errorCount = 0;
    for (i = 0; i < noItems; i++){
        if(itemDetails[i].pBlob){
            delete itemDetails[0].pBlob;
        }

        if (FAILED(itemResult[i])){
            delete itemsCreated[i];
            itemsCreated[i] = NULL;
            errors[i] = itemResult[i];
            errorCount++;
        } else {
            (itemsCreated[i])->setOPCParams(itemDetails[i].hServer, itemDetails[i].vtCanonicalDataType, itemDetails[i].dwAccessRights);
            errors[i] = ERROR_SUCCESS;
        }
    }


    //delete itemDetails;
    //delete itemResult;

    return errorCount;
}
void COPCGroup::removeItems( DWORD dwCount, OPCHANDLE *phServer){
  HRESULT *itemResult;
  HRESULT	result = getItemManagementInterface()->RemoveItems(dwCount, phServer, &itemResult);
  if (FAILED(result)){
    throw OPCException("Failed to remove items");
  }
}

void COPCGroup::enableAsynch(unique_ptr<IAsynchDataCallback>&& handler){
    if (!asynchDataCallBackHandler == false){
        throw OPCException("Asynch already enabled");
    }

    IConnectionPointContainer* iConnectionPointContainer = 0;
    HRESULT result = iStateManagement->QueryInterface(IID_IConnectionPointContainer, (void**)&iConnectionPointContainer);
    if (FAILED(result))
    {
        throw OPCException("Could not get IID_IConnectionPointContainer");
    }

    result = iConnectionPointContainer->FindConnectionPoint(IID_IOPCDataCallback, &iAsynchDataCallbackConnectionPoint);
    if (FAILED(result))
    {
        throw OPCException("Could not get IID_IOPCDataCallback");
    }


    asynchDataCallBackHandler = new CAsynchDataCallback(*this);
    result = iAsynchDataCallbackConnectionPoint->Advise(asynchDataCallBackHandler, &callbackHandle);
    //cout << callbackHandle << endl;
    if (FAILED(result))
    {
        iAsynchDataCallbackConnectionPoint = NULL;
        asynchDataCallBackHandler = NULL;
        throw OPCException("Failed to set DataCallbackConnectionPoint");
    }
    //CoTaskMemFree(iConnectionPointContainer);
    //CoTaskMemFree(iAsynchDataCallbackConnectionPoint);
    //iConnectionPointContainer->Release();
    //iAsynchDataCallbackConnectionPoint->Release();
    userAsynchCBHandler = move(handler);
}

void COPCGroup::disableAsynch() {
    IConnectionPointContainer* iConnectionPointContainer = 0;
    HRESULT result = iStateManagement->QueryInterface(IID_IConnectionPointContainer, (void**)&iConnectionPointContainer);
    if (FAILED(result))
    {
        throw OPCException("Could not get IID_IConnectionPointContainer");
    }

    result = iConnectionPointContainer->FindConnectionPoint(IID_IOPCDataCallback, &iAsynchDataCallbackConnectionPoint);
    if (FAILED(result))
    {
        throw OPCException("Could not get IID_IOPCDataCallback");
    }

//    asynchDataCallBackHandler = new CAsynchDataCallback(*this);
    result = iAsynchDataCallbackConnectionPoint->Unadvise(callbackHandle);
    //cout << callbackHandle << endl;
    if (FAILED(result))
    {
        throw OPCException("Failed to disableAsynch");
    }
    iAsynchDataCallbackConnectionPoint = NULL;
    asynchDataCallBackHandler = NULL;
}

void COPCItem::setOPCParams(OPCHANDLE handle, VARTYPE type, DWORD dwAccess){
    serversItemHandle	=handle;
    vtCanonicalDataType	=type;
    dwAccessRights		=dwAccess;
}

void COPCItem::writeSync(VARIANT &data){
    HRESULT * itemWriteErrors;
    HRESULT result = group.getSychIOInterface()->Write(1, &serversItemHandle, &data, &itemWriteErrors);
    if (FAILED(result))
    {
        throw OPCException("write failed");
    }

    if (FAILED(itemWriteErrors[0])){
        throw OPCException("write failed");
    }
}

void COPCItem::readSync(OPCItemData& data, OPCDATASOURCE source){
    std::vector<COPCItem *> items;
    items.push_back(this);
    map<ItemID, OPCItemData*> opcData;
    group.readSync(items, opcData, source);

    auto pos = opcData.find(_itemID);
    if (pos != opcData.end()){
        OPCItemData * readData = opcData[_itemID];
        if (readData && !FAILED(readData->error)){
            data = *readData;
            return;
        }
    }

    throw OPCException("Read failed");
}

OPCItemData::OPCItemData(HRESULT err):error(err){
    vDataValue.vt = VT_EMPTY;
}

OPCItemData & OPCItemData::operator=(OPCItemData &itemData){
    HRESULT result = VariantCopy( &vDataValue, &(itemData.vDataValue));
    if (FAILED(result)){
        throw OPCException("VarCopy failed");
    }

    ftTimeStamp = itemData.ftTimeStamp;
    wQuality = itemData.wQuality;

    return *this;
}


OPCItemData::OPCItemData(FILETIME time, WORD qual, VARIANT & val, HRESULT err){
    vDataValue.vt = VT_EMPTY;
    HRESULT result = VariantCopy( &vDataValue, &val);
    if (FAILED(result)){
        throw OPCException("VarCopy failed");
    }

    ftTimeStamp = time;
    wQuality = qual;
    error = err;
}
OPCItemData::~OPCItemData(){
    VariantClear(&vDataValue);
}

OPCItemData::OPCItemData(){
    vDataValue.vt = VT_EMPTY;
}
COPCItem * COPCGroup::addItem(std::string &itemName, bool active)
{
    std::vector<std::string> names;
    std::vector<COPCItem *> itemsCreated;
    std::vector<HRESULT> errors;
    names.push_back(itemName);
    if (addItems(names, itemsCreated, errors, active)!= 0){
        //return NULL;
        throw OPCException("Failed to add item");
    }
    return itemsCreated[0];
}


void OPCClient::getItemNames(std::vector<std::string> & opcItemNames){
    if (!_iOpcNamespace) return;

    OPCNAMESPACETYPE nameSpaceType;
    HRESULT result = _iOpcNamespace->QueryOrganization(&nameSpaceType);

    WCHAR emptyString[] = {0};
    //result = iOpcNamespace->ChangeBrowsePosition(OPC_BROWSE_TO,emptyString);

    IEnumString* iEnum;
    result = _iOpcNamespace->BrowseOPCItemIDs(OPC_FLAT,emptyString,VT_EMPTY,0,(&iEnum));
    if (FAILED(result)){
        return;
    }


    WCHAR * str;
    ULONG strSize;
    while((result = iEnum->Next(1, &str, &strSize)) == S_OK)
    {
        WCHAR * fullName;
        result = _iOpcNamespace->GetItemID(str, &fullName);
        if (SUCCEEDED(result)){
            char* cStr = OLE2T(fullName);
            //char * cStr = OLE2T(str);
            //printf("Adding %s\n", cStr);
            opcItemNames.push_back((char*)cStr);
            //delete fullName;
        }
        //delete str;
    }
}

CAsynchDataCallback::CAsynchDataCallback(COPCGroup &group):callbacksGroup(group){
        mRefCount = 0;
}

STDMETHODIMP CAsynchDataCallback::OnDataChange(DWORD Transid, OPCHANDLE grphandle, HRESULT masterquality,
                          HRESULT mastererror, DWORD count, OPCHANDLE * clienthandles,
                          VARIANT * values, WORD * quality, FILETIME  * time,
                          HRESULT * errors) {

    IAsynchDataCallback * usrHandler = callbacksGroup.getUsrAsynchHandler();
    if (usrHandler){
        map<ItemID, OPCItemData *>  dataChanges;
        updateOPCData(dataChanges, count, clienthandles, values,quality,time,errors);
        usrHandler->OnDataChange(callbacksGroup, dataChanges);
    }
    return S_OK;
}

void FileTimeToMs( FILETIME ft, unsigned long long& t ) {
    ULARGE_INTEGER ui;
    ui.LowPart = ft.dwLowDateTime;
    ui.HighPart = ft.dwHighDateTime;
    t = ((LONGLONG)(ui.QuadPart - 116444736000000000) / 10000);
}

void MsToFileTime( FILETIME & ft, unsigned long long t ) {
    ULARGE_INTEGER ui;
    ui.QuadPart = t*10000 + 116444736000000000;
    ft.dwLowDateTime = ui.LowPart;
    ft.dwHighDateTime = ui.HighPart;
}

void log_item(OPCItemData& data) {
    unsigned long long ms;
    FileTimeToMs(data.ftTimeStamp, ms);
    time_t sec = ms/1000;
    auto timeinfo=localtime(&sec);
    printf("%d-%d-%d %d:%d:%d:%lld\n",timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour , timeinfo->tm_min, timeinfo->tm_sec, ms%1000);
    cout << data.wQuality << ":" << data.vDataValue.intVal << endl;
}

