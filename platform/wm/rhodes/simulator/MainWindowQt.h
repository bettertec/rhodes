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

#include <string>
#include "logging/RhoLog.h"
#include "common/RhoConf.h"
#include "common/RhodesApp.h"
#include "common/rhoparams.h"
#include "Alert.h"
#include "RhoNativeViewManagerWM.h"
#include "SyncStatusDlg.h"
#include "NativeToolbarQt.h"
#include "NativeTabbarQt.h"
#if defined(OS_WINDOWS_DESKTOP)
#include "menubar.h"
#endif
#include "LogView.h"
#include "qt/rhodes/MainWindowCallback.h"

static UINT WM_TAKEPICTURE             = ::RegisterWindowMessage(L"RHODES_WM_TAKEPICTURE");
static UINT WM_SELECTPICTURE           = ::RegisterWindowMessage(L"RHODES_WM_SELECTPICTURE");
//static UINT WM_TAKESIGNATURE           = ::RegisterWindowMessage(L"RHODES_WM_TAKESIGNATURE");
static UINT WM_ALERT_SHOW_POPUP        = ::RegisterWindowMessage(L"RHODES_WM_ALERT_SHOW_POPUP");
static UINT WM_ALERT_HIDE_POPUP        = ::RegisterWindowMessage(L"RHODES_WM_ALERT_HIDE_POPUP");
static UINT WM_DATETIME_PICKER         = ::RegisterWindowMessage(L"RHODES_WM_DATETIME_PICKER");
static UINT WM_EXECUTE_COMMAND         = ::RegisterWindowMessage(L"RHODES_WM_EXECUTE_COMMAND");
static UINT WM_EXECUTE_RUNNABLE        = ::RegisterWindowMessage(L"RHODES_WM_EXECUTE_RUNNABLE");

#define ID_CUSTOM_MENU_ITEM_FIRST (WM_APP+3)
#define ID_CUSTOM_MENU_ITEM_LAST  (ID_CUSTOM_MENU_ITEM_FIRST + (APP_MENU_ITEMS_MAX) - 1)
#define ID_CUSTOM_TOOLBAR_ITEM_FIRST (ID_CUSTOM_MENU_ITEM_LAST+1)
#define ID_CUSTOM_TOOLBAR_ITEM_LAST  (ID_CUSTOM_TOOLBAR_ITEM_FIRST + 20 - 1)

typedef struct _TCookieData {
    char* url;
    char* cookie;
} TCookieData;

