/*------------------------------------------------------------------------
* (The MIT License)
* 
* Copyright (c) 2008-2011 Rhomobile, Inc.
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
* 
* http://rhomobile.com
*------------------------------------------------------------------------*/

#include "RhodesApp.h"
#include "common/RhoMutexLock.h"
#include "common/IRhoClassFactory.h"
#include "common/RhoFile.h"
#include "common/RhoConf.h"
#include "common/RhoFilePath.h"
#include "common/RhoAppAdapter.h"
#include "net/INetRequest.h"
#include "sync/ClientRegister.h"
#include "sync/SyncThread.h"
#include "net/URI.h"

#include "net/HttpServer.h"
#include "ruby/ext/rho/rhoruby.h"
#include "net/AsyncHttp.h"
#include "rubyext/WebView.h"
#include "rubyext/GeoLocation.h"
#include "common/app_build_configs.h"
#include "unzip/unzip.h"
#include "common/Tokenizer.h"

#include <algorithm>

#ifdef OS_WINCE
#include <winsock.h>
#endif 

using rho::net::HttpHeader;
using rho::net::HttpHeaderList;
using rho::net::CHttpServer;

extern "C" {
void rho_map_location(char* query);
void rho_appmanager_load( void* httpContext, const char* szQuery);
void rho_db_init_attr_manager();
void rho_sys_app_exit();
void rho_sys_report_app_started();
}
    
namespace rho {
namespace common{

IMPLEMENT_LOGCLASS(CRhodesApp,"RhodesApp");
String CRhodesApp::m_strStartParameters;
boolean CRhodesApp::m_bSecurityTokenNotPassed = false;

class CAppCallbacksQueue : public CThreadQueue
{
    DEFINE_LOGCLASS;
public:
    enum callback_t
    {
        app_deactivated,
        local_server_restart,
        local_server_started,
        ui_created,
        app_activated
    };

public:
    CAppCallbacksQueue();
	CAppCallbacksQueue(LogCategory logCat);
	~CAppCallbacksQueue();

    //void call(callback_t type);

    friend class Command;
    struct Command : public IQueueCommand
    {
        callback_t type;
        Command(callback_t t) :type(t) {}

        boolean equals(IQueueCommand const &) {return false;}
        String toString() {return CAppCallbacksQueue::toString(type);}
    };

private:

    void processCommand(IQueueCommand* pCmd);

