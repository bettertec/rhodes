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

import com.rho.Mutex;
import com.rho.RhoClassFactory;
import com.rho.RhoEmptyLogger;
import com.rho.RhoLogger;
import com.rho.RhoAppAdapter;

import java.util.Enumeration;
import java.util.Hashtable;
import com.rho.*;
import com.rho.db.*;
import com.rho.net.*;

import java.util.Vector;

public class SyncNotify {
	private static final RhoLogger LOG = RhoLogger.RHO_STRIP_LOG ? new RhoEmptyLogger() : 
		new RhoLogger("Sync");

	final int MAX_SERVER_ERROR_LEN = 1000;
    public static class SyncNotification
    {
        String m_strUrl ="", m_strParams="";
        boolean m_bRemoveAfterFire;
        
        SyncNotification(){m_bRemoveAfterFire = false;}
        
        SyncNotification(String strUrl, String strParams, boolean bRemoveAfterFire)
        {
            if ( strUrl.length() > 0 )
                m_strUrl = RhodesApp.getInstance().canonicalizeRhoUrl(strUrl);
        	
        	m_strParams = strParams; 
        	m_bRemoveAfterFire = bRemoveAfterFire; 
        }
        
        public String toString()
        {
        	return "Url: " + m_strUrl + ";Params:" + m_strParams + ";RemoveAfterFire: " + m_bRemoveAfterFire;
        }
    };
	
	public static final Integer enNone = new Integer(0), enDelete=new Integer(1), enUpdate=new Integer(2), enCreate=new Integer(3);
	
    SyncEngine m_syncEngine;

    static String m_strObjectNotifyUrl = "";
    Hashtable/*<int, Hashtable<String,int>* >*/ m_hashSrcIDAndObject = new Hashtable();
    Hashtable/*<int, Hashtable<String,String>* >*/ m_hashCreateObjectErrors = new Hashtable();
    String m_strSingleObjectSrcName = "", m_strSingleObjectID ="";
    Hashtable/*<int,int>*/ m_hashSrcObjectCount = new Hashtable();
    
    static Mutex m_mxObjectNotify = new Mutex();

    Hashtable/*<int,SyncNotification>*/ m_mapSyncNotifications = new Hashtable();
    SyncNotification m_pSearchNotification;
    SyncNotification m_pAllNotification;
    SyncNotification m_emptyNotify = new SyncNotification();
    
    Mutex m_mxSyncNotifications = new Mutex();
    ISyncStatusListener m_syncStatusListener = null;
    boolean m_bEnableReporting = false;
    boolean m_bEnableReportingGlobal = true;
    Vector/*<String>*/ m_arNotifyBody = new Vector();
    boolean m_bFakeServerResponse = false;
    boolean m_isInsideCallback = false;
    NetRequest m_NetRequest;
    
    SyncEngine getSync(){ return m_syncEngine; }
    NetRequest getNet() { return m_NetRequest;}

    void cleanNotifyBody(){ m_arNotifyBody.removeAllElements(); setFakeServerResponse(false); }
    boolean isFakeServerResponse(){ return m_bFakeServerResponse; }
    void setFakeServerResponse(boolean bFakeServerResponse){ m_bFakeServerResponse = bFakeServerResponse; }

    boolean isReportingEnabled(){return m_bEnableReporting&&m_bEnableReportingGlobal;}
    void enableReporting(boolean bEnable) {	m_bEnableReporting = bEnable; }
    void enableStatusPopup(boolean bEnable){m_bEnableReportingGlobal = bEnable;}
    boolean isInsideCallback() { return m_isInsideCallback; }
    
    SyncNotify( SyncEngine syncEngine ) 
    {
    	m_syncEngine = syncEngine;
    }
    
    void setFactory(RhoClassFactory factory)throws Exception{ 
		m_NetRequest = RhoClassFactory.createNetRequest();
    }
    
