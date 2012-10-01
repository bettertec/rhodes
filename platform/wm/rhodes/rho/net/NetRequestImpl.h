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

#include <wininet.h>
#include "net/INetRequest.h"
#include "logging/RhoLog.h"
#if defined(RHO_SYMBIAN) || defined(OS_WINDOWS_DESKTOP)
#include <atlstr.h>
#endif

namespace rho {
namespace common {
    class CRhoFile;
}

namespace net {

class CNetResponseImpl : public INetResponse
{
    bool   m_bValid;
    String m_data;
    int    m_nRespCode;
    String m_cookies;
public:
    CNetResponseImpl() : m_bValid(false),m_nRespCode(-1){}

    virtual const char* getCharData()
    {
        return m_bValid ? m_data.c_str() : "";
    }

    virtual void setCharData(const char* szData)
    {
        m_bValid = true;
        m_data = szData;
    }

    virtual unsigned int getDataSize()
    {
        return m_bValid ? m_data.size() : 0;
    }

    virtual String getCookies()
    {
        return m_cookies;
    }
    virtual int getRespCode(){ return m_nRespCode;}

    bool isValid()const{ return m_bValid; }
    void setValid(bool b){ m_bValid = b; }

    String& getRawData(){ return m_data; }

    void setCookies(String s)
    {
        m_cookies = s;
    }
    
    void setResponseCode(int nRespCode){ m_nRespCode = nRespCode;}

    boolean isSuccess()
    {
        return m_nRespCode > 0 && m_nRespCode < 400;
    }

	boolean isOK()
	{
		return m_nRespCode == 200 || m_nRespCode == 206;
	}

/*
    boolean isResponseRecieved(){ return m_nRespCode!=-1;}

	boolean isOK()
	{
		return m_nRespCode == 200 || m_nRespCode == 206;
	}

    boolean isUnathorized()
    {
        return m_nRespCode == 401;
    }

    boolean isRedirect()
    {
       return m_nRespCode == 301 || m_nRespCode ==302;
    }

    virtual boolean isSuccess()
    {
        return m_nRespCode > 0 && m_nRespCode < 400;
    }
*/
};

class CNetRequest;
class CNetRequestImpl : public INetRequestImpl
{
    DEFINE_LOGCLASS;

    static common::CMutex m_mxInternet;
    static HINTERNET m_hInternet;
    static HANDLE    m_hWceConnMgrConnection;
    HINTERNET  m_hConnection, m_hRequest;

    LPCTSTR  m_pszErrFunction;

    URL_COMPONENTS m_uri;
    CAtlStringW m_strReqUrlW;
    String      m_strUrl;
    Hashtable<String,String>* m_pHeaders;
    boolean m_bCancel;
    IRhoSession* m_pSession;
    boolean m_sslVerifyPeer;

public :
    CNetRequestImpl();
    ~CNetRequestImpl();

    virtual INetResponse* doRequest( const char* method, const String& strUrl, const String& strBody, IRhoSession* oSession, Hashtable<String,String>* pHeaders );
    virtual INetResponse* pullFile(const String& strUrl, common::CRhoFile& oFile, IRhoSession* oSession, Hashtable<String,String>* pHeaders);
    virtual INetResponse* pushMultipartData(const String& strUrl, VectorPtr<CMultipartItem*>& arItems, IRhoSession* oSession, Hashtable<String,String>* pHeaders);

    virtual void cancel();

    virtual boolean getSslVerifyPeer() { return m_sslVerifyPeer; }
    virtual void setSslVerifyPeer(boolean mode){ m_sslVerifyPeer = mode; }

    virtual INetResponse* createEmptyNetResponse(){ return new CNetResponseImpl(); }

    static void deinitConnection();

private:
    void close();
    bool isError(){ return m_pszErrFunction!= null; }

    void init( const char* method, const String& strUrl, IRhoSession* oSession, Hashtable<String,String>* pHeaders);

    String makeClientCookie();

    void ErrorMessage(LPCTSTR pszFunction);
    void readInetFile( void* hRequest, CNetResponseImpl* pNetData, common::CRhoFile* pFile = NULL, char* pBuf = NULL, DWORD dwBufSize = 0);
    void alloc_url_components(URL_COMPONENTS *uri, const wchar_t *url);
    void free_url_components(URL_COMPONENTS *uri);
    void readResponse(CNetResponseImpl* pNetData);
    bool SetupInternetConnection(LPCTSTR url);

    boolean readHeaders(Hashtable<String,String>& oHeaders);
    void    writeHeaders(Hashtable<String,String>* pHeaders);
    boolean checkSslCertError();

    String getBodyContentType();
    bool internetWriteHeader( const char* szPrefix, const char* szBody, const char* szPrefixEnd);

    int processMultipartItems( VectorPtr<CMultipartItem*>& arItems );

    bool initConnection(boolean bLocalHost, LPCTSTR url);
};

}
}

