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

#include "logging/RhoLogConf.h"
#include "resource.h"

#if defined(OS_WINDOWS)

static UINT WM_FIND_TEXT = ::RegisterWindowMessage(FINDMSGSTRING);

#define WS_EX_LAYOUT_RTL	0x00400000

class CResizableGrip  
{

	// Members
protected:
	HWND m_hParent;
	SIZE m_sizeGrip; // holds grip size
	HWND m_wndGrip;	// grip control
	CAtlArray<void*> m_wndControls; // controls allowed to dynamically move and resize
	
	RECT m_initialrect;
	BOOL m_binitialrect;

	// Constructor
public:
	CResizableGrip();
	virtual ~CResizableGrip();


	// Methods
public:
	BOOL InitGrip(HWND hParent);
	void UpdateGripPos();
	void ShowSizeGrip(BOOL bShow = TRUE);	// show or hide the size grip

	HWND GetSafeHwnd();

	void SetInitialRect(RECT *rect) { m_initialrect = *rect; m_binitialrect = TRUE;}
	BOOL IsRectInitialized() { return m_binitialrect; }
	RECT GetFinalRect() { return m_initialrect; }



	// Helpers
protected:
	static BOOL IsRTL(HWND hwnd) {
		DWORD dwExStyle = ::GetWindowLong(hwnd, GWL_EXSTYLE);
		return (dwExStyle & WS_EX_LAYOUT_RTL);
	}

	static LRESULT CALLBACK GripWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

};
#endif //OS_WINDOWS

// CLogView

class CLogView : 
	public CDialogImpl<CLogView>
#if defined(OS_WINDOWS)
   ,public rho::ILogSink
#endif
{
    HBRUSH m_hBrush;
	HWND m_hWndCommandBar;

    void loadLogText();
public:
    CLogView() : 
	  m_hBrush ( NULL ), m_hWndCommandBar(0)
#if defined(OS_WINDOWS)
	, m_pFindDialog(NULL), m_findText(L""), m_findParams(FR_DOWN)
#endif
	{}
	~CLogView(){}

#if defined(OS_WINDOWS)
// IlogSink
	static rho::common::CMutex m_ViewFlushLock;
	CAtlList<rho::String> m_buffer;
	void writeLogMessage( rho::String& strMsg );
    int getCurPos(){ return -1; }
    void clear(){}
	void OnPopupMenuCommand();
#endif

// Dialog
#if defined(OS_WINDOWS)
	enum { IDD = IDD_SIMULATOR_LOGVIEW };
#else
	enum { IDD = IDD_LOGVIEW };
#endif

BEGIN_MSG_MAP(CLogView)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
    MESSAGE_HANDLER(WM_CTLCOLORSTATIC,OnCtlColor)
    MESSAGE_HANDLER(WM_CTLCOLOREDIT,OnCtlColor)
#if defined(OS_WINDOWS)
	MESSAGE_HANDLER(WM_SIZING, OnSizing)
	MESSAGE_HANDLER(WM_CLOSE,OnClose)
	MESSAGE_HANDLER(WM_TIMER, OnTimer)
	MESSAGE_HANDLER(WM_WINDOWPOSCHANGED, OnPosChanged)
	MESSAGE_HANDLER(WM_FIND_TEXT,OnFindText)
	MESSAGE_HANDLER(WM_NOTIFY, OnNotify)

	COMMAND_ID_HANDLER(ID_MENU_COPY, OnCopy)
	COMMAND_ID_HANDLER(ID_MENU_SELECTALL, OnSelectAll)
	COMMAND_ID_HANDLER(ID_MENU_FIND, OnFind)
#endif
    COMMAND_ID_HANDLER(IDM_BACK, OnBack)
    COMMAND_ID_HANDLER(IDM_OPTIONS, OnOptions)
    COMMAND_ID_HANDLER(IDM_SENDLOG, OnSendLog)
    COMMAND_ID_HANDLER(IDM_REFRESH, OnRefresh)
    COMMAND_ID_HANDLER(IDM_CLEAR, OnClear)

#ifdef OS_WINCE
    COMMAND_ID_HANDLER(IDOK, OnOK)
    COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	MESSAGE_HANDLER(WM_CLOSE,OnClose)
#endif

END_MSG_MAP()
//	CHAIN_MSG_MAP(CAxDialogImpl<CLogView>)

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroyDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnBack(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnOptions(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSendLog(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRefresh(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClear(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

#ifdef OS_WINCE
    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
   	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
#endif

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCtlColor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
#if defined(OS_WINDOWS)
	LRESULT OnSizing(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPosChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnFindText(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnNotify(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);

	LRESULT OnCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSelectAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnFind(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
#endif
    virtual void OnFinalMessage(HWND /*hWnd*/);

#if 0 //defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		DRA::RelayoutDialog(
			AtlGetThisModuleHandle(),
			m_hWnd, 
			DRA::GetDisplayMode() != DRA::Portrait ? MAKEINTRESOURCE(IDD_LOGVIEW_WIDE) : MAKEINTRESOURCE(IDD_LOGVIEW));
		return 0;
	}
#endif
#if defined(OS_WINDOWS)
protected:
  CResizableGrip m_grip;
  CFindReplaceDialog *m_pFindDialog;
  rho::StringW m_findText;
  WPARAM m_findParams;
#endif
};

#if defined(OS_WINDOWS)

int  getIniInt(LPCTSTR lpKeyName, int nDefault);
void setIniInt(LPCTSTR lpKeyName, int nValue);

#endif

