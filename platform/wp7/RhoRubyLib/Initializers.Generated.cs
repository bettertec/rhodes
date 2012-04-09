/* ****************************************************************************
 *
 * Copyright (c) Microsoft Corporation. 
 *
 * This source code is subject to terms and conditions of the Apache License, Version 2.0. A 
 * copy of the license can be found in the License.html file at the root of this distribution. If 
 * you cannot locate the  Apache License, Version 2.0, please send an email to 
 * ironruby@microsoft.com. By using this source code in any fashion, you are agreeing to be bound 
 * by the terms of the Apache License, Version 2.0.
 *
 * You must not remove this notice, or any other, from this software.
 *
 *
 * ***************************************************************************/

#pragma warning disable 169 // mcs: unused private method
[assembly: IronRuby.Runtime.RubyLibraryAttribute(typeof(rho.rubyext.rubyextLibraryInitializer))]

namespace rho.rubyext {
    using System;
    using Microsoft.Scripting.Utils;
    using System.Runtime.InteropServices;
    
    public sealed class rubyextLibraryInitializer : IronRuby.Builtins.LibraryInitializer {
        protected override void LoadModules() {
            IronRuby.Builtins.RubyClass classRef0 = GetClass(typeof(System.Object));
            
            
            DefineGlobalModule("Alert", typeof(rho.rubyext.RhoAlert), 0x00000008, null, LoadAlert_Class, null, IronRuby.Builtins.RubyModule.EmptyArray);
            DefineGlobalModule("Camera", typeof(rho.rubyext.RhoCamera), 0x00000008, null, LoadCamera_Class, null, IronRuby.Builtins.RubyModule.EmptyArray);
            DefineGlobalModule("DateTimePicker", typeof(rho.rubyext.RhoDateTimePicker), 0x00000008, null, LoadDateTimePicker_Class, null, IronRuby.Builtins.RubyModule.EmptyArray);
            DefineGlobalModule("NativeBar", typeof(rho.rubyext.RhoNativeBar), 0x00000008, null, LoadNativeBar_Class, null, IronRuby.Builtins.RubyModule.EmptyArray);
            IronRuby.Builtins.RubyModule def3 = DefineGlobalModule("Rho", typeof(rho.rubyext.RhoRoot), 0x00000008, null, null, null, IronRuby.Builtins.RubyModule.EmptyArray);
            IronRuby.Builtins.RubyModule def1 = DefineGlobalModule("Rho", typeof(rho.rubyext.Rho), 0x00000008, null, null, null, IronRuby.Builtins.RubyModule.EmptyArray);
            IronRuby.Builtins.RubyModule def4 = DefineModule("Rho::AsyncHttp", typeof(rho.rubyext.RhoRoot.RhoAsyncHttp), 0x00000008, null, LoadRho__AsyncHttp_Class, null, IronRuby.Builtins.RubyModule.EmptyArray);
            IronRuby.Builtins.RubyModule def2 = DefineModule("Rho::JSON", typeof(rho.rubyext.Rho.RhoJSON), 0x00000008, null, LoadRho__JSON_Class, null, IronRuby.Builtins.RubyModule.EmptyArray);
            DefineGlobalModule("RhoConf", typeof(rho.rubyext.RhoConfig), 0x00000008, null, LoadRhoConf_Class, null, IronRuby.Builtins.RubyModule.EmptyArray);
            IronRuby.Builtins.RubyModule def5 = DefineGlobalModule("SQLite3", typeof(rho.rubyext.RhoSQLite3), 0x00000008, null, null, null, IronRuby.Builtins.RubyModule.EmptyArray);
            IronRuby.Builtins.RubyClass def6 = DefineClass("SQLite3::Database", typeof(rho.rubyext.RhoSQLite3.RhoDatabase), 0x00000008, classRef0, LoadSQLite3__Database_Instance, null, null, IronRuby.Builtins.RubyModule.EmptyArray, 
                new Func<IronRuby.Builtins.RubyClass, IronRuby.Builtins.MutableString, IronRuby.Builtins.MutableString, rho.rubyext.RhoSQLite3.RhoDatabase>(rho.rubyext.RhoSQLite3.RhoDatabase.Create)
            );
            DefineGlobalModule("SyncEngine", typeof(rho.rubyext.RhoSyncEngine), 0x00000008, null, LoadSyncEngine_Class, null, IronRuby.Builtins.RubyModule.EmptyArray);
            DefineGlobalModule("System", typeof(rho.rubyext.RhoSystem), 0x00000008, null, LoadSystem_Class, null, IronRuby.Builtins.RubyModule.EmptyArray);
            DefineGlobalModule("WebView", typeof(rho.rubyext.RhoWebView), 0x00000008, null, LoadWebView_Class, null, IronRuby.Builtins.RubyModule.EmptyArray);
            SetConstant(def3, "AsyncHttp", def4);
            SetConstant(def1, "JSON", def2);
            SetConstant(def5, "Database", def6);
        }
        
