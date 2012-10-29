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
package com.rho.sync;

import com.rho.RhoClassFactory;
import com.rho.RhoConf;
import com.rho.RhoEmptyLogger;
import com.rho.RhoEmptyProfiler;
import com.rho.RhoLogger;
import com.rho.RhoProfiler;
import com.rho.RhoAppAdapter;
import com.rho.TimeInterval;
import com.rho.db.*;
import com.rho.file.IFileAccess;
import com.rho.net.*;
import com.rho.*;
import java.io.IOException;
import java.util.Vector;
import java.util.Hashtable;

public class SyncEngine implements NetRequest.IRhoSession
{
	private static final RhoLogger LOG = RhoLogger.RHO_STRIP_LOG ? new RhoEmptyLogger() : 
		new RhoLogger("Sync");
	private static final RhoProfiler PROF = RhoProfiler.RHO_STRIP_PROFILER ? new RhoEmptyProfiler() : 
		new RhoProfiler();
	RhoConf RHOCONF(){ return RhoConf.getInstance(); }
	
    public static final int esNone = 0, esSyncAllSources = 1, esSyncSource = 2, esSearch=3, esStop = 4, esExit = 5;
    public static final int ebsNotSynced = 0, ebsSynced = 1, ebsLoadBlobs = 2;    
    //public static final String RHO_SETTING_BULKSYNC_STATE  = "bulksync_state";
    
    static class SourceID
    {
        String m_strName = "";
        int m_nID;

        SourceID(int id, String strName ){ m_nID = id; m_strName = strName; }
        SourceID(String strName ){ m_strName = strName; }
        
        public String toString()
        {
            if ( m_strName.length() > 0 )
                return "name : " + m_strName;

            return "# : " + m_nID;
        }
        boolean isEqual(SyncSource src)
        {
            if ( m_strName.length() > 0 )
                return src.getName().equals(m_strName);

            return m_nID == src.getID().intValue();
        }
    };
    
    static class SourceOptions
    {
        private Mutex m_mxSrcOptions = new Mutex();
        private Hashtable/*Ptr<int, Hashtable<String,String>* >*/ m_hashSrcOptions = new Hashtable();
        
    	public void setProperty(Integer nSrcID, String szPropName, String szPropValue)
    	{
    	    synchronized(m_mxSrcOptions)
    	    {
    	        Hashtable/*<String,String>* */phashOptions = (Hashtable)m_hashSrcOptions.get(nSrcID);
    	        if ( phashOptions == null )
    	        {
    	            phashOptions = new Hashtable/*<String,String>*/();
    	            m_hashSrcOptions.put( nSrcID, phashOptions );
    	        }

    	        //Hashtable<String,String>& hashOptions = *phashOptions;
    	        phashOptions.put(szPropName,szPropValue!=null?szPropValue:"");
    	    }
    	}
        public String getProperty(Integer nSrcID, String szPropName)
        {
            String res = "";
            synchronized(m_mxSrcOptions)
            {
                Hashtable/*<String,String>* */phashOptions = (Hashtable)m_hashSrcOptions.get(nSrcID);
                if ( phashOptions != null )
                {
                    //Hashtable<String,String>& hashOptions = *phashOptions;
                    res = (String)phashOptions.get(szPropName);
                }
            }

            return res != null ? res : "";
        }
        public boolean getBoolProperty(Integer nSrcID, String szPropName)
        {
            String strValue = getProperty(nSrcID, szPropName);
            return strValue.compareTo("1") == 0 || strValue.compareTo("true") == 0 ? true : false;
        	
        }
        
        public int getIntProperty(Integer nSrcID, String szPropName)
        {
            String strValue = getProperty(nSrcID, szPropName);

            return strValue != null && strValue.length() > 0 ? Integer.valueOf(strValue).intValue() : 0;
        }
        
        void clearProperties()
        {
        	synchronized(m_mxSrcOptions)
        	{
        		m_hashSrcOptions.clear();
        	}
        }
        
    };
    
    Vector/*<SyncSource*>*/ m_sources = new Vector();
    NetRequest m_NetRequest, m_NetRequestClientID;
    ISyncProtocol m_SyncProtocol;
    int         m_syncState;
    String     m_clientID = "";
    Mutex m_mxLoadClientID = new Mutex();
    String m_strSession = "";
    SyncNotify m_oSyncNotify = new SyncNotify(this);
    boolean m_bStopByUser = false;
    int m_nSyncPageSize = 2000;
    boolean m_bNoThreaded = false;
    int m_nErrCode = RhoAppAdapter.ERR_NONE;
    String m_strError = "",m_strServerError = "";
    boolean m_bIsSearch, m_bIsSchemaChanged;
    static SourceOptions m_oSourceOptions = new SourceOptions();
    
    void setState(int eState){ m_syncState = eState; }
    int getState(){ return m_syncState; }
    boolean isSearch(){ return m_bIsSearch; }
    boolean isContinueSync(){ return m_syncState != esExit && m_syncState != esStop; }
	boolean isSyncing(){ return m_syncState == esSyncAllSources || m_syncState == esSyncSource || m_syncState == esSearch; }
    void stopSync()
    { 
    	if (isContinueSync())
    	{ 
	    	setState(esStop); 
	    	if (m_NetRequest!=null) 
	    		m_NetRequest.cancel(); 
	    	
	    	if (m_NetRequestClientID!=null) 
	    		m_NetRequestClientID.cancel(); 
    	} 
    }
    void stopSyncByUser(){ m_bStopByUser = true; stopSync(); }
    void exitSync()
    { 
    	setState(esExit); 
    	if (m_NetRequest!=null) 
    		m_NetRequest.cancel(); 
    	
    	if (m_NetRequestClientID!=null) 
    		m_NetRequestClientID.cancel(); 
    }
    boolean isStoppedByUser(){ return m_bStopByUser; }
    