    static char const *toString(int type);
    void   callCallback(const String& strCallback);

private:
    callback_t m_expected;
    Vector<int> m_commands;
    boolean m_bFirstServerStart;
};
IMPLEMENT_LOGCLASS(CAppCallbacksQueue,"AppCallbacks");

char const *CAppCallbacksQueue::toString(int type)
{
    switch (type)
    {
    case local_server_started:
        return "LOCAL-SERVER-STARTED";
    case ui_created:
        return "UI-CREATED";
    case app_activated:
        return "APP-ACTIVATED";
    case app_deactivated:
        return "APP-DEACTIVATED";
    case local_server_restart:
        return "LOCAL-SERVER-RESTART";
    default:
        return "UNKNOWN";
    }
}

CAppCallbacksQueue::CAppCallbacksQueue()
    :CThreadQueue(), m_expected(local_server_started), m_bFirstServerStart(true)
{
    CThreadQueue::setLogCategory(getLogCategory());
    setPollInterval(QUEUE_POLL_INTERVAL_INFINITE);
    start(epNormal);
}

CAppCallbacksQueue::~CAppCallbacksQueue()
{
}
/*
void CAppCallbacksQueue::call(CAppCallbacksQueue::callback_t type)
{
    addQueueCommand(new Command(type));
}*/

void CAppCallbacksQueue::callCallback(const String& strCallback)
{
    String strUrl = RHODESAPP().getBaseUrl();
    strUrl += strCallback;
    NetResponse resp = getNetRequest().pullData( strUrl, null );
    if ( !resp.isOK() )
    {
        boolean bTryAgain = false;
#if defined( __SYMBIAN32__ ) || defined( OS_ANDROID )
        if ( String_startsWith( strUrl, "http://localhost:" ) )
        {
            RHODESAPP().setBaseUrl("http://127.0.0.1:");
            bTryAgain = true;
        }
#else
        if ( String_startsWith( strUrl, "http://127.0.0.1:" ) )
        {
            RHODESAPP().setBaseUrl("http://localhost:");
            bTryAgain = true;
        }
#endif

        if ( bTryAgain )
        {
            LOG(INFO) + "Change base url and try again.";
            strUrl = RHODESAPP().getBaseUrl();
            strUrl += strCallback;
            resp = getNetRequest().pullData( strUrl, null );
        }

        if ( !resp.isOK() )
            LOG(ERROR) + strCallback + " call failed. Code: " + resp.getRespCode() + "; Error body: " + resp.getCharData();
    }
}

void CAppCallbacksQueue::processCommand(IQueueCommand* pCmd)
{
    Command *cmd = (Command *)pCmd;
    if (!cmd)
        return;
    
/*
    if (cmd->type < m_expected)
    {
        LOG(ERROR) + "received command " + toString(cmd->type) + " which is less than expected "+toString(m_expected)+" - ignore it";
        return;
    }
*/
    if ( m_expected == app_deactivated && cmd->type == app_activated )
    {
        LOG(INFO) + "received duplicate activate skip it";
        return;
    }

    if ( m_expected == local_server_restart )
    {
        if ( cmd->type != local_server_started )
            RHODESAPP().restartLocalServer(*this);
        else
            LOG(INFO) + "Local server restarted before activate.Do not restart it again.";

        m_expected = local_server_started;
    }

    if (cmd->type > m_expected)
    {
        boolean bDuplicate = false;
        for( int i = 0; i < (int)m_commands.size() ; i++)
        {
            if ( m_commands.elementAt(i) == cmd->type )
            {
                bDuplicate = true;
                break;
            }
        }

        if ( bDuplicate )
        {
            LOG(INFO) + "Received duplicate command " + toString(cmd->type) + "skip it";
        }else
        {
            // Don't do that now
            LOG(INFO) + "Received command " + toString(cmd->type) + " which is greater than expected (" + toString(m_expected) + ") - postpone it";
            m_commands.push_back(cmd->type);
            std::sort(m_commands.begin(), m_commands.end());
        }
        return;
    }

    if ( cmd->type == app_deactivated )
        m_commands.clear();

    m_commands.insert(m_commands.begin(), cmd->type);
    for (Vector<int>::const_iterator it = m_commands.begin(), lim = m_commands.end(); it != lim; ++it)
    {
        int type = *it;
        LOG(INFO) + "process command: " + toString(type);
        switch (type)
        {
        case app_deactivated:
#if !defined( OS_WINCE ) && !defined (OS_WINDOWS)
            m_expected = local_server_restart;
#else
            m_expected = app_activated;
#endif
            break;

        case local_server_started:
            if ( m_bFirstServerStart )
            {
                m_expected = ui_created;
                m_bFirstServerStart = false;
            }
            else
                m_expected = app_activated;

            rho_sys_report_app_started();
            break;
        case ui_created:
            {
                callCallback("/system/uicreated");
                m_expected = app_activated;
            }
            break;
        case app_activated:
            {
                callCallback("/system/activateapp");
                m_expected = app_deactivated;
            }
            break;
        }
        //if (type < app_activated && type != app_deactivated)
        //    m_expected = (callback_t)(type + 1);
    }
    m_commands.clear();
}

/*static*/ CRhodesApp* CRhodesApp::Create(const String& strRootPath, const String& strUserPath, const String& strRuntimePath)
{
    if ( m_pInstance != null) 
        return (CRhodesApp*)m_pInstance;

    m_pInstance = new CRhodesApp(strRootPath, strUserPath, strRuntimePath);

    String push_pin = RHOCONF().getString("push_pin");
    if(!push_pin.empty())
        rho::sync::CClientRegister::Create(push_pin.c_str());

    return (CRhodesApp*)m_pInstance;
}

/*static*/void CRhodesApp::Destroy()
{
    if ( m_pInstance )
        delete m_pInstance;

    m_pInstance = 0;
    
    

}

CRhodesApp::CRhodesApp(const String& strRootPath, const String& strUserPath, const String& strRuntimePath)
    :CRhodesAppBase(strRootPath, strUserPath, strRuntimePath)
{
    m_bExit = false;
    m_bDeactivationMode = false;
    m_bRestartServer = false;
    m_bSendingLog = false;
    //m_activateCounter = 0;
    m_pExtManager = 0;

    m_appCallbacksQueue = new CAppCallbacksQueue();

#if defined(OS_WINCE) || defined (OS_WINDOWS)
    //initializing winsock
    WSADATA WsaData;
    int result = WSAStartup(MAKEWORD(2,2),&WsaData);
#endif

    initAppUrls();

   	LOGCONF().initRemoteLog();

    initHttpServer();

    getSplashScreen().init();
}

void CRhodesApp::startApp()
{
    start(epNormal);
}

void CRhodesApp::run()
{
    LOG(INFO) + "Starting RhodesApp main routine...";
    RhoRubyStart();
    rubyext::CGeoLocation::Create();

    //rho_db_init_attr_manager();

    LOG(INFO) + "Starting sync engine...";
    sync::CSyncThread::Create();

    LOG(INFO) + "RhoRubyInitApp...";
    RhoRubyInitApp();
    rho_ruby_call_config_conflicts();
    RHOCONF().conflictsResolved();

    while (!m_bExit) {
        m_httpServer->run();
        if (m_bExit)
            break;

        if ( !m_bRestartServer )
        {
            LOG(INFO) + "RhodesApp thread wait.";
            wait(-1);
        }
        m_bRestartServer = false;
    }

    LOG(INFO) + "RhodesApp thread shutdown";

    getExtManager().close();
    rubyext::CGeoLocation::Destroy();
    sync::CSyncThread::Destroy();

    net::CAsyncHttp::Destroy();

    RhoRubyStop();
}

CRhodesApp::~CRhodesApp(void)
{
    stopApp();

	LOGCONF().closeRemoteLog();

#ifdef OS_WINCE
    WSACleanup();
#endif

}

boolean CRhodesApp::callTimerCallback(const String& strUrl, const String& strData)
{
    String strBody = "rho_callback=1";
		
    if ( strData.length() > 0 )
        strBody += "&" + strData;

    String strReply;
    return m_httpServer->call_ruby_method(strUrl, strBody, strReply);
}

void CRhodesApp::restartLocalServer(common::CThreadQueue& waitThread)
{
    LOG(INFO) + "restart local server.";
    m_bRestartServer = true;
    m_httpServer->stop();
	stopWait();
}

void CRhodesApp::stopApp()
{
   	m_appCallbacksQueue->stop(1000);

    if (!m_bExit)
    {
        m_bExit = true;
        m_httpServer->stop();
        stopWait();
        stop(2000);
    }

//    net::CAsyncHttp::Destroy();
}

template <typename T>
class CRhoCallInThread : public common::CRhoThread
{
public:
    CRhoCallInThread(T* cb)
        :CRhoThread(), m_cb(cb)
    {
        start(epNormal);
    }

private:
    virtual void run()
    {
        m_cb->run(*this);
    }

    virtual void runObject()
    {
        common::CRhoThread::runObject();
        delete this;
    }

private:
    common::CAutoPtr<T> m_cb;
};

template <typename T>
void rho_rhodesapp_call_in_thread(T *cb)
{
    new CRhoCallInThread<T>(cb);
}

class CRhoCallbackCall
{
    String m_strCallback, m_strBody;
public:
    CRhoCallbackCall(const String& strCallback, const String& strBody)
      : m_strCallback(strCallback), m_strBody(strBody)
    {}