    void addObjectNotify(Integer nSrcID, String strObject)
    {
    	synchronized(m_mxObjectNotify)
    	{
	        Hashtable/*<String,int>*/ hashObject = (Hashtable)m_hashSrcIDAndObject.get(nSrcID);
	        if ( hashObject == null )
	        {
	            hashObject = new Hashtable/*<String,int>*/();
	            m_hashSrcIDAndObject.put( nSrcID, hashObject );
	        }
	
	        hashObject.put(strObject,enNone);
    	}
    }

    void addObjectNotify(String strSrcName, String strObject )
    {
    	synchronized(m_mxObjectNotify)
    	{
	        m_strSingleObjectSrcName = strSrcName;
	        m_strSingleObjectID = strObject.charAt(0) == '{' ? strObject.substring(1,strObject.length()-2) : strObject ;
    	}
    }

    void cleanObjectNotifications()
    {
        synchronized(m_mxObjectNotify)
        {
            m_strSingleObjectSrcName = "";
            m_strSingleObjectID = "";
            m_hashSrcIDAndObject.clear();
        }
    }

    void cleanCreateObjectErrors()
    {
        synchronized(m_mxObjectNotify)
        {
            m_hashCreateObjectErrors.clear();
        }
    }

    void processSingleObject()
    {
        if ( m_strSingleObjectSrcName.length() == 0 )
            return;

        SyncSource Src = getSync().findSourceByName(m_strSingleObjectSrcName);
        if ( Src != null)
        {
            Integer nSrcID = Src.getID();
            addObjectNotify(nSrcID,m_strSingleObjectID);
        }

        m_strSingleObjectSrcName = "";
        m_strSingleObjectID = "";
    }

    void fireObjectsNotification()throws Exception
    {
    	String strBody = "";
        String strUrl = "";

        synchronized(m_mxObjectNotify)
        {
            if ( m_strObjectNotifyUrl.length() == 0 )
                return;

            strUrl = getNet().resolveUrl(m_strObjectNotifyUrl);

        	Enumeration valsNotify = m_hashSrcIDAndObject.elements();
        	Enumeration keysNotify = m_hashSrcIDAndObject.keys();
    		while (valsNotify.hasMoreElements()) 
    		{
    			Integer nSrcID = (Integer)keysNotify.nextElement();
    			Hashtable hashObject = (Hashtable)valsNotify.nextElement();

            	Enumeration valsObject = hashObject.elements();
            	Enumeration keysObject = hashObject.keys();
        		while (valsObject.hasMoreElements()) 
        		{
        			int nNotifyType = ((Integer)valsObject.nextElement()).intValue();
        			String strObject = (String)keysObject.nextElement();
        			
                    if (nNotifyType == enNone.intValue())
                        continue;

                  //This is slow operation
/*
                    if ( nNotifyType == enDelete.intValue() )
                    {
                        IDBResult res = getDB().executeSQL("SELECT object FROM object_values where object=? LIMIT 1 OFFSET 0", strObject );
                        if ( !res.isOneEnd() )
                            nNotifyType = enUpdate.intValue();    
                    }
*/
                    if ( strBody.length() > 0 )
                    	strBody += "&rho_callback=1&";

                    if (nNotifyType == enDelete.intValue() ) 
                    {
                        strBody += "deleted[][object]=" + strObject;
                        strBody += "&deleted[][source_id]=" + nSrcID;
                    }else if ( nNotifyType == enUpdate.intValue() )
                    {
                        strBody += "updated[][object]=" + strObject;
                        strBody += "&updated[][source_id]=" + nSrcID;
                    }else if ( nNotifyType == enCreate.intValue() )
                    {
                        strBody += "created[][object]=" + strObject;
                        strBody += "&created[][source_id]=" + nSrcID;
                    }

                    hashObject.put(strObject, enNone);
                }
            }

            if ( strBody.length() == 0 )
                return;
        }

        callNotify( new SyncNotification(strUrl,"",false), strBody);
    }

    void onObjectChanged(Integer nSrcID, String strObject, Integer nType)
    {
        synchronized(m_mxObjectNotify)
        {
            processSingleObject();

            Hashtable/*<String,int>*/ hashObject = (Hashtable)m_hashSrcIDAndObject.get(nSrcID);
            if ( hashObject == null )
                return;

            if( hashObject.containsKey(strObject) )
                hashObject.put(strObject, nType);
        }
    }