    String getClientID(){ return m_clientID; }
    void setSession(String strSession){m_strSession=strSession;}
    boolean isSessionExist(){ return m_strSession != null && m_strSession.length() > 0; }
    
    void setSchemaChanged(boolean bChanged){ m_bIsSchemaChanged = bChanged; }
    boolean isSchemaChanged(){ return m_bIsSchemaChanged; }
    
    DBAdapter getUserDB(){ return DBAdapter.getUserDB(); }
    DBAdapter getDB(String strPartition){ return DBAdapter.getDB(strPartition); }
    
  //IRhoSession
    public String getSession(){ return m_strSession; }
    public String getContentType(){ return getProtocol().getContentType();}
    
    SyncNotify getNotify(){ return m_oSyncNotify; }
    NetRequest getNet() { return m_NetRequest;}
    NetRequest getNetClientID(){ return m_NetRequestClientID; }
    ISyncProtocol getProtocol(){ return m_SyncProtocol; }
    
    boolean isNoThreadedMode(){ return m_bNoThreaded; }
    void setNonThreadedMode(boolean b){m_bNoThreaded = b;}
    static SourceOptions getSourceOptions(){ return m_oSourceOptions; }
    
    SyncEngine(){
		m_NetRequest = null;
		m_NetRequestClientID = null;
    	m_syncState = esNone;
    	
    	initProtocol();
    }

    void initProtocol()
    {
        m_SyncProtocol = new SyncProtocol_3();
    }
    
    int getSyncPageSize() { return m_nSyncPageSize; }
    void setSyncPageSize(int nPageSize){ m_nSyncPageSize = nPageSize; }
    
    void setFactory(RhoClassFactory factory)throws Exception{ 
		m_NetRequest = RhoClassFactory.createNetRequest();
		m_NetRequestClientID = RhoClassFactory.createNetRequest();
		
		m_oSyncNotify.setFactory(factory);		
    }
    
    void prepareSync(int eState, SourceID oSrcID)throws Exception
    {
        setState(eState);
        m_bIsSearch =  eState == esSearch;
        m_bStopByUser = false;
        m_nErrCode = RhoAppAdapter.ERR_NONE;
        m_strError = "";
        m_strServerError = "";
        m_bIsSchemaChanged = false;
        
        loadAllSources();

        m_strSession = loadSession();
        if ( isSessionExist()  )
        {
            m_clientID = loadClientID();
            if ( m_nErrCode == RhoAppAdapter.ERR_NONE )
            {
                getNotify().cleanLastSyncObjectCount();
       	        doBulkSync();

                return;
            }
        }else
            m_nErrCode = RhoAppAdapter.ERR_CLIENTISNOTLOGGEDIN;
        
        SyncSource src = null;
        if ( oSrcID != null )
        	src = findSource(oSrcID);
        
    	if ( src != null )
    	{
            src.m_nErrCode = m_nErrCode;
            src.m_strError = m_strError;
            getNotify().fireSyncNotification(src, true, src.m_nErrCode, "");
        }else
        {
            getNotify().fireAllSyncNotifications(true, m_nErrCode, m_strError, "");
        }
        
        stopSync();
    }
    
    void doSyncAllSources(String strQueryParams, boolean bSyncOnlyChangedSources)
    {
	    try
	    {
	        prepareSync(esSyncAllSources, null);
	
	        if ( isContinueSync() )
	        {
			    PROF.CREATE_COUNTER("Net");	    
			    PROF.CREATE_COUNTER("Parse");
			    PROF.CREATE_COUNTER("DB");
			    PROF.CREATE_COUNTER("Data");
			    PROF.CREATE_COUNTER("Data1");
			    PROF.CREATE_COUNTER("Pull");
			    PROF.START("Sync");
	
	            syncAllSources(strQueryParams, bSyncOnlyChangedSources);
	
			    PROF.DESTROY_COUNTER("Net");	    
			    PROF.DESTROY_COUNTER("Parse");
			    PROF.DESTROY_COUNTER("DB");
			    PROF.DESTROY_COUNTER("Data");
			    PROF.DESTROY_COUNTER("Data1");
			    PROF.DESTROY_COUNTER("Pull");
			    PROF.STOP("Sync");
	        }
	
	        getNotify().cleanCreateObjectErrors();
	    }catch(Exception exc)
	    {
	    	LOG.ERROR("Sync failed.", exc);
	    }

        if ( getState() != esExit )
            setState(esNone);
    }

