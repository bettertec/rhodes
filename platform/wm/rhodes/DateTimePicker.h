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

#ifndef __DATETIMEPICKER_H__
#define __DATETIMEPICKER_H__

#include "OkCancelModalDialog.h"
#include "resource.h"

#ifdef RHODES_EMULATOR
#include "qt/rhodes/impl/DateTimePickerImpl.h"
#else
//TODO: delegates
class CDateTimeMessage 
{
public:
	static enum 
	{
		FORMAT_DATE_TIME = 0,
		FORMAT_DATE,
		FORMAT_TIME
	};

	CDateTimeMessage (const char* callback, const char* title, long initial_time, int format, const char* data, long min_time, long max_time)
	{
		m_callback    = strdup (callback);
		m_title       = strdup (title);
		m_initialTime = initial_time;
		m_format      = format;
		m_data        = strdup (data);
		m_min_time = min_time;
		m_max_time = max_time;
	}

	~CDateTimeMessage ()
	{
		free (m_callback);
		free (m_title);
		free (m_data);
	}

	char *m_callback;
	char *m_title;
	char *m_data;
	long  m_initialTime;
	int   m_format;
	long  m_min_time;
	long  m_max_time;
};
#endif

class CDateTimePickerDialog : public 
#if defined(OS_WINDOWS) || defined(OS_PLATFORM_MOTCE)
	COkCancelModalDialog<CDateTimePickerDialog>
#else
	CDialogImpl <CDateTimePickerDialog>
#endif
{
public:
	CDateTimePickerDialog  (const CDateTimeMessage *msg);
	~CDateTimePickerDialog ();

	time_t GetTime();

	enum { IDD = IDD_DATETIME_PICKER };
	
	BEGIN_MSG_MAP(CDateTimePickerDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

public:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDestroyDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
private:
	int         m_format;
	time_t      m_returnTime;
	time_t		m_initialTime;
	const char *m_title;
	time_t      m_min_time;
	time_t      m_max_time;
	HWND m_hWndCommandBar;
};


class CTimePickerDialog : public
#if defined(OS_WINDOWS) || defined(OS_PLATFORM_MOTCE)
	COkCancelModalDialog<CTimePickerDialog>
#else
	CDialogImpl <CTimePickerDialog>
#endif
{
public:
	CTimePickerDialog  (const CDateTimeMessage *msg);
	~CTimePickerDialog ();

	time_t GetTime();

	enum { IDD = IDD_TIME_PICKER };
	
	BEGIN_MSG_MAP(CTimePickerDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

private:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDestroyDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

private:
	int         m_format;
	time_t      m_returnTime;
	time_t		m_initialTime;
	const char *m_title;
	HWND m_hWndCommandBar;
};


extern "C" void  choose_datetime(char* callback, char* title, 
								 long initial_time, int format, char* data);
extern "C" void  choose_datetime_with_range(char* callback, char* title, 
								 long initial_time, int format, char* data, long min_time, long max_time);

extern "C" void set_change_value_callback_datetime(char* callback);

#endif //__DATETIMEPICKER_H__
