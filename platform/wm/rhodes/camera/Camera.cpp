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

#include <atltime.h>
#include "ext/rho/rhoruby.h"
#include "../MainWindow.h"
#include "Camera.h"
#include "common/RhodesApp.h"

#ifdef _MSC_VER
// warning C4800: 'int' : forcing to bool 'true' or 'false' (performance warning)
#pragma warning ( disable : 4800 )
#endif

extern "C" HWND getMainWnd();

//#if defined(_WIN32_WCE)

static bool copy_file(LPTSTR from, LPTSTR to);
static LPTSTR get_file_name(LPTSTR from, LPTSTR to);
static LPTSTR generate_filename(LPTSTR filename, LPCTSTR szExt );
static void create_folder(LPTSTR Path);

IMPLEMENT_LOGCLASS(Camera,"Camera");

Camera::Camera(void) {
}

Camera::~Camera(void) {
}

HRESULT Camera::takePicture(HWND hwndOwner,LPTSTR pszFilename) 
{
    HRESULT         hResult = S_OK;
#if defined(_WIN32_WCE) && !defined( OS_PLATFORM_MOTCE )
    SHCAMERACAPTURE shcc;

    StringW root;
    convertToStringW(rho_rhodesapp_getblobsdirpath(), root);
    wsprintf( pszFilename, L"%s", root.c_str() );

	create_folder(pszFilename);

    //LPCTSTR szExt = wcsrchr(pszFilename, '.');
    TCHAR filename[256];
	generate_filename(filename,L".jpg");

    // Set the SHCAMERACAPTURE structure.
    ZeroMemory(&shcc, sizeof(shcc));
    shcc.cbSize             = sizeof(shcc);
    shcc.hwndOwner          = hwndOwner;
    shcc.pszInitialDir      = pszFilename;
    shcc.pszDefaultFileName = filename;
    shcc.pszTitle           = TEXT("Camera");
    shcc.VideoTypes			= CAMERACAPTURE_VIDEOTYPE_MESSAGING;
    shcc.nResolutionWidth   = 176;
    shcc.nResolutionHeight  = 144;
    shcc.nVideoTimeLimit    = 15;
    shcc.Mode               = CAMERACAPTURE_MODE_STILL;

    // Display the Camera Capture dialog.
    hResult = SHCameraCapture(&shcc);

    // The next statements will execute only after the user takes
    // a picture or video, or closes the Camera Capture dialog.
    if (S_OK == hResult) {
        LPTSTR fname = get_file_name(shcc.szFile,pszFilename);
		if (fname) {
			StringCchCopy(pszFilename, MAX_PATH, fname);
			free(fname);
		} else {
            LOG(ERROR) + "takePicture error get file: " + shcc.szFile;

			hResult = E_INVALIDARG;
		}
    }else
    {
        LOG(ERROR) + "takePicture failed with code: " + LOGFMT("0x%X") + hResult;
    }
#endif //_WIN32_WCE

    return hResult;
}

HRESULT Camera::selectPicture(HWND hwndOwner,LPTSTR pszFilename) 
{
	RHO_ASSERT(pszFilename);
#if defined( _WIN32_WCE ) && !defined( OS_PLATFORM_MOTCE )
	OPENFILENAMEEX ofn = {0};
#else
    OPENFILENAME ofn = {0};
#endif
    pszFilename[0] = 0;

	ofn.lStructSize     = sizeof(ofn);
	ofn.lpstrFilter     = NULL;
	ofn.lpstrFile       = pszFilename;
	ofn.nMaxFile        = MAX_PATH;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle      = _T("Select an image");
#if defined( _WIN32_WCE ) && !defined( OS_PLATFORM_MOTCE )
	ofn.ExFlags         = OFN_EXFLAG_THUMBNAILVIEW|OFN_EXFLAG_NOFILECREATE|OFN_EXFLAG_LOCKDIRECTORY;
    if (GetOpenFileNameEx(&ofn))
#else
    if (GetOpenFileName(&ofn))
#endif

    {
		HRESULT hResult = S_OK;

        /*
		TCHAR rhoroot[MAX_PATH];
		wchar_t* root  = wce_mbtowc(rho_rhodesapp_getblobsdirpath());
		wsprintf(rhoroot,L"%s",root);
		free(root);

		create_folder(rhoroot);*/

        StringW strBlobRoot = convertToStringW( RHODESAPP().getBlobsDirPath() );

        LPCTSTR szExt = wcsrchr(pszFilename, '.');
		TCHAR filename[256];
		generate_filename(filename, szExt);
		
		int len = strBlobRoot.length() + wcslen(L"\\") + wcslen(filename);
		wchar_t* full_name = (wchar_t*) malloc((len+2)*sizeof(wchar_t));
		wsprintf(full_name,L"%s\\%s",strBlobRoot.c_str(),filename);

		if (copy_file(pszFilename,full_name)) 
        {
			wcscpy( pszFilename, filename );
		} else {
			hResult = E_INVALIDARG;
		}

		free(full_name);

		return hResult;
	} else if (GetLastError()==ERROR_SUCCESS) {
		return S_FALSE; //user cancel op
	}
	return E_INVALIDARG;
}