    void doSearch(Vector/*<rho::String>*/ arSources, String strParams, String strAction, boolean bSearchSyncChanges, int nProgressStep)
    {
	    try
	    {
		    prepareSync(esSearch, null);
		    if ( !isContinueSync() )
		    {
		        if ( getState() != esExit )
		            setState(esNone);
		
		        return;
		    }
		
		    TimeInterval startTime = TimeInterval.getCurrentTime();
		
		    if ( bSearchSyncChanges )
		    {
		        for ( int i = 0; i < (int)arSources.size(); i++ )
		        {
		            SyncSource pSrc = findSourceByName((String)arSources.elementAt(i));
		            if ( pSrc != null )
		                pSrc.syncClientChanges();
		        }
		    }
		
		    while( isContinueSync() )
		    {
		        int nSearchCount = 0;
		        String strUrl = getProtocol().getServerQueryUrl(strAction);
		        String strQuery = getProtocol().getServerQueryBody("", getClientID(), getSyncPageSize());
		
		        if ( strParams.length() > 0 )
		            strQuery += strParams;
		
		        String strTestResp = "";
		        for ( int i = 0; i < (int)arSources.size(); i++ )
		        {
		            SyncSource pSrc = findSourceByName((String)arSources.elementAt(i));
		            if ( pSrc != null )
		            {
		                strQuery += "&sources[][name]=" + pSrc.getName();
		
		                if ( !pSrc.isTokenFromDB() && pSrc.getToken() > 1 )
		                    strQuery += "&sources[][token]=" + pSrc.getToken();
		                
		                strTestResp = getSourceOptions().getProperty(pSrc.getID(), "rho_server_response");
		            }
		        }
		
				LOG.INFO("Call search on server. Url: " + (strUrl+strQuery) );
		        NetResponse resp = getNet().pullData(strUrl+strQuery, this);
		
		        if ( !resp.isOK() )
		        {
		            stopSync();
		            m_nErrCode = RhoAppAdapter.getErrorFromResponse(resp);
		            m_strError = resp.getCharData();
		            continue;
		        }
		
		        String szData = null;
		        if ( strTestResp != null && strTestResp.length() > 0 )
		        {
		        	szData = strTestResp;
		        	getNotify().setFakeServerResponse(true);
		        }
		        else
		        	szData = resp.getCharData();		        
		
		        JSONArrayIterator oJsonArr = new JSONArrayIterator(szData);
		
		        for( ; !oJsonArr.isEnd() && isContinueSync(); oJsonArr.next() )
		        {
		            JSONArrayIterator oSrcArr = oJsonArr.getCurArrayIter();//new JSONArrayIterator(oJsonArr.getCurItem());
		            if (oSrcArr.isEnd())
		            	break;
		            
		            int nVersion = 0;
		            if ( !oSrcArr.isEnd() && oSrcArr.getCurItem().hasName("version") )
		            {
		                nVersion = oSrcArr.getCurItem().getInt("version");
		                oSrcArr.next();
		            }
		
		            if ( nVersion != getProtocol().getVersion() )
		            {
		                LOG.ERROR( "Sync server send search data with incompatible version. Client version: " + getProtocol().getVersion() +
		                    "; Server response version: " + nVersion );
		                stopSync();
		                m_nErrCode = RhoAppAdapter.ERR_SYNCVERSION;
		                continue;
		            }
		
		            if ( !oSrcArr.isEnd() && oSrcArr.getCurItem().hasName("token"))
		            {
		                oSrcArr.next();
		            }
		            
		            if ( !oSrcArr.getCurItem().hasName("source") )
		            {
		                LOG.ERROR( "Sync server send search data without source name." );
		                stopSync();
		                m_nErrCode = RhoAppAdapter.ERR_UNEXPECTEDSERVERRESPONSE;
		                m_strError = szData;
		                continue;
		            }
		
		            String strSrcName = oSrcArr.getCurItem().getString("source");
		            SyncSource pSrc = findSourceByName(strSrcName);
		            if ( pSrc == null )
		            {
		                LOG.ERROR("Sync server send search data for unknown source name:" + strSrcName);
		                stopSync();
		                m_nErrCode = RhoAppAdapter.ERR_UNEXPECTEDSERVERRESPONSE;
		                m_strError = szData;
		                continue;
		            }
		
		            oSrcArr.reset(0);
		            pSrc.setProgressStep(nProgressStep);
		            pSrc.processServerResponse_ver3(oSrcArr);
		
		            nSearchCount += pSrc.getCurPageCount();
		        }
		
		        if ( nSearchCount == 0 )
		        {
		        	for ( int i = 0; i < (int)arSources.size(); i++ )
					{
						SyncSource pSrc = findSourceByName((String)arSources.elementAt(i));
						if ( pSrc != null )
							pSrc.processToken(0);
					}		        	
		            break;
		        }
		        
		        if ( strTestResp!= null && strTestResp.length() > 0 )
		        	break;		        
		    }  
		
		    getNotify().fireAllSyncNotifications(true, m_nErrCode, m_strError, m_strServerError);
		
		    //update db info
		    TimeInterval endTime = TimeInterval.getCurrentTime();
		    //unsigned long timeUpdated = CLocalTime().toULong();
		    for ( int i = 0; i < (int)arSources.size(); i++ )
		    {
		        SyncSource oSrc = findSourceByName((String)arSources.elementAt(i));
		        if ( oSrc == null )
		            continue;
		        oSrc.getDB().executeSQL("UPDATE sources set last_updated=?,last_inserted_size=?,last_deleted_size=?, "+
					 "last_sync_duration=?,last_sync_success=?, backend_refresh_time=? WHERE source_id=?", 
					 new Long(endTime.toULong()/1000), new Integer(oSrc.getInsertedCount()), new Integer(oSrc.getDeletedCount()), 
					 new Long((endTime.minus(startTime)).toULong()), new Integer(oSrc.m_nErrCode == RhoAppAdapter.ERR_NONE?1:0), 
					 new Integer(oSrc.getRefreshTime()),  oSrc.getID() );
		    }
		    //
		
		    getNotify().cleanCreateObjectErrors();
		    if ( getState() != esExit )
		        setState(esNone);

	    } catch(Exception exc) {
    		LOG.ERROR("Search failed.", exc);

    		getNotify().fireAllSyncNotifications(true, RhoAppAdapter.ERR_RUNTIME, "", "");
	    }
		    
    }

