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
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using System.Xml.Linq;


using System.IO.IsolatedStorage;
using System.IO;
using System.Windows.Resources;
using rho.common;
   
namespace Rhodes
{
    public partial class MainPage : PhoneApplicationPage
    {
        private static MainPage m_instance = null;
        public static MainPage Instance()
        {
            if (m_instance == null)
                m_instance = new MainPage();

            return m_instance;
        }

        private static string GetWinPhoneAttribute(string attributeName)
        {
            string ret = string.Empty;

            try
            {
                XElement xe = XElement.Load("WMAppManifest.xml");
                var attr = (from manifest in xe.Descendants("App")
                            select manifest).SingleOrDefault();
                if (attr != null)
                    ret = attr.Attribute(attributeName).Value;
            }
            catch
            {
                // Ignore errors in case this method is called
                // from design time in VS.NET
            }

            return ret;
        }
        
        // Constructor
        public MainPage()
        {
            InitializeComponent();
            ApplicationTitle.Text = GetWinPhoneAttribute("Title");
            m_instance = this;
            rhoview1.MainPage = this;
            rhoview1.MainPageLayoutRoot = LayoutRoot;
            rhoview1.MasterView = true;

            /*if (CRhoFile.isResourceFileExist("/apps/app/loading.png") == false)
            {
                LoadingImage.Visibility = Visibility.Collapsed;
                webBrowser2.Visibility = Visibility.Visible;
            }
            else
                webBrowser2.Visibility = Visibility.Collapsed;

            webBrowser1.Visibility = Visibility.Collapsed;
            webBrowser1.IsScriptEnabled = true;

            webBrowser1.Loaded += WebBrowser_OnLoaded;
            webBrowser1.LoadCompleted += WebBrowser_OnLoadCompleted;
            webBrowser1.Navigating += WebBrowser_OnNavigating;
            webBrowser1.Navigated += WebBrowser_OnNavigated;
            webBrowser1.ScriptNotify += WebBrowser_OnScriptNotify;
            //progressBar.IsIndeterminate = true;*/
        }

        private CRhodesApp RHODESAPP(){return CRhodesApp.Instance;}

        //private void Navigate(string url)
        //{
            //webBrowser1.NavigateToString("Hello!!!");
        //}

        //window.external.notify(<data>)
        private void WebBrowser_OnScriptNotify(object sender, NotifyEventArgs e)
        {
            //webBrowser1.InvokeScript("test_call3", "one");
        }

        private void WebBrowser_OnLoaded(object sender, RoutedEventArgs e)
        {
            //if (webBrowser2.Visibility == Visibility.Visible)
            //    webBrowser2.NavigateToString(CRhoFile.readStringFromResourceFile("/apps/app/loading.html"));
            //RHODESAPP().Init(webBrowser1, this);
        }

        private void WebBrowser_OnLoadCompleted(object sender, NavigationEventArgs e)
        {
        }

        private void WebBrowser_OnNavigating(object sender, NavigatingEventArgs e)
        {
            //if (!RHODESAPP().HttpServer.processBrowserRequest(e.Uri))
            //    return;

            //e.Cancel = true;
        }

        private void WebBrowser_OnNavigated(object sender, NavigationEventArgs e)
        {
            /*if (webBrowser1.Visibility == Visibility.Collapsed)
            {
                LoadingImage.Visibility = Visibility.Collapsed;
                webBrowser2.Visibility = Visibility.Collapsed;
                webBrowser1.Visibility = Visibility.Visible;
                progressBar.Visibility = Visibility.Collapsed;
            }*/
            RHODESAPP().addToHistory(e.Uri);
        }

        protected override void OnBackKeyPress(System.ComponentModel.CancelEventArgs e)
        {
            //if (Browser.Visibility == Visibility.Visible)
            //{
              //  Browser.Visibility = Visibility.Collapsed;
                e.Cancel = true;
                RHODESAPP().back();
           // }
        }


    }
}