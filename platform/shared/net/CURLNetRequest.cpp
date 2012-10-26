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

#include "net/CURLNetRequest.h"
#include "logging/RhoLog.h"
#include "common/RhoFile.h"
#include "common/RhodesApp.h"
#include "common/RhoConf.h"
#include "net/URI.h"

#include <algorithm>

#undef DEFAULT_LOGCATEGORY
#define DEFAULT_LOGCATEGORY "Net"

extern "C" void rho_net_impl_network_indicator(int active);

namespace rho
{
namespace net
{
curl_slist *set_curl_options(bool trace, CURL *curl, const char *method, const String& strUrl, const String& strBody,
                             IRhoSession* pSession, Hashtable<String,String>* pHeaders, bool sslVerifyPeer);
CURLcode do_curl_perform(CURLM *curlm, CURL *curl);
	
IMPLEMENT_LOGCLASS(CURLNetRequest, "Net");

class CURLNetResponseImpl : public INetResponse
{
    //Vector<char> m_data;
	String m_data;
    int   m_nRespCode;
    String m_cookies;

public:
    CURLNetResponseImpl(char const *data, size_t size, int nRespCode)
        :m_nRespCode(nRespCode)
    {
        m_data.assign(data, size);
    }

    virtual const char* getCharData()
    {
        return m_data.c_str();
    }

    virtual unsigned int getDataSize()
    {
        return m_data.size();
    }

    virtual int getRespCode() 
    {
        return m_nRespCode;
    }

    virtual String getCookies()
    {
        return m_cookies;
    }

    virtual void setCharData(const char* szData)
    {
        m_data = szData;
    }

    void setRespCode(int nRespCode) 
    {
        m_nRespCode = nRespCode;
    }

    boolean isOK()
    {
        return m_nRespCode == 200 || m_nRespCode == 206;
    }
    
    boolean isUnathorized()
    {
        return m_nRespCode == 401;
    }

    boolean isSuccess()
    {
        return m_nRespCode > 0 && m_nRespCode < 400;
    }

    boolean isResponseRecieved(){ return m_nRespCode!=-1;}

    void setCharData(const String &data)
    {
        m_data = data;//.assign(data.begin(), data.end());
    }
    
