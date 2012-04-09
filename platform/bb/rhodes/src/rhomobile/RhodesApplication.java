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

package rhomobile;

import j2me.util.LinkedList;
import j2me.util.StringParser;

import java.io.IOException;
import java.io.InputStream;
import java.util.Date;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Vector;

import javax.microedition.io.HttpConnection;

import net.rim.blackberry.api.browser.Browser;
import net.rim.blackberry.api.browser.BrowserSession;
import net.rim.blackberry.api.invoke.Invoke;
import net.rim.device.api.browser.field.RenderingOptions;
import net.rim.device.api.io.http.HttpHeaders;
import net.rim.device.api.system.Application;
import net.rim.device.api.system.ApplicationManager;
import net.rim.device.api.system.Bitmap;
import net.rim.device.api.system.Characters;
import net.rim.device.api.system.Display;
import net.rim.device.api.system.EncodedImage;
import net.rim.device.api.system.KeyListener;
import net.rim.device.api.system.SystemListener;
//import javax.microedition.io.file.FileSystemListener;
import net.rim.device.api.system.TrackwheelListener;
import net.rim.device.api.ui.*;
import net.rim.device.api.ui.component.BitmapField;
import net.rim.device.api.ui.component.Dialog;
import net.rim.device.api.ui.component.Menu;
import net.rim.device.api.ui.container.PopupScreen;
import net.rim.device.api.ui.container.VerticalFieldManager;
//import net.rim.device.api.ui.container.HorizontalFieldManager;
import net.rim.device.api.ui.component.ButtonField;
import net.rim.device.api.ui.component.LabelField;
import net.rim.device.api.ui.Manager;
import net.rim.device.api.math.Fixed32;
//import net.rim.device.api.system.EventInjector.KeyCodeEvent;
import net.rim.blackberry.api.invoke.MessageArguments;
import net.rim.blackberry.api.invoke.PhoneArguments;
import net.rim.blackberry.api.mail.Address;
import net.rim.blackberry.api.mail.Message.RecipientType;

import com.rho.*;
//import com.rho.db.DBAdapter;
import com.rho.rubyext.GeoLocation;
import com.rho.net.NetResponse;
import com.rho.net.RhoConnection;
import com.rho.net.URI;
import com.rho.sync.ClientRegister;
import com.rho.sync.SyncThread;
import com.rho.sync.ISyncStatusListener;
import com.rho.file.Jsr75File;
import com.rho.RhodesApp;
import com.xruby.runtime.lang.RubyProgram;

/**
 *
 */
final public class RhodesApplication extends RhodesApplicationPlatform implements SystemListener, ISyncStatusListener
{
	// Menu Labels
	public static final String LABEL_HOME = "Home";
	public static final String LABEL_REFRESH = "Refresh";
	public static final String LABEL_BACK = "Back";
	public static final String LABEL_SYNC = "Sync";
	public static final String LABEL_OPTIONS = "Options";
	public static final String LABEL_LOG = "Log";
	public static final String LABEL_SEPARATOR = "separator";
	public static final String LABEL_CLOSE = "Close";
	public static final String LABEL_EXIT = "Exit";
	public static final String LABEL_NONE = "none";
	public static final String LABEL_COPYPASTE = "copy_paste";
	
	private static final RhoLogger LOG = RhoLogger.RHO_STRIP_LOG ? new RhoEmptyLogger() : 
		new RhoLogger("RhodesApplication");
	

	class CKeyListener  implements KeyListener{

		public boolean keyChar(char key, int status, int time) {
			if ( m_bDisableInput )
				return true;
			
	        if( key == Characters.ENTER ) {
	        
	        	return openLink();
	        	//return true;
	        }
			return false;
		}
		public boolean keyDown(int keycode, int time) {
			if ( m_bDisableInput )
				return true;
			
			int nKey = Keypad.key(keycode);
			if ( nKey == Keypad.KEY_ESCAPE )
			{
				/*if ( m_bSkipKeyPress )
					m_bSkipKeyPress = false;
				else*/	
					//back();
				RHODESAPP().navigateBack();
				return true;
			}

			return false;
		}

		public boolean keyRepeat(int keycode, int time) {return false;}
		public boolean keyStatus(int keycode, int time) {return false;}
		public boolean keyUp(int keycode, int time) {return false;}
    };

    class CTrackwheelListener implements TrackwheelListener{

		public boolean trackwheelClick(int status, int time) {
			if ( m_bDisableInput )
				return true;

			return openLink();
			//return true;
		}

		public boolean trackwheelRoll(int amount, int status, int time) 
		{
			return false;
		}
		public boolean trackwheelUnclick(int status, int time) {return false;}
    }

    public void navigateUrl(String url){
    	String strUrl = RHODESAPP().canonicalizeRhoUrl(url); 
    	PrimaryResourceFetchThread thread = new PrimaryResourceFetchThread(
        		strUrl, null, null, strUrl);
        thread.start();                       
    }

    public void navigateUrlWithEvent(String url, Object ev){
    	PrimaryResourceFetchThread thread = new PrimaryResourceFetchThread(
        		RHODESAPP().canonicalizeRhoUrl(url), null, null, ev);
        thread.start();                       
    }
    
    public void addMenuItem(String label, String value){
    	LOG.TRACE("Adding menu item: label: " + label + ", value: " + value);
    	_mainScreen.addCustomMenuItem(label, value);
    }
    
	//private String m_strAppBackUrl ="";
	
    public void resetMenuItems() {
    	_mainScreen.setMenuItems(new Vector());
    	//m_strAppBackUrl = "";
    	RHODESAPP().setAppBackUrl("");
    }

    public void postUrl(String url, String body, HttpHeaders headers) {
    	postUrl(url, body, headers, null);
    }
    
    public void postUrl(String url, String body, HttpHeaders headers, Runnable callback){
        PrimaryResourceFetchThread thread = new PrimaryResourceFetchThread(
        		RHODESAPP().canonicalizeRhoUrl(url), headers, body.getBytes(), null, callback);
        thread.setInternalRequest(true);
        thread.start();                       
    }

	public static class NetCallback
	{
		public NetResponse m_response;
		
		public void waitForResponse()
		{
			synchronized(this)
			{
				try{ this.wait(); }catch(InterruptedException exc){}
			}
		}
		
		public void setResponse(NetResponse resp)
		{
			synchronized(this)
			{
				m_response = resp;
				this.notifyAll();
			}
		}
	}
    public void postUrlWithCallback(String url, String body, HttpHeaders headers, NetCallback netCallback){
        PrimaryResourceFetchThread thread = new PrimaryResourceFetchThread(
        		RHODESAPP().canonicalizeRhoUrl(url), headers, body.getBytes(), null);
        thread.setNetCallback(netCallback);
        thread.start();                       
    }
    
