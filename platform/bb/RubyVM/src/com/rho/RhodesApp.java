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

import com.rho.net.*;
import com.xruby.runtime.lang.RubyValue;
import java.util.Vector;

public class RhodesApp
{
	private static final RhoLogger LOG = RhoLogger.RHO_STRIP_LOG ? new RhoEmptyLogger() : 
		new RhoLogger("RhodesApp");
	RhoConf RHOCONF(){ return RhoConf.getInstance(); }
	
	static RhodesApp m_pInstance;
	
    private String m_strRhoRootPath, m_strBlobsDirPath, m_strDBDirPath;
	private String m_strStartUrl, m_strHomeUrl;
	private String m_strAppBackUrl = "", m_strAppBackUrlOrig = "";
	
    Vector/*<unsigned long>*/ m_arCallbackObjects = new Vector();
    private SplashScreen m_oSplashScreen = new SplashScreen();
    
    Mutex m_mxPushCallback = new Mutex();
    String m_strPushCallback = "", m_strPushCallbackParams = "";

	Mutex m_mxScreenRotationCallback = new Mutex();
    String m_strScreenRotationCallback = "", m_strScreenRotationCallbackParams = "";
    static String m_strStartParameters = "";
    static boolean m_bSecurityTokenNotPassed = false;
    
    int m_currentTabIndex = 0;
    String[] m_currentUrls = new String[5];
    RhoTimer m_oTimer = new RhoTimer();
    
    public RhoTimer     getTimer(){ return m_oTimer; }
    String getAppBackUrl(){return m_strAppBackUrl;}

    public static RhodesApp Create(String strRootPath)
    {
        if ( m_pInstance != null ) 
            return m_pInstance;

        m_pInstance = new RhodesApp(strRootPath);
        return m_pInstance;
    }
    
    public static void Destroy()
    {
    	
    }
    
    public static RhodesApp getInstance(){ return m_pInstance; }
    public SplashScreen getSplashScreen(){return m_oSplashScreen;}
	
    public String getBlobsDirPath(){return m_strBlobsDirPath; }
    public String getDBDirPath(){return m_strDBDirPath; }
    public String getRhoRootPath(){return m_strRhoRootPath;}
    
    NetRequest getNet() { return RhoClassFactory.createNetRequest();}
    public String getHomeUrl(){ return m_strHomeUrl; }
    
    public static void setStartParameters(String szParams ){ m_strStartParameters = (szParams != null? szParams : ""); }
    public static String getStartParameters(){ return m_strStartParameters; }
    
    public static void setSecurityTokenNotPassed(boolean is_not_passed) {m_bSecurityTokenNotPassed = is_not_passed;}
    public static boolean isSecurityTokenNotPassed() {return m_bSecurityTokenNotPassed;}
    
    RhodesApp(String strRootPath)
    {
        m_strRhoRootPath = strRootPath;
        
        getSplashScreen().init();
        
        initAppUrls();        
    }

    void initAppUrls() 
    {
    	m_strHomeUrl = "http://localhost:2375";
        m_strBlobsDirPath = getRhoRootPath() + "db/db-files";
    	m_strDBDirPath = getRhoRootPath() + "db";
    }
    
    public String resolveDBFilesPath(String strFilePath)
    {
        if ( strFilePath.length() == 0 || strFilePath.startsWith(getRhoRootPath()) )
            return strFilePath;

        return FilePath.join(getRhoRootPath(), strFilePath);
    }
    
    public String addCallbackObject(RubyValue valObject, String strName)
    {
        int nIndex = -1;
        for (int i = 0; i < (int)m_arCallbackObjects.size(); i++)
        {
            if ( m_arCallbackObjects.elementAt(i) == null )
                nIndex = i;
        }
        if ( nIndex  == -1 )
        {
            m_arCallbackObjects.addElement(valObject);
            nIndex = m_arCallbackObjects.size()-1;
        }else
            m_arCallbackObjects.setElementAt(valObject,nIndex);

        String strRes = "__rho_object[" + strName + "]=" + nIndex;

        return strRes;
    }