        private static void LoadAlert_Class(IronRuby.Builtins.RubyModule/*!*/ module) {
            DefineLibraryMethod(module, "hide_popup", 0x21, 
                0x00000000U, 
                new Action<IronRuby.Builtins.RubyModule>(rho.rubyext.RhoAlert.HidePopup)
            );
            
            DefineLibraryMethod(module, "play_file", 0x21, 
                0x00000006U, 
                new Action<IronRuby.Builtins.RubyModule, System.String, System.String>(rho.rubyext.RhoAlert.PlayFile)
            );
            
            DefineLibraryMethod(module, "show_popup", 0x21, 
                0x00000000U, 
                new Action<IronRuby.Builtins.RubyModule, IronRuby.Builtins.Hash>(rho.rubyext.RhoAlert.ShowPopup)
            );
            
            DefineLibraryMethod(module, "show_status", 0x21, 
                0x00000006U, 
                new Action<IronRuby.Builtins.RubyModule, System.String, System.String, System.String>(rho.rubyext.RhoAlert.ShowStatus)
            );
            
            DefineLibraryMethod(module, "vibrate", 0x21, 
                0x00000000U, 
                new Action<IronRuby.Builtins.RubyModule, System.Int32>(rho.rubyext.RhoAlert.Vibrate)
            );
            
        }
        
        private static void LoadCamera_Class(IronRuby.Builtins.RubyModule/*!*/ module) {
            DefineLibraryMethod(module, "choose_picture", 0x21, 
                0x00000002U, 
                new Action<IronRuby.Builtins.RubyModule, System.String>(rho.rubyext.RhoCamera.choosePicture)
            );
            
            DefineLibraryMethod(module, "take_picture", 0x21, 
                0x00000002U, 
                new Action<IronRuby.Builtins.RubyModule, System.String>(rho.rubyext.RhoCamera.takePicture)
            );
            
        }
        
        private static void LoadDateTimePicker_Class(IronRuby.Builtins.RubyModule/*!*/ module) {
            DefineLibraryMethod(module, "choose", 0x21, 
                0x80000000U, 
                new Action<IronRuby.Builtins.RubyModule, System.Object[]>(rho.rubyext.RhoDateTimePicker.choose)
            );
            
            DefineLibraryMethod(module, "choose_with_range", 0x21, 
                0x80000000U, 
                new Action<IronRuby.Builtins.RubyModule, System.Object[]>(rho.rubyext.RhoDateTimePicker.choose_with_range)
            );
            
        }
        