    void saveCurrentLocation(String url) 
    {
    	if (RhoConf.getInstance().getBool("KeepTrackOfLastVisitedPage")) {
			RhoConf.getInstance().setString("LastVisitedPage",url,true);
			LOG.TRACE("Saved LastVisitedPage: " + url);
		}   	
    }
/*
    boolean restoreLocation() {
    	LOG.TRACE("Restore Location to LastVisitedPage");
    	if (RhoConf.getInstance().getBool("KeepTrackOfLastVisitedPage")) {
			String url = RhoConf.getInstance().getString("LastVisitedPage");
			if (url.length()>0) {
				LOG.TRACE("Navigating to LastVisitedPage: " + url);
				
				if ( _history.size() == 0 )
					_history.addElement(url);
				
				navigateUrl(url);
				return true;
			}
		} 
		return false;
    }*/
   
    public void navigateBack()
    {
    	String url = "";
    	if ( _history.size() <= 1 )
    	{
    		if ( RhoConf.getInstance().getBool("bb_disable_closebyback"))
    			return;
    		
    		_mainScreen.close();
    		return;
       	}	
    	int nPos = _history.size()-2;
    	url = (String)_history.elementAt(nPos);
    	_history.removeElementAt(nPos+1);

//		this.m_oBrowserAdapter.goBack();
    	
    	saveCurrentLocation(url);
    	navigateUrl(url);
    }
/*    
    void back()
    {
    	String url = m_strAppBackUrl;
    	if ( url.length() == 0)
    	{
	    	if ( _history.size() <= 1 )
	    	{
	    		if ( RhoConf.getInstance().getBool("bb_disable_closebyback"))
	    			return;
	    		
	    		_mainScreen.close();
	    		return;
	       	}	
	    	int nPos = _history.size()-2;
	    	url = (String)_history.elementAt(nPos);
	    	_history.removeElementAt(nPos+1);

//    		this.m_oBrowserAdapter.goBack();
    	}else if ( url.equalsIgnoreCase("close"))
    	{
    		_mainScreen.close();
    		return;
    	}else
    		addToHistory(url,null);
    	
    	saveCurrentLocation(url);
    	navigateUrl(url);
    }*/

    String removeSemicolon(String str)
    {
    	if ( str == null )
    		return null;
    	
    	int nCol = str.indexOf(';');
    	if ( nCol >= 0 )
    		return str.substring(0,nCol);
    	
    	return str;
    }
    
    private boolean isHistoryIgnore(String strUrl)
    {
    	if ( strUrl == null || strUrl.length() == 0 )
    		return true;
    	
    	if (strUrl.indexOf(':') == -1 )
    		return false;
    	
    	return !(strUrl.startsWith("http:")||strUrl.startsWith("https:"));
    }
    
    public void addToHistory(String strUrl, String refferer )
    {
    	if ( isHistoryIgnore(strUrl) )
    		return;
    	
    	strUrl = removeSemicolon(strUrl);
    	refferer = removeSemicolon(refferer);
    	
    	strUrl = RHODESAPP().canonicalizeRhoUrl(strUrl);
//        if ( !strUrl.startsWith(_httpRoot) && !isExternalUrl(strUrl) )
//        	strUrl = _httpRoot + (strUrl.startsWith("/") ? strUrl.substring(1) : strUrl);
        
    	int nPos = -1;
    	for( int i = 0; i < _history.size(); i++ ){
    		if ( strUrl.equalsIgnoreCase((String)_history.elementAt(i)) ){
    			nPos = i;
    			break;
    		}
    		/*String strUrl1 = strUrl + "/index";
    		if ( strUrl1.equalsIgnoreCase((String)_history.elementAt(i)) ){
    			nPos = i;
    			break;
    		}*/
    		if ( refferer != null && refferer.equalsIgnoreCase((String)_history.elementAt(i)) ){
    			nPos = i;
    			break;
    		}

    	}
    	if ( nPos == -1 ){
    		boolean bReplace = RhoConnection.findIndex(strUrl) != -1;

    		if ( bReplace )
    			_history.setElementAt(strUrl, _history.size()-1 );
    		else
    			_history.addElement(strUrl);
    	}
    	else{
    		_history.setSize(nPos+1);
    		_history.setElementAt(strUrl, _history.size()-1 );
    	}
    	saveCurrentLocation(strUrl);
    }

    private boolean m_bOpenLink = false;
    private String m_strGetLink, m_strEmailMenu, m_strCallMenu, m_strChangeOptionMenu;
    boolean openLink(){
    	LOG.TRACE("openLink");
    	try{
    		m_bOpenLink = true;
    		//TODO: catch by ID?    		
            if (m_strGetLink==null)
            {
		        Version.SoftVersion ver = Version.getSoftVersion();
		        if ( ver.nMajor > 4 )
		        	m_strGetLink = RhoAppAdapter.getMessageText("open_link_menu");
		        else
		        	m_strGetLink = RhoAppAdapter.getMessageText("get_link_menu");
            }

            if (m_strEmailMenu==null)
            	m_strEmailMenu = RhoAppAdapter.getMessageText("email_menu");
            if (m_strCallMenu==null)
            	m_strCallMenu = RhoAppAdapter.getMessageText("call_menu");
            if (m_strChangeOptionMenu==null)
            	m_strChangeOptionMenu = RhoAppAdapter.getMessageText("change_option_menu");
            
	    	Menu menu = _mainScreen.getMenu(0);
	        int size = menu.getSize();
	        for(int i=0; i<size; i++)
	        {
	            MenuItem item = menu.getItem(i);
	            String label = item.toString();
	            //LOG.INFO("*******openLink: " + label);
	            
	            if( label.equalsIgnoreCase(m_strGetLink) 
	                ||label.indexOf(m_strEmailMenu)>=0 || label.indexOf(m_strCallMenu)>=0 ||
	                label.equalsIgnoreCase(m_strChangeOptionMenu) )
	            {
	              item.run();
	              return true;
	            }
	        }
	        /*
	        String strMenuItems = "";
	        for(int i=0; i<size; i++)
	        {
	            MenuItem item = menu.getItem(i);
	            strMenuItems += item.toString() + ";"; 
	        }
	        
	        LOG.ERROR("Cannot find link menu item in menu: " + strMenuItems );*/
    	}finally
	    {
    		m_bOpenLink = false;
	    }
//    	MenuItem item = _mainScreen.getSavedGetLinkItem();
//    	if ( item != null ) {
//    		item.run();
//    	}
        
        return false;
    }
    
	private IBrowserAdapter m_oBrowserAdapter;
    private CMainScreen _mainScreen = null;

    private SyncStatusPopup _syncStatusPopup = null;
    
    private String _lastStatusMessage = null;
    private String _hideStatus = null;
    
    //private HttpConnection  _currentConnection;

    private Vector _history;

    private static boolean m_isFullBrowser = false;
    
    private static PushListeningThread _pushListeningThread = null;
    
    private static RhodesApplication _instance;
    
    public static RhodesApplication getInstance(){ return _instance; }
    private static RhodesApp RHODESAPP(){ return RhodesApp.getInstance(); }
    
    public static boolean isFullBrowser(){ return m_isFullBrowser; }
    
