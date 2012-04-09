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

import java.util.Hashtable;

import net.rim.blackberry.api.browser.Browser;
import net.rim.blackberry.api.browser.BrowserSession;
import net.rim.device.api.system.ApplicationDescriptor;
import net.rim.device.api.system.CodeModuleGroup;
import net.rim.device.api.system.DeviceInfo;
import net.rim.device.api.io.http.HttpHeaders;
import com.rho.rubyext.Alert;
import rhomobile.NativeBar;
import com.rho.rubyext.RhoPhonebook;
import com.rho.rubyext.RhoCalendar;
import rhomobile.RhodesApplication;
import rhomobile.RingtoneManager;
import com.rho.rubyext.WebView;

import rhomobile.bluetooth.BluetoothManager;
import rhomobile.camera.Camera;
import rhomobile.datetime.DateTimePicker;
import rhomobile.mapview.MapView;
import com.rho.rubyext.GeoLocation;

//import com.rho.db.HsqlDBStorage;
import com.rho.db.IDBStorage;
import com.rho.file.*;
import com.rho.net.SSLSocket;
import com.rho.net.TCPSocket;
import com.xruby.runtime.builtin.RubyArray;
import com.xruby.runtime.lang.RubyProgram;
import com.xruby.runtime.lang.RubyRuntime;
import com.rho.net.NetResponse;
import javax.microedition.io.HttpConnection;

public class RhoRubyHelper implements IRhoRubyHelper 
{
	private static RhodesApp RHODESAPP(){ return RhodesApp.getInstance(); }

	// WARNING!!! Be very careful when modify these lines! There was a case when
	// entire application has verification error in case if this line is not at start
	// of class. It is impossible to explain why it happened but need to be remembered
	public static final String USE_PERSISTENT = "use_persistent_storage";
	
	public static final int COVERAGE_BIS_B = 4;
	  
	public void initRubyExtensions(){
        RhoPhonebook.initMethods(RubyRuntime.PhonebookClass);
        RhoCalendar.initConstants(RubyRuntime.EventModule);
        RhoCalendar.initMethods(RubyRuntime.CalendarClass);
        Camera.initMethods(RubyRuntime.CameraClass);
        BluetoothManager.initMethods(RubyRuntime.RhoBluetoothClass);
        WebView.initMethods(RubyRuntime.WebViewClass);
        RhoConf.initMethods(RubyRuntime.RhoConfClass);
        Alert.initMethods(RubyRuntime.AlertClass);        
        DateTimePicker.initMethods(RubyRuntime.DateTimePickerClass);
        RingtoneManager.initMethods(RubyRuntime.RingtoneManagerClass);
        NativeBar.initMethods(RubyRuntime.NativeBarClass);
        TCPSocket.initMethods(RubyRuntime.TCPSocketClass);
        SSLSocket.initMethods(RubyRuntime.SSLSocketClass);
        MapView.initMethods(RubyRuntime.MapViewClass);
        GeoLocation.initMethods(RubyRuntime.GeoLocationClass);
        com.rho.rubyext.System.initMethods(RubyRuntime.SystemClass);
        com.rho.rubyext.XMLParser.initMethods(RubyRuntime.XMLParserClass);
        com.rho.rubyext.SignatureCapture.initMethods(RubyRuntime.SignatureCaptureClass);
	}
	
	public RubyProgram createMainObject() throws Exception
	{
    	/*RhoRubyHelper helper = new RhoRubyHelper();
    	String appName = RhoSupport.getAppName();
		
		String strName = appName + ".ServeME.main";//com.xruby.runtime.lang.RhoSupport.createMainClassName("");
		
        Class c = Class.forName(strName);
        Object o = c.newInstance();
        RubyProgram p = (RubyProgram) o;
		
		return p;*/
		return new xruby.rhoframework.main();
	}

	public String getPlatform() {
		return "Blackberry";
	}

	public void loadBackTrace(RubyArray backtrace) {
		//TODO:
	}

	public String getDeviceId(){
		return new Integer( DeviceInfo.getDeviceId() ).toString();
    }

	public boolean isSimulator(){
		return DeviceInfo.isSimulator();
		//return false;
    }

	public String getModuleName()
	{
		return ApplicationDescriptor.currentApplicationDescriptor().getModuleName();
	}
	
	public void showLog()
	{
		synchronized ( RhodesApplication.getEventLock() ) {		
			RhodesApplication.getInstance().showLogScreen();
		}
	}
	
