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

#include "stdafx.h"
#include "logging/RhoLog.h"

#include "MainWindow.h"
#include "Utils.h"
#include "DateTimePicker.h"

extern "C" int rho_sys_get_screen_height();
extern "C" int rho_sys_get_screen_width();

#define DLG_ITEM_SET_FONT_BOLD(ITEM_ID)							\
{																\
	HFONT hFont = GetDlgItem((ITEM_ID)).GetFont();				\
	LOGFONT fontAttributes = { 0 };								\
    ::GetObject(hFont, sizeof(fontAttributes), &fontAttributes);\
    fontAttributes.lfWeight = FW_BOLD;							\
	hFont = CreateFontIndirect(&fontAttributes);				\
	GetDlgItem((ITEM_ID)).SetFont(hFont);						\
}


/*
 * TODO: title and initial time
 */

extern "C" HWND getMainWnd();

CDateTimePickerDialog::CDateTimePickerDialog (const CDateTimeMessage *msg) : m_hWndCommandBar(0)
{
	m_returnTime  = 0;
	m_format       = msg->m_format;
	m_title        = msg->m_title;
	m_initialTime  = msg->m_initialTime;
	m_min_time	   = msg->m_min_time;
	m_max_time     = msg->m_max_time;
}

CDateTimePickerDialog::~CDateTimePickerDialog ()
{
}

LRESULT CDateTimePickerDialog::OnDestroyDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    if ( m_hWndCommandBar )
        ::DestroyWindow(m_hWndCommandBar);

    m_hWndCommandBar = 0;

	return FALSE;
}

LRESULT CDateTimePickerDialog::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if ((m_title != NULL) && (strlen(m_title) > 0)) {
		String title = m_title;
		StringW title_w = convertToStringW(title);
		//SetWindowText(title_w.c_str());
		SetDlgItemText(ID_DATETIME_CAPTION, title_w.c_str());
		//GetDlgItem(ID_DATETIME_CAPTION).SetWindowText(title_w.c_str());
	}
	else {
		SetWindowText(_T("Date"));
	}
#if defined(_WIN32_WCE)  && !defined(OS_PLATFORM_MOTCE)

    SHINITDLGINFO shidi = { SHIDIM_FLAGS, m_hWnd, SHIDIF_SIZEDLGFULLSCREEN };
    RHO_ASSERT(SHInitDialog(&shidi));


    SHMENUBARINFO mbi = { sizeof(mbi), 0 };
    mbi.hwndParent = m_hWnd;
    mbi.nToolBarId = IDR_GETURL_MENUBAR;
    mbi.hInstRes = _AtlBaseModule.GetResourceInstance();
    SHCreateMenuBar(&mbi);

	SYSTEMTIME times[2]; // min and max times
	unsigned int flags = 0;


	if (m_min_time != 0) {
		UnixTimeToSystemTime(m_min_time, &(times[0]));
		flags = flags | GDTR_MIN;
	}
	if (m_max_time != 0) {
		UnixTimeToSystemTime(m_max_time, &(times[1]));
		flags = flags | GDTR_MAX;
	}
	DateTime_SetRange( GetDlgItem(IDC_DATE_CTRL), flags, times);

	SYSTEMTIME start_time;
	if (m_initialTime != 0) {
		UnixTimeToSystemTime(m_initialTime, &start_time);
		DateTime_SetSystemtime( GetDlgItem(IDC_DATE_CTRL), GDT_VALID, &start_time);
		DateTime_SetSystemtime( GetDlgItem(IDC_TIME_CTRL), GDT_VALID, &start_time);
	}

	GotoDlgCtrl(GetDlgItem(IDC_DATE_CTRL));

#elif defined( OS_PLATFORM_MOTCE )

	//CreateButtons();
	//GotoDlgCtrl(m_btnOk);
	SetWindowLong(GWL_STYLE,(long)WS_BORDER);
	m_hWndCommandBar = CommandBar_Create(_AtlBaseModule.GetResourceInstance(), m_hWnd, 1);
    CommandBar_AddAdornments(m_hWndCommandBar, CMDBAR_OK, 0 );

    CommandBar_Show(m_hWndCommandBar, TRUE);

#endif

	DLG_ITEM_SET_FONT_BOLD (IDC_DATE_STATIC);
	DLG_ITEM_SET_FONT_BOLD (IDC_TIME_STATIC);
	DLG_ITEM_SET_FONT_BOLD (ID_DATETIME_CAPTION);
	

	if (m_format == CDateTimeMessage::FORMAT_DATE) {
		GetDlgItem(IDC_TIME_CTRL).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TIME_STATIC).ShowWindow(SW_HIDE);
	}

    return FALSE;
}

