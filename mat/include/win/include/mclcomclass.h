/* Copyright 2001-2015 The MathWorks, Inc. */


#ifndef _MCLCOMCLASS_H_
#define _MCLCOMCLASS_H_

#pragma warning( disable : 4786 )
#include "mclmcrrt.h"
#include "mwcomutil.h"
#include <olectl.h>
#include <wchar.h>

#ifdef __cplusplus

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <queue>
using namespace std;

#ifndef HWND_MESSAGE
#define HWND_MESSAGE (HWND)NULL
#endif

// Structure used to pass supporting info for objects into CMCLModule Init method.

typedef struct _MCLOBJECT_MAP_ENTRY
{
	// Pointer to CLSID value

	const CLSID* pclsid;
	// Pointer to function responsible for registering the class

	HRESULT (__stdcall* pfnRegisterClass)(const GUID*, unsigned short, unsigned short, 
                                          const char*, const char*, const char*, const char*);
	// Pointer to function responsible for unregistering the class

	HRESULT (__stdcall* pfnUnregisterClass)(const char*, const char*);
	// Pointer to function responsible for returning an instance of the object's class factory

	HRESULT (__stdcall* pfnGetClassObject)(REFCLSID, REFIID, void**);
	// Class's friendly name

	const char* lpszFriendlyName;
	// Class's version independent ProgID

	const char* lpszVerIndProgID;
	// Class's ProgID

	const char* lpszProgID;
} _MCLOBJECT_MAP_ENTRY, *MCLOBJECT_MAP_ENTRY;

class mwLock
{
public:
    mwLock()
    {
        mclAcquireMutex();
    }
    virtual ~mwLock()
    {
        mclReleaseMutex();
    }
};

class IMCLFeval
{
public:
    virtual bool Feval(HMCRINSTANCE hinst, const char* name, int nlhs, mxArray** plhs, int nrhs, mxArray** prhs) = 0;
	virtual const char* getErrorMessage() const = 0;
	virtual void RaiseEvent(OLECHAR* lpwszName, DISPID dispid, IDispatch* pDisp, DISPPARAMS* pDispParams) = 0;
    virtual bool init(HINSTANCE hInstance) = 0;
    virtual bool stop() = 0;
};

class IMCLEvent
{
public:
    virtual void mclRaiseEventA(const char* lpszName, DISPID dispid, int nargin, mxArray** prhs) = 0;
};

class IMCLEventMap
{
public:
    virtual int size() = 0;
    virtual void add(void *context, IMCLEvent* pEvent) = 0;
    virtual void remove(void *context, IMCLEvent* pEvent) = 0;
    virtual void invokeA(void *context, const char* lpszName, DISPID dispid, int nargin, mxArray** prhs) = 0;
};

typedef bool (*MCLInitializeInstancePtr)(HMCRINSTANCE*, const char* path_to_component);
typedef bool (*MCLInitializeInstanceExPtr)(HMCRINSTANCE*, const char* path_to_component, 
                                           mclCtfStream ctfStream);
typedef bool (*MCLTerminateInstancePtr)(HMCRINSTANCE*);

#define WM_FEVALCOMPLETE WM_USER
#define WM_EVENTPENDING WM_USER+1

// Class for managing global list of event listeners.
class mclEventMap : public IMCLEventMap
{
public:
    mclEventMap(){}
    virtual ~mclEventMap(){}
    // Returns current number of listeners
    int size()
    {
        mwLock lock;
        return (int)m_events.size();
    }
    // Adds a listener
    void add(void *context, IMCLEvent* pEvent)
    {
        mwLock lock;
        if (!pEvent)
            return;
        std::map<void *, IMCLEvent*>::iterator it = m_events.find(context);
        if (it == m_events.end())
            m_events[context] = pEvent;
    }
    // Removes a listener
    void remove(void *context, IMCLEvent* pEvent)
    {
        (void)pEvent;
        mwLock lock;
        std::map<void *, IMCLEvent*>::iterator it = m_events.find(context);
        if (it != m_events.end())
            m_events.erase(it);
    }
    // Invokes the named event in the listener of current call context.
    void invokeA(void *context, const char* lpszName, DISPID dispid, int nargin, mxArray** prhs)
    {
        mwLock lock;
        std::map<void *, IMCLEvent*>::iterator it = m_events.find(context);
        if (it != m_events.end())
	  ((*it).second)->mclRaiseEventA(lpszName, dispid, nargin, prhs);
    }
private:
    std::map<void *, IMCLEvent*> m_events; // Array of listeners
};

// Class for managing global list of event listeners for singleton MCR case.
class mclSingleEventMap : public IMCLEventMap
{
public:
    mclSingleEventMap(){}
    virtual ~mclSingleEventMap(){}
    // Returns current number of listeners
    int size()
    {
        mwLock lock;
        return (int)m_events.size();
    }
    // Adds a listener
    void add(void *context, IMCLEvent* pEvent)
    {
        mwLock lock;
        if (!pEvent)
            return;
        m_events.insert(pEvent);
    }
    // Removes a listener
    void remove(void *context, IMCLEvent* pEvent)
    {
        context;
        mwLock lock;
        std::set<IMCLEvent*>::iterator it = m_events.find(pEvent);
        if (it != m_events.end())
            m_events.erase(it);
    }
    // Invokes the named event in the listener of current call context.
    void invokeA(void *context, const char* lpszName, DISPID dispid, int nargin, mxArray** prhs)
    {
        context;
        mwLock lock;
        std::set<IMCLEvent*>::iterator it = m_events.begin();
        while (it != m_events.end())
        {
	  (*it)->mclRaiseEventA(lpszName, dispid, nargin, prhs);
            it++;
        }
    }
private:
    std::set<IMCLEvent*> m_events; // Array of listeners
};

// Stack proxy for a mutex
class ModuleLock
{
public:
    ModuleLock()
    {
#if defined (mclcommain_h) || defined (mclxlmain_published_api_h)
        RequestGlobalLock();
#else
        mclRequestGlobalLock();
#endif
    }
    virtual ~ModuleLock()
    {
#if defined (mclcommain_h) || defined (mclxlmain_published_api_h)
        ReleaseGlobalLock();
#else
        mclReleaseGlobalLock();
#endif
    }
private:
    ModuleLock(const ModuleLock& ml);
    ModuleLock& operator=(const ModuleLock& ml);
};

