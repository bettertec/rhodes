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

#pragma once

#ifndef RHODES_EMULATOR

#if !defined(_WIN32_WCE) || defined( OS_PLATFORM_MOTCE )
#include <exdispid.h>
#include <exdisp.h>
#endif

#include "resource.h"
#include "logging/RhoLog.h"
#include "common/RhoConf.h"
#include "common/RhodesApp.h"
#include "Alert.h"
#include "RhoNativeViewManagerWM.h"
#include "SyncStatusDlg.h"
#include "rho/rubyext/NativeToolbar.h"
#include "IBrowserEngine.h"
#include "common/app_build_capabilities.h"

#if defined(OS_WINDOWS)
#include "menubar.h"
#endif

#include "LogView.h"

#ifdef APP_BUILD_CAPABILITY_WEBKIT_BROWSER
#define PB_NAVIGATETAB					WM_USER	+ 1
#define PB_GEN_ACTIVEX					WM_USER	+ 2
#define PB_GEN_QUIT						WM_USER + 3
#define PB_INIT							WM_USER	+ 4
#define PB_SETPLG_PROPERTY				WM_USER	+ 5
#define PB_CALLPLG_METHOD				WM_USER + 6
#define PB_DEFTAGS						WM_USER + 7
#define PB_ONMETA						WM_USER + 8
#define PB_ONTOPMOSTWINDOW				WM_USER + 9
#define PB_WINDOW_RESTORE				WM_USER + 10
#define PB_SCREEN_ORIENTATION_CHANGED	WM_USER + 11
#define PB_NEWGPSDATA					WM_USER + 12

extern UINT WM_BROWSER_ONDOCUMENTCOMPLETE;
extern UINT WM_BROWSER_ONNAVIGATECOMPLETE;
extern UINT WM_BROWSER_ONTITLECHANGE;
extern UINT WM_BROWSER_ONBEFORENAVIGATE;
extern UINT WM_BROWSER_ONNAVIGATIONTIMEOUT;
extern UINT WM_BROWSER_ONNAVIGATIONERROR;
extern UINT WM_BROWSER_ONSETSIPSTATE;
extern UINT WM_BROWSER_ONALERTPOPUP;

#else
#if defined (_WIN32_WCE) && !defined( OS_PLATFORM_MOTCE )
#include <pvdispid.h>
#include <piedocvw.h>
#endif
#endif //APP_BUILD_CAPABILITY_WEBKIT_BROWSER

#define ID_CUSTOM_MENU_ITEM_FIRST (WM_APP+3)
#define ID_CUSTOM_MENU_ITEM_LAST  (ID_CUSTOM_MENU_ITEM_FIRST + (APP_MENU_ITEMS_MAX) - 1)
#define ID_CUSTOM_TOOLBAR_ITEM_FIRST (ID_CUSTOM_MENU_ITEM_LAST+1)
#define ID_CUSTOM_TOOLBAR_ITEM_LAST  (ID_CUSTOM_TOOLBAR_ITEM_FIRST + 20 - 1)

static const UINT ID_BROWSER = 1;

static UINT WM_TAKEPICTURE             = ::RegisterWindowMessage(L"RHODES_WM_TAKEPICTURE");
static UINT WM_SELECTPICTURE           = ::RegisterWindowMessage(L"RHODES_WM_SELECTPICTURE");
//static UINT WM_TAKESIGNATURE           = ::RegisterWindowMessage(L"RHODES_WM_TAKESIGNATURE");
static UINT WM_CONNECTIONSNETWORKCOUNT = ::RegisterWindowMessage(L"RHODES_WM_CONNECTIONSNETWORKCOUNT");
static UINT WM_CONNECTIONSNETWORKCELL  = ::RegisterWindowMessage(L"RHODES_WM_CONNECTIONSNETWORKCELL");
static UINT WM_ALERT_SHOW_POPUP        = ::RegisterWindowMessage(L"RHODES_WM_ALERT_SHOW_POPUP");
static UINT WM_ALERT_HIDE_POPUP        = ::RegisterWindowMessage(L"RHODES_WM_ALERT_HIDE_POPUP");
static UINT WM_DATETIME_PICKER         = ::RegisterWindowMessage(L"RHODES_WM_DATETIME_PICKER");
static UINT WM_BLUETOOTH_DISCOVER      = ::RegisterWindowMessage(L"RHODES_WM_BLUETOOTH_DISCOVER");
static UINT WM_BLUETOOTH_DISCOVERED    = ::RegisterWindowMessage(L"RHODES_WM_BLUETOOTH_DISCOVERED");
static UINT WM_BLUETOOTH_CALLBACK	   = ::RegisterWindowMessage(L"RHODES_WM_BLUETOOTH_CALLBACK");
static UINT WM_EXECUTE_COMMAND		   = ::RegisterWindowMessage(L"RHODES_WM_EXECUTE_COMMAND");
static UINT WM_EXECUTE_RUNNABLE		   = ::RegisterWindowMessage(L"RHODES_WM_EXECUTE_RUNNABLE");