        private static void LoadNativeBar_Class(IronRuby.Builtins.RubyModule/*!*/ module) {
            DefineLibraryMethod(module, "create", 0x21, 
                0x00000000U, 
                new Action<IronRuby.Builtins.RubyModule, System.Int32, System.Object>(rho.rubyext.RhoNativeBar.Create)
            );
            
            DefineLibraryMethod(module, "create_native_tabbar", 0x21, 
                0x00000000U, 
                new Action<IronRuby.Builtins.RubyModule, System.Int32, System.Object>(rho.rubyext.RhoNativeBar.createNativeTabBar)
            );
            
            DefineLibraryMethod(module, "create_native_toolbar", 0x21, 
                0x00000000U, 
                new Action<IronRuby.Builtins.RubyModule, System.Int32, System.Object>(rho.rubyext.RhoNativeBar.createNativeToolBar)
            );
            
            DefineLibraryMethod(module, "native_tabbar_get_current_tab", 0x21, 
                0x00000000U, 
                new Func<IronRuby.Builtins.RubyModule, System.Int32>(rho.rubyext.RhoNativeBar.nativeTabBarGetCurrentTab)
            );
            
            DefineLibraryMethod(module, "native_tabbar_set_tab_badge", 0x21, 
                0x00000000U, 
                new Action<IronRuby.Builtins.RubyModule, System.Int32, IronRuby.Builtins.MutableString>(rho.rubyext.RhoNativeBar.nativeTabBarSetTabBadge)
            );
            
            DefineLibraryMethod(module, "native_tabbar_switch_tab", 0x21, 
                0x00000000U, 
                new Action<IronRuby.Builtins.RubyModule, System.Int32>(rho.rubyext.RhoNativeBar.nativeTabBarSwitchTab)
            );
            
            DefineLibraryMethod(module, "remove", 0x21, 
                0x00000000U, 
                new Action<IronRuby.Builtins.RubyModule>(rho.rubyext.RhoNativeBar.Remove)
            );
            
            DefineLibraryMethod(module, "remove_native_tabbar", 0x21, 
                0x00000000U, 
                new Action<IronRuby.Builtins.RubyModule>(rho.rubyext.RhoNativeBar.removeNativeTabBar)
            );
            
            DefineLibraryMethod(module, "remove_native_toolbar", 0x21, 
                0x00000000U, 
                new Action<IronRuby.Builtins.RubyModule>(rho.rubyext.RhoNativeBar.removeNativeToolBar)
            );
            
            DefineLibraryMethod(module, "set_tab_badge", 0x21, 
                0x00000000U, 
                new Action<IronRuby.Builtins.RubyModule, System.Int32, IronRuby.Builtins.MutableString>(rho.rubyext.RhoNativeBar.setTabBadge)
            );
            
            DefineLibraryMethod(module, "started", 0x21, 
                0x00000000U, 
                new Func<IronRuby.Builtins.RubyModule, System.Boolean>(rho.rubyext.RhoNativeBar.isStarted)
            );
            
            DefineLibraryMethod(module, "switch_tab", 0x21, 
                0x00000000U, 
                new Action<IronRuby.Builtins.RubyModule, System.Int32>(rho.rubyext.RhoNativeBar.switchTab)
            );
            
        }
        
        private static void LoadRho__AsyncHttp_Class(IronRuby.Builtins.RubyModule/*!*/ module) {
            DefineLibraryMethod(module, "cancel", 0x21, 
                0x00000002U, 0x00000000U, 
                new Action<IronRuby.Builtins.RubyModule, System.String>(rho.rubyext.RhoRoot.RhoAsyncHttp.Cancel), 
                new Action<IronRuby.Builtins.RubyModule>(rho.rubyext.RhoRoot.RhoAsyncHttp.Cancel)
            );
            
            DefineLibraryMethod(module, "do_request", 0x21, 
                0x00000002U, 
                new Func<IronRuby.Builtins.RubyModule, System.String, IronRuby.Builtins.Hash, IronRuby.Builtins.MutableString>(rho.rubyext.RhoRoot.RhoAsyncHttp.doRequest)
            );
            
        }
        
        private static void LoadRho__JSON_Class(IronRuby.Builtins.RubyModule/*!*/ module) {
            DefineLibraryMethod(module, "parse", 0x21, 
                0x00000002U, 
                new Func<IronRuby.Builtins.RubyModule, System.String, System.Object>(rho.rubyext.Rho.RhoJSON.parse)
            );
            
            DefineLibraryMethod(module, "quote_value", 0x21, 
                0x00000002U, 
                new Func<IronRuby.Builtins.RubyModule, System.String, System.Object>(rho.rubyext.Rho.RhoJSON.quote_value)
            );
            
        }
        
