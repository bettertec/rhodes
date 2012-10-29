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

import java.io.InputStream;

import javax.microedition.io.StreamConnection;

import com.rho.RhoClassFactory;
import com.rho.RhoEmptyLogger;
import com.rho.RhoLogger;
import com.rho.RhoThread;
import com.rho.RhodesApp;
import com.rho.ThreadQueue;
import com.rho.ThreadQueue.IQueueCommand;
import com.rho.net.URI;

import net.rim.blackberry.api.push.*;
import net.rim.device.api.io.http.PushInputStream;
import net.rim.device.api.system.ApplicationDescriptor;
import net.rim.device.api.ui.UiApplication;
import net.rim.device.api.util.DataBuffer;
/**
 *
 */
public class RhodesApplicationPlatform extends UiApplication implements PushApplication
{
	private static final RhoLogger LOG = RhoLogger.RHO_STRIP_LOG ? new RhoEmptyLogger() : 
		new RhoLogger("RhodesPUSH");
	
	private static final int CHUNK_SIZE = 256;
	PushMessageThread m_PushMessageThread;

    private static class PushStatusCommand implements IQueueCommand
    {
		String m_strStatus; 
    	
		public PushStatusCommand(String strStatus) 
		{
			m_strStatus = strStatus;
		}
		public boolean equals(IQueueCommand cmd){return false;}
		public void cancel(){}
		
		public void execute()
		{
	        try
	        {
	        	RhodesApp.getInstance().callPushCallback(m_strStatus);
	        }catch(Exception exc)
	        {
	        	LOG.ERROR("callPushCallback failed.", exc);
	        }
		}
    }
    
    private static class PushMessageCommand implements IQueueCommand
    {
		PushInputStream m_inputStream; 
		StreamConnection m_conn;
    	
		public PushMessageCommand(PushInputStream inputStream, StreamConnection conn) 
		{
			m_inputStream = inputStream;
			m_conn = conn;
		}
		public boolean equals(IQueueCommand cmd){return false;}
		public void cancel(){}
		
		public void execute()
		{
			InputStream input = null;

			try
			{
				input = m_conn.openInputStream();
                DataBuffer db = new DataBuffer();
                byte[] data = new byte[CHUNK_SIZE];
                int chunk = 0;
                
                while ( -1 != (chunk = input.read(data)) )
                {
                    db.write(data, 0, chunk);
                }
			
                data = db.getArray();
                
	            try
	            {
	            	rhomobile.PushListeningThread.processPushMessage(data, db.getLength());
	            }catch(Exception exc)
	            {
	            	LOG.ERROR("processPushMessage failed.Data: " + new String(data), exc);
	            }catch(Throwable th)
	            {
	            	LOG.ERROR("processPushMessage crashed.Data: " + new String(data), th);
	            }
            }catch(Exception exc)
            {
            	LOG.ERROR("Read PUSH message failed.", exc);
            }catch(Throwable th)
            {
            	LOG.ERROR("Read PUSH message crashed.", th);
            }finally
            {
            	try
            	{
	                m_inputStream.accept();
	
	                if ( input != null)
	                	input.close();
	                
	                m_inputStream.close();
	                m_conn.close();
            	}catch(Exception exc)
            	{
            		
            	}
            }
		}		
		
    }
	
	private static class PushMessageThread extends ThreadQueue
	{
		public PushMessageThread() 
		{
			super(new RhoClassFactory());
			
	        super.setLogCategory(LOG.getLogCategory());

	        setPollInterval(QUEUE_POLL_INTERVAL_INFINITE);
	        
	        start(epLow);
		}
		
		public void processCommand(IQueueCommand pCmd)
		{
			((PushMessageCommand)pCmd).execute();
		}
	}
	
	public void onMessage(PushInputStream inputStream, StreamConnection conn) 
	{
		LOG.INFO("onMessage");
		m_PushMessageThread.addQueueCommand( new PushMessageCommand(inputStream, conn) );
	}