    void addCreateObjectError(Integer nSrcID, String strObject, String strError)
    {
        synchronized(m_mxObjectNotify)
        {
            Hashtable/*<String,String>*/ hashErrors = (Hashtable)m_hashCreateObjectErrors.get(nSrcID);
            if ( hashErrors == null )
            {
                hashErrors = new Hashtable/*<String,String>*/();
                m_hashCreateObjectErrors.put( nSrcID, hashErrors );
            }

            hashErrors.put(strObject,strError);
        }
    }

    String makeCreateObjectErrorBody(Integer nSrcID)
    {
        String strBody = "";

        synchronized(m_mxObjectNotify)
        {
            Hashtable/*<String,String>*/ hashErrors = (Hashtable)m_hashCreateObjectErrors.get(nSrcID);
            if ( hashErrors == null )
                return "";

        	Enumeration valsErrors = hashErrors.elements();
        	Enumeration keysErrors = hashErrors.keys();
    		while (valsErrors.hasMoreElements()) 
    		{
    			String strObject = (String)keysErrors.nextElement();
    			String strError = (String)valsErrors.nextElement();
                strBody += "&create_error[][object]=" + strObject;
                strBody += "&create_error[][error_message]=" + strError;
            }

            hashErrors.clear();
        }

        return strBody;
    }

    static void setObjectNotifyUrl(String strUrl)
    { 
        synchronized(m_mxObjectNotify)
        {
            m_strObjectNotifyUrl = strUrl; 
        }
    }

    String getObjectNotifyUrl()
    {
        synchronized(m_mxObjectNotify)
        {
            return m_strObjectNotifyUrl;
        }
    }
   
    void onSyncSourceEnd( int nSrc, Vector/*Ptr<CSyncSource*>&*/ sources )
    {
        SyncSource src = (SyncSource)sources.elementAt(nSrc);
/*
        if ( getSync().getState() == SyncEngine.esStop && src.m_nErrCode != RhoAppAdapter.ERR_NONE )
        {
        	SyncNotification pSN = getSyncNotifyBySrc(src);
        	if ( pSN != null )        	
        		fireSyncNotification(src, true, src.m_nErrCode, "");
        	else
        		fireAllSyncNotifications(true, src.m_nErrCode, src.m_strError, "" );
        }
        else*/
            fireSyncNotification( src, true, src.m_nErrCode, "");

        cleanCreateObjectErrors();
    }
/*    
	void setSyncNotification(int source_id, String strUrl, String strParams )throws Exception
	{
		LOG.INFO( "Set notification. Source ID: " + source_id + "; Url :" + strUrl + "; Params: " + strParams );
	    String strFullUrl = getNet().resolveUrl(strUrl);
		
		if ( source_id == -1 )
		{
			synchronized(m_mxSyncNotifications)
			{
				m_pAllNotification = new SyncNotification( strFullUrl, strParams, false );				
			}
			LOG.INFO( " Done Set notification for all sources; Url :" + strFullUrl + "; Params: " + strParams );			
		}else
		{
		    if ( strFullUrl.length() > 0 )
		    {
		        synchronized(m_mxSyncNotifications){
		        	m_mapSyncNotifications.put(new Integer(source_id),new SyncNotification( strFullUrl, strParams, true ) );
		        }
				LOG.INFO( " Done Set notification. Source ID: " + source_id + "; Url :" + strFullUrl + "; Params: " + strParams );
		    }
		}
	}*/
    void setSyncNotification(int source_id, SyncNotification pNotify)
    {
        LOG.INFO("Set notification. Source ID: " + source_id + ";" + (pNotify != null? pNotify.toString() : "") );

    	if ( source_id == -1 )
    	{
    		synchronized(m_mxSyncNotifications)
            {
                m_pAllNotification = pNotify;
            }
    	}else
    	{
            synchronized(m_mxSyncNotifications)
            {
                m_mapSyncNotifications.put( new Integer(source_id), pNotify );
            }
        }
    	
    }