    void run(common::CRhoThread &)
    {
        getNetRequest().pushData( m_strCallback, m_strBody, null );
    }
};

void CRhodesApp::runCallbackInThread(const String& strCallback, const String& strBody)
{
    rho_rhodesapp_call_in_thread(new CRhoCallbackCall(strCallback, strBody ) );
}

static void callback_activateapp(void *arg, String const &strQuery)
{
    rho_ruby_activateApp();
    String strMsg;
    rho_http_sendresponse(arg, strMsg.c_str());
}

static void callback_deactivateapp(void *arg, String const &strQuery)
{
    rho_ruby_deactivateApp();
    String strMsg;
    rho_http_sendresponse(arg, strMsg.c_str());
}

static void callback_uicreated(void *arg, String const &strQuery)
{
    rho_ruby_uiCreated();
    rho_http_sendresponse(arg, "");
}

static void callback_uidestroyed(void *arg, String const &strQuery)
{
    rho_ruby_uiDestroyed();
    rho_http_sendresponse(arg, "");
}

static void callback_loadserversources(void *arg, String const &strQuery)
{
    RhoAppAdapter.loadServerSources(strQuery);
    String strMsg;
    rho_http_sendresponse(arg, strMsg.c_str());
}

static void callback_loadallsyncsources(void *arg, String const &strQuery)
{
    RhoAppAdapter.loadAllSyncSources();
    String strMsg;
    rho_http_sendresponse(arg, strMsg.c_str());
}

static void callback_resetDBOnSyncUserChanged(void *arg, String const &strQuery)
{
    RhoAppAdapter.resetDBOnSyncUserChanged();
    String strMsg;
    rho_http_sendresponse(arg, strMsg.c_str());
}

void CRhodesApp::callUiCreatedCallback()
{
    m_appCallbacksQueue->addQueueCommand(new CAppCallbacksQueue::Command(CAppCallbacksQueue::ui_created));
}

void CRhodesApp::callUiDestroyedCallback()
{
    if ( m_bExit )
        return;

    String strUrl = m_strHomeUrl + "/system/uidestroyed";
    NetResponse resp = getNetRequest().pullData( strUrl, null );
    if ( !resp.isOK() )
    {
        LOG(ERROR) + "UI destroy callback failed. Code: " + resp.getRespCode() + "; Error body: " + resp.getCharData();
    }
}

void CRhodesApp::callAppActiveCallback(boolean bActive)
{
    LOG(INFO) + "callAppActiveCallback";
    if (bActive)
    {
        // Restart server each time when we go to foreground
/*        if (m_activateCounter++ > 0)
        {
#if !defined( OS_WINCE ) && !defined (OS_WINDOWS)
            m_httpServer->stop();
#endif
            this->stopWait();

        } */

        m_appCallbacksQueue->addQueueCommand(new CAppCallbacksQueue::Command(CAppCallbacksQueue::app_activated));
    }
    else
    {
        // Deactivation callback must be called in place (not in separate thread!)
        // This is because system can kill application at any time after this callback finished
        // So to guarantee user code is executed on deactivation, we must not exit from this function
        // until application finish executing of user-defined deactivation callback.
        // However, blocking UI thread can cause problem with API refering to UI (such as WebView.navigate etc)
        // To fix this problem, new mode 'deactivation' introduced. When this mode active, no UI operations allowed.
        // All such operation will throw exception in ruby code when calling in 'deactivate' mode.
        m_bDeactivationMode = true;
        m_appCallbacksQueue->addQueueCommand(new CAppCallbacksQueue::Command(CAppCallbacksQueue::app_deactivated));

        String strUrl = m_strHomeUrl + "/system/deactivateapp";
        NetResponse resp = getNetRequest().pullData( strUrl, null );
        if ( !resp.isOK() )
        {
            LOG(ERROR) + "deactivate app failed. Code: " + resp.getRespCode() + "; Error body: " + resp.getCharData();
        }else
        {
            const char* szData = resp.getCharData();
            boolean bStop = szData && strcmp(szData,"stop_local_server") == 0;

            if (bStop)
            {
#if !defined( OS_WINCE ) && !defined (OS_WINDOWS)
                LOG(INFO) + "Stopping local server.";
                m_httpServer->stop();
#endif
            }
        }

        m_bDeactivationMode = false;
    }
}

void CRhodesApp::callBarcodeCallback(String strCallbackUrl, const String& strBarcode, bool isError) 
{
    strCallbackUrl = canonicalizeRhoUrl(strCallbackUrl);
    String strBody;
    strBody = "barcode=" + strBarcode;

    if (isError)
    {
        strBody += "&status=ok";
    }
    else
    {
        strBody += "&status=fail";
    }

    strBody += "&rho_callback=1";
    //getNetRequest().pushData( strCallbackUrl, strBody, null );
    runCallbackInThread(strCallbackUrl, strBody);
}

void CRhodesApp::callCallbackWithData(String strCallbackUrl, String strBody, const String& strCallbackData, bool bWaitForResponse) 
{
    strCallbackUrl = canonicalizeRhoUrl(strCallbackUrl);

    strBody += "&rho_callback=1";

    if (strCallbackData.length() > 0 )
    {
        if ( !String_startsWith( strCallbackData, "&" ) )
            strBody += "&";

        strBody += strCallbackData;
    }

    if (bWaitForResponse)
        getNetRequest().pushData( strCallbackUrl, strBody, null );
    else
        runCallbackInThread(strCallbackUrl, strBody);
}

extern "C" VALUE rjson_tokener_parse(const char *str, char** pszError );

class CJsonResponse : public rho::ICallbackObject
{
    String m_strJson;
public:
    CJsonResponse(const char* szJson) : m_strJson(szJson) { }
    virtual unsigned long getObjectValue()
    {
        char* szError = 0;
        unsigned long valBody = rjson_tokener_parse(m_strJson.c_str(), &szError);
        if ( valBody != 0 )
            return valBody;

        LOG(ERROR) + "Incorrect json body.Error:" + (szError ? szError : "");
        if ( szError )
            free(szError);

        return rho_ruby_get_NIL();
    }
};

void CRhodesApp::callCallbackWithJsonBody( const char* szCallback, const char* szCallbackBody, const char* szCallbackData, bool bWaitForResponse)
{
    String strBody;
    strBody = addCallbackObject( new CJsonResponse( szCallbackBody ), "__rho_inline" );

    callCallbackWithData(szCallback, strBody, szCallbackData, bWaitForResponse );
}

void CRhodesApp::callCameraCallback(String strCallbackUrl, const String& strImagePath, 
    const String& strError, boolean bCancel ) 
{
    strCallbackUrl = canonicalizeRhoUrl(strCallbackUrl);
    String strBody;
    if ( bCancel || strError.length() > 0 )
    {
        if ( bCancel )
            strBody = "status=cancel&message=User canceled operation.";
        else
            strBody = "status=error&message=" + strError;
    }else
        strBody = "status=ok&image_uri=db%2Fdb-files%2F" + strImagePath;

    strBody += "&rho_callback=1";
    getNetRequest().pushData( strCallbackUrl, strBody, null );
}

void CRhodesApp::callSignatureCallback(String strCallbackUrl, const String& strSignaturePath, 
										const String& strError, boolean bCancel ) 
	{
		strCallbackUrl = canonicalizeRhoUrl(strCallbackUrl);
		String strBody;
		if ( bCancel || strError.length() > 0 )
		{
			if ( bCancel )
				strBody = "status=cancel&message=User canceled operation.";
			else
				strBody = "status=error&message=" + strError;
		}else
			strBody = "status=ok&signature_uri=db%2Fdb-files%2F" + strSignaturePath;
		
		strBody += "&rho_callback=1";
		//getNetRequest().pushData( strCallbackUrl, strBody, null );
        runCallbackInThread(strCallbackUrl, strBody);
	}
	
void CRhodesApp::callDateTimeCallback(String strCallbackUrl, long lDateTime, const char* szData, int bCancel )
{
    strCallbackUrl = canonicalizeRhoUrl(strCallbackUrl);
    String strBody;
    if ( bCancel )
        strBody = "status=cancel&message=User canceled operation.";
    else
        strBody = "status=ok&result=" + convertToStringA(lDateTime);

    if ( szData && *szData )
    {
        strBody += "&opaque=";
        strBody += szData;
    }

    strBody += "&rho_callback=1";
    getNetRequest().pushData( strCallbackUrl, strBody, null );
}

void CRhodesApp::callBluetoothCallback(String strCallbackUrl, const char* body) {
	strCallbackUrl = canonicalizeRhoUrl(strCallbackUrl);
	String strBody = body;
	strBody += "&rho_callback=1";
	getNetRequest().pushData( strCallbackUrl, strBody, null );
}

void CRhodesApp::callPopupCallback(String strCallbackUrl, const String &id, const String &title)
{
    if ( strCallbackUrl.length() == 0 )
        return;

    strCallbackUrl = canonicalizeRhoUrl(strCallbackUrl);
    String strBody = "button_id=" + id + "&button_title=" + title;
    strBody += "&rho_callback=1";
    getNetRequest().pushData( strCallbackUrl, strBody, null );
}

static void callback_syncdb(void *arg, String const &/*query*/ )
{
    rho_sync_doSyncAllSources(1,"");
    rho_http_sendresponse(arg, "");
}

static void callback_logger(void *arg, String const &query )
{
    int nLevel = 0;
    String strMsg, strCategory;

    CTokenizer oTokenizer(query, "&");
    while (oTokenizer.hasMoreTokens()) 
    {
	    String tok = oTokenizer.nextToken();
	    if (tok.length() == 0)
		    continue;

        if ( String_startsWith( tok, "level=") )
        {
            String strLevel = tok.substr(6);
            convertFromStringA( strLevel.c_str(), nLevel ); 
        }else if ( String_startsWith( tok, "msg=") )
        {
            strMsg = rho::net::URI::urlDecode(tok.substr(4));
        }else if ( String_startsWith( tok, "cat=") )
        {
            strCategory = rho::net::URI::urlDecode(tok.substr(4));
        }
    }

    rhoPlainLog( "", 0, nLevel, strCategory.c_str(), strMsg.c_str() );

    rho_http_sendresponse(arg, "");
}

static void callback_redirect_to(void *arg, String const &strQuery )
{
    size_t nUrl = strQuery.find("url=");
    String strUrl;
    if ( nUrl != String::npos )
        strUrl = strQuery.substr(nUrl+4);

    if ( strUrl.length() == 0 )
        strUrl = "/app/";
	
    rho_http_redirect(arg, (rho::net::URI::urlDecode(strUrl)).c_str());
}

static void callback_map(void *arg, String const &query )
{
    rho_map_location( const_cast<char*>(query.c_str()) );
    rho_http_sendresponse(arg, "");
}

static void callback_shared(void *arg, String const &/*query*/)
{
    rho_http_senderror(arg, 404, "Not Found");
}

static void callback_AppManager_load(void *arg, String const &query )
{
    rho_appmanager_load( arg, query.c_str() );
}

static void callback_getrhomessage(void *arg, String const &strQuery)
{
    String strMsg;
    size_t nErrorPos = strQuery.find("error=");
    if ( nErrorPos != String::npos )
    {
        String strError = strQuery.substr(nErrorPos+6);
        int nError = atoi(strError.c_str());

        strMsg = rho_ruby_internal_getErrorText(nError);
    }else
    {
        size_t nMsgIdPos = strQuery.find("msgid=");
        if ( nMsgIdPos != String::npos )
        {
            String strName = strQuery.substr(nMsgIdPos+6);
            strMsg = rho_ruby_internal_getMessageText(strName.c_str());
        }
    }

    rho_http_sendresponse(arg, strMsg.c_str());
}

const String& CRhodesApp::getRhoMessage(int nError, const char* szName)
{
    String strUrl = m_strHomeUrl + "/system/getrhomessage?";
    if ( nError )
        strUrl += "error=" + convertToStringA(nError);
    else if ( szName && *szName )
    {
        strUrl += "msgid=";
        strUrl += szName;
    }

    NetResponse resp = getNetRequest().pullData( strUrl, null );
    if ( !resp.isOK() )
    {
        LOG(ERROR) + "getRhoMessage failed. Code: " + resp.getRespCode() + "; Error body: " + resp.getCharData();
        m_strRhoMessage = "";
    }
    else
        m_strRhoMessage = resp.getCharData();

    return m_strRhoMessage;
}

void CRhodesApp::initHttpServer()
{
    String strAppRootPath = getRhoRootPath();
    String strAppUserPath = getRhoUserPath();
    String strRuntimePath = getRhoRuntimePath();
#ifndef RHODES_EMULATOR
    strAppRootPath += "apps";
#endif

    m_httpServer = new net::CHttpServer(atoi(getFreeListeningPort()), strAppRootPath, strAppUserPath, strRuntimePath);
    m_httpServer->register_uri("/system/geolocation", rubyext::CGeoLocation::callback_geolocation);
    m_httpServer->register_uri("/system/syncdb", callback_syncdb);
    m_httpServer->register_uri("/system/redirect_to", callback_redirect_to);
    m_httpServer->register_uri("/system/map", callback_map);
    m_httpServer->register_uri("/system/shared", callback_shared);
    m_httpServer->register_uri("/AppManager/loader/load", callback_AppManager_load);
    m_httpServer->register_uri("/system/getrhomessage", callback_getrhomessage);
    m_httpServer->register_uri("/system/activateapp", callback_activateapp);
    m_httpServer->register_uri("/system/deactivateapp", callback_deactivateapp);
    m_httpServer->register_uri("/system/uicreated", callback_uicreated);
    m_httpServer->register_uri("/system/uidestroyed", callback_uidestroyed);
    m_httpServer->register_uri("/system/loadserversources", callback_loadserversources);
    m_httpServer->register_uri("/system/resetDBOnSyncUserChanged", callback_resetDBOnSyncUserChanged);
    m_httpServer->register_uri("/system/loadallsyncsources", callback_loadallsyncsources);
    m_httpServer->register_uri("/system/logger", callback_logger);
}

const char* CRhodesApp::getFreeListeningPort()
{
	if ( m_strListeningPorts.length() > 0 )
		return m_strListeningPorts.c_str();

    int nFreePort = determineFreeListeningPort();
    m_strListeningPorts = convertToStringA(nFreePort);

	LOG(INFO) + "Free listening port: " + m_strListeningPorts;

    return m_strListeningPorts.c_str();
}

int CRhodesApp::determineFreeListeningPort()
{
    int sockfd = -1;
    sockaddr_in serv_addr = sockaddr_in();
    int nFreePort = 0, noerrors = 1;

	LOG(INFO) + "Trying to get free listening port.";
	
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if ( sockfd < 0 )
    {
        LOG(WARNING) + "Unable to open socket";
        noerrors = 0;
    }
    
    int disable = 0;
    if (noerrors && setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&disable, sizeof(disable)) != 0)
    {
        LOG(WARNING) + "Unable to set socket option";
        noerrors = 0;
    }
#if defined(OS_MACOSX)
    if (noerrors && setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char *)&disable, sizeof(disable)) != 0)
    {
        LOG(WARNING) + "Unable to set socket option";
        noerrors = 0;
    }
