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

#ifndef _RHOSTD_H_
#define _RHOSTD_H_

#include "RhoPort.h"
#define _GLIBCXX_CSTDLIB 1
namespace std
{
	using ::rand;	
}

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace rho {

typedef std::string String;
#ifdef OS_ANDROID
typedef std::string StringW;
#else
typedef std::wstring StringW;
#endif
typedef bool boolean;
//#define Vector std::vector

#define null 0

inline String String_trimRight(const String& str)
{
    // trim trailing spaces
    size_t endpos = str.find_last_not_of(" \t");
    if( String::npos != endpos )
        return str.substr( 0, endpos+1 );

    return str;
}

inline String String_trimLeft(const String& str)
{
    size_t startpos = str.find_first_not_of(" \t");
    if( String::npos != startpos )
        return str.substr( startpos );

    return str;
}

inline String String_trim(const String& str)
{
    String strRes = String_trimLeft(str);
    return String_trimRight(strRes);
}

inline boolean String_endsWith(const String& str, const char* szSuffix)
{
    if ( !szSuffix || !*szSuffix )
        return false;

    int nOff = str.length() - strlen(szSuffix);
    if ( nOff < 0 )
        return false;

    return strcmp(str.c_str()+nOff, szSuffix) == 0;
}

inline boolean String_endsWith(const String& str, const String& strSuffix)
{
    if ( strSuffix.length() == 0 )
        return false;

    int nOff = str.length() - strSuffix.length();
    if ( nOff < 0 )
        return false;

    return strcmp(str.c_str()+nOff, strSuffix.c_str()) == 0;
}

inline boolean String_startsWith(const String& str, const String& strPrefix)
{
    if ( strPrefix.length() == 0 )
        return false;

    if ( str.length() < strPrefix.length() )
        return false;

    return strncmp(str.c_str(), strPrefix.c_str(), strPrefix.length()) == 0;
}

template<typename StrType>
inline void String_replace(StrType& str, int from, int to)
{
    for( int i = 0; i < (int)str.length(); i++)
    {
        if ( str[i] == from )
            str[i] = to;
    }
}

template<typename StrType, typename CharType>
inline bool String_getline(const StrType& str, StrType& line, size_t& pos, CharType delim = ' ')
{
    bool res = pos != StrType::npos;
    if(res)
    {
        size_t end = str.find(delim, pos);
        line = str.substr(pos, end);
        pos = (end == StrType::npos) ? end : end +1;
    }
    return res;
}

template<class Type>
class Vector : public std::vector<Type>{
public:
    Vector(){}

    void addElement(const Type& item)
    {
        std::vector<Type>::push_back(item);
    }

    boolean isEmpty()
    {
        return std::vector<Type>::size()==0;
    }

    void setElementAt(const Type& item, typename std::vector<Type>::size_type i)
    {
        (*this)[i] = item;
    }

    void insertElementAt(const Type& item, typename std::vector<Type>::size_type i)
    {
        std::vector<Type>::insert( std::vector<Type>::begin() + i, item );
    }

    void removeElement(const Type& item)
    {
        for (typename std::vector<Type>::iterator it = Vector<Type>::begin();  it !=Vector<Type>::end();  )
        {
            if ( *it == item )
            {
                it = std::vector<Type>::erase(it);    
            }else
                it++;
        }
    }

    void removeElementAt(int nItem)
    {
        std::vector<Type>::erase( std::vector<Type>::begin() + nItem );
    }

    int indexOf(const Type& item)
    {
        for (int i = 0; i < (int)std::vector<Type>::size(); i++)
        {
            if ( std::vector<Type>::at(i) == item )
                return i;
        }

        return -1;
    }

    void removeAllElements()
    {
        Vector<Type>::clear();
    }

    typename std::vector<Type>::reference elementAt(typename std::vector<Type>::size_type i){ return std::vector<Type>::at(i);}
    typename std::vector<Type>::const_reference elementAt(typename std::vector<Type>::size_type i) const{ return std::vector<Type>::at(i); }
};

template<class Type>
class VectorPtr : public Vector<Type>{
public:
    VectorPtr(){}
    ~VectorPtr(){removeAllElements();}

    void removeAllElements()
    {
        for (typename std::vector<Type>::iterator it = Vector<Type>::begin();  it !=Vector<Type>::end(); ++it )
            delete *it;
        
        Vector<Type>::clear();
    }

    void removeElement(const Type& item)
    {
        for (typename std::vector<Type>::iterator it = Vector<Type>::begin();  it !=Vector<Type>::end();  )
        {
            if ( *it == item )
            {
                delete *it;
                it = std::vector<Type>::erase(it);    
            }else
                it++;
        }
    }

    void removeElementAt(int nItem, boolean bDelete = true)
    {
        if ( bDelete )
            delete Vector<Type>::at(nItem);

        Vector<Type>::erase( Vector<Type>::begin() + nItem );
    }
};

template<class Type>
class LinkedListPtr : public VectorPtr<Type>{
public:

    void add(const Type& item)
    {
        VectorPtr<Type>::addElement(item);
    }

    void addToFront(const Type& item)
    {
        this->insert(this->begin(), item);
    }

    Type removeFirst()
    {
        if ( VectorPtr<Type>::isEmpty() )
            return null;

        Type ptrFirst = VectorPtr<Type>::elementAt(0);
        VectorPtr<Type>::erase(VectorPtr<Type>::begin());

        return ptrFirst;
    }

    Type get(typename std::vector<Type>::size_type i)
    {
        return VectorPtr<Type>::elementAt(i);
    }

    void remove(int nItem)
    {
        VectorPtr<Type>::removeElementAt(nItem);
    }
};

template<class TKEY, class TVALUE>
class Hashtable : public std::map<TKEY,TVALUE>
{
public:
    Hashtable(){}

    void put(const TKEY& key, const TVALUE& val)
    {
        (*this)[key] = val;
    }

    void remove(const TKEY& key)
    {
        std::map<TKEY,TVALUE>::erase(key);
    }

    TVALUE get(const TKEY& key)
    {
        if ( containsKey(key) )
            return (*this)[key];

        return TVALUE();
    }

    boolean containsKey(const TKEY& key)
    {
        return !std::map<TKEY,TVALUE>::empty() && std::map<TKEY,TVALUE>::find(key) != std::map<TKEY,TVALUE>::end();
    }
};

template<class TKEY, class TVALUE>
class HashtablePtr : public Hashtable<TKEY,TVALUE>
{
public:
    ~HashtablePtr()
    {
        clear();
    }
    void clear()
    {
        for (typename std::map<TKEY,TVALUE>::iterator it = Hashtable<TKEY,TVALUE>::begin();  it != Hashtable<TKEY,TVALUE>::end(); ++it )
            delete it->second;
        
        Hashtable<TKEY,TVALUE>::clear();
    }

    void remove(const TKEY& key)
    {
        typename std::map<TKEY,TVALUE>::iterator it = Hashtable<TKEY,TVALUE>::find(key);
        if ( it != Hashtable<TKEY,TVALUE>::end() )
            delete it->second;

        Hashtable<TKEY,TVALUE>::erase(key);
    }

};

} 

#endif //_RHOSTD_H_