    public RubyValue getCallbackObject(int nIndex)
    {
        if ( nIndex < 0 || nIndex > m_arCallbackObjects.size() )
            return null;

        RubyValue res = (RubyValue)m_arCallbackObjects.elementAt(nIndex);
        m_arCallbackObjects.setElementAt(null,nIndex);        
        return res;
    }
    
    public boolean isExternalUrl(String strUrl)
    {
    	return strUrl.indexOf(':') != -1;
    	//strUrl.startsWith("http://") || strUrl.startsWith("https://") ||
    	//	strUrl.startsWith("javascript:") || strUrl.startsWith("mailto:")
    	//	 || strUrl.startsWith("tel:")|| strUrl.startsWith("wtai:") ||
    	//	 strUrl.startsWith("sms:");    
    }
    
    public String canonicalizeRhoUrl(String url) 
    {
		if ( url == null || url.length() == 0 )
			return getHomeUrl();

		String strUrl = new String(url);
		strUrl.replace('\\', '/');
		if ( !strUrl.startsWith(getHomeUrl()) && !isExternalUrl(strUrl) )
			strUrl = FilePath.join(getHomeUrl(), strUrl);
		
		return strUrl;
    }
    
    public boolean isRhodesAppUrl(String url)
    {
    	return url.startsWith(getHomeUrl()); 
    }
    
    public void setPushNotification(String strUrl, String strParams )throws Exception
    {
        synchronized(m_mxPushCallback)
        {
            m_strPushCallback = canonicalizeRhoUrl(strUrl);
            m_strPushCallbackParams = strParams;
        }
    }

    public boolean callPushCallback(String strData)throws Exception
    {
        synchronized(m_mxPushCallback)
        {
            if ( m_strPushCallback.length() == 0 )
                return false;
        	
            String strBody = strData + "&rho_callback=1";
            if ( m_strPushCallbackParams.length() > 0 )
                strBody += "&" + m_strPushCallbackParams;

            NetResponse resp = getNet().pushData( m_strPushCallback, strBody, null );
            if (!resp.isOK())
                LOG.ERROR("Push notification failed. Code: " + resp.getRespCode() + "; Error body: " + resp.getCharData() );
            else
            {
                String szData = resp.getCharData();
                return !(szData != null && szData.equals("rho_push"));
            }
        }

        return false;
    }
 
    public void setScreenRotationNotification(String strUrl, String strParams)throws Exception
    {
        synchronized(m_mxScreenRotationCallback)
        {
            m_strScreenRotationCallback = canonicalizeRhoUrl(strUrl);
            m_strScreenRotationCallbackParams = strParams;
        }
    }

    public void callScreenRotationCallback(int width, int height, int degrees)throws Exception
    {
    	synchronized(m_mxScreenRotationCallback) 
    	{
    		if (m_strScreenRotationCallback.length() == 0)
    			return;
    		
    		String strBody = "rho_callback=1";
    		
            strBody += "&width=";   strBody += width;
    		strBody += "&height=";  strBody += height;
    		strBody += "&degrees="; strBody += degrees;
    		
            if ( m_strScreenRotationCallbackParams.length() > 0 )
                strBody += "&" + m_strPushCallbackParams;
    			
            NetResponse resp = getNet().pushData( m_strScreenRotationCallback, strBody, null );
            if (!resp.isOK()) {
                LOG.ERROR("Screen rotation notification failed. Code: " + resp.getRespCode() + "; Error body: " + resp.getCharData());
            }
        }
    }
    
    public void callPopupCallback(String strCallbackUrl, String id, String title)throws Exception
    {
        if ( strCallbackUrl == null || strCallbackUrl.length() == 0 )
            return;
    	
        strCallbackUrl = canonicalizeRhoUrl(strCallbackUrl);
        String strBody = "button_id=" + id + "&button_title=" + title;
        strBody += "&rho_callback=1";
        //getNet().pushData( strCallbackUrl, strBody, null );
        
		IRhoRubyHelper helper = RhoClassFactory.createRhoRubyHelper();
		helper.postUrlNoWait(strCallbackUrl,strBody);
    }
 