typedef struct _TCookieData {
    char* url;
    char* cookie;
} TCookieData;

class CMainWindow :
#if defined(_WIN32_WCE)&& !defined( OS_PLATFORM_MOTCE )
	public CFrameWindowImpl<CMainWindow>, 
	public CFullScreenFrame<CMainWindow>
#else
    public CWindowImpl<CMainWindow, CWindow, CWinTraits<WS_BORDER | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_CLIPSIBLINGS> >
#endif
#ifndef APP_BUILD_CAPABILITY_WEBKIT_BROWSER
    ,public IDispEventImpl<ID_BROWSER, CMainWindow>
#endif
{
    DEFINE_LOGCLASS;
public:
    CMainWindow();
    ~CMainWindow();
    //
	void Navigate2(BSTR URL);
    void Navigate(BSTR URL);

	HWND getWebViewHWND();
    CNativeToolbar& getToolbar(){ return m_toolbar; }
    void performOnUiThread(rho::common::IRhoRunnable* pTask);

    LRESULT InitMainWindow();

    // Required to forward messages to the PIEWebBrowser control
    BOOL TranslateAccelerator(MSG* pMsg);

#if defined( OS_PLATFORM_MOTCE )
   	void SetFullScreen(bool bFull);
	bool m_bFullScreen;
#endif

	void openNativeView(	NativeViewFactory* nativeViewFactory, 
							NativeView* nativeView,
							String nativeViewType);
	void closeNativeView();
    rho::IBrowserEngine* getWebKitEngine(){return m_pBrowserEng; }

#if defined(OS_WINDOWS)
    DECLARE_WND_CLASS(TEXT("Rhodes.MainWindow"))
#else
	static ATL::CWndClassInfo& GetWndClassInfo() 
	{ 
        static rho::StringW strAppName = RHODESAPP().getAppNameW() + L".MainWindow";
		static ATL::CWndClassInfo wc = 
		{ 
			{ CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, StartWindowProc, 
			0, 0, NULL, NULL, NULL, (HBRUSH)(COLOR_WINDOW + 1), NULL, strAppName.c_str() }, 
			NULL, NULL, IDC_ARROW, TRUE, 0, _T("") 
		}; 
		return wc; 
	}
#endif
    
	BEGIN_MSG_MAP(CMainWindow)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
        MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        //MESSAGE_HANDLER(WM_SETTEXT, OnSetText)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
        COMMAND_ID_HANDLER(IDM_EXIT, OnExitCommand)
        COMMAND_ID_HANDLER(IDM_NAVIGATE_BACK, OnNavigateBackCommand)
        COMMAND_ID_HANDLER(IDM_NAVIGATE_FORWARD, OnNavigateForwardCommand)
        COMMAND_ID_HANDLER(IDM_SK1_EXIT, OnBackCommand)
        COMMAND_ID_HANDLER(IDM_REFRESH, OnRefreshCommand)
		COMMAND_ID_HANDLER(IDM_NAVIGATE, OnNavigateCommand)
        COMMAND_ID_HANDLER(IDM_EXECUTEJS, OnExecuteJSCommand)
        COMMAND_ID_HANDLER(IDM_STOPNAVIGATE, OnStopNavigate)
        COMMAND_ID_HANDLER(IDM_ZOOMPAGE, OnZoomPage)
        COMMAND_ID_HANDLER(IDM_ZOOMTEXT, OnZoomText)
        COMMAND_ID_HANDLER(IDM_LOG,OnLogCommand)
		COMMAND_ID_HANDLER(ID_FULLSCREEN, OnFullscreenCommand)
        COMMAND_ID_HANDLER(ID_SETCOOKIE, OnSetCookieCommand)
		COMMAND_RANGE_HANDLER(ID_CUSTOM_MENU_ITEM_FIRST, ID_CUSTOM_MENU_ITEM_LAST, OnCustomMenuItemCommand)
		COMMAND_RANGE_HANDLER(ID_CUSTOM_TOOLBAR_ITEM_FIRST, ID_CUSTOM_TOOLBAR_ITEM_LAST, OnCustomToolbarItemCommand)
#if defined(OS_WINDOWS) || defined( OS_PLATFORM_MOTCE )
		COMMAND_ID_HANDLER(IDM_POPUP_MENU, OnPopupMenuCommand)
#endif

#if defined(OS_WINDOWS)
		MESSAGE_HANDLER(WM_WINDOWPOSCHANGED, OnPosChanged)
#endif
		MESSAGE_HANDLER(WM_TAKEPICTURE, OnTakePicture)
		MESSAGE_HANDLER(WM_SELECTPICTURE, OnSelectPicture)
		//MESSAGE_HANDLER(WM_TAKESIGNATURE, OnTakeSignature)
		MESSAGE_HANDLER(WM_CONNECTIONSNETWORKCOUNT, OnConnectionsNetworkCount)
        MESSAGE_HANDLER(WM_CONNECTIONSNETWORKCELL, OnConnectionsNetworkCell)
        MESSAGE_HANDLER(WM_ALERT_SHOW_POPUP, OnAlertShowPopup)
		MESSAGE_HANDLER(WM_ALERT_HIDE_POPUP, OnAlertHidePopup);
		MESSAGE_HANDLER(WM_DATETIME_PICKER, OnDateTimePicker);
		MESSAGE_HANDLER(WM_BLUETOOTH_DISCOVER, OnBluetoothDiscover);
		MESSAGE_HANDLER(WM_BLUETOOTH_DISCOVERED, OnBluetoothDiscovered);
		MESSAGE_HANDLER(WM_BLUETOOTH_CALLBACK, OnBluetoothCallback);
		MESSAGE_HANDLER(WM_EXECUTE_COMMAND, OnExecuteCommand);
        MESSAGE_HANDLER(WM_EXECUTE_RUNNABLE, OnExecuteRunnable);
#ifdef APP_BUILD_CAPABILITY_WEBKIT_BROWSER
        MESSAGE_HANDLER(WM_BROWSER_ONDOCUMENTCOMPLETE, OnBrowserDocumentComplete);
        MESSAGE_HANDLER(WM_BROWSER_ONNAVIGATECOMPLETE, OnNavigateComplete);
        MESSAGE_HANDLER(WM_BROWSER_ONTITLECHANGE, OnTitleChange);
        MESSAGE_HANDLER(WM_BROWSER_ONBEFORENAVIGATE, OnBeforeNavigate);
        MESSAGE_HANDLER(WM_BROWSER_ONNAVIGATIONTIMEOUT, OnNavigateTimeout);
        MESSAGE_HANDLER(WM_BROWSER_ONNAVIGATIONERROR, OnNavigateError);
        MESSAGE_HANDLER(WM_BROWSER_ONSETSIPSTATE, OnSetSIPState);
        MESSAGE_HANDLER(WM_BROWSER_ONALERTPOPUP, OnAlertPopup);

        MESSAGE_RANGE_HANDLER(PB_NAVIGATETAB, PB_NEWGPSDATA, OnWebKitMessages)
#endif

    END_MSG_MAP()
	
private:
    // WM_xxx handlers
    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnActivate(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
    LRESULT OnSettingChange(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnSetText(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
    LRESULT OnNotify(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);

    // WM_COMMAND handlers
    LRESULT OnExitCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBackCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnNavigateBackCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnNavigateForwardCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnRefreshCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnNavigateCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnExecuteJSCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnStopNavigate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnZoomPage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnZoomText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnLogCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFullscreenCommand (WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnSetCookieCommand (WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCustomMenuItemCommand (WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnCustomToolbarItemCommand (WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

#if defined(OS_WINDOWS) || defined( OS_PLATFORM_MOTCE )
	LRESULT OnPopupMenuCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
#endif

#if defined(OS_WINDOWS)
	LRESULT OnPosChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
#endif

	LRESULT OnTakePicture(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnSelectPicture(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	//LRESULT OnTakeSignature(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnConnectionsNetworkCount(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
    LRESULT OnConnectionsNetworkCell(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
    LRESULT OnAlertShowPopup (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnAlertHidePopup (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnDateTimePicker (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnBluetoothDiscover (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnBluetoothDiscovered (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnBluetoothCallback (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnExecuteCommand (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
    LRESULT OnExecuteRunnable (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
#ifdef APP_BUILD_CAPABILITY_WEBKIT_BROWSER
    LRESULT OnBrowserDocumentComplete (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
    LRESULT OnNavigateComplete (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
    LRESULT OnTitleChange (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
    LRESULT OnWebKitMessages (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
    LRESULT OnBeforeNavigate (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);    
    LRESULT OnNavigateTimeout (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);    
    LRESULT OnNavigateError (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);    
    LRESULT OnSetSIPState (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);    
    LRESULT OnAlertPopup (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);    
#endif

public:

#ifndef APP_BUILD_CAPABILITY_WEBKIT_BROWSER
    BEGIN_SINK_MAP(CMainWindow)
        SINK_ENTRY(ID_BROWSER, DISPID_BEFORENAVIGATE2, &CMainWindow::OnBeforeNavigate2)
        SINK_ENTRY(ID_BROWSER, DISPID_TITLECHANGE, &CMainWindow::OnBrowserTitleChange)
        SINK_ENTRY(ID_BROWSER, DISPID_NAVIGATECOMPLETE2, &CMainWindow::OnNavigateComplete2)
        SINK_ENTRY(ID_BROWSER, DISPID_DOCUMENTCOMPLETE, &CMainWindow::OnDocumentComplete)
        SINK_ENTRY(ID_BROWSER, DISPID_COMMANDSTATECHANGE, &CMainWindow::OnCommandStateChange)
    END_SINK_MAP()
#endif

private:
    // event handlers
    void __stdcall OnBeforeNavigate2(IDispatch* pDisp, VARIANT * pvtURL, 
                                     VARIANT * /*pvtFlags*/, VARIANT * pvtTargetFrameName,
                                     VARIANT * /*pvtPostData*/, VARIANT * /*pvtHeaders*/, 
                                     VARIANT_BOOL * /*pvbCancel*/);
    void __stdcall OnBrowserTitleChange(BSTR bstrTitleText);
    void __stdcall OnNavigateComplete2(IDispatch* pDisp, VARIANT * pvtURL);
    void __stdcall OnDocumentComplete(IDispatch* pDisp, VARIANT * pvtURL);
    void __stdcall OnCommandStateChange(long lCommand, BOOL bEnable);

    // utility functions
    BOOL SetMenuItemEnabled      (UINT uMenuItemID, BOOL bEnable);
	BOOL SetToolbarButtonEnabled (UINT uTbbID, BOOL bEnable);
	
	void ShowLoadingPage();

	void createCustomMenu(void);

    void ProcessDocumentComplete(LPCTSTR url);
    void ProcessNavigateComplete(LPCTSTR url);
    void ProcessTitleChange(LPCTSTR title);

	// return cleared URL or empty string
	String processForNativeView(String url);
	void restoreWebView();
	void hideWebView();
	void showWebView();

private:
	NativeViewFactory* mNativeViewFactory;
	NativeView* mNativeView;
	String mNativeViewType;
	bool mIsOpenedByURL;


private:
    bool mIsBrowserViewHided;

    rho::IBrowserEngine* m_pBrowserEng;

#if defined(_WIN32_WCE)
    // main menu bar for application
    CWindow m_menuBar;
#elif defined (OS_WINDOWS)
	CMenuBar m_menuBar;
//	int m_menuBarHeight;
	CLogView m_logView;
#endif //_WIN32_WCE

// #if defined( OS_PLATFORM_MOTCE )
    int m_menuBarHeight;
    HWND				g_hWndCommandBar;	// command bar handle
// #endif

#if defined(_WIN32_WCE)
    // Used to manage SIP state. Also used to adjust window for SIP.
    SHACTIVATEINFO m_sai;
#endif

	bool m_bLoading;
    CNativeToolbar m_toolbar;

#if !defined(_WIN32_WCE)
private:
	static int m_screenWidth;
	static int m_screenHeight;
	
public:
	static int getScreenWidth() {return m_screenWidth;}
	static int getScreenHeight() {return m_screenHeight;}
#endif
private:
	int m_pageCounter;

    rho::Vector<rho::common::CAppMenuItem> m_arAppMenuItems;
	CAlertDialog *m_alertDialog;

    CSyncStatusDlg m_SyncStatusDlg;
};

#if !defined(_WIN32_WCE) || defined( OS_PLATFORM_MOTCE ) 
HBITMAP SHLoadImageFile (  LPCTSTR pszFileName );
#endif

#else

#include "simulator\MainWindowQt.h"

#endif
