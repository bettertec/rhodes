﻿/*------------------------------------------------------------------------
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

using Microsoft.Scripting.Utils;
using Microsoft.Scripting.Runtime;
using IronRuby.Runtime;
using IronRuby.Builtins;
using System;
using rho.sync;
using rho.common;
using rho.db;

namespace rho.rubyext
{
    [RubyModule("SyncEngine")]
    public static class RhoSyncEngine
    {
        private static RhoLogger LOG = RhoLogger.RHO_STRIP_LOG ? new RhoEmptyLogger() :
                    new RhoLogger("Sync");
        static CRhoRuby RhoRuby { get { return CRhoRuby.Instance; } }

        [RubyMethod("dosync", RubyMethodAttributes.PublicSingleton)]
        public static object dosync(RubyModule/*!*/ self, params object[] args)
        {
            object res = null;
            try
            {
                boolean bShowStatus = true;	
                String query_params = "";
                if ( args != null && args.Length > 0 )	
                {
                    if (args[0] is int)
                        bShowStatus = ((int)args[0]).Equals(1);
                    else if (args[0] is bool)
                        bShowStatus = (bool)args[0];
                    else
                    {
                        String str = ((MutableString)args[0]).ToString();
                        //bShowStatus = args[0].Equals(RubyConstant.QTRUE)||"true".equalsIgnoreCase(str);	
                        bShowStatus = "true".equalsIgnoreCase(str);
                    }
                }
        
                if (args != null && args.Length > 1)
                    query_params = ((MutableString)args[1]).ToString();
                
                SyncThread.getInstance().addQueueCommand(new SyncThread.SyncCommand(SyncThread.scSyncAll, bShowStatus, query_params));

                res = SyncThread.getInstance().getRetValue();
            }
            catch (Exception ex)
            {
                Exception rubyEx = self.Context.CurrentException;
                if (rubyEx == null)
                {
                    rubyEx = RubyExceptionData.InitializeException(new RuntimeError(ex.Message.ToString()), ex.Message);
                }
                LOG.ERROR("dosync", ex);
                throw rubyEx;
            }

            return res;
        }

        /*[RubyMethod("dosync", RubyMethodAttributes.PublicSingleton)]
        public static object dosync(RubyModule self, bool show_status_popup)
        {
            object res = null;
            try
            {
                SyncThread.getInstance().addQueueCommand(new SyncThread.SyncCommand(SyncThread.scSyncAll, show_status_popup));

                res = SyncThread.getInstance().getRetValue();
            }
            catch (Exception ex)
            {
                Exception rubyEx = self.Context.CurrentException;
                if (rubyEx == null)
                {
                    rubyEx = RubyExceptionData.InitializeException(new RuntimeError(ex.Message.ToString()), ex.Message);
                }
                LOG.ERROR("dosync", ex);
                throw rubyEx;
            }

            return res;
        }*/

        [RubyMethod("dosync_source", RubyMethodAttributes.PublicSingleton)]
        public static object dosync_source(RubyModule/*!*/ self, params object[] args)
        {
            object res = null;
            try
            {
                int nSrcID = 0;
                String strName = "";

                if ( args[0] is  long)
                    nSrcID = (int)(long)args[0];
                else if (args[0] is int)
                    nSrcID = (int)args[0];
                else
                    strName = ((MutableString)args[0]).ToString();

                /*if (srcID is long)
                    nSrcID = (int)(long)srcID;
                else if (srcID is int)
                    nSrcID = (int)srcID;
                else
                    strName = srcID.ToString();*/

                boolean bShowStatus = true;	
                String query_params = "";
                if ( args != null && args.Length > 1 )	
                {
                    if (args[1] is int)
                        bShowStatus = ((int)args[1]).Equals(1);
                    else if (args[1] is bool)
                        bShowStatus = (bool)args[1];
                    else
                    {
                        String str = ((MutableString)args[1]).ToString();
                        //bShowStatus = args[0].Equals(RubyConstant.QTRUE)||"true".equalsIgnoreCase(str);	
                        bShowStatus = "true".equalsIgnoreCase(str);
                    }
                }

                if (args != null && args.Length > 2)
                    query_params = ((MutableString)args[2]).ToString();

                SyncThread.getInstance().addQueueCommand(new SyncThread.SyncCommand(SyncThread.scSyncOne, strName, nSrcID, bShowStatus, query_params));

                res = SyncThread.getInstance().getRetValue();
            }
            catch (Exception ex)
            {
                Exception rubyEx = self.Context.CurrentException;
                if (rubyEx == null)
                {
                    rubyEx = RubyExceptionData.InitializeException(new RuntimeError(ex.Message.ToString()), ex.Message);
                }
                LOG.ERROR("dosync_source", ex);
                throw rubyEx;
            }

            return res;
        }
        
        /*[RubyMethod("dosync_source", RubyMethodAttributes.PublicSingleton)]
        public static object dosync_source(RubyModule self, [NotNull]object srcID, bool show_status_popup)
        {
            object res = null;
            try
            {
                int nSrcID = 0;
                String strName = "";
                if (srcID is long)
                    nSrcID = (int)(long)srcID;
                else if (srcID is int)
                    nSrcID = (int)srcID;
                else
                    strName = srcID.ToString();

                SyncThread.getInstance().addQueueCommand(new SyncThread.SyncCommand(SyncThread.scSyncOne, strName, nSrcID, show_status_popup));

                res = SyncThread.getInstance().getRetValue();
            }
            catch (Exception ex)
            {
                Exception rubyEx = self.Context.CurrentException;
                if (rubyEx == null)
                {
                    rubyEx = RubyExceptionData.InitializeException(new RuntimeError(ex.Message.ToString()), ex.Message);
                }
                LOG.ERROR("dosync_source", ex);
                throw rubyEx;
            }

            return res;
        }*/

        [RubyMethod("dosearch", RubyMethodAttributes.PublicSingleton)]
        public static object dosearch(RubyModule/*!*/ self, [NotNull]RubyArray/*!*/ arSourcesR, [NotNull]String/*!*/ from, [NotNull]String/*!*/ strParams,
            [NotNull]bool/*!*/ bSearchSyncChanges, [NotNull]int/*!*/ nProgressStep, String/*!*/ strCallback, String/*!*/ strCallbackParams)
        {
            object res = null;
            try
            {
                SyncThread.stopSync();

                if (strCallback != null && strCallback.Length > 0)
                    SyncThread.getSyncEngine().getNotify().setSearchNotification(strCallback, strCallbackParams);

                Vector<String> arSources = RhoRuby.makeVectorStringFromArray(arSourcesR);

                SyncThread.getInstance().addQueueCommand(new SyncThread.SyncSearchCommand(from, strParams, arSources, bSearchSyncChanges, nProgressStep));

                res = SyncThread.getInstance().getRetValue();
            }
            catch (Exception ex)
            {
                Exception rubyEx = self.Context.CurrentException;
                if (rubyEx == null)
                {
                    rubyEx = RubyExceptionData.InitializeException(new RuntimeError(ex.Message.ToString()), ex.Message);
                }
                LOG.ERROR("dosearch", ex);
                throw rubyEx;
            }

            return res;
        }

        [RubyMethod("logged_in", RubyMethodAttributes.PublicSingleton)]
        public static int logged_in(RubyModule/*!*/ self)
        {
            int res = 0;
            try
            {
                res = SyncThread.getSyncEngine().isLoggedIn() ? 1 : 0;
            }
            catch (Exception ex)
            {
                Exception rubyEx = self.Context.CurrentException;
                if (rubyEx == null)
                {
                    rubyEx = RubyExceptionData.InitializeException(new RuntimeError(ex.Message.ToString()), ex.Message);
                }
                LOG.ERROR("logged_in", ex);
                throw rubyEx;
            }

            return res;
        }

        [RubyMethod("login", RubyMethodAttributes.PublicSingleton)]
        public static object login(RubyModule/*!*/ self, [NotNull]String/*!*/ name, [NotNull]String/*!*/ pwd, [NotNull]String/*!*/ callback)
        {
            try{
                SyncThread.stopSync();

                SyncThread.getInstance().addQueueCommand(new SyncThread.SyncLoginCommand(name, pwd, callback,
                        new SyncNotify.SyncNotification(callback, "", false)));
                return SyncThread.getInstance().getRetValue();
            }
            catch (Exception e)
            {
                LOG.ERROR("SyncEngine.login", e);
                RhoRuby.raise_RhoError(RhoAppAdapter.ERR_RUNTIME);
            }

            return null;
        }

        [RubyMethod("set_notification", RubyMethodAttributes.PublicSingleton)]
        public static void set_notification(RubyModule/*!*/ self, [NotNull]int/*!*/ nSrcID, [NotNull]String/*!*/ url, string/*!*/ callback_param)
        {
            try
            {
                SyncThread.getSyncEngine().getNotify().setSyncNotification(nSrcID,
                    new SyncNotify.SyncNotification(url, callback_param != null ? callback_param : "", nSrcID != -1));
            }
            catch (Exception ex)
            {
                Exception rubyEx = self.Context.CurrentException;
                if (rubyEx == null)
                {
                    rubyEx = RubyExceptionData.InitializeException(new RuntimeError(ex.Message.ToString()), ex.Message);
                }
                LOG.ERROR("set_notification", ex);
                throw rubyEx;
            }
        }

        [RubyMethod("enable_status_popup", RubyMethodAttributes.PublicSingleton)]
        public static void enable_status_popup(RubyModule/*!*/ self, bool/*!*/ bEnable)
        {
            try
            {
                SyncThread.getSyncEngine().getNotify().enableStatusPopup(bEnable);
            }
            catch (Exception ex)
            {
                Exception rubyEx = self.Context.CurrentException;
                if (rubyEx == null)
                {
                    rubyEx = RubyExceptionData.InitializeException(new RuntimeError(ex.Message.ToString()), ex.Message);
                }
                LOG.ERROR("enable_status_popup", ex);
                throw rubyEx;
            }
        }

        [RubyMethod("is_blob_attr", RubyMethodAttributes.PublicSingleton)]
        public static bool is_blob_attr(RubyModule/*!*/ self, [NotNull]String/*!*/ strPartition, [NotNull]int/*!*/ nSrcID, [NotNull]String/*!*/ strAttrName)
        {
            bool res = false;
            try
            {
                res = DBAdapter.getDB(strPartition).getAttrMgr().isBlobAttr(nSrcID, strAttrName);
            }
            catch (Exception ex)
            {
                Exception rubyEx = self.Context.CurrentException;
                if (rubyEx == null)
                {
                    rubyEx = RubyExceptionData.InitializeException(new RuntimeError(ex.Message.ToString()), ex.Message);
                }
                LOG.ERROR("is_blob_attr", ex);
                throw rubyEx;
            }

            return res;
        }

        [RubyMethod("update_blob_attribs", RubyMethodAttributes.PublicSingleton)]
        public static void update_blob_attribs(RubyModule/*!*/ self, [NotNull]String/*!*/ strPartition, [NotNull]int/*!*/ srcID)
        {
            try
            {
                DBAdapter db = DBAdapter.getDB(strPartition);
                db.getAttrMgr().loadBlobAttrs(db);
            }
            catch (Exception ex)
            {
                Exception rubyEx = self.Context.CurrentException;
                if (rubyEx == null)
                {
                    rubyEx = RubyExceptionData.InitializeException(new RuntimeError(ex.Message.ToString()), ex.Message);
                }
                LOG.ERROR("update_blob_attribs", ex);
                throw rubyEx;
            }
        }

        [RubyMethod("set_threaded_mode", RubyMethodAttributes.PublicSingleton)]
        public static void set_threaded_mode(RubyModule/*!*/ self, [NotNull]bool/*!*/ bThreadMode)
        {
            try
            {
                SyncThread.getInstance().setNonThreadedMode(!bThreadMode);
                SyncThread.getSyncEngine().setNonThreadedMode(!bThreadMode);
            }
            catch (Exception ex)
            {
                Exception rubyEx = self.Context.CurrentException;
                if (rubyEx == null)
                {
                    rubyEx = RubyExceptionData.InitializeException(new RuntimeError(ex.Message.ToString()), ex.Message);
                }
                LOG.ERROR("set_threaded_mode", ex);
                throw rubyEx;
            }
        }

        [RubyMethod("logout", RubyMethodAttributes.PublicSingleton)]
        public static void logout(RubyModule/*!*/ self)
        {
            try
            {
                SyncThread.stopSync();
                SyncThread.getSyncEngine().logout_int();
            }
            catch (Exception ex)
            {
                Exception rubyEx = self.Context.CurrentException;
                if (rubyEx == null)
                {
                    rubyEx = RubyExceptionData.InitializeException(new RuntimeError(ex.Message.ToString()), ex.Message);
                }
                LOG.ERROR("logout", ex);
                throw rubyEx;
            }
        }

        [RubyMethod("set_syncserver", RubyMethodAttributes.PublicSingleton)]
        public static void set_syncserver(RubyModule/*!*/ self, [NotNull]String/*!*/ syncserver)
        {

            try
            {
                SyncThread.stopSync();
                SyncThread.getSyncEngine().setSyncServer(syncserver);

                if (syncserver != null && syncserver.length() > 0)
                {
                    SyncThread.getInstance().start(SyncThread.epLow);
                    if (ClientRegister.getInstance() != null)
                        ClientRegister.getInstance().startUp();
                }
            }
            catch (Exception ex)
            {
                Exception rubyEx = self.Context.CurrentException;
                if (rubyEx == null)
                {
                    rubyEx = RubyExceptionData.InitializeException(new RuntimeError(ex.Message.ToString()), ex.Message);
                }
                LOG.ERROR("set_syncserver", ex);
                throw rubyEx;
            }
        }

        [RubyMethod("set_pollinterval", RubyMethodAttributes.PublicSingleton)]
        public static int set_pollinterval(RubyModule/*!*/ self, [NotNull]int/*!*/ nInterval)
        {
            int nOldInterval = 0;
            try
            {
                nOldInterval = SyncThread.getInstance().getPollInterval();
                SyncThread.getInstance().setPollInterval(nInterval);
            }
            catch (Exception ex)
            {
                Exception rubyEx = self.Context.CurrentException;
                if (rubyEx == null)
                {
                    rubyEx = RubyExceptionData.InitializeException(new RuntimeError(ex.Message.ToString()), ex.Message);
                }
                LOG.ERROR("set_pollinterval", ex);
                throw rubyEx;
            }

            return nOldInterval;
        }

        [RubyMethod("stop_sync", RubyMethodAttributes.PublicSingleton)]
        public static void stop_sync(RubyModule/*!*/ self)
        {
            try
            {
                SyncThread.stopSync();
            }
            catch (Exception ex)
            {
                Exception rubyEx = self.Context.CurrentException;
                if (rubyEx == null)
                {
                    rubyEx = RubyExceptionData.InitializeException(new RuntimeError(ex.Message.ToString()), ex.Message);
                }
                LOG.ERROR("stop_sync", ex);
                throw rubyEx;
            }
        }

        [RubyMethod("is_syncing", RubyMethodAttributes.PublicSingleton)]
        public static bool is_syncing(RubyModule/*!*/ self)
        {
            try
            {
                return SyncThread.getSyncEngine().isSyncing();
            }
            catch (Exception ex)
            {
                LOG.HandleRubyException(ex, RhoRuby.rubyContext.CurrentException, "is_syncing");
                return false;
            }
        }

        [RubyMethod("set_source_property", RubyMethodAttributes.PublicSingleton)]
        public static void set_source_property(RubyModule/*!*/ self, [NotNull]int/*!*/ nSrcID, [NotNull]String/*!*/ strPropName, [NotNull]String/*!*/ strPropValue)
        {
            try
            {
                SyncEngine.getSourceOptions().setProperty(nSrcID, strPropName, strPropValue);
            }
            catch (Exception ex)
            {
                Exception rubyEx = self.Context.CurrentException;
                if (rubyEx == null)
                {
                    rubyEx = RubyExceptionData.InitializeException(new RuntimeError(ex.Message.ToString()), ex.Message);
                }
                LOG.ERROR("set_source_property", ex);
                throw rubyEx;
            }
        }

        [RubyMethod("set_objectnotify_url", RubyMethodAttributes.PublicSingleton)]
        public static void set_objectnotify_url(RubyModule/*!*/ self, [DefaultProtocol, NotNull]string/*!*/ url)
        {
            try
            {
                SyncNotify.setObjectNotifyUrl(url);
            }
            catch (Exception ex)
            {
                Exception rubyEx = self.Context.CurrentException;
                if (rubyEx == null)
                {
                    rubyEx = RubyExceptionData.InitializeException(new RuntimeError(ex.Message.ToString()), ex.Message);
                }
                LOG.ERROR("set_objectnotify_url", ex);
                throw rubyEx;
            }
        }

        [RubyMethod("add_objectnotify", RubyMethodAttributes.PublicSingleton)]
        public static void add_objectnotify(RubyModule/*!*/ self, [NotNull]int/*!*/ nSrcID, [NotNull]string/*!*/ strObject)
        {
            try
            {
                SyncThread.getSyncEngine().getNotify().addObjectNotify(nSrcID, strObject);
            }
            catch (Exception ex)
            {
                Exception rubyEx = self.Context.CurrentException;
                if (rubyEx == null)
                {
                    rubyEx = RubyExceptionData.InitializeException(new RuntimeError(ex.Message.ToString()), ex.Message);
                }
                LOG.ERROR("add_objectnotify", ex);
                throw rubyEx;
            }
        }

        [RubyMethod("clean_objectnotify", RubyMethodAttributes.PublicSingleton)]
        public static void clean_objectnotify(RubyModule/*!*/ self)
        {
            try
            {
                SyncThread.getSyncEngine().getNotify().cleanObjectNotifications();
            }
            catch (Exception ex)
            {
                Exception rubyEx = self.Context.CurrentException;
                if (rubyEx == null)
                {
                    rubyEx = RubyExceptionData.InitializeException(new RuntimeError(ex.Message.ToString()), ex.Message);
                }
                LOG.ERROR("clean_objectnotify", ex);
                throw rubyEx;
            }
        }
    }
}