﻿/*------------------------------------------------------------------------
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

using System;
using Microsoft.Phone.Controls;
using rho.net;
using rho;
using rho.db;
using rho.sync;
using System.Threading;
using System.Collections.Generic;
using Microsoft.Phone.Shell;
using IronRuby.Builtins;
using System.Windows.Media;
using System.Windows.Navigation;
using System.Windows.Controls;
using rho.views;
using System.Windows;

namespace rho.common
{
    public sealed class CRhodesApp
    {
        private static RhoLogger LOG = RhoLogger.RHO_STRIP_LOG ? new RhoEmptyLogger() : 
		    new RhoLogger("RhodesApp");
        RhoConf RHOCONF() { return RhoConf.getInstance(); }

        private static readonly CRhodesApp m_instance = new CRhodesApp();
        public static CRhodesApp Instance { get { return m_instance; } }
        private CRhodesApp() { }

        private WebBrowser m_webBrowser;
        private PhoneApplicationPage m_appMainPage;
        public PhoneApplicationPage MainPage { get { return m_appMainPage; } }
        private Grid m_layoutRoot;
        //private TabControl m_tabControl;
        private Pivot m_tabControl;
        public Pivot Tab { get { return m_tabControl; } }
        private Stack<Uri> m_backHistory = new Stack<Uri>();
        private Stack<Uri> m_forwardHistory = new Stack<Uri>();
        private Hash m_menuItems = null;
        private Uri m_currentUri = null; 
        private CHttpServer m_httpServer;
        private bool m_barIsStarted = false;
        int m_currentTabIndex = 0;
        String[] m_currentUrls = new String[5];
        private String m_strBlobsDirPath, m_strDBDirPath;
        private String m_strHomeUrl;
        private String m_strAppBackUrl;
        private RhoView m_rhoView;
        private static RhoView m_masterView = null;
        public  bool m_transition = false;
        ManualResetEvent m_UIWaitEvent = new ManualResetEvent(false);
        Vector<Object> m_arCallbackObjects = new Vector<Object>();

        public WebBrowser WebBrowser{ get { return m_webBrowser; } }
        public CHttpServer HttpServer{ get { return m_httpServer; } }
        public CRhoRuby RhoRuby { get { return CRhoRuby.Instance; } }
        public bool barIsStarted { get { return m_barIsStarted; } }
        public Stack<Uri> BackHistory { get { return m_backHistory; } set { m_backHistory = value; } }
        public Stack<Uri> ForwardHistory { get { return m_forwardHistory; } set { m_forwardHistory = value; } }
        public Uri CurrentUri { get { return m_currentUri; } set { m_currentUri = value; } }
        public RhoView rhoView { get { return m_rhoView; } }

        public String getBlobsDirPath() { return m_strBlobsDirPath; }
        public String getHomeUrl() { return m_strHomeUrl; }

        private void Pivot_OnChanged(object sender, RoutedEventArgs e)
        {
            m_currentTabIndex = m_tabControl.SelectedIndex;
            ((RhoView)(((PivotItem)m_tabControl.SelectedItem).Content)).refresh();
        }

        public void Init(WebBrowser browser, PhoneApplicationPage appMainPage, Grid layoutRoot, RhoView rhoView)
        {
            initAppUrls();
            RhoLogger.InitRhoLog();
            LOG.INFO("Init");
            CRhoFile.recursiveCreateDir(CFilePath.join(getBlobsDirPath()," "));

            m_webBrowser = browser;
            if (m_appMainPage == null)
                m_appMainPage = appMainPage;
            if (m_layoutRoot == null)
                m_layoutRoot = layoutRoot;
            //m_appMainPage.ApplicationBar = null;

            if (m_httpServer == null)
                m_httpServer = new CHttpServer(CFilePath.join(getRhoRootPath(), "apps"));

            m_rhoView = rhoView;
            if (m_rhoView.MasterView)
                m_masterView = rhoView;
        }

        public void startApp()
        {
            LOG.INFO("startApp");

            CRhoResourceMap.deployContent();
            RhoRuby.Init(m_webBrowser);

            DBAdapter.initAttrManager();

            LOG.INFO("Starting sync engine...");
            SyncThread sync = null;
            try{
	        	sync = SyncThread.Create();
	        	
	        }catch(Exception exc){
	        	LOG.ERROR("Create sync failed.", exc);
	        }
	        if (sync != null) {
	        	//sync.setStatusListener(this);
	        }
	        
	        RhoRuby.InitApp();
	        RhoRuby.call_config_conflicts();
            RHOCONF().conflictsResolved();
        }

        public void closeApp()
        {
            m_httpServer.stop(10);

            RhoLogger.close();
            m_UIWaitEvent.Close();
        }

        public void stopApp()
        {
            //string[] ar1 = CRhoFile.enumDirectory("db");

            RhoRuby.callUIDestroyed();

            SyncThread.getInstance().Destroy();
            CAsyncHttp.Destroy();

            RhoRuby.Stop();

            //string[] ar2 = CRhoFile.enumDirectory("db");
            //int i = 0;
            //net::CAsyncHttp::Destroy();
        }

        public bool isMainRubyThread()
        {
            return m_httpServer.CurrentThread == Thread.CurrentThread;
        }

        void initAppUrls()
        {
            m_strHomeUrl = "http://localhost:2375";
            m_strBlobsDirPath = getRhoRootPath() + "db/db-files";
            m_strDBDirPath = getRhoRootPath() + "db";
        }

        boolean isExternalUrl(String strUrl)
        {
            return strUrl.startsWith("http://") || strUrl.startsWith("https://") ||
                strUrl.startsWith("javascript:") || strUrl.startsWith("mailto:")
                 || strUrl.startsWith("tel:") || strUrl.startsWith("wtai:") ||
                 strUrl.startsWith("sms:");
        }

        public String canonicalizeRhoPath(String path)
        {
            return path;
        }

        public String canonicalizeRhoUrl(String url)
        {
            if (url == null || url.length() == 0)
                return getHomeUrl();

            String strUrl = url.Replace('\\', '/');
            if (!strUrl.startsWith(getHomeUrl()) && !isExternalUrl(strUrl))
                strUrl = CFilePath.join(getHomeUrl(), strUrl);

            return strUrl;
        }

        public boolean isRhodesAppUrl(String url)
        {
            return url.startsWith(getHomeUrl());
        }

        public NetResponse processCallback(String strUrl, String strBody)
        {
//            m_webBrowser.Dispatcher.BeginInvoke( () =>
//            {
                String strReply;
                m_httpServer.call_ruby_method(strUrl, strBody, out strReply);

//                m_UIWaitEvent.Set();
//            });

//            m_UIWaitEvent.Reset();
//            m_UIWaitEvent.WaitOne();

            return new NetResponse("", 200);
        }

        public void processWebNavigate(String strUrl, int index)
        {
            m_webBrowser.Dispatcher.BeginInvoke( () =>
            {
                try{
                    if (m_tabControl != null && m_tabControl.Items.Count > 0)
                    {
                        if (index == -1)
                            index = getCurrentTab();

                        ((RhoView)((PivotItem)m_tabControl.Items[index]).Content).webBrowser1.IsScriptEnabled = true;
                        if (isExternalUrl(strUrl))
                            ((RhoView)((PivotItem)m_tabControl.Items[index]).Content).webBrowser1.Navigate(new Uri(strUrl, UriKind.Absolute));
                        else
                            ((RhoView)((PivotItem)m_tabControl.Items[index]).Content).webBrowser1.Navigate(new Uri(strUrl, UriKind.Relative));
                    }
                    else
                    {
                        m_webBrowser.IsScriptEnabled = true;
                        if (isExternalUrl(strUrl))
                            m_webBrowser.Navigate(new Uri(strUrl, UriKind.Absolute));
                        else
                            m_webBrowser.Navigate(new Uri(strUrl, UriKind.Relative));
                    }
                }
                catch (Exception exc)
                {
                    LOG.ERROR("WebView.navigate failed : " + strUrl + ", index: " + index, exc);
                }
            });
        }

        public void processWebRefresh(int index)
        {
            m_webBrowser.Dispatcher.BeginInvoke(() =>
            {
                try
                {
                    if (m_tabControl != null && m_tabControl.Items.Count > 0)
                    {
                        if (index == -1)
                            index = this.getCurrentTab();

                        ((RhoView)((PivotItem)m_tabControl.Items[index]).Content).webBrowser1.Navigate(m_webBrowser.Source);
                    }
                    else
                    {
                        m_webBrowser.Navigate(m_webBrowser.Source);
                    }
                }
                catch (Exception exc)
                {
                    LOG.ERROR("WebView.refresh failed : " + index, exc);
                }
            });
        }

        public int getTabIndexFor(object obj)
        {
            if (!(obj is WebBrowser) || m_tabControl == null || m_tabControl.Items.Count == 0) return -1;

            for (int i = 0; i < m_tabControl.Items.Count; i++)
            {
                if (obj == ((RhoView)((PivotItem)m_tabControl.Items[i]).Content).webBrowser1) return i;
            }

            return -1;
        }

        public void processInvokeScriptArgs(String strFuncName, String[] arrParams, int index)
        {
            if (null != strFuncName && 0 < strFuncName.Length && (null == arrParams || 0 == arrParams.Length))
            {
                arrParams = new String[]{strFuncName};
                strFuncName = CHttpServer.JS_EVAL_FUNCNAME;
            }
            m_webBrowser.Dispatcher.BeginInvoke(() =>
            {
                try
                {
                    if (m_tabControl != null && m_tabControl.Items.Count > 0)
                    {
                        if (index == -1)
                            index = getCurrentTab();

                        ((RhoView)((PivotItem)m_tabControl.Items[index]).Content).webBrowser1.InvokeScript(strFuncName, arrParams);
                    }
                    else
                    {
                        m_webBrowser.InvokeScript(strFuncName, arrParams);
                    }
                }
                catch (Exception exc)
                {
                    LOG.ERROR("WebView.execute_js failed: " + strFuncName + "(" + String.Join(",", arrParams), exc);
                }
            });
        }
        
        public static String getRhoRootPath()
        {
            return "/";
        }

        public String resolveDBFilesPath(String strFilePath)
        {
            if (strFilePath.length() == 0 || strFilePath.startsWith(getRhoRootPath()))
                return strFilePath;

            return CFilePath.join(getRhoRootPath(), strFilePath);
        }

        public String getCurrentUrl(int index)
        {
            if ( index == -1 )
                index = m_currentTabIndex;

            return m_currentUrls[index];
        }

        public void keepLastVisitedUrl(String strUrl)
        {
            m_currentUrls[m_currentTabIndex] = canonicalizeRhoUrl(strUrl);
        }

        public String getPlatform()
        {
            return "WINDOWS_PHONE";
        }

        public boolean unzip_file(String path)
        {
            //TODO: unzip_file
            return false;
        }

        private static Color getColorFromString(String strColor)
        {
            if (strColor == null || strColor == "")
                return Color.FromArgb(255, 0, 0, 0);

	        int c = Convert.ToInt32(strColor);

	        int cR = (c & 0xFF0000) >> 16;
	        int cG = (c & 0xFF00) >> 8;
	        int cB = (c & 0xFF);

            return Color.FromArgb(255, Convert.ToByte(cR), Convert.ToByte(cG), Convert.ToByte(cB));
        }

        private String getDefaultImagePath(String strAction)
        {
            String strImagePath = "";
            if ( strAction == "options" )
                strImagePath = "lib/res/options_btn.png";
            else if ( strAction == "home" )
                strImagePath = "lib/res/home_btn.png";
            else if ( strAction == "refresh" )
                strImagePath = "lib/res/refresh_btn.png";
            else if ( strAction == "back" )
                strImagePath = "lib/res/back_btn.png";
            else if ( strAction == "forward" )
                strImagePath = "lib/res/forward_btn.png";

            return strImagePath.Length > 0 ? CFilePath.join(getRhoRootPath(), strImagePath) : null;
        }   

        private void createToolBarButtons(int barType, Object[] hashArray)
        {
            int bCount = 0;
            for (int i = 0; hashArray != null && i < hashArray.Length; i++)
            {
                if (hashArray[i] != null && hashArray[i] is Hash)
                {
                    String action = null;
                    String icon = null;
                    String label = null;
                    object val = null;

                    Hash values = (Hash)hashArray[i];
                    if (values.TryGetValue(CRhoRuby.CreateSymbol("action"), out val))
                        action = val.ToString();
                    if (values.TryGetValue(CRhoRuby.CreateSymbol("icon"), out val))
                        icon = val.ToString();
                    if (values.TryGetValue(CRhoRuby.CreateSymbol("label"), out val))
                        label = val.ToString();

                    if (label == null && barType == 0)
                        label = ".";//Text can not be empty. it's WP7's restriction!!!

                    if (icon == null)//icon can not be null or empty. so now i don't know how to create separator
                    {
                        icon = getDefaultImagePath(action);
                    }

                    if (icon == null || action == null)
                        continue;

                    if (action == "forward" || action == "close")// && RHOCONF().getBool("jqtouch_mode"))
                        continue;
                    ApplicationBarIconButton button = new ApplicationBarIconButton(new Uri(icon, UriKind.Relative));
                    button.Text = label;
                    button.Click += delegate(object sender, EventArgs e) { processToolBarCommand(sender, e, action); };

                    m_appMainPage.ApplicationBar.Buttons.Add(button);
                    //ApplicationBar Allows developers to create and display an application bar
                    //with between 1 and 4 buttons and a set of text menu items
                    //in Windows Phone applications.
                    if (++bCount == 4)
                        return;
                }
            }
        }

        public void setMenuItems(Hash menuItems)
        {
            m_appMainPage.Dispatcher.BeginInvoke(() =>
            {
                if (m_appMainPage.ApplicationBar == null)
                    createEmptyToolBar();
                else
                {
                    m_appMainPage.ApplicationBar.MenuItems.Clear();
                    setAppBackUrl("");
                }

                m_menuItems = menuItems;

                foreach (KeyValuePair<object, object> kvp in m_menuItems)
                {
                    ApplicationBarMenuItem item = new ApplicationBarMenuItem();
                    item.Text = kvp.Key.ToString();
                    String action = null; 
                    if (kvp.Value == null) 
                        continue;
                    else
                        action = kvp.Value.ToString();

                    if (action == "close") continue;

                    if (item.Text.toLowerCase() == "back" && action.toLowerCase() != "back") setAppBackUrl(action);

                    item.Click += delegate(object sender, EventArgs e) { processToolBarCommand(sender, e, action); };

                    m_appMainPage.ApplicationBar.MenuItems.Add(item);
                }
            });
        }

        private void createEmptyToolBar()
        {
            m_appMainPage.ApplicationBar = new ApplicationBar();
            m_appMainPage.ApplicationBar.IsMenuEnabled = true;
            m_appMainPage.ApplicationBar.IsVisible = true;
            m_appMainPage.ApplicationBar.Opacity = 1.0;
            setAppBackUrl("");
        }

        public void createToolBar(int barType, Object barParams)
        {
            m_appMainPage.Dispatcher.BeginInvoke(() =>
            {
                createEmptyToolBar();

                Object[] hashArray = null;
                Hash paramHash = null;
                object val = null;

                if (barParams is RubyArray)
                    hashArray = ((RubyArray)barParams).ToArray();
                else
                    paramHash = (Hash)barParams;

                if (paramHash != null && paramHash.TryGetValue(CRhoRuby.CreateSymbol("background_color"), out val))
                    m_appMainPage.ApplicationBar.BackgroundColor = getColorFromString(val.ToString());

                if (paramHash != null && paramHash.TryGetValue(CRhoRuby.CreateSymbol("buttons"), out val) && val is RubyArray)
                    hashArray = ((RubyArray)val).ToArray();

                createToolBarButtons(barType, hashArray);
                setMenuItems(m_menuItems);
                m_barIsStarted = true;
            });
        }

        public void removeToolBar()
        {
            m_appMainPage.Dispatcher.BeginInvoke(() =>
            {
                if (m_appMainPage.ApplicationBar != null)
                {
                    m_appMainPage.ApplicationBar.MenuItems.Clear();
                    m_appMainPage.ApplicationBar.Buttons.Clear();
                    m_appMainPage.ApplicationBar.IsMenuEnabled = false;
                    m_appMainPage.ApplicationBar.IsVisible = false;

                    m_appMainPage.ApplicationBar = null;
                    m_barIsStarted = false;
                }
            });
        }

        public void showLogScreen()
        {
        }

        public void addToHistory(Uri uri)
        {
            Uri previous = null;
            if (m_backHistory.Count > 0)
                previous = m_backHistory.Peek();

            if (uri == previous)
            {
                m_backHistory.Pop();
                m_forwardHistory.Push(m_currentUri);
            }
            else
            {
                if (m_currentUri != null)
                {
                    m_backHistory.Push(m_currentUri);
                    if (m_forwardHistory.Count > 0)
                        m_forwardHistory.Pop();
                }
            }
            m_currentUri = uri; 
        }

        public void back()
        {
            if (m_strAppBackUrl.length() > 0)
                processToolBarCommand(this, null, m_strAppBackUrl);
            else if (m_backHistory.Count > 0)
            {
                Uri destination = m_backHistory.Peek();
                m_webBrowser.Navigate(destination);
            }
            return;
        }
        
        private void processToolBarCommand(object sender, EventArgs e,  String strAction)
        {

            boolean callback = false;
            if (strAction == "back")
            {
                if (m_strAppBackUrl.length() > 0)
                    processToolBarCommand(this, e, m_strAppBackUrl);
                else if (m_backHistory.Count > 0)
                {
                    Uri destination = m_backHistory.Peek();
                    m_webBrowser.Navigate(destination);
                }
                return;
            }

            if (strAction.startsWith("callback:"))
            {
                strAction = strAction.substring(9);
                callback = true;
            }

            if (strAction == "forward" && m_forwardHistory.Count > 0)
            {
                Uri destination = m_forwardHistory.Peek();
                m_webBrowser.Navigate(destination);
                return;
            }

            if (strAction == "home")
            {
                String strHomePage = RhoRuby.getStartPage();
                strHomePage = canonicalizeRhoUrl(strHomePage);
                m_webBrowser.Navigate(new Uri(strHomePage));
                return;
            }

            if (strAction == "log")
            {
                showLogScreen();
                return;
            }

            if (strAction == "options")
            {
                String curUrl = RhoRuby.getOptionsPage();
				curUrl = canonicalizeRhoUrl(curUrl);
			    m_webBrowser.Navigate(new Uri(curUrl));
                return;
            }

            if (strAction == "refresh" && m_currentUri != null)
            {
                m_webBrowser.Navigate(m_currentUri);
                return;
            }

            if (strAction == "sync")
            {
                SyncThread.doSyncAllSources(true);
                return;
            }

            strAction = canonicalizeRhoUrl(strAction);
            if (callback)
            {
                RhoClassFactory.createNetRequest().pushData(strAction, "rho_callback=1", null);
            }
            else
                m_webBrowser.Navigate(new Uri(strAction));
        }

        public String addCallbackObject(Object valObject, String strName)
        {
            int nIndex = -1;
            for (int i = 0; i < (int)m_arCallbackObjects.size(); i++)
            {
                if (m_arCallbackObjects.elementAt(i) == null)
                    nIndex = i;
            }
            if (nIndex == -1)
            {
                m_arCallbackObjects.addElement(valObject);
                nIndex = m_arCallbackObjects.size() - 1;
            }
            else
                m_arCallbackObjects[nIndex] = valObject;

            String strRes = "__rho_object[" + strName + "]=" + nIndex;

            return strRes;
        }

        public Object getCallbackObject(int nIndex)
        {
            if (nIndex < 0 || nIndex > m_arCallbackObjects.size())
                return null;

            Object res = (Object)m_arCallbackObjects.elementAt(nIndex);
            m_arCallbackObjects[nIndex] = null;
            return res;
        }

        public void setAppBackUrl(String url)
        {
            m_strAppBackUrl = url;
            if (m_backHistory != null)
                m_backHistory.Clear();
        }

        private void createTabBarButtons(int barType, Object[] hashArray)
        {
            for (int i = 0; hashArray != null && i < hashArray.Length; i++)
            {
                if (hashArray[i] != null && hashArray[i] is Hash)
                {
                    String action = null;
                    String icon = null;
                    String label = null;
                    Brush web_bkg_color = null;
                    bool reload = false;
                    bool use_current_view_for_tab = false;
                    object val = null;

                    Hash values = (Hash)hashArray[i];
                    if (values.TryGetValue(CRhoRuby.CreateSymbol("action"), out val))
                        action = val.ToString();
                    if (values.TryGetValue(CRhoRuby.CreateSymbol("icon"), out val))
                        icon = val.ToString();
                    if (values.TryGetValue(CRhoRuby.CreateSymbol("label"), out val))
                        label = val.ToString();
                    if (values.TryGetValue(CRhoRuby.CreateSymbol("reload"), out val))
                        reload = Convert.ToBoolean(val);
                    if (values.TryGetValue(CRhoRuby.CreateSymbol("web_bkg_color"), out val))
                        web_bkg_color = new SolidColorBrush(getColorFromString(val.ToString()));
                    if (values.TryGetValue(CRhoRuby.CreateSymbol("use_current_view_for_tab"), out val))
                        use_current_view_for_tab = Convert.ToBoolean(val);

                    //if (label == null && barType == 0)
                    //    label = ".";//Text can not be empty. it's WP7's restriction!!!

                    if (icon == null)//icon can not be null or empty. so now i don't know how to create separator
                    {
                        icon = getDefaultImagePath(action);
                    }

                    //if (icon == null || action == null)
                       // continue;


                    //PivotItem.Header = "Test";to do
                    //PivotItem PivotItem = new PivotItem();
                    PivotItem PivotItem = new PivotItem();
                    PivotItem.Header = new RhoTabHeader(label, icon);
                    if (i == 0)
                    {// && use_current_view_for_tab)
                        ((Grid)m_masterView.Parent).Children.Remove(m_masterView);
                        m_masterView.Init(m_appMainPage, m_layoutRoot, action, reload, web_bkg_color, i);
                        PivotItem.Content = m_masterView;
                    }
                    else
                        PivotItem.Content = new RhoView(m_appMainPage, m_layoutRoot, action, reload, web_bkg_color, i);
                    if (values.TryGetValue(CRhoRuby.CreateSymbol("selected_color"), out val))
                        PivotItem.Background = new SolidColorBrush(getColorFromString(val.ToString()));
                    if (values.TryGetValue(CRhoRuby.CreateSymbol("disabled"), out val))
                        PivotItem.IsEnabled = !Convert.ToBoolean(val);
                    m_tabControl.Items.Add(PivotItem);
                }
            }
        }

        public void createTabBar(int tabBarType, Object tabBarParams)
        {
            m_appMainPage.Dispatcher.BeginInvoke( () =>
            {
                /*m_tabControl = new TabControl();
                if (tabBarType == 1)
                    m_tabControl.TabStripPlacement = Dock.Top;
                else if (tabBarType == 3)
                    m_tabControl.TabStripPlacement = Dock.Left;*/
                
                m_tabControl = new Pivot();
                m_tabControl.SelectionChanged += Pivot_OnChanged;

                Object[] hashArray = null;
                Hash paramHash = null;
                object val = null;

                if (tabBarParams is RubyArray)
                    hashArray = ((RubyArray)tabBarParams).ToArray();
                else
                    paramHash = (Hash)tabBarParams;

                if (paramHash != null && paramHash.TryGetValue(CRhoRuby.CreateSymbol("background_color"), out val))
                    m_tabControl.Background = new SolidColorBrush(getColorFromString(val.ToString()));

                if (paramHash != null && paramHash.TryGetValue(CRhoRuby.CreateSymbol("tabs"), out val) && val is RubyArray)
                    hashArray = ((RubyArray)val).ToArray();

                createTabBarButtons(tabBarType, hashArray);
                m_tabControl.Margin = new Thickness(0, 70, 0, 0);
                m_layoutRoot.Children.Add(m_tabControl);
                m_masterView.removeBrowser();
            });
        }

        public void removeTabBar()
        {
            m_appMainPage.Dispatcher.BeginInvoke( () =>
            {
                if (m_tabControl != null)
                {
                    m_tabControl.Items.Clear();
                    ((Grid)m_masterView.Parent).Children.Add(m_masterView);
                    m_layoutRoot.Children.Remove(m_tabControl);
                    m_masterView.refresh();
                }
            });
        }

        public void switchTab(int index)
        {
            m_appMainPage.Dispatcher.BeginInvoke(() =>
            {
                if (m_tabControl != null)
                {
                    m_tabControl.SelectedIndex = index;
                }
            });
        }

        public void setTabBadge(int index, MutableString val)
        {
            m_appMainPage.Dispatcher.BeginInvoke(() =>
            {
                if (m_tabControl != null)
                {
                    //TO DO
                    //((PivotItem)m_tabControl.Items[index]).Header
                }
            });
        }

        public int getCurrentTab()
        {
            return m_currentTabIndex;
        }
    }
}