    void doSyncSource(SourceID oSrcID, String strQueryParams)
    {
        SyncSource src = null;

	    try
	    {
	        prepareSync(esSyncSource, oSrcID);
	
	        if ( isContinueSync() )
	        {
	        	src = findSource(oSrcID);
	            if ( src != null )
	            {
		            LOG.INFO("Started synchronization of the data source: " + src.getName() );
	
		            src.m_strQueryParams = strQueryParams;
	                src.sync();
	
				    getNotify().fireSyncNotification(src, true, src.m_nErrCode, src.m_nErrCode == RhoAppAdapter.ERR_NONE ? RhoAppAdapter.getMessageText("sync_completed") : "");
	            }else
	            {
//                    LOG.ERROR( "Sync one source : Unknown Source " + oSrcID.toString() );
	            
		        	src = new SyncSource(this, getUserDB());
			    	//src.m_strError = "Unknown sync source.";
			    	src.m_nErrCode = RhoAppAdapter.ERR_RUNTIME;
		        	
	    	    	throw new RuntimeException("Sync one source : Unknown Source " + oSrcID.toString() );
	            }
	        }
	
	    } catch(Exception exc) {
    		LOG.ERROR("Sync source " + oSrcID.toString() + " failed.", exc);
	    	
	    	if ( src != null && src.m_nErrCode == RhoAppAdapter.ERR_NONE )
	    		src.m_nErrCode = RhoAppAdapter.ERR_RUNTIME;
	    	
	    	getNotify().fireSyncNotification(src, true, src.m_nErrCode, "" ); 
	    }

        getNotify().cleanCreateObjectErrors();
        if ( getState() != esExit )
            setState(esNone);
    }

	SyncSource findSource(SourceID oSrcID)
	{
	    for( int i = 0; i < (int)m_sources.size(); i++ )
	    {
	        SyncSource src = (SyncSource)m_sources.elementAt(i);
	        if ( oSrcID.isEqual(src) )
	            return src;
	    }
	    
	    return null;
	}
	
	SyncSource findSourceByName(String strSrcName)
	{
		return findSource(new SourceID(strSrcName));		
	}
	
	public void applyChangedValues(DBAdapter db)throws Exception
	{
	    IDBResult resSrc = db.executeSQL( "SELECT DISTINCT(source_id) FROM changed_values" );
	    for ( ; !resSrc.isEnd(); resSrc.next() )
	    {
	        int nSrcID = resSrc.getIntByIdx(0);
	        IDBResult res = db.executeSQL("SELECT source_id,sync_type,name, partition from sources WHERE source_id=?", nSrcID);
	        if ( res.isEnd() )
	            continue;

	        SyncSource src = new SyncSource( res.getIntByIdx(0), res.getStringByIdx(2), "none", db, this );

	        src.applyChangedValues();
	    }
	}
	
	void loadAllSources()throws Exception
	{
		if (isNoThreadedMode())
	        RhoAppAdapter.loadAllSyncSources();
	    else
	    {
	        getNet().pushData( getNet().resolveUrl("/system/loadallsyncsources"), "", null );
	    }
		
	    m_sources.removeAllElements();
	    Vector/*<String>*/ arPartNames = DBAdapter.getDBAllPartitionNames();

	    for( int i = 0; i < (int)arPartNames.size(); i++ )
	    {
	        DBAdapter dbPart = DBAdapter.getDB((String)arPartNames.elementAt(i));	    
		    IDBResult res = dbPart.executeSQL("SELECT source_id,sync_type,name from sources ORDER BY sync_priority");
		    for ( ; !res.isEnd(); res.next() )
		    { 
		        String strShouldSync = res.getStringByIdx(1);
		        if ( strShouldSync.compareTo("none") == 0)
		            continue;
	
		        String strName = res.getStringByIdx(2);
		        
		        m_sources.addElement( new SyncSource( res.getIntByIdx(0), strName, strShouldSync, dbPart, this) );
		    }
	    }
	    
	    checkSourceAssociations();
	}

	static int findSrcIndex( Vector/*Ptr<CSyncSource*>*/ sources, String strSrcName)
	{
	    for ( int i = 0; i < (int)sources.size(); i++)
	    {
	        if (strSrcName.compareTo( ((SyncSource)sources.elementAt(i)).getName()) == 0 )
	            return i;
	    }

	    return -1;
	}

	void checkSourceAssociations()
	{
	    Hashtable/*<String, int>*/ hashPassed = new Hashtable();
	    
	    for( int nCurSrc = m_sources.size()-1; nCurSrc >= 0 ; )
	    {
	        SyncSource oCurSrc = (SyncSource)m_sources.elementAt(nCurSrc);
	        if ( oCurSrc.getAssociations().size() == 0 || hashPassed.containsKey(oCurSrc.getName()) )
	            nCurSrc--;
	        else
	        {
	            int nSrc = nCurSrc;
	            for( int i = 0; i < (int)oCurSrc.getAssociations().size(); i++ )
	            {
	                SyncSource.CAssociation oAssoc = (SyncSource.CAssociation)oCurSrc.getAssociations().elementAt(i);
	                int nAssocSrcIndex = findSrcIndex( m_sources, oAssoc.m_strSrcName);
	                if ( nAssocSrcIndex >= 0 )
	                	((SyncSource)m_sources.elementAt(nAssocSrcIndex)).addBelongsTo( oAssoc.m_strAttrib, oCurSrc.getID() );
	                			
	                if ( nAssocSrcIndex >=0 && nAssocSrcIndex < nSrc )
	                {
	                    m_sources.removeElementAt( nSrc );
	                    m_sources.insertElementAt( oCurSrc, nAssocSrcIndex );

	                    nSrc = nAssocSrcIndex;
	                }
	            }
	        }

	        hashPassed.put(oCurSrc.getName(), new Integer(1) );
	    }
	}