class mclSimpleFeval : public IMCLFeval
{
public:
    mclSimpleFeval(){}
    virtual ~mclSimpleFeval(){}
    // Calls feval.
    virtual bool Feval(HMCRINSTANCE hinst, const char* name, int nlhs, mxArray** plhs, int nrhs, mxArray** prhs)
    {
        return mclFeval(hinst, name, nlhs, plhs, nrhs, prhs);
    }
    // Makes an event callback.
    virtual void RaiseEvent(OLECHAR* lpwszName, DISPID mdispid, IDispatch* pDisp, DISPPARAMS* pDispParams)
    {
        DISPID dispid = 0;
        VARIANT varResult;
        HRESULT hr = S_OK;

        VariantInit(&varResult);
        if (SUCCEEDED(hr = pDisp->GetIDsOfNames(IID_NULL, &lpwszName, 1, LOCALE_USER_DEFAULT, &dispid)))
        {
            hr = pDisp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pDispParams, 
                                    &varResult, NULL, NULL);
        }
    }
	virtual const char* getErrorMessage() const
	{
		return mclGetLastErrorMessage();
	}
    virtual bool init(HINSTANCE hInstance)
    {
        (void)hInstance;
        return true;
    }
    virtual bool stop()
    {
        return true;
    }
};
// Feval with events. Creates a thread on which all
// feval calls are made. Calling thread waits on feval
// thread to complete feval call. If an event arrives
// before the feval call returns, the calling thread 
// executes the callback, then resumes waiting for the
// the feval to finish. This ensures that synchronous
// callbacks are always executed by the same thread that
// made the original feval call. This is needed because 
// Visual Basic does not allow callbacks to be executed
// on a seperate thread, and the MCR always calls back on
// its thread, not the caller's.
class mclFevalWithEvents : public IMCLFeval
{
private:
    // Class used to pass feval args when being called by
    // Same thread as the message window runs on. When
    // feval is complete, posts a message to the window.
    class FevalArgs
    {
    public:
        FevalArgs(HMCRINSTANCE hinst, const char* name, int nlhs, mxArray** plhs, int nrhs, mxArray** prhs, HWND hWnd)
            : m_hinst(hinst), m_name(name), m_nlhs(nlhs), m_plhs(plhs), m_nrhs(nrhs), m_prhs(prhs), m_retval(false), m_hWnd(hWnd), m_errorMessage(""){}
        virtual ~FevalArgs(){}
        virtual bool execute()
        {
            m_retval = mclFeval(m_hinst, m_name, m_nlhs, m_plhs, m_nrhs, m_prhs);
			
			if(!m_retval)
			{
				m_errorMessage = mclGetLastErrorMessage();
			}
            
			PostMessage(m_hWnd, WM_FEVALCOMPLETE, 0, 0);
            return m_retval;
        }
        bool retval() const {return m_retval;}
		const char* getErrorMessage() const {return m_errorMessage;}
    private:
        HMCRINSTANCE m_hinst;
        const char* m_name;
        int m_nlhs;
        mxArray** m_plhs;
        int m_nrhs;
        mxArray** m_prhs;
        bool m_retval;
        HWND m_hWnd;
		const char* m_errorMessage;
    private:
        FevalArgs();
    };
    // Class used to pass event args
    class EventArgs
    {
    public:
        EventArgs(OLECHAR* lpwszName, DISPID dispid, IDispatch* pDisp, DISPPARAMS* pDispParams)
	  : m_lpwszName(lpwszName), m_dispid(dispid), m_pDisp(pDisp), m_pDispParams(pDispParams), m_hr(S_OK)
        {
            VariantInit(&m_varResult);
        }
        virtual ~EventArgs()
        {
            VariantClear(&m_varResult);
        }
        virtual bool execute()
        {
            DISPID dispid = 0;

            m_hr = m_pDisp->Invoke(m_dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, m_pDispParams, 
                                     &m_varResult, NULL, NULL);
            return (SUCCEEDED(m_hr));
        }
        HRESULT retval() {return m_hr;}
    private:
        OLECHAR* m_lpwszName;
        IDispatch* m_pDisp;
        DISPPARAMS* m_pDispParams;
        VARIANT m_varResult;
        HRESULT m_hr;
        DISPID m_dispid;
    private:
        EventArgs();
    };
public:
    mclFevalWithEvents() : m_hInstance(NULL), m_hThread(NULL), m_hWnd(NULL), m_errorMessage("") {}
    virtual ~mclFevalWithEvents()
    {
        stop();
    }
    // Calls feval. Queues up the request, then
    // waits for the call to finish.
    virtual bool Feval(HMCRINSTANCE hinst, const char* name, int nlhs, mxArray** plhs, int nrhs, mxArray** prhs)
    {
        HWND hWnd = NULL;
        BOOL bRet = FALSE;
        MSG msg = {0, 0, 0, 0, 0, {0,0}};
        if (!(hWnd = get_thread_window()))
            return false;
        FevalArgs Args(hinst, name, nlhs, plhs, nrhs, prhs, hWnd);
        // Queue the request
        add(&Args);
        // Start timer to periodically refresh windows owned by this thread
        SetTimer(hWnd, 1, 100, NULL);
        // Process window message loop. Break when feval is finished.
        while ((bRet = GetMessage(&msg, hWnd, 0, 0)) != 0)
        {
            if (bRet == -1)
            {
                return false;
            }
            else if (msg.message == WM_FEVALCOMPLETE)
            {
                break;
            }
            else if (msg.message == WM_TIMER)
            {
                DoEvents();
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        // Stop timer
        KillTimer(hWnd, 1);
		m_errorMessage = Args.getErrorMessage();
        return Args.retval();
    }

	virtual const char* getErrorMessage() const
	{
		return m_errorMessage;
	}
    // Makes an event callback. Queues up the call,
    // then waits for it to finish.
	virtual void RaiseEvent(OLECHAR* lpwszName, DISPID dispid, IDispatch* pDisp, DISPPARAMS* pDispParams)
    {
        EventArgs Args(lpwszName, dispid, pDisp, pDispParams);
        SendMessage(m_hWnd, WM_EVENTPENDING, 0, (LPARAM)(&Args));
    }
    // Performs idle processing.
    void DoEvents()
    {
        EnumThreadWindows(GetCurrentThreadId(), EnumThreadWndProc, NULL);
    }
    // Starts the feval thread
   virtual bool init(HINSTANCE hInstance)
    {
        m_hInstance = hInstance;
        if (!(m_hExit = CreateEvent(NULL, FALSE, FALSE, NULL)))
            return false;
        if (!(m_hFevalPending = CreateEvent(NULL, FALSE, FALSE, NULL)))
            return false;
        WNDCLASS wc = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        if (!GetClassInfo(m_hInstance, "feval_window_class", &wc))
        {
            WNDCLASS wc_new = {0, (WNDPROC)ModuleWndProc, 0, 0, m_hInstance, NULL, NULL, NULL, NULL, "feval_window_class"};
            if (!RegisterClass(&wc_new))
                return false;
        }
        if (!(m_hWnd = get_thread_window()))
            return false;
        if (!(m_hThread = CreateThread(NULL, 0, FevalProc, (LPVOID)this, 0, &m_dwThreadID)))
            return false;
        return true;
    }
    // Stops the feval thread, destroys window
    virtual bool stop()
    {
        SetEvent(m_hExit);
        WaitForSingleObject(m_hThread, INFINITE);
        CloseHandle(m_hThread);
        map<DWORD, HWND>::iterator it = m_ThreadData.begin();
        while(it != m_ThreadData.end())
        {
            HWND hWnd = (*it).second;
            if (hWnd)
                DestroyWindow(hWnd);
            it++;
        }
        UnregisterClass("feval_window_class", m_hInstance);
        m_hWnd = NULL;
        m_ThreadData.clear();
        CloseHandle(m_hExit);
        CloseHandle(m_hFevalPending);
        return true;
    }
private:
    // Returns the HWND associated with the current thread
    // A new HWND is created by this function the first time
    // it gets called on a given thread.
    HWND get_thread_window()
    {
        ModuleLock lock;
        DWORD dwThreadID = GetCurrentThreadId();
        HWND hWnd = NULL;
        map<DWORD, HWND>::iterator it = m_ThreadData.find(dwThreadID);
        if (it == m_ThreadData.end())
        {
            char tmp[128];
            char window_name[128];
            sprintf(tmp, "%u", dwThreadID);
            strcpy(window_name, "feval_");
            strcat(window_name, tmp);
            strcat(window_name, "_window");
            hWnd = CreateWindowEx(0, "feval_window_class", window_name, 0,
                                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                HWND_MESSAGE, (HMENU)NULL, m_hInstance, NULL);
            if (!hWnd)
            {
                return NULL;
            }
            m_ThreadData[dwThreadID] = hWnd;
        }
        else
            hWnd = (*it).second;
        return hWnd;
    }
    // Adds an feval call
    void add(FevalArgs* pArgs)
    {
        ModuleLock lock;
        m_fevalQueue.push(pArgs);
        SetEvent(m_hFevalPending);
    }
    // Gets the next available feval call
    FevalArgs* get_next()
    {
        ModuleLock lock;
        if (m_fevalQueue.size() == 0)
            return NULL;
        FevalArgs* pArgs = m_fevalQueue.front();
        m_fevalQueue.pop();
        return pArgs;
    }
    // Feval thread function. Executes fevals as they become available,
    // Exits when signaled to.
    static DWORD WINAPI FevalProc(LPVOID pThis)
    {
        mclFevalWithEvents* pf = static_cast<mclFevalWithEvents*>(pThis);
        HANDLE hEvents[2] = {pf->m_hFevalPending, pf->m_hExit};
        DWORD dwWait = 0;

        for (;;)
        {
            dwWait = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
            if (dwWait == WAIT_OBJECT_0)
            {
                FevalArgs* pArgs = NULL;
                while ((pArgs = pf->get_next()))
                    pArgs->execute();
            }
            else if (dwWait == WAIT_OBJECT_0+1)
                break;
            else
                return 1;
        }
        return 0;
    }
    // Window process for HWNDs created by this class.
    static LRESULT CALLBACK ModuleWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_EVENTPENDING:
            {
                EventArgs* pArgs = (EventArgs*)lParam;
                if (pArgs)
                    pArgs->execute();
                break;
            }
        default: 
            return DefWindowProc(hwnd, uMsg, wParam, lParam); 
        } 
        return 0; 
    }
    // This function clears all WM_PAINT messages from the window
    static BOOL CALLBACK EnumThreadWndProc(HWND hWnd, LPARAM lParam)
    {
        BOOL bRet = FALSE;
        MSG msg = {0, 0, 0, 0, 0, {0,0}};
        while((bRet = PeekMessage(&msg, hWnd, WM_PAINT, WM_PAINT, PM_REMOVE)))
        {
            if (bRet == -1)
            {
                return FALSE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        return TRUE;
    }
private:
    HINSTANCE m_hInstance;					// HINSTANCE passed from DLLMain
    HANDLE m_hThread;
    DWORD m_dwThreadID;
    HANDLE m_hExit;
    HANDLE m_hFevalPending;
    queue<FevalArgs*> m_fevalQueue;
    HWND m_hWnd;
    map<DWORD, HWND> m_ThreadData;
	const char* m_errorMessage;
};
/*------------------------------------------------------------------------------
  CMCLModule class definition. The CMCLModule class is used as the global
  DLL controller object for all COM DLL's. Manages global lock count, 
  registration/de-registration, and class factory services for the DLL.
------------------------------------------------------------------------------*/
class CMCLModule
{
/*--------------------------------------
  Construction/destruction
--------------------------------------*/
public:
	CMCLModule(bool with_events = false)
    {
        InitializeData(NULL, with_events);
        m_pEvents = new mclEventMap();
    }
    CMCLModule(IMCLEventMap* pEvents, bool with_events = false)
    {
        InitializeData(NULL, with_events);
        m_pEvents = pEvents;
    }
    CMCLModule(MCLInitializeInstancePtr pInit, MCLTerminateInstancePtr pTerm, bool with_events = false)
    {
        InitializeData(pTerm, with_events);
        m_pInitialize = pInit;
        m_pEvents = new mclEventMap();
    }
    CMCLModule(MCLInitializeInstancePtr pInit, MCLTerminateInstancePtr pTerm, IMCLEventMap* pEvents, bool with_events = false)
    {
        InitializeData(pTerm, with_events);	
        m_pInitialize = pInit;
        m_pEvents = pEvents;
    }

    CMCLModule(MCLInitializeInstanceExPtr pInit, MCLTerminateInstancePtr pTerm, bool with_events = false)
    {
        InitializeData(pTerm, with_events);
        m_pInitializeEx = pInit;
        m_pEvents = new mclEventMap();
    }
    CMCLModule(MCLInitializeInstanceExPtr pInit, MCLTerminateInstancePtr pTerm, IMCLEventMap* pEvents, bool with_events = false)
    {
        InitializeData(pTerm, with_events);	
        m_pInitializeEx = pInit;
        m_pEvents = pEvents;
    }

	virtual ~CMCLModule()
    {
        if (m_pFeval)
            delete m_pFeval;
        if (m_pEvents)
            delete m_pEvents;
    }
/*--------------------------------------
  Methods
--------------------------------------*/
public:
	// Initialization/uninitialization method to be called from DLLMain
	virtual BOOL InitMain(MCLOBJECT_MAP_ENTRY pobjectmap, const GUID* plibid, WORD wMajor, WORD wMinor,
						  HINSTANCE hInstance, DWORD dwReason, void* pv)
    {
        HRESULT hr = S_OK;
        if (dwReason == DLL_PROCESS_ATTACH)
        {
            char szDllPath[_MAX_PATH];
            char szDir[_MAX_DIR];
            if (!Init(pobjectmap, hInstance, plibid, wMajor, wMinor))
                return FALSE;
            DisableThreadLibraryCalls(hInstance);
            if (GetModuleFileName(hInstance, szDllPath, _MAX_PATH) > 0)
            {
                _splitpath(szDllPath, m_szPath, szDir, NULL, NULL);
                strcat(m_szPath, szDir);
            }
        }
        else if (dwReason == DLL_PROCESS_DETACH)
	    {
		    Term();
	    }
        return TRUE;
    }
	// Initializes the class with object, instance, and type lib info
	virtual bool Init(MCLOBJECT_MAP_ENTRY pobjectmap, HINSTANCE hInstance, const GUID* plibid, WORD wMajor, WORD wMinor)
    {
        if (isInitialized())
		    return true;
	    m_hInstance = hInstance;
	    m_plibid = plibid;
	    m_wMajor = wMajor;
	    m_wMinor = wMinor;
	    m_pObjectMap = pobjectmap;
        if (!m_pFeval->init(hInstance))
            return false;
        m_bInitialized = TRUE;
        return true;
    }
	// Uninitializes the class
	virtual void Term()
    {
        if (!isInitialized())
		    return;
	    m_hInstance = NULL;
	    m_plibid = NULL;
	    m_wMajor = 0;
	    m_wMinor = 0;
	    m_bInitialized = FALSE;
    }
	// Returns the current lock count
	long GetLockCount()
    {
        long cCount = 0;
        InterlockedExchange(&cCount, m_cLockCount);
        return cCount;
    }
	// Updates the registry for all classes in list and type lib. TRUE = Register, FALSE = Unregister
	virtual HRESULT UpdateRegistry(BOOL bRegister)
    {
      HRESULT hr = S_OK;
      
      if (!isInitialized())
        return E_FAIL;
      if (bRegister)
      {
        char szDllPath[MAX_PATH];
        OLECHAR *wDllPath=NULL;
        ITypeLib* pTypeLib = 0;
        
        GetModuleFileName(m_hInstance, szDllPath, MAX_PATH);
        try
        {
        int	 wLength = (int)strlen (szDllPath) + 1; // +1 means don't forget the terminating null character
        wDllPath = new OLECHAR[wLength];  
        MultiByteToWideChar (
			CP_ACP,		    // code page 
			MB_PRECOMPOSED,	// character-type options 
			szDllPath,	    // address of string to map 
			wLength,        // number of characters in string 
			wDllPath,	    // address of wide-character buffer 
			wLength);	    // size of buffer 
        hr = LoadTypeLibEx(wDllPath, REGKIND_REGISTER, &pTypeLib);
        if(FAILED(hr)) {
            delete wDllPath; 
            wDllPath = NULL; 
            return hr;
        }
        pTypeLib->Release();
		    if (m_pObjectMap == NULL)
          return S_OK;
        int i = 0;
        while (m_pObjectMap[i].pclsid != NULL)
        {
          hr = m_pObjectMap[i].pfnRegisterClass(m_plibid, m_wMajor, m_wMinor, m_pObjectMap[i].lpszFriendlyName, 
                                                m_pObjectMap[i].lpszVerIndProgID, m_pObjectMap[i].lpszProgID, szDllPath);
			    if (FAILED(hr))
            return hr;
          i++;
        }
        
        delete wDllPath; 
        wDllPath = NULL; 
        }
        catch(...) {
            if(wDllPath != NULL) {
                delete wDllPath; 
                wDllPath = NULL;
            }
        }
      }
      else
      {
#ifdef _WIN64        
	    hr = UnRegisterTypeLib(*m_plibid, m_wMajor, m_wMinor, LANG_NEUTRAL, SYS_WIN64);
        //since we add a "win32" key under typelib at the time of registration, 
        //we need to remove them here.
        hr = UnRegisterTypeLib(*m_plibid, m_wMajor, m_wMinor, LANG_NEUTRAL, SYS_WIN32);
#else
        hr = UnRegisterTypeLib(*m_plibid, m_wMajor, m_wMinor, LANG_NEUTRAL, SYS_WIN32);
#endif
        int i = 0;
        while (m_pObjectMap[i].pclsid != NULL)
        {
          hr = m_pObjectMap[i].pfnUnregisterClass(m_pObjectMap[i].lpszVerIndProgID, m_pObjectMap[i].lpszProgID);
		  if (FAILED(hr))
            return hr;
          i++;
        }
      }
      return S_OK;
    }
        // Returns a class factory pointer for the specified CLSID
	virtual HRESULT GetClassObject(REFCLSID clsid, REFIID iid, void** ppv)
        {
            if (!mclmcrInitialize2(mclStandaloneContainer)) 
                return CLASS_E_CLASSNOTAVAILABLE;

            if (!isInitialized())
                return CLASS_E_CLASSNOTAVAILABLE;
            if (m_pObjectMap == NULL)
                return CLASS_E_CLASSNOTAVAILABLE;
            int i = 0;
            while (m_pObjectMap[i].pclsid != NULL)
            {
                if (*(m_pObjectMap[i].pclsid) == clsid)
                    return m_pObjectMap[i].pfnGetClassObject(clsid, iid, ppv);
                i++;
            }
            return CLASS_E_CLASSNOTAVAILABLE;
        }
	// Increments the lock count
	long Lock() {return InterlockedIncrement(&m_cLockCount);}
	// Decrements the lock count
	long Unlock() {return InterlockedDecrement(&m_cLockCount);}
	// Returns a new type info pointer for the module's type lib
	HRESULT GetTypeInfo(REFGUID riid, ITypeInfo** ppTypeInfo)
    {
        HRESULT hr = S_OK;
	    ITypeLib* pTypeLib = NULL;

	    if (!isInitialized())
		    return E_FAIL;
	    if(FAILED(hr = LoadRegTypeLib(*m_plibid, m_wMajor, m_wMinor, LANG_NEUTRAL, &pTypeLib)))
		    return hr;
	    hr = pTypeLib->GetTypeInfoOfGuid(riid, ppTypeInfo);
	    pTypeLib->Release();
	    return hr;
    }
	// Returns the progid for a given clsid, returns null if invalid clsid
	const char* GetProgID(REFCLSID clsid)
    {
        int i = 0;
        while (m_pObjectMap[i].pclsid != NULL)
	    {
		    if (*(m_pObjectMap[i].pclsid) == clsid)
			    return m_pObjectMap[i].lpszProgID;
		    i++;
	    }
	    return NULL;
    }
    // Initializes the MCR instance. Called by object constructors
    virtual bool InitializeComponentInstance(HMCRINSTANCE* inst)
    {
        if (!m_pInitialize || !inst)
            return false;
        return m_pInitialize(inst, getPath());
    }
    // Initializes the MCR instance with CTF embedded in resource file. 
    // Called by object constructors
    virtual bool InitializeComponentInstanceEx(HMCRINSTANCE* inst)
    {
        if (!m_pInitializeEx || !inst)
            return false;
        
        DWORD ctfSize = 0;
        char* ctfData = GetEmbeddedCtf(ctfSize);
        if(ctfData == NULL || ctfSize <= 0)
                return false;

        mclCtfStream ctfStream = mclGetStreamFromArraySrc(ctfData, ctfSize);
        bool bResult = m_pInitializeEx(inst, getPath(), ctfStream);
        
        if(ctfStream != NULL)
        {
            mclDestroyStream(ctfStream);
        }
        return bResult;
    }
    // Terminates an MCR instance. Called by object destructors.
    virtual bool TerminateInstance(HMCRINSTANCE* inst)
    {
        if (!m_pTerminate || !inst)
            return false;
        return m_pTerminate(inst);
    }
    virtual BOOL isInitialized()
    {
        return m_bInitialized;
    }
    virtual void setInitialized(BOOL bInit)
    {
        m_bInitialized = bInit;
    }
    // Returns the path to the Dll
    const char* getPath()
    {
        return m_szPath;
    }
    // Returns a reference to the Event map
    virtual IMCLEventMap* getEventMap()
    {
        return m_pEvents;
    }
    virtual bool Feval(HMCRINSTANCE hinst, const char* name, int nlhs, mxArray** plhs, int nrhs, mxArray** prhs)
    {
        return m_pFeval->Feval(hinst, name, nlhs, plhs, nrhs, prhs);
    }
	virtual const char* getErrorMessage()
	{
		return m_pFeval->getErrorMessage();
	}
	virtual void RaiseEvent(OLECHAR* lpwszName, long dispid, IDispatch* pDisp, DISPPARAMS* pDispParams)
    {
      m_pFeval->RaiseEvent(lpwszName, dispid, pDisp, pDispParams);
    }

    char* GetEmbeddedCtf(DWORD& ctfSize)
    {
        DWORD err = 0; 
        ctfSize = 0;

        HRSRC rsrc  = FindResource(m_hInstance, MAKEINTRESOURCE(2),"RT_RCDATA");
        if(!rsrc)
        {
            err = GetLastError();
            return NULL;
        }
       
        ctfSize = SizeofResource(m_hInstance, rsrc);
        HGLOBAL MemoryHandle = LoadResource(m_hInstance, rsrc);
        if(MemoryHandle == NULL)
        {
            err = GetLastError();
            return NULL;
        }
        char *ctfData = (char *) LockResource(MemoryHandle);
        if(ctfData == NULL)
        {
            err = GetLastError();
        }
        
        return ctfData;
    }

private:
    void InitializeData(MCLTerminateInstancePtr pTerm, bool with_events)
    {
        m_cLockCount = 0;
        m_hInstance = NULL;
        m_pObjectMap = NULL;
        m_plibid = NULL;
        m_wMajor = 0;
        m_wMinor = 0;
        m_bInitialized = FALSE;
        m_szPath[0] = '\0';
        m_pTerminate = pTerm;
        m_pFeval = (with_events ? static_cast<IMCLFeval*>(new mclFevalWithEvents()) 
                                : static_cast<IMCLFeval*>(new mclSimpleFeval()));
    }

/*--------------------------------------
  Properties
--------------------------------------*/
protected:
    MCLInitializeInstancePtr m_pInitialize; // Function used to create an MCR instance
    MCLInitializeInstanceExPtr m_pInitializeEx; // Function used to create an MCR instance with embedded ctf
    MCLTerminateInstancePtr m_pTerminate;   // Function to destroy an MCR instance
private:
	long m_cLockCount;						// Lock count on module
	HINSTANCE m_hInstance;					// HINSTANCE passed from DLLMain
	MCLOBJECT_MAP_ENTRY m_pObjectMap;		// Object info array
	const GUID* m_plibid;					// LIBID of type lib
	BOOL m_bInitialized;					// Is-initialized flag
	WORD m_wMajor;							// Major rev number of type lib
	WORD m_wMinor;							// Minor rev number of type lib
    char m_szPath[_MAX_PATH];               // Stores the location of the Dll
    IMCLEventMap* m_pEvents;                // event map
    IMCLFeval* m_pFeval;
};

/*------------------------------------------------------------------------------
  CMCLSingleModule class definition. The CMCLSingleModule class specializes
  the CMCLModule class for the case of a singleton MCR instance.
------------------------------------------------------------------------------*/
class CMCLSingleModule : public CMCLModule
{
/*--------------------------------------
  Construction/destruction
--------------------------------------*/
public:
    CMCLSingleModule(bool with_events = false) : CMCLModule(new mclSingleEventMap(), with_events), m_mcrInstance(NULL)
    {
    }
    CMCLSingleModule(MCLInitializeInstancePtr pInit, MCLTerminateInstancePtr pTerm, bool with_events = false)
        : CMCLModule(pInit, pTerm, new mclSingleEventMap(), with_events), m_mcrInstance(NULL)
    {
    }
    CMCLSingleModule(MCLInitializeInstanceExPtr pInit, MCLTerminateInstancePtr pTerm, bool with_events = false)
        : CMCLModule(pInit, pTerm, new mclSingleEventMap(), with_events), m_mcrInstance(NULL)
    {
    }
	virtual ~CMCLSingleModule()
    {
    }
/*--------------------------------------
  Methods
--------------------------------------*/
public:
	// Uninitializes the class
	void Term()
    {
        if (!isInitialized())
            return;
        CMCLModule::Term();
        ModuleLock lock;
        if (m_mcrInstance)
            m_mcrInstance = NULL;
    }
    // Initializes the MCR instance. Called by object constructors
    bool InitializeComponentInstance(HMCRINSTANCE* inst)
    {
        if (!inst)
            return false;
        bool ret = false;
        ModuleLock lock;
        if (!m_mcrInstance)
        {
            if (!m_pInitialize)
                return false;
            ret = m_pInitialize(&m_mcrInstance, getPath());
        }
        *inst = m_mcrInstance;
        return ret;
    }
    // Initializes the MCR instance with CTF embedded in resource file. 
    // Called by object constructors
    bool InitializeComponentInstanceEx(HMCRINSTANCE* inst)
    {
        if (!inst)
            return false;
        bool ret = false;
        ModuleLock lock;
        if (!m_mcrInstance)
        {
            if (!m_pInitializeEx)
                return false;
            
            DWORD ctfSize = 0;
            char* ctfData = GetEmbeddedCtf(ctfSize);
            if(ctfData == NULL || ctfSize <= 0)
                return false;

            mclCtfStream ctfStream = mclGetStreamFromArraySrc(ctfData, ctfSize);
            ret = m_pInitializeEx(&m_mcrInstance, getPath(), ctfStream);
            
            if(ctfStream != NULL)
            {
                mclDestroyStream(ctfStream);         
            }
        }
        *inst = m_mcrInstance;
        return ret;
    }
    // Terminates an MCR instance. Called by object destructors.
    bool TerminateInstance(HMCRINSTANCE* inst)
    {
        if (!inst)
            return false;
        *inst = NULL;
        return true;
    }
/*--------------------------------------
  Properties
--------------------------------------*/
private:
    HMCRINSTANCE m_mcrInstance;             // Module-level MCR instance
};

class mclmxarray_list {
	int count;
	mxArray **list;
public:
	mclmxarray_list( int icount, mxArray **ilist ) : count(icount), list(ilist) { }
	~mclmxarray_list( ) {
		for (int i =0; i<count; i++) {
			mxDestroyArray( list[i] );
		}
	}
};

// Globals
extern CMCLModule* g_pModule;

template<class T, const IID* piid, class T1, const CLSID* pclsid, const IID* piidEvents = NULL>
class CMCLBaseImpl: public T, public ISupportErrorInfo, public IConnectionPointContainer
{
/*--------------------------------------
  Construction/destruction
--------------------------------------*/
public:
	// CMCLBaseImpl constructor
	CMCLBaseImpl()
	{
		m_cRef = 1;
        m_pTypeInfo = NULL;
        m_pEvents = NULL;
		g_pModule->Lock();
	}
	// CMCLBaseImpl destructor
	virtual ~CMCLBaseImpl()
	{
        if (m_pEvents != NULL)
            m_pEvents->Release();
        if (m_pTypeInfo != NULL)
            m_pTypeInfo->Release();
        g_pModule->Unlock();
	}

/*--------------------------------------
  IUnknown implementation
--------------------------------------*/
public:
	// IUnknown::AddRef implementation
	ULONG __stdcall AddRef()
	{
		return InterlockedIncrement(&m_cRef);
	}
	// IUnknown::Release implementation
	ULONG __stdcall Release()
	{
        ULONG cRef = InterlockedDecrement(&m_cRef);
		if(cRef != 0)
			return cRef;
		delete this;
		return 0;
	}
	// IUnknown::QueryInterface implementation
	HRESULT __stdcall QueryInterface(REFIID riid, void** ppv)
	{
		if(riid == *piid)
			*ppv = static_cast<T*>(this);
		else if(riid == IID_IUnknown)
			*ppv = reinterpret_cast<IUnknown*>(this);
		else if(riid == IID_IDispatch)
			*ppv = reinterpret_cast<IDispatch*>(this);
		else if(riid == IID_ISupportErrorInfo)
			*ppv = static_cast<ISupportErrorInfo*>(this);
        else if (riid == IID_IConnectionPointContainer)
            *ppv = static_cast<IConnectionPointContainer*>(this);
		else 
		{
			*ppv = NULL;
			return E_NOINTERFACE;
		}
		AddRef();
		return S_OK;
	}
/*--------------------------------------
  IDispatch implementation
--------------------------------------*/
	// IDispatch::GetTypeInfoCount implementation
	HRESULT __stdcall GetTypeInfoCount(UINT* pCountTypeInfo)
	{
        if (pCountTypeInfo != NULL)
		    *pCountTypeInfo = 1;
		return S_OK;
	}
	// IDispatch::GetTypeInfo implementation
	HRESULT __stdcall GetTypeInfo(UINT iTypeInfo, LCID lcid, ITypeInfo** ppITypeInfo)
	{
        if (ppITypeInfo != NULL)
        {
		    *ppITypeInfo = NULL;
		    if(iTypeInfo != 0)
			    return DISP_E_BADINDEX;
            if (m_pTypeInfo != NULL)
		        m_pTypeInfo->AddRef();
		    *ppITypeInfo = m_pTypeInfo;
        }
		return S_OK;
	}
	// IDispatch::GetIDsOfNames implementation
	HRESULT __stdcall GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, 
									LCID lcid, DISPID* rgDispId)
	{
		if(riid != IID_NULL)
			return DISP_E_UNKNOWNINTERFACE;
		return DispGetIDsOfNames(m_pTypeInfo, rgszNames, cNames, rgDispId);
	}
	// IDispatch::Invoke implementation
	HRESULT __stdcall Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, 
							 DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, 
							 UINT* puArgErr)
	{
		if(riid != IID_NULL)
			return DISP_E_UNKNOWNINTERFACE;
		return DispInvoke(this, m_pTypeInfo, dispIdMember, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr); 
	}
/*--------------------------------------
  ISupportErrorInfo implementation
--------------------------------------*/
    // ISupportErrorInfo::InterfaceSupportsErrorInfo implementation
	HRESULT __stdcall InterfaceSupportsErrorInfo(REFIID riid)
	{
		if(riid == *piid)
			return S_OK;
		else
			return S_FALSE;
	}
/*--------------------------------------
  IConnectionPointContainer implementation
--------------------------------------*/
    // IConnectionPointContainer::EnumConnectionPoints implementation
    HRESULT __stdcall EnumConnectionPoints(IEnumConnectionPoints** ppEnum)
    {
        HRESULT hr = S_OK;
        ULONG cConnections = 0;
        CMCLEnumConnectionPoints* pEnum = NULL;

        if (ppEnum == NULL)
            return E_POINTER;
        if (m_pEvents != NULL)
            cConnections = 1;
        pEnum = new CMCLEnumConnectionPoints(reinterpret_cast<IUnknown*>(this), cConnections, &m_pEvents);
        if (pEnum == NULL)
            return E_OUTOFMEMORY;
        hr = pEnum->QueryInterface(IID_IEnumConnectionPoints, (void**)ppEnum);
        pEnum->Release();
        return hr;
    }
    // IConnectionPointContainer::FindConnectionPoint implementation
    HRESULT __stdcall FindConnectionPoint(REFIID riid, IConnectionPoint** ppCP)
    {
        if (ppCP == NULL)
            return E_POINTER;
        if (piidEvents == NULL)
            return CONNECT_E_NOCONNECTION;
        if (riid == *piidEvents)
        {
            if (m_pEvents == NULL)
                return CONNECT_E_NOCONNECTION;
            *ppCP = m_pEvents;
            (*ppCP)->AddRef();
            return S_OK;
        }
        return CONNECT_E_NOCONNECTION;
    }
/*--------------------------------------
  CMCLBaseImpl Methods
--------------------------------------*/
	// Initializes class, loads type info stuff and initializes connection point if necessary.
    // Put any additional init stuff in here.
	virtual bool Init(void)
	{
        HRESULT hr = S_OK;

        hr = g_pModule->GetTypeInfo(*piid, &m_pTypeInfo);
		if(FAILED(hr))
            return false;
        if (piidEvents != NULL)
        {
            CMCLConnectionPointImpl<piidEvents>* pEvents 
                = new CMCLConnectionPointImpl<piidEvents>(static_cast<IConnectionPointContainer*>(this));
            if (pEvents == NULL)
                return false;
            if (pEvents != NULL)
            {
                if (FAILED(pEvents->QueryInterface(IID_IConnectionPoint, (void**)&m_pEvents)))
                    return false;
                pEvents->Release();
            }
        }
        return true;
	}
    // Registers the class
	static HRESULT __stdcall RegisterClass(const GUID* plibid, unsigned short wMajor, unsigned short wMinor, const char* lpszFriendlyName, 
                                           const char* lpszVerIndProgID, const char* lpszProgID, const char* lpszModuleName)
	{
		return mclRegisterServer(lpszModuleName, *pclsid, *plibid, wMajor, wMinor, lpszFriendlyName,
                                 lpszVerIndProgID, lpszProgID, "Both");
	}
	// Unregisters the class
	static HRESULT __stdcall UnregisterClass(const char* lpszVerIndProgID, const char* lpszProgID)
	{
		return mclUnregisterServer(*pclsid, lpszVerIndProgID, lpszProgID);
	}
	// Called by COM framework to get IClassFactory pointer on which to create new instances of object
	static HRESULT __stdcall GetClassObject(REFCLSID clsid, REFIID iid, void** ppv)
	{	
		if(clsid != *pclsid)
			return CLASS_E_CLASSNOTAVAILABLE;

		CMCLFactoryImpl<T1>* pFactory = new CMCLFactoryImpl<T1>;
		if(pFactory == NULL)
			return E_OUTOFMEMORY;

		// QueryInterface for IClassFactory
		HRESULT hr = pFactory->QueryInterface(iid, ppv);
		pFactory->Release();
		return hr;
	}
	// Method to report an error
	static HRESULT __stdcall Error(const char* lpszMessage)
	{
		ICreateErrorInfo* pCreateErrorInfo = NULL;
		IErrorInfo* pErrorInfo = NULL;
		HRESULT hr = S_OK;
		OLECHAR* lpwszMessage = NULL;
		OLECHAR* lpwszSource = NULL;
		const char* lpszSource = NULL;
		int nLen = 0;

		if (FAILED(hr = CreateErrorInfo(&pCreateErrorInfo)))
			goto EXIT;
		// Set message text
		if (lpszMessage != NULL)
		{
                  pwcsStackPointer slpwszMessage = NULL;
                  initializeWcsStackPointer(&slpwszMessage);
                  if(mwMbstowcs(slpwszMessage, lpszMessage) < 0) {
                    deleteWcsStackPointer(slpwszMessage);
                    pCreateErrorInfo->SetDescription(L"Error converting multibyte string to wide character string.");
                  } else {
                    lpwszMessage = _wcsdup(reinterpret_cast<wchar_t*>(slpwszMessage->hPtr));
                    deleteWcsStackPointer(slpwszMessage);
                    pCreateErrorInfo->SetDescription(lpwszMessage);
                  }
                  
		} 
		else
                  pCreateErrorInfo->SetDescription(L"");
		// Set IID
		pCreateErrorInfo->SetGUID(*piid);
		// Set error source
		lpszSource = g_pModule->GetProgID(*pclsid);
		if (lpszSource != NULL)
		{
                  pwcsStackPointer slpwszSource = NULL;
                  initializeWcsStackPointer(&slpwszSource);
                  if ( mwMbstowcs(slpwszSource, lpszSource) < 0){
                    deleteWcsStackPointer( slpwszSource );
                    pCreateErrorInfo->SetDescription(L"Error converting multibyte string to wide character string");
                  } else {
                    lpwszSource=_wcsdup(reinterpret_cast<wchar_t*>(slpwszSource->hPtr));
                    deleteWcsStackPointer( slpwszSource );
                    pCreateErrorInfo->SetSource(lpwszSource);
                  }

		}
		else
                  pCreateErrorInfo->SetSource(L"");
		// Set error info
		if (FAILED(hr = pCreateErrorInfo->QueryInterface(IID_IErrorInfo, (void**)&pErrorInfo)))
			goto EXIT;
		hr = SetErrorInfo(0, pErrorInfo);
	EXIT:
		if (lpwszMessage != NULL)
			delete lpwszMessage;
		if (lpwszSource != NULL)
			delete lpwszSource;
		if (pErrorInfo != NULL)
			pErrorInfo->Release();
		if (pCreateErrorInfo != NULL)
			pCreateErrorInfo->Release();
		return hr;
	}
    // Method to report an error from an EXCEPINFO structure (note: does not use template *piid in SetGUID)
    static HRESULT __stdcall Error(REFIID riid, EXCEPINFO* pExcepInfo)
    {
	    ICreateErrorInfo* pCreateErrorInfo = NULL;
	    IErrorInfo* pErrorInfo = NULL;
	    HRESULT hr = S_OK;

        if (pExcepInfo == NULL)
            goto EXIT;
	    if (FAILED(hr = CreateErrorInfo(&pCreateErrorInfo)))
		    goto EXIT;
	    // Set message text
	    pCreateErrorInfo->SetDescription(pExcepInfo->bstrDescription);
	    // Set IID
	    pCreateErrorInfo->SetGUID(riid);
	    // Set error source
	    pCreateErrorInfo->SetSource(pExcepInfo->bstrSource);
	    // Set error info
	    if (FAILED(hr = pCreateErrorInfo->QueryInterface(IID_IErrorInfo, (void**)&pErrorInfo)))
		    goto EXIT;
	    hr = SetErrorInfo(0, pErrorInfo);
    EXIT:
	    if (pErrorInfo != NULL)
		    pErrorInfo->Release();
	    if (pCreateErrorInfo != NULL)
		    pCreateErrorInfo->Release();
    	return hr;
    }
protected:
    int RequestLocalLock()
    {
        mclAcquireMutex();
        return 0;
    }
    int ReleaseLocalLock()
    {
        mclReleaseMutex();
        return 0;
    }
/*--------------------------------------
  CMCLBaseImpl Properties
--------------------------------------*/
private:
	long m_cRef;				// Reference count
	ITypeInfo* m_pTypeInfo;		// Type Info pointer
protected:
    IConnectionPoint* m_pEvents;// Connection point for objects that implement an event interface
};

