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

#include "AppManager.h"

#ifdef ENABLE_DYNAMIC_RHOBUNDLE
#include "net/INetRequest.h"

#include "common/app_build_capabilities.h"
#include "common/RhodesApp.h"

#include "unzip/unzip.h"
#include "ext/rho/rhoruby.h"
#include "common/AutoPointer.h"
#include "common/StringConverter.h"

using namespace rho::common;

CAppManager::CAppManager(void)
{
}

CAppManager::~CAppManager(void)
{
}

bool CAppManager::RemoveFolder(String pathname)
{
	if (pathname.length() > 0) 
    {
		StringW  swPath = convertToStringW(pathname);
		TCHAR name[MAX_PATH+2];
        wsprintf(name, L"%s%c", swPath.c_str(), '\0');

		SHFILEOPSTRUCT fop;

		fop.hwnd = NULL;
		fop.wFunc = FO_DELETE;		
		fop.pFrom = name;
		fop.pTo = NULL;
		fop.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR;
		int result = SHFileOperation(&fop);

		return result == 0;
	}
	return false;
}

bool CAppManager::MoveFolder(const String& pathFrom, const String &pathTo)
{
	if (pathFrom.length() > 0 && pathTo.length() > 0) {
		
		StringW  swPathFrom = convertToStringW(pathFrom);
		StringW  swPathTo   = convertToStringW(pathTo);

		TCHAR tcPathFrom[MAX_PATH+2];
		TCHAR tcPathTo[MAX_PATH+2];

        wsprintf(tcPathFrom, L"%s%c", swPathFrom.c_str(),'\0');
		wsprintf(tcPathTo, L"%s%c", swPathTo.c_str(),'\0');

		SHFILEOPSTRUCT fop;
		
		fop.hwnd = NULL;
		fop.wFunc = FO_MOVE;
		fop.pFrom = tcPathFrom;
		fop.pTo   = tcPathTo;
		fop.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR;
		
		int result = SHFileOperation(&fop);

		return result == 0;
	}
	return false;
}

bool CAppManager::RestartClient(HWND hwnd) 
{
	TCHAR module[MAX_PATH];
	GetModuleFileName(NULL,module,MAX_PATH);
	HINSTANCE res = ShellExecute(hwnd, L"open", module, L"-restarting", NULL, SW_SHOWNORMAL);
	return false;
}

void CAppManager::ReloadRhoBundle(HWND hwnd, const char* szUrl, const char* szZipPassword)
{	
	const char  *tmp_dir_name = "_tmp_";
	const char	*loadData = NULL;
	unsigned int loadSize = 0;
	String		 root_dir = rho_native_rhopath();
	String		 tmp_unzip_dir;
	int          errCode = RRB_UNKNOWN_ERR;

	root_dir.at(root_dir.length()-1) = '\\';
	tmp_unzip_dir = root_dir + tmp_dir_name;

	if (szUrl == NULL) {
		LOG(ERROR) + "null url passed to ReloadRhoBundle";
		return;
	}

	//trying to load file
	//NB: for mobile devices should use filesystem instead of RAM
	NetResponse resp = getNetRequest().pullData(szUrl, null);
	if (resp.getDataSize() > 0 && resp.getCharData()) 
    {
		loadData = resp.getCharData();
		loadSize = resp.getDataSize();
		errCode = RRB_NONE_ERR;
	} else {
		errCode = RRB_LOADING_ERR;
	}

	//trying to unzip loaded file
	if (errCode == RRB_NONE_ERR)
		errCode = unzipRhoBundle(loadData, loadSize, szZipPassword, tmp_unzip_dir);
	
	RHODESAPP().stopApp();

	//trying to delete old rhobundle
	if (errCode == RRB_NONE_ERR)
		errCode = removeOldRhoBundle();

	//move new rhobundle from temporary dir
	if (errCode == RRB_NONE_ERR) {
		errCode = updateRhoBundle (root_dir, tmp_unzip_dir);
	}

	showMessage(hwnd, errCode);
}