	public String readClientID()throws Exception
	{
	    String clientID = "";
		
		synchronized( m_mxLoadClientID )
		{
	        IDBResult res = getUserDB().executeSQL("SELECT client_id,reset from client_info");
	        if ( !res.isEnd() )
	            clientID = res.getStringByIdx(0);
		}
		
		return clientID;
	}
	
	public String loadClientID()throws Exception
	{
	    String clientID = "";
		
		synchronized( m_mxLoadClientID )
		{
		    boolean bResetClient = false;
		    {
		        IDBResult res = getUserDB().executeSQL("SELECT client_id,reset from client_info");
		        if ( !res.isEnd() )
		        {
		            clientID = res.getStringByIdx(0);
		            bResetClient = res.getIntByIdx(1) > 0;
		        }
		    }
		    
		    if ( clientID.length() == 0 )
		    {
		        clientID = requestClientIDByNet();
		
	            IDBResult res = getUserDB().executeSQL("SELECT * FROM client_info");
	            if ( !res.isEnd() )
	            	getUserDB().executeSQL("UPDATE client_info SET client_id=?", clientID);
	            else
	            	getUserDB().executeSQL("INSERT INTO client_info (client_id) values (?)", clientID);
	            
		    	if ( clientID != null && clientID.length() > 0 && ClientRegister.getInstance() != null )
		    		ClientRegister.getInstance().startUp();	    	
	            
		    }else if ( bResetClient )
		    {
		    	if ( !resetClientIDByNet(clientID) )
		    		stopSync();
		    	else
		    		getUserDB().executeSQL("UPDATE client_info SET reset=? where client_id=?", new Integer(0), clientID );	    	
		    }
		}
		
		return clientID;
	}

	void processServerSources(String strSources)throws Exception
	{
	    if ( strSources.length() > 0 )
	    {
	        if (isNoThreadedMode())
	            RhoAppAdapter.loadServerSources(strSources);            
	        else
	        {
	        	getNet().pushData( getNet().resolveUrl("/system/loadserversources"), strSources, null);
	        }
	        
	        loadAllSources();
	        
	        DBAdapter.initAttrManager();
	    }
	}
	
	boolean resetClientIDByNet(String strClientID)throws Exception
	{
	    NetResponse resp = getNetClientID().pullData(getProtocol().getClientResetUrl(strClientID), this);
	    
	    if ( !resp.isOK() )
	    {
	    	m_nErrCode = RhoAppAdapter.getErrorFromResponse(resp);
	    	m_strError = resp.getCharData();
	    }else
	        RHOCONF().setString("reset_models", "", true);
	    
	    return resp.isOK();
	}

	String requestClientIDByNet()throws Exception
	{
        String strBody = "";
        //TODO: send client register info in client create 
//        if ( ClientRegister.getInstance() != null )
//            strBody += ClientRegister.getInstance().getRegisterBody();
	
	    NetResponse resp = getNetClientID().pullData(getProtocol().getClientCreateUrl(), this);
	    if ( resp.isOK() && resp.getCharData() != null )
	    {
	    	String szData = resp.getCharData();
	    	
	        JSONEntry oJsonEntry = new JSONEntry(szData);
	
	        //if (oJsonEntry.hasName("sources") )
	        //    processServerSources(szData);
	        
	        JSONEntry oJsonObject = oJsonEntry.getEntry("client");
	        if ( !oJsonObject.isEmpty() )
	            return oJsonObject.getString("client_id");
	    }else
	    {
	    	m_nErrCode = RhoAppAdapter.getErrorFromResponse(resp);
	    	if ( m_nErrCode == RhoAppAdapter.ERR_NONE )
	    	{
	    		m_nErrCode = RhoAppAdapter.ERR_UNEXPECTEDSERVERRESPONSE;
	    		m_strError = resp.getCharData();
	    	}
	    }
	
	    return "";
	}

	void doBulkSync()throws Exception
	{
//	    processServerSources("{\"partition\":\"" + "application" + "\"}");

	    if ( !RhoConf.getInstance().isExist("bulksync_state") )
	        return;
		
	    int nBulkSyncState = RHOCONF().getInt("bulksync_state");
		if ( !isContinueSync() ) {
			return;
		}
		
		switch (nBulkSyncState) {
			case ebsNotSynced:
				loadBulkPartitions();
				
				if ( !isContinueSync() ) {
					return;
				}
				
				//no break here is intentional.
			case ebsLoadBlobs:
				if ( !processBlobs() ) {
					return;
				}
				break;
				
			default:
				return;
		}
	    
	    if (isContinueSync())
	    {
	    	RHOCONF().setInt("bulksync_state", ebsSynced, true);
	    	getNotify().fireBulkSyncNotification(true, "complete", "", RhoAppAdapter.ERR_NONE);
	    }
	}

