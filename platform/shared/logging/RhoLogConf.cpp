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

#include "RhoLogConf.h"
#include "RhoLogCat.h"
#include "RhoLogSink.h"
#include "common/RhoFile.h"
#include "common/RhoFilePath.h"
#include "common/RhoConf.h"
#include "common/Tokenizer.h"
#include "common/app_build_capabilities.h"

#ifndef RHO_NO_RUBY
#include "ruby/ext/rho/rhoruby.h"
#endif //RHO_NO_RUBY

namespace rho{
common::CMutex LogSettings::m_FlushLock;
common::CMutex LogSettings::m_CatLock;


LogSettings::MemoryInfoCollectorThread::MemoryInfoCollectorThread( LogSettings& logSettings ) :
                 m_collectMemoryIntervalMilliseconds(0), m_pCollector(0), m_logSettings(logSettings)
{
    
}

void LogSettings::MemoryInfoCollectorThread::run()
{
    while( !isStopping() )    
    {
        unsigned int toWait = 0;
        {
            common::CMutexLock lock(m_accessLock);
            toWait = m_collectMemoryIntervalMilliseconds;
        }
        
        if ( 0 == toWait )
        {
            continue;   
        }
        
        wait(toWait / 1000);

        if (!isStopping())
        {
            common::CMutexLock lock(m_accessLock);
            if ( m_pCollector!=0 )
            {
                String str = m_pCollector->collect();

                LogCategory oLogCat("MEMORY");
                rho::LogMessage oLogMsg(__FILE__, __LINE__, L_INFO, LOGCONF(), oLogCat, true );
                oLogMsg + str;

                //m_logSettings.sinkLogMessage( str );
            }            
        }        
    }
}

void LogSettings::MemoryInfoCollectorThread::setCollectMemoryInfoInterval( unsigned int interval )
{
    common::CMutexLock lock(m_accessLock);
    m_collectMemoryIntervalMilliseconds = interval;
}

void LogSettings::MemoryInfoCollectorThread::setMemoryInfoCollector( IMemoryInfoCollector* memInfoCollector )
{
    common::CMutexLock lock(m_accessLock); 
    m_pCollector = memInfoCollector;   
}

boolean LogSettings::MemoryInfoCollectorThread::willCollect() const
{
    common::CMutexLock lock(m_accessLock); 
    return (m_collectMemoryIntervalMilliseconds>0) && (m_pCollector!=0);
}


LogSettings g_LogSettings;

LogSettings::LogSettings()
{ 
    m_nMinSeverity = 0; 
    m_bLogToOutput = true; 
    m_bLogToFile = false;
    m_bLogToSocket = false;

    m_nMaxLogFileSize = 0; 
    m_bLogPrefix = true; 

    m_strLogURL = "";

    m_pFileSink = new CLogFileSink(*this);
    m_pOutputSink = new CLogOutputSink(*this);
    m_pLogViewSink = NULL;
	m_pSocketSink = NULL;
	m_pMemoryInfoCollector = NULL;
    m_pMemoryCollectorThread = NULL;
}

LogSettings::~LogSettings(){
    
    if ( m_pMemoryCollectorThread != 0 )
    {
        m_pMemoryCollectorThread->stop(1);
        delete m_pMemoryCollectorThread;
    }
    delete m_pFileSink;
    delete m_pOutputSink;
	if(m_pSocketSink)
		delete m_pSocketSink;
}

void LogSettings::closeRemoteLog()
{
	if(m_pSocketSink)
	{
		delete m_pSocketSink;
	    m_pSocketSink = 0;
	}
}

void LogSettings::initRemoteLog()
{

#if defined( OS_PLATFORM_MOTCE )// && !defined (APP_BUILD_CAPABILITY_BARCODE)
    //TODO: remote log prevent loading app - stuck on loading.png when no barcode. very strange!
    OSVERSIONINFO osv = {0};
	osv.dwOSVersionInfoSize = sizeof(osv);
	if (GetVersionEx(&osv) && osv.dwMajorVersion == 5)
		return;
#endif

	m_strLogURL = RHOCONF().getString("rhologurl"); 

	if(!m_pSocketSink && m_strLogURL != "")
		m_pSocketSink = new CLogSocketSink(*this);
}

void LogSettings::reinitRemoteLog() {
    closeRemoteLog();
	if(!m_pSocketSink && m_strLogURL != "")
		m_pSocketSink = new CLogSocketSink(*this);
}


void LogSettings::getLogTextW(StringW& strTextW)
{
    boolean bOldSaveToFile = isLogToFile();
    setLogToFile(false);

    common::CRhoFile oFile;
    if ( oFile.open( getLogFilePath().c_str(), common::CRhoFile::OpenReadOnly) )
        oFile.readStringW(strTextW);

    setLogToFile(bOldSaveToFile);
}

void LogSettings::getLogText(String& strText)
{
    boolean bOldSaveToFile = isLogToFile();
    setLogToFile(false);

    common::CRhoFile oFile;
    if ( oFile.open( getLogFilePath().c_str(), common::CRhoFile::OpenReadOnly) )
        oFile.readString(strText);

    setLogToFile(bOldSaveToFile);
}

int LogSettings::getLogTextPos()
{
    return m_pFileSink ? m_pFileSink->getCurPos() : -1;
}

void LogSettings::saveToFile(){
    RHOCONF().setInt("MinSeverity", getMinSeverity(), true );
    RHOCONF().setBool("LogToOutput", isLogToOutput(), true );
    RHOCONF().setBool("LogToFile", isLogToFile(), true );
#if !defined(OS_MACOSX) 
    RHOCONF().setString("LogFilePath", getLogFilePath(), true );
#endif
    RHOCONF().setInt("MaxLogFileSize", getMaxLogFileSize(), true );
    RHOCONF().setString("LogCategories", getEnabledCategories(), true );
    RHOCONF().setString("ExcludeLogCategories", getDisabledCategories(), true );
}

void LogSettings::loadFromConf(rho::common::RhoSettings& oRhoConf)
{
    if ( oRhoConf.isExist( "MinSeverity" ) )
        setMinSeverity( oRhoConf.getInt("MinSeverity") );
    if ( oRhoConf.isExist( "LogToOutput") )
        setLogToOutput( oRhoConf.getBool("LogToOutput") );
    if ( oRhoConf.isExist( "LogToFile") )
        setLogToFile( oRhoConf.getBool("LogToFile"));
    if ( oRhoConf.isExist( "LogFilePath") )
        setLogFilePath( oRhoConf.getString("LogFilePath").c_str() );
    if ( oRhoConf.isExist( "MaxLogFileSize") )
        setMaxLogFileSize( oRhoConf.getInt("MaxLogFileSize") );
    if ( oRhoConf.isExist( "LogCategories") )
        setEnabledCategories( oRhoConf.getString("LogCategories").c_str() );
    if (oRhoConf.isExist( "ExcludeLogCategories") )
        setDisabledCategories( oRhoConf.getString("ExcludeLogCategories").c_str() );
	if ( oRhoConf.isExist( "LogToSocket") )
		setLogToSocket( oRhoConf.getBool("LogToSocket") );
	if ( oRhoConf.isExist( "log_exclude_filter") )
        setExcludeFilter( oRhoConf.getString("log_exclude_filter") );
	if ( oRhoConf.isExist( "LogMemPeriod" ) )
	{
		int milliseconds = oRhoConf.getInt("LogMemPeriod");
		setCollectMemoryInfoInterval(milliseconds);
	}
}

void LogSettings::setLogFilePath(const String& logFilePath){
    if ( m_strLogFilePath.compare(logFilePath) != 0 ){
        common::CMutexLock oLock(m_FlushLock);

        m_strLogFilePath = logFilePath;
        if ( m_pFileSink ){
            delete m_pFileSink;
            m_pFileSink = new CLogFileSink(*this);
        }
    }
}

void LogSettings::clearLog(){
    common::CMutexLock oLock(m_FlushLock);

    if ( m_pFileSink ){
        m_pFileSink->clear();
    }

}

void LogSettings::sinkLogMessage( String& strMsg ){
    /*
	String logMemory ("");
    processMemoryInfo( logMemory );
	if (logMemory.length() > 0)
		internalSinkLogMessage(logMemory);
    */

	internalSinkLogMessage(strMsg);
}

void LogSettings::internalSinkLogMessage( String& strMsg ){
    common::CMutexLock oLock(m_FlushLock);

	if ( isLogToFile() )
        m_pFileSink->writeLogMessage(strMsg);

    if (m_pLogViewSink)
        m_pLogViewSink->writeLogMessage(strMsg);

    //Should be at the end
    if ( isLogToOutput() )
        m_pOutputSink->writeLogMessage(strMsg);

	if (m_pSocketSink)
        m_pSocketSink->writeLogMessage(strMsg);
}

bool LogSettings::isCategoryEnabled(const LogCategory& cat)const{
    //TODO: Optimize categories search : add map
    common::CMutexLock oLock(m_CatLock);

    if ( m_strDisabledCategories.length() > 0 && strstr(m_strDisabledCategories.c_str(), cat.getName().c_str() ) != 0 )
        return false;

    if ( m_strEnabledCategories.length() == 0 )
        return false;

    return strcmp(m_strEnabledCategories.c_str(),"*") == 0 || strstr(m_strEnabledCategories.c_str(), cat.getName().c_str() ) != 0;
}

void LogSettings::setEnabledCategories( const char* szCatList ){
    common::CMutexLock oLock(m_CatLock);

    if ( szCatList && *szCatList )
    	m_strEnabledCategories = szCatList;
    else
    	m_strEnabledCategories = "";
}

void LogSettings::setDisabledCategories( const char* szCatList ){
    common::CMutexLock oLock(m_CatLock);

    if ( szCatList && *szCatList )
    	m_strDisabledCategories = szCatList;
    else
    	m_strDisabledCategories = "";
}

void LogSettings::setExcludeFilter( const String& strExcludeFilter )
{
    if ( strExcludeFilter.length() > 0 )
    {
        rho::common::CTokenizer oTokenizer( strExcludeFilter, "," );
	    while (oTokenizer.hasMoreTokens()) 
        {
            String tok = rho::String_trim(oTokenizer.nextToken());
		    if (tok.length() == 0)
			    continue;

            //m_arExcludeAttribs.addElement( "\"" + tok + "\"=>\"" );
            m_arExcludeAttribs.addElement( tok );
        } 
    }
    else
    	m_arExcludeAttribs.removeAllElements();
}

void LogSettings::setCollectMemoryInfoInterval( unsigned int interval )
{ 
    if ( 0 == m_pMemoryCollectorThread )
    {
        m_pMemoryCollectorThread = new MemoryInfoCollectorThread(*this);
    }
    
    m_pMemoryCollectorThread->setCollectMemoryInfoInterval(interval);
    if ( m_pMemoryCollectorThread->willCollect() )
    {        
        m_pMemoryCollectorThread->start(common::IRhoRunnable::epLow);        
    } 
    else 
    {
        m_pMemoryCollectorThread->stop(0);        
    }
}

void LogSettings::setMemoryInfoCollector( IMemoryInfoCollector* memInfoCollector ) 
{
    if ( 0 == m_pMemoryCollectorThread )
    {
        m_pMemoryCollectorThread = new MemoryInfoCollectorThread(*this);
    }
    
    m_pMemoryCollectorThread->setMemoryInfoCollector(memInfoCollector);
    if ( m_pMemoryCollectorThread->willCollect() )
    {
        m_pMemoryCollectorThread->start(common::IRhoRunnable::epLow);        
    } 
    else 
    {
        m_pMemoryCollectorThread->stop(0);        
    }
}

}