template<class T, const IID* piid, class T1, const CLSID* pclsid, const IID* piidEvents = NULL>
class CMCLSingleBaseImpl: public CMCLBaseImpl<T, piid, T1, pclsid, piidEvents>
{
/*--------------------------------------
  Construction/destruction
--------------------------------------*/
public:
	// CMCLSingleBaseImpl constructor
	CMCLSingleBaseImpl(){}
	// CMCLSingleBaseImpl destructor
	virtual ~CMCLSingleBaseImpl(){}
/*--------------------------------------
  IUnknown implementation
--------------------------------------*/
public:
	// IUnknown::AddRef implementation
	ULONG __stdcall AddRef()
	{
        return 2;
	}
	// IUnknown::Release implementation
	ULONG __stdcall Release()
	{
        return 1;
	}
	// Called by COM framework to get IClassFactory pointer on which to create new instances of object
	static HRESULT __stdcall GetClassObject(REFCLSID clsid, REFIID iid, void** ppv)
	{	
		if(clsid != *pclsid)
			return CLASS_E_CLASSNOTAVAILABLE;

		CMCLSingleFactoryImpl<T1>* pFactory = new CMCLSingleFactoryImpl<T1>;
		if(pFactory == NULL)
			return E_OUTOFMEMORY;

		// QueryInterface for IClassFactory
		HRESULT hr = pFactory->QueryInterface(iid, ppv);
		pFactory->Release();
		return hr;
	}
};