#endif
    
    if (noerrors)
    {
        int listenPort = rho_conf_getInt("local_server_port");
        if (listenPort < 0)
            listenPort = 0;
        if (listenPort > 65535)
            listenPort = 0;
        memset((void *) &serv_addr, 0, sizeof(serv_addr));
#if defined(OS_MACOSX)
        serv_addr.sin_len = sizeof(serv_addr);
#endif
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        serv_addr.sin_port = htons((short)listenPort);
        
        LOG(INFO) + "Trying to bind of " + listenPort + " port...";

        if ( bind( sockfd, (struct sockaddr *) &serv_addr, sizeof( serv_addr ) ) != 0 )
        {
            if (listenPort != 0)
            {
                // Fill serv_addr again but with dynamically selected port
#if defined(OS_MACOSX)
                serv_addr.sin_len = sizeof(serv_addr);
#endif
                serv_addr.sin_family = AF_INET;
                serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
                serv_addr.sin_port = htons(0);

                LOG(INFO) + "Trying to bind on dynamic port...";
            
                if ( bind( sockfd, (struct sockaddr *) &serv_addr, sizeof( serv_addr ) ) != 0 )
                {
                    LOG(WARNING) + "Unable to bind";
                    noerrors = 0;
                }
            }
            else
            {
                LOG(WARNING) + "Unable to bind";
                noerrors = 0;
            }
        }
    }

    if ( noerrors )
    {
        char buf[10] = {0};
        
        socklen_t length = sizeof( serv_addr );
        
        if (getsockname( sockfd, (struct sockaddr *)&serv_addr, &length ) != 0)
        {
            LOG(WARNING) + "Can not get socket info";
            nFreePort = 0;
        }
        else
        {
            nFreePort = (int)ntohs(serv_addr.sin_port);
            LOG(INFO) + "Got port to bind on: " + nFreePort;
        }
    }
    
    //Clean up