class CMainWindow :
    public CWindowImpl<CMainWindow, CWindow, CWinTraits<WS_BORDER | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_CLIPSIBLINGS> >,
    public IMainWindowCallback
{
    DEFINE_LOGCLASS;
public:
    CMainWindow();
    ~CMainWindow();
    // IMainWindowCallback
    virtual void updateSizeProperties(int width, int height);
    virtual void onActivate(int active);
    virtual void logEvent(const ::std::string& message);
    virtual void createCustomMenu(void);
    virtual void onCustomMenuItemCommand(int nItemPos);
	virtual void onWindowClose(void);
    virtual void onWebViewUrlChanged(const ::std::string& url);
    // public methods:
    void Navigate2(BSTR URL, int index);
    HWND Initialize(const wchar_t* title);
    void MessageLoop(void);
	void DestroyUi(void);
    void performOnUiThread(rho::common::IRhoRunnable* pTask);
    CNativeToolbar& getToolbar(){ return m_toolbar; }
    CNativeTabbar& getTabbar(){ return m_tabbar; }
    HWND getWebViewHWND(int index);
	// for 'main_window_closed' System property
	static bool mainWindowClosed;

    static HINSTANCE rhoApplicationHINSTANCE;

    // proxy methods:
    void* init(IMainWindowCallback* callback, const wchar_t* title);
    void setCallback(IMainWindowCallback* callback);
    void messageLoop(void);
    void navigate(const wchar_t* url, int index);
    void GoBack(void);
    void GoForward(void);
    void Refresh(int index);
	// toolbar/tabbar
    bool isStarted();
    // toolbar proxy
    int getToolbarHeight();
    void createToolbar(rho_param *p);
    void removeToolbar();
    void removeAllButtons();
    // menu proxy
    void menuClear();
    void menuAddSeparator();
    void menuAddAction(const char* label, int item);
	// tabbar
    int getTabbarHeight();
    void removeAllTabs(bool restore);
    void createTabbar(int bar_type, rho_param *p);
    void removeTabbar();
    void tabbarSwitch(int index);
    void tabbarBadge(int index, char* badge);
    int tabbarGetCurrent();
    bool isExistJavascript(const wchar_t* szJSFunction, int index){return true;}


    BEGIN_MSG_MAP(CMainWindow)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        //MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
        COMMAND_ID_HANDLER(IDM_EXIT, OnExitCommand)
        COMMAND_ID_HANDLER(IDM_NAVIGATE_BACK, OnNavigateBackCommand)
        COMMAND_ID_HANDLER(IDM_NAVIGATE_FORWARD, OnNavigateForwardCommand)
        COMMAND_ID_HANDLER(IDM_SK1_EXIT, OnBackCommand)
        COMMAND_ID_HANDLER(IDM_LOG,OnLogCommand)
        COMMAND_ID_HANDLER(IDM_REFRESH, OnRefreshCommand)
        COMMAND_ID_HANDLER(IDM_NAVIGATE, OnNavigateCommand)
        COMMAND_ID_HANDLER(ID_SETCOOKIE, OnSetCookieCommand)
        COMMAND_ID_HANDLER(IDM_EXECUTEJS, OnExecuteJS)
        MESSAGE_HANDLER(WM_TAKEPICTURE, OnTakePicture)
        MESSAGE_HANDLER(WM_SELECTPICTURE, OnSelectPicture)
        MESSAGE_HANDLER(WM_ALERT_SHOW_POPUP, OnAlertShowPopup)
        MESSAGE_HANDLER(WM_ALERT_HIDE_POPUP, OnAlertHidePopup);
        MESSAGE_HANDLER(WM_DATETIME_PICKER, OnDateTimePicker);
        MESSAGE_HANDLER(WM_EXECUTE_COMMAND, OnExecuteCommand);
        MESSAGE_HANDLER(WM_EXECUTE_RUNNABLE, OnExecuteRunnable);
    END_MSG_MAP()
    
private:
    // WM_xxx handlers
    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
    //LRESULT OnActivate(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);

    // WM_COMMAND handlers
    LRESULT OnExitCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnNavigateBackCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnNavigateForwardCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBackCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnLogCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnRefreshCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/);
    LRESULT OnNavigateCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/);
    LRESULT OnSetCookieCommand (WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnExecuteJS(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/);

    LRESULT OnTakePicture(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
    LRESULT OnSelectPicture(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
    LRESULT OnAlertShowPopup (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
    LRESULT OnAlertHidePopup (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
    LRESULT OnDateTimePicker (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
    LRESULT OnExecuteCommand (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
    LRESULT OnExecuteRunnable (UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);

private:
    CLogView m_logView;
    CNativeToolbar m_toolbar;
	CNativeTabbar m_tabbar;
    bool m_started;
    void* qtMainWindow;
    void* qtApplication;

private:
    static int m_screenWidth;
    static int m_screenHeight;
    
public:
    static int getScreenWidth() {return m_screenWidth;}
    static int getScreenHeight() {return m_screenHeight;}

private:
    rho::Vector<rho::common::CAppMenuItem> m_arAppMenuItems;
    CAlertDialog *m_alertDialog;
    CSyncStatusDlg *m_SyncStatusDlg;
};

#if !defined(_WIN32_WCE)
HBITMAP SHLoadImageFile (  LPCTSTR pszFileName );
#endif