	public void onStatusChange(PushApplicationStatus status) 
	{
        String strError = status.getError();

        String strStatus = "", strRegReason = "";

        switch( status.getStatus() ) 
        {
            case PushApplicationStatus.STATUS_ACTIVE:
            	strStatus = "Active";
                break;
            case PushApplicationStatus.STATUS_FAILED:
            	strStatus = "Failed";
                break;
            case PushApplicationStatus.STATUS_NOT_REGISTERED:
            	strStatus = "Not Registered";
                break;
            case PushApplicationStatus.STATUS_PENDING:
                strStatus = "Pending";
                break;
        }
        
        switch( status.getReason() ) {
	        case PushApplicationStatus.REASON_SIM_CHANGE:
	        	strRegReason = "Status change was initiated by SIM card change";
	            break;
	        case PushApplicationStatus.REASON_API_CALL:
	        	strRegReason = "Status change was initiated by API call";
	            break;
	        case PushApplicationStatus.REASON_REJECTED_BY_SERVER:
	        	strRegReason = "Registration was rejected by server ";
	        	break;
	        case PushApplicationStatus.REASON_NETWORK_ERROR:
	        	strRegReason = "Communication failed due to network error ";
	        	break;
	        case PushApplicationStatus.REASON_INVALID_PARAMETERS:
	        	strRegReason = "Invalid parameters were provided that caused registration failure ";
	            break;
        }
        
        String strMsg = "status="+URI.urlEncode(strStatus)+"&reason="+URI.urlEncode(strRegReason);
        
        if ( strError != null && strError.length() > 0 )
        	strMsg += "&error="+URI.urlEncode(strError);

		LOG.INFO("onStatusChange : " + strMsg);
		
		if ( m_PushMessageThread != null )
			m_PushMessageThread.addQueueCommand( new PushStatusCommand(strMsg) );
	}
	
	public void onPlatformActivate()
	{
		if ( com.rho.Capabilities.ENABLE_PUSH )
		{
			if ( !com.rho.sync.ClientRegister.isPushServiceEnabled() )
				return;
			
			getUiApplication().invokeLater( new Runnable()
	        {
	            public void run()
	            {
	                try
	                {
					    m_PushMessageThread = new PushMessageThread();
					    String strAppName = RhoConf.getInstance().getString("push_service_appname");
    			        String strUrl = RhoConf.getInstance().getString("push_service_url");
					    if ( strAppName != null && strAppName.length() > 0 && strUrl != null && strUrl.length() > 0 )
					    {
					        int nPort = RhoConf.getInstance().getInt("push_service_port");
					        byte btServiceType = RhoConf.getInstance().getString("push_service_type").equalsIgnoreCase("BPAS") ?
							        PushApplicationDescriptor.SERVER_TYPE_BPAS : PushApplicationDescriptor.SERVER_TYPE_NONE;
        					
					        PushApplicationDescriptor pad = new PushApplicationDescriptor(
							        strAppName, nPort, strUrl, btServiceType, 
							        ApplicationDescriptor.currentApplicationDescriptor());
        					
					        PushApplicationStatus status = PushApplicationRegistry.getStatus(pad);
        			
					        LOG.INFO("registerPushService status : " + status.toString());
        			
					        if (status.getStatus() != PushApplicationStatus.STATUS_ACTIVE && status.getStatus() != PushApplicationStatus.STATUS_PENDING)
					        {
        						
						        LOG.INFO("registerPushService registering push");
        						
						        PushApplicationRegistry.registerApplication(pad);
					        }
					    }
					}catch(Exception exc)
					{
	                    LOG.ERROR("registerPushService failed.", exc);					
					}catch(Throwable t)
					{
					    LOG.ERROR("registerPushService crashed.", t);
					}
	            }
	        });
		}
	}
	
	public void onPlatformClose()
	{
		if ( m_PushMessageThread != null )
			m_PushMessageThread.stop(3);
		
		m_PushMessageThread = null;
	}
	
}