template<class T, const IID* piid, class T1, const CLSID* pclsid, const IID* piidEvents = (const IID *)NULL>
class CMCLClassImpl: public CMCLBaseImpl<T, piid, T1, pclsid, piidEvents>, public IMCLEvent
{
/*--------------------------------------
  Construction/destruction
--------------------------------------*/
public:
	// CMCLClassImpl constructor
	CMCLClassImpl()
    {
            m_pFlags = NULL;
            m_hinst = NULL;
	}
	// CMCLClassImpl destructor
	virtual ~CMCLClassImpl()
	{
            if (m_pFlags != NULL)
                m_pFlags->Release();
	}
protected:
    void RegisterListener()
    {
        g_pModule->getEventMap()->add(m_hinst, static_cast<IMCLEvent*>(this));
    }
    void UnregisterListener()
    {
        g_pModule->getEventMap()->remove(m_hinst, static_cast<IMCLEvent*>(this));
    }
public:
/*--------------------------------------
  CMCLClassImpl Methods
--------------------------------------*/
    // Registers the class and adds it to the MatLab XL component catagory
	static HRESULT __stdcall RegisterClass(const GUID* plibid, unsigned short wMajor, unsigned short wMinor, const char* lpszFriendlyName,
                                           const char* lpszVerIndProgID, const char* lpszProgID, const char* lpszModuleName)
	{
		return mclRegisterMatLabComponent(lpszModuleName, pclsid, plibid, wMajor, wMinor, lpszFriendlyName, lpszVerIndProgID, lpszProgID);
	}
	// Unregisters the class and removes it from the MatLab XL component catagory
	static HRESULT __stdcall UnregisterClass(const char* lpszVerIndProgID, const char* lpszProgID)
	{
		return mclUnRegisterMatLabComponent(pclsid, lpszVerIndProgID, lpszProgID);
	}
    // Returns a pointer to the contained MWFlags object
    HRESULT __stdcall get_MWFlags(IMWFlags** ppFlags)
    {
        HRESULT hr = S_OK;       // Return code
        ModuleLock lock;
        if (ppFlags == NULL)
            return E_INVALIDARG;
        *ppFlags = NULL;
        // If there is not one already allocated, creat a new one.
        if (m_pFlags == NULL)
        {
            hr = CoCreateInstance(CLSID_MWFlags, NULL, CLSCTX_INPROC_SERVER, 
                                  IID_IMWFlags, (void**)&m_pFlags);
            if (FAILED(hr))
            {
                return hr;
            }
        }
        *ppFlags = m_pFlags;
        m_pFlags->AddRef();
        return hr;
    }
    // Sets the array-formatting-object
    HRESULT __stdcall put_MWFlags(IMWFlags* pFlags)
    {
        HRESULT hr = S_OK;      // Return code
        ModuleLock lock;
        if (pFlags == NULL)
            return E_INVALIDARG;
        // Set internal object to new one. 
        if (m_pFlags != NULL)
            m_pFlags->Release();
        m_pFlags = pFlags;
        m_pFlags->AddRef();
        return hr;
    }
    // Call the MatLab mex funtion pointed to by mlxF
    HRESULT CallComFcn(const char* name, int nargout, int fnout, int fnin, ...)
    {
        mxArray **plhs;
        //int nargout:  comes in as ip param to this function
        int alloc_nargout = (nargout == 0) ? 1: nargout;
        mxArray **prhs;
        int nargin = 0;

        VARIANT **plvar;
        const VARIANT **prvar;

        bool bVarargout = fnout < 0;
        bool bVarargin = fnin < 0;

        mxArray *varargout = NULL;
        mxArray *varargin = NULL;

        int i;
        int new_nargin;
        va_list ap;
        bool bFoundDefault = false;
        HRESULT retval = S_OK;
        _MCLCONVERSION_FLAGS flags;
        IMWFlags* pFlags = NULL;
        bool bRet = false;
        int dc_status = 0;

        // Check MCR instance
        if (!m_hinst)
        {
            Error("MCR instance is not available");
            return E_FAIL;
        }
        // Get Conversion flags
        if (FAILED(get_MWFlags(&pFlags)))
        {
            Error("Error getting data conversion flags");
            return E_FAIL;
        }
        if (FAILED(GetConversionFlags(pFlags, &flags)))
        {
            Error("Error getting data conversion flags");
            return E_FAIL;
        }
        pFlags->Release();
        try 
        {    
            // O/P from mclMlxFeval (lhs)
            va_start( ap, fnin );
            if (bVarargout) 
            {
                fnout = -fnout;
            }
            plvar = (VARIANT **) _alloca( fnout * sizeof( VARIANT * ) );
            for (i=0; i< fnout; i++) 
            {
                plvar[i] = va_arg( ap, VARIANT *);
            }
            plhs = (mxArray **) _alloca( alloc_nargout * sizeof( mxArray * ));
            for (i=0; i< alloc_nargout; i++) 
            {
                plhs[i] = NULL;
            }
            // I/P to mclMlxFeval (rhs)
            if (bVarargin) 
            {
                fnin = -fnin;      
            }
            prvar = (const VARIANT **) _alloca( fnin * sizeof(VARIANT *));
            for (i = 0; i< fnin; i++) 
            {
                prvar[i] = va_arg( ap, VARIANT *);
                if (IsVisualBasicDefault( prvar[i] )) 
                {
                    bFoundDefault = true;
                }
                else 
                {
                    if (bFoundDefault) 
                    {
                        Error( "Error: Arguments may only be defaulted at the end of an argument list" );
                        retval = E_FAIL;
                        goto finish;
                    }
                    nargin++;
				}
            }
            if (bVarargin && !bFoundDefault) 
            {
                //g830130 - if there is only one varargin, and MCLUtil.MWPack() is not called, 
                //the varargin won't be passed in as an array. In this case, the conversion flag's 
                //nInputInd shoulnd't be incremented.
                if (prvar[fnin-1]->vt == (VT_VARIANT|VT_BYREF))
                {
                    if ((prvar[fnin-1]->pvarVal != NULL) && (prvar[fnin-1]->pvarVal->vt & VT_ARRAY))
                    {
                        flags.nInputInd += 1;
                    }
                }
                if ((dc_status = Variant2mxArray( prvar[fnin-1], &varargin, &flags)) < 0)
                {
                    mclSetLastErrIdAndMsg("", GetCOMErrorMessage(dc_status));
                    retval = E_FAIL;
                    goto finish;
                }
                flags.nInputInd -= 1;
                if (varargin == NULL)
                    nargin -= 1;
                /*Empty varargin should be passed to MATLAB as a empty cell array*/
                else if(mxIsEmpty(varargin))
                {
                    varargin = NULL;
                    nargin -= 1;
                }
                else if (mxGetClassID(varargin) == mxCELL_CLASS)
                    nargin += static_cast<int>(mxGetN( varargin )) - 1;
            }
            prhs = (mxArray **) _alloca( nargin * sizeof(mxArray *));
            for (i = 0; i< nargin; i++) 
                prhs[i] = NULL;
            mclmxarray_list protect_plhs( nargout, plhs );
            mclmxarray_list protect_prhs( nargin, prhs );
            for (i = 0; i< nargin; i++) 
            {
                if (i < fnin-1 || (!bVarargin && i == fnin-1))
                {
		            if ((dc_status = Variant2mxArray( prvar[i], &prhs[i], &flags)) < 0)
                    {
                        mclSetLastErrIdAndMsg("", GetCOMErrorMessage(dc_status));
                        retval = E_FAIL;
                        goto finish;
                    }
                }
                else
                {
                    if ( varargin != NULL && mxGetClassID(varargin) == mxCELL_CLASS )
                        prhs[i] = mclCreateSharedCopy(((mxArray **)mxGetData(varargin))[i-fnin+1]);
                    else
                        prhs[i] = mclCreateSharedCopy(varargin);
                }
            }
            bFoundDefault = false;
            new_nargin = nargin;
            for (i = 0; i< nargin; i++) 
            {
                if (!bFoundDefault) {
                    if (prhs[i] == NULL) {
                        bFoundDefault = true;
                        new_nargin = i;
                     }
                }
                else {
                    if (prhs[i] != NULL) {
                        Error( "Error: Arguments may only be defaulted at the end of an argument list" );
                        retval = E_FAIL;
                    }
                }
            }
            if (retval != E_FAIL) {
                nargin = new_nargin;
                // call mclMlxFeval
                bRet = g_pModule->Feval(m_hinst, name, nargout, plhs, nargin, prhs);
                if (!bRet)
                    goto finish;
                // translate o/p (lhs) of  mclMlxFeval back to variant
                if (nargout == 0 && plhs[0] != NULL )
                {
                    mxDestroyArray( plhs[0] );
                    plhs[0] = NULL;
                }
                if (bVarargout && nargout >= fnout) 
                {
                    varargout = mclCreateCellArrayFromArrayList(nargout-fnout+1, &plhs[fnout-1]);
                }
                for (i = 0; i<fnout && i<nargout; i++) 
                {     
                    if (i < fnout -1 || (!bVarargout && i == fnout-1 ))
                    {
                        if ((dc_status = mxArray2Variant( plhs[i], plvar[i], &flags)) < 0)
                        {
                            mclSetLastErrIdAndMsg("", GetCOMErrorMessage(dc_status));
                            retval = E_FAIL;
                            goto finish;
                        }
                    }
                    else
                    {
                        flags.nOutputInd += 1;
                        flags.nTransposeInd += 1;
                        if ((dc_status = mxArray2Variant( varargout, plvar[i], &flags)) < 0)
                        {
                            mclSetLastErrIdAndMsg("", GetCOMErrorMessage(dc_status));
                            retval = E_FAIL;
                            goto finish;
                        }
                        flags.nOutputInd -= 1;
                        flags.nTransposeInd -= 1;
                    }
                }
            }
        }
        catch (...)
        {
            Error("Unexpected Error Thrown");
            retval = E_FAIL;       
        }
    finish:
        if (varargin != NULL)
            mxDestroyArray( varargin );
        if (varargout != NULL)
            mxDestroyArray( varargout );
        if (!bRet)
        {
            const char* msg = g_pModule->getErrorMessage();
			
			//If there is no error message on the CMCLModule (which might 
			//  have run its work on a different thread), then check the 
			//  local thread for an error message.
			if(!msg)
			{
				msg = mclGetLastErrorMessage();
			}
			//If the g_pModule has an error (the thread that actually called feval), 
			//  use that, otherwise see if this thread has an error, 
			//  if all else fails, give a generic message.  
			Error((msg ? msg : "Unspecified error"));
            retval = E_FAIL;
        }
        return(retval); 
    }
    void mclRaiseEventA(const char* lpszName, DISPID dispid, int nargin, mxArray** prhs)
    {
        HRESULT hr = S_OK;              // Return code
        IEnumConnections* pEnum = NULL; // Pointer to enum of all connections
        CONNECTDATA ConnData = {0,0};   // CONNECTDATA structure for each connection
        ULONG cFetched = 0;             // Number of connections fetched in a call
        IDispatch* pDisp = NULL;        // IDispatch pointer to make call on
        VARIANT varResult;              // Result from call
        VARIANT* pvars = NULL;          // Varaibles to send into call
        DISPPARAMS disp = {NULL,NULL,0,0};// Disparams used to make call
	//        DISPID dispid = 0;              // DISPID of method to invoke
        OLECHAR* lpwszName = NULL;      // Temp buffer for method name
        int nLen = 0;                   // Length of name string
        int i = 0;                      // Loop counter
        _MCLCONVERSION_FLAGS flags;     // Temp flags struct
        IMWFlags* pFlags = NULL;        // Temp flags object

        pwcsStackPointer slpwszName = NULL;
        initializeWcsStackPointer(&slpwszName);

        // If no connection point available or invalid name, return
        if (m_pEvents == NULL || lpszName == NULL)
            return;
        if (nargin < 0 || (prhs == NULL && nargin > 0))
            nargin = 0;
        // Get Conversion flags
        if (FAILED(get_MWFlags(&pFlags)))
            InitConversionFlags(&flags);
        else
        {
            if (FAILED(GetConversionFlags(pFlags, &flags)))
                InitConversionFlags(&flags);
            pFlags->Release();
        }
        // Enum all connections 
        hr = m_pEvents->EnumConnections(&pEnum);
        if (FAILED(hr))
            goto EXIT;
        VariantInit(&varResult);
        // Create Array of Variants for call
        if (nargin > 0)
        {
            pvars = new VARIANT[nargin];
            if (pvars == NULL)
                goto EXIT;
            for (i=0;i<nargin;i++)
                VariantInit(&pvars[i]);
	        disp.rgvarg = pvars;
	        disp.cArgs = nargin;
        }
        for (i=0;i<nargin;i++)
        {
            if (prhs[i] != NULL)
            {
                if (mxArray2Variant(prhs[i], &pvars[nargin-i-1], &flags) < 0)
                    goto EXIT;
            }
        }
        // Get name
        if (mwMbstowcs(slpwszName,lpszName) < 0) {
          deleteWcsStackPointer(slpwszName);
          goto EXIT;
        }
        lpwszName=_wcsdup(reinterpret_cast<wchar_t*>(slpwszName->hPtr));
        deleteWcsStackPointer(slpwszName);

        // Loop over all connections and call each one
        pEnum->Reset();
        for(;;)
        {
            hr = pEnum->Next(1, &ConnData, &cFetched);
            if (cFetched == 0)
                break;
            if (ConnData.pUnk != NULL)
            {
                hr = ConnData.pUnk->QueryInterface(IID_IDispatch, (void**)&pDisp);
                ConnData.pUnk->Release();
                if (SUCCEEDED(hr) && pDisp != NULL)
                {
		    g_pModule->RaiseEvent(lpwszName, dispid, pDisp, &disp);
                    pDisp->Release();
                    pDisp = NULL;
                }
            }
        }
    EXIT:
        if (pEnum != NULL)
            pEnum->Release();
        if (pDisp != NULL)
            pDisp->Release();
        if (lpwszName != NULL)
            delete lpwszName;
        if (pvars != NULL)
        {
            for (i=0;i<nargin;i++)
                VariantClear(&pvars[i]);
            delete [] pvars;
        }
    }
    // Gets a property from the array
    HRESULT GetProperty(const char* name, VARIANT* pvarValue)
    {
        _MCLCONVERSION_FLAGS flags;
        HRESULT hr = S_OK;
        IMWFlags* pFlags = NULL;
        mxArray* px = NULL;
        int dc_status = 0;
        const char* msg = NULL;

        mwLock lock;
        // Check MCR instance
        if (!m_hinst)
        {
            Error("MCR instance is not available");
            return E_FAIL;
        }
        if (pvarValue == NULL)
            return E_INVALIDARG;
        // Get Conversion flags
        if (FAILED(get_MWFlags(&pFlags)))
        {
            Error("Error getting data conversion flags");
            return E_FAIL;
        }
        if (FAILED(GetConversionFlags(pFlags, &flags)))
        {
            Error("Error getting data conversion flags");
            return E_FAIL;
        }
        pFlags->Release();
        try
        {
            if (!mclGetGlobal(m_hinst, name, &px))
            {
                const char* msg = mclGetLastErrorMessage();
                Error((msg ? msg : "Unspecified error"));
                hr = E_FAIL;
            }
            else
            {
                if (pvarValue->vt != VT_EMPTY)
                    VariantClear(pvarValue);
                if ((dc_status = mxArray2Variant(px, pvarValue, &flags)) < 0)
                {
                    msg = GetCOMErrorMessage(dc_status);
                    Error((msg ? msg : "Unspecified error"));
                    hr = E_FAIL;
                }
            }
        }
        catch (...)
        {
            Error("Unexpected Error Thrown");
            hr = E_FAIL;       
        }
        if (px)
            mxDestroyArray(px);
        return hr;
    }
    // Sets a property in the array
    HRESULT PutProperty(const char* name, const VARIANT* pvarValue)
    {
        _MCLCONVERSION_FLAGS flags;
        HRESULT hr = S_OK;
        IMWFlags* pFlags = NULL;
        mxArray* px = NULL;
        int dc_status = 0;
        const char* msg = NULL;

        mwLock lock;
        // Check MCR instance
        if (!m_hinst)
        {
            Error("MCR instance is not available");
            return E_FAIL;
        }
        if (pvarValue == NULL)
            return E_INVALIDARG;
        // Get Conversion flags
        if (FAILED(get_MWFlags(&pFlags)))
        {
            Error("Error getting data conversion flags");
            return E_FAIL;
        }
        if (FAILED(GetConversionFlags(pFlags, &flags)))
        {
            Error("Error getting data conversion flags");
            return E_FAIL;
        }
        pFlags->Release();
        try
        {
            if ((dc_status = Variant2mxArray(pvarValue, &px, &flags)) < 0)
            {
                msg = GetCOMErrorMessage(dc_status);
                Error((msg ? msg : "Unspecified error"));
                hr = E_FAIL;
            }
            else if (!mclSetGlobal(m_hinst, name, px))
            {
                msg = mclGetLastErrorMessage();
                Error((msg ? msg : "Unspecified error"));
                hr = E_FAIL;
            }
        }
        catch (...)
        {
            Error("Unexpected Error Thrown");
            hr = E_FAIL;       
        }
        if (px)
            mxDestroyArray(px);
        return hr;
    }
/*--------------------------------------
  CMCLClassImpl Properties
--------------------------------------*/
private:
    IMWFlags* m_pFlags;             // Pointer to a formatting/conversion-flags object
protected:
    HMCRINSTANCE m_hinst;           // MCR instance
};