    void invalidateMainScreen()
    {
    	_mainScreen.invalidate();
    }
    
    static String m_strCmdLine = "", m_strSecurityToken = "";
    /***************************************************************************
     * Main.
     **************************************************************************/
    public static void main(String[] args)
    {
    	LOG.INFO_EVENT("main start");
    	
		try{
			if ( args != null )
			{
				for( int i = 0; i < args.length; i++)
				{
					if ( i > 0 )
						m_strCmdLine += " ";
					
					m_strCmdLine += args[i];
					
					if ( args[i].startsWith("security_token=") )
						m_strSecurityToken = args[i].substring(15);
				}
			}
			_instance = new RhodesApplication();
			LOG.INFO_EVENT( "RhodesApplication created" );
			
			_instance.enterEventDispatcher();
		}catch(Exception exc)
		{
			if ( RhoConf.getInstance() != null )
				LOG.ERROR("Error in application.", exc);
        	RhoConf.sendLog();
			
        	throw new RuntimeException("Application failed and will exit. Log will send to log server." + exc.toString());
		}catch(Throwable e)
		{
			if ( RhoConf.getInstance() != null )
				LOG.ERROR("Error in application.", e);
        	RhoConf.sendLog();
			
        	throw new RuntimeException("Application failed and will exit. Log will send to log server." + e.toString());
		}
		
		//LOG.TRACE("Rhodes MAIN exit ***--------------------------***");
    }

    void doClose(){   	
    	LOG.TRACE("Rhodes DO CLOSE ***--------------------------***");

    	onPlatformClose();
    	if ( _pushListeningThread != null )
    		_pushListeningThread.stop();
		
    	if ( ClientRegister.getInstance() != null )
    		ClientRegister.getInstance().Destroy();
    	
    	if (!m_bInDeactivate)
    		RhoRuby.rho_ruby_deactivateApp();
    	
    	RhoRuby.rho_ruby_uiDestroyed();
    	
		GeoLocation.stop();

    	if ( SyncThread.getInstance() != null )
    		SyncThread.getInstance().Destroy();

        RhoRuby.RhoRubyStop();
    	
    	try{
    		RhoClassFactory.getNetworkAccess().close();
    	}catch(IOException exc){
    		LOG.ERROR(exc);
    	}

        RhoLogger.close();
    }
    
    private int m_activateHookNo = 0;
    private Hashtable m_activateHooks;
    
    public static abstract class ActivateHook {
    	public abstract void run();
    };
    
    public int addActivateHook(ActivateHook hook) {
    	synchronized(m_activateHooks) {
	    	int no = ++m_activateHookNo;
	    	m_activateHooks.put(new Integer(no), hook);
	    	return no;
    	}
    }
    public void removeActivateHook(int no) {
    	synchronized (m_activateHooks) {
    		m_activateHooks.remove(new Integer(no));
		}
    }

    private void runActivateHooks()
    {
		synchronized(m_activateHooks) {
			if (m_activateHooks != null && m_activateHooks.size() != 0) {
				Enumeration e = m_activateHooks.elements();
				while(e.hasMoreElements()) {
					ActivateHook hook = (ActivateHook)e.nextElement();
					hook.run();
				}
				m_activateHooks.clear();
				return;
			}
		}
    }
    
    private static Object m_eventRubyInit = new Object();
    private static boolean m_bRubyInit = false;
	public void activate()
	{
		m_bInDeactivate = false;
		if (!m_bActivate)
			rhodes_activate();
		else
			RhoRuby.rho_ruby_activateApp();
		
		super.activate();
	}
	
	private boolean m_bActivate = false;
	private void rhodes_activate()
	{
		if ( !m_bStartupFinish )
		{
	        this.invokeLater( new Runnable() { 
	            public void run() 
	            {
	            	//LOG.INFO_EVENT("Activate wait till Startup finish");
	            	rhodes_activate(); 
	            }
	        }, 100, false );
	        return;
		}
		
		m_bActivate = true;
		//DO NOT DO ANYTHING before doStartupWork 
		//doStartupWork();
		showSplashScreen();
    	LOG.TRACE("Rhodes start activate ***--------------------------***");

    	UiApplication.getUiApplication().invokeLater(new Runnable() {
    		
    		public void run() {
		    	if ( !m_bRubyInit )
		    	{
					synchronized (m_eventRubyInit) {
						try{
							m_eventRubyInit.wait();
						}catch(Exception e)
						{
							LOG.ERROR("wait failed", e);
						}
					}
		    	}
		    	
		    	if ( !RhoRuby.rho_ruby_isValid() )
		    	{
		    		LOG.ERROR("Cannot initialize Rho framework. Application will exit.");
		        	Dialog.alert("Cannot initialize Rho framework. Application will exit. Log will send to log server.");
		        	
		        	RhoConf.sendLog();
		        	
		    		System.exit(1);
		    	}
		    	
		    	RhoRuby.rho_ruby_uiCreated();
		    	
		    	runActivateHooks();
		    	
				RhoRuby.rho_ruby_activateApp();
		
		        //if(!restoreLocation()) {
		        //	navigateHome();
		        //}    
		
		    	onPlatformActivate();	    		

		    	LOG.TRACE("Rhodes end activate ***--------------------------***");
    		}
    		
    	});
    	
		super.activate();
	}

    void initRuby()throws Exception
    {
    	try
    	{
	        RhoRuby.RhoRubyStart("");
	        //com.rho.db.DBAdapter.initAttrManager();
	        
	        SyncThread sync = null;
	        
	        try{
	        	sync = SyncThread.Create( new RhoClassFactory() );
	        	
	        }catch(Exception exc){
	        	LOG.ERROR("Create sync failed.", exc);
	        }
	        if (sync != null) {
	        	sync.setStatusListener(this);
	        }
	        
	        RhoRuby.RhoRubyInitApp();
	        RhoRuby.call_config_conflicts();
	        RhoConf.getInstance().conflictsResolved();	        
    	}finally
    	{
	        m_bRubyInit = true;
			synchronized (m_eventRubyInit) {
				m_eventRubyInit.notifyAll();
			}
    	}
    }

    boolean m_bInDeactivate = false;
	public void deactivate() {
    	LOG.TRACE("Rhodes deactivate ***--------------------------***");		
    	
    	m_bInDeactivate = true;
    	RhoRuby.rho_ruby_deactivateApp();
    	
//		SyncEngine.stop(null);
		GeoLocation.stop();
		RingtoneManager.stop();

		super.deactivate();
	}
    
	synchronized public void setSyncStatusPopup(SyncStatusPopup popup) {
		_syncStatusPopup = popup;
		if (_syncStatusPopup != null) {
			_syncStatusPopup.showStatus(_lastStatusMessage);
		} else {
			_lastStatusMessage = null;
		}
	}