	public NetResponse postUrl(String url, String body)
	{
		RhodesApplication.NetCallback netCallback = new RhodesApplication.NetCallback();
		
		HttpHeaders headers = new HttpHeaders();
		headers.addProperty("Content-Type", "application/x-www-form-urlencoded");
		
		RhodesApplication.getInstance().postUrlWithCallback(url, body, headers, netCallback);
		netCallback.waitForResponse();
		
		return netCallback.m_response;
	}

	public void postUrlNoWait(String url, String body)
	{
		HttpHeaders headers = new HttpHeaders();
		headers.addProperty("Content-Type", "application/x-www-form-urlencoded");
		
		RhodesApplication.getInstance().postUrl(url, body, headers);
	}
	
	public NetResponse postUrlSync(String url, String body)throws Exception
	{
		HttpHeaders headers = new HttpHeaders();
		headers.addProperty("Content-Type", "application/x-www-form-urlencoded");
		
		HttpConnection connection = rhomobile.Utilities.makeConnection(url, headers, body.getBytes(), null);
		
		java.io.InputStream is = connection.openInputStream();		
		int nRespCode = connection.getResponseCode();
		
		String strRespBody = "";
		if ( is != null )
		{
			byte[] buffer = new byte[is.available()];
			is.read(buffer);
			strRespBody = new String(buffer);
		}
			
		return new NetResponse(strRespBody, nRespCode);
	}
	
	public void navigateUrl(String url)
	{
		WebView.navigate(url);
	}
	
	public void navigateBack()
	{
		RhodesApplication.getInstance().navigateBack();
	}
	
	public void app_exit()
	{
		RhodesApplication.getInstance().close();
	}

	public void open_url(String url)
	{
		BrowserSession session = Browser.getDefaultSession();
		session.showBrowser();
		
		if( !RHODESAPP().isExternalUrl(url) )
		{
			try{
				url = FilePath.join(Jsr75File.getRhoPath(), url);
			}catch(  java.io.IOException exc){}
		}
		//String strURL = RHODESAPP().canonicalizeRhoUrl(url);
		session.displayPage(url);
	}
	
	static Hashtable m_appProperties = new Hashtable();
	static CodeModuleGroup m_groupRhodes = null;
	static boolean m_bGroupsInited = false;
	public String getAppProperty(String name)
	{
		String strRes = null;
		synchronized (m_appProperties)
		{
			if ( m_appProperties.containsKey(name) )
				strRes = (String)m_appProperties.get(name);
			else	
			{
				if (!m_bGroupsInited)
				{
					m_bGroupsInited = true;
					CodeModuleGroup[] codeModule = CodeModuleGroup.loadAll();
	
					if ( codeModule != null )
					{
						String moduleName = getModuleName();
						
						for(int i = 0; i < codeModule.length; i++) 
						{
							String module = codeModule[i].getName();
							if( module.indexOf( moduleName ) != -1)
							{
								m_groupRhodes = codeModule[i];
								break;
							}
						}
					}
				}
				
				if (m_groupRhodes != null)				
					strRes = m_groupRhodes.getProperty(name);
				
/*				
				CodeModuleGroup[] allGroups = CodeModuleGroupManager.loadAll();
				if ( allGroups != null )
				{
					String moduleName = ApplicationDescriptor
					   .currentApplicationDescriptor().getModuleName();
	
					CodeModuleGroup myGroup = null;
					for (int i = 0; i < allGroups.length; i++) {
					   if (allGroups[i].containsModule(moduleName)) {
					      myGroup = allGroups[i];
					      break;
					   	 }
					}
	
					if ( myGroup != null )
						strRes = myGroup.getProperty(name);
				} */
				
				if ( strRes == null )
					strRes = "";
				
				m_appProperties.put(name,strRes);
			}
		}
		
		return strRes;
	}

	public IFileAccess createFileAccess() {
		return new FileAccessBB();
	}

	public IRAFile createRAFile() {
		if (RhoConf.getInstance().getBool(USE_PERSISTENT))
			return new PersistRAFileImpl();
		else
			return new Jsr75RAFileImpl();
	}

	public IRAFile createFSRAFile()
	{
		return new Jsr75RAFileImpl();
	}
	
	public String getGeoLocationText()
	{
		return GeoLocation.getGeoLocationText();
	}
	
	public void wakeUpGeoLocation()
	{
		GeoLocation.wakeUp();
	}
	
	public void unzip_file(String strPath)throws Exception
	{
		com.rho.rubyext.System.unzip_file(strPath);
	}
}