template<class T, const IID* piid, class T1, const CLSID* pclsid, const IID* piidEvents = NULL>
class CMCLSingleImpl: public CMCLClassImpl<T, piid, T1, pclsid, piidEvents>
{
/*--------------------------------------
  Construction/destruction
--------------------------------------*/
public:
	// CMCLSingleImpl constructor
	CMCLSingleImpl(){}
	// CMCLSingleImpl destructor
	virtual ~CMCLSingleImpl(){}
/*--------------------------------------
  IUnknown implementation
--------------------------------------*/
public:
	// IUnknown::AddRef implementation
	ULONG __stdcall AddRef()
	{
        return 2;
	}
	// IUnknown::Release implementation
	ULONG __stdcall Release()
	{
        return 1;
	}
	// Called by COM framework to get IClassFactory pointer on which to create new instances of object
	static HRESULT __stdcall GetClassObject(REFCLSID clsid, REFIID iid, void** ppv)
	{	
		if(clsid != *pclsid)
			return CLASS_E_CLASSNOTAVAILABLE;

		CMCLSingleFactoryImpl<T1>* pFactory = new CMCLSingleFactoryImpl<T1>;
		if(pFactory == NULL)
			return E_OUTOFMEMORY;

		// QueryInterface for IClassFactory
		HRESULT hr = pFactory->QueryInterface(iid, ppv);
		pFactory->Release();
		return hr;
	}
};

