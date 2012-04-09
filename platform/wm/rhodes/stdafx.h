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

//#if !defined(_WIN32_WCE)
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
//#endif

#ifndef _CRT_NONSTDC_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS 1
#endif //_CRT_NONSTDC_NO_WARNINGS

#if defined( _X86_) || defined(OS_PLATFORM_CE) 
    #pragma comment(linker, "/nodefaultlib:libc.lib")
    #pragma comment(linker, "/nodefaultlib:libcd.lib")
    #pragma comment(linker, "/nodefaultlib:oldnames.lib")
#endif

#ifndef STRICT
#define STRICT
#endif

#include "logging/RhoATLTrace.h"
#define __DBGAPI_H__

#if defined (_WIN32_WCE)
#include <ceconfig.h>
#endif

#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
#define SHELL_AYGSHELL
#endif

#if defined (_WIN32_WCE)
// NOTE - this is value is not strongly correlated to the Windows CE OS version being targeted
#undef WINVER
#define WINVER _WIN32_WCE
#else
// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define _ATL_APARTMENT_THREADED

#endif

#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS  // some CString constructors will be explicit

#ifndef OS_PLATFORM_MOTCE
// CE COM has no single threaded apartment (everything runs in the MTA).
// Hence we declare we're not concerned about thread safety issues:
#define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA

#else
#define SHELL_AYGSHELL
#endif

#include <atlbase.h>
#include <atlcom.h>
#include <atlwin.h>
#include <atlhost.h>
//#include <atlsiface.h>
#include <atlstr.h>
#include <atlcoll.h>

#if defined (_WIN32_WCE) 
//--- Define max and min macroses for WTL only ---
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#include <atlapp.h>
#include <atlframe.h>
#include <atlmisc.h>
#include <atlctrls.h>
#include <atldlgs.h>

#if defined(WIN32_PLATFORM_WFSP) && !defined(_TPCSHELL_H_)
#include <tpcshell.h>
#endif

#if defined(_WIN32_WCE) && !defined(OS_PLATFORM_MOTCE)
#include <aygshell.h>
#pragma comment(lib, "aygshell.lib") 
//#include <tpcshell.h> // Required for SHSendBackToFocusWindow
//#endif // SHELL_AYGSHELL

//#ifndef OS_PLATFORM_MOTCE
#define _WTL_CE_NO_ZOOMSCROLL
#define _WTL_CE_NO_CONTROLS
#include <atlwince.h>
#endif 

#undef max
#undef min
//---   ---
#endif

//#if defined (_WIN32_WCE) && !defined( OS_PLATFORM_CE )
//#include <pvdispid.h>
//#include <piedocvw.h>
//#endif


#include "Macros.h"

#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
#ifndef _DEVICE_RESOLUTION_AWARE
#define _DEVICE_RESOLUTION_AWARE
#endif
#endif

#ifdef _DEVICE_RESOLUTION_AWARE
#include "DeviceResolutionAware.h"
#endif

using namespace ATL;

/*#ifdef SHELL_AYGSHELL
#include <aygshell.h>
#pragma comment(lib, "aygshell.lib") 
#include <tpcshell.h> // Required for SHSendBackToFocusWindow
#endif // SHELL_AYGSHELL
*/

// TODO: temporary code, until the CE compilers correctly implement the /MT[d], /MD[d] switches, and MFCCE fixes some #pragma issues
#ifdef _DLL // /MD
    #if defined(_DEBUG)
        #pragma comment(lib, "msvcrtd.lib")
    #else
        #pragma comment(lib, "msvcrt.lib")
    #endif
#else // /MT
    #if defined(_DEBUG)
        #pragma comment(lib, "libcmtd.lib")
    #else
        #pragma comment(lib, "libcmt.lib")
    #endif
#endif

#if _WIN32_WCE < 0x500 && ( defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP) )
    #pragma comment(lib, "ccrtrtti.lib")
    #ifdef _X86_
        #if defined(_DEBUG)
            #pragma comment(lib, "libcmtx86d.lib")
        #else
            #pragma comment(lib, "libcmtx86.lib")
        #endif
    #endif
#endif

#include "tcmalloc/rhomem.h"
#include "logging/RhoLog.h"
//#include <afxwin.h>

#if defined(OS_WINDOWS_DESKTOP) && !defined(RHO_SYMBIAN)
#include <atlapp.h>
//#include <atlwin.h>
//#include <atlcrack.h>
//#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
//#include <atlctrlx.h>
//#include <atlmisc.h>
//#include <atlddx.h>
#include <atlmisc.h>
#endif
