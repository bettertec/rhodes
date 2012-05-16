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

#pragma once

#include "common/RhoStd.h"
#include "logging/RhoLog.h"
#include "common/AutoPointer.h"
#include "common/IRhoClassFactory.h"
#include "net/INetRequest.h"

typedef int (*RHOC_CALLBACK)(const char* szNotify, void* callback_data);

namespace rho {
namespace db {
    class CDBAdapter;
}

namespace sync {
class CSyncEngine;
class CSyncSource;

struct CSyncNotification
{
    String m_strUrl, m_strParams;
    RHOC_CALLBACK m_cCallback;
    void*         m_cCallbackData;

    boolean m_bRemoveAfterFire;
    CSyncNotification(): m_cCallback(0), m_cCallbackData(0), m_bRemoveAfterFire(false){}

    CSyncNotification(String strUrl, String strParams, boolean bRemoveAfterFire);
    CSyncNotification(RHOC_CALLBACK callback, void* callback_data, boolean bRemoveAfterFire) : 
        m_cCallback(callback), m_cCallbackData(callback_data), m_bRemoveAfterFire(false){}

    String toString()const;

	~CSyncNotification();
};

struct CObjectNotification
{
    String m_strUrl;
    RHOC_CALLBACK m_cCallback;
    void*         m_cCallbackData;
    
    CObjectNotification(): m_cCallback(0), m_cCallbackData(0){}
    
    CObjectNotification(String strUrl);
    CObjectNotification(RHOC_CALLBACK callback, void* callback_data) : 
        m_cCallback(callback), m_cCallbackData(callback_data){}
    
    String toString()const;
    
    ~CObjectNotification();
};
    
class CSyncNotify
{
    DEFINE_LOGCLASS;

public:
    enum ENotifyType{ enNone, enDelete, enUpdate, enCreate };

private:

    CSyncEngine& m_syncEngine;

    static common::CAutoPtr<CObjectNotification> m_pObjectNotify;
    HashtablePtr<int, Hashtable<String,int>* > m_hashSrcIDAndObject;
    HashtablePtr<int, Hashtable<String,String>* > m_hashCreateObjectErrors;
    String m_strSingleObjectSrcName, m_strSingleObjectID;
    Hashtable<int,int> m_hashSrcObjectCount;
    boolean m_bEnableReporting;
    boolean m_bEnableReportingGlobal;

    static common::CMutex m_mxObjectNotify;

    HashtablePtr<int,CSyncNotification*> m_mapSyncNotifications;
    common::CAutoPtr<CSyncNotification> m_pAllNotification;
    common::CAutoPtr<CSyncNotification> m_pSearchNotification;
    CSyncNotification m_emptyNotify;
    common::CMutex m_mxSyncNotifications;
    Vector<String> m_arNotifyBody;
    String m_strStatusHide;
    boolean m_bFakeServerResponse;
    boolean m_isInsideCallback;

   	NetRequest     m_NetRequest;

    net::CNetRequestWrapper getNet(){ return getNetRequest(&m_NetRequest); }
    CSyncEngine& getSync(){ return m_syncEngine; }
public:
    CSyncNotify( CSyncEngine& syncEngine ) : m_syncEngine(syncEngine), m_bEnableReporting(false), 
        m_bEnableReportingGlobal(false), m_bFakeServerResponse(false), m_isInsideCallback(false) {}

    //Object notifications
    void fireObjectsNotification();
    void onObjectChanged(int nSrcID, const String& strObject, int nType);
    void addCreateObjectError(int nSrcID, const String& strObject, const String& strError);

    void addObjectNotify(int nSrcID, const String& strObject );
    void addObjectNotify(const String& strSrcName, const String& strObject );
    static void setObjectNotification(CObjectNotification* pNotify);
    static CObjectNotification* getObjectNotification();
    void cleanObjectNotifications();
    void cleanCreateObjectErrors();

    //Sync notifications
    void setSyncNotification(int source_id, CSyncNotification* pNotify);
    void setSearchNotification(CSyncNotification* pNotify);

    void clearSyncNotification(int source_id);

    void onSyncSourceEnd( int nSrc, VectorPtr<CSyncSource*>& sources );
    void fireSyncNotification( CSyncSource* psrc, boolean bFinish, int nErrCode, String strMessage);
    void fireSyncNotification2( CSyncSource* src, boolean bFinish, int nErrCode, String strServerError);

    void fireBulkSyncNotification( boolean bFinish, String status, String partition, int nErrCode );

    void cleanLastSyncObjectCount();
    int incLastSyncObjectCount(int nSrcID);
    int getLastSyncObjectCount(int nSrcID);

    void callLoginCallback(const CSyncNotification& oNotify, int nErrCode, String strMessage);

    boolean isReportingEnabled(){return m_bEnableReporting&&m_bEnableReportingGlobal;}
    void enableReporting(boolean bEnable){m_bEnableReporting = bEnable;}
    void enableStatusPopup(boolean bEnable){m_bEnableReportingGlobal = bEnable;}

    const String& getNotifyBody();
    void cleanNotifyBody(){ m_arNotifyBody.removeAllElements(); setFakeServerResponse(false); }
    boolean isFakeServerResponse(){ return m_bFakeServerResponse; }
    void setFakeServerResponse(boolean bFakeServerResponse){ m_bFakeServerResponse = bFakeServerResponse; }

    void fireAllSyncNotifications( boolean bFinish, int nErrCode, String strError, String strServerError );
    void reportSyncStatus(String status, int error, String strDetails);
    void showStatusPopup(const String& status);
    
    bool isInsideCallback() const { return m_isInsideCallback; }
private:
    CSyncNotification* getSyncNotifyBySrc(CSyncSource* src);

    String makeCreateObjectErrorBody(int nSrcID);
    void processSingleObject();

    void doFireSyncNotification( CSyncSource* src, boolean bFinish, int nErrCode, String strError, String strParams, String strServerError);

    boolean callNotify(const CSyncNotification& oNotify, const String& strBody );

    void clearNotification(CSyncSource* src);

};

}
}

extern "C" void alert_show_status(const char* title, const char* message, const char* szHide);