#if defined(OS_ANDROID)
    close(sockfd);
#else
    closesocket(sockfd);
#endif
	
	return nFreePort;
}
	
void CRhodesApp::initAppUrls() 
{
    CRhodesAppBase::initAppUrls(); 
   
#if defined( __SYMBIAN32__ ) || defined( OS_ANDROID )
    m_strHomeUrl = "http://localhost:";
#elif defined( OS_WINCE ) && !defined(OS_PLATFORM_MOTCE)
    TCHAR oem[257];
    SystemParametersInfo(SPI_GETPLATFORMNAME, sizeof(oem), oem, 0);
    LOG(INFO) + "Device name: " + oem;
    //if ((_tcscmp(oem, _T("MC75"))==0) || (_tcscmp(oem, _T("MC75A"))==0))
    //   m_strHomeUrl = "http://localhost:";
    //else
       m_strHomeUrl = "http://127.0.0.1:";
#else
    m_strHomeUrl = "http://127.0.0.1:";
#endif
    m_strHomeUrl += getFreeListeningPort();

    m_strLoadingPagePath = "file://" + getRhoRootPath() + "apps/app/loading.html";
	m_strLoadingPngPath = getRhoRootPath() + "apps/app/loading.png";
}

void CRhodesApp::keepLastVisitedUrl(String strUrl)
{
    //LOG(INFO) + "Current URL: " + strUrl;
    int nIndex = rho_webview_active_tab();
    if (nIndex < 0) nIndex = 0;
    int nToAdd = nIndex - m_currentUrls.size();
    for ( int i = 0; i <= nToAdd; i++ )
    {
        m_currentUrls.addElement("");
    }

    m_currentUrls[nIndex] = canonicalizeRhoUrl(strUrl);
}

const String& CRhodesApp::getCurrentUrl(int index)
{ 
    if (index < 0) index = rho_webview_active_tab();
    if (index < 0) index = 0;
    if ( index < static_cast<int>(m_currentUrls.size()) )
        return m_currentUrls[index]; 

    return m_EmptyString;
}

const String& CRhodesApp::getAppBackUrl()
{
    int index = rho_webview_active_tab();
    if (index < 0)
        index = 0;
    if ( index < static_cast<int>(m_arAppBackUrl.size()) )
        return m_arAppBackUrl[index]; 

    return m_EmptyString;
}

void CRhodesApp::setAppBackUrl(const String& url)
{
    int nIndex = rho_webview_active_tab();
    if (nIndex < 0)
        nIndex = 0;
    int nToAdd = nIndex - m_arAppBackUrl.size();
    for ( int i = 0; i <= nToAdd; i++ )
    {
        m_arAppBackUrl.addElement("");
        m_arAppBackUrlOrig.addElement("");
    }

    if ( url.length() > 0 )
    {
        m_arAppBackUrlOrig[nIndex] = url;
        m_arAppBackUrl[nIndex] = canonicalizeRhoUrl(url);
    }
    else
    {
        m_arAppBackUrlOrig[nIndex] = "";
        m_arAppBackUrl[nIndex] = "";
    }
}

void CRhodesApp::navigateBack()
{
    int nIndex = rho_webview_active_tab();

    if((nIndex < static_cast<int>(m_arAppBackUrlOrig.size()))
        && (m_arAppBackUrlOrig[nIndex].length() > 0))
    {
        loadUrl(m_arAppBackUrlOrig[nIndex]);
    }
    else if(strcasecmp(getCurrentUrl(nIndex).c_str(),getStartUrl().c_str()) != 0)
    {
        rho_webview_navigate_back();
    }
}

String CRhodesApp::getAppName()
{
    String strAppName;
#ifdef OS_WINCE
    String path = rho_native_rhopath();
    String_replace(path, '/', '\\');

    int nEnd = path.find_last_of('\\');
    nEnd = path.find_last_of('\\', nEnd-1)-1;

    int nStart = path.find_last_of('\\', nEnd) +1;
    strAppName = path.substr( nStart, nEnd-nStart+1);

#else
    strAppName = "Rhodes";
#endif

    return strAppName;
}

StringW CRhodesApp::getAppNameW()
{
    return convertToStringW( RHODESAPP().getAppName() );
}

String CRhodesApp::getAppTitle()
{
    String strTitle = RHOCONF().getString("title_text");
    if ( strTitle.length() == 0 )
        strTitle = getAppName();

    return strTitle;
}