	void setSearchNotification(String strUrl, String strParams )throws Exception
	{
		LOG.INFO( "Set search notification. Url :" + strUrl + "; Params: " + strParams );
	    String strFullUrl = getNet().resolveUrl(strUrl);
		
	    if ( strFullUrl.length() > 0 )
	    {
	        synchronized(m_mxSyncNotifications){
	        	m_pSearchNotification = new SyncNotification( strFullUrl, strParams, true );
	        }
			LOG.INFO( " Done Set search notification. Url :" + strFullUrl + "; Params: " + strParams );
	    }
	}

    public void setSyncStatusListener(ISyncStatusListener listener) 
    { 
    	synchronized(m_mxSyncNotifications){
    		m_syncStatusListener = listener;
    	}
    }
    
    public void reportSyncStatus(String status, int error, String strDetails) 
    {
    	synchronized(m_mxSyncNotifications)
    	{    	
	    	if (m_syncStatusListener != null && (isReportingEnabled() || error == RhoAppAdapter.ERR_SYNCVERSION) ) {
	    		
	    		if ( error == RhoAppAdapter.ERR_SYNCVERSION )
	    			status = RhoAppAdapter.getErrorText(error);
	    		else
	    		{
		    		if ( strDetails.length() == 0 )
		    			strDetails = RhoAppAdapter.getErrorText(error);
		    		status += (strDetails.length() > 0 ? RhoAppAdapter.getMessageText("details") + strDetails: "");
	    		}
	    		
	        	LOG.INFO("Status: "+status);
	    		
	        	m_syncStatusListener.reportStatus( status, error);
	    	}
    	}
    }

	void fireBulkSyncNotification( boolean bFinish, String status, String partition, int nErrCode )
	{
		if ( getSync().getState() == SyncEngine.esExit )
			return;
		
		if( nErrCode != RhoAppAdapter.ERR_NONE)
		{
			String strMessage = RhoAppAdapter.getMessageText("sync_failed_for") + "bulk.";
			reportSyncStatus(strMessage,nErrCode,"");
		}

	    String strParams = "";
	    strParams += "partition=" + partition;
	    strParams += "&bulk_status="+status;
	    strParams += "&sync_type=bulk";

	    doFireSyncNotification( null, bFinish, nErrCode, "", strParams, "" );
	}

	void fireAllSyncNotifications( boolean bFinish, int nErrCode, String strError, String strServerError )
	{
	    if ( getSync().getState() == SyncEngine.esExit )
			return;

		if( nErrCode != RhoAppAdapter.ERR_NONE)
		{
			if ( !getSync().isSearch() )
			{
				String strMessage = RhoAppAdapter.getMessageText("sync_failed_for") + "all.";
				
				reportSyncStatus(strMessage,nErrCode,strError);
			}
		}
	    
	    synchronized(m_mxSyncNotifications)
	    {
	        SyncNotification pSN = getSyncNotifyBySrc(null);    
	        if ( pSN != null )
	            doFireSyncNotification( null, bFinish, nErrCode, strError, "", strServerError );
	    }
	}

	void fireSyncNotification( SyncSource src, boolean bFinish, int nErrCode, String strMessage )
	{
		if ( getSync().getState() == SyncEngine.esExit )
			return;
		
		if( strMessage.length() > 0 || nErrCode != RhoAppAdapter.ERR_NONE)
		{
			if ( !getSync().isSearch() )
			{
				if ( src != null && (strMessage==null || strMessage.length() == 0) )
					strMessage = RhoAppAdapter.getMessageText("sync_failed_for") + src.getName() + ".";
				
				reportSyncStatus(strMessage,nErrCode,src!= null?src.m_strError:"");
			}
		}
		
		doFireSyncNotification(src, bFinish, nErrCode, "", "", "" );
	}
	
	SyncNotification getSyncNotifyBySrc(SyncSource src)
	{
	    SyncNotification pSN = null;
		if ( getSync().isSearch() )
			pSN = m_pSearchNotification;
		else
	    {
	        if ( src != null )
			    pSN = (SyncNotification)m_mapSyncNotifications.get( src.getID());

	        if ( pSN == null )
	            pSN = m_pAllNotification;
	    }

		if ( pSN == null && !getSync().isNoThreadedMode() )
	        return null;

	    return pSN != null ? pSN : m_emptyNotify;
	}
	