    public String getCurrentUrl(int index)
    { 
        return m_currentUrls[m_currentTabIndex]; 
    }
    
    public void keepLastVisitedUrl(String strUrl)
    {
        //LOG(INFO) + "Current URL: " + strUrl;

    	try{
	        m_currentUrls[m_currentTabIndex] = canonicalizeRhoUrl(strUrl);
    	}catch(Exception exc)
    	{
    		LOG.ERROR("Save current location failed.", exc);
    	}
	}
    
    String getStartUrl()
    {
        m_strStartUrl = canonicalizeRhoUrl( RHOCONF().getString("start_path") );
        return m_strStartUrl;
    }
    
    void navigateToUrl(String url)throws Exception
    {
    	IRhoRubyHelper helper = RhoClassFactory.createRhoRubyHelper();
    	helper.navigateUrl(url);
    }
    
    public void loadUrl(String url)throws Exception
    {
        boolean callback = false;
        if (url.startsWith("callback:") )
        {
            callback = true;
            url = url.substring(9);
        }else if ( url.equalsIgnoreCase("exit") || url.equalsIgnoreCase("close") )
        {
        	IRhoRubyHelper helper = RhoClassFactory.createRhoRubyHelper();
        	helper.app_exit();
            return;
        }

        url = canonicalizeRhoUrl(url);
        if (callback)
        {
            getNet().pushData( url,  "rho_callback=1", null );
        }
        else
            navigateToUrl(url);
    }
    
    public void setAppBackUrl(String url)
    {
        if ( url != null && url.length() > 0 )
        {
            m_strAppBackUrlOrig = url;
            m_strAppBackUrl = canonicalizeRhoUrl(url);
        }
        else
        {
            m_strAppBackUrlOrig = "";
            m_strAppBackUrl = "";
        }
    }
    
    public void navigateBack()
    {
    	try{
	        if ( m_strAppBackUrlOrig.length() > 0 )
	            loadUrl(m_strAppBackUrlOrig);
	        else if ( !getCurrentUrl(0).equalsIgnoreCase(getStartUrl()) )
	        {
	        	IRhoRubyHelper helper = RhoClassFactory.createRhoRubyHelper();
	        	helper.navigateBack();
	        }
    	}catch(Exception exc)
    	{
    		LOG.ERROR("Navigate back failed.", exc);
    	}
    }
    
    public boolean callTimerCallback( String strUrl, String strData)
    {
        String strBody = "rho_callback=1";
    		
        if ( strData != null && strData.length() > 0 )
            strBody += "&" + strData;

        try
        {
	        IRhoRubyHelper helper = RhoClassFactory.createRhoRubyHelper();
			NetResponse resp = helper.postUrlSync(canonicalizeRhoUrl(strUrl),strBody);
        }catch(Exception exc)
        {
        	LOG.ERROR("callTimerCallback failed.", exc);
        	return false;
        }
        return true;
    }
   
    public void callSignatureCallback( String strCallbackUrl, String strSignaturePath, String strError, boolean bCancel ) throws Exception
    {
    	strCallbackUrl = canonicalizeRhoUrl(strCallbackUrl);
    	String strBody;
    	if ( bCancel || strError.length() > 0 )
    	{
    		if ( bCancel )
    			strBody = "status=cancel&message=User canceled operation.";
    		else
    			strBody = "status=error&message=" + strError;
    	}else
    		strBody = "status=ok&signature_uri=db%2Fdb-files%2F" + strSignaturePath;

    	strBody += "&rho_callback=1";
    	
    	IRhoRubyHelper helper = RhoClassFactory.createRhoRubyHelper();
		helper.postUrlNoWait(strCallbackUrl,strBody);
    }    
}

