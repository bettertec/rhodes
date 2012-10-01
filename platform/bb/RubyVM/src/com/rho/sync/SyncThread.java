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

import j2me.util.LinkedList;

import com.rho.Mutex;
import com.rho.RhoClassFactory;
import com.rho.RhoConf;
import com.rho.RhoEmptyLogger;
import com.rho.RhoLogger;
import com.rho.RhoAppAdapter;
import com.rho.RhoRuby;
import com.rho.ThreadQueue;
import com.rho.TimeInterval;
import com.rho.db.DBAdapter;
import com.rho.db.IDBResult;
import com.xruby.runtime.builtin.*;
import com.xruby.runtime.lang.*;

import java.util.Vector;

public class SyncThread extends ThreadQueue
{
	private static final RhoLogger LOG = RhoLogger.RHO_STRIP_LOG ? new RhoEmptyLogger() : 
		new RhoLogger("Sync");
	//private static final int SYNC_POLL_INTERVAL_SECONDS = 300;
	//private static final int SYNC_POLL_INTERVAL_INFINITE = Integer.MAX_VALUE/1000;
	private static final int SYNC_WAIT_BEFOREKILL_SECONDS  = 3;
	
//scSyncOneByUrl = 4, scChangePollInterval=5, scExit=6, 
   	public final static int scNone = 0, scSyncAll = 1, scSyncOne = 2, scLogin = 3, scSearchOne=4; 
    
   	static private class SyncCommand implements ThreadQueue.IQueueCommand
   	{
   		int m_nCmdCode;
   		int m_nCmdParam;
   		String m_strCmdParam, m_strQueryParams;
   		boolean m_bShowStatus;
   		
   		SyncCommand(int nCode, int nParam, boolean bShowStatus, String query_params)
   		{
   			m_nCmdCode = nCode;
   			m_nCmdParam = nParam;
   			m_bShowStatus = bShowStatus;
   			m_strQueryParams = query_params != null? query_params : "";
   		}
   		SyncCommand(int nCode, String strParam, boolean bShowStatus, String query_params)
   		{
   			m_nCmdCode = nCode;
   			m_strCmdParam = strParam;
   			m_bShowStatus = bShowStatus;
   			m_strQueryParams = query_params != null? query_params : "";
   		}
	    SyncCommand(int nCode, String strParam, int nCmdParam, boolean bShowStatus, String query_params)
	    {
		    m_nCmdCode = nCode;
		    m_strCmdParam = strParam;
            m_nCmdParam = nCmdParam;
            m_bShowStatus = bShowStatus;
            m_strQueryParams = query_params != null? query_params : "";
	    }
   		
   		SyncCommand(int nCode, boolean bShowStatus, String query_params)
   		{
   			m_nCmdCode = nCode;
   			m_nCmdParam = 0;
   			m_bShowStatus = bShowStatus;
   			m_strQueryParams = query_params != null? query_params : "";
   		}
   		
   		public boolean equals(IQueueCommand obj)
   		{
   			SyncCommand oSyncCmd = (SyncCommand)obj;
   			return m_nCmdCode == oSyncCmd.m_nCmdCode && m_nCmdParam == oSyncCmd.m_nCmdParam &&
   				(m_strCmdParam == oSyncCmd.m_strCmdParam ||
   				(m_strCmdParam != null && oSyncCmd.m_strCmdParam != null && m_strCmdParam.equals(oSyncCmd.m_strCmdParam)))&&
   				(m_strQueryParams == oSyncCmd.m_strQueryParams ||
   				(m_strQueryParams != null && oSyncCmd.m_strQueryParams != null && m_strQueryParams.equals(oSyncCmd.m_strQueryParams)));  		
   		}
   		
   		public String toString()
   		{
   		    switch(m_nCmdCode)
   		    {
   		    case scNone:
   		        return "CheckPollInterval";

   		    case scSyncAll:
   		        return "SyncAll";
   		    case scSyncOne:
   		        return "SyncOne";
   		    case scLogin:
   		        return "Login";
   		    case scSearchOne:
   		        return "Search";
   		    }

   		    return "Unknown; Code : " + m_nCmdCode;
   		}
   		