	synchronized public void showStatus(String status, String hide) 
	{
		_lastStatusMessage = status;
		_hideStatus = hide;
		invokeLater( new Runnable() {
			public void run() {
				if (_syncStatusPopup != null) { 
					_syncStatusPopup.showStatus(_lastStatusMessage);
				}else
				{
					SyncStatusPopup popup = new SyncStatusPopup(_lastStatusMessage, _hideStatus);
					RhodesApplication.getInstance().setSyncStatusPopup(popup);
					pushScreen(popup);
				}
			}
		});	
				
	}
	
	synchronized public void reportStatus(String status, int error) {
		_lastStatusMessage = status;
		//LOG.INFO("Sync status: " + status);
		//if (_syncStatusPopup == null && error != 0) {
		//	createStatusPopup();
		//} else
		invokeLater( new Runnable() {
			public void run() {
				if (_syncStatusPopup != null) { 
					_syncStatusPopup.showStatus(_lastStatusMessage);
				}else
				{
					SyncStatusPopup popup = new SyncStatusPopup(_lastStatusMessage, null);
					RhodesApplication.getInstance().setSyncStatusPopup(popup);
					pushScreen(popup);
				}
			}
		});	
				
	}
	
	public void createStatusPopup(String status) {
		_lastStatusMessage = status;
		invokeLater( new Runnable() {
			public void run() {
				if (_syncStatusPopup == null) {
					SyncStatusPopup popup = new SyncStatusPopup(_lastStatusMessage, null);
					RhodesApplication.getInstance().setSyncStatusPopup(popup);
					pushScreen(popup);
				}
			}
		});	
	}
	
	static class SyncStatusPopup extends PopupScreen {
		LabelField _labelStatus;
	    public SyncStatusPopup(String status, String hide) {
	        super( new VerticalFieldManager( Manager.NO_VERTICAL_SCROLL | Manager.NO_VERTICAL_SCROLLBAR) );
			
	        add(_labelStatus = new LabelField(status != null ? status : "", Field.FIELD_HCENTER));
	        add(new LabelField(""));
	        
	        if ( hide == null )
	        	hide = RhoAppAdapter.getMessageText("hide");
	        
	        ButtonField hideButton = new ButtonField( hide, Field.FIELD_HCENTER );
			hideButton.setChangeListener( new HideListener(this) );
			add(hideButton);
	    }
	    
	    public void showStatus(String status) {
	    	if (status == null) return;
            //synchronized (Application.getEventLock()) {	
	    		_labelStatus.setText(status);
           // }
	    }
	    
	    protected boolean keyDown( int keycode, int status ) {
	        if ( Keypad.key( keycode ) == Keypad.KEY_ESCAPE ) {
	            close();
	            RhodesApplication.getInstance().setSyncStatusPopup(null);
	            return true;
	        }
	        return super.keyDown( keycode, status );
	    }
	    
		private class HideListener implements FieldChangeListener {
			SyncStatusPopup owner;
			public HideListener(SyncStatusPopup _owner) {
				super();
				owner = _owner;
			}
			public void fieldChanged(Field field, int context) {
				owner.close();
				RhodesApplication.getInstance().setSyncStatusPopup(null);
			}
		}
	}
	
    class CMainScreen extends RhoMainScreen{
    	
    	protected boolean navigationMovement(int dx, int dy, int status, int time) 
    	{
    		if (m_oBrowserAdapter.navigationMovement(dx, dy, status, time))
    		{
    			updateLayout();
    			return true;
    		}
   			return super.navigationMovement(dx, dy, status, time);
		}

		int m_nOrientation = -1;
    	protected void onChangeOrientation(int x, int y, int nOrientation)
    	{
    		if ( m_nOrientation == -1 )
    		{
    			m_nOrientation = nOrientation;
    			return;
    		}
    		
    		if ( m_nOrientation != nOrientation && m_bRubyInit )
	    	{
    			try{
    				RhodesApp.getInstance().callScreenRotationCallback(x, y, m_nOrientation==1 ? 90 : -90);
    			}catch(Exception exc)
    			{
    				LOG.ERROR("Screen rotation callback failed.", exc);
    			}
    			
        		//this.invalidate();
        		//this.updateDisplay();
    		}
    		
    		m_nOrientation = nOrientation;
    	}
    	
		protected boolean navigationClick(int status, int time) {
			//LOG.INFO("navigationClick: " + status);
			return super.navigationClick(status, time);
		}

    	protected boolean onTouchUnclick(int x, int y) {
			if ( m_bDisableInput )
				return true;
    		
			return openLink();
    	}
    	
		private Vector menuItems = new Vector();
		private boolean m_bMenuCopyPaste = false;
		
		private MenuItem homeItem = new MenuItem("", 200000, 10) {
			public void run() {
					navigateHome();
				}
			};
		private MenuItem refreshItem = new MenuItem("", 200000, 10) {
			public void run() {
					refreshCurrentPage();
				}
			};
		private MenuItem backItem = new MenuItem("", 200000, 10) {
			public void run() {
					//back();
					RHODESAPP().navigateBack();
				}
			};
		private MenuItem syncItem = new MenuItem("", 200000, 10) {
			public void run() {
					//RhodesApplication.getInstance().createStatusPopup();		
					SyncThread.doSyncAllSources(true);
				}
			};
		private MenuItem optionsItem = new MenuItem("", 200000, 10) {
			public void run() {
					String curUrl = RhoRuby.getOptionsPage();
					curUrl = RHODESAPP().canonicalizeRhoUrl(curUrl);

					addToHistory(curUrl, null );
			    	
					navigateUrl(curUrl);
				}
			};
		private MenuItem logItem = new MenuItem("", 200000, 10) {
			public void run() {
					showLogScreen();
				}
			};

		private MenuItem separatorItem = MenuItem.separator(200000);
		
		private MenuItem closeItem = new MenuItem("", 200000, 10) {
			public void run() {
					close();
				}
			};

		private MenuItem savedGetLinkItem = null;

		protected void makeMenu(Menu menu, int instance) 
		{
			if (m_bOpenLink)
			{
				super.makeMenu(menu, instance);
				return;
			}
			//return;
			
			if (!m_bMenuCopyPaste)
				menu.deleteAll();
			
			// Don't draw menu if menuItems is null
			if (menuItems == null)
				return;
	    	
			ContextMenu contextMenu = ContextMenu.getInstance();
	        contextMenu.clear();
	        
			// Draw default menu
			if (menuItems != null && menuItems.size() == 0) 
			{
				updateMenuItemsLabel();
				contextMenu.addItem(homeItem);
				contextMenu.addItem(refreshItem);
				contextMenu.addItem(syncItem);
				contextMenu.addItem(optionsItem);
				contextMenu.addItem(logItem);
				contextMenu.addItem(separatorItem);
				contextMenu.addItem(closeItem);
			}

			// Draw menu from rhodes framework
			Enumeration elements = menuItems.elements();
			while (elements.hasMoreElements()) {
				MenuItem item = (MenuItem)elements.nextElement();
				contextMenu.addItem(item);
			}
		
			this.makeContextMenu(contextMenu);
			menu.add(contextMenu);
		}
		