        private static void LoadRhoConf_Class(IronRuby.Builtins.RubyModule/*!*/ module) {
            DefineLibraryMethod(module, "clean_log", 0x21, 
                0x00000000U, 
                new Action<IronRuby.Builtins.RubyModule>(rho.rubyext.RhoConfig.clean_log)
            );
            
            DefineLibraryMethod(module, "get_property_by_name", 0x21, 
                0x00000002U, 
                new Func<IronRuby.Builtins.RubyModule, System.String, System.Object>(rho.rubyext.RhoConfig.get_property_by_name)
            );
            
            DefineLibraryMethod(module, "is_property_exists", 0x21, 
                0x00000002U, 
                new Func<IronRuby.Builtins.RubyModule, System.String, System.Boolean>(rho.rubyext.RhoConfig.is_property_exists)
            );
            
            DefineLibraryMethod(module, "read_log", 0x21, 
                0x00000000U, 
                new Action<IronRuby.Builtins.RubyModule, System.Int32>(rho.rubyext.RhoConfig.read_log)
            );
            
            DefineLibraryMethod(module, "send_log", 0x21, 
                0x00000000U, 
                new Action<IronRuby.Builtins.RubyModule>(rho.rubyext.RhoConfig.send_log)
            );
            
            DefineLibraryMethod(module, "set_property_by_name", 0x21, 
                0x00000002U, 
                new Action<IronRuby.Builtins.RubyModule, System.String, System.Object>(rho.rubyext.RhoConfig.set_property_by_name)
            );
            
            DefineLibraryMethod(module, "show_log", 0x21, 
                0x00000000U, 
                new Action<IronRuby.Builtins.RubyModule>(rho.rubyext.RhoConfig.show_log)
            );
            
        }
        
        private static void LoadSQLite3__Database_Instance(IronRuby.Builtins.RubyModule/*!*/ module) {
            DefineLibraryMethod(module, "close", 0x11, 
                0x00000000U, 
                new Action<rho.rubyext.RhoSQLite3.RhoDatabase>(rho.rubyext.RhoSQLite3.RhoDatabase.Close)
            );
            
            DefineLibraryMethod(module, "commit", 0x11, 
                0x00000000U, 
                new Action<rho.rubyext.RhoSQLite3.RhoDatabase>(rho.rubyext.RhoSQLite3.RhoDatabase.Commit)
            );
            
            DefineLibraryMethod(module, "destroy_tables", 0x11, 
                0x00000000U, 
                new Action<rho.rubyext.RhoSQLite3.RhoDatabase, IronRuby.Builtins.RubyArray, IronRuby.Builtins.RubyArray>(rho.rubyext.RhoSQLite3.RhoDatabase.destroyTables)
            );
            
            DefineLibraryMethod(module, "execute", 0x11, 
                0x00000000U, 
                new Func<rho.rubyext.RhoSQLite3.RhoDatabase, IronRuby.Builtins.MutableString, System.Boolean, IronRuby.Builtins.RubyArray, IronRuby.Builtins.RubyArray>(rho.rubyext.RhoSQLite3.RhoDatabase.Execute)
            );
            
            DefineLibraryMethod(module, "is_ui_waitfordb", 0x11, 
                0x00000000U, 
                new Func<rho.rubyext.RhoSQLite3.RhoDatabase, System.Boolean>(rho.rubyext.RhoSQLite3.RhoDatabase.isUiWaitForDb)
            );
            
            DefineLibraryMethod(module, "lock_db", 0x11, 
                0x00000000U, 
                new Action<rho.rubyext.RhoSQLite3.RhoDatabase>(rho.rubyext.RhoSQLite3.RhoDatabase.Lock)
            );
            
            DefineLibraryMethod(module, "rollback", 0x11, 
                0x00000000U, 
                new Action<rho.rubyext.RhoSQLite3.RhoDatabase>(rho.rubyext.RhoSQLite3.RhoDatabase.Rollback)
            );
            
            DefineLibraryMethod(module, "start_transaction", 0x11, 
                0x00000000U, 
                new Action<rho.rubyext.RhoSQLite3.RhoDatabase>(rho.rubyext.RhoSQLite3.RhoDatabase.startTransaction)
            );
            
            DefineLibraryMethod(module, "table_exist?", 0x11, 
                0x00000000U, 
                new Func<rho.rubyext.RhoSQLite3.RhoDatabase, IronRuby.Builtins.MutableString, System.Boolean>(rho.rubyext.RhoSQLite3.RhoDatabase.isTableExist)
            );
            
            DefineLibraryMethod(module, "unlock_db", 0x11, 
                0x00000000U, 
                new Action<rho.rubyext.RhoSQLite3.RhoDatabase>(rho.rubyext.RhoSQLite3.RhoDatabase.Unlock)
            );
            
        }
        