   		public void cancel(){}
   		
   	};
   	static private class SyncLoginCommand extends SyncCommand
   	{
   		String m_strName, m_strPassword;
   		/*common::CAutoPtr<C*/SyncNotify.SyncNotification/*>*/ m_pNotify;
   		public SyncLoginCommand(String name, String password, String callback, SyncNotify.SyncNotification pNotify)
   		{
   			super(scLogin,callback,false,"");
   			
   			m_strName = name;
   			m_strPassword = password;
   			m_pNotify = pNotify; 
   		}
   	};
    static class SyncSearchCommand extends SyncCommand
    {
	    String m_strFrom;
	    boolean   m_bSyncChanges;
	    Vector/*<rho::String>*/ m_arSources;
	    
        public SyncSearchCommand(String from, String params, Vector arSources, boolean sync_changes, int nProgressStep)
	    {
        	super(scSearchOne,params,nProgressStep, false, "");
		    m_strFrom = from;
		    m_bSyncChanges = sync_changes;
		    m_arSources = arSources;
	    }
    };

	static SyncThread m_pInstance;
    SyncEngine  m_oSyncEngine;

    public static SyncThread getInstance(){ return m_pInstance; }
    public static SyncEngine getSyncEngine(){ return m_pInstance!= null ? m_pInstance.m_oSyncEngine : null; }
    public boolean isSkipDuplicateCmd() { return true; }
    
	public static SyncThread Create(RhoClassFactory factory)throws Exception
	{
	    if ( m_pInstance != null) 
	        return m_pInstance;
	
	    m_pInstance = new SyncThread(factory);
	    return m_pInstance;
	}

	public void Destroy()
	{
	    m_oSyncEngine.exitSync();
	    LOG.INFO("Stopping Sync thread");
	    stop(SYNC_WAIT_BEFOREKILL_SECONDS);
		
	    if ( ClientRegister.getInstance() != null )
	    	ClientRegister.getInstance().Destroy();
	    
	    DBAdapter.closeAll();
	    
	    m_pInstance = null;
	}

	SyncThread(RhoClassFactory factory)throws Exception
	{
		super(factory);
		super.setLogCategory(LOG.getLogCategory());
		
		if( RhoConf.getInstance().isExist("sync_poll_interval") )
			setPollInterval(RhoConf.getInstance().getInt("sync_poll_interval"));

		m_oSyncEngine = new SyncEngine();
		m_oSyncEngine.setFactory(factory);
	
	    LOG.INFO("sync_poll_interval: " + RhoConf.getInstance().getInt("sync_poll_interval"));
	    LOG.INFO("syncserver: " + RhoConf.getInstance().getString("syncserver"));
	    LOG.INFO("bulksync_state: " + RhoConf.getInstance().getInt("bulksync_state"));
	    
	    ClientRegister.Create(factory);
	    
		if ( RhoConf.getInstance().getString("syncserver").length() > 0 )
			start(epLow);
	}

    RubyValue getRetValue()
    {
    	RubyValue ret = RubyConstant.QNIL;
        if ( isNoThreadedMode()  )
        {
            ret = ObjectFactory.createString( getSyncEngine().getNotify().getNotifyBody() );
            getSyncEngine().getNotify().cleanNotifyBody();
        }

        return ret;
    }
	
    public int getLastPollInterval()
    {
    	try{
	    	long nowTime = (TimeInterval.getCurrentTime().toULong())/1000;
	    	long latestTimeUpdated = 0;
	    	
	    	Vector/*<String>*/ arPartNames = DBAdapter.getDBAllPartitionNames();
	    	for( int i = 0; i < (int)arPartNames.size(); i++ )
	    	{
	    		DBAdapter dbPart = DBAdapter.getDB((String)arPartNames.elementAt(i));
			    IDBResult res = dbPart.executeSQL("SELECT last_updated from sources");
			    for ( ; !res.isEnd(); res.next() )
			    { 
			        long timeUpdated = res.getLongByIdx(0);
			        if ( latestTimeUpdated < timeUpdated )
			        	latestTimeUpdated = timeUpdated;
			    }
	    	}
	    	
	    	return latestTimeUpdated > 0 ? (int)(nowTime-latestTimeUpdated) : 0;
    	}catch(Exception exc)
    	{
    		LOG.ERROR("isStartSyncNow failed.", exc);
    	}
    	return 0;
    }

	public void onTimeout()//throws Exception
	{
	    if ( isNoCommands() && getPollInterval()>0 )
	        addQueueCommandInt(new SyncCommand(scSyncAll,false, ""));
	}
	