int CAppManager::unzipRhoBundle (const char *zipData, unsigned int zipDataSize, const char *zipPassword, const String &targetPath)
{
	int err_code = RRB_NONE_ERR;
	HANDLE hFind = 0;

	HZIP   hz = OpenZip((void *)zipData, zipDataSize, zipPassword);
	if (!hz) {
		LOG(ERROR) + "Failed to open zipfile";
		err_code = RRB_UNZIP_ERR;
	}

	if (err_code == RRB_NONE_ERR) {
		//check for target dir
		WIN32_FIND_DATA wfd;
		hFind = FindFirstFile(convertToStringW(targetPath).c_str(), &wfd);
		if (INVALID_HANDLE_VALUE == hFind) {
			if (CreateDirectory(convertToStringW(targetPath).c_str(), NULL) != TRUE) {
				LOG(ERROR) + "Failed to create temporary dir";
				err_code = RRB_UNZIP_ERR;
			}
		} else {
			LOG(WARNING) + "Temporary dir exists";
			if (!RemoveFolder (targetPath)) {
				LOG(ERROR) + "Failed to delete old temporary dir";
				err_code = RRB_UNZIP_ERR;
			}
		}
	}

	if (err_code == RRB_NONE_ERR) {
		// Set base for unziping
		SetUnzipBaseDir(hz, convertToStringW(targetPath).c_str());
					
		// Get info about the zip
		// -1 gives overall information about the zipfile
		ZIPENTRY ze;
		GetZipItem(hz,-1, &ze);
		int numitems = ze.index;
					
		// Iterate through items and unzip them
		for (int zi = 0; zi<numitems; zi++) { 
			// fetch individual details, e.g. the item's name.
			if (ZR_OK != GetZipItem(hz,zi,&ze)) {
				LOG(ERROR) + "Failed to unzip item";
				RemoveFolder (targetPath);
				err_code = RRB_UNZIP_ERR;
				break;
			}
			UnzipItem(hz, zi, ze.name);
		}
	}

	if (hz)
		CloseZip(hz);

	return err_code;
}

int CAppManager::removeOldRhoBundle (void)
{
	String root = rho_native_rhopath();
	root.at(root.length()-1) = '\\';

	if (!RemoveFolder(root + "apps")) {
		LOG(ERROR) + "Failed to remove" + "\"" + (root + "apps") + "\"";
		return RRB_REMOVEOLD_ERR;
	}

#if defined(APP_BUILD_CAPABILITY_MOTOROLA)
	const char *rhopath = rho_native_rhopath();
	const char *runtimepath = rho_native_reruntimepath();
	int rhopath_len = strlen(rhopath);
	int runtimepath_len = strlen(runtimepath);
	if ((rhopath_len > 0) && (rhopath_len == runtimepath_len) && (_strnicmp(rhopath, runtimepath, rhopath_len)==0)) {
#endif
		if (!RemoveFolder(root + "lib")) {
			LOG(ERROR) + "Failed to remove" + "\"" + (root + "lib	") + "\"";
			return RRB_REMOVEOLD_ERR;
		}
#if defined(APP_BUILD_CAPABILITY_MOTOROLA)
	}
#endif
	
	return RRB_NONE_ERR;
}


int CAppManager::updateRhoBundle (const String &root_path, const String &from_path)
{
	int err_code = RRB_NONE_ERR;

	if (!MoveFolder (from_path + "\\*.*", root_path)) {
		LOG(ERROR) + "Failed to move new rhobundle content";
		err_code = RRB_UPDATE_ERR;
	}

	if (err_code == RRB_NONE_ERR) {
		if (!RemoveFolder (from_path)) {
			LOG(ERROR) + "Failed to remove temporary dir";
		}
	}

	return err_code;
}

void CAppManager::showMessage(HWND hWnd, int rrbErrCode) 
{
	String messageText;

	if (rrbErrCode == RRB_NONE_ERR) {
		MessageBox(hWnd, 
					_T("Rhobundle has been updated successfully.\nApplication will be restarted."), 
					_T("Information"), 
					MB_OK | MB_ICONINFORMATION );
		RestartClient(hWnd); 
		//Close main window and client
		::PostMessage(hWnd,WM_CLOSE,0,0);
	}

	switch (rrbErrCode) {
	case RRB_LOADING_ERR:
		messageText = "Error loading rhobundle.";
		break;
	case RRB_UNZIP_ERR:
		messageText= "Can't unzip loaded rhobundle.";
		break;
	case RRB_REMOVEOLD_ERR:
		messageText = "Can't remove old version of rhobundle. However, it may be corrupted. Exit application and reinstall rhobundle manualy.";
		break;
	case RRB_UPDATE_ERR:
		messageText= "Error updating rhobundle. Exit application and reinstall rhobundle manualy.";
		break;
	default:
		messageText = "Error reloading rhobundle.";
		break;
	}

	MessageBox(hWnd, convertToStringW(messageText).c_str(), _T("Stop"), MB_OK | MB_ICONSTOP );
}

#endif