    void setCookies(String s)
    {
        m_cookies = s;
    }
   
};

INetResponse *CURLNetRequest::makeResponse(String const &body, int nErrorCode)
{
    return makeResponse(body.c_str(), body.size(), nErrorCode);
}

INetResponse *CURLNetRequest::makeResponse(Vector<char> const &body, int nErrorCode)
{
    return makeResponse(&body[0], body.size(), nErrorCode);
}

INetResponse *CURLNetRequest::makeResponse(char const *body, size_t bodysize, int nErrorCode)
{
    RAWTRACE1("CURLNetRequest::makeResponse - nErrorCode: %d", nErrorCode);
    if (!body) {
        body = "";
        bodysize = 0;
    }

    std::auto_ptr<CURLNetResponseImpl> resp(new CURLNetResponseImpl(body, bodysize, nErrorCode>0?nErrorCode:-1));
    if (resp->isSuccess())
        resp->setCookies(makeCookies());
    return resp.release();
}

static size_t curlHeaderCallback(void *ptr, size_t size, size_t nmemb, void *opaque)
{
    Hashtable<String,String>* pHeaders = (Hashtable<String,String>*)opaque;
    size_t nBytes = size*nmemb;
    String strHeader((const char *)ptr, nBytes);
    RAWTRACE1("Received header: %s", strHeader.c_str());
    
    int nSep = strHeader.find(':');
    if (nSep > 0 )
    {		
        String strName = String_trim(strHeader.substr(0, nSep));
        String lName;
        std::transform(strName.begin(), strName.end(), std::back_inserter(lName), &::tolower);
        
        String strValue = String_trim(strHeader.substr(nSep+1, strHeader.length() - (nSep+3) ));
        
        if ( pHeaders->containsKey(lName) )
        {
            strValue += ";" + pHeaders->get( lName );
            pHeaders->put( lName, strValue );
        }
        else
            pHeaders->put(lName, strValue);
    }
    
    return nBytes;
}

static size_t curlBodyStringCallback(void *ptr, size_t size, size_t nmemb, void *opaque)
{
    String *pStr = (String *)opaque;
    size_t nBytes = size*nmemb;
    RAWTRACE1("Received %d bytes", nBytes);
    pStr->append((const char *)ptr, nBytes);
    return nBytes;
}

static size_t curlBodyBinaryCallback(void *ptr, size_t size, size_t nmemb, void *opaque)
{
    Vector<char> *pBody = (Vector<char> *)opaque;
    size_t nBytes = size*nmemb;
    RAWTRACE1("Received %d bytes", nBytes);
    std::copy((char*)ptr, (char*)ptr + nBytes, std::back_inserter(*pBody));
    return nBytes;
}

extern "C" int rho_net_ping_network(const char* szHost);	

INetResponse* CURLNetRequest::doRequest(const char *method, const String& strUrl,
                                        const String& strBody, IRhoSession *oSession,
                                        Hashtable<String,String>* pHeaders)
{
    INetResponse* pResp = doPull(method, strUrl, strBody, null, oSession, pHeaders);
    return pResp;
}

CURLcode CURLNetRequest::doCURLPerform(const String& strUrl)
{
	CURLcode err = m_curl.perform();
	if ( err !=  CURLE_OK && !RHODESAPP().isBaseUrl(strUrl.c_str()) && err != CURLE_OBSOLETE4 )
	{
		long statusCode = 0;
		curl_easy_getinfo(m_curl.curl(), CURLINFO_RESPONSE_CODE, &statusCode);
		if ( statusCode == 0 && rho_net_ping_network(strUrl.substr(0, strUrl.find("?")).c_str()) )
			err = m_curl.perform();
	}

	return err;
}
	
INetResponse* CURLNetRequest::doPull(const char* method, const String& strUrl,
                                     const String& strBody, common::CRhoFile *oFile,
                                     IRhoSession* oSession, Hashtable<String,String>* pHeaders )
{
    int nRespCode = -1;
    Vector<char> respBody;
    long nStartFrom = 0;
    if (oFile)
        nStartFrom = oFile->size();

	if( !RHODESAPP().isBaseUrl(strUrl.c_str()) )
    {
        //Log every non localhost requests
        RAWLOG_INFO2("%s request (Pull): %s", method, strUrl.c_str());
        rho_net_impl_network_indicator(1);
    }

    Hashtable<String,String> h;
    if (pHeaders)
        h = *pHeaders;

    for (int nAttempts = 0; nAttempts < 10; ++nAttempts) {
        Vector<char> respChunk;
        
        curl_slist *hdrs = m_curl.set_options(method, strUrl, strBody, oSession, &h);
        CURL *curl = m_curl.curl();
        if (pHeaders) {
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, pHeaders);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, &curlHeaderCallback);
        }
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &respChunk);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &curlBodyBinaryCallback);
        if (nStartFrom > 0)
		{
			RAWLOG_INFO1("CURLNetRequest::doPull - resuming from %d",nStartFrom);
            curl_easy_setopt(curl, CURLOPT_RESUME_FROM, nStartFrom);
		}

        CURLcode err = doCURLPerform(strUrl);
        curl_slist_free_all(hdrs);
        
        long statusCode = 0;
        if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode) != 0)
            statusCode = 500;
        
        RAWTRACE2("CURLNetRequest::doPull - Status code: %d, response size: %d", (int)statusCode, respChunk.size() );
		
		if (statusCode == 416 )
		{
			//Do nothing, file is already loaded
		}else if (statusCode == 206) {
            if (oFile)
                oFile->write(&respChunk[0], respChunk.size());
            else
                std::copy(respChunk.begin(), respChunk.end(), std::back_inserter(respBody));
            // Clear counter of attempts because 206 response does not considered to be failed attempt
            nAttempts = 0;
		}
        else {
            if (oFile) {
                oFile->movePosToStart();
                oFile->write(&respChunk[0], respChunk.size());
            }
            else
                respBody = respChunk;
        }
        
        if (err == CURLE_OPERATION_TIMEDOUT && respChunk.size() > 0) {
            RAWLOG_INFO("Connection was closed by timeout, but we have part of data received; try to restore connection");
            nRespCode = -1;
			if ( oFile != 0 ) {
				oFile->flush();
				nStartFrom = oFile->size();
			} else {
				nStartFrom = respBody.size();
			}
            continue;
        }
        
        nRespCode = getResponseCode(err, respBody, oSession);
        break;
    }

	if( !RHODESAPP().isBaseUrl(strUrl.c_str()) )		   
	   rho_net_impl_network_indicator(0);

    return makeResponse(respBody, nRespCode);
}

