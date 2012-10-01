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

package com.rho;

import java.io.InputStream;

import com.rho.RhoEmptyLogger;
import com.rho.RhoLogger;
import com.xruby.runtime.lang.*;
import com.xruby.runtime.builtin.*;

import java.io.IOException;
import com.rho.db.DBAdapter;
import com.rho.sync.SyncThread;
import com.rho.net.AsyncHttp;
import com.rho.Properties;
//import net.rim.device.api.system.CodeModuleManager;
import java.util.Hashtable;
import java.util.Vector;
import com.rho.rjson.RJSONTokener;
import com.rho.net.NetResponse;

public class RhoRuby {

	private static final RhoLogger LOG = RhoLogger.RHO_STRIP_LOG ? new RhoEmptyLogger() : 
		new RhoLogger("RhoRuby");
	private static final RhoProfiler PROF = RhoProfiler.RHO_STRIP_PROFILER ? new RhoEmptyProfiler() : 
		new RhoProfiler();
	
	public static final RubyID serveID = RubyID.intern("serve_hash");
	public static final RubyID serveIndexID = RubyID.intern("serve_index_hash");
	public static final RubyID raiseRhoError = RubyID.intern("raise_rhoerror");
	public static final RubyID initApp = RubyID.intern("init_app");
	public static final RubyID onConfigConflicts_mid = RubyID.intern("on_config_conflicts");
	public static final RubyID uiCreated = RubyID.intern("ui_created");
	public static final RubyID uiDestroyed = RubyID.intern("ui_destroyed");
	public static final RubyID activateApp = RubyID.intern("activate_app");
	public static final RubyID deactivateApp = RubyID.intern("deactivate_app");
	
//	public static final RubyID getStartPath = RubyID.intern("get_start_path");
//	public static final RubyID getOptionsPath = RubyID.intern("get_options_path");
	
	static RubyValue receiver;
	static RubyProgram mainObj;
	
	public static void RhoRubyStart(String szAppPath)throws Exception
	{
		String[] args = new String[0];

		IRhoRubyHelper helper = null;
		RubyRuntime.init(args);

        DBAdapter.initMethods(RubyRuntime.DatabaseClass);
        SyncThread.initMethods(RubyRuntime.SyncEngineClass);
        AsyncHttp.initMethods(RubyRuntime.AsyncHttpModule);
        RJSONTokener.initMethods(RubyRuntime.JSONClass);
        
        helper = RhoClassFactory.createRhoRubyHelper();
        helper.initRubyExtensions();
        
		//TODO: implement recursive dir creation
		RhoClassFactory.createFile().getDirPath("apps");
		RhoClassFactory.createFile().getDirPath("apps/public");
		RhoClassFactory.createFile().getDirPath("db");
		RhoClassFactory.createFile().getDirPath("db/db-files");
		
    	//Class mainRuby = Class.forName("xruby.ServeME.main");
		//DBAdapter.startAllDBTransaction();
		try{
			mainObj = helper.createMainObject();//new xruby.ServeME.main();//(RubyProgram)mainRuby.newInstance();
			receiver = mainObj.invoke();
			
			if ( !rho_ruby_isValid() )
				throw new RuntimeException("Initialize Rho framework failed.");
		}finally
		{
			//DBAdapter.commitAllDBTransaction();
		}
		
//		RubyModule modRhom = (RubyModule)RubyRuntime.ObjectClass.getConstant("Rhom");
	}
	
	public static void RhoRubyInitApp(){
		RubyAPI.callPublicNoArgMethod(receiver, null, initApp);
	}

	public static void call_config_conflicts()
	{
		RubyHash hashConflicts = RhoConf.getInstance().getRubyConflicts();
		if (hashConflicts.size().toInt() == 0 )
			return;
		
		RubyAPI.callPublicOneArgMethod(receiver, hashConflicts, null, onConfigConflicts_mid);
	}
	
	public static void rho_ruby_uiCreated() {
		RubyAPI.callPublicNoArgMethod(receiver, null, uiCreated);
	}
	
	public static void rho_ruby_uiDestroyed() {
		RubyAPI.callPublicNoArgMethod(receiver, null, uiDestroyed);
	}
	
	public static void rho_ruby_activateApp(){
		RubyAPI.callPublicNoArgMethod(receiver, null, activateApp);
	}

	public static void rho_ruby_deactivateApp(){
		RubyAPI.callPublicNoArgMethod(receiver, null, deactivateApp);
	}
	
	public static boolean rho_ruby_isValid(){
		return receiver!= null && receiver != RubyConstant.QNIL;
	}
	
	public static void rho_ruby_callmethod(String strMethod)
	{
		RubyID methodID = RubyID.intern(strMethod);
		RubyAPI.callGlobalNoArgMethod(null, methodID);
	}
	
	public static boolean isMainRubyThread()
	{
		return RubyThread.isMainThread();
	}
	
	public static void RhoRubyStop(){
		
		receiver = null;
		mainObj = null;
		
		com.xruby.runtime.lang.RubyRuntime.fini();	
	}
	
