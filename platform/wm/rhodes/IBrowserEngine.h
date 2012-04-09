#pragma once

class CMainWindow;
namespace rho
{

struct IBrowserEngine
{
public:
    virtual ~IBrowserEngine(void){};

    virtual BOOL Navigate(LPCTSTR szURL) = 0;
    virtual HWND GetHTMLWND() = 0;
    virtual BOOL ResizeOnTab(int iInstID,RECT rcNewSize) = 0;
    virtual BOOL BackOnTab(int iInstID,int iPagesBack = 1) = 0;
    virtual BOOL ForwardOnTab(int iInstID) = 0;
    virtual BOOL ReloadOnTab(bool bFromCache, UINT iTab) = 0;
    virtual BOOL StopOnTab(UINT iTab) = 0;
    virtual BOOL NavigateToHtml(LPCTSTR szHtml) = 0;
    virtual LRESULT OnWebKitMessages(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) = 0;
    virtual void RunMessageLoop(CMainWindow& mainWnd) = 0;
    virtual void SetCookie(char* url, char* cookie) = 0;
    virtual bool isExistJavascript(const wchar_t* szJSFunction, int index) = 0;
    virtual void executeJavascript(const wchar_t* szJSFunction, int index) = 0;
    virtual BOOL ZoomPageOnTab(float fZoom, UINT iTab) = 0;
    virtual BOOL ZoomTextOnTab(int nZoom, UINT iTab) = 0;
    virtual int GetTextZoomOnTab(UINT iTab) = 0;
    virtual BOOL GetTitleOnTab(LPTSTR szURL, UINT iMaxLen, UINT iTab) = 0;
};

}