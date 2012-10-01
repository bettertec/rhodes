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

#ifndef _RHOMUTEXLOCK_H_
#define _RHOMUTEXLOCK_H_

#include "RhoPort.h"

#ifdef __cplusplus

namespace rho{
namespace common{

#if defined(WINDOWS_PLATFORM)
typedef CRITICAL_SECTION MutexType;
#else
typedef pthread_mutex_t MutexType;
#endif 

class CMutex{
public:
    inline CMutex();
    inline ~CMutex();

    inline void Lock();    // Block if needed until free then acquire exclusively
    inline void Unlock();  // Release a lock acquired via Lock()
	
	MutexType* getNativeMutex(){ return &m_nativeMutex;}
private:

    MutexType m_nativeMutex;
};

#if defined(WINDOWS_PLATFORM)
#if defined(OS_WP8)
CMutex::CMutex()             { InitializeCriticalSectionEx(&m_nativeMutex, 0, 0); }
#else
CMutex::CMutex()             { InitializeCriticalSection(&m_nativeMutex); }
#endif
CMutex::~CMutex()            { DeleteCriticalSection(&m_nativeMutex); }
void CMutex::Lock()         { EnterCriticalSection(&m_nativeMutex); }
void CMutex::Unlock()       { LeaveCriticalSection(&m_nativeMutex); }
#else
CMutex::CMutex()             {
	pthread_mutexattr_t mutex_details;
	pthread_mutexattr_init(&mutex_details);
	pthread_mutexattr_settype(&mutex_details,PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m_nativeMutex, &mutex_details);
	pthread_mutexattr_destroy(&mutex_details);
	
	//pthread_mutex_init(&m_nativeMutex, NULL); 
}
CMutex::~CMutex()            { pthread_mutex_destroy(&m_nativeMutex); }
void CMutex::Lock()         { pthread_mutex_lock(&m_nativeMutex); }
void CMutex::Unlock()       { pthread_mutex_unlock(&m_nativeMutex); }
#endif 

class CMutexLock {
public:
    explicit CMutexLock(CMutex& mutex) : m_Mutex(mutex) { m_Mutex.Lock(); }
    ~CMutexLock() { m_Mutex.Unlock(); }
private:
    CMutex& m_Mutex;

    // Disallow "evil" constructors
    CMutexLock(const CMutexLock&);
    void operator=(const CMutexLock&);
};

#define synchronized(mutex) rho::common::CMutexLock __lock(mutex);

class CLocalMutexLock {
public:
    explicit CLocalMutexLock() : m_Mutex() { m_Mutex.Lock(); }
    ~CLocalMutexLock() { m_Mutex.Unlock(); }
private:
    CMutex m_Mutex;

    // Disallow "evil" constructors
    CLocalMutexLock(const CLocalMutexLock&);
    void operator=(const CLocalMutexLock&);
};


}
}

#else

#if !defined(WINDOWS_PLATFORM)

#ifdef OS_SYMBIAN

#define RHO_INIT_LOCK(name)\
static int __g_mutex_init_##name = 0;\
pthread_mutex_t __g_mutex_##name = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

#else

#define RHO_INIT_LOCK(name)\
static int __g_mutex_init_##name = 0;\
pthread_mutex_t __g_mutex_##name = PTHREAD_MUTEX_INITIALIZER;

#endif

#define RHO_LOCK(name) {if(!__g_mutex_init_##name){\
  pthread_mutexattr_t attr;\
  pthread_mutexattr_init(&attr);\
  pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);\
  pthread_mutex_init(&__g_mutex_##name, &attr);\
  pthread_mutexattr_destroy(&attr);\
  __g_mutex_init_##name=1;\
} pthread_mutex_lock(&__g_mutex_##name);}
#define RHO_UNLOCK(name) pthread_mutex_unlock(&__g_mutex_##name);

#else

#define RHO_INIT_LOCK(name)\
static int __g_cs_init_##name = 0;\
CRITICAL_SECTION __g_cs_##name;

#define RHO_LOCK(name) {if(!__g_cs_init_##name){InitializeCriticalSection(&__g_cs_##name);__g_cs_init_##name=1;} EnterCriticalSection(&__g_cs_##name);}
#define RHO_UNLOCK(name) LeaveCriticalSection(&__g_cs_##name);

#endif

#endif //__cplusplus

#endif //_RHOMUTEXLOCK_H_