/*------------------------------------------------------------------------------
  Standard Class factory implementation.
------------------------------------------------------------------------------*/
template<class T>
class CMCLFactoryImpl : public IClassFactory
{
public:
	// Construction/destruction
	CMCLFactoryImpl() : m_cRef(1) { }
	virtual ~CMCLFactoryImpl() { }

	// IClassFactory::AddRef implementation
	ULONG __stdcall AddRef()
	{
		return InterlockedIncrement(&m_cRef);
	}
	// IClassFactory::Release implementation
	ULONG __stdcall Release()
	{
        ULONG cRef = InterlockedDecrement(&m_cRef);
		if(cRef != 0)
			return cRef;
		delete this;
		return 0;
	}
	// IClassFactory::QueryInterface implementation
	HRESULT __stdcall QueryInterface(REFIID iid, void** ppv)
	{
		if((iid == IID_IUnknown) || (iid == IID_IClassFactory))
			*ppv = (IClassFactory*)this;
		else
		{
			*ppv = NULL;
			return E_NOINTERFACE;
		}
		AddRef();
		return S_OK;
	}
	// IClassFactory::CreateInstance implementation
	virtual HRESULT __stdcall CreateInstance(IUnknown *pUnknownOuter, REFIID iid, void** ppv)
	{
		if(pUnknownOuter != NULL)
			return CLASS_E_NOAGGREGATION;

#if defined (mclcommain_h) || defined (mclxlmain_published_api_h)
		if (!mclComCheckMWComUtil())
		{
			const char* mcrversion=NULL;
			mclGetMCRVersion(&mcrversion);
			std::string errmsg;
			errmsg = "MWComUtil ";
			errmsg.append(mcrversion);
			errmsg.append(" could not be found in the registry. Please refer to MATLAB Builder documentation for instructions on how to install and register MWComUtil.");
			T::Error(errmsg.c_str());
			return E_FAIL;
		}
#endif		

		T* p = new T;

		if(p == NULL)
			return E_OUTOFMEMORY;

		// Call the Init method to load the type information
		if (!p->Init())
        {
            delete p;
            return E_UNEXPECTED;
        }

		HRESULT hr = p->QueryInterface(iid, ppv);
		p->Release();
		return hr;
	}
	// IClassFactory::LockServer implementation
	HRESULT __stdcall LockServer(BOOL bLock)
	{
		if(bLock)
			g_pModule->Lock();
		else
			g_pModule->Unlock();
		return S_OK;
	}
private:
	long m_cRef;	// Ref count
};

/*------------------------------------------------------------------------------
  Singleton Class factory implementation.
------------------------------------------------------------------------------*/
template<class T>
class CMCLSingleFactoryImpl : public CMCLFactoryImpl<T>
{
public:
	// IClassFactory::CreateInstance implementation
	HRESULT __stdcall CreateInstance(IUnknown *pUnknownOuter, REFIID iid, void** ppv)
	{
        ModuleLock lock;
		if(pUnknownOuter != NULL)
			return CLASS_E_NOAGGREGATION;
        if (m_p == NULL)
        {
            m_p = new T;
            if(m_p == NULL)
            {
			    return E_OUTOFMEMORY;
            }
            if (!m_p->Init())
            {
                delete m_p;
                m_p = NULL;
                return E_UNEXPECTED;
            }
            m_p->AddRef();
        }
        HRESULT hr = m_p->QueryInterface(iid, ppv);
		m_p->Release();
		return hr;
	}
private:
	long m_cRef;	// Ref count
    static T* m_p;  // Static instance of T
};

class CMCLEnumConnectionPoints : public IEnumConnectionPoints
{
public:
/*--------------------------------------
  Construction/destruction
--------------------------------------*/
    // CMCLEnumConnectionPoints default constructor
    CMCLEnumConnectionPoints()
    {
        m_cRef = 1;
	    m_nIndex = 0;
        m_pThis = NULL;
        m_rgpcn = NULL;
        m_cConnections = 0;
    }
    // CMCLEnumConnectionPoints constructor from container reference and array of connection points
    CMCLEnumConnectionPoints(IUnknown* pThis, ULONG cConnections, IConnectionPoint** rgpcn)
    {
        m_cRef = 1;
	    m_nIndex = 0;
        m_pThis = pThis;
        m_rgpcn = NULL;
        m_cConnections = 0;
        if (cConnections > 0 && rgpcn != NULL)
        {
            m_cConnections = cConnections;
            m_rgpcn = new IConnectionPoint*[m_cConnections];
            if (m_rgpcn != NULL)
            {
	            for(ULONG i=0;i<cConnections;i++)
                {
                    m_rgpcn[i] = NULL;
                    if (rgpcn[i] != NULL)
		                rgpcn[i]->QueryInterface(IID_IConnectionPoint, (void**)&m_rgpcn[i]);
                }
            }
            else
                m_cConnections = 0;
        }
    }
    // CMCLEnumConnectionPoints destructor
	virtual ~CMCLEnumConnectionPoints()
    {
        if (m_rgpcn != NULL)
        {
            for (size_t i=0;i<m_cConnections;i++)
            {
                if (m_rgpcn[i] != NULL)
                    m_rgpcn[i]->Release();
            }
            delete [] m_rgpcn;
        }
    }
/*--------------------------------------
  IUnknown implementation
--------------------------------------*/
public:
	// IUnknown::AddRef implementation
	ULONG __stdcall AddRef()
    {
        if (m_pThis != NULL)
	        m_pThis->AddRef();
	    return InterlockedIncrement(&m_cRef);
    }
    // IUnknown::Release implementation
	ULONG __stdcall Release()
    {
        if (m_pThis != NULL)
	        m_pThis->Release();
	    ULONG cRef = InterlockedDecrement(&m_cRef);
		if(cRef != 0)
			return cRef;
        delete this;
        return 0;
    }
	// IUnknown::QueryInterface implementation
	HRESULT __stdcall QueryInterface(REFIID riid, void** ppv)
	{
		if(riid == IID_IEnumConnectionPoints)
			*ppv = static_cast<IEnumConnectionPoints*>(this);
		else if(riid == IID_IUnknown)
			*ppv = static_cast<IUnknown*>(this);
		else
		{
			*ppv = NULL;
			return E_NOINTERFACE;
		}
		AddRef();
		return S_OK;
	}
/*--------------------------------------
  IEnumConnectionPoints implementation
--------------------------------------*/
	// IEnumConnectionPoints::Next implementation
	HRESULT __stdcall Next(ULONG cConnections, IConnectionPoint** rgpcn, ULONG* pcFetched)
    {
        LONG nIndex = 0;
        HRESULT hr = S_OK;

	    if(rgpcn == NULL || pcFetched == NULL)
		    return E_INVALIDARG;
	    *pcFetched = 0;
	    if (cConnections == 0)
            return S_OK;
	    for (ULONG i=0;i<cConnections;i++)
	    {
            InterlockedExchange(&nIndex, m_nIndex);
            if (nIndex >= (LONG)m_cConnections)
            {
                hr = S_FALSE;
                break;
            }
		    rgpcn[i] = m_rgpcn[nIndex];
		    if(rgpcn[i] != NULL)
			    rgpcn[i]->AddRef();
			(*pcFetched)++;
            InterlockedIncrement(&m_nIndex);
	    }
	    return hr;
    }
    // IEnumConnectionPoints::Skip implementation
	HRESULT __stdcall Skip(ULONG cConnections)
    {
        LONG nIndex = 0;
        HRESULT hr = S_OK;

        InterlockedExchange(&nIndex, m_nIndex);
        if (nIndex >= (LONG)m_cConnections)
            return S_FALSE;
        for (ULONG i=0;i<cConnections;i++)
	    {
            nIndex = InterlockedIncrement(&m_nIndex);
            if (nIndex >= (LONG)m_cConnections)
            {
                hr = S_FALSE;
                break;
            }
        }
        return hr;
    }
    // IEnumConnectionPoints::Reset implementation
	HRESULT __stdcall Reset()
    {
        InterlockedExchange(&m_nIndex, 0);
        return S_OK;
    }
    // IEnumConnectionPoints::Clone implementation
	HRESULT __stdcall Clone(IEnumConnectionPoints** ppEnum)
    {
        CMCLEnumConnectionPoints* pNew = NULL;
        HRESULT hr = S_OK;

	    if(ppEnum == NULL)
		    return E_INVALIDARG;
	    *ppEnum = NULL;
        pNew = new CMCLEnumConnectionPoints(m_pThis, (ULONG)m_cConnections, m_rgpcn);
        if(pNew == NULL)
            return E_OUTOFMEMORY;
        pNew->m_nIndex = m_nIndex;
        hr = pNew->QueryInterface(IID_IEnumConnectionPoints, (void**)ppEnum);
        pNew->Release();
        return hr;
    }
/*--------------------------------------
  CMCLEnumConnectionPoints Properties
--------------------------------------*/
private:
	long m_cRef;
    IUnknown* m_pThis;          // Containing IUnknown for ref counting
    long m_nIndex;              // Index of current element
    IConnectionPoint** m_rgpcn; // Array of connection points
    ULONG m_cConnections;        // Number of connection points in the array
};