INetResponse* CURLNetRequest::createEmptyNetResponse()
{
    return new CURLNetResponseImpl("", 0, -1);
}

INetResponse* CURLNetRequest::pullFile(const String& strUrl, common::CRhoFile& oFile, IRhoSession* oSession, Hashtable<String,String>* pHeaders)
{
    return doPull("GET", strUrl, String(), &oFile, oSession, pHeaders);
}

INetResponse* CURLNetRequest::pushMultipartData(const String& strUrl, VectorPtr<CMultipartItem*>& arItems, IRhoSession* oSession, Hashtable<String,String>* pHeaders)
{
    int nRespCode = -1;
    String strRespBody;
    
    RAWLOG_INFO1("POST request (Push): %s", strUrl.c_str());
    rho_net_impl_network_indicator(1);
    
    curl_slist *hdrs = m_curl.set_options("POST", strUrl, String(), oSession, pHeaders);
    CURL *curl = m_curl.curl();
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &strRespBody);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &curlBodyStringCallback);
	
    curl_httppost *post = NULL, *last = NULL;
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, NULL);

    for (size_t i = 0, lim = arItems.size(); i < lim; ++i) {
        CMultipartItem *mi = arItems[i];
        
        size_t cl;
        if (mi->m_strFilePath.empty())
            cl = mi->m_strBody.size();
        else {
            common::CRhoFile f;
            if (!f.open(mi->m_strFilePath.c_str(), common::CRhoFile::OpenReadOnly))
                cl = 0;
            else {
                cl = f.size();
                f.close();
            }
        }

        char buf[32];
        buf[sizeof(buf) - 1] = '\0';
        snprintf(buf, sizeof(buf) - 1, "Content-Length: %lu", (unsigned long)cl);
        curl_slist *fh = NULL;
        fh = curl_slist_append(fh, buf);

        const char *name = mi->m_strName.empty() ? "blob" : mi->m_strName.c_str();
        int opt = mi->m_strFilePath.empty() ? CURLFORM_COPYCONTENTS : CURLFORM_FILE;
        const char *data = mi->m_strFilePath.empty() ? mi->m_strBody.c_str() : mi->m_strFilePath.c_str();
        const char *ct = mi->m_strContentType.empty() ? NULL : mi->m_strContentType.c_str();
        if (ct) {
            curl_formadd(&post, &last,
                         CURLFORM_COPYNAME, name,
                         opt, data,
                         CURLFORM_CONTENTTYPE, ct,
                         CURLFORM_CONTENTHEADER, fh,
                         CURLFORM_END);
        }
        else {
            curl_formadd(&post, &last,
                         CURLFORM_COPYNAME, name,
                         opt, data,
                         CURLFORM_CONTENTHEADER, fh,
                         CURLFORM_END);
        }
    }
        
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
    
    CURLcode err = doCURLPerform(strUrl);
	
    curl_slist_free_all(hdrs);
    curl_formfree(post);
    
    rho_net_impl_network_indicator(0);
    
    nRespCode = getResponseCode(err, strRespBody, oSession);
    return makeResponse(strRespBody, nRespCode);
}

int CURLNetRequest::getResponseCode(CURLcode err, String const &body, IRhoSession* oSession)
{
    return getResponseCode(err, body.c_str(), body.size(), oSession);
}

int CURLNetRequest::getResponseCode(CURLcode err, Vector<char> const &body, IRhoSession* oSession)
{
    return getResponseCode(err, &body[0], body.size(), oSession);
}

