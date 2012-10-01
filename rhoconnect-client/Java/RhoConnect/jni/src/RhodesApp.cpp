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

#include "rhodes/JNIRhodes.h"
#include "common/RhoStd.h"
#include "common/RhodesApp.h"


namespace rho {
namespace common {

//boolean CRhodesApp::isBaseUrl(const String& strUrl)
//{
//    return String_startsWith(strUrl, getBaseUrl());
//}

const String& CRhodesApp::getBaseUrl()
{
    return rho_root_path();
}
	
void CRhodesApp::stopApp()
{
}

}}
/*
RHO_GLOBAL int rho_conf_send_log(const char* callback)
{
    return 0;//RHODESAPP().sendLog();
}
*/

RHO_GLOBAL int rho_sys_zip_files_with_path_array_ptr(const char* szZipFilePath, const char* base_path, void* ptrFilesArray, const char* psw)
{
return -1;
}

RHO_GLOBAL void rho_rhodesapp_callAppActiveCallback( int nActive )
{
}

RHO_GLOBAL void rho_rhodesapp_callUiCreatedCallback()
{
}

RHO_GLOBAL void rho_rhodesapp_callUiDestroyedCallback()
{
}

RHO_GLOBAL char* rho_http_normalizeurl(const char* szUrl)
{
	return 0;
}

RHO_GLOBAL void rho_http_free(void* data)
{
	
}

RHO_GLOBAL void rho_platform_restart_application()
{
	
}

RHO_GLOBAL void rho_sys_app_exit()
{
	
}