const String& CRhodesApp::getStartUrl()
{
    m_strStartUrl = canonicalizeRhoUrl( RHOCONF().getString("start_path") );
    return m_strStartUrl;
}

boolean CRhodesApp::isOnStartPage()
{
    String strStart = getStartUrl();
    String strCurUrl = getCurrentUrl(0);

    if ( strStart.compare(strCurUrl) == 0 )
        return true;

    //check for index
    int nIndexLen = CHttpServer::isIndex(strStart);
    if ( nIndexLen > 0 && String_startsWith(strStart, strCurUrl) )
    {
        return strncmp(strStart.c_str(), strCurUrl.c_str(), strStart.length() - nIndexLen - 1) == 0;
    }

    nIndexLen = CHttpServer::isIndex(strCurUrl);
    if ( nIndexLen > 0 && String_startsWith(strCurUrl, strStart) )
    {
        return strncmp(strCurUrl.c_str(), strStart.c_str(), strCurUrl.length() - nIndexLen - 1) == 0;
    }

    return false;

}

const String& CRhodesApp::getBaseUrl()
{
    return m_strHomeUrl;
}

void CRhodesApp::setBaseUrl(const String& strBaseUrl)
{
    m_strHomeUrl = strBaseUrl + getFreeListeningPort();
}

const String& CRhodesApp::getOptionsUrl()
{
    m_strOptionsUrl = canonicalizeRhoUrl( RHOCONF().getString("options_path") );
    return m_strOptionsUrl;
}

const String& CRhodesApp::getRhobundleReloadUrl() 
{
    m_strRhobundleReloadUrl = RHOCONF().getString("rhobundle_zip_url");
    return m_strRhobundleReloadUrl;
}

void CRhodesApp::navigateToUrl( const String& strUrl)
{
    rho_webview_navigate(strUrl.c_str(), -1);
}

class CRhoSendLogCall
{
    String m_strCallback;
public:
    CRhoSendLogCall(const String& strCallback): m_strCallback(strCallback){}

    void run(common::CRhoThread &)
    {
        String strDevicePin = rho::sync::CClientRegister::getInstance() ? rho::sync::CClientRegister::getInstance()->getDevicePin() : "";
	    String strClientID = rho::sync::CSyncThread::getSyncEngine().readClientID();
        
        String strLogUrl = RHOCONF().getPath("logserver");
        if ( strLogUrl.length() == 0 )
            strLogUrl = RHOCONF().getPath("syncserver");
        
	    String strQuery = strLogUrl + "client_log?" +
            "client_id=" + strClientID + "&device_pin=" + strDevicePin + "&log_name=" + RHOCONF().getString("logname");
        
        net::CMultipartItem oItem;
        oItem.m_strFilePath = LOGCONF().getLogFilePath();
        oItem.m_strContentType = "application/octet-stream";
        
        boolean bOldSaveToFile = LOGCONF().isLogToFile();
        LOGCONF().setLogToFile(false);
        NetRequest oNetRequest;
        oNetRequest.setSslVerifyPeer(false);
        
        NetResponse resp = getNetRequest(&oNetRequest).pushMultipartData( strQuery, oItem, &(rho::sync::CSyncThread::getSyncEngine()), null );
        LOGCONF().setLogToFile(bOldSaveToFile);
        
        boolean isOK = true;
        
        if ( !resp.isOK() )
        {
            LOG(ERROR) + "send_log failed : network error - " + resp.getRespCode() + "; Body - " + resp.getCharData();
            isOK = false;
        }

        if (m_strCallback.length() > 0) 
        {
            const char* body = isOK ? "rho_callback=1&status=ok" : "rho_callback=1&status=error";

            rho_net_request_with_data(RHODESAPP().canonicalizeRhoUrl(m_strCallback).c_str(), body);
        }

        RHODESAPP().setSendingLog(false);
    }
};

boolean CRhodesApp::sendLog( const String& strCallbackUrl) 
{
    if ( m_bSendingLog )
        return true;

    m_bSendingLog = true;
    rho_rhodesapp_call_in_thread( new CRhoSendLogCall(strCallbackUrl) );
    return true;
}

String CRhodesApp::addCallbackObject(ICallbackObject* pCallbackObject, String strName)
{
    int nIndex = -1;
    for (int i = 0; i < (int)m_arCallbackObjects.size(); i++)
    {
        if ( m_arCallbackObjects.elementAt(i) == 0 )
            nIndex = i;
    }
    if ( nIndex  == -1 )
    {
        m_arCallbackObjects.addElement(pCallbackObject);
        nIndex = m_arCallbackObjects.size()-1;
    }else
        m_arCallbackObjects.setElementAt(pCallbackObject,nIndex);

    String strRes = "__rho_object[" + strName + "]=" + convertToStringA(nIndex);

    return strRes;
}

unsigned long CRhodesApp::getCallbackObject(int nIndex)
{
    if ( nIndex < 0 || nIndex > (int)m_arCallbackObjects.size() )
        return rho_ruby_get_NIL();

    ICallbackObject* pCallbackObject = m_arCallbackObjects.elementAt(nIndex);
    m_arCallbackObjects.setElementAt(0,nIndex);

    if ( !pCallbackObject )
        return rho_ruby_get_NIL();

    unsigned long valRes = pCallbackObject->getObjectValue();

    delete pCallbackObject;

    return valRes;
}

void CRhodesApp::setPushNotification(String strUrl, String strParams )
{
    synchronized(m_mxPushCallback)
    {
        m_strPushCallback = canonicalizeRhoUrl(strUrl);
        m_strPushCallbackParams = strParams;
    }
}

boolean CRhodesApp::callPushCallback(String strData)
{
    synchronized(m_mxPushCallback)
    {
        if ( m_strPushCallback.length() == 0 )
            return false;

        String strBody = strData + "&rho_callback=1";
        if ( m_strPushCallbackParams.length() > 0 )
            strBody += "&" + m_strPushCallbackParams;

        NetResponse resp = getNetRequest().pushData( m_strPushCallback, strBody, null );
        if (!resp.isOK())
            LOG(ERROR) + "Push notification failed. Code: " + resp.getRespCode() + "; Error body: " + resp.getCharData();
        else
        {
            const char* szData = resp.getCharData();
            return !(szData && strcmp(szData,"rho_push") == 0);
        }
    }

    return false;
}

void CRhodesApp::setScreenRotationNotification(String strUrl, String strParams)
{
    synchronized(m_mxScreenRotationCallback)
    {
        if (strUrl.length() > 0) {
            m_strScreenRotationCallback = canonicalizeRhoUrl(strUrl);
        }
        else {
            m_strScreenRotationCallback = "";
        }
        m_strScreenRotationCallbackParams = strParams;
    }
}

