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
#include "db/DBResult.h"
#include "logging/RhoLog.h"

namespace rho {

namespace db {
    class CDBAdapter;
}
namespace net {
    struct CMultipartItem;
    class CNetRequestWrapper;
}
namespace json {
    class CJSONEntry;
    class CJSONArrayIterator;
    class CJSONStructIterator;
}

namespace sync {
struct ISyncProtocol;

class CAttrValue
{
public:
	String m_strAttrib;
    String m_strValue;
    String m_strBlobSuffix;

    CAttrValue(const String& strAttrib, const String& strValue);
};

class CSyncEngine;
class CSyncNotify;
class CSyncSource
{
    DEFINE_LOGCLASS;

    CSyncEngine& m_syncEngine;
    db::CDBAdapter& m_dbAdapter;

    int    m_nID;
    String m_strName;
    uint64 m_token;
    String m_strSyncType;
    boolean m_bTokenFromDB;

    int m_nCurPageCount, m_nInserted, m_nDeleted, m_nTotalCount;
    int m_nRefreshTime;
    int m_nProgressStep;
    boolean m_bSchemaSource;
public:
    struct CAssociation
    {
        String m_strSrcName, m_strAttrib;
        CAssociation( String strSrcName, String strAttrib ){m_strSrcName = strSrcName; m_strAttrib = strAttrib; }
    };
private:
    Vector<CAssociation> m_arAssociations;
    VectorPtr<net::CMultipartItem*> m_arMultipartItems;
    Vector<String>                  m_arBlobAttrs;
    Hashtable<String,int>           m_hashIgnorePushObjects;
    Hashtable<String,int>           m_hashBelongsTo;

public:
    int m_nErrCode;
    String m_strError;
    String m_strQueryParams;

public:
    CSyncSource(int id, const String& strName, const String& strSyncType, db::CDBAdapter& db, CSyncEngine& syncEngine );
    virtual void sync();
    virtual void syncClientChanges();

    int getID()const { return m_nID; }
    String getName() { return m_strName; }
    String getSyncType(){ return m_strSyncType; }
    int getErrorCode(){ return m_nErrCode; }

    int getServerObjectsCount()const{ return m_nInserted+m_nDeleted; }

    uint64 getToken()const{ return m_token; }
    boolean isTokenFromDB(){ return m_bTokenFromDB; }
    void setToken(uint64 token){ m_token = token; m_bTokenFromDB = false; }
    virtual boolean isEmptyToken()
    {
        return m_token == 0;
    }

    int getProgressStep(){ return m_nProgressStep; }
    void setProgressStep(int nProgressStep){ m_nProgressStep = nProgressStep; }

    int getRefreshTime(){ return m_nRefreshTime; }

    Vector<CAssociation>& getAssociations(){ return m_arAssociations; }
//private:
    //CSyncSource();
    CSyncSource(CSyncEngine& syncEngine, db::CDBAdapter& db );

    void doSyncClientChanges();
    void checkIgnorePushObjects();
    int    getBelongsToSrcID(const String& strAttrib);
    void   addBelongsTo(const String& strAttrib, int nSrcID);

    //boolean isPendingClientChanges();

    void syncServerChanges();
    void makePushBody_Ver3(String& strBody, const String& strUpdateType, boolean isSync);

    void processToken(uint64 token);

    int getInsertedCount()const { return m_nInserted; }
    int getDeletedCount()const { return m_nDeleted; }
    void setCurPageCount(int nCurPageCount){m_nCurPageCount = nCurPageCount;}
    void setTotalCount(int nTotalCount){m_nTotalCount = nTotalCount;}
    int  getCurPageCount(){return m_nCurPageCount;}
    int  getTotalCount(){return m_nTotalCount;}

    void processServerResponse_ver3(json::CJSONArrayIterator& oJsonArr);
    void processServerCmd_Ver3(const String& strCmd, const String& strObject, const String& strAttrib, const String& strValue, boolean bCheckUIRequest);//throws Exception

    String makeFileName(const CAttrValue& value);//throws Exception
    boolean downloadBlob(CAttrValue& value);//throws Exception
    boolean processBlob( const String& strCmd, const String& strObject, CAttrValue& oAttrValue );
	boolean processAllBlobs();
	boolean processServerBlobAttrs();

    void setRefreshTime( int nRefreshTime ){ m_nRefreshTime = nRefreshTime;}

    db::CDBAdapter& getDB(){ return m_dbAdapter; }

    void processSyncCommand(const String& strCmd, json::CJSONEntry oCmdEntry, boolean bCheckUIRequest );

    void processServerCmd_Ver3_Schema(const String& strCmd, const String& strObject, json::CJSONStructIterator& attrIter, boolean bCheckUIRequest);//throws Exception

    void parseAssociations(const String& strAssociations);
    void processAssociations(const String& strOldObject, const String& strNewObject);
    void updateAssociation(const String& strOldObject, const String& strNewObject, const String& strAttrib);

    void applyChangedValues();
    void processServerErrors(json::CJSONEntry& oCmds);

    void checkProgressStepNotify(boolean bEndTransaction);
    boolean checkFreezedProps(String strProp);
	
	bool haveChangedValues();

private:
    CSyncEngine& getSync(){ return m_syncEngine; }
    CSyncNotify& getNotify();
    net::CNetRequestWrapper getNet();
    ISyncProtocol& getProtocol();
	
};

}
}