	void fireSyncNotification2( SyncSource src, boolean bFinish, int nErrCode, String strServerError)
	{
	    doFireSyncNotification(src, bFinish, nErrCode, "", "", strServerError);
	}
	
	void doFireSyncNotification( SyncSource src, boolean bFinish, int nErrCode, String strError, String strParams, String strServerError )
	{
		if ( getSync().isStoppedByUser() )
			return;
		
		try{
			SyncNotification pSN = null;		
			
		    String strBody = "";
		    boolean bRemoveAfterFire = bFinish;
		    {
		    	synchronized(m_mxSyncNotifications){
		    		pSN = getSyncNotifyBySrc(src);
			        if ( pSN == null )
			            return;
			
				    strBody = "";

		            if ( src != null )
		            {
				        strBody += "total_count=" + src.getTotalCount();
				        strBody += "&processed_count=" + src.getCurPageCount();
				        strBody += "&processed_objects_count=" + getLastSyncObjectCount(src.getID());
				        strBody += "&cumulative_count=" + src.getServerObjectsCount();			        
				        strBody += "&source_id=" + src.getID();
				        strBody += "&source_name=" + src.getName();
		            }
		            
		            if ( strParams.length() > 0 )
		            	strBody += (strBody.length() > 0 ? "&" : "") + strParams;
		            else
		            	strBody += (strBody.length() > 0 ? "&" : "") + "sync_type=incremental";
			        
			        strBody += "&status=";
			        if ( bFinish )
			        {
				        if ( nErrCode == RhoAppAdapter.ERR_NONE )
				        {
				        	if ( getSync().isSchemaChanged() )
				        		strBody += "schema_changed";
				        	else				        	
				        		strBody += (src == null && strParams.length() == 0) ? "complete" : "ok";
				        }
				        else
				        {
				        	if ( getSync().isStoppedByUser() )
		                        nErrCode = RhoAppAdapter.ERR_CANCELBYUSER;
				        	
				        	strBody += "error";				        	
						    strBody += "&error_code=" + nErrCode;
						    
						    if ( strError != null && strError.length() > 0 )
						    {
							    if ( strError.length() > MAX_SERVER_ERROR_LEN )
							    	strError = strError.substring(0, MAX_SERVER_ERROR_LEN);
						    	
						    	strBody += "&error_message=" + URI.urlEncode(strError);
						    }
						    else  if ( src != null && src.m_strError != null)
						    {
							    if ( src.m_strError.length() > MAX_SERVER_ERROR_LEN )
							    	src.m_strError = src.m_strError.substring(0, MAX_SERVER_ERROR_LEN);
						    	
						    	strBody += "&error_message=" + URI.urlEncode(src.m_strError);
						    }
						    
						    if ( strServerError != null && strServerError.length() > 0 )
						    {
							    if ( strServerError.length() > MAX_SERVER_ERROR_LEN )
							    	strServerError = strServerError.substring(0, MAX_SERVER_ERROR_LEN);
						    	
						    	strBody += "&" + strServerError;
						    }
				        }
				        
		                if ( src != null )
		                    strBody += makeCreateObjectErrorBody( src.getID());
			        }
			        else
			        	strBody += "in_progress";
			        
			        strBody += "&rho_callback=1";
			        if ( pSN.m_strParams != null && pSN.m_strParams.length() > 0 )
			        {
			        	if ( !pSN.m_strParams.startsWith("&") )
			        		strBody += "&";
			        	
			        	strBody += pSN.m_strParams;
			        }			        	
			        
			        bRemoveAfterFire = bRemoveAfterFire && pSN.m_bRemoveAfterFire;
		        }
		    }
		    if ( bRemoveAfterFire )
		    	clearNotification(src);
		    
		    LOG.INFO("Fire notification. Source : " + (src != null ? (src).getName():"") + "; " + pSN.toString() + ";Body: " + strBody);
		    
            if ( callNotify(pSN, strBody) )
                clearNotification(src);
		}catch(Exception exc)
		{
			LOG.ERROR("Fire notification failed.", exc);
		}
	}