	void checkShowStatus(SyncCommand oSyncCmd)
	{
		boolean bShowStatus = oSyncCmd.m_bShowStatus && !this.isNoThreadedMode();
		m_oSyncEngine.getNotify().enableReporting(bShowStatus);
		if (m_oSyncEngine.getNotify().isReportingEnabled())
			m_statusListener.createStatusPopup(RhoAppAdapter.getMessageText("syncronizing_data"));
	}	
	
	public void processCommand(IQueueCommand pCmd)
	{
		SyncCommand oSyncCmd = (SyncCommand)pCmd;
	    switch(oSyncCmd.m_nCmdCode)
	    {
	    case scSyncAll:
	    	checkShowStatus(oSyncCmd);
	        m_oSyncEngine.doSyncAllSources(oSyncCmd.m_strQueryParams);
	        break;
        case scSyncOne:
	        {
				checkShowStatus(oSyncCmd);
	            m_oSyncEngine.doSyncSource(new SyncEngine.SourceID(oSyncCmd.m_nCmdParam,oSyncCmd.m_strCmdParam), oSyncCmd.m_strQueryParams);
	        }
	        break;
	        
	    case scSearchOne:
		    {
				checkShowStatus(oSyncCmd);
	            m_oSyncEngine.doSearch( ((SyncSearchCommand)oSyncCmd).m_arSources, oSyncCmd.m_strCmdParam, 
	                    ((SyncSearchCommand)oSyncCmd).m_strFrom, ((SyncSearchCommand)oSyncCmd).m_bSyncChanges,
	                    oSyncCmd.m_nCmdParam);
		    }
	        break;
	        
	    case scLogin:
	    	{
	    		SyncLoginCommand oLoginCmd = (SyncLoginCommand)oSyncCmd;
	    		checkShowStatus(oSyncCmd);
	    		m_oSyncEngine.login(oLoginCmd.m_strName, oLoginCmd.m_strPassword, oLoginCmd.m_pNotify );
	    	}
	        break;
	        
	    }
	}

	static ISyncStatusListener m_statusListener = null;
	public boolean setStatusListener(ISyncStatusListener listener) {
		m_statusListener = listener;
		if (m_oSyncEngine != null) {
			m_oSyncEngine.getNotify().setSyncStatusListener(listener);
			return true;
		}
		return false;
	}
	
	public void setPollInterval(int nInterval)
	{ 
	    //if ( m_nPollInterval == 0 )
	    //    m_oSyncEngine.stopSync();
	    
	    super.setPollInterval(nInterval);	    
	}
	
	public static void doSyncAllSources(boolean bShowStatus)
	{
		getInstance().addQueueCommand(new SyncCommand(SyncThread.scSyncAll,bShowStatus, ""));
	}
	
	public static void doSyncSourceByName(String strSrcName, boolean bShowStatus)
	{
		if (bShowStatus&&(m_statusListener != null)) {
			m_statusListener.createStatusPopup(RhoAppAdapter.getMessageText("syncronizing_data"));
		}
	    getInstance().addQueueCommand(new SyncCommand(SyncThread.scSyncOne, strSrcName, (int)0, bShowStatus, "") );		
	}
	
	public static void stopSync()throws Exception
	{
		SyncThread.getInstance().stopAll();		
	}
	
	public void stopAll()throws Exception
	{
		LOG.INFO("STOP sync");
		
		if ( getSyncEngine().isSyncing() )
		{
			LOG.INFO("STOP sync in progress.");

            synchronized(getCommandLock())
	        {
	            getCommands().clear();
	        }
			
			getSyncEngine().stopSyncByUser();
			
			//don't wait if calling from notify callback
			if ( getSyncEngine().getNotify().isInsideCallback() )
			{
				LOG.INFO("STOP sync called inside notify.");
				return;
			}
			
			getInstance().stopWait();
			
			int nWait = 0;
			//while( nWait < 30000 && getSyncEngine().getState() != SyncEngine.esNone )
			while( nWait < 30000 && isWaiting() /*DBAdapter.isAnyInsideTransaction()*/ )
				try{ Thread.sleep(100); nWait += 100; }catch(Exception e){}
				
			//if (getSyncEngine().getState() != SyncEngine.esNone)
			if ( /*DBAdapter.isAnyInsideTransaction()*/ isWaiting() )
			{
				getSyncEngine().exitSync();
				getInstance().stop(0);
				RhoClassFactory ptrFactory = getInstance().getFactory();
				m_pInstance = null;
				
				Create(ptrFactory);
			}
		}
	}
	
