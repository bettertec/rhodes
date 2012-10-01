#pragma once
#include "ibrowserengine.h"

#include "logging/RhoLog.h"

#if defined (_WIN32_WCE) && !defined( OS_PLATFORM_MOTCE )
#include <pvdispid.h>
#include <piedocvw.h>
#endif

class CIEBrowserEngine :  public rho::IBrowserEngine
{
    // Represents the PIEWebBrowser control contained in the main application.
    // window. m_browser is used to manage the control and its associated 
    // "AtlAxWin" window. (AtlAxWin is a window class that ATL uses to support 
    // containment of controls in windows.)
    CAxWindow m_browser;

    // cached copy of hosted control's IWebBrowser2 interface pointer
    CComPtr<IWebBrowser2> m_spIWebBrowser2;
    bool  m_bLoadingComplete;
public:
    CIEBrowserEngine(HWND hParentWnd, HINSTANCE hInstance);
    virtual ~CIEBrowserEngine(void);

    virtual BOOL Navigate(LPCTSTR szURL);
    virtual HWND GetHTMLWND(){ return m_browser.m_hWnd; }
    virtual BOOL ResizeOnTab(int iInstID,RECT rcNewSize);
    virtual BOOL BackOnTab(int iInstID,int iPagesBack = 1);
    virtual BOOL ForwardOnTab(int iInstID);
    virtual BOOL ReloadOnTab(bool bFromCache, UINT iTab);
    virtual BOOL StopOnTab(UINT iTab);
    virtual BOOL NavigateToHtml(LPCTSTR szHtml);
    virtual LRESULT OnWebKitMessages(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual void RunMessageLoop(CMainWindow& mainWnd);
    virtual void SetCookie(char* url, char* cookie);
    virtual bool isExistJavascript(const wchar_t* szJSFunction, int index);
    virtual void executeJavascript(const wchar_t* szJSFunction, int index);
    virtual BOOL ZoomPageOnTab(float fZoom, UINT iTab);
    virtual BOOL ZoomTextOnTab(int nZoom, UINT iTab);
    virtual int GetTextZoomOnTab(UINT iTab);
    virtual BOOL GetTitleOnTab(LPTSTR szURL, UINT iMaxLen, UINT iTab);
    virtual void OnDocumentComplete(LPCTSTR url);
    virtual void setBrowserGesturing(bool bEnableGesturing);
    virtual void NotifyEngineOfSipPosition();
    virtual DWORD GetProcessID();
};
