/* syncengine.i */
%module SyncEngine
%{
/* Put header files here or function declarations like below */
	#define dosync rho_sync_doSyncAllSources
	extern VALUE rho_sync_doSyncAllSources(int show_status_popup, const char * query_params, int sync_only_changed_sources);
	#define dosync_source rho_sync_doSyncSource
	extern VALUE rho_sync_doSyncSource(VALUE source_id,int show_status_popup, const char * query_params);
	#define dosearch rho_sync_doSearch
	extern VALUE rho_sync_doSearch(VALUE ar_sources, const char *from, const char *params, int sync_changes, int nProgressStep, const char* callback, const char* callback_params);
	extern VALUE rho_sync_login(const char *login, const char *password, const char* callback);
	#define login rho_sync_login
	extern int rho_sync_logged_in();
	#define logged_in rho_sync_logged_in
	extern void rho_sync_logout();
	#define logout rho_sync_logout
	extern void rho_sync_stop();
	#define stop_sync rho_sync_stop
	extern void rho_sync_set_notification(int source_id, const char *url, char* params);
	#define set_notification rho_sync_set_notification
	extern void rho_sync_clear_notification(int source_id);
	#define clear_notification rho_sync_clear_notification
	extern int rho_sync_set_pollinterval(int interval);
	#define set_pollinterval rho_sync_set_pollinterval
	extern int rho_sync_get_pollinterval();
	#define get_pollinterval rho_sync_get_pollinterval
	
	extern void rho_sync_set_syncserver(char* syncserver);
	#define set_syncserver rho_sync_set_syncserver
	extern VALUE rho_sync_get_attrs(const char* szPartition, int source_id);
	#define get_src_attrs rho_sync_get_attrs
	extern VALUE rho_sync_is_blob_attr(const char* szPartition, int source_id, const char* szAttrName);
	#define is_blob_attr rho_sync_is_blob_attr

	extern void rho_sync_update_blob_attribs(const char* szPartition, int source_id);
	#define update_blob_attribs rho_sync_update_blob_attribs
	
    extern void  rho_sync_setobjectnotify_url(const char* szUrl);
    #define set_objectnotify_url rho_sync_setobjectnotify_url
    extern void  rho_sync_addobjectnotify(int nSrcID, const char* szObject);
    #define add_objectnotify rho_sync_addobjectnotify
    extern void  rho_sync_cleanobjectnotify();
    #define clean_objectnotify rho_sync_cleanobjectnotify

    extern int  rho_sync_get_lastsync_objectcount(int nSrcID);
    #define get_lastsync_objectcount rho_sync_get_lastsync_objectcount

    extern int  rho_sync_get_pagesize();
    #define get_pagesize rho_sync_get_pagesize

    extern void  rho_sync_set_pagesize(int pagesize);
    #define set_pagesize rho_sync_set_pagesize

	extern void rho_sync_set_bulk_notification(const char *url, char* params);
	#define set_bulk_notification rho_sync_set_bulk_notification
	extern void rho_sync_clear_bulk_notification();
	#define clear_bulk_notification rho_sync_clear_bulk_notification
	extern void rho_sync_set_threaded_mode(int b);
	#define set_threaded_mode rho_sync_set_threaded_mode

	extern void rho_sync_enable_status_popup(int b);
	#define enable_status_popup rho_sync_enable_status_popup

    extern void rho_sync_set_source_property(int nSrcID, const char* szPropName, const char* szPropValue);
	#define set_source_property rho_sync_set_source_property

    extern VALUE rho_sync_get_source_property(int nSrcID, const char* szPropName);
	#define get_source_property rho_sync_get_source_property

	extern void rho_sync_set_ssl_verify_peer(int b);
	#define set_ssl_verify_peer rho_sync_set_ssl_verify_peer

	extern VALUE rho_sync_is_syncing();
	#define is_syncing rho_sync_is_syncing

    extern VALUE rho_sync_register_push();
	#define register_push rho_sync_register_push	
	
	#if !defined(bool)
	#define bool int
	#define true  1
	#define false 0
	#endif
%}

%typemap(default) bool show_status_popup {
 $1 = 1;
}

%typemap(default) const char * query_params {
 $1 = 0;
}

%typemap(default) bool sync_only_changed_sources {
  $1 = 0;
}

extern VALUE dosync(bool show_status_popup, const char * query_params,bool sync_only_changed_sources);
extern VALUE dosync_source(VALUE source_id, bool show_status_popup, const char * query_params);
extern VALUE dosearch(VALUE ar_sources, const char *from, const char *params, bool sync_changes, int nProgressStep, const char* callback, const char* callback_params);
extern VALUE login(const char *login, const char *password, const char* callback);
extern int logged_in();
extern void logout();
extern void stop_sync();
extern void set_notification(int source_id, const char *url, char* params);
extern void clear_notification(int source_id);
extern int set_pollinterval(int interval);
extern int get_pollinterval();
extern void set_syncserver(char* syncserver);
extern VALUE get_src_attrs(const char* szPartition, int source_id);
extern VALUE is_blob_attr(const char* szPartition, int source_id, const char* szAttrName);
extern void  set_objectnotify_url(const char* szUrl);
extern void  add_objectnotify(int nSrcID, const char* szObject);
extern void  clean_objectnotify();
extern int   get_lastsync_objectcount(int nSrcID);
extern int   get_pagesize();
extern void  set_pagesize(int pagesize);
extern void  set_threaded_mode(bool b);
extern void  enable_status_popup(bool b);
extern void  set_source_property(int nSrcID, const char* szPropName, const char* szPropValue);
extern VALUE  get_source_property(int nSrcID, const char* szPropName);
extern void  set_ssl_verify_peer(bool b);
extern void  update_blob_attribs(const char* szPartition, int source_id);
extern VALUE is_syncing();
extern void  register_push();