void CRhodesApp::callScreenRotationCallback(int width, int height, int degrees)
{
	synchronized(m_mxScreenRotationCallback) 
	{
		if (m_strScreenRotationCallback.length() == 0)
			return;
		
		String strBody = "rho_callback=1";
		
        strBody += "&width=";   strBody += convertToStringA(width);
		strBody += "&height=";  strBody += convertToStringA(height);
		strBody += "&degrees="; strBody += convertToStringA(degrees);
		
        if ( m_strScreenRotationCallbackParams.length() > 0 )
            strBody += "&" + m_strPushCallbackParams;
			
		NetResponse resp = getNetRequest().pushData( m_strScreenRotationCallback, strBody, null);
        if (!resp.isOK()) {
            LOG(ERROR) + "Screen rotation notification failed. Code: " + resp.getRespCode() + "; Error body: " + resp.getCharData();
        }
    }
}

void CRhodesApp::loadUrl(String url)
{
    if ( url.length() == 0 )
        return;

    boolean callback = false;
    if (String_startsWith(url, "callback:") )
    {
        callback = true;
        url = url.substr(9);
    }else if ( strcasecmp(url.c_str(), "exit")==0 || strcasecmp(url.c_str(), "close") == 0 )
    {
        rho_sys_app_exit();
        return;
    }else if ( strcasecmp(url.c_str(), "options")==0 )
    {
        rho_webview_navigate(getOptionsUrl().c_str(), 0);
        return;
    }else if ( strcasecmp(url.c_str(), "home")==0 )
    {
        rho_webview_navigate(getStartUrl().c_str(), 0);
        return;
    }else if ( strcasecmp(url.c_str(), "refresh")==0 )
    {
        rho_webview_refresh(0);
        return;
    }else if ( strcasecmp(url.c_str(), "back")==0 )
    {
        navigateBack();
        return;
    }else if ( strcasecmp(url.c_str(), "sync")==0 )
    {
        rho_sync_doSyncAllSources(1,"");
        return;
    }

    url = canonicalizeRhoUrl(url);
    if (callback)
    {
        getNetRequest().pushData( url,  "rho_callback=1", null );
    }
    else
        navigateToUrl(url);
}

void CRhodesApp::notifyLocalServerStarted()
{
    m_appCallbacksQueue->addQueueCommand(new CAppCallbacksQueue::Command(CAppCallbacksQueue::local_server_started));
}

} //namespace common
} //namespace rho