	public void addobjectnotify_bysrcname(String strSrcName, String strObject)
	{
		getSyncEngine().getNotify().addObjectNotify(strSrcName, strObject);
	}
	
	public static void initMethods(RubyModule klass) {
		klass.getSingletonClass().defineMethod("dosync", new RubyVarArgMethod(){ 
			protected RubyValue run(RubyValue receiver, RubyArray args, RubyBlock block )
			{
				try 
				{
					boolean bShowStatus = true;
					String query_params = "";
					if ( args != null && args.size() > 0 )
					{
						String str = args.get(0).asString();
						bShowStatus = args.get(0).equals(RubyConstant.QTRUE)||"true".equalsIgnoreCase(str);
					}
					if ( args != null &&  args.size() > 1 )
						query_params = args.get(1).asString();
					
					getInstance().addQueueCommand(new SyncCommand(SyncThread.scSyncAll,bShowStatus, query_params));
				} catch(Exception e) {
					LOG.ERROR("dosync failed", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
				return getInstance().getRetValue();
			}
		});		
		klass.getSingletonClass().defineMethod("dosync_source", new RubyVarArgMethod(){ 
			protected RubyValue run(RubyValue receiver, RubyArray args, RubyBlock block )
			{
				try {
					int nSrcID = 0;
					String strName = "";
					if ( args.get(0) instanceof RubyFixnum )
						nSrcID = args.get(0).toInt();
					else
						strName = args.get(0).toStr();

					boolean bShowStatus = true;
					String query_params = "";
					if ( args.size() > 1 )
					{
						String str = args.get(0).asString();
						bShowStatus = args.get(0).equals(RubyConstant.QTRUE)||"true".equalsIgnoreCase(str);
					}
					
					if ( args.size() > 2 )
						query_params = args.get(2).asString();
					
					getInstance().addQueueCommand(new SyncCommand(SyncThread.scSyncOne, strName, nSrcID, bShowStatus, query_params) );
				} catch(Exception e) {
					LOG.ERROR("dosync_source failed", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
				return getInstance().getRetValue();
			}
		});
		
		klass.getSingletonClass().defineMethod("dosearch",
			new RubyVarArgMethod() {
				protected RubyValue run(RubyValue receiver, RubyArray args, RubyBlock block) {
					if ( args.size() != 7 )
						throw new RubyException(RubyRuntime.ArgumentErrorClass, 
								"in SyncEngine.dosearch_source: wrong number of arguments ( " + args.size() + " for " + 7 + " )");			
					
					try{
						Vector arSources = RhoRuby.makeVectorStringFromArray(args.get(0));
						
						String from = args.get(1).toStr();
						String params = args.get(2).toStr();
						
						String str = args.get(3).asString();
						int nProgressStep = args.get(4).toInt();
						String callback = args.get(5) != RubyConstant.QNIL ? args.get(5).toStr() : "";
						String callback_params = args.get(6) != RubyConstant.QNIL ? args.get(6).toStr() : "";
						
						boolean bSearchSyncChanges = args.get(3).equals(RubyConstant.QTRUE)||"true".equalsIgnoreCase(str);
						stopSync();

						if ( callback != null && callback.length() > 0 )
							getSyncEngine().getNotify().setSearchNotification(callback, callback_params);
						
						getInstance().addQueueCommand(new SyncSearchCommand(from,params,arSources,bSearchSyncChanges, nProgressStep) );
					}catch(Exception e)
					{
						LOG.ERROR("SyncEngine.login", e);
						RhoRuby.raise_RhoError(RhoAppAdapter.ERR_RUNTIME);
					}
					
					return getInstance().getRetValue();
				    
				}
			});
		
		klass.getSingletonClass().defineMethod("stop_sync", new RubyNoArgMethod() {
			protected RubyValue run(RubyValue receiver, RubyBlock block) {
				try{
					stopSync();
				}catch(Exception e)
				{
					LOG.ERROR("stop_sync failed", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
				
				return RubyConstant.QNIL;
			}
		});

		klass.getSingletonClass().defineMethod("is_syncing", new RubyNoArgMethod() {
			protected RubyValue run(RubyValue receiver, RubyBlock block) {
				try{
					return ObjectFactory.createBoolean( getSyncEngine().isSyncing() );
				}catch(Exception e)
				{
					LOG.ERROR("is_syncing failed", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
			}
		});
		
		klass.getSingletonClass().defineMethod("login",
				new RubyVarArgMethod() {
					protected RubyValue run(RubyValue receiver, RubyArray args, RubyBlock block) {
						if ( args.size() != 3 )
							throw new RubyException(RubyRuntime.ArgumentErrorClass, 
									"in SyncEngine.login: wrong number of arguments ( " + args.size() + " for " + 3 + " )");			
						
						try{
							String name = args.get(0).toStr();
							String password = args.get(1).toStr();
							String callback = args.get(2).toStr();
							
							stopSync();
							
							getInstance().addQueueCommand(new SyncLoginCommand(name, password, callback,
									new SyncNotify.SyncNotification(callback, "", false) ) );
						}catch(Exception e)
						{
							LOG.ERROR("SyncEngine.login", e);
							RhoRuby.raise_RhoError(RhoAppAdapter.ERR_RUNTIME);
						}
						
						return getInstance().getRetValue();
					    
					}
				});
		
		klass.getSingletonClass().defineMethod("logged_in",
			new RubyNoArgMethod() {
				protected RubyValue run(RubyValue receiver, RubyBlock block) {
					DBAdapter db = DBAdapter.getUserDB();

					try{
					    return getSyncEngine().isLoggedIn() ? 
					    		ObjectFactory.createInteger(1) : ObjectFactory.createInteger(0);
					}catch(Exception e)
					{
						LOG.ERROR("logged_in failed", e);
						throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
					}
				    
				}
			});
		
		klass.getSingletonClass().defineMethod("logout",
			new RubyNoArgMethod() {
				protected RubyValue run(RubyValue receiver, RubyBlock block) {
					DBAdapter db = DBAdapter.getUserDB();

					try{
						stopSync();
					    getSyncEngine().logout_int();
					}catch(Exception e)
					{
						LOG.ERROR("logout failed", e);
						throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
					}
					
				    return RubyConstant.QNIL;
				}
			});
		
		klass.getSingletonClass().defineMethod("set_notification",
			new RubyVarArgMethod() {
				protected RubyValue run(RubyValue receiver, RubyArray args, RubyBlock block) {
					
					try{
						int source_id = args.get(0).toInt();
						String url = args.get(1).toStr();
						String params = args.get(2).toStr();
						getSyncEngine().getNotify().setSyncNotification(source_id, 
								new SyncNotify.SyncNotification(url, params != null ? params : "", source_id != -1));
					}catch(Exception e)
					{
						LOG.ERROR("set_notification failed", e);
						throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
					}
					return RubyConstant.QNIL;
				}
			});
		klass.getSingletonClass().defineMethod("clear_notification",
			new RubyOneArgMethod() {
				protected RubyValue run(RubyValue receiver, RubyValue arg1, RubyBlock block) {
					try{
						int source_id = arg1.toInt();
						getSyncEngine().getNotify().clearSyncNotification(source_id);
					}catch(Exception e)
					{
						LOG.ERROR("clear_notification failed", e);
						throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
					}
					
					
					return RubyConstant.QNIL;
				}
			});
		
		klass.getSingletonClass().defineMethod("set_pollinterval",
			new RubyOneArgMethod() {
				protected RubyValue run(RubyValue receiver, RubyValue arg1, RubyBlock block) {
					try{
						int nOldInterval = getInstance().getPollInterval(); 
						int nInterval = arg1.toInt();
						getInstance().setPollInterval(nInterval);
						
						
						return ObjectFactory.createInteger(nOldInterval);
					}catch(Exception e)
					{
						LOG.ERROR("set_pollinterval failed", e);
						throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
					}
				}
			});
		klass.getSingletonClass().defineMethod("get_pollinterval",
				new RubyNoArgMethod() {
					protected RubyValue run(RubyValue receiver, RubyBlock block) {
						try{
							int nOldInterval = getInstance().getPollInterval(); 
							return ObjectFactory.createInteger(nOldInterval);
						}catch(Exception e)
						{
							LOG.ERROR("set_pollinterval failed", e);
							throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
						}
					}
				});
		
		klass.getSingletonClass().defineMethod("set_syncserver",
				new RubyOneArgMethod() {
					protected RubyValue run(RubyValue receiver, RubyValue arg1, RubyBlock block) {
						try{
							String syncserver = arg1.toStr();
							
							stopSync();							
							getSyncEngine().setSyncServer(syncserver);
							
						    if ( syncserver != null && syncserver.length() > 0 )
						    {
						        SyncThread.getInstance().start(SyncThread.epLow);
						    	if ( ClientRegister.getInstance() != null )
						    		ClientRegister.getInstance().startUp();	    	
						    }
						    else
						    {
						    	//DO NOT STOP thread. because they cannot be restarted
						        //SyncThread.getInstance().stop(SYNC_WAIT_BEFOREKILL_SECONDS);
						    	//if ( ClientRegister.getInstance() != null )
						    	//	ClientRegister.getInstance().stop(SYNC_WAIT_BEFOREKILL_SECONDS);
						    }
							
						}catch(Exception e)
						{
							LOG.ERROR("set_syncserver failed", e);
							throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
						}
						
						return RubyConstant.QNIL;
					}
			});
		
		klass.getSingletonClass().defineMethod("get_src_attrs",
				new RubyTwoArgMethod() {
					protected RubyValue run(RubyValue receiver, RubyValue arg0, RubyValue arg1, RubyBlock block) {
						try{
							//String strPartition = arg0.toStr(); 
							//int nSrcID = arg1.toInt();
							//return DBAdapter.getDB(strPartition).getAttrMgr().getAttrsBySrc(nSrcID);
							return RubyConstant.QNIL;
						}catch(Exception e)
						{
							LOG.ERROR("get_src_attrs failed", e);
							throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
						}
					}
			});

		klass.getSingletonClass().defineMethod("is_blob_attr",
				new RubyVarArgMethod() {
					protected RubyValue run(RubyValue receiver, RubyArray args, RubyBlock block) {
						try{
							String strPartition = args.get(0).toStr(); 
							Integer nSrcID = new Integer(args.get(1).toInt());
							String strAttrName = args.get(2).toStr();
							boolean bExists = DBAdapter.getDB(strPartition).getAttrMgr().isBlobAttr(nSrcID, strAttrName);
							return ObjectFactory.createBoolean(bExists);
						}catch(Exception e)
						{
							LOG.ERROR("get_src_attrs failed", e);
							throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
						}
					}
			});
		
		klass.getSingletonClass().defineMethod("set_objectnotify_url",
				new RubyOneArgMethod() {
					protected RubyValue run(RubyValue receiver, RubyValue arg1, RubyBlock block) {
						try{
							String url = arg1.toStr();
							SyncNotify.setObjectNotifyUrl(url);
						}catch(Exception e)
						{
							LOG.ERROR("set_objectnotify_url failed", e);
							throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
						}
						
						return RubyConstant.QNIL;
					}
			});

		klass.getSingletonClass().defineMethod("add_objectnotify",
				new RubyTwoArgMethod() {
					protected RubyValue run(RubyValue receiver, RubyValue arg1, RubyValue arg2, RubyBlock block) {
						try{
							Integer nSrcID = new Integer(arg1.toInt());
							String strObject = arg2.toStr();
							
							getSyncEngine().getNotify().addObjectNotify(nSrcID, strObject);
						}catch(Exception e)
						{
							LOG.ERROR("add_objectnotify failed", e);
							throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
						}
						
						return RubyConstant.QNIL;
					}
			});
		klass.getSingletonClass().defineMethod("clean_objectnotify",
				new RubyNoArgMethod() {
					protected RubyValue run(RubyValue receiver, RubyBlock block) {
						try{
							getSyncEngine().getNotify().cleanObjectNotifications();
						}catch(Exception e)
						{
							LOG.ERROR("clean_objectnotify failed", e);
							throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
						}
						
						return RubyConstant.QNIL;
					}
			});
		
		klass.getSingletonClass().defineMethod("get_lastsync_objectcount",
				new RubyOneArgMethod() {
					protected RubyValue run(RubyValue receiver, RubyValue arg1, RubyBlock block) {
						try{
							Integer nSrcID = new Integer(arg1.toInt());
							int nCount = getSyncEngine().getNotify().getLastSyncObjectCount(nSrcID);
							
							return ObjectFactory.createInteger(nCount);
						}catch(Exception e)
						{
							LOG.ERROR("get_lastsync_objectcount failed", e);
							throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
						}
					}
			});
		klass.getSingletonClass().defineMethod("get_pagesize",
				new RubyNoArgMethod() {
					protected RubyValue run(RubyValue receiver, RubyBlock block) {
						try{
							return ObjectFactory.createInteger(getSyncEngine().getSyncPageSize());
						}catch(Exception e)
						{
							LOG.ERROR("get_pagesize failed", e);
							throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
						}
					}
			});
		
		klass.getSingletonClass().defineMethod("set_pagesize",
				new RubyOneArgMethod() {
					protected RubyValue run(RubyValue receiver, RubyValue arg1, RubyBlock block) {
						try{
							getSyncEngine().setSyncPageSize(arg1.toInt());
						}catch(Exception e)
						{
							LOG.ERROR("set_pagesize failed", e);
							throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
						}
						
						return RubyConstant.QNIL;
					}
			});

		klass.getSingletonClass().defineMethod("set_threaded_mode",
				new RubyOneArgMethod() {
					protected RubyValue run(RubyValue receiver, RubyValue arg1, RubyBlock block) {
						try{
							boolean bThreadMode = arg1 == RubyConstant.QTRUE;
							getInstance().setNonThreadedMode(!bThreadMode);
							getSyncEngine().setNonThreadedMode(!bThreadMode);
						}catch(Exception e)
						{
							LOG.ERROR("set_threaded_mode failed", e);
							throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
						}
						
						return RubyConstant.QNIL;
					}
			});

		klass.getSingletonClass().defineMethod("enable_status_popup",
				new RubyOneArgMethod() {
					protected RubyValue run(RubyValue receiver, RubyValue arg1, RubyBlock block) {
						try{
							boolean bEnable = arg1 == RubyConstant.QTRUE;
							getSyncEngine().getNotify().enableStatusPopup(bEnable);
						}catch(Exception e)
						{
							LOG.ERROR("enable_status_popup failed", e);
							throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
						}
						
						return RubyConstant.QNIL;
					}
			});

		klass.getSingletonClass().defineMethod("set_source_property",
				new RubyVarArgMethod() {
					protected RubyValue run(RubyValue receiver, RubyArray args, RubyBlock block) {
						try{
							Integer nSrcID = new Integer(args.get(0).toInt());
							String strPropName = args.get(1).toStr(); 
							String strPropValue = args.get(2).toStr();
							
							SyncEngine.getSourceOptions().setProperty(nSrcID, strPropName, strPropValue);
							
							return RubyConstant.QNIL;
						}catch(Exception e)
						{
							LOG.ERROR("set_source_property failed", e);
							throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
						}
					}
			});
		klass.getSingletonClass().defineMethod("get_source_property",
				new RubyTwoArgMethod() {
					protected RubyValue run(RubyValue receiver, RubyValue arg0, RubyValue arg1, RubyBlock block) {
						try{
							Integer nSrcID = new Integer(arg0.toInt());
							String strPropName = arg1.toStr();
							
							return ObjectFactory.createString(SyncEngine.getSourceOptions().getProperty(nSrcID, strPropName) );
						}catch(Exception e)
						{
							LOG.ERROR("get_src_attrs failed", e);
							throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
						}
					}
			});
		
		klass.getSingletonClass().defineMethod("set_ssl_verify_peer",
				new RubyOneArgMethod() {
					protected RubyValue run(RubyValue receiver, RubyValue arg1, RubyBlock block) {
						try{
							boolean bVerify = arg1 == RubyConstant.QTRUE;
							getSyncEngine().getNet().sslVerifyPeer(bVerify);
							getSyncEngine().getNetClientID().sslVerifyPeer(bVerify);
						}catch(Exception e)
						{
							LOG.ERROR("set_ssl_verify_peer failed", e);
							throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
						}
						
						return RubyConstant.QNIL;
					}
			});

		klass.getSingletonClass().defineMethod("update_blob_attribs",
				new RubyTwoArgMethod() {
					protected RubyValue run(RubyValue receiver, RubyValue arg1, RubyValue arg2, RubyBlock block) {
						try{
							String strPartition = arg1.toStr();
							//Integer nSrcID = new Integer(arg2.toInt());
							DBAdapter db = DBAdapter.getDB(strPartition); 
							db.getAttrMgr().loadBlobAttrs(db);
							return RubyConstant.QNIL;
						}catch(Exception e)
						{
							LOG.ERROR("add_objectnotify failed", e);
							throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
						}
						
						
					}
			});
		
	}

}