		public void addCustomMenuItem(String label, final String value) 
		{
			final String _label = label;
			// Is this a default item? If so, use the existing menu item we have.

			if (value.equalsIgnoreCase(RhodesApplication.LABEL_BACK)) {
				setDefaultItemToMenuItems(label, backItem);
			}else if ( label.equalsIgnoreCase("back") ){
	    		RHODESAPP().setAppBackUrl(value);
	    	}else if (value.equalsIgnoreCase(RhodesApplication.LABEL_HOME)) {
    	    	setDefaultItemToMenuItems(label, homeItem);
    	    } else if (value.equalsIgnoreCase(RhodesApplication.LABEL_REFRESH)) {
    	    	setDefaultItemToMenuItems(label, refreshItem);
    	    }  else if (value.equalsIgnoreCase(RhodesApplication.LABEL_SYNC)) {
    	    	setDefaultItemToMenuItems(label, syncItem);
    	    } else if (value.equalsIgnoreCase(RhodesApplication.LABEL_OPTIONS)) {
    	    	setDefaultItemToMenuItems(label, optionsItem);
    	    } else if (value.equalsIgnoreCase(RhodesApplication.LABEL_LOG)) {
    	    	setDefaultItemToMenuItems(label, logItem);
    	    } else if (label.equalsIgnoreCase(RhodesApplication.LABEL_SEPARATOR) || 
    	    		   (value != null && value.equalsIgnoreCase(RhodesApplication.LABEL_SEPARATOR))) {
    	    	menuItems.addElement(separatorItem);
    	    } else if (value.equalsIgnoreCase(RhodesApplication.LABEL_CLOSE) ||
    	    		value.equalsIgnoreCase(RhodesApplication.LABEL_EXIT)) {
    	    	setDefaultItemToMenuItems(label, closeItem);
    	    } else if (label.equalsIgnoreCase(RhodesApplication.LABEL_NONE)) {
    	    	menuItems = null;
    	    } else if (value.equalsIgnoreCase(RhodesApplication.LABEL_COPYPASTE)) {
    	    	m_bMenuCopyPaste = true;
    	    } else {
				MenuItem itemToAdd = new MenuItem(label, 200000, 10) 
				{
					public void run() 
					{
						try{
							RHODESAPP().loadUrl(value);
						}catch(Exception exc)
						{
							LOG.ERROR("Execute menu item: '" + _label + "' failed.", exc);
						}
/*							if (value != null && value.startsWith("callback:") )
							{
								String url = RHODESAPP().canonicalizeRhoUrl(value.substring(9));
								RhoRubyHelper helper = new RhoRubyHelper();
								helper.postUrl(url, "");
							}else
							{
						    	String url = RHODESAPP().canonicalizeRhoUrl(value);
								addToHistory(url, null );
								navigateUrl(url);
							}*/
					}
				};
				menuItems.addElement(itemToAdd);
    	    }
		}

		void updateMenuItemsLabel()
		{
	    	homeItem.setText(RhoAppAdapter.getMessageText("home_menu"));
	    	refreshItem.setText(RhoAppAdapter.getMessageText("refresh_menu"));
	    	backItem.setText(RhoAppAdapter.getMessageText("back_menu"));
	    	syncItem.setText(RhoAppAdapter.getMessageText("sync_menu"));
	    	optionsItem.setText(RhoAppAdapter.getMessageText("options_menu"));
	    	logItem.setText(RhoAppAdapter.getMessageText("log_menu"));
	    	closeItem.setText(RhoAppAdapter.getMessageText("close_menu"));
		}
		
		private void setDefaultItemToMenuItems(String label, MenuItem item) {
			item.setText(label);
	    	menuItems.addElement(item);
		}
		
		public void close() {
			LOG.TRACE("Calling Screen.close");
			
			if (com.rho.Capabilities.RUNAS_SERVICE)
				Application.getApplication().requestBackground();
			else
			{
				doClose();
				super.close();
			}
		}
/*		
		public boolean onClose() {
			doClose();
			return super.onClose();
			//System.exit(0);
			//return true;
		}*/

		public boolean onMenu(int instance) {
			// TODO Auto-generated method stub
			return super.onMenu(instance);
		}

		public Vector getMenuItems() {
			return menuItems;
		}

		public void setMenuItems(Vector menuItems) {
			this.menuItems = menuItems;
	    	m_bMenuCopyPaste = false;
		}

		public MenuItem getSavedGetLinkItem() {
			return savedGetLinkItem;
		}
    }

    public void showSplashScreen()
    {
    	SplashScreen splash = RHODESAPP().getSplashScreen();
    	
    	InputStream is = null;
    	try {
    		RubyProgram obj = new xruby.version.main();
	    	String pngname = "/apps/app/loading.png";
	    	is = obj.getClass().getResourceAsStream(pngname);
	    	if ( is != null )
	    	{
		    	int size = is.available();
		    	byte[] data = new byte[size];
		    	for (int offset = 0; offset < size;) {
		    		int n = is.read(data, offset, size - offset);
		    		if (n < 0)
		    			break;
		    		offset += n;
		    	}
		    	EncodedImage img = EncodedImage.createEncodedImage(data, 0, size);
		    	long nFlags = 0;
		    	if (splash.isFlag(SplashScreen.HCENTER) )
		    		nFlags |= Field.FIELD_HCENTER;
		    	if (splash.isFlag(SplashScreen.VCENTER) )
		    		nFlags |= Field.FIELD_VCENTER;

		    	int scaleX = 65536, scaleY = 65536;
				int currentWidthFixed32 = Fixed32.toFP(img.getWidth());
				int currentHeightFixed32 = Fixed32.toFP(img.getHeight());
				int screenWidthFixed32 = Fixed32.toFP(Display.getWidth());
				int screenHeightFixed32 = Fixed32.toFP(Display.getHeight());
				
		    	if (splash.isFlag(SplashScreen.VZOOM) )
		    		scaleY = Fixed32.div(currentHeightFixed32, screenHeightFixed32);
		    	else
		    		scaleY = Fixed32.div(currentHeightFixed32, currentHeightFixed32);
		    	
		    	if (splash.isFlag(SplashScreen.HZOOM) )
		    		scaleX = Fixed32.div(currentWidthFixed32, screenWidthFixed32);
		    	else
		    		scaleX = Fixed32.div(currentWidthFixed32, currentWidthFixed32);
		    	
		    	EncodedImage img2 = img;
		    	if ( scaleX != 65536 || scaleY != 65536)
		    		img2 = img.scaleImage32(scaleX, scaleY);
		    	Bitmap bitmap = img2.getBitmap();
		    	
		    	splash.start();
		    	BitmapField imageField = new BitmapField(bitmap, nFlags);
		    	_mainScreen.deleteAll();
		    	_mainScreen.add(imageField);
	    	}
    	}
    	catch (Exception e) {
    		LOG.ERROR("Can't show splash screen", e);
    	}finally
    	{
    		if ( is != null )
    			try{is.close();}catch(IOException exc){}
    		is = null;
    	}
    	
    	pushScreen(_mainScreen);
    }
    