int CURLNetRequest::getResponseCode(CURLcode err, char const *body, size_t bodysize, IRhoSession* oSession )	
{    
    //if (err != CURLE_OK)
    //    return -1;

    if (!body) {
        body = "";
        bodysize = 0;
    }
	
    long statusCode = 0;
    CURL *curl = m_curl.curl();
    if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode) != 0)
        statusCode = 500;
    
	if (statusCode == 416) {
		statusCode = 206;
	}
	
    if (statusCode >= 400) {
        RAWLOG_ERROR2("Request failed. HTTP Code: %d returned. HTTP Response: %s",
                      (int)statusCode, body);
        if (statusCode == 401)
            if (oSession)
                oSession->logout();
    }
    else {
        if (err != CURLE_OK && err != CURLE_PARTIAL_FILE && statusCode != 0)
            statusCode = 500;

        RAWTRACE1("RESPONSE----- (%d bytes)", bodysize);
        if ( !rho_conf_getBool("log_skip_post") )
        {
#ifndef OS_SYMBIAN
            RAWTRACE_DATA(body, bodysize);
#endif
        }
        RAWTRACE("END RESPONSE-----");
    }

    RAWTRACE1("CURLNetRequest::getResponseCode - Status code: %d", (int)statusCode);
    return (int)statusCode;
}

void CURLNetRequest::cancel()
{
    m_curl.cancel();
}

String CURLNetRequest::makeCookies()
{
	String cookies;
	curl_slist *rcookies = NULL;
	if (curl_easy_getinfo(m_curl.curl(), CURLINFO_COOKIELIST, &rcookies) != 0) {
        // No cookies
        RAWTRACE("No cookies");
        return cookies;
    }
	
    for(curl_slist *cookie = rcookies; cookie; cookie = cookie->next) {
        char *data = cookie->data;
        
        // Parse cookie which is in Netscape format:
        // domain <Tab> tailmatch <Tab> path <Tab> secure <Tab> expires <Tab> name <Tab> value
        char *s = data;
        
        // Skip 'domain'
        for (; *s == '\t'; ++s);
        for (; *s != '\t' && *s != '\0'; ++s);
        if (*s == '\0') continue;
        
        // Skip 'tailmatch'
        for (; *s == '\t'; ++s);
        for (; *s != '\t' && *s != '\0'; ++s);
        if (*s == '\0') continue;
        
        // Skip 'path'
        for (; *s == '\t'; ++s);
        for (; *s != '\t' && *s != '\0'; ++s);
        if (*s == '\0') continue;
        
        // Skip 'secure'
        for (; *s == '\t'; ++s);
        for (; *s != '\t' && *s != '\0'; ++s);
        if (*s == '\0') continue;
        
        // Skip 'expires'
        for (; *s == '\t'; ++s);
        for (; *s != '\t' && *s != '\0'; ++s);
        if (*s == '\0') continue;
        
        // Parse 'name'
        for (; *s == '\t'; ++s);
        char *name_start = s;
        for (; *s != '\t' && *s != '\0'; ++s);
        if (*s == '\0') continue;
        char *name_finish = s;
        String name(name_start, name_finish);
        
        // Parse 'value'
        for (; *s == '\t'; ++s);
        char *value = s;
        
        cookies += name;
        cookies += "=";
        cookies += value;
        cookies += ";";
    }
    curl_slist_free_all(rcookies);
	
	return cookies;
}

static int curl_trace(CURL *curl, curl_infotype type, char *data, size_t size, void *opaque)
{
    const char *text = "";
    switch (type) {
        case CURLINFO_TEXT:         text = "== Info"; break;
        case CURLINFO_HEADER_IN:    text = "<= Recv headers"; break;
        case CURLINFO_HEADER_OUT:   text = "=> Send headers"; break;
        case CURLINFO_DATA_IN:      text = "<= Recv data"; break;
        case CURLINFO_DATA_OUT:     text = "=> Send data"; break;
        case CURLINFO_SSL_DATA_IN:  text = "<= Recv SSL data"; break;
        case CURLINFO_SSL_DATA_OUT: text = "=> Send SSL data"; break;
        default:
            RAWLOG_INFO1("!! Unknown type of curl trace: %d", (int)type);
            return 0;
    }
    
    String strData(data, size);
    RAWLOG_INFO3("%s (%d bytes): %s", text, size, strData.c_str());
    return 0;
}