extern "C" {
using namespace rho;
using namespace rho::common;

void rho_logconf_Init_with_separate_user_path(const char* szLogPath, const char* szRootPath, const char* szLogPort, const char* szUserPath)
{

#ifdef RHODES_EMULATOR
    String strRootPath = szLogPath;
    strRootPath += RHO_EMULATOR_DIR"/";
    rho::common::CFilePath oLogPath( strRootPath );
#else
    rho::common::CFilePath oLogPath( szLogPath );
#endif

    //Set defaults
#ifdef RHO_DEBUG
    LOGCONF().setMinSeverity( L_TRACE );
    LOGCONF().setLogToOutput(true);
    LOGCONF().setEnabledCategories("*");
    LOGCONF().setDisabledCategories("");
#else //!RHO_DEBUG
    LOGCONF().setMinSeverity( L_ERROR );
    LOGCONF().setLogToOutput(false);
    LOGCONF().setEnabledCategories("");
#endif//!RHO_DEBUG

    LOGCONF().setLogPrefix(true);

    rho::String logPath = oLogPath.makeFullPath("rholog.txt");
    LOGCONF().setLogToFile(true);
    LOGCONF().setLogFilePath( logPath.c_str() );
    LOGCONF().setMaxLogFileSize(1024*50);

    rho_conf_Init_with_separate_user_path(szRootPath, szUserPath);

    LOGCONF().loadFromConf(RHOCONF());
}

void rho_logconf_Init(const char* szLogPath, const char* szRootPath, const char* szLogPort){
    rho_logconf_Init_with_separate_user_path(szLogPath, szRootPath, szLogPort, szRootPath);
}

char* rho_logconf_getText() {
    rho::String strText;
    LOGCONF().getLogText(strText);
	return strdup(strText.c_str());
}

int rho_logconf_getTextPos() {
	return LOGCONF().getLogTextPos();
}

char* rho_logconf_getEnabledCategories() {
	return strdup(LOGCONF().getEnabledCategories().c_str());
}

char* rho_logconf_getDisabledCategories() {
	return strdup(LOGCONF().getDisabledCategories().c_str());
}

int rho_logconf_getSeverity() {
	return LOGCONF().getMinSeverity();
}

void rho_logconf_setEnabledCategories(const char* categories) {
	LOGCONF().setEnabledCategories(categories);
}

void rho_logconf_setDisabledCategories(const char* categories) {
	LOGCONF().setDisabledCategories(categories);
}

void rho_logconf_setSeverity(int nLevel) {
	LOGCONF().setMinSeverity(nLevel);
}

void rho_logconf_saveSettings() {
	 LOGCONF().saveToFile();
}

void rho_logconf_freeString(char* str) {
	free(str);
}

// RhoConf.set_property_by_name
void rho_conf_set_property_by_name(char* name, char* value)
{
	RHOCONF().setString(name, value, true);

    LOGCONF().loadFromConf(RHOCONF());
}

void rho_conf_clean_log()
{
    LOGCONF().clearLog();
}

#ifndef RHO_NO_RUBY
VALUE rho_conf_get_property_by_name(char* name)
{
    return rho_ruby_create_string(RHOCONF().getString(name).c_str());
}

VALUE rho_conf_get_conflicts()
{
    CHoldRubyValue hashConflicts(rho_ruby_createHash());

    HashtablePtr<String,Vector<String>* >& mapConflicts = RHOCONF().getConflicts();
    for ( HashtablePtr<String,Vector<String>* >::iterator it=mapConflicts.begin() ; it != mapConflicts.end(); it++ ) 
    {
        Vector<String>& values = *(it->second);
        CHoldRubyValue arValues(rho_ruby_create_array());
        for( int i = 0; i < (int)values.size(); i++)
            rho_ruby_add_to_array(arValues, rho_ruby_create_string(values.elementAt(i).c_str()) );

        addHashToHash(hashConflicts, it->first.c_str(), arValues);
    }

    return hashConflicts;
}

VALUE rho_conf_read_log(int limit)
{
    VALUE res = rho_ruby_create_string("");
    bool bOldSaveToFile = LOGCONF().isLogToFile();
    LOGCONF().setLogToFile(false);

    rho::common::CRhoFile oFile;
    if ( oFile.open( LOGCONF().getLogFilePath().c_str(), rho::common::CRhoFile::OpenReadOnly) )
    {
        int nFileSize = oFile.size();
        int nPos = LOGCONF().getLogTextPos();
        int nMaxSize = nFileSize > nPos ? nFileSize : nPos;
        if ( limit <= 0 || limit > nMaxSize)
            limit = nMaxSize;

        res = rho_ruby_create_string_withlen(limit);
        char* szStr = getStringFromValue(res);

        if ( limit <= nPos )
        {
            oFile.setPosTo(nPos-limit);
            oFile.readData(szStr,0,limit);
        }else
        {
            oFile.setPosTo(nFileSize-(limit-nPos));
            int nRead = oFile.readData(szStr,0,limit);

            oFile.setPosTo(0);
            oFile.readData(szStr,nRead,limit-nRead);
        }

    }

    LOGCONF().setLogToFile(bOldSaveToFile);

    return res;
}


void rho_log_resetup_http_url(const char* http_log_url) {
    LOGCONF().setLogURL(http_log_url);
    LOGCONF().reinitRemoteLog();
}

#endif //RHO_NO_RUBY

}