    public void showLogScreen()
    {
		LogScreen screen = new LogScreen();
	    //Push this screen to display it to the user.
	    UiApplication.getUiApplication().pushScreen(screen);
    }
    
    boolean m_bStartupFinish = false;
    private void doStartupWork() 
    {
/*    	if (_mainScreen!=null)
    		return;
    	
    	if ( com.rho.Capabilities.RUNAS_SERVICE && ApplicationManager.getApplicationManager().inStartup() )// || isWaitForSDCardAtStartup() )
    	{
            this.invokeLater( new Runnable() {
                public void run() 
                {
                    doStartupWork(); 
                }
            } );
            
            return;
    	}*/
    	
    	try{
    	    if ( com.rho.Capabilities.RUNAS_SERVICE )
    	    {
    	    	while( ApplicationManager.getApplicationManager().inStartup() )
    	    	{
    	    		LOG.INFO_EVENT("inStartup");
    	    		Thread.sleep(1000);
    	    	}
    	    	
    			if ( !Jsr75File.isSDCardExist() )
    			{
    				LOG.INFO_EVENT("isSDCardExist");
    				Thread.sleep(5000); //Wait till SDCard may appear
    			}
    	    }
    	    
        	RhoLogger.InitRhoLog();
        	
        	if ( AppBuildConfig.getItem("security_token") != null && 
        		 AppBuildConfig.getItem("security_token").compareTo(m_strSecurityToken) != 0)
            {
                LOG.INFO("This is hidden app and can be started only with security key.");
                RhodesApp.setSecurityTokenNotPassed(true);
            }
            
	        LOG.INFO(" STARTING RHODES: ***----------------------------------*** " );
	        RhodesApp.setStartParameters(m_strCmdLine);
	        
	        RhodesApp.Create(RhoConf.getInstance().getRhoRootPath());
        	
	    	CKeyListener list = new CKeyListener();
	    	CTrackwheelListener wheel = new CTrackwheelListener();
	    	this._history = new Vector();
	
	    	synchronized (Application.getEventLock()) 
	    	{	    	
		        _mainScreen = new CMainScreen();
		        _mainScreen.addKeyListener(list);
		        _mainScreen.addTrackwheelListener(wheel);
		        //pushScreen(_mainScreen);
		        createBrowserControl();
	    	}
	    	
	    	LOG.INFO_EVENT("createBrowserControl" );
	        
	    	try {
	    		RhoClassFactory.getNetworkAccess().configure();
	    		
	    	} catch(IOException exc) {
	    		LOG.ERROR(exc.getMessage());
	    	}
	    	LOG.INFO_EVENT("getNetworkAccess");
	    	
	        //PrimaryResourceFetchThread.Create(this);
	        LOG.INFO("RHODES STARTUP COMPLETED: ***----------------------------------*** " );
	        
	        m_bStartupFinish = true;
	        if ( com.rho.Capabilities.RUNAS_SERVICE && !m_bActivate &&
	        	 RhoConf.getInstance().getBool("activate_at_startup"))
	        {
		        this.invokeLater( new Runnable() { 
		            public void run() 
		            {
		            	rhodes_activate(); 
		            }
		        });
	        	
	        }
    	}catch(Exception exc)
    	{
    		LOG.ERROR("doStartupWork failed", exc);
    		
    		LOG.ERROR_EVENT("doStartupWork failed", exc);
    	}catch(Throwable exc)
    	{
    		LOG.ERROR("doStartupWork crashed.", exc);
    		
    		LOG.ERROR_EVENT("doStartupWork failed", exc);
    		
    		return;
    	}

    }
    
    public void executeJavascript(String strJavascript)
    {
    	final String url = strJavascript;
        this.invokeLater( new Runnable() { 
            public void run() 
            {
            	m_oBrowserAdapter.executeJavascript(url);
            }
        } );
    }
    
    private Hashtable cookies = new Hashtable();
    
    public String getCookie(String url) {
    	URI uri = new URI(url);
    	String baseUrl = uri.getPathNoFragmentNoQuery();
    	Object c = cookies.get(baseUrl);
    	if (c instanceof String)
    		return (String)c;
    	return null;
    }
    
    public void setCookie(String url, String cookie) {
    	URI uri = new URI(url);
    	String baseUrl = uri.getPathNoFragmentNoQuery();
    	cookies.put(baseUrl, cookie);
    	
    	m_oBrowserAdapter.setCookie(url, cookie);
    }
    
    public boolean hasTouchScreen()
    {
    	return _mainScreen.isTouchScreen();
    }
    
    private void createBrowserControl()
    {
    	//touch;5
    	String strFullBrowser = RhoConf.getInstance().getString("use_bb_full_browser");
    	boolean bTouch = strFullBrowser.indexOf("touch") >= 0;
    	boolean bBB5 = strFullBrowser.indexOf("5") >= 0;
    	boolean bBB6 = strFullBrowser.indexOf("6") >= 0;
    	
        if ( bTouch || bBB5 || bBB6)
        {
        	if ( bTouch )
        		m_isFullBrowser = _mainScreen.isTouchScreen();
        	
            if (!m_isFullBrowser && (bBB5 || bBB6 ) )
            {
                Version.SoftVersion ver = Version.getSoftVersion();
                if ( bBB5 )
                	m_isFullBrowser = ver.nMajor >= 5;
                if ( bBB6 )
                	m_isFullBrowser = ver.nMajor >= 6;
            }
        }else if ( RhoConf.getInstance().getBool("use_bb_full_browser") )
        {
	        Version.SoftVersion ver = Version.getSoftVersion();
	        if ( ver.nMajor > 4 || ( ver.nMajor == 4 && ver.nMinor >= 6 ) )
	        	m_isFullBrowser = true;
        }

        if ( m_isFullBrowser )
        {
	        Version.SoftVersion ver = Version.getSoftVersion();
        	
	        if ( ver.nMajor >= 5 )
	        	m_oBrowserAdapter = new BrowserAdapter5(_mainScreen, this);
	        else
	        	m_oBrowserAdapter = new BrowserAdapter(_mainScreen, this, RhoConf.getInstance().getBool("bb_loadimages_async") );
	        
        	m_oBrowserAdapter.setFullBrowser();
        }else
        	m_oBrowserAdapter = new BrowserAdapter(_mainScreen, this, RhoConf.getInstance().getBool("bb_loadimages_async"));
    }
    /*
    private void invokeStartupWork() {
        // I think this can get called twice
        // 1) Directly from startup, if the app starts while the BB is up - e.g. after download
        // 2) From System Listener - after system restart and when the app is originally installed
        // To make sure we don't actually do the startup stuff twice,
        // we use _mainScreen as a flag
        if ( _mainScreen == null ) 
        {
        	if ( com.rho.Capabilities.RUNAS_SERVICE )
        	{
	            LOG.INFO_OUT(" Shedule doStartupWork() ***---------------------------------- " );
	            this.invokeLater( new Runnable() { 
	                public void run() 
	                {
	                    doStartupWork(); 
	                }
	            } );
        	}else
        		doStartupWork();
        }
    }*/