curl_slist *CURLNetRequest::CURLHolder::set_options(const char *method, const String& strUrl, const String& strBody,
                             IRhoSession* pSession, Hashtable<String,String>* pHeaders)
{
    if (method != NULL) {
        mStrMethod = method;
    }
    else {
        mStrMethod = "NULL";
    }
    mStrUrl = strUrl;
    mStrBody = strBody;
    
    curl_easy_setopt(m_curl, CURLOPT_BUFFERSIZE, CURL_MAX_WRITE_SIZE-1);

    if (strcasecmp(method, "GET") == 0)
        curl_easy_setopt(m_curl, CURLOPT_HTTPGET, 1);
    else if (strcasecmp(method, "POST") == 0)
        curl_easy_setopt(m_curl, CURLOPT_POST, 1);
	else
        curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, method);

    curl_easy_setopt(m_curl, CURLOPT_URL, strUrl.c_str());
    
    // Just to enable cookie parser
    curl_easy_setopt(m_curl, CURLOPT_COOKIEFILE, "");
    // It will clear all stored cookies
    curl_easy_setopt(m_curl, CURLOPT_COOKIELIST, "ALL");
    String session;
    if (pSession)
        session = pSession->getSession();
    
    if (!session.empty()) {
        //RAWTRACE1("Set cookie: %s", session.c_str());
        curl_easy_setopt(m_curl, CURLOPT_COOKIE, session.c_str());
    }
    
    curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(m_curl, CURLOPT_TCP_NODELAY, 0); //enable Nagle algorithm
    
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, (long)m_sslVerifyPeer);
    
    // Set very large timeout in case of local requests
    // It is required because otherwise requests (especially requests to writing!)
    // could repeated twice or more times
    if (RHODESAPP().isBaseUrl(strUrl))
        curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, 10*24*60*60);
    
    curl_slist *hdrs = NULL;
    // Disable "Expect: 100-continue"
    hdrs = curl_slist_append(hdrs, "Expect:");
    // Add Keep-Alive header
    hdrs = curl_slist_append(hdrs, "Connection: Keep-Alive");

    if (strBody.size()>0) {
        curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, strBody.size());
        curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, strBody.c_str());
    }
    
    bool hasContentType = false;
    if (pHeaders)
    {
        for ( Hashtable<String,String>::iterator it = pHeaders->begin();  it != pHeaders->end(); ++it )
        {
            if (!hasContentType && strcasecmp(it->first.c_str(), "content-type") == 0)
                hasContentType = true;
            String strHeader = it->first + ": " + it->second;
            hdrs = curl_slist_append(hdrs, strHeader.c_str());
        }
    }
    
    if (!hasContentType && /*strcasecmp(method, "POST") == 0 &&*/ strBody.length() > 0)
    {
        String strHeader = "Content-Type: ";
        if ( pSession )
            strHeader += pSession->getContentType().c_str();
        else
            strHeader += "application/x-www-form-urlencoded";
        hdrs = curl_slist_append(hdrs, strHeader.c_str());
    }
    
    curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, hdrs);

    // Enable all available encodings (identity, deflate and gzip)
    curl_easy_setopt(m_curl, CURLOPT_ENCODING, "");
    curl_easy_setopt(m_curl, CURLOPT_HTTP_CONTENT_DECODING, (long)1);
    curl_easy_setopt(m_curl, CURLOPT_HTTP_TRANSFER_DECODING, (long)1);
    
    if (m_bTraceCalls) {
        curl_easy_setopt(m_curl, CURLOPT_DEBUGFUNCTION, &curl_trace);
        curl_easy_setopt(m_curl, CURLOPT_DEBUGDATA, NULL);
        curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);
    }
    
    return hdrs;
}

CURLNetRequest::CURLHolder::CURLHolder()
    :m_active(0)
{
    m_bTraceCalls = rho_conf_getBool("net_trace") && !rho_conf_getBool("log_skip_post");
    timeout = rho_conf_getInt("net_timeout");
    if (timeout == 0)
        timeout = 30; // 30 seconds by default
    m_sslVerifyPeer = true;
    
    m_curl = curl_easy_init();
    m_curlm = curl_multi_init();
    curl_easy_setopt(m_curl, CURLOPT_ERRORBUFFER, &errbuf);
}

CURLNetRequest::CURLHolder::~CURLHolder()
{
    curl_easy_cleanup(m_curl);
    curl_multi_cleanup(m_curlm);
}
    
void CURLNetRequest::CURLHolder::activate()
{
    common::CMutexLock guard(m_lock);
    if (m_active > 0)
        return;
    ++m_active;
    curl_multi_add_handle(m_curlm, m_curl);
}

void CURLNetRequest::CURLHolder::deactivate()
{
    common::CMutexLock guard(m_lock);
    if (m_active == 0)
        return;
    --m_active;
    curl_multi_remove_handle(m_curlm, m_curl);
}

