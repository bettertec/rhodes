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

#include "common/RhoPort.h"
#ifdef OS_WINDOWS_DESKTOP
#include <atlstr.h>
#endif
#if defined(WINDOWS_PLATFORM)
#include "ruby\wince\direct.h"
#endif

#include <gtest/gtest.h>

void runAllLogTests();
extern "C" void runAllProfilingTests();
extern "C" void rho_logconf_Init(const char* logPath, const char* path, const char* logPort);
extern "C" const char* RhoGetRootPath();

#ifdef OS_MACOSX
#include <sys/stat.h>
extern "C" int rho_sync_logged_in_cookies()
{
	return 1;
}

int main(int argc, char* argv[])
{
    mkdir(RhoGetRootPath(), 0777);
#else
int _tmain(int argc, _TCHAR* argv[])
{
	_mkdir(RhoGetRootPath());
		
#endif
	
    rho_logconf_Init(RhoGetRootPath(), RhoGetRootPath(), "");

    //runAllLogTests();
    //runAllProfilingTests();

    //testing::GTEST_FLAG(catch_exceptions) = true;
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#if defined(WIN32)
#if defined(_WIN32_WCE)
extern DWORD GetModuleFileNameA(HMODULE hModule,LPSTR lpFileName,DWORD size);
#endif
static int _root_loaded = 0;
static char _rootpath[MAX_PATH];
#if !defined(_WIN32_WCE)
void __setRootPath(const char* path) {
	strcpy(_rootpath,path);
	_root_loaded = 1;
}
#endif
const char* RhoGetRootPath() {
  int len;
  if (_root_loaded) {
    return _rootpath;
  }
  if( (len = GetModuleFileNameA(NULL,_rootpath,MAX_PATH)) == 0 )
  {
    strcpy(_rootpath,".");
  }
  else
  {
    while( !(_rootpath[len] == '\\'  || _rootpath[len] == '/') )
      len--;
    _rootpath[len]=0;
    sprintf(_rootpath,"%s\\rho\\",_rootpath);
  }
  _root_loaded = 1;
  return _rootpath;
}

#endif// WIN32

int rho_testvprintf(const char * fmt, va_list args)
{
#ifdef OS_WINDOWS_DESKTOP
    CAtlStringA str;
    str.FormatV(fmt,args);
    OutputDebugStringA(str);
#endif //OS_WINDOWS_DESKTOP
    return vfprintf(stdout,fmt, args);
}

int rho_testprintf(const char * fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int nres = rho_testvprintf(fmt, args);
    va_end(args);

    return nres;
}

#if defined( __SYMBIAN32__ ) || defined(_WIN32_WCE)
extern "C" char *getenv(const char *env)
{
	return 0;
}
#endif