    //----------------------------------------------------------------------
    // SystemListener methods

    public void powerUp() {
    	LOG.INFO_EVENT("POWER UP" );
        if ( com.rho.Capabilities.RUNAS_SERVICE)
        {
        	//invokeStartupWork();
        	this.requestBackground();
        }
    }
    public void powerOff() {
        LOG.TRACE(" POWER DOWN ***----------------------------------*** " );
//        _mainScreen = null;
//        doClose();
    }
    public void batteryLow() { }
    public void batteryGood() { }    
    public void batteryStatusChange(int status) { }

    // end of SystemListener methods
    //----------------------------------------------------------------------
    
    private RhodesApplication() {
        LOG.INFO_OUT(" Construct RhodesApplication() ***----------------------------------*** " );
        m_activateHooks = new Hashtable();
        this.addSystemListener(this);
        PrimaryResourceFetchThread.Create(this);
        
        //this.addFileSystemListener(this);
/*        if ( com.rho.Capabilities.RUNAS_SERVICE && ApplicationManager.getApplicationManager().inStartup() ) 
        {
        	EventLogger.logEvent(0x4c9d3452d87922f2L, "inStartup".getBytes());
        	
            LOG.INFO_OUT("We are in the phone startup, don't start Rhodes yet, leave it to power up call");
        } else {
            invokeStartupWork();
        }*/
    }

    public void refreshCurrentPage(){
		navigateUrl((String)_history.lastElement());
    }

    void navigateHome()
    {
    	String strHomePage = RhoRuby.getStartPage();
    	String strStartPage = RhodesApp.getInstance().canonicalizeRhoUrl(strHomePage);
    	
        _history.removeAllElements();
	    _history.addElement(strStartPage);
	    navigateUrl(strStartPage);
    }
    
    public void close() 
    {
    	this.invokeLater(new Runnable() {
            public void run() 
            {
				_mainScreen.close();
            }
    	});
    	
    	this.invokeLater(new Runnable() {
            public void run() 
            {
				System.exit(1);
            }
    	});
    	
    }

    boolean m_bDisableInput = false;
    public boolean isInputDisabled(){ return m_bDisableInput; }
    public void processConnection(HttpConnection connection, Object e) 
    {
        // cancel previous request
        /*if (_currentConnection != null) {
            try {
                _currentConnection.close();
            } catch (IOException e1) {
            }
        }

        _currentConnection = connection;*/
        
        RHODESAPP().getSplashScreen().hide();
        
        m_bDisableInput = true;
        m_oBrowserAdapter.processConnection(connection, e);
        m_bDisableInput = false;
        
		try {
			IRhoRubyHelper helper = RhoClassFactory.createRhoRubyHelper();
			helper.wakeUpGeoLocation();
		}catch(Exception exc)
		{
			LOG.ERROR("wakeUpGeoLocation failed", exc);
		}
        
    }

    public static class PrimaryResourceFetchThread {//extends Thread {

    	private static class HttpServerThread extends RhoThread
    	{
    		private Object        m_mxStackCommands;// = new Mutex();
    		private LinkedList	  m_stackCommands = new LinkedList();	         
    		boolean m_bExit = false;
    		private static final int INTERVAL_INFINITE = Integer.MAX_VALUE/1000;
    		static final int WAIT_BEFOREKILL_SECONDS  = 3;
    		
    		HttpServerThread()
    		{
    			super(new RhoClassFactory());
    			
    			m_mxStackCommands = getSyncObject();
    			start(epNormal);
    		}
    		
            public void run() 
            {
            	_application.doStartupWork();
            	if (!_application.m_bStartupFinish)
            	{
            		return;
            	}
            	
        		LOG.INFO( "Starting HttpServerThread main routine..." );
            	//wait(80);
    	    	try{
        		
	        		_application.initRuby();
	        		
    	    	}catch(Exception e)
    	    	{
    	    		LOG.ERROR("initRuby failed.", e);
    	    		return;
    	    	}catch(Throwable exc)
    	    	{
    	    		LOG.ERROR("initRuby crashed.", exc);
    	    		return;
    	    	}
    	    	
    	    	if ( com.rho.Capabilities.ENABLE_PUSH  )
    	    	{
    	    		if ( PushListeningThread.isMDSPushEnabled() )
    	    		{
		    	    	_pushListeningThread = new PushListeningThread();
		    	    	_pushListeningThread.start();
    	    		}
    	    	}
    	    	
        		while( !m_bExit )
        		{
	        		while(!m_stackCommands.isEmpty())
	        		{
	        			PrimaryResourceFetchThread oCmd = null;
	        	    	synchronized(m_mxStackCommands)
	        	    	{
	        	    		oCmd = (PrimaryResourceFetchThread)m_stackCommands.removeFirst();
	        	    	}
	        			
	        	    	try{
	        	    		oCmd.processCommand();
	        	    	}catch(Exception e)
	        	    	{
	        	    		LOG.ERROR("Process command failed.", e);
	        	    	}catch(Throwable exc)
	        	    	{
	        	    		LOG.ERROR("Process command crashed.", exc);
	        	    	}
	        		}

        	    	synchronized(m_mxStackCommands)
        	    	{
        	    		if ( m_stackCommands.isEmpty() )
        	    		{
        	    			int nTimeout = (int)RHODESAPP().getTimer().getNextTimeout();
        	    			waitMs(nTimeout == 0 ? INTERVAL_INFINITE : nTimeout);
        	    			
        	    			if ( !m_bExit && m_stackCommands.isEmpty() )
        	    				RHODESAPP().getTimer().checkTimers();
        	    				
        	    		}
        	    	}
        		}
        		
        		LOG.INFO( "Exit HttpServerThread main routine..." );
        		
            }
            
            void addCommand(PrimaryResourceFetchThread oCmd)
            { 
            	synchronized(m_mxStackCommands)
            	{
          			m_stackCommands.add(oCmd);
            	}
            	stopWait(); 
            }
            
    	};
    	
    	private static HttpServerThread m_oFetchThread;
    	
        private static RhodesApplication _application;
        private static Runnable _callback;

        private Object _event;

        private byte[] _postData;

        private HttpHeaders _requestHeaders;

        private String _url;
        private boolean m_bInternalRequest = false;
        private boolean m_bActivateApp = false;
        NetCallback m_netCallback;
        
        public void setInternalRequest(boolean b)
        {
        	m_bInternalRequest = b;
        }
        
        public PrimaryResourceFetchThread(String url, HttpHeaders requestHeaders, byte[] postData,
                						Object event) 
        {
			_url = url;
			_requestHeaders = requestHeaders;
			_postData = postData;
			_event = event;
			//_callback = null;
		}
        