	void loadBulkPartitions()throws Exception 
	{
		LOG.INFO( "Bulk sync: start" );
		getNotify().fireBulkSyncNotification(false, "start", "", RhoAppAdapter.ERR_NONE);        
		Vector/*<String>*/ arPartNames = DBAdapter.getDBAllPartitionNames();
		
		for (int i = 0; i < (int)arPartNames.size() && isContinueSync(); i++)
		{
			if ( ((String)arPartNames.elementAt(i)).compareTo("local") !=0 )
				loadBulkPartition( (String)arPartNames.elementAt(i) );
		}
	}

	boolean processBlobs()throws Exception 
	{
		LOG.INFO("Bulk sync: download BLOBs");
		
		RHOCONF().setInt("bulksync_state", ebsLoadBlobs, true );
		getNotify().fireBulkSyncNotification( false, "blobs", "", RhoAppAdapter.ERR_NONE);
		
		LOG.TRACE( "=== Processing server blob attributes ===" );
		
		for ( int i = 0; i < (int)m_sources.size(); ++i ) 
		{
			SyncSource src = (SyncSource)m_sources.elementAt(i);
			if ( !src.processServerBlobAttrs() ) {
				getNotify().fireBulkSyncNotification(false, "error", "", RhoAppAdapter.ERR_UNEXPECTEDSERVERRESPONSE);
				return false;
			}
		}
		
		LOG.TRACE( "=== Processing server blob attributes DONE ===" );
		
		DBAdapter.initAttrManager();
		
		for ( int i = 0; i < (int)m_sources.size(); ++i ) 
		{
			SyncSource src = (SyncSource)m_sources.elementAt(i);
			if (!src.processAllBlobs()) {
				getNotify().fireBulkSyncNotification(false, "error", "", RhoAppAdapter.ERR_UNEXPECTEDSERVERRESPONSE);
				return false;
			}
		}
		
		return true;
	}
	
	void loadBulkPartition(String strPartition )throws Exception
	{
		DBAdapter dbPartition = getDB(strPartition); 		
	    String serverUrl = RhoConf.getInstance().getPath("syncserver");
	    String strUrl = serverUrl + "bulk_data";
	    String strQuery = "?client_id=" + m_clientID + "&partition=" + strPartition + "&sources=";
		for ( int i = 0; i < (int)m_sources.size(); ++i ) 
		{
			strQuery += URI.urlEncode( ((SyncSource)m_sources.elementAt(i)).getName() );
			if ( i < (int)m_sources.size()-1 ) {
				strQuery += ",";
			}
		}
	    
	    String strDataUrl = "", strCmd = "";

	    getNotify().fireBulkSyncNotification(false, "start", strPartition, RhoAppAdapter.ERR_NONE);
	    
	    while(strCmd.length() == 0&&isContinueSync())
	    {	    
	        NetResponse resp = getNet().pullData(strUrl+strQuery, this);
	        if ( !resp.isOK() || resp.getCharData() == null )
	        {
	    	    LOG.ERROR( "Bulk sync failed: server return an error." );
	    	    stopSync();
	    	    getNotify().fireBulkSyncNotification(true, "", strPartition,RhoAppAdapter.getErrorFromResponse(resp));
	    	    return;
	        }

		    LOG.INFO("Bulk sync: got response from server: " + resp.getCharData() );
	    	
	        String szData = resp.getCharData();
	        JSONEntry oJsonEntry = new JSONEntry(szData);
	        strCmd = oJsonEntry.getString("result");
	        if ( oJsonEntry.hasName("url") )
	   	        strDataUrl = oJsonEntry.getString("url");
	        
	        if ( strCmd.compareTo("wait") == 0)
	        {
	            int nTimeout = RhoConf.getInstance().getInt("bulksync_timeout_sec");
	            if ( nTimeout == 0 )
	                nTimeout = 5;

	            SyncThread.getInstance().wait(nTimeout);
	            strCmd = "";
	        }
	    }

	    if ( strCmd.compareTo("nop") == 0)
	    {
		    LOG.INFO("Bulk sync return no data.");
		    getNotify().fireBulkSyncNotification(true, "ok", strPartition, RhoAppAdapter.ERR_NONE);		    
		    return;
	    }

        if ( !isContinueSync() )
            return;

	    getNotify().fireBulkSyncNotification(false, "download", strPartition, RhoAppAdapter.ERR_NONE);
	    
	    String fDataName = makeBulkDataFileName(strDataUrl, dbPartition.getDBPath(), "_bulk.data");
	    String fScriptName = makeBulkDataFileName(strDataUrl, dbPartition.getDBPath(), "_bulk.script" );
	    
	    String strZip = ".gzip";
	    if (Capabilities.USE_SQLITE)
	    {
		    String strHsqlDataUrl = FilePath.join(getHostFromUrl(serverUrl), strDataUrl) + strZip;
		    downloadBulkDataAndUnzip(strHsqlDataUrl, fDataName+strZip, strPartition);
	    }
	    else
	    {
		    String strHsqlDataUrl = FilePath.join(getHostFromUrl(serverUrl), strDataUrl) + ".hsqldb.data" + strZip;
		    downloadBulkDataAndUnzip(strHsqlDataUrl, fDataName+strZip, strPartition);
	        if ( !isContinueSync() )
	            return;
		    
		    String strHsqlScriptUrl = FilePath.join(getHostFromUrl(serverUrl), strDataUrl) + ".hsqldb.script";
		    LOG.INFO("Bulk sync: download script from server: " + strHsqlScriptUrl);
		    {
			    NetResponse resp1 = getNet().pullFile(strHsqlScriptUrl, fScriptName, this, null);
			    if ( !resp1.isOK() )
			    {
				    LOG.ERROR("Bulk sync failed: cannot download database file.");
				    stopSync();
				    getNotify().fireBulkSyncNotification(true, "", strPartition, RhoAppAdapter.getErrorFromResponse(resp1));
			    }
		    }
	    }
        if ( !isContinueSync() )
            return;
	    
		LOG.INFO("Bulk sync: start change db");
		getNotify().fireBulkSyncNotification(false, "change_db", strPartition, RhoAppAdapter.ERR_NONE);
		
	    dbPartition.setBulkSyncDB(fDataName, fScriptName);
	    getSourceOptions().clearProperties();
	    processServerSources("{\"partition\":\"" + strPartition + "\"}");
	    
		LOG.INFO("Bulk sync: end change db");
		getNotify().fireBulkSyncNotification(false, "ok", strPartition, RhoAppAdapter.ERR_NONE);
	}
	
