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

#ifndef _RHODEFS_H_
#define _RHODEFS_H_

#if defined(__SYMBIAN32__)
# define OS_SYMBIAN
# undef UNICODE

#elif defined(_WIN32_WCE)
# define OS_WINCE _WIN32_WINCE
#elif defined(WIN32)
# define OS_WINDOWS_DESKTOP
#elif defined(__CYGWIN__) || defined(__CYGWIN32__)
# define OS_CYGWIN
#elif defined(linux) || defined(__linux) || defined(__linux__)
# define OS_LINUX
#elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
# define OS_MACOSX
#elif defined(__FreeBSD__)
# define OS_FREEBSD
#else
#endif

#if defined(OS_WINDOWS_DESKTOP) || defined(OS_WINCE)
#define WINDOWS_PLATFORM
#endif

#ifdef __cplusplus
# if defined(__GNUC__) && __GNUC__ >= 4
#  define RHO_GLOBAL extern "C" __attribute__ ((visibility ("default")))
#  define RHO_LOCAL  __attribute__ ((visibility ("hidden")))
# else
#  define RHO_GLOBAL extern "C"
#  define RHO_LOCAL
# endif
#else
# if defined(__GNUC__) && __GNUC__ >= 4
#  define RHO_GLOBAL extern __attribute__ ((visibility ("default")))
#  define RHO_LOCAL  __attribute__ ((visibility ("hidden")))
# else
#  define RHO_GLOBAL extern
#  define RHO_LOCAL
# endif
#endif

#define RHO_ABORT(x) \
  do { \
    assert(!(x)); \
    abort(); \
  } while( __LINE__ != -1 )

#define RHO_USE_OWN_HTTPD
#define RHO_HTTPD_COMMON_IMPL

#ifdef OS_MACOSX
#include <TargetConditionals.h>
#endif //OS_MACOSX

#if defined( _DEBUG ) || defined (TARGET_IPHONE_SIMULATOR)
#define RHO_DEBUG 1
#endif

#define L_TRACE   0
#define L_INFO    1
#define L_WARNING 2
#define L_ERROR   3
#define L_FATAL   4
#define L_NUM_SEVERITIES  5

#define RHO_STRIP_LOG 0
#define RHO_STRIP_PROFILER 1

typedef int LogSeverity;

#if defined( WINDOWS_PLATFORM )
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif //_CRT_SECURE_NO_WARNINGS

#ifndef _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_NON_CONFORMING_SWPRINTFS 1
#endif //_CRT_NON_CONFORMING_SWPRINTFS

#ifndef _CRT_NONSTDC_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS 1
#endif //_CRT_NONSTDC_NO_WARNINGS

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1
#endif //_CRT_SECURE_NO_DEPRECATE

#endif

#if !defined(OS_ANDROID)
#include "tcmalloc/rhomem.h"
#endif

#ifdef RHODES_EMULATOR
#define RHO_RB_EXT ".rb"
#define RHO_ERB_EXT ".erb"
#define RHO_EMULATOR_DIR "rhosimulator"
#else
#define RHO_RB_EXT ".iseq"
#define RHO_ERB_EXT "_erb.iseq"
#define RHO_EMULATOR_DIR ""
#endif

#endif //_RHODEFS_H_

