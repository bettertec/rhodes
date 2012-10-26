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

#include "common/IRhoThreadImpl.h"

namespace rho {
namespace common {

//From boost
class CNonCopyable
{
 protected:
    CNonCopyable() {}
    ~CNonCopyable() {}
 private:  // emphasize the following members are private
    CNonCopyable( const CNonCopyable& );
    const CNonCopyable& operator=( const CNonCopyable& );
};

template <typename PTRTYPE>
class CBaseAutoPointer //: public CNonCopyable
{
public:
    CBaseAutoPointer() { m_ptr = 0; }

    PTRTYPE* operator &(){ Close(); return &m_ptr; }
    void Set( PTRTYPE ptr ){ Close(); m_ptr = ptr; }

    PTRTYPE operator ->(){ return m_ptr; }
    operator PTRTYPE(){ return m_ptr; }

    virtual void FreePtr() = 0;
    void Close()
    {
        if ( m_ptr ) 
        { 
            FreePtr(); 
            m_ptr = 0;
        }
    }

protected:
    PTRTYPE m_ptr;
};

template <typename TYPE, typename PTRTYPE=TYPE*>
class CAutoPtr : public CBaseAutoPointer<PTRTYPE>
{
public:
    CAutoPtr( PTRTYPE ptr ){   CBaseAutoPointer<PTRTYPE>::Set(ptr); }
    CAutoPtr(){}
    ~CAutoPtr(){ CBaseAutoPointer<PTRTYPE>::Close(); }

    CAutoPtr( const CAutoPtr& orig){ *this = orig; }
    CAutoPtr& operator=( const CAutoPtr& orig)
    {
        if ( CBaseAutoPointer<PTRTYPE>::m_ptr ) 
            FreePtr(); 

        CBaseAutoPointer<PTRTYPE>::m_ptr = orig.m_ptr;
        const_cast<CAutoPtr&>(orig).m_ptr = 0;
        return *this;
    }

    bool operator==(const CAutoPtr& orig)const{ return true;}

    //operator PTRTYPE(){ return m_ptr; }
    virtual void FreePtr()
    { 
        delete CBaseAutoPointer<PTRTYPE>::m_ptr;
    }
};

template <typename PTRTYPE, class BaseClass>
class CAutoPointer1 : public CBaseAutoPointer<PTRTYPE>, public BaseClass
{
public:
    ~CAutoPointer1(){ CBaseAutoPointer<PTRTYPE>::Close(); }

    virtual void FreePtr()
    { 
        (*((BaseClass*)this))(CBaseAutoPointer<PTRTYPE>::m_ptr); 
    }
};
/*
class CFunctor_NetApiBufferFree
{
public:
    void operator() ( void* pObject ){ NetApiBufferFree(pObject); }
};*/

template <typename PTRTYPE, typename FUNCTYPE, FUNCTYPE pFunc>
class CAutoPointer2 : public CBaseAutoPointer<PTRTYPE>
{
public:
    ~CAutoPointer2(){ CBaseAutoPointer<PTRTYPE>::Close(); }

    virtual void FreePtr()
    { 
        if(pFunc) 
            (*pFunc)(CBaseAutoPointer<PTRTYPE>::m_ptr); 
    }
};

template <typename PTRTYPE>
class CAutoPointer : public CBaseAutoPointer<PTRTYPE>
{
    struct CBaseDeleter{ virtual void Delete(PTRTYPE) = 0; };

    template <typename FUNCTYPE>
    struct CDeleter : public CBaseDeleter
    {
        CDeleter( FUNCTYPE pFunc ): m_pFunc(pFunc){;}
        void Delete(PTRTYPE ptr)
        { 
            m_pFunc(ptr); 
        }

        FUNCTYPE m_pFunc;
    };

public:

    template <typename FUNCTYPE>
    CAutoPointer( FUNCTYPE pFunc ) 
    { 
        static CDeleter<FUNCTYPE> oDeleter(pFunc);
        CBaseAutoPointer<PTRTYPE>::m_ptr = 0; 
        m_pDeleter = &oDeleter;
    }
    ~CAutoPointer(){ CBaseAutoPointer<PTRTYPE>::Close(); }

    virtual void FreePtr()
    { 
        if(m_pDeleter) 
            m_pDeleter->Delete(CBaseAutoPointer<PTRTYPE>::m_ptr); 
    }
private:
    CBaseDeleter* m_pDeleter;
};

template <typename FUNCTYPE, typename PARAMTYPE>
class CStaticClassFunctor : public rho::common::IRhoRunnable
{
public:

    CStaticClassFunctor( FUNCTYPE pFunc, PARAMTYPE param ): m_pFunc(pFunc), m_param(param){}
    virtual void runObject()
    { 
        m_pFunc(m_param); 
    }

    ~CStaticClassFunctor()
    {
    }

private:
    FUNCTYPE m_pFunc;
    PARAMTYPE m_param;

};

}
}