class CMCLEnumConnections : public IEnumConnections
{
public:
/*--------------------------------------
  Construction/destruction
--------------------------------------*/
    // CMCLEnumConnections default constructor
    CMCLEnumConnections()
    {
        m_cRef = 1;
	    m_nIndex = 0;
        m_pThis = NULL;
        m_rgpcd = NULL;
        m_cConnections = 0;
    }
    // CMCLEnumConnections constructor from parent connection point reference and array of CONNECTDATA's
    CMCLEnumConnections(IUnknown* pThis, ULONG cConnections, CONNECTDATA* rgpcd)
    {
        m_cRef = 1;
	    m_nIndex = 0;
        m_pThis = pThis;
        m_rgpcd = NULL;
        m_cConnections = 0;
        if (cConnections > 0 && rgpcd != NULL)
        {
            m_cConnections = cConnections;
            m_rgpcd = new CONNECTDATA[m_cConnections];
            if (m_rgpcd != NULL)
            {
	            for(ULONG i=0;i<cConnections;i++)
                {
                    m_rgpcd[i] = rgpcd[i];
                    if (m_rgpcd[i].pUnk != NULL)
                        m_rgpcd[i].pUnk->AddRef();
                }
            }
            else
                m_cConnections = 0;
        }
    }
    // CMCLEnumConnections constructor from parent connection point reference and vector class of CONNECTDATA's
    CMCLEnumConnections(IUnknown* pThis, vector<CONNECTDATA*>& vecpcd)
    {
        vector<CONNECTDATA*>::iterator it;
        CONNECTDATA* pConnData = NULL;
        int i = 0;

        m_cRef = 1;
	    m_nIndex = 0;
        m_pThis = pThis;
        m_rgpcd = NULL;
        m_cConnections = 0;
        if (vecpcd.size() > 0)
        {
            m_cConnections = (ULONG)vecpcd.size();
            m_rgpcd = new CONNECTDATA[m_cConnections];
            if (m_rgpcd != NULL)
            {
	            for(it = vecpcd.begin(); it != vecpcd.end(); it++)
                {
                    pConnData = *it;
                    if (pConnData != NULL)
                    {
                        m_rgpcd[i] = *pConnData;
                        if (m_rgpcd[i].pUnk != NULL)
                            m_rgpcd[i].pUnk->AddRef();
                    }
                    else
                    {
                        m_rgpcd[i].pUnk = NULL;
                        m_rgpcd[i].dwCookie = 0;
                    }
                }
                i++;
            }
            else
                m_cConnections = 0;
        }
    }
    // CMCLEnumConnections destructor
	virtual ~CMCLEnumConnections()
    {
        if (m_rgpcd != NULL)
        {
            for (size_t i=0;i<m_cConnections;i++)
            {
                if (m_rgpcd[i].pUnk != NULL)
                    m_rgpcd[i].pUnk->Release();
            }
            delete [] m_rgpcd;
        }
    }
/*--------------------------------------
  IUnknown implementation
--------------------------------------*/
public:
	// IUnknown::AddRef implementation
	ULONG __stdcall AddRef()
    {
        if (m_pThis != NULL)
	        m_pThis->AddRef();
	    return InterlockedIncrement(&m_cRef);
    }
    // IUnknown::Release implementation
	ULONG __stdcall Release()
    {
        if (m_pThis != NULL)
	        m_pThis->Release();
	    ULONG cRef = InterlockedDecrement(&m_cRef);
		if(cRef != 0)
			return cRef;
        delete this;
        return 0;
    }
	// IUnknown::QueryInterface implementation
	HRESULT __stdcall QueryInterface(REFIID riid, void** ppv)
	{
		if(riid == IID_IEnumConnections)
			*ppv = static_cast<IEnumConnections*>(this);
		else if(riid == IID_IUnknown)
			*ppv = static_cast<IUnknown*>(this);
		else
		{
			*ppv = NULL;
			return E_NOINTERFACE;
		}
		AddRef();
		return S_OK;
	}
/*--------------------------------------
  IEnumConnections implementation
--------------------------------------*/
public:
    // IEnumConnections::Next implementation
	HRESULT __stdcall Next(ULONG cConnections, CONNECTDATA* rgpcd, ULONG* pcFetched)
    {
        LONG nIndex = 0;
        HRESULT hr = S_OK;

	    if(rgpcd == NULL || pcFetched == NULL)
		    return E_INVALIDARG;
	    *pcFetched = 0;
	    if (cConnections == 0)
            return S_OK;
	    for (ULONG i=0;i<cConnections;i++)
	    {
            InterlockedExchange(&nIndex, m_nIndex);
            if (nIndex >= (LONG)m_cConnections)
            {
                hr = S_FALSE;
                break;
            }
		    rgpcd[i] = m_rgpcd[nIndex];
		    if(rgpcd[i].pUnk != NULL)
			    rgpcd[i].pUnk->AddRef();
			(*pcFetched)++;
            InterlockedIncrement(&m_nIndex);
	    }
	    return hr;
    }
    // IEnumConnections::Skip implementation
	HRESULT __stdcall Skip(ULONG cConnections)
    {
         LONG nIndex = 0;
        HRESULT hr = S_OK;

        InterlockedExchange(&nIndex, m_nIndex);
        if (nIndex >= (LONG)m_cConnections)
            return S_FALSE;
        for (ULONG i=0;i<cConnections;i++)
	    {
            nIndex = InterlockedIncrement(&m_nIndex);
            if (nIndex >= (LONG)m_cConnections)
            {
                hr = S_FALSE;
                break;
            }
        }
        return hr;
    }
    // IEnumConnections::Reset implementation
	HRESULT __stdcall Reset()
    {
        InterlockedExchange(&m_nIndex, 0);
        return S_OK;
    }
    // IEnumConnections::Clone implementation
	HRESULT __stdcall Clone(IEnumConnections** ppEnum)
    {
        CMCLEnumConnections* pNew = NULL;
        HRESULT hr = S_OK;

	    if(ppEnum == NULL)
		    return E_INVALIDARG;
	    *ppEnum = NULL;
        pNew = new CMCLEnumConnections(m_pThis,(ULONG) m_cConnections, m_rgpcd);
        if(pNew == NULL)
            return E_OUTOFMEMORY;
        pNew->m_nIndex = m_nIndex;
        hr = pNew->QueryInterface(IID_IEnumConnections, (void**)ppEnum);
        pNew->Release();
        return hr;
    }
/*--------------------------------------
  CMCLEnumConnectionPoints Properties
--------------------------------------*/
private:
	long m_cRef;
    IUnknown* m_pThis;           // Containing IUnknown for ref counting
    long m_nIndex;               // Index of current element
    ULONG m_cConnections;         // Number of connections in the array
    CONNECTDATA* m_rgpcd;        // Array of connections
};

template<const IID* piid>
class CMCLConnectionPointImpl : public IConnectionPoint
{
/*--------------------------------------
  Construction/destruction
--------------------------------------*/
public:
    // CMCLConnectionPointImpl default constructor
    CMCLConnectionPointImpl()
    {
        m_cRef = 1;
	    m_pCPC = NULL;
        m_nNextCookie = 0;
    }
    // CMCLConnectionPointImpl constructor from container reference
    CMCLConnectionPointImpl(IConnectionPointContainer* pCPC)
    {
        m_cRef = 1;
	    m_pCPC = NULL;
        m_nNextCookie = 0;
        m_pCPC = pCPC;
    }
    // CMCLConnectionPointImpl destructor
	virtual ~CMCLConnectionPointImpl()
    {
        vector<CONNECTDATA*>::iterator it;
        CONNECTDATA* pConnData = NULL;

        for(it = m_vecpcd.begin(); it != m_vecpcd.end(); it++)
        {
		    pConnData = *it;
            if (pConnData != NULL)
            {
                if (pConnData->pUnk != NULL)
                    pConnData->pUnk->Release();
                delete pConnData;
            }
        }
        m_vecpcd.clear();
    }
/*--------------------------------------
  IUnknown implementation
--------------------------------------*/
public:
	// IUnknown::AddRef implementation
	ULONG __stdcall AddRef()
    {
	    return InterlockedIncrement(&m_cRef);
    }
    // IUnknown::Release implementation
	ULONG __stdcall Release()
    {
	    ULONG cRef = InterlockedDecrement(&m_cRef);
		if(cRef != 0)
			return cRef;
        delete this;
        return 0;
    }
	// IUnknown::QueryInterface implementation
	HRESULT __stdcall QueryInterface(REFIID riid, void** ppv)
	{
		if(riid == IID_IConnectionPoint)
			*ppv = static_cast<IConnectionPoint*>(this);
		else if(riid == IID_IUnknown)
			*ppv = static_cast<IUnknown*>(this);
		else
		{
			*ppv = NULL;
			return E_NOINTERFACE;
		}
		AddRef();
		return S_OK;
	}

/*--------------------------------------
  IConnectionPoint implementation
--------------------------------------*/
    // IConnectionPoint::GetConnectionInterface implementation
	HRESULT __stdcall GetConnectionInterface(IID *pIID)
    {
        if (pIID == NULL)
            return E_INVALIDARG;
        *pIID = *piid;
        return S_OK;
    }
    // IConnectionPoint::GetConnectionPointContainer implementation
	HRESULT __stdcall GetConnectionPointContainer(IConnectionPointContainer** ppCPC)
    {
        if (ppCPC == NULL)
            return E_POINTER;
        if (m_pCPC == NULL)
            return E_UNEXPECTED;
        *ppCPC = m_pCPC;
        if (*ppCPC != NULL)
            (*ppCPC)->AddRef();
        return S_OK;
    }
    // IConnectionPoint::Advise implementation
	HRESULT __stdcall Advise(IUnknown* pUnk, DWORD* pdwCookie)
    {
    	IUnknown* pSink = NULL;
        long nCookie = 0;
        CONNECTDATA* pConnData = NULL;

        ModuleLock lock;
        if (pUnk == NULL || pdwCookie == NULL)
            return E_POINTER;
	    *pdwCookie = 0;
	    if(FAILED(pUnk->QueryInterface(*piid, (void**)&pSink)))
		    return CONNECT_E_CANNOTCONNECT;
        pConnData = new CONNECTDATA;
        if (pConnData == NULL)
            return E_OUTOFMEMORY;
        nCookie = InterlockedIncrement(&m_nNextCookie);
        pConnData->dwCookie = (DWORD)nCookie;
        pConnData->pUnk = pUnk;
	    m_vecpcd.push_back(pConnData);
        return S_OK;
    }
    // IConnectionPoint::Unadvise implementation
	HRESULT __stdcall Unadvise(DWORD dwCookie)
    {
        vector<CONNECTDATA*>::iterator it;
        CONNECTDATA* pConnData = NULL;
        bool bFound = false;
        
        ModuleLock lock;
	    if(dwCookie == 0)
		    return CONNECT_E_NOCONNECTION;
	    for(it = m_vecpcd.begin(); it != m_vecpcd.end(); it++)
        {
		    pConnData = *it;
            if (pConnData != NULL)
            {
                if (pConnData->dwCookie == dwCookie)
                {
                    bFound = true;
                    if (pConnData->pUnk != NULL)
                        pConnData->pUnk->Release();
                    delete pConnData;
                    break;
                }
            }
        }
        if (bFound)
        {
            m_vecpcd.erase(it);
            return S_OK;
        }
	    return CONNECT_E_NOCONNECTION;
    }
    // IConnectionPoint::EnumConnections implementation
	HRESULT __stdcall EnumConnections(IEnumConnections** ppEnum)
    {
        CMCLEnumConnections* pEnum = NULL;

        ModuleLock lock;
        if (ppEnum == NULL)
            return E_POINTER;
	    *ppEnum = NULL;
	    pEnum = new CMCLEnumConnections(this, m_vecpcd);
        if (pEnum == NULL)
            return E_OUTOFMEMORY;
	    if (FAILED(pEnum->QueryInterface(IID_IEnumConnections, (void**)ppEnum)))
            return E_UNEXPECTED;
        return S_OK;
    }
/*--------------------------------------
  CConnectionPoint Properties
--------------------------------------*/
private:
	long m_cRef;                        // Reference count
	IConnectionPointContainer* m_pCPC;  // Pointer to parent container
    long m_nNextCookie;                 // Next available cookie value
    vector<CONNECTDATA*> m_vecpcd;      // Array of connections
};