	static public boolean resourceFileExists(String path)
	{
		InputStream is = loadFile(path);
		boolean bRes = is != null;
		try{ if ( is != null ) is.close(); }catch(java.io.IOException exc){}
		
		return bRes;
	}
	
	static public InputStream loadFile(String path){
		try {
			return RhoClassFactory.createFile().getResourceAsStream(mainObj.getClass(), path);
		} catch (Exception e) {
			e.printStackTrace();
		}	
		return null;
	}

	public static RubyValue processIndexRequest(String strIndexArg, RubyValue hashReq ){
		
		String strIndex = strIndexArg.replace('\\', '/');
/*		int nAppsIndex = strIndex.indexOf("/apps/");
		if ( nAppsIndex >= 0 ){
			int endIndex = strIndex.indexOf('/', nAppsIndex+6);
			if ( endIndex >= 0 )
				RhoSupport.setCurAppPath( strIndex.substring(nAppsIndex, endIndex+1));
		}*/
		
		RubyValue value = RubyAPI.callPublicTwoArgMethod(receiver, 
				ObjectFactory.createString(strIndex), hashReq, null, serveIndexID);
		
		return value;
	}

	public static void raise_RhoError(int errCode)
	{
		RubyAPI.callPublicOneArgMethod(receiver, ObjectFactory.createInteger(errCode), null, raiseRhoError);
	}
	
	public static String getStartPage()
	{
		return RhoConf.getInstance().getString("start_path");
		//RubyValue value = RubyAPI.callPublicNoArgMethod(receiver, null, getStartPath);
		//return value.toString();
	}

	public static String getOptionsPage()
	{
		return RhoConf.getInstance().getString("options_path");

		//RubyValue value = RubyAPI.callPublicNoArgMethod(receiver, null, getOptionsPath);
		//return value.toString();
	}
	
	public static RubyValue processRequest(Properties reqHash, 
			Properties reqHeaders, Properties resHeaders, String strIndex )throws IOException
	{
		RubyHash rh = ObjectFactory.createHash();
		for( int i = 0; i < reqHash.size(); i++ ){
			if ( reqHash.getValueAt(i) != null)
				addStrToHash(rh, reqHash.getKeyAt(i), reqHash.getValueAt(i) );
		}

		RubyHash headers = ObjectFactory.createHash();
		for( int i = 0; i < reqHeaders.size(); i++ ){
			if ( reqHeaders.getValueAt(i) != null){
				String strKey = reqHeaders.getKeyAt(i);
				if ( strKey.equalsIgnoreCase("content-type"))
					strKey = "Content-Type";
				else if ( strKey.equalsIgnoreCase("content-length"))
					strKey = "Content-Length";
				
				addStrToHash(headers, strKey, reqHeaders.getValueAt(i) );
			}
		}
		
		addHashToHash( rh, "headers", headers );
		
		RubyValue res = strIndex != null? processIndexRequest(strIndex, rh) : callFramework(rh); 
		return res; 
	}
	
	static RubyValue callFramework(RubyValue hashReq) {
		RubyValue value = RubyAPI.callPublicOneArgMethod(receiver, hashReq, null, serveID);
		
		return value;
	}

	public static RubyValue getFrameworkObj() {
		return receiver;
	}
	
	public static RubyHash createHash() {
		return ObjectFactory.createHash();
	}

	public static RubyArray create_array() {
		return new RubyArray();
	}

	public static RubyString create_string(String str) {
		return ObjectFactory.createString(str);
	}
	
	public static void add_to_array(RubyValue ar, RubyValue val)
	{
		((RubyArray)ar).add(val);
	}
	
	public static Vector makeVectorStringFromArray(RubyValue v)
	{
		Vector res = new Vector();

		if ( v == RubyConstant.QNIL )
			return res;
		
		RubyArray ar = (RubyArray)v;
		for ( int i = 0; i < ar.size(); i++ )
			res.addElement(ar.get(i).toStr());
		
		return res;
	}
	
	public static RubyValue addTimeToHash(RubyHash hash, String key, long val) {
		return hash.add( ObjectFactory.createString(key), ObjectFactory.createTime(val) );
	}

	public static RubyValue addIntToHash(RubyHash hash, String key, int val) {
		return hash.add( ObjectFactory.createString(key), ObjectFactory.createInteger(val));
	}

	public static RubyValue addStrToHash(RubyHash hash, String key, String val) {
		return hash.add( ObjectFactory.createString(key), ObjectFactory.createString(val));
	}

	public static RubyValue addHashToHash(RubyHash hash, String key, RubyValue val) {
		return hash.add( ObjectFactory.createString(key), val);	
	}
	
	public static Hashtable enum_strhash( RubyValue valHash)
	{
		Hashtable hash = new Hashtable();
		
		if ( valHash == null || valHash == RubyConstant.QNIL )
			return hash;
		
		RubyHash items = (RubyHash)valHash;
		RubyArray keys = items.keys();
		RubyArray values = items.values();
		for( int i = 0; i < keys.size(); i++ ){
			String label = keys.get(i).toString();
			String value = values.get(i).toString();
			
			hash.put(label, value);
		}
		
		return hash;
	}
}
