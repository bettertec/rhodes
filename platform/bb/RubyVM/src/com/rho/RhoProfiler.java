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

package com.rho;

import java.util.Calendar;

public class RhoProfiler {

	public static boolean RHO_STRIP_PROFILER = true;
	
	public static final String FILE_READ = "FileRead";
	public static final String FILE_WRITE = "FileWrite";
	public static final String FILE_SYNC = "FileSync";
	public static final String FILE_SET_SIZE = "FileSetSize";
	public static final String FILE_DELETE = "FileDelete";
	public static final String FILE_RENAME = "FileRename";
	
	//public static final String SQL_SELECT = "SqlSelect";
	//public static final String SQL_FIND_NODE = "SqlFindNode";
	//public static final String SQL_GET_OBJECT_FROM_CACHE = "SqlGetObjectFromCache";
	//public static final String SQL_READ_OBJECT = "SqlReadObject";
	//public static final String SQL_GET_OBJECT_FROM_STORE = "SqlGetObjectFromStore";
	//public static final String SQL_PUT_OBJECT_TO_CACHE = "SqlPutObjectToCache";
	//public static final String SQL_READ_DATA = "SqlReadData";
	//public static final String SQL_READ_STRING = "SqlReadString";
	
	private static final RhoLogger LOG = new RhoLogger("PROFILER");
	
    private static class CCounter
    {
        long  m_startTime;
        boolean m_bWasStarted;
        
        CCounter(boolean bStart){
        	m_bWasStarted = false;
            if ( bStart )
                start();     
        }

        boolean isGlobal(){ return false;}

        long getCurTime(){
        	return Calendar.getInstance().getTime().getTime();
        }
        void start(){ 
        	m_startTime = getCurTime(); 
        	m_bWasStarted = true; 
        }
        
        long stop(){
            if ( m_startTime == 0 )
                return m_startTime;

            long res = getCurTime()-m_startTime;
            m_startTime = 0;
            return res;
        }
        long flush(){ return stop(); }

        boolean isWasStarted(){ return m_bWasStarted;}
    };

    private class CGlobalCounter extends CCounter
    {
    	long m_sumGlobal;
        
        CGlobalCounter() { super(false); }
        boolean isGlobal(){ return true;}

        long stop(){
            m_sumGlobal += super.stop();

            return m_sumGlobal;
        }
        long flush(){ 
        	long res = stop(); 
            m_sumGlobal = 0;
            return res;
        }

    };
	
    private static java.util.Hashtable m_mapCounters = new java.util.Hashtable();
    
	public void START(String szCounterName){
	    CCounter pCounter = (CCounter)m_mapCounters.get(szCounterName);
	    if ( pCounter==null || !pCounter.isWasStarted() )
	        LOG.INFO( szCounterName + " : START" );

	    if ( pCounter == null)
	        m_mapCounters.put( szCounterName, new CCounter(true) ); 
	    else
	        pCounter.start();
	}

	public void START_CREATED(String szCounterName){
	    CCounter pCounter = (CCounter)m_mapCounters.get(szCounterName);
	    if ( pCounter == null )
	    	return;
	    
	    if ( !pCounter.isWasStarted() )
	        LOG.INFO( szCounterName + " : START" );

        pCounter.start();
	}
	
	private String intervalToString(long nInterval){
        long nMin = nInterval/(60*1000);
        long nSec = (nInterval - nMin*(60*1000))/1000;
        long mSec = nInterval - nSec*1000 - nMin*(60*1000);

        String strTime = Long.toString(nMin) + ":" +
        	Long.toString(nSec) + ":" +
        	Long.toString(mSec);
        
		return strTime;
	}
	
	private void stopCounter(String szCounterName, boolean bDestroy){
	    CCounter pCounter = (CCounter)m_mapCounters.get(szCounterName);
	    if ( pCounter==null ){
	        LOG.ERROR( szCounterName + " : Cannot find counter." );
	        return;
	    }

	    if ( bDestroy || !pCounter.isGlobal() )
	    {
	    	long oInterval = pCounter.stop();
	        LOG.INFO( szCounterName + " (" + intervalToString(oInterval) + ") : STOP" );

	        m_mapCounters.remove(szCounterName);
	    }else
	        pCounter.stop();
		
	}

	public void STOP(String szCounterName){
	    stopCounter(szCounterName,false);
	}
	
	//Global accumulative counters
	public void CREATE_COUNTER(String szCounterName){
	    m_mapCounters.put(szCounterName, new CGlobalCounter() ); 
	}
	
	public void DESTROY_COUNTER(String szCounterName){
	    stopCounter( szCounterName, true );
	}
	
	public void FLUSH_COUNTER(String szCounterName, String msg){
	    CCounter pCounter = (CCounter)m_mapCounters.get(szCounterName);
	    if ( pCounter==null ){
	        LOG.ERROR( szCounterName + " : Cannot find counter." );
	        return;
	    }

	    long oInterval = pCounter.flush();
	    LOG.INFO( szCounterName + (msg !=null&&msg.length()>0 ? " - " : "" ) + 
	    	(msg !=null&&msg.length()>0 ? msg : "" ) +
	        " (" + intervalToString(oInterval) + ") : STOP" );
	}	
	
	/*
	public void createSqlCounters() {
		CREATE_COUNTER(SQL_SELECT);
		CREATE_COUNTER(SQL_FIND_NODE);
		CREATE_COUNTER(SQL_GET_OBJECT_FROM_CACHE);
		CREATE_COUNTER(SQL_READ_OBJECT);
		CREATE_COUNTER(SQL_GET_OBJECT_FROM_STORE);
		CREATE_COUNTER(SQL_PUT_OBJECT_TO_CACHE);
		CREATE_COUNTER(SQL_READ_DATA);
		//CREATE_COUNTER(SQL_READ_STRING);
	}
	
	public void destroySqlCounters() {
		DESTROY_COUNTER(SQL_SELECT);
		DESTROY_COUNTER(SQL_FIND_NODE);
		DESTROY_COUNTER(SQL_GET_OBJECT_FROM_CACHE);
		DESTROY_COUNTER(SQL_READ_OBJECT);
		DESTROY_COUNTER(SQL_GET_OBJECT_FROM_STORE);
		DESTROY_COUNTER(SQL_PUT_OBJECT_TO_CACHE);
		DESTROY_COUNTER(SQL_READ_DATA);
		//DESTROY_COUNTER(SQL_READ_STRING);
	}
	
	public void flushSqlCounters(String msg) {
		FLUSH_COUNTER(SQL_SELECT, msg);
		FLUSH_COUNTER(SQL_FIND_NODE, msg);
		FLUSH_COUNTER(SQL_GET_OBJECT_FROM_CACHE, msg);
		FLUSH_COUNTER(SQL_READ_OBJECT, msg);
		FLUSH_COUNTER(SQL_GET_OBJECT_FROM_STORE, msg);
		FLUSH_COUNTER(SQL_PUT_OBJECT_TO_CACHE, msg);
		FLUSH_COUNTER(SQL_READ_DATA, msg);
		//FLUSH_COUNTER(SQL_READ_STRING, msg);
	}
	*/
}
