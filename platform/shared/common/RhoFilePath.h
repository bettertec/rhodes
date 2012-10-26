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

#ifndef _RHOFILEPATH_H_
#define _RHOFILEPATH_H_

#include "RhoStd.h"

namespace rho{
namespace common{

class CFilePath{
public:
    CFilePath(const char* path) : m_szPath(path){}
    CFilePath(const String& str) : m_szPath(str.c_str()){}

    const char* getBaseName(){ 
        const char* base = findLastSlash();
        if (base)
            return base+1;

        return m_szPath;
    }

    String getFolderName(){ 
        const char* base = findLastSlash();
        if (base)
        {
            String strRes = m_szPath;
            return strRes.substr(0, base-m_szPath);
        }

        return m_szPath;
    }

    String makeFullPath(const char* szFileName){
        String res = m_szPath;
        if (res.length() > 0)
        {
            const char* pSlash = findLastSlash();
            if (!pSlash || (pSlash != (res.c_str() + res.length())))
                res += "/";
        }

        res += szFileName;

        return res;
    }

    String changeBaseName( const String& strFileName )
    {
        return changeBaseName(strFileName.c_str());
    }

    String changeBaseName( const char* szFileName )
    {
        const char* base = findLastSlash();
        if ( base && *(base+1) ){
            String res( m_szPath, base-m_szPath+1);
            res += szFileName;

            return res;
        }

        return makeFullPath(szFileName);
    }

    static String join(const String& path1, const String& path2)
    {
        boolean bSlash1 = path1.length()>0 && (path1[path1.length()-1] == '/' || path1[path1.length()-1] == '\\');
        boolean bSlash2 = path2.length()>0 && (path2[0] == '/' || path2[0] == '\\');
        String res;
        if (bSlash1 && bSlash2)
            res = path1 + path2.substr(1);
        else if ( bSlash1 || bSlash2 )
            res = path1 + path2;
        else
            res = path1 + '/' + path2;

        return res;
    }
#if defined(WINDOWS_PLATFORM)
    static StringW join(const StringW& path1, const StringW& path2)
    {
        boolean bSlash1 = path1.length()>0 && (path1[path1.length()-1] == L'/' || path1[path1.length()-1] == L'\\');
        boolean bSlash2 = path2.length()>0 && (path2[0] == L'/' || path2[0] == L'\\');
        StringW res;
        if (bSlash1 && bSlash2)
            res = path1 + path2.substr(1);
        else if ( bSlash1 || bSlash2 )
            res = path1 + path2;
        else
        {
            res = path1 + L"/" + path2;
        }

        return res;
    }
#endif

    static String normalizePath(const String& path1)
    {
        String path = path1;
        rho::String::size_type pos = 0;
        while ( (pos = path.find('\\', pos)) != rho::String::npos ) {
            path.replace( pos, 1, "/");
            pos++;
        }

        return path;
    }

    static boolean isEqualBaseNames(const String& path1, const String& path2)
    {
        CFilePath oPath1(path1);
        CFilePath oPath2(path2);

        return strcasecmp(oPath1.getBaseName(), oPath2.getBaseName()) == 0;
    }

    static String getRelativePath( const String& path1, const String& path2)
    {
        if ( !String_startsWith(path1, path2) )
            return path1;

        return path1.substr(path2.length());
    }

private:

    const char* findLastSlash(){
        const char* slash = strrchr(m_szPath, '/');
        if ( !slash )
            slash = strrchr(m_szPath, '\\');

        return slash;
    }

    const char* m_szPath;
};

}
}

#endif //_RHOFILEPATH_H_