        private static void LoadSyncEngine_Class(IronRuby.Builtins.RubyModule/*!*/ module) {
            DefineLibraryMethod(module, "add_objectnotify", 0x21, 
                0x00000006U, 
                new Action<IronRuby.Builtins.RubyModule, System.Int32, System.String>(rho.rubyext.RhoSyncEngine.add_objectnotify)
            );
            
            DefineLibraryMethod(module, "clean_objectnotify", 0x21, 
                0x00000000U, 
                new Action<IronRuby.Builtins.RubyModule>(rho.rubyext.RhoSyncEngine.clean_objectnotify)
            );
            
            DefineLibraryMethod(module, "dosearch", 0x21, 
                0x000000feU, 
                new Func<IronRuby.Builtins.RubyModule, IronRuby.Builtins.RubyArray, System.String, System.String, System.Boolean, System.Int32, System.String, System.String, System.Object>(rho.rubyext.RhoSyncEngine.dosearch)
            );
            
            DefineLibraryMethod(module, "dosync", 0x21, 
                0x80000000U, 
                new Func<IronRuby.Builtins.RubyModule, System.Object[], System.Object>(rho.rubyext.RhoSyncEngine.dosync)
            );
            
            DefineLibraryMethod(module, "dosync_source", 0x21, 
                0x80000000U, 
                new Func<IronRuby.Builtins.RubyModule, System.Object[], System.Object>(rho.rubyext.RhoSyncEngine.dosync_source)
            );
            
            DefineLibraryMethod(module, "enable_status_popup", 0x21, 
                0x00000000U, 
                new Action<IronRuby.Builtins.RubyModule, System.Boolean>(rho.rubyext.RhoSyncEngine.enable_status_popup)
            );
            
            DefineLibraryMethod(module, "is_blob_attr", 0x21, 
                0x0000000eU, 
                new Func<IronRuby.Builtins.RubyModule, System.String, System.Int32, System.String, System.Boolean>(rho.rubyext.RhoSyncEngine.is_blob_attr)
            );
            
            DefineLibraryMethod(module, "is_syncing", 0x21, 
                0x00000000U, 
                new Func<IronRuby.Builtins.RubyModule, System.Boolean>(rho.rubyext.RhoSyncEngine.is_syncing)
            );
            
            DefineLibraryMethod(module, "logged_in", 0x21, 
                0x00000000U, 
                new Func<IronRuby.Builtins.RubyModule, System.Int32>(rho.rubyext.RhoSyncEngine.logged_in)
            );
            
            DefineLibraryMethod(module, "login", 0x21, 
                0x0000000eU, 
                new Func<IronRuby.Builtins.RubyModule, System.String, System.String, System.String, System.Object>(rho.rubyext.RhoSyncEngine.login)
            );
            
            DefineLibraryMethod(module, "logout", 0x21, 
                0x00000000U, 
                new Action<IronRuby.Builtins.RubyModule>(rho.rubyext.RhoSyncEngine.logout)
            );
            
            DefineLibraryMethod(module, "set_notification", 0x21, 
                0x00000006U, 
                new Action<IronRuby.Builtins.RubyModule, System.Int32, System.String, System.String>(rho.rubyext.RhoSyncEngine.set_notification)
            );
            
            DefineLibraryMethod(module, "set_objectnotify_url", 0x21, 
                0x00000002U, 
                new Action<IronRuby.Builtins.RubyModule, System.String>(rho.rubyext.RhoSyncEngine.set_objectnotify_url)
            );
            
            DefineLibraryMethod(module, "set_pollinterval", 0x21, 
                0x00000002U, 
                new Func<IronRuby.Builtins.RubyModule, System.Int32, System.Int32>(rho.rubyext.RhoSyncEngine.set_pollinterval)
            );
            
            DefineLibraryMethod(module, "set_source_property", 0x21, 
                0x0000000eU, 
                new Action<IronRuby.Builtins.RubyModule, System.Int32, System.String, System.String>(rho.rubyext.RhoSyncEngine.set_source_property)
            );
            
            DefineLibraryMethod(module, "set_syncserver", 0x21, 
                0x00000002U, 
                new Action<IronRuby.Builtins.RubyModule, System.String>(rho.rubyext.RhoSyncEngine.set_syncserver)
            );
            
            DefineLibraryMethod(module, "set_threaded_mode", 0x21, 
                0x00000002U, 
                new Action<IronRuby.Builtins.RubyModule, System.Boolean>(rho.rubyext.RhoSyncEngine.set_threaded_mode)
            );
            
            DefineLibraryMethod(module, "stop_sync", 0x21, 
                0x00000000U, 
                new Action<IronRuby.Builtins.RubyModule>(rho.rubyext.RhoSyncEngine.stop_sync)
            );
            
            DefineLibraryMethod(module, "update_blob_attribs", 0x21, 
                0x00000006U, 
                new Action<IronRuby.Builtins.RubyModule, System.String, System.Int32>(rho.rubyext.RhoSyncEngine.update_blob_attribs)
            );
            
        }
        
