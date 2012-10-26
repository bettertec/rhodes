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

#ifndef _RHOSTRINGCONV_H_
#define _RHOSTRINGCONV_H_

namespace rho{
namespace common{
/*
#ifdef  _UNICODE
#define convertFromString convertFromStringW
#define convertToString convertToStringW
#else
#define convertFromString convertFromStringA
#define convertToString convertToStringA
#endif
*/
template<typename T>  inline void convertFromStringW( const wchar_t* szValue, T& value ){ value = szValue; }
template<typename T>  inline StringW convertToStringW( const T& value ){ return value; }


template<typename T>  inline void convertFromStringA( const char* szValue, T& value ){ value = szValue; }
template<typename T>  inline String convertToStringA( const T& value ){ return value; }

inline String convertToStringA( const wchar_t* szValue )
{ 
    String res;
    if ( !szValue || !*szValue )
        return res;

#if defined(WINDOWS_PLATFORM)
    int nSize = WideCharToMultiByte(CP_UTF8, 0, szValue, -1, NULL, 0, NULL, NULL);
    if ( nSize > 1 )
    {
        res.resize(nSize-1);
        WideCharToMultiByte(CP_UTF8, 0, szValue, -1, &res[0], nSize, NULL, NULL);
    }
#else
    int nSize = wcstombs( NULL, szValue, 0 );
    if ( nSize >= 0 )
    {
        res.resize(nSize);
        int nRes = wcstombs( &res[0], szValue, nSize );
        if ( nRes >= 0 ){
            res[nRes] = 0;
        }
    }
#endif
    return res;
}

inline String convertToStringA( wchar_t* szValue ){ 
    return convertToStringA( (const wchar_t*)szValue );
}

/*inline String convertToStringA( const char* szValue ){ 
	return String(szValue);
}*/
	
template<>  inline String convertToStringA<StringW>( const StringW& strValue ){
    return convertToStringA(strValue.c_str());
}

inline void convertToStringW( const char* szValue, StringW& res )
{ 
    if ( !szValue || !*szValue )
        return;

#ifdef OS_ANDROID
	res = szValue;
#else

#if defined(WINDOWS_PLATFORM)
    int nSize = MultiByteToWideChar(CP_UTF8, 0, szValue, -1, NULL, 0);
    if ( nSize > 1 )
    {
        res.resize(nSize-1);
        MultiByteToWideChar(CP_UTF8, 0, szValue, -1, &res[0], nSize);
    }
#else
    int nSize =mbstowcs( NULL, szValue, 0 );
    if ( nSize >= 0 )
    {
        res.resize(nSize);
        int nRes = mbstowcs( &res[0], szValue, nSize );
        if ( nRes >= 0 ){
            res[nRes] = 0;
        }
    }
#endif
#endif
}

inline StringW convertToStringW( const String& str )
{ 
    StringW res;
    convertToStringW(str.c_str(), res);
    return res;
}

/*template<>  inline void convertFromStringA<StringW>( const char* szValue, StringW& value ){ value = StringW(szValue); }
template<>  inline String convertToStringA<StringW>( const StringW& value ){ return String(value); }
*/
#define CONVERT_TYPE_W( Type, Fmt ) \
template<> inline void convertFromStringW<Type>( const wchar_t* szValue, Type& value )\
{ swscanf( szValue, Fmt, &value ); }\
template<> inline StringW convertToStringW<Type>( const Type& value )\
{ \
    wchar_t buf[100];\
    swprintf( buf, Fmt, value );\
    return StringW(buf);\
}

#define CONVERT_TYPE_A( Type, Fmt ) \
template<> inline void convertFromStringA<Type>( const char* szValue, Type& value )\
{ sscanf( szValue, Fmt, &value ); }\
template<> inline String convertToStringA<Type>( const Type& value )\
{ \
    char buf[100];\
    sprintf( buf, Fmt, value );\
    return String(buf);\
}

#if defined(WINDOWS_PLATFORM)
CONVERT_TYPE_W( unsigned int, L"%u" );
CONVERT_TYPE_W( int, L"%d" );
CONVERT_TYPE_W( unsigned long, L"%lu" );
CONVERT_TYPE_W( long, L"%d" );
CONVERT_TYPE_W( wchar_t, L"%lc" );
CONVERT_TYPE_W( char, L"%c" );
CONVERT_TYPE_W( unsigned char, L"%c" );
CONVERT_TYPE_W( short, L"%hd" );
//CONVERT_TYPE_W( unsigned short, _T("%u") );
CONVERT_TYPE_W( bool, L"%d" );
CONVERT_TYPE_W( float, L"%f" );
CONVERT_TYPE_W( double, L"%lf" );

#endif
CONVERT_TYPE_A( unsigned int, "%u" );
CONVERT_TYPE_A( int, "%d" );
CONVERT_TYPE_A( unsigned long, "%lu" );
CONVERT_TYPE_A( long, "%ld" );
CONVERT_TYPE_A( wchar_t, "%lc" );
CONVERT_TYPE_A( char, "%c" );
CONVERT_TYPE_A( unsigned char, "%c" );
CONVERT_TYPE_A( short, "%hd" );
//CONVERT_TYPE_A( unsigned short, "%u" );
//CONVERT_TYPE_A( bool, "%d" );
CONVERT_TYPE_A( float, "%f" );
CONVERT_TYPE_A( double, "%lf" );

CONVERT_TYPE_A( uint64, FMTU64 );
CONVERT_TYPE_A( int64, FMTI64 );

// Special case for bool
template<> inline void convertFromStringA<bool>( const char* szValue, bool& value )
{ char c; sscanf( szValue, "%c", &c ); value = c ? true:false; }
template<> inline String convertToStringA<bool>( const bool& value )
{
    char buf[100];
    sprintf( buf, "%c", (char)value );
    return String(buf);
}


}
}

#endif //_RHOSTRINGCONV_H_
