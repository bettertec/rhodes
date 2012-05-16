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

package com.rho.rubyext;

import rhomobile.RhodesApplication;
import net.rim.blackberry.api.phone.Phone;
import net.rim.device.api.i18n.Locale;
import net.rim.device.api.system.Display;

import com.rho.BBVersionSpecific;
import com.rho.FilePath;
import com.rho.RhoClassFactory;
import com.rho.RhoEmptyLogger;
import com.rho.RhoLogger;
import com.rho.RhodesApp;
import com.xruby.runtime.lang.*;
import com.xruby.runtime.builtin.*;
import com.rho.RhoRubyHelper;
import com.rho.file.IRAFile;
import com.rho.Version;

import net.rim.device.api.system.ApplicationDescriptor;
import net.rim.device.api.system.ApplicationManager;
import net.rim.device.api.system.CodeModuleManager;
import net.rim.device.api.system.DeviceInfo;
import net.rim.device.api.system.Backlight;
import net.rim.device.api.compress.*;
import java.io.*;

public class System {

	private static final RhoLogger LOG = RhoLogger.RHO_STRIP_LOG ? new RhoEmptyLogger() : 
		new RhoLogger("System");
	
	private static RhodesApp RHODESAPP(){ return RhodesApp.getInstance(); }
	