        public PrimaryResourceFetchThread(String url, HttpHeaders requestHeaders, byte[] postData,
                                      	Object event, Runnable callback) 
        {
            _url = url;
            _requestHeaders = requestHeaders;
            _postData = postData;
            _event = event;
            if ( callback != null )
            	_callback = callback;
        }

        public void setNetCallback(NetCallback netCallback) 
		{
			m_netCallback = netCallback;
			m_bInternalRequest = true;
		}
        
        public PrimaryResourceFetchThread(boolean bActivateApp) {
        	m_bActivateApp = bActivateApp; 
        }
        
        static void Create(RhodesApplication app)
        {
        	if ( m_oFetchThread != null )
        		return;

        	_application = app;
        	m_oFetchThread = new HttpServerThread(); 
        }
        
    	public void Destroy()
    	{
    		m_oFetchThread.m_bExit = true;
    		m_oFetchThread.stop(HttpServerThread.WAIT_BEFOREKILL_SECONDS);
    		
    		m_oFetchThread = null;
    	}
        
    	public void start()
    	{
            m_oFetchThread.addCommand(this);
    	}
    	
    	static class RhoTextMessage implements javax.wireless.messaging.TextMessage
    	{
    		String m_strAddress = "", m_strBody = "";
    		RhoTextMessage(String strAddr, String strBody)
    		{
    			super();
    			
    			m_strAddress = strAddr;
    			m_strBody = strBody;
    		}
			public String getPayloadText() {
				return m_strBody;
			}

			public void setPayloadText(String arg0) 
			{
				m_strBody = arg0;
			}

			public String getAddress() {
				return m_strAddress;
			}

			public Date getTimestamp() {
				return null;
			}

			public void setAddress(String addr) 
			{
				m_strAddress = addr;	
			}
    		
    	};
    	
    	void runMailApplication(URI uri)
    	{
    		net.rim.blackberry.api.mail.Message msg = new net.rim.blackberry.api.mail.Message();
    		
    		String strTo = uri.getPath();
    		
    		try
    		{
	    		if ( strTo != null && strTo.length() > 0 )
	    			msg.addRecipient(RecipientType.TO, new Address( strTo, strTo) );
	    		
    		}catch(Exception exc)
    		{
    			LOG.ERROR("Error setting mail TO: " + strTo, exc);
    		}
    		
        	String query = uri.getQueryString();
        	if (query != null) 
        	{
        		StringParser tok = new StringParser(query, "&");
        		while (tok.hasMoreElements()) {
        			String pair = (String)tok.nextElement();
        			StringParser nv = new StringParser(pair, "=");
        			String name = (String)nv.nextElement();
        			String value = (String)nv.nextElement();
        			if (name == null || value == null)
        				continue;

            		try
            		{
	        			if (name.equalsIgnoreCase("subject") )
	        				msg.setSubject(value);
	        			else if (name.equalsIgnoreCase("body")) 
	        				msg.setContent(value);
	        			else if (name.equalsIgnoreCase("cc")) 
	        				msg.addRecipient(RecipientType.CC, new Address( value, value) );
	        			else if (name.equalsIgnoreCase("bcc")) 
	        				msg.addRecipient(RecipientType.BCC, new Address( value, value) );
	        			else if (name.equalsIgnoreCase("from")) 
	        				msg.addRecipient(RecipientType.FROM, new Address( value, value) );
	        			else if (name.equalsIgnoreCase("to")) 
	        				msg.addRecipient(RecipientType.TO, new Address( value, value) );
	        			else if (name.equalsIgnoreCase("sender")) 
	        				msg.addRecipient(RecipientType.SENDER, new Address( value, value) );
	        			else if (name.equalsIgnoreCase("reply_to")) 
	        				msg.addRecipient(RecipientType.REPLY_TO, new Address( value, value) );
	        			else
	        				msg.addHeader(name.toUpperCase(), value);

            		}catch(Exception exc)
            		{
            			LOG.ERROR("Error setting message property: " + name + ";value:" + value, exc);
            		}
        		}
        	}
    		Invoke.invokeApplication(Invoke.APP_TYPE_MESSAGES, new MessageArguments( msg ));
    	}
    	
        void processCommand()throws IOException
        {
        	if ( m_bActivateApp )
        	{
        		RhoRuby.rho_ruby_activateApp();
        		return;
        	}
        	
        	URI uri = new URI(_url);
        	String strMsgBody = "";
        	String query = uri.getQueryString();
        	if (query != null) {
        		StringParser tok = new StringParser(query, "&");
        		while (tok.hasMoreElements()) {
        			String pair = (String)tok.nextElement();
        			StringParser nv = new StringParser(pair, "=");
        			String name = (String)nv.nextElement();
        			String value = (String)nv.nextElement();
        			if (name == null || value == null)
        				continue;
        			
        			if (name.equals("rho_open_target") && value.equals("_blank")) {
    		    		RhoRubyHelper helper = new RhoRubyHelper();
    		    		helper.open_url(_url);
        				return;
        			}
        			
        			if (name.equalsIgnoreCase("body")) 
        			{
        				strMsgBody = value;
        			}
        		}
        	}

        	if ( uri.getScheme().equalsIgnoreCase("rhomailto") || uri.getScheme().equalsIgnoreCase("mailto"))
        	{
        		runMailApplication(uri);
        		return;
        	}else if ( uri.getScheme().equalsIgnoreCase("sms"))
        	{
        		RhoTextMessage msg = new RhoTextMessage(uri.getPath(), URI.urlDecode(strMsgBody) );
        		
        		MessageArguments args = new MessageArguments( msg );
        		Invoke.invokeApplication(Invoke.APP_TYPE_MESSAGES, args);
        		return;
        	}else if ( uri.getScheme().equalsIgnoreCase("tel"))
        	{
        		Invoke.invokeApplication(Invoke.APP_TYPE_PHONE, new PhoneArguments( PhoneArguments.ARG_CALL,
        				uri.getPath() ));
        		return;
        	}

    		HttpConnection connection = Utilities.makeConnection(_url, _requestHeaders, _postData, null);
    		
    		if ( m_bInternalRequest )
    		{
    			try{
    				
    				int nRespCode = connection.getResponseCode();
    				
    				if ( m_netCallback != null )
    				{
    					String strRespBody = "";
    					InputStream is = connection.openInputStream();
    					
    					if ( is != null )
    					{
	    					byte[] buffer = new byte[is.available()];
	    					is.read(buffer);
	    					strRespBody = new String(buffer);
    					}
    					
    					m_netCallback.setResponse( new NetResponse(strRespBody, nRespCode) );
    				}
    			}catch(IOException exc)
    			{
    				LOG.ERROR("Callback failed: " + _url, exc);
    				
    				if ( m_netCallback != null )
    					m_netCallback.setResponse( new NetResponse("", 500) );
    			}
    		}
    		else
    		{
    			_application.processConnection(connection, _event);
    		}
    		
        	if (_callback != null )
        	{
        		_callback.run();
        		_callback = null;
        	}
        }
        
    }
}