	String getNotifyBody()
	{
	    String emptyBody = "";
	    if ( m_arNotifyBody.size() == 0 )
	        return emptyBody;

	    if ( isFakeServerResponse() )
	        return (String)m_arNotifyBody.elementAt(0);

	    return (String)m_arNotifyBody.elementAt(m_arNotifyBody.size()-1);
	}
	
    boolean callNotify(SyncNotification oNotify, String strBody )throws Exception
    {
        String strUrl = new String(oNotify.m_strUrl); //Need to copy url since notify may be cleared in callback
        
        if ( getSync().isNoThreadedMode() )
        {
        	m_arNotifyBody.addElement( strBody );
            return false;
        }

        if ( strUrl.length() == 0 )
            return true;

        if ( strUrl.startsWith("javascript:") )
        {
            String js = strUrl.substring(11) + "('" + strBody + "');";
            RhoClassFactory.createRhoRubyHelper().webview_execute_js(js);
        	return true;
        }
        
        m_isInsideCallback = true; 
        NetResponse resp = getNet().pushData( strUrl, strBody, null );
        m_isInsideCallback = false; 
        if ( !resp.isOK() )
            LOG.ERROR( "Fire object notification failed. Code: " + resp.getRespCode() + "; Error body: " + resp.getCharData() );
        else
        {
            String szData = resp.getCharData();
            return szData != null && szData.equals("stop");
        }

        return false;
    }

	void clearNotification(SyncSource src)
	{
		LOG.INFO( "Clear notification. Source : " + (src != null ? src.getName() : "" ) );

	    synchronized(m_mxSyncNotifications)
	    {
	    	if ( getSync().isSearch() )
	            m_pSearchNotification = null;
	        else
	            m_mapSyncNotifications.remove(src.getID());
	    }
	}
	
	void clearSyncNotification(int source_id) 
	{
		LOG.INFO( "Clear notification. Source ID: " + source_id );
		
		synchronized(m_mxSyncNotifications){
			if ( source_id == -1 )//Clear all
				m_pAllNotification = null;
			else			
				m_mapSyncNotifications.remove(new Integer(source_id));
		}
	}

	void cleanLastSyncObjectCount()
	{
	    synchronized(m_mxSyncNotifications)
	    {
	        m_hashSrcObjectCount.clear();
	    }
	}

	int incLastSyncObjectCount(Integer nSrcID)
	{
		Integer nCount = null;
	    synchronized(m_mxSyncNotifications)
	    {
	        nCount = ((Integer)m_hashSrcObjectCount.get(nSrcID));
	        if ( nCount == null )
	        	nCount = new Integer(0);
	        
	        nCount = new Integer(nCount.intValue()+1);
	        
	        m_hashSrcObjectCount.put(nSrcID,nCount);
	    }
	    
	    return nCount != null ? nCount.intValue() : 0;
	}

	int getLastSyncObjectCount(Integer nSrcID)
	{
	    int nCount = 0;
	    synchronized(m_mxSyncNotifications)
	    {
	        Integer nCountVal = (Integer)m_hashSrcObjectCount.get(nSrcID);
	        if ( nCountVal != null )
	        	nCount = nCountVal.intValue();
	    }

	    return nCount;
	}

	void callLoginCallback(SyncNotification oNotify, int nErrCode, String strMessage)
	{
		try{
			if ( getSync().isStoppedByUser() )
				return;
			
		    String strBody = "error_code=" + nErrCode;
		    if ( strMessage != null&& strMessage.length() > MAX_SERVER_ERROR_LEN )
		    	strMessage = strMessage.substring(0, MAX_SERVER_ERROR_LEN);
		    	
	        strBody += "&error_message=" + URI.urlEncode(strMessage != null? strMessage : "");
	        strBody += "&rho_callback=1";
	        
			LOG.INFO( "Login callback: " + oNotify.toString() + ". Body: "+ strBody );

            callNotify(oNotify, strBody);	
		}catch(Exception exc)
		{
			LOG.ERROR("Call Login callback failed.", exc);
		}
	}
	
	
}