        private static void LoadSystem_Class(IronRuby.Builtins.RubyModule/*!*/ module) {
            DefineLibraryMethod(module, "app_install", 0x21, 
                0x00000002U, 
                new Action<IronRuby.Builtins.RubyModule, System.String>(rho.rubyext.RhoSystem.app_install)
            );
            
            DefineLibraryMethod(module, "app_installed?", 0x21, 
                0x00000002U, 
                new Func<IronRuby.Builtins.RubyModule, System.String, System.Boolean>(rho.rubyext.RhoSystem.is_app_installed)
            );
            
            DefineLibraryMethod(module, "app_uninstall", 0x21, 
                0x00000002U, 
                new Action<IronRuby.Builtins.RubyModule, System.String>(rho.rubyext.RhoSystem.app_uninstall)
            );
            
            DefineLibraryMethod(module, "exit", 0x21, 
                0x00000000U, 
                new Action<IronRuby.Builtins.RubyModule>(rho.rubyext.RhoSystem.exit)
            );
            
            DefineLibraryMethod(module, "get_locale", 0x21, 
                0x00000000U, 
                new Func<IronRuby.Builtins.RubyModule, System.String>(rho.rubyext.RhoSystem.get_locale)
            );
            
            DefineLibraryMethod(module, "get_property", 0x21, 
                0x00000002U, 
                new Func<IronRuby.Builtins.RubyModule, System.String, System.Object>(rho.rubyext.RhoSystem.get_property_by_name)
            );
            
            DefineLibraryMethod(module, "get_screen_height", 0x21, 
                0x00000000U, 
                new Func<IronRuby.Builtins.RubyModule, System.Int32>(rho.rubyext.RhoSystem.get_screen_height)
            );
            
            DefineLibraryMethod(module, "get_screen_width", 0x21, 
                0x00000000U, 
                new Func<IronRuby.Builtins.RubyModule, System.Int32>(rho.rubyext.RhoSystem.get_screen_width)
            );
            
            DefineLibraryMethod(module, "get_start_params", 0x21, 
                0x00000002U, 
                new Func<IronRuby.Builtins.RubyModule, System.String, System.String>(rho.rubyext.RhoSystem.get_start_params)
            );
            
            DefineLibraryMethod(module, "has_network", 0x21, 
                0x00000000U, 
                new Func<IronRuby.Builtins.RubyModule, System.Boolean>(rho.rubyext.RhoSystem.has_network)
            );
            
            DefineLibraryMethod(module, "open_url", 0x21, 
                0x00000002U, 
                new Action<IronRuby.Builtins.RubyModule, System.String>(rho.rubyext.RhoSystem.open_url)
            );
            
            DefineLibraryMethod(module, "run_app", 0x21, 
                0x00000002U, 
                new Action<IronRuby.Builtins.RubyModule, System.String, System.String>(rho.rubyext.RhoSystem.run_app)
            );
            
            DefineLibraryMethod(module, "set_push_notification", 0x21, 
                0x00000006U, 
                new Action<IronRuby.Builtins.RubyModule, System.String, System.String>(rho.rubyext.RhoSystem.set_push_notification)
            );
            
            DefineLibraryMethod(module, "set_screen_rotation_notification", 0x21, 
                0x00000006U, 
                new Action<IronRuby.Builtins.RubyModule, System.String, System.String>(rho.rubyext.RhoSystem.set_screen_rotation_notification)
            );
            
            DefineLibraryMethod(module, "set_sleeping", 0x21, 
                0x00000000U, 
                new Action<IronRuby.Builtins.RubyModule, System.Boolean>(rho.rubyext.RhoSystem.set_sleeping)
            );
            
            DefineLibraryMethod(module, "start_timer", 0x21, 
                0x00000004U, 
                new Action<IronRuby.Builtins.RubyModule, System.Int32, System.String, System.String>(rho.rubyext.RhoSystem.start_timer)
            );
            
            DefineLibraryMethod(module, "stop_timer", 0x21, 
                0x00000002U, 
                new Action<IronRuby.Builtins.RubyModule, System.String>(rho.rubyext.RhoSystem.stop_timer)
            );
            
            DefineLibraryMethod(module, "unzip_file", 0x21, 
                0x00000002U, 
                new Action<IronRuby.Builtins.RubyModule, System.String>(rho.rubyext.RhoSystem.unzip_file)
            );
            
        }
        