LRESULT CDateTimePickerDialog::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND hwnd, BOOL& /*bHandled*/)
{
	SYSTEMTIME sysTime;

	if (m_format == CDateTimeMessage::FORMAT_TIME)
	{
		DateTime_GetSystemtime (GetDlgItem(IDC_TIME_CTRL), &sysTime);
	} 
	else if (m_format == CDateTimeMessage::FORMAT_DATE)
	{
		DateTime_GetSystemtime (GetDlgItem(IDC_DATE_CTRL), &sysTime);
	} 
	else if (m_format == CDateTimeMessage::FORMAT_DATE_TIME)
	{
		SYSTEMTIME time, date;

		DateTime_GetSystemtime (GetDlgItem(IDC_TIME_CTRL), &time);
		DateTime_GetSystemtime (GetDlgItem(IDC_DATE_CTRL), &date);

		sysTime.wYear		= date.wYear; 
		sysTime.wMonth		= date.wMonth; 
		sysTime.wDayOfWeek	= date.wDayOfWeek; 
		sysTime.wDay		= date.wDay; 
		sysTime.wHour		= time.wHour; 
		sysTime.wMinute		= time.wMinute;
		sysTime.wSecond		= time.wSecond; 
	} 
	else
	{
		LOG(ERROR) + "invalid format";
		m_returnTime = 0;
	}

	m_returnTime = SystemTimeToUnixTime (&sysTime);

    EndDialog(wID);
    return 0;
}

time_t CDateTimePickerDialog::GetTime()
{
	return m_returnTime;
}

LRESULT CDateTimePickerDialog::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    EndDialog(wID);
    return 0;
}


CTimePickerDialog::CTimePickerDialog (const CDateTimeMessage *msg) : m_hWndCommandBar(0)
{
	m_returnTime  = 0;
	m_format       = msg->m_format;
	m_title        = msg->m_title;
	m_initialTime  = msg->m_initialTime;
}

CTimePickerDialog::~CTimePickerDialog ()
{
}

LRESULT CTimePickerDialog::OnDestroyDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    if ( m_hWndCommandBar )
        ::DestroyWindow(m_hWndCommandBar);

    m_hWndCommandBar = 0;

	return FALSE;
}

LRESULT CTimePickerDialog::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if ((m_title != NULL) && (strlen(m_title) > 0)) {
		String title = m_title;
		StringW title_w = convertToStringW(title);
		//SetWindowText(title_w.c_str());
		//GetDlgItem(IDC_TIME_CTRL).SetWindowText(title_w.c_str());
		SetDlgItemText(ID_TIME_CAPTION, title_w.c_str());
	}
	else {
		SetWindowText(_T("Date"));
	}

#if defined(_WIN32_WCE)  && !defined(OS_PLATFORM_MOTCE)

    SHINITDLGINFO shidi = { SHIDIM_FLAGS, m_hWnd, SHIDIF_SIZEDLGFULLSCREEN };
    RHO_ASSERT(SHInitDialog(&shidi));

    SHMENUBARINFO mbi = { sizeof(mbi), 0 };
    mbi.hwndParent = m_hWnd;
    mbi.nToolBarId = IDR_GETURL_MENUBAR;
    mbi.hInstRes = _AtlBaseModule.GetResourceInstance();
    SHCreateMenuBar(&mbi);
	GotoDlgCtrl(GetDlgItem(IDC_TIME_CTRL));

	SYSTEMTIME start_time;
	if (m_initialTime != 0) {
		UnixTimeToSystemTime(m_initialTime, &start_time);
		DateTime_SetSystemtime( GetDlgItem(IDC_TIME_CTRL), GDT_VALID, &start_time);
	}

#elif defined( OS_PLATFORM_MOTCE )

	//CreateButtons();
	//GotoDlgCtrl(m_btnOk);
	SetWindowLong(GWL_STYLE,(long)WS_BORDER);
	m_hWndCommandBar = CommandBar_Create(_AtlBaseModule.GetResourceInstance(), m_hWnd, 1);
    CommandBar_AddAdornments(m_hWndCommandBar, CMDBAR_OK, 0 );

    CommandBar_Show(m_hWndCommandBar, TRUE);

#endif

	DLG_ITEM_SET_FONT_BOLD (IDC_TIME_STATIC);
	DLG_ITEM_SET_FONT_BOLD (ID_TIME_CAPTION);

    return FALSE;
}

LRESULT CTimePickerDialog::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND hwnd, BOOL& /*bHandled*/)
{
	SYSTEMTIME sysTime;
	DateTime_GetSystemtime (GetDlgItem(IDC_TIME_CTRL), &sysTime);
	m_returnTime = SystemTimeToUnixTime (&sysTime);

    EndDialog(wID);
    return 0;
}

time_t CTimePickerDialog::GetTime()
{
	return m_returnTime;
}

LRESULT CTimePickerDialog::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    EndDialog(wID);
    return 0;
}


static char ourTitle[1024];

void  choose_datetime_with_range(char* callback, char* title, 
					  long initial_time, int format, char* data,
					  long min_time, long max_time)
{
	LOG(INFO) + __FUNCTION__ + "callback = " + callback + " title = " + title;

	if (title != NULL) {
		strcpy(ourTitle, title);
	}
	else {
		strcpy(ourTitle, "");
	}

	HWND main_wnd = getMainWnd();
	::PostMessage(main_wnd, WM_DATETIME_PICKER, 0, 
					(LPARAM)new CDateTimeMessage(callback, ourTitle, initial_time, format, data, min_time, max_time));
}


void  choose_datetime(char* callback, char* title, 
					  long initial_time, int format, char* data)
{
	choose_datetime_with_range( callback, title, initial_time, format, data, 0, 0);
}

void set_change_value_callback_datetime(char* callback) {
}