	void downloadBulkDataAndUnzip(String strDataUrl, String fDataName, String strPartition)throws Exception
	{
	    LOG.INFO("Bulk sync: download data from server: " + strDataUrl);
	    {
		    NetResponse resp1 = getNet().pullFile(strDataUrl, fDataName, this, null);
		    if ( !resp1.isOK() )
		    {
			    LOG.ERROR("Bulk sync failed: cannot download database file: " + resp1.getRespCode() );
			    stopSync();
			    getNotify().fireBulkSyncNotification(true, "", strPartition, RhoAppAdapter.getErrorFromResponse(resp1));
			    return;
		    }
	    }
		
	    LOG.INFO("Bulk sync: unzip db");

	    try
	    {
	    	RhoClassFactory.createRhoRubyHelper().unzip_file(fDataName);
	    }catch(Exception exc)
	    {
	        LOG.ERROR("Bulk sync failed: cannot unzip database file.", exc);
	        stopSync();
	        getNotify().fireBulkSyncNotification(true, "", strPartition, RhoAppAdapter.ERR_UNEXPECTEDSERVERRESPONSE);
	    }
	    
	    IFileAccess fs = RhoClassFactory.createFileAccess();
	    fs.delete(fDataName);
	}
	
	String makeBulkDataFileName(String strDataUrl, String strDbPath, String strExt)throws Exception
	{
	    FilePath oUrlPath = new FilePath(strDataUrl);
	    String strNewName = oUrlPath.getBaseName();
	    String strOldName = RhoConf.getInstance().getString("bulksync_filename");
	    if ( strOldName.length() > 0 && strNewName.compareTo(strOldName) != 0 )
	    {
	        FilePath oFilePath = new FilePath(strDbPath);
	        String strFToDelete = oFilePath.changeBaseName(strOldName+strExt);
	        LOG.INFO( "Bulk sync: remove old bulk file '" + strFToDelete + "'" );

	        //RhoFile.deleteFile( strFToDelete.c_str() );
	        RhoClassFactory.createFile().delete(strFToDelete);	        
	    }

	    RhoConf.getInstance().setString("bulksync_filename", strNewName, true);

	    FilePath oFilePath = new FilePath(strDbPath);
	    return oFilePath.changeBaseName(strNewName+strExt);
	}
/*	
	int getStartSource()
	{
	    for( int i = 0; i < m_sources.size(); i++ )
	    {
	        SyncSource src = (SyncSource)m_sources.elementAt(i);
	        if ( !src.isEmptyToken() )
	            return i;
	    }
	
	    return -1;
	}
*/
	void syncOneSource(int i, String strQueryParams, boolean syncOnlyIfChanged)throws Exception
	{
    	SyncSource src = null;
    	//boolean bError = false;
    	try{
		    src = (SyncSource)m_sources.elementAt(i);
		    if ( src.getSyncType().compareTo("bulk_sync_only")==0 )
		        return;
	
		    if ( isSessionExist() && getState() != esStop )
		    {
		    	src.m_strQueryParams = strQueryParams;
				if (syncOnlyIfChanged) {
					if (src.haveChangedValues() ) {
						src.sync();
					}
				} else {
					src.sync();
				}
		    }
		    
		    //getNotify().onSyncSourceEnd(i, m_sources);
    	}catch(Exception exc)
    	{
	    	if ( src.m_nErrCode == RhoAppAdapter.ERR_NONE )
	    		src.m_nErrCode = RhoAppAdapter.ERR_RUNTIME;
	    	
	    	//setState(esStop);
    		//throw exc;
	    	LOG.ERROR("Sync of source '" + src.getName()+ "' failed.", exc);
    	}finally{
    		getNotify().onSyncSourceEnd( i, m_sources );
    		//bError = src.m_nErrCode != RhoAppAdapter.ERR_NONE;
    	}
    	
	    //return !bError;
	}
	
	void syncAllSources(String strQueryParams, boolean bSyncOnlyChangedSources)throws Exception
	{
//	    boolean bError = false;

//	    int nStartSrc = getStartSource();
//	    if ( nStartSrc >= 0 )
//	        bError = !syncOneSource(nStartSrc);

	    //TODO: do not stop on error source
	    for( int i = 0; i < (int)m_sources.size() && isContinueSync(); i++ )
	    {
	        /*bError = !*/syncOneSource(i, strQueryParams, bSyncOnlyChangedSources);
	    }

	    if ( !isSchemaChanged()  && getState() != SyncEngine.esStop )
	    	getNotify().fireSyncNotification(null, true, RhoAppAdapter.ERR_NONE, RhoAppAdapter.getMessageText("sync_completed"));
	}
	