bool copy_file(LPTSTR from, LPTSTR to) 
{
	RHO_ASSERT(from);
	RHO_ASSERT(to);
/*
	SHFILEOPSTRUCT SHFileOp;
	ATL::CString source(from);
	ATL::CString destination(to);

	// add required string terminators
	source+=_T("\0\0");
	destination+=_T("\0\0");

	// set up File Operation structure
	ZeroMemory(&SHFileOp, sizeof(SHFILEOPSTRUCT));
	SHFileOp.hwnd = NULL;
	SHFileOp.wFunc = FO_COPY;
	SHFileOp.pFrom = source;
	SHFileOp.pTo = destination;
	SHFileOp.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;
*/
	//if(SHFileOperation(&SHFileOp) != 0) {
    if ( !CopyFile(from, to, TRUE) )
    {
        DWORD dwErr = GetLastError();
		return false;
	}
	return true;
}

LPTSTR get_file_name(LPTSTR from, LPTSTR to) {
	int len = wcslen(to);
	if (wcsncmp(to,from,len)==0) {
		LPTSTR fname = from+len;
		if ( (wcsncmp(L"\\",fname,1)==0) || 
			 (wcsncmp(L"/",fname,1)==0) ) {
			fname++;
		}
		len = wcslen(fname);
		wchar_t* name = (wchar_t*) malloc(len*sizeof(wchar_t)+1);
		wcscpy(name,fname);
		return name;
	} else {
		return NULL;
	}
}

LPTSTR generate_filename(LPTSTR filename, LPCTSTR szExt) {
	RHO_ASSERT(filename);

	CTime time(CTime::GetCurrentTime());
	tm tl, tg;
	time.GetLocalTm(&tl);
	time.GetGmtTm(&tg);
	int tz = tl.tm_hour-tg.tm_hour; //TBD: fix tz

    wsprintf(filename, L"Image_%02i-%02i-%0004i_%02i.%02i.%02i_%c%03i%s", 
		tg.tm_mon, tg.tm_mday, 1900+tg.tm_year, tg.tm_hour, tg.tm_min, tg.tm_sec,  
        tz>0?'_':'-',abs(tz),(szExt?szExt:L""));

	return filename;
}

void create_folder(LPTSTR Path)
{
	TCHAR DirName[256];
	LPTSTR p = Path;
	LPTSTR q = DirName; 
	while(*p)
	{
		if (('\\' == *p) || ('/' == *p))
		{
			if (':' != *(p-1))
			{
				CreateDirectory(DirName, NULL);
			}
		}
		*q++ = *p++;
		*q = '\0';
	}
	CreateDirectory(DirName, NULL);
}

//#endif //_WIN32_WCE

void choose_picture(char* callback_url, rho_param *options_hash) {
//#if defined(_WIN32_WCE)
	HWND main_wnd = getMainWnd();
	::PostMessage(main_wnd,WM_SELECTPICTURE,0,(LPARAM)strdup(callback_url));
//#endif
}

void take_picture(char* callback_url, rho_param * options_hash) {
	HWND main_wnd = getMainWnd();
	::PostMessage(main_wnd,WM_TAKEPICTURE,0,(LPARAM)strdup(callback_url));
}

VALUE get_camera_info(const char* camera_type) {
     return rho_ruby_get_NIL();
}
