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

#include "common/RhoStd.h"
#include "common/AutoPointer.h"
#include "common/RhoMutexLock.h"
#include "sqlite/sqlite3.h"

namespace rho{
namespace db{

class CDBAdapter;
	
class CDBError
{
	int m_rc;
	String m_strError;
		
public:
	CDBError() : m_rc(SQLITE_OK){}
	
	void setError( int rc, const char* szError){ m_rc = rc; m_strError = szError ? szError : ""; }
	boolean isOK(){ return m_rc == SQLITE_OK; }
	boolean isSuccessfulCode(){return m_rc == SQLITE_OK || m_rc == SQLITE_ROW || m_rc == SQLITE_DONE; }
		
	int getCode(){ return m_rc;}
	const String& getError(){ return m_strError;}
};	
	
class CDBResult
{
    CDBAdapter* m_pDB;
    sqlite3_stmt* m_dbStatement;
    boolean m_bReportNonUnique;
    CDBError m_dbError;
	
public:
    CDBResult(sqlite3_stmt* st,CDBAdapter* pDB);
    CDBResult();
    ~CDBResult(void);

    void setStatement(sqlite3_stmt* st);
    sqlite3_stmt* getStatement(){ return m_dbStatement; }
    boolean getReportNonUnique(){ return m_bReportNonUnique; }
    void setReportNonUnique(boolean bSet){ m_bReportNonUnique = bSet; }
    boolean isNonUnique(){ return m_dbError.getCode()==SQLITE_CONSTRAINT; }
	
	CDBError& getDBError(){ return m_dbError; }
//    boolean isError(){ return m_dbError.getCode()!=SQLITE_OK; }
//    int     getErrorCode(){ return m_dbError.getCode(); }
//    void    setErrorCode(int nError){ m_nErrorCode=nError; }

    virtual bool isEnd(){ return m_dbStatement == null; }
    virtual bool isOneEnd(){ return m_dbStatement == null; }
    void next()
    {
        if ( sqlite3_step(m_dbStatement) != SQLITE_ROW )
            setStatement(null);
    }

    virtual String getStringByIdx(int nCol)
    {
        char* res = (char *)sqlite3_column_text(m_dbStatement, nCol);
        return res ? res : String();
    }

    int getIntByIdx(int nCol)
    {
        return sqlite3_column_int(m_dbStatement, nCol);
    }

    uint64 getUInt64ByIdx(int nCol)
    {
        return sqlite3_column_int64(m_dbStatement, nCol);
    }

    int getColCount()
    {
        return sqlite3_data_count(m_dbStatement);
    }

    boolean isNullByIdx(int nCol)
    {
        return sqlite3_column_type(m_dbStatement,nCol) == SQLITE_NULL;
    }

    String getColName(int nCol)
    {
        return sqlite3_column_name(m_dbStatement,nCol);
    }

};

typedef rho::common::CAutoPtr<rho::db::CDBResult> DBResultPtr;
/*
#define DBResult(name, call)\
    rho::db::DBResultPtr p##name = call;\
    rho::db::CDBResult& name = *p##name;
*/

class CDBResultWrapper
{
    DBResultPtr m_dbRes;
public:
    CDBResultWrapper(const DBResultPtr& dbRes) : m_dbRes(dbRes){}
    void operator=( const DBResultPtr& dbRes){ m_dbRes = dbRes; }

    sqlite3_stmt* getStatement(){ return m_dbRes->getStatement(); }
    boolean isNonUnique(){ return m_dbRes->isNonUnique(); }
    bool isEnd(){ return m_dbRes->isEnd(); }
    bool isOneEnd(){ return m_dbRes->isOneEnd(); }
    void next(){ m_dbRes->next(); }
    String getStringByIdx(int nCol){ return m_dbRes->getStringByIdx(nCol); }
    int getIntByIdx(int nCol){ return m_dbRes->getIntByIdx(nCol); }
    uint64 getUInt64ByIdx(int nCol){ return m_dbRes->getUInt64ByIdx(nCol); }
    int getColCount(){ return m_dbRes->getColCount(); }
    boolean isNullByIdx(int nCol){ return m_dbRes->isNullByIdx(nCol); }
    String getColName(int nCol){ return m_dbRes->getColName(nCol); }

};

}
}

typedef rho::db::CDBResultWrapper IDBResult;


