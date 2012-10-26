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

#include "common/IRhoClassFactory.h"
#include "net/CURLNetRequest.h"
#include "common/PosixThreadImpl.h"
#include "net/iphone/sslimpl.h"
#include "RhoCryptImpl.h"

#include <common/RhoMutexLock.h>
#include <common/AutoPointer.h>

namespace rho {
namespace common {
		
class CRhoClassFactory : public common::IRhoClassFactory
{
public:
    net::INetRequestImpl* createNetRequestImpl()
    {
        return new net::CURLNetRequest();
    }
    common::IRhoThreadImpl* createThreadImpl()
    {
        return new CPosixThreadImpl;
    }

    IRhoCrypt* createRhoCrypt()
    {
        return new CRhoCryptImpl();
    }

    net::ISSL* createSSLEngine()
    {
        if(!m_pSsl)
        {
            CMutexLock lock(m_sslMutex);
            m_pSsl = new net::SSLImpl();
        }
        return m_pSsl;
    }
    
    
private:    
    CMutex m_sslMutex;
    common::CAutoPtr<net::ISSL> m_pSsl;    
};

}
}