CURLcode CURLNetRequest::CURLHolder::perform()
{
    activate();
    if ( !rho_conf_getBool("log_skip_post") )
        RAWTRACE3("   Activate CURLNetRequest: METHOD = [%s] URL = [%s] BODY = [%s]", mStrMethod.c_str(), mStrUrl.c_str(), mStrBody.c_str());
    else
        RAWTRACE1("   Activate CURLNetRequest: METHOD = [%s]", mStrMethod.c_str() );
    
    int const CHUNK = 1;
    
    long noactivity = 0;
    
    CURLcode result;
    for(;;)
    {
        common::CMutexLock guard(m_lock);
        if (m_active <= 0) {
            RAWLOG_INFO("CURLNetRequest: request was canceled from another thread !");
            if ( !rho_conf_getBool("log_skip_post") )
                RAWLOG_INFO3("   CURLNetRequest: METHOD = [%s] URL = [%s] BODY = [%s]", mStrMethod.c_str(), mStrUrl.c_str(), mStrBody.c_str());
            else
                RAWLOG_INFO1("   CURLNetRequest: METHOD = [%s]", mStrMethod.c_str());

            return CURLE_OBSOLETE4;   
        }
    
        int running;
        CURLMcode err = curl_multi_perform(m_curlm, &running);
        if (err == CURLM_CALL_MULTI_PERFORM)
            continue;
        if (err != CURLM_OK) {
            RAWLOG_ERROR1("curl_multi_perform error: %d", (int)err);
        }
        else {
            if (running > 0 && noactivity < timeout) {
                RAWTRACE("we still have active transfers but no data ready at this moment; waiting...");
                fd_set rfd, wfd, efd;
                int n = 0;
                FD_ZERO(&rfd);
                FD_ZERO(&wfd);
                FD_ZERO(&efd);
                err = curl_multi_fdset(m_curlm, &rfd, &wfd, &efd, &n);
                if (err == CURLM_OK) {
                    if (n > 0) {
                        timeval tv;
                        tv.tv_sec = CHUNK;
                        tv.tv_usec = 0;
                        int e = select(n + 1, &rfd, &wfd, &efd, &tv);
                        if (e < 0) {
                            RAWLOG_ERROR1("select (on curl handles) error: %d", errno);
                        }
                        else {
                            if (e == 0) {
                                RAWTRACE("No activity on sockets, check them again");
                                noactivity += CHUNK;
                            }
                            else noactivity = 0;
                            continue;
                        }
                    }
                }
                else {
                    RAWLOG_ERROR1("curl_multi_fdset error: %d", (int)err);
                }

            }
        }
        int nmsgs;
        CURLMsg *msg = curl_multi_info_read(m_curlm, &nmsgs);
        result = CURLE_OK;
        if (msg && msg->msg == CURLMSG_DONE)
            result = msg->data.result;
        if (result == CURLE_OK && noactivity >= timeout)
            result = CURLE_OPERATION_TIMEDOUT;
        if (result == CURLE_OK || result == CURLE_PARTIAL_FILE) {
            RAWTRACE2("Operation completed successfully with result %d: %s", (int)result, curl_easy_strerror(result));
        }
        else {
            RAWLOG_ERROR2("Operation finished with error %d: %s", (int)result, curl_easy_strerror(result));

            if ( !rho_conf_getBool("log_skip_post") )
                RAWLOG_ERROR3("  CURLNetRequest: METHOD = [%s] URL = [%s] BODY = [%s]", mStrMethod.c_str(), mStrUrl.c_str(), mStrBody.c_str());
            else
                RAWLOG_ERROR1("  CURLNetRequest: METHOD = [%s]", mStrMethod.c_str());
        }
        break;
    }

    if ( !rho_conf_getBool("log_skip_post") )
        RAWTRACE3("Deactivate CURLNetRequest: METHOD = [%s] URL = [%s] BODY = [%s]", mStrMethod.c_str(), mStrUrl.c_str(), mStrBody.c_str());
    else
        RAWTRACE1("Deactivate CURLNetRequest: METHOD = [%s]", mStrMethod.c_str() );

    deactivate();
    RAWTRACE("     Deactivation is DONE");
    return result;
}

} // namespace net
} // namespace rho

