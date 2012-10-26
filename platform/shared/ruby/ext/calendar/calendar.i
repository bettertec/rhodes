%module "Rho::Calendar"
%{
#include "ext/rho/rhoruby.h"

extern VALUE event_fetch(VALUE params);
#define fetch event_fetch
extern VALUE event_fetch_by_id(const char *id);
#define fetch_by_id event_fetch_by_id
extern const char* event_save(VALUE event);
#define save event_save
extern void event_delete(const char *id);
#define delete event_delete

extern const char* calendar_get_authorization_status(void);
%}

%typemap(check) VALUE {
    Check_Type($1, T_HASH);
}

extern VALUE fetch(VALUE params);
extern VALUE fetch_by_id(const char *id);

%typemap(check) VALUE {
    Check_Type($1, T_HASH);
}
extern const char* save(VALUE event);

extern void delete(const char *id);

extern const char* calendar_get_authorization_status(void);
