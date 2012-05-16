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

#ifndef _RHOPORT_H_
#define _RHOPORT_H_

#include "RhoDefs.h"

#ifdef OS_SYMBIAN
#define strnicmp strncasecmp
#define stricmp strcasecmp
#endif

#ifdef OS_WINDOWS_DESKTOP
#define strncasecmp _strnicmp
#endif

#ifdef __SYMBIAN32__
#include <arpa/inet.h>
#endif

#if defined( WINDOWS_PLATFORM )

#include <windows.h>
#include <time.h>
#ifdef _MSC_VER
#pragma warning(disable:4100)
#pragma warning(disable:4189)
#endif

#if defined(OS_PLATFORM_CE)
#ifdef __cplusplus
extern "C"{
#endif
time_t time(time_t *);
struct tm * __cdecl localtime(const time_t *);
size_t strftime(char *, size_t, const char *,
	const struct tm *);
struct tm * gmtime(const time_t *);
time_t mktime(struct tm *);

_CRTIMP extern char * tzname[2];

extern int _daylight;
#define daylight _daylight

extern long _timezone;
#ifdef __cplusplus
};
#endif
#endif

#if defined(OS_WINDOWS_DESKTOP)
#define _USE_MATH_DEFINES
#endif

#if defined(OS_WINCE)
#define M_PI 3.14159265358979323846
#define M_LN2 0.69314718055994530942
#endif

#include <math.h>

typedef int socklen_t;

#if defined(OS_WINCE)
#include "ruby/wince/sys/types.h"
#include "ruby/wince/sys/stat.h"
#ifdef __cplusplus
extern "C"
#endif //__cplusplus
char *strdup(const char * str);

#else //!defined(OS_WINCE)
#include <sys/stat.h>
#endif //!defined(OS_WINCE)

#define LOG_NEWLINE "\r\n"
#define LOG_NEWLINELEN 2

//typedef __int32 int32;
//typedef unsigned __int32 uint32;
typedef __int64 int64;
typedef unsigned __int64 uint64;

#define strcasecmp _stricmp
#define snprintf _snprintf

#define FMTI64 "%I64d"
#define FMTU64 "%I64u"

#else // !defined( WINDOWS_PLATFORM)

#define FMTI64 "%lli"
#define FMTU64 "%llu"

#  if defined(OS_ANDROID) || defined(OS_LINUX)
// Needed for va_list on Android
#    include <stdarg.h>
#    include <sys/select.h>
#    include <stdio.h>
#    include <ctype.h>
#  else
#    include <wchar.h>
#  endif // OS_ANDROID

#if defined(OS_SYMBIAN)
#  include <sys/select.h>
#endif

#  include <sys/types.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <unistd.h>
#  include <errno.h>
#  if defined(OS_MACOSX) || defined(OS_LINUX)
#    include <sys/time.h>
#  endif
#  include <stdlib.h>
#  include <string.h>
#  include <pthread.h>
#  include <fcntl.h>
#  include <common/stat.h>

#undef ASSERT
#define ASSERT RHO_ASSERT

#define LOG_NEWLINE "\n"
#define LOG_NEWLINELEN 1

//typedef int32_t int32;
//typedef uint32_t uint32;
typedef long long int64;
typedef unsigned long long uint64;

#if defined(OS_SYMBIAN) || defined(OS_LINUX) || defined(OS_MACOSX)
#define M_PI 3.14159265358979323846
#endif

#endif 

#if defined( OS_WINCE )
#  define	vsnprintf	_vsnprintf
#  define	vswnprintf	_vsnwprintf
#elif defined( OS_ANDROID )
RHO_GLOBAL int vswnprintf(wchar_t *, size_t, const wchar_t *, void *);
#else
#  define	vswnprintf vswprintf
#endif //OS_WINCE

//#include "tcmalloc/rhomem.h"

#ifdef __cplusplus
extern "C" {
#endif
	
char* str_assign_ex( char* data, int len); 
char* str_assign(char* data); 
#ifdef __cplusplus
}
#endif

#define RHO_TRACE_POINT RHO_LOG("trace point")

#endif //_RHOPORT_H_
