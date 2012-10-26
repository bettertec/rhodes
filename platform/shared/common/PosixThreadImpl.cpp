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

#include "common/PosixThreadImpl.h"

extern "C"
{
void* rho_nativethread_start();
void rho_nativethread_end(void*);
}

namespace rho
{
namespace common
{

IMPLEMENT_LOGCLASS(CPosixThreadImpl, "RhoThread");

CPosixThreadImpl::CPosixThreadImpl()
    :m_stop_wait(false)
{
#if defined(OS_ANDROID)
    // Android has no pthread_condattr_xxx API
    pthread_cond_init(&m_condSync, NULL);
#else
    pthread_condattr_t sync_details;
    pthread_condattr_init(&sync_details);
    pthread_cond_init(&m_condSync, &sync_details);
    pthread_condattr_destroy(&sync_details);
#endif
}

CPosixThreadImpl::~CPosixThreadImpl()
{
    pthread_cond_destroy(&m_condSync);
}

void *runProc(void *pv)
{
    IRhoRunnable *p = static_cast<IRhoRunnable *>(pv);
    void *pData = rho_nativethread_start();
    p->runObject();
    rho_nativethread_end(pData);
    return 0;
}

void CPosixThreadImpl::start(IRhoRunnable *pRunnable, IRhoRunnable::EPriority ePriority)
{
    pthread_attr_t  attr;
    int return_val = pthread_attr_init(&attr);
    //return_val = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    RHO_ASSERT(!return_val);

    if ( ePriority != IRhoRunnable::epNormal)
    {
        sched_param param;
        return_val = pthread_attr_getschedparam (&attr, &param);
        param.sched_priority = ePriority == IRhoRunnable::epLow ? 20 : 100; //TODO: sched_priority
        return_val = pthread_attr_setschedparam (&attr, &param);
    }

    #ifdef __SYMBIAN32__
        size_t stacksize = 80000;
        pthread_attr_setstacksize(&attr, stacksize);
    #endif

    int thread_error = pthread_create(&m_thread, &attr, &runProc, pRunnable);
    return_val = pthread_attr_destroy(&attr);
    RHO_ASSERT(!return_val);
    RHO_ASSERT(thread_error==0);
}

void CPosixThreadImpl::stop(unsigned int nTimeoutToKill)
{
    stopWait();

    //TODO: wait for nTimeoutToKill and kill thread
    void* status;
    pthread_join(m_thread,&status);
}

int CPosixThreadImpl::wait(unsigned int nTimeout)
{
    struct timeval    tp;
    struct timespec   ts;
    unsigned long long max;
    bool timed_wait = (int)nTimeout >= 0;
    
    if (timed_wait)
    {
        gettimeofday(&tp, NULL);
        /* Convert from timeval to timespec */
        ts.tv_sec  = tp.tv_sec;
        ts.tv_nsec = tp.tv_usec * 1000;
        ts.tv_sec += nTimeout;
        max = ((unsigned long long)tp.tv_sec + nTimeout)*1000000 + tp.tv_usec;
    }

    common::CMutexLock oLock(m_mxSync);
    int nRet = 0;
    while (!m_stop_wait)
    {
        if (timed_wait) {
            gettimeofday(&tp, NULL);
            unsigned long long now = ((unsigned long long)tp.tv_sec)*1000000 + tp.tv_usec;
            if (now > max)
                break;
            
            nRet = pthread_cond_timedwait(&m_condSync, m_mxSync.getNativeMutex(), &ts) == ETIMEDOUT ? 1 : 0;
        }
        else
            pthread_cond_wait(&m_condSync, m_mxSync.getNativeMutex());
    }
    m_stop_wait = false;

    return nRet;
}

void CPosixThreadImpl::sleep(unsigned int nTimeout)
{
    ::usleep(1000*nTimeout);
}

void CPosixThreadImpl::stopWait()
{
    common::CMutexLock oLock(m_mxSync);
    m_stop_wait = true;
    pthread_cond_broadcast(&m_condSync);
}

} // namespace common
} // namespace rho