	void login(String name, String password, SyncNotify.SyncNotification oNotify)
	{
		try {
/*			
			processServerSources("{\"sources\":{ \"ProductEx\":{ "+
	        "\"sync_type\":\"incremental\", \"partition\":\"application\", \"source_id\":\"7\","+
	        " \"sync_priority\":\"0\", \"model_type\":\"fixed_schema\", "+
	        " \"schema\":{\"version\":\"1.1\", \"property\":{\"brand\":\"string\", \"price\":\"string\", \"quantity\":\"string\", \"name\":\"string\", "+
	        " \"image_url\":\"blob\", \"image_url_ex\":\"blob,overwrite\"}, "+
	        " \"index\":[{\"by_brand_price1\":\"brand,price\"}, {\"by_quantity1\":\"quantity\"}], \"unique_index\":[{\"by_name1\":\"name\"}]}, "+
	        " \"belongs_to\":{\"brand\":\"Customer\"}}}}");//, \"schema_version\":\"1.0\"
	        */
		    NetResponse resp = null;
		    m_bStopByUser = false;
		    
		    try{
				
			    resp = getNet().pullCookies( getProtocol().getLoginUrl(), getProtocol().getLoginBody(name, password), this );
			    int nErrCode = RhoAppAdapter.getErrorFromResponse(resp);
			    if ( nErrCode != RhoAppAdapter.ERR_NONE )
			    {
			        getNotify().callLoginCallback(oNotify, nErrCode, resp.getCharData());
			        return;
			    }
		    }catch(IOException exc)
		    {
				LOG.ERROR("Login failed.", exc);
		    	getNotify().callLoginCallback(oNotify, RhoAppAdapter.getNetErrorCode(exc), "" );
		    	return;
		    }
		    
		    String strSession = resp.getCharData();
		    if ( strSession == null || strSession.length() == 0 )
		    {
		    	LOG.ERROR("Return empty session.");
		    	getNotify().callLoginCallback(oNotify, RhoAppAdapter.ERR_UNEXPECTEDSERVERRESPONSE, "" );
		        return;
		    }
		    
		    if ( isStoppedByUser() )
		    	return;
		    
		    IDBResult res = getUserDB().executeSQL("SELECT * FROM client_info");
		    if ( !res.isEnd() )
		    	getUserDB().executeSQL( "UPDATE client_info SET session=?", strSession );
		    else
		    	getUserDB().executeSQL("INSERT INTO client_info (session) values (?)", strSession);
		
		    if ( RHOCONF().isExist("rho_sync_user") )
		    {
		        String strOldUser = RHOCONF().getString("rho_sync_user");
		        if ( name.compareTo(strOldUser) != 0 )
		        {
		            if (isNoThreadedMode())
		                RhoAppAdapter.resetDBOnSyncUserChanged();
		            else
		            {
		                NetResponse resp1 = getNet().pushData( getNet().resolveUrl("/system/resetDBOnSyncUserChanged"), "", null );
		            }
		        }
		    }
		    RHOCONF().setString("rho_sync_user", name, true);
		    
	    	getNotify().callLoginCallback(oNotify, RhoAppAdapter.ERR_NONE, "" );
		    
	    	if ( ClientRegister.getInstance() != null )
	    	{
	    		getUserDB().executeSQL("UPDATE client_info SET token_sent=?", 0 );
	    		ClientRegister.getInstance().startUp();
	    	}
	    	
		}catch(Exception exc)
		{
			LOG.ERROR("Login failed.", exc);
	    	getNotify().callLoginCallback(oNotify, RhoAppAdapter.ERR_RUNTIME, "" );
		}
	}

	boolean isLoggedIn()throws DBException
	{
	    String strRes = "";
	    IDBResult res = getUserDB().executeSQL("SELECT session FROM client_info");
	    
	    if ( !res.isEnd() )
	    	strRes = res.getStringByIdx(0);
	    
	    return strRes.length() > 0;
	}

	String loadSession()throws DBException
	{
		m_strSession = "";
	    IDBResult res = getUserDB().executeSQL("SELECT session FROM client_info");
	    
	    if ( !res.isEnd() )
	    	m_strSession = res.getStringByIdx(0);
	    
	    return m_strSession;
	}
	
	public void logout()throws Exception
	{
		stopSync();
		logout_int();
	}

	public void logout_int()throws Exception
	{
		getUserDB().executeSQL( "UPDATE client_info SET session = NULL");
	    m_strSession = "";
	
	    //loadAllSources();
	}
	
	public void setSyncServer(String syncserver)throws Exception
	{
		String strOldSrv = RhoConf.getInstance().getString("syncserver");
		String strNewSrv = syncserver != null ? syncserver : "";
		
		if ( strOldSrv.compareTo(strNewSrv) != 0)
		{
			RhoConf.getInstance().setString("syncserver", syncserver, true);
			
			getUserDB().executeSQL("DELETE FROM client_info");

			logout_int();
		}
	}
	
	static String getHostFromUrl( String strUrl )
	{
		URI uri = new URI(strUrl);
		return uri.getHostSpecificPart() + "/";
	}
	
}