template<typename Iterface>
struct MarshalHelperDelegate
{
    template<typename Derived>
    static HRESULT copyMWFlags(MCLCONVERSION_FLAGS flags, void * receiver)
    {
        HRESULT hr;
        Derived * derivedObject = static_cast<Derived*>(/*this*/receiver);
        IMWFlags * mwflags = NULL;
        if(FAILED(hr = derivedObject->get_MWFlags(&mwflags)))
            goto EXIT;
        if(FAILED(hr = mclPutConversionFlags(mwflags, flags)))
            goto EXIT;
        if(FAILED(derivedObject->put_MWFlags(mwflags)))
            goto EXIT;
EXIT:
        if(mwflags)
            mwflags->Release();
        return hr;
    }
};

template<>
struct MarshalHelperDelegate<IMWStruct>
{
    template<typename Derived>
    static HRESULT copyMWFlags(MCLCONVERSION_FLAGS, void *)
    {
        return E_NOTIMPL;
    }
};


template<typename Interface, typename Derived>
struct MarshalHelper
{
    HRESULT copyMWFlags(MCLCONVERSION_FLAGS flags, void * receiver)
    {
        return MarshalHelperDelegate<Interface>::copyMWFlags<Derived>(flags,receiver);
    }
};

template<typename T, const IID* piid, typename T1, const CLSID* pclsid>
class CMCLMarshalImpl: public CMCLBaseImpl<T, piid, T1, pclsid>, public IMarshal, public MarshalHelper<T, T1>
{
public:
    CMCLMarshalImpl()
    {
        m_serializedData = NULL;
    }

    // IUnknown
	ULONG __stdcall AddRef()
    {
        return typename CMCLBaseImpl<T, piid, T1, pclsid>::AddRef();
    }
	ULONG __stdcall Release()
    {
        return typename CMCLBaseImpl<T, piid, T1, pclsid>::Release();
    }
	HRESULT __stdcall QueryInterface(REFIID riid, void** ppv)
    {
        if(riid == IID_IMarshal)
        {
			*ppv = static_cast<IMarshal*>(this);
		    AddRef();
		    return S_OK;
        }
        else 
        return typename CMCLBaseImpl<T, piid, T1, pclsid>::QueryInterface(riid, ppv);
    }

    // IMarshal
	HRESULT __stdcall GetUnmarshalClass(REFIID riid, void* pv, DWORD dwDestContext, void* pvDestContext, DWORD dwFlags, CLSID* pClsid)
    {
        if(dwDestContext == MSHCTX_DIFFERENTMACHINE || dwDestContext == MSHCTX_INPROC)
	    {
		    IMarshal* pMarshal;
		    CoGetStandardMarshal(riid, (T*)pv, dwDestContext, pvDestContext, dwFlags, &pMarshal);
		    HRESULT hr = pMarshal->GetUnmarshalClass(riid, pv, dwDestContext, pvDestContext, dwFlags, pClsid);
		    pMarshal->Release();
		    return hr;
	    }
	    *pClsid = *(const_cast<CLSID*>(pclsid));
        return S_OK;
    }

	HRESULT __stdcall GetMarshalSizeMax(REFIID riid, void* pv, DWORD dwDestContext, void* pvDestContext, DWORD dwFlags, DWORD* pSize)
    {
        if(dwDestContext == MSHCTX_DIFFERENTMACHINE || dwDestContext == MSHCTX_INPROC)
	    {
		    IMarshal* pMarshal;
		    CoGetStandardMarshal(riid, (T*)pv, dwDestContext, pvDestContext, dwFlags, &pMarshal);
		    HRESULT hr = pMarshal->GetMarshalSizeMax(riid, pv, dwDestContext, pvDestContext, dwFlags, pSize);
		    pMarshal->Release();
		    return hr;
	    }
        //serialize the data the first time, this method is called twice during a marshalling operation
        if(m_serializedData == NULL)
        {
            if(FAILED(Serialize(pv)))
            {
                return E_FAIL;
            }
        }
        *pSize = static_cast<DWORD>(mxGetN(m_serializedData) + 4);
        T1 * derivedObject = dynamic_cast<T1*>(this);
        if(derivedObject->getCurrentMWFlags())
        {
            *pSize += sizeof(_MCLCONVERSION_FLAGS);
        }
        return S_OK;
    }

	HRESULT __stdcall MarshalInterface(IStream* pStream, REFIID riid, void* pv, DWORD dwDestContext, void* pvDestContext, DWORD dwFlags)
    {
        if(dwDestContext == MSHCTX_DIFFERENTMACHINE || dwDestContext == MSHCTX_INPROC)
	    {
		    IMarshal* pMarshal;
		    CoGetStandardMarshal(riid, (T*)pv, dwDestContext, pvDestContext, dwFlags, &pMarshal);
		    HRESULT hr = pMarshal->MarshalInterface(pStream, riid, pv, dwDestContext, pvDestContext, dwFlags);
		    pMarshal->Release();
		    return hr;
	    }
        
        HRESULT hr = S_OK;
        DWORD sizeOfPacket = static_cast<DWORD>(mxGetN(m_serializedData));
        unsigned long bytesWritten = 0;
        if(FAILED(hr = pStream->Write(&sizeOfPacket, 4, &bytesWritten)) 
            || bytesWritten != 4)
        {
            Error("Serialization error writing the size of the packet.");
            return hr;
        }
        if(FAILED(hr = pStream->Write(mxGetData(m_serializedData), sizeOfPacket, &bytesWritten)) 
            || bytesWritten != sizeOfPacket) 
        {
            Error("Serialization error writing the packet.");
            return hr;
        }
        T1 * derivedObject = dynamic_cast<T1*>(this);

        IMWFlags * mwflags;
        if((mwflags = static_cast<IMWFlags*>(derivedObject->getCurrentMWFlags())) != NULL)
        {
             HRESULT hr;   
            _MCLCONVERSION_FLAGS flags;
            if((hr = mclGetConversionFlags(mwflags,&flags)) != S_OK)
                return hr;
            if(FAILED(hr = pStream->Write(&flags, sizeof(_MCLCONVERSION_FLAGS), &bytesWritten)) 
                || bytesWritten != sizeof(_MCLCONVERSION_FLAGS))
            {
                Error("Serialization error writing conversion flags.");
                return hr;
            }
        }
        return hr;
    }

	HRESULT __stdcall DisconnectObject(DWORD dwReserved)
    {
        return E_NOTIMPL;
    }
	HRESULT __stdcall UnmarshalInterface(IStream* pStream, REFIID riid, void** ppv)
    {
         HRESULT hr = S_OK; 
        DWORD sizeOfPacket = 0;
        unsigned long bytesRead = 0;
        if(FAILED(hr = pStream->Read(&sizeOfPacket, 4, &bytesRead)))
        {
            Error("Deserialization error reading the size of the packet.");
            return hr;
        }
        char * unmarshalledPacket = new char[sizeOfPacket];

        if(FAILED(pStream->Read(unmarshalledPacket, sizeOfPacket, &bytesRead)))
        {
            Error("Deserialization error reading the packet.");
            delete [] unmarshalledPacket;
            return hr;
        }

        mxArray * unmarshalledMxArray  = mclMxDeserialize(unmarshalledPacket, sizeOfPacket);
        delete [] unmarshalledPacket;
        
        _MCLCONVERSION_FLAGS flags;
        mclInitConversionFlags(&flags);
        VARIANT out;
        VariantInit(&out);
        if(mclmxArray2Variant(unmarshalledMxArray, &out, &flags) < 0) 
        {
            Error("Deserialization error");
            return E_FAIL;
        }
        if(unmarshalledMxArray != NULL)
            mxDestroyArray(unmarshalledMxArray);
        
       
        //Try to unmarshall MWFlags from pStream. This should only be true for data types 
        //which have a MWFlags and it was not NULL during marshalling. 
        hr = pStream->Read(&flags, sizeof(_MCLCONVERSION_FLAGS), &bytesRead);
        if(!(hr == S_OK || hr == S_FALSE))
        {
            Error("Deserialization error reading conversion flags.");
            return hr;
        }

        //if unmarshalledMxArray == NULL, it means that the Variant type was default constructed
        //but no value was assigned. So we can simply assign the this pointer to *ppv
        if(unmarshalledMxArray != NULL)
        {
             if(bytesRead == sizeof(_MCLCONVERSION_FLAGS))
             {
                copyMWFlags(&flags, out.pdispVal);
             }
             *ppv = static_cast<T*>(out.pdispVal);
        }
        else
        {
            if(bytesRead == sizeof(_MCLCONVERSION_FLAGS))
            {
                copyMWFlags(&flags, this);
            }
            return QueryInterface(*piid, ppv);
        }

        return hr;
    }

	HRESULT __stdcall ReleaseMarshalData(IStream* pStream)
    {
        if(m_serializedData != NULL)
            mxDestroyArray(m_serializedData);
        return S_OK;
    }
    
private:

    HRESULT __stdcall Serialize(void * pv)
    {
        VARIANT wrapper;
        wrapper.vt = VT_DISPATCH;
        wrapper.pdispVal = static_cast<IDispatch *>(pv);
        mxArray * temp = NULL;
        _MCLCONVERSION_FLAGS flags;
        mclInitConversionFlags(&flags);
        if(mclVariant2mxArray(&wrapper,&temp,&flags) < 0)
        {
            Error("Error in data conversion");
            return E_FAIL;
        }
        
        m_serializedData = mclMxSerialize(temp);
        if(temp != NULL)
            mxDestroyArray(temp);

        if(m_serializedData == NULL)
        {
            Error("Error in internal data serialization");
            return E_FAIL;
        }
        return S_OK;
    }

private:
    mxArray * m_serializedData;
};


#endif //ifdef __cplusplus
#endif //ifndef _MCLCOMCLASS_H_