extern "C" {

using namespace rho;
using namespace rho::common;
unsigned long rho_rhodesapp_GetCallbackObject(int nIndex)
{
    return RHODESAPP().getCallbackObject(nIndex);
}

char* rho_http_normalizeurl(const char* szUrl) 
{
    rho::String strRes = RHODESAPP().canonicalizeRhoUrl(szUrl);
    return strdup(strRes.c_str());
}

void rho_http_free(void* data)
{
    free(data);
}
    
void rho_http_redirect( void* httpContext, const char* szUrl)
{
    HttpHeaderList headers;
    headers.push_back(HttpHeader("Location", szUrl));
    headers.push_back(HttpHeader("Content-Length", 0));
    headers.push_back(HttpHeader("Pragma", "no-cache"));
    headers.push_back(HttpHeader("Cache-Control", "must-revalidate"));
    headers.push_back(HttpHeader("Cache-Control", "no-cache"));
    headers.push_back(HttpHeader("Cache-Control", "no-store"));
    headers.push_back(HttpHeader("Expires", 0));
    headers.push_back(HttpHeader("Content-Type", "text/plain"));
    
    CHttpServer *serv = (CHttpServer *)httpContext;
    serv->send_response(serv->create_response("301 Moved Permanently", headers), true);
}

void rho_http_senderror(void* httpContext, int nError, const char* szMsg)
{
    char buf[30];
    snprintf(buf, sizeof(buf), "%d", nError);
    
    rho::String reason = buf;
    reason += " ";
    reason += szMsg;
    
    CHttpServer *serv = (CHttpServer *)httpContext;
    serv->send_response(serv->create_response(reason));
}

void rho_http_sendresponse(void* httpContext, const char* szBody)
{
    size_t nBodySize = strlen(szBody);
    
    HttpHeaderList headers;
    headers.push_back(HttpHeader("Content-Length", nBodySize));
    headers.push_back(HttpHeader("Pragma", "no-cache"));
    headers.push_back(HttpHeader("Cache-Control", "no-cache"));
    headers.push_back(HttpHeader("Expires", 0));
    
    const char *fmt = "%a, %d %b %Y %H:%M:%S GMT";
    char date[64], lm[64], etag[64];
    time_t	_current_time = time(0);
    strftime(date, sizeof(date), fmt, localtime(&_current_time));
    strftime(lm, sizeof(lm), fmt, localtime(&_current_time));
    snprintf(etag, sizeof(etag), "\"%lx.%lx\"", (unsigned long)_current_time, (unsigned long)nBodySize);
    
    headers.push_back(HttpHeader("Date", date));
    headers.push_back(HttpHeader("Last-Modified", lm));
    headers.push_back(HttpHeader("Etag", etag));
    
    CHttpServer *serv = (CHttpServer *)httpContext;
    serv->send_response(serv->create_response("200 OK", headers, szBody));
}

int	rho_http_snprintf(char *buf, size_t buflen, const char *fmt, ...)
{
	va_list		ap;
	int		n;
		
	if (buflen == 0)
		return (0);
		
	va_start(ap, fmt);
	n = vsnprintf(buf, buflen, fmt, ap);
	va_end(ap);
		
	if (n < 0 || (size_t) n >= buflen)
		n = buflen - 1;
	buf[n] = '\0';
		
	return (n);
}
	
void rho_rhodesapp_create(const char* szRootPath)
{
    rho::common::CRhodesApp::Create(szRootPath, szRootPath, szRootPath);
}

void rho_rhodesapp_create_with_separate_user_path(const char* szRootPath, const char* szUserPath)
{
    rho::common::CRhodesApp::Create(szRootPath, szUserPath, szRootPath);
}

void rho_rhodesapp_create_with_separate_runtime(const char* szRootPath, const char* szRuntimePath)
{
    rho::common::CRhodesApp::Create(szRootPath, szRootPath, szRuntimePath);
}
    
    
void rho_rhodesapp_start()
{
    RHODESAPP().startApp();
}
    
void rho_rhodesapp_destroy()
{
    rho::common::CRhodesApp::Destroy();
}
/*
const char* rho_rhodesapp_getfirststarturl()
{
    return RHODESAPP().getFirstStartUrl().c_str();
}*/

const char* rho_rhodesapp_getstarturl()
{
    return RHODESAPP().getStartUrl().c_str();
}

const char* rho_rhodesapp_gethomeurl()
{
    return RHODESAPP().getHomeUrl().c_str();
}

const char* rho_rhodesapp_getoptionsurl()
{
    return RHODESAPP().getOptionsUrl().c_str();
}

void rho_rhodesapp_keeplastvisitedurl(const char* szUrl)
{
    RHODESAPP().keepLastVisitedUrl(szUrl);
}

const char* rho_rhodesapp_getcurrenturl(int index)
{
    return RHODESAPP().getCurrentUrl(index).c_str();
}

const char* rho_rhodesapp_getloadingpagepath()
{
    return RHODESAPP().getLoadingPagePath().c_str();
}

const char* rho_rhodesapp_getblobsdirpath()
{
    return RHODESAPP().getBlobsDirPath().c_str();
}
    
const char* rho_rhodesapp_getuserrootpath()
{
    return RHODESAPP().getRhoUserPath().c_str();
}
    

const char* rho_rhodesapp_getdbdirpath()
{
    return RHODESAPP().getDBDirPath().c_str();
}

const char* rho_rhodesapp_getapprootpath() {
    return RHODESAPP().getAppRootPath().c_str();
}


void rho_rhodesapp_navigate_back()
{
    RHODESAPP().navigateBack();
}

void rho_rhodesapp_callCameraCallback(const char* strCallbackUrl, const char* strImagePath, 
    const char* strError, int bCancel )
{
    RHODESAPP().callCameraCallback(strCallbackUrl, strImagePath, strError, bCancel != 0);
}

void rho_rhodesapp_callSignatureCallback(const char* strCallbackUrl, const char* strSignaturePath, 
									  const char* strError, int bCancel )
{
	RHODESAPP().callSignatureCallback(strCallbackUrl, strSignaturePath, strError, bCancel != 0);
}
	
void rho_rhodesapp_callDateTimeCallback(const char* strCallbackUrl, long lDateTime, const char* szData, int bCancel )
{
    RHODESAPP().callDateTimeCallback(strCallbackUrl, lDateTime, szData, bCancel != 0);
}

void rho_rhodesapp_callBluetoothCallback(const char* strCallbackUrl, const char* body) {
	RHODESAPP().callBluetoothCallback(strCallbackUrl, body);
}

void rho_rhodesapp_callPopupCallback(const char *strCallbackUrl, const char *id, const char *title)
{
    RHODESAPP().callPopupCallback(strCallbackUrl, id, title);
}

void rho_rhodesapp_callAppActiveCallback(int nActive)
{
	if ( rho::common::CRhodesApp::getInstance() )
		RHODESAPP().callAppActiveCallback(nActive!=0);
}

void rho_rhodesapp_callUiCreatedCallback()
{
    if ( rho::common::CRhodesApp::getInstance() )
        RHODESAPP().callUiCreatedCallback();
}

void rho_rhodesapp_callUiDestroyedCallback()
{
    if ( rho::common::CRhodesApp::getInstance() )
        RHODESAPP().callUiDestroyedCallback();
}

void rho_rhodesapp_setViewMenu(unsigned long valMenu)
{
    RHODESAPP().getAppMenu().setAppMenu(valMenu);
}

const char* rho_rhodesapp_getappbackurl()
{
    return RHODESAPP().getAppBackUrl().c_str();
}

int rho_rhodesapp_callPushCallback(const char* szData)
{
    if ( !rho::common::CRhodesApp::getInstance() )
        return 1;

    return RHODESAPP().callPushCallback(szData?szData:"") ? 1 : 0;
}

void rho_rhodesapp_callScreenRotationCallback(int width, int height, int degrees)
{
    if ( !rho::common::CRhodesApp::getInstance() )
        return;
	RHODESAPP().callScreenRotationCallback(width, height, degrees);
}

const char* rho_ruby_getErrorText(int nError)
{
    return RHODESAPP().getRhoMessage( nError, "").c_str();
}

const char* rho_ruby_getMessageText(const char* szName)
{
    return RHODESAPP().getRhoMessage( 0, szName).c_str();
}

int rho_rhodesapp_isrubycompiler()
{
    return 0;
}

int rho_conf_send_log(const char* callback_url)
{
    rho::String s_callback_url = "";
    if (callback_url != NULL) {
        s_callback_url = callback_url;
    }
    return RHODESAPP().sendLog(s_callback_url);
}

void rho_net_request(const char *url)
{
    getNetRequest().pullData(url, null);
}

void rho_net_request_with_data(const char *url, const char *str_body) 
{
    getNetRequest().pushData(url, str_body, null);
}
	
void rho_rhodesapp_load_url(const char *url)
{
    RHODESAPP().loadUrl(url);
}

int rho_rhodesapp_check_mode()
{
    if (RHODESAPP().deactivationMode())
    {
        LOG(ERROR) + "Operation is not allowed in 'deactivation' mode";
        return 0;
    }
    return 1;
}

#if defined(OS_ANDROID) && defined(RHO_LOG_ENABLED)
int rho_log(const char *fmt, ...)
{
  va_list vl;
  va_start(vl, fmt);
  int ret = __android_log_vprint(ANDROID_LOG_INFO, "RhoLog", fmt, vl);
  va_end(vl);
  return ret;
}

unsigned long long rho_cur_time()
{
    timeval tv;
    gettimeofday(&tv, NULL);
    return ((unsigned long long)tv.tv_sec)*1000000 + tv.tv_usec;
}
#endif

void rho_free_callbackdata(void* pData)
{
	//It is used in SyncClient.
}

int rho_rhodesapp_canstartapp(const char* szCmdLine, const char* szSeparators)
{
    String strCmdLineSecToken;
    String security_key = "security_token=";
    String strCmdLine = szCmdLine ? szCmdLine : "";

    CRhodesApp::setStartParameters(szCmdLine);
    RAWLOGC_INFO1("RhodesApp", "New start params: %s", strCmdLine.c_str());

    CRhodesApp::setSecurityTokenNotPassed(false);
    
	const char* szAppSecToken = get_app_build_config_item("security_token");
    if ( !szAppSecToken || !*szAppSecToken)
        return 1;

	int skpos = strCmdLine.find(security_key);
	if ((String::size_type)skpos != String::npos) 
    {
		String tmp = strCmdLine.substr(skpos+security_key.length(), strCmdLine.length() - security_key.length() - skpos);

		int divider = tmp.find_first_of(szSeparators);
		if ((String::size_type)divider != String::npos)
			strCmdLineSecToken = tmp.substr(0, divider);
		else
			strCmdLineSecToken = tmp;
	}
    int result = strCmdLineSecToken.compare(szAppSecToken) != 0 ? 0 : 1; 
    CRhodesApp::setSecurityTokenNotPassed(!result);

    return result; 
}

} //extern "C"