        private static void LoadWebView_Class(IronRuby.Builtins.RubyModule/*!*/ module) {
            DefineLibraryMethod(module, "current_location", 0x21, 
                0x00000000U, 
                new Func<IronRuby.Builtins.RubyModule, System.Int32, System.String>(rho.rubyext.RhoWebView.currentLocation)
            );
            
            DefineLibraryMethod(module, "execute_js", 0x21, 
                0x00000002U, 
                new Action<IronRuby.Builtins.RubyModule, System.String, System.Int32, IronRuby.Builtins.RubyArray>(rho.rubyext.RhoWebView.execute_js)
            );
            
            DefineLibraryMethod(module, "navigate", 0x21, 
                0x00000002U, 
                new Action<IronRuby.Builtins.RubyModule, System.String, System.Int32>(rho.rubyext.RhoWebView.Navigate)
            );
            
            DefineLibraryMethod(module, "refresh", 0x21, 
                0x00000000U, 
                new Action<IronRuby.Builtins.RubyModule, System.Int32>(rho.rubyext.RhoWebView.Refresh)
            );
            
            DefineLibraryMethod(module, "set_menu_items", 0x21, 
                0x00000000U, 
                new Action<IronRuby.Builtins.RubyModule, IronRuby.Builtins.Hash>(rho.rubyext.RhoWebView.setMenuItems)
            );
            
        }
        
    }
}