	public static void initMethods(RubyModule klass){
		klass.getSingletonClass().defineMethod( "get_property", new RubyOneArgMethod(){ 
			protected RubyValue run(RubyValue receiver, RubyValue arg, RubyBlock block )
			{
				try {
					return get_property(arg);
				} catch(Exception e) {
					LOG.ERROR("get_property failed", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
			}
		});
		klass.getSingletonClass().defineMethod( "has_network", new RubyNoArgMethod(){ 
			protected RubyValue run(RubyValue receiver, RubyBlock block )
			{
				try {
					return ObjectFactory.createBoolean(hasNetwork());
				} catch(Exception e) {
					LOG.ERROR("has_network failed", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
			}
		});
		klass.getSingletonClass().defineMethod( "get_locale", new RubyNoArgMethod(){ 
			protected RubyValue run(RubyValue receiver, RubyBlock block )
			{
				try {
					return ObjectFactory.createString(getLocale());
				} catch(Exception e) {
					LOG.ERROR("get_locale failed", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
			}
		});
		klass.getSingletonClass().defineMethod( "get_screen_width", new RubyNoArgMethod() {
			protected RubyValue run(RubyValue receiver, RubyBlock block) 
			{
				try {
					return ObjectFactory.createInteger(getScreenWidth());
				} catch(Exception e) {
					LOG.ERROR("get_screen_width failed", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
			}
		});
		klass.getSingletonClass().defineMethod( "get_screen_height", new RubyNoArgMethod() {
			protected RubyValue run(RubyValue receiver, RubyBlock block) 
			{
				try {
					return ObjectFactory.createInteger(getScreenHeight());
				} catch(Exception e) {
					LOG.ERROR("get_screen_height failed", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
			}
		});
		klass.getSingletonClass().defineMethod( "set_push_notification", new RubyTwoArgMethod(){ 
			protected RubyValue run(RubyValue receiver, RubyValue arg1, RubyValue arg2, RubyBlock block )
			{
				try {
					String url = arg1 != RubyConstant.QNIL ? arg1.toStr() : "";
					String params = arg2 != RubyConstant.QNIL ? arg2.toStr() : "";
					
					RhodesApp.getInstance().setPushNotification(url, params);
					
					return RubyConstant.QNIL;
				} catch(Exception e) {
					LOG.ERROR("set_push_notification failed", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
			}
		});
		klass.getSingletonClass().defineMethod( "set_screen_rotation_notification", new RubyTwoArgMethod(){ 
			protected RubyValue run(RubyValue receiver, RubyValue arg1, RubyValue arg2, RubyBlock block )
			{
				try {
					String url = arg1 != RubyConstant.QNIL ? arg1.toStr() : "";
					String params = arg2 != RubyConstant.QNIL ? arg2.toStr() : "";
					
					RhodesApp.getInstance().setScreenRotationNotification(url, params);
					
					return RubyConstant.QNIL;
				} catch(Exception e) {
					LOG.ERROR("set_screen_rotation_notification failed", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
			}
		});
		klass.getSingletonClass().defineMethod("exit", new RubyNoArgMethod() {
			protected RubyValue run(RubyValue receiver, RubyBlock block) {
				//synchronized(RhodesApplication.getInstance().getEventLock()) {
					RhodesApplication.getInstance().close();
					return RubyConstant.QNIL;
				//}
			}
		});
		
		klass.getSingletonClass().defineMethod( "set_sleeping", new RubyOneArgMethod(){ 
			protected RubyValue run(RubyValue receiver, RubyValue arg1, RubyBlock block )
			{
				try {
					RubyValue ret = !Backlight.isEnabled() ? RubyConstant.QTRUE : RubyConstant.QFALSE;
					
					if ( arg1 != RubyConstant.QTRUE )
						Backlight.enable(true, 255);
					else
						Backlight.enable(false, Backlight.getTimeoutDefault());
					
					return ret;
				} catch(Exception e) {
					LOG.ERROR("set_sleeping failed", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
			}
		});

		klass.getSingletonClass().defineMethod( "app_installed?", new RubyOneArgMethod(){ 
			protected RubyValue run(RubyValue receiver, RubyValue arg1, RubyBlock block )
			{
				try 
				{
					String app_name = arg1.toStr();
					int nHandle = CodeModuleManager.getModuleHandle(app_name);
					
					return nHandle != 0 ? RubyConstant.QTRUE : RubyConstant.QFALSE;
				} catch(Exception e) {
					LOG.ERROR("run_app failed", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
			}
		});
		
		klass.getSingletonClass().defineMethod( "app_install", new RubyOneArgMethod() {
			protected RubyValue run(RubyValue receiver, RubyValue arg,
					RubyBlock block) {
				try 
				{
		    		RhoRubyHelper helper = new RhoRubyHelper();
		    		helper.open_url(arg.toStr());
					
					return RubyConstant.QNIL;
				} catch(Exception e) {
					LOG.ERROR("app_install failed", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
			}
		});

		klass.getSingletonClass().defineMethod( "app_uninstall", new RubyOneArgMethod(){ 
			protected RubyValue run(RubyValue receiver, RubyValue arg1, RubyBlock block )
			{
				try 
				{
					String app_name = arg1.toStr();
					int nHandle = CodeModuleManager.getModuleHandle(app_name);
					if ( nHandle == 0 )
						LOG.ERROR("Cannot find application: " + app_name);
					else
					{
						int nCode = CodeModuleManager.deleteModuleEx(nHandle, true);
						LOG.INFO("CodeModuleManager.deleteModuleEx return code: " + nCode);
						
						if ( nCode == CodeModuleManager.CMM_OK_MODULE_MARKED_FOR_DELETION ) 
						{
							LOG.INFO("Device need to be restarted.");
							CodeModuleManager.promptForResetIfRequired(); 
						}
						
					}
					
					return RubyConstant.QNIL;
				} catch(Exception e) {
					LOG.ERROR("app_uninstall failed", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
			}
		});
		
		klass.getSingletonClass().defineMethod( "run_app", new RubyTwoArgMethod(){ 
			protected RubyValue run(RubyValue receiver, RubyValue arg1, RubyValue arg2, RubyBlock block )
			{
				try {
					String app_name = arg1.toStr();
					ApplicationManager appMan = ApplicationManager.getApplicationManager();
					String strParams = arg2 != null && arg2 != RubyConstant.QNIL ? arg2.toStr() : "";
					
					appMan.launch(app_name + (strParams.length() > 0 ? "?" + strParams : "") );
					
					return RubyConstant.QNIL;
				} catch(Exception e) {
					LOG.ERROR("run_app failed", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
			}
		});

		klass.getSingletonClass().defineMethod( "open_url", new RubyOneArgMethod(){ 
			protected RubyValue run(RubyValue receiver, RubyValue arg1, RubyBlock block )
			{
				try 
				{
		    		RhoRubyHelper helper = new RhoRubyHelper();
		    		helper.open_url(arg1.toStr());
					
					return RubyConstant.QNIL;
				} catch(Exception e) {
					LOG.ERROR("open_url failed", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
			}
		});
		klass.getSingletonClass().defineMethod( "unzip_file", new RubyOneArgMethod(){ 
			protected RubyValue run(RubyValue receiver, RubyValue arg1, RubyBlock block )
			{
				try 
				{
					String strPath = arg1.toStr();
					unzip_file(strPath);
					return RubyConstant.QNIL;
				} catch(Exception e) {
					LOG.ERROR("open_url failed", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
			}
		});
		klass.getSingletonClass().defineMethod( "get_start_params", new RubyNoArgMethod(){ 
			protected RubyValue run(RubyValue receiver, RubyBlock block )
			{
				try 
				{
					return ObjectFactory.createString(RhodesApp.getStartParameters());
				} catch(Exception e) {
					LOG.ERROR("get_start_params failed", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
			}
		});
		
		klass.getSingletonClass().defineMethod( "start_timer", new RubyVarArgMethod(){ 
			protected RubyValue run(RubyValue receiver, RubyArray args, RubyBlock block )
			{
				try 
				{
					if ( args == null || args.size() != 3 )
						throw new RubyException(RubyRuntime.ArgumentErrorClass, 
								"in System.start_timer: wrong number of arguments ( " + args.size() + " for " + 3 + " )");
					
					int nInterval = args.get(0).toInt();
					String url = args.get(1).toStr();
					String params = args.get(2).toStr();
					
					RHODESAPP().getTimer().addTimer(nInterval, url, params);
					return RubyConstant.QNIL;
				} catch(Exception e) {
					LOG.ERROR("start_timer failed", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
			}
		});
		
		klass.getSingletonClass().defineMethod( "stop_timer", new RubyOneArgMethod(){ 
			protected RubyValue run(RubyValue receiver, RubyValue arg1, RubyBlock block )
			{
				try 
				{
					String url = arg1.toStr();
					
					RHODESAPP().getTimer().stopTimer(url);
					return RubyConstant.QNIL;
				} catch(Exception e) {
					LOG.ERROR("stop_timer failed", e);
					throw (e instanceof RubyException ? (RubyException)e : new RubyException(e.getMessage()));
				}
			}
		});		
	}
    
    //@RubyLevelMethod(name="get_property", module=true)
    private static RubyValue get_property(RubyValue arg) {
    	String strPropName = arg.toStr();
    	if ( strPropName.equalsIgnoreCase("platform") )
    	{
    		RhoRubyHelper helper = new RhoRubyHelper();
    		return ObjectFactory.createString( helper.getPlatform() );
    	}
    	if ( strPropName.equalsIgnoreCase("has_network") )
    		return ObjectFactory.createBoolean(hasNetwork()); 
    	if ( strPropName.equalsIgnoreCase("locale") )
    		return ObjectFactory.createString(getLocale());
    	if ( strPropName.equalsIgnoreCase("country") )
    		return ObjectFactory.createString(getCountry());
    	if ( strPropName.equalsIgnoreCase("screen_width") )
    		return ObjectFactory.createInteger(getScreenWidth()); 
    	if ( strPropName.equalsIgnoreCase("screen_height") )
    		return ObjectFactory.createInteger(getScreenHeight());
    	if ( strPropName.equalsIgnoreCase("screen_orientation")) {
    		if (getScreenHeight() >= getScreenWidth()) {
    			return ObjectFactory.createString("portrait");
    		}
			return ObjectFactory.createString("landscape");
    	}
    	if ( strPropName.equalsIgnoreCase("ppi_x"))
    		return ObjectFactory.createFloat(getScreenPpiX());
    	if ( strPropName.equalsIgnoreCase("ppi_y"))
    		return ObjectFactory.createFloat(getScreenPpiY());
    	if ( strPropName.equalsIgnoreCase("has_camera") )
    		return ObjectFactory.createBoolean(hasCamera()); 
    	if ( strPropName.equalsIgnoreCase("phone_number") )
    		return ObjectFactory.createString(Phone.getDevicePhoneNumber(true)); 
    	if ( strPropName.equalsIgnoreCase("device_id") )
    		return ObjectFactory.createString(new Integer( DeviceInfo.getDeviceId() ).toString()); 
    	if ( strPropName.equalsIgnoreCase("full_browser") )
    		return ObjectFactory.createBoolean(rhomobile.RhodesApplication.isFullBrowser());
    	if ( strPropName.equalsIgnoreCase("device_name") )
    		return ObjectFactory.createString(DeviceInfo.getDeviceName());
    	if ( strPropName.equalsIgnoreCase("os_version") )
    		return ObjectFactory.createString(DeviceInfo.getSoftwareVersion());
    	if ( strPropName.equalsIgnoreCase("rhodes_port") )
    		return ObjectFactory.createInteger(0);
    	if ( strPropName.equalsIgnoreCase("is_emulator") )
    		return ObjectFactory.createBoolean(DeviceInfo.isSimulator());
    	if ( strPropName.equalsIgnoreCase("has_calendar") )
    		return ObjectFactory.createBoolean(RhoCalendar.has_calendar());
    	if ( strPropName.equalsIgnoreCase("has_touchscreen") )
    		return ObjectFactory.createBoolean(rhomobile.RhodesApplication.getInstance().hasTouchScreen());
    	if ( strPropName.equalsIgnoreCase("has_sqlite") )
    		return ObjectFactory.createBoolean(com.rho.Capabilities.USE_SQLITE);
    	if ( strPropName.equalsIgnoreCase("webview_framework") ) {
    		Version.SoftVersion ver = Version.getSoftVersion();
    		
    		if ( rhomobile.RhodesApplication.isFullBrowser() )
    			return ObjectFactory.createString( "WEBKIT"+"/"+DeviceInfo.getSoftwareVersion() );
    		
    		return ObjectFactory.createString( "BB" +"/"+DeviceInfo.getSoftwareVersion() ); 
    	}
        if (strPropName.equalsIgnoreCase("security_token_not_passed"))
        	return ObjectFactory.createBoolean(RHODESAPP().isSecurityTokenNotPassed());

    	return RubyConstant.QNIL;
    }

	public static String getLocale()
	{
    	Locale loc = Locale.getDefault();
    	
    	String lang = loc != null ? loc.getLanguage() : "en";
		return lang;
	}
	
	private static String getCountry() {
		Locale loc = Locale.getDefault();
		String country = loc != null ? loc.getCountry() : "US";
		return country;
	}
	
	public static boolean hasCamera() 
	{
		return DeviceInfo.hasCamera();
	}
	
	public static boolean hasNetwork() {
		/*if ((RadioInfo.getActiveWAFs() & RadioInfo.WAF_WLAN) != 0) {
			if (CoverageInfo.isCoverageSufficient( CoverageInfo.COVERAGE_CARRIER,RadioInfo.WAF_WLAN, false) || 
					CoverageInfo.isCoverageSufficient( CoverageInfo.COVERAGE_MDS,RadioInfo.WAF_WLAN, false) ||
					CoverageInfo.isCoverageSufficient( COVERAGE_BIS_B,RadioInfo.WAF_WLAN, false))
				return true;
		}

		if (CoverageInfo.isOutOfCoverage())
	        return false; 
		*/
		int nStatus = net.rim.device.api.system.RadioInfo.getNetworkService();
		boolean hasGPRS = ( nStatus & net.rim.device.api.system.RadioInfo.NETWORK_SERVICE_DATA) != 0;
		
		boolean hasWifi = BBVersionSpecific.isWifiActive();
		LOG.INFO("hasGPRS : " + hasGPRS + "; Wifi: " + hasWifi);
		boolean bRes = hasGPRS || hasWifi;
		return bRes;
	}

	public static int getScreenHeight() {
		return Display.getHeight();
	}

	public static int getScreenWidth() {
		return Display.getWidth();
	}
	
	public static double getScreenPpiX() {
		// Convert PPM (Pixels Per Meter) to PPI (Pixels Per Inch)
		int ppm = Display.getHorizontalResolution();
		double retval = (ppm*25.4)/1000;
		return retval;
	}
	
	public static double getScreenPpiY() {
		// Convert PPM (Pixels Per Meter) to PPI (Pixels Per Inch)
		int ppm = Display.getVerticalResolution();
		double retval = (ppm*25.4)/1000;
		return retval;
	}
	
	public static void unzip_file(String strPath)throws Exception
	{
		IRAFile file = null;
		com.rho.file.SimpleFile fileZip = null;
		InputStream inputStream = null;
		GZIPInputStream is = null;
		try
		{
	        if (!strPath.startsWith("file:")) { 
	    		strPath = FilePath.join(RhoClassFactory.createFile().getDirPath(""), strPath);
	        }
			
			file = RhoClassFactory.createFSRAFile();
			String strOutFileName = new FilePath(strPath).getPathNoExt(); 
			file.open(strOutFileName, "rw");
				
			fileZip = RhoClassFactory.createFile();
			fileZip.open(strPath, true, false);
			
		    inputStream = fileZip.getInputStream();
		    //is = new ZLibInputStream(inputStream, false);
		    is = new GZIPInputStream(inputStream);
		    
			byte[]  byteBuffer = new byte[1024*20]; 
			int nRead = 0, nSize = 0;
			do{
				nRead = /*bufferedReadByByte(m_byteBuffer, is);*/is.read(byteBuffer);
				if ( nRead > 0 )
				{
					file.write(byteBuffer, 0, nRead);
					nSize += nRead;
				}
			}while( nRead >= 0 );
		    
		}finally
		{
			if ( inputStream != null )
				inputStream.close();
			if ( is != null )
				is.close();
			
			if ( fileZip != null )
				fileZip.close();
			if ( file != null )
				file.close();
			
		}
	}	
}
