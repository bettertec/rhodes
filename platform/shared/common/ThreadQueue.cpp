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

#include "ThreadQueue.h"

//1. when stop thread - cancel current command. Add cancelCurrentCommand to ThreadQueue and call it from stop


namespace rho {
namespace common {

unsigned int CThreadQueue::m_logThreadId = 0;

CThreadQueue::CThreadQueue() : CRhoThread()
{
    m_nPollInterval = QUEUE_POLL_INTERVAL_SECONDS;
    m_bNoThreaded = false;
    m_pCurCmd = null;
}

CThreadQueue::~CThreadQueue(void)
{
}

boolean CThreadQueue::isAlreadyExist(IQueueCommand *pCmd)
{
    boolean bExist = false;
    if ( isSkipDuplicateCmd() )
    {
        for (int i = 0; i < (int)m_stackCommands.size(); ++i)
        {
            if (m_stackCommands.get(i)->equals(*pCmd))
            {
                LOG(INFO) + "Command already exists in queue. Skip it.";
                bExist = true;
                break;
            }
        }
    }

    return bExist;
}

int CThreadQueue::getCommandsCount() {
    synchronized(m_mxStackCommands);
    return (int)m_stackCommands.size();
}
    
void CThreadQueue::addQueueCommandInt(IQueueCommand* pCmd)
{
	LOG(INFO) + "addCommand: " + pCmd->toString();

    synchronized(m_mxStackCommands);

    if ( !isAlreadyExist(pCmd) )
        m_stackCommands.add(pCmd);
}

void CThreadQueue::addQueueCommandToFrontInt(IQueueCommand *pCmd)
{
    LOG(INFO) + "addCommand to front: " + pCmd->toString();

    synchronized(m_mxStackCommands);

    if (!isAlreadyExist(pCmd))
        m_stackCommands.addToFront(pCmd);
}

void CThreadQueue::addQueueCommand(IQueueCommand* pCmd)
{ 
    addQueueCommandInt(pCmd);

    if ( isNoThreadedMode()  )
        processCommands();
    else if ( isAlive() )
	    stopWait(); 
}

void CThreadQueue::addQueueCommandToFront(IQueueCommand* pCmd)
{
    addQueueCommandToFrontInt(pCmd);

    if ( isNoThreadedMode() )
        processCommands();
    else if ( isAlive() )
        stopWait();
}

void CThreadQueue::stop(unsigned int nTimeoutToKill)
{
    cancelCurrentCommand();
    CRhoThread::stop(nTimeoutToKill);
}

void CThreadQueue::cancelCurrentCommand()
{
    synchronized(m_mxStackCommands);
    if ( m_pCurCmd != null )
        m_pCurCmd->cancel();
}

void CThreadQueue::processCommandBase(IQueueCommand* pCmd)
{
    {
        synchronized(m_mxStackCommands);
        m_pCurCmd = pCmd;
    }

    processCommand(pCmd);

    {
        synchronized(m_mxStackCommands);
        m_pCurCmd = null;
    }
}

void CThreadQueue::run()
{
    if(__rhoCurrentCategory.getName() == "NO_LOGGING")
		m_logThreadId = getThreadID();

	LOG(INFO) + "Starting main routine...";

	int nLastPollInterval = getLastPollInterval();
	while( !isStopping() )
	{
        unsigned int nWait = m_nPollInterval > 0 ? m_nPollInterval : QUEUE_POLL_INTERVAL_INFINITE;

        if ( m_nPollInterval > 0 && nLastPollInterval > 0 )
        {
            int nWait2 = m_nPollInterval - nLastPollInterval;
            if ( nWait2 <= 0 )
                nWait = QUEUE_STARTUP_INTERVAL_SECONDS;
            else
                nWait = nWait2;
        }

        if ( !m_bNoThreaded && !isStopping() && isNoCommands() )
		{
            LOG(INFO) + "ThreadQueue blocked for " + nWait + " seconds...";
            if ( wait(nWait) == 1 )
                onTimeout();
        }
        nLastPollInterval = 0;

        if ( !m_bNoThreaded && !isStopping() )
    		processCommands();
	}

    LOG(INFO) + "Thread shutdown";
}

boolean CThreadQueue::isNoCommands()
{
	boolean bEmpty = false;
	synchronized(m_mxStackCommands)
    {		
		bEmpty = m_stackCommands.isEmpty();
	}

	return bEmpty;
}

void CThreadQueue::processCommands()//throws Exception
{
	while(!isStopping() && !isNoCommands())
	{
		common::CAutoPtr<IQueueCommand> pCmd = null;
    	{
        	synchronized(m_mxStackCommands);
    		pCmd = (IQueueCommand*)m_stackCommands.removeFirst();
    	}
		
		processCommandBase(pCmd);
	}
}

void CThreadQueue::setPollInterval(int nInterval)
{ 
    m_nPollInterval = nInterval; 
    if ( isAlive() )
        stopWait();
}

};
};

