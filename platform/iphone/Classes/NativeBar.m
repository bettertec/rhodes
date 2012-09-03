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

#import "NativeBar.h"
#import "Rhodes.h"
#import "SimpleMainView.h"
#import "TabbedMainView.h"
#import "SplittedMainView.h"

//#include "common/rhoparams.h"
#include "logging/RhoLog.h"
#include "ruby/ext/rho/rhoruby.h"

#undef DEFAULT_LOGCATEGORY
#define DEFAULT_LOGCATEGORY "NativeBar"

static int started = 0;

@interface RhoNativeBarCreateTask : NSObject {}
+ (void)run:(NSValue*)value :(NSDictionary*)parameters;
@end

@implementation RhoNativeBarCreateTask
+ (void)run:(NSValue*)value :(NSDictionary*)parameters {
    int type;
    [value getValue:&type];
    
    id view = nil;
   
    Rhodes *r = [Rhodes sharedInstance];
    
    id mainView = [r mainView];
    UIWindow* w = r.window;

	SimpleMainView* smv = nil;
    if ([mainView isKindOfClass:[SimpleMainView class]]) {
	    smv = (SimpleMainView*)mainView;
	}
    switch (type) {
		case NOBAR_TYPE:
			if (smv != nil) {
				[smv removeToolbar];
			}
			else {
				view = [[SimpleMainView alloc] initWithMainView:mainView parent:w ];
				[r setMainView:view];
				[view release];
			}
			started = 0;
			break;
		case TOOLBAR_TYPE:
			if (smv != nil) {
				[smv addToolbar:parameters];
			}
			else {
				view = [[SimpleMainView alloc] initWithMainView:mainView parent:w bar_info:parameters];
				[r setMainView:view];
				[view release];
			}
			started = 1;
			break;
		case TABBAR_TYPE: {
			[[Rhodes sharedInstance] hideSplash];
			view = [[TabbedMainView alloc] initWithMainView:mainView parent:w bar_info:parameters];
			started = 1;
			[r setMainView:view];
			[view release];
		}
			break;
		case VTABBAR_TYPE: {
			[[Rhodes sharedInstance] hideSplash];
			BOOL is_iPad = NO;
			{
				NSString *model = [[UIDevice currentDevice] model]; // "iPad ..."
				if ([model hasPrefix:@"iPad"]) {
					is_iPad = YES;
				}
			}
			if (is_iPad) {
				view = [[SplittedMainView alloc] initWithMainView:mainView parent:w bar_info:parameters];
			}
			else {
				view = [[TabbedMainView alloc] initWithMainView:mainView parent:w bar_info:parameters];
			}
			started = 1;
			[r setMainView:view];
			[view release];
			}
			break;
		default:
			RAWLOG_ERROR1("Unknown bar type passed: %d", type);
        return;
    }
 
}
@end

@interface RhoNativeBarSwitchTabTask : NSObject {}
+ (void)run:(NSValue*)value;
@end

@implementation RhoNativeBarSwitchTabTask
+ (void)run:(NSValue*)value {
    int index;
    [value getValue:&index];
    [[[Rhodes sharedInstance] mainView] switchTab:index];
}
@end

#define BADGE_TEXT @"badge_text"
#define TAB_INDEX @"tab_index"

@interface RhoNativeTabBarSetBadgeTask : NSObject {}
+ (void)run:(NSDictionary*)params;
@end

@implementation RhoNativeTabBarSetBadgeTask
+ (void)run:(NSDictionary*)params {
    NSValue* ns_tab_index  = [params objectForKey:TAB_INDEX];
    int tab_index = 0;
    [ns_tab_index getValue:&tab_index];
    NSString* badge_text = (NSString*)[params objectForKey:BADGE_TEXT];

    [[[Rhodes sharedInstance] mainView] setTabBarBadge:badge_text tab_index:tab_index];
}
@end


void create_nativebar_innner(int bar_type, rho_param *p)
{
    if (!rho_rhodesapp_check_mode())
        return;

	const char* background_color = NULL;
	const char* background_color_enable = NULL;
    
    const char* on_change_tab_callback = NULL;
	
	rho_param *params = NULL;
    switch (p->type) {
        case RHO_PARAM_ARRAY:
            params = p;
            break;
        case RHO_PARAM_HASH: {
            for (int i = 0, lim = p->v.hash->size; i < lim; ++i) {
                const char *name = p->v.hash->name[i];
                rho_param *value = p->v.hash->value[i];
                
                if (strcasecmp(name, "background_color") == 0) {
					background_color = value->v.string;
					background_color_enable = "true";
                }

                if (strcasecmp(name, "on_change_tab_callback") == 0) {
					on_change_tab_callback = value->v.string;
                }
                
                if (strcasecmp(name, "buttons") == 0 || strcasecmp(name, "tabs") == 0) {
                    params = value;
                }
            }
        }
            break;
        default: {
            RAWLOG_ERROR("Unexpected parameter type for create_nativebar, should be Array or Hash");
            return;
        }
    }
    
    if (!params) {
        RAWLOG_ERROR("Wrong parameters for create_nativebar");
        return;
    }
    
    int size = params->v.array->size;

	NSMutableDictionary* main_properties = [NSMutableDictionary dictionaryWithCapacity:2];
	
	NSMutableDictionary* properties = [NSMutableDictionary dictionaryWithCapacity:1];
	[main_properties setObject:properties forKey:NATIVE_BAR_PROPERTIES];
	
	NSMutableArray* items = [NSMutableArray arrayWithCapacity:size];
	[main_properties setObject:items forKey:NATIVE_BAR_ITEMS];
	
	
    for (int i = 0; i < size; ++i) {
        rho_param *hash = params->v.array->value[i];
        if (hash->type != RHO_PARAM_HASH) {
            RAWLOG_ERROR("Unexpected type of array item for create_nativebar, should be Hash");
            return;
        }
        
        const char *label = NULL;
        const char *action = NULL;
        const char *icon = NULL;
        const char *reload = NULL;
        const char *colored_icon = NULL;
		
		const char *selected_color = NULL;
		//const char *selected_color_enable = NULL;
		const char *disabled = NULL;
		const char* web_bkg_color = NULL;
		const char* use_current_view_for_tab = NULL;
		
        BOOL skip_item = NO;
        for (int j = 0, lim = hash->v.hash->size; j < lim; ++j) {
            const char *name = hash->v.hash->name[j];
            rho_param *value = hash->v.hash->value[j];
            if (value->type != RHO_PARAM_STRING) {
                RAWLOG_ERROR1("Unexpected '%s' type, should be String", name);
                return;
            }
			if (strcasecmp(name, "background_color") == 0) {
				background_color = value->v.string;
				//background_color_enable = "true";
				skip_item = YES;
			}
            
            if (strcasecmp(name, "label") == 0)
                label = value->v.string;
            else if (strcasecmp(name, "action") == 0)
                action = value->v.string;
            else if (strcasecmp(name, "icon") == 0)
                icon = value->v.string;
            else if (strcasecmp(name, "reload") == 0)
                reload = value->v.string;
            else if (strcasecmp(name, "colored_icon") == 0)
                colored_icon = value->v.string;
            else if (strcasecmp(name, "selected_color") == 0){
                selected_color = value->v.string;
				//selected_color_enable = "true";
			}	
            else if (strcasecmp(name, "disabled") == 0)
                disabled = value->v.string;
            else if (strcasecmp(name, "web_bkg_color") == 0)
                web_bkg_color = value->v.string;
			else if (strcasecmp(name, "use_current_view_for_tab") == 0) {
                use_current_view_for_tab = value->v.string;
				if (strcasecmp(use_current_view_for_tab, "true") == 0) {
					action = "none";
				}
			}
				
        }
        
        if (label == NULL && bar_type == TOOLBAR_TYPE)
            label = "";
        
        if ((label == NULL || (action == NULL)) && (!skip_item)) {
            RAWLOG_ERROR("Illegal argument for create_nativebar");
            return;
        }
		if (!skip_item) {
			NSMutableDictionary* item = [NSMutableDictionary dictionaryWithCapacity:10];	
			
			[item setObject:[NSString stringWithUTF8String:label] forKey:NATIVE_BAR_ITEM_LABEL];
			[item setObject:[NSString stringWithUTF8String:action] forKey:NATIVE_BAR_ITEM_ACTION];
			[item setObject:[NSString stringWithUTF8String:(icon ? icon : "")] forKey:NATIVE_BAR_ITEM_ICON];
			[item setObject:[NSString stringWithUTF8String:(reload ? reload : "false")] forKey:NATIVE_BAR_ITEM_RELOAD];
			[item setObject:[NSString stringWithUTF8String:(colored_icon ? colored_icon : "false")] forKey:NATIVE_BAR_ITEM_COLORED_ICON];
			if (selected_color != nil) {
				[item setObject:[NSString stringWithUTF8String:selected_color] forKey:NATIVE_BAR_ITEM_SELECTED_COLOR];
			}
			[item setObject:[NSString stringWithUTF8String:(disabled ? disabled : "false")] forKey:NATIVE_BAR_ITEM_DISABLED];
			if (web_bkg_color != NULL) {
				[item setObject:[NSString stringWithUTF8String:web_bkg_color] forKey:NATIVE_BAR_ITEM_WEB_BACKGROUND_COLOR];
			}
			[item setObject:[NSString stringWithUTF8String:(use_current_view_for_tab ? use_current_view_for_tab : "false")] forKey:NATIVE_BAR_ITEM_USE_CURRENT_VIEW_FOR_TAB];
			
			[items addObject:item];
		}
		
	}
	if (background_color != NULL) {
		[properties setObject:[NSString stringWithUTF8String:background_color] forKey:NATIVE_BAR_BACKGOUND_COLOR];	
	}
    if (on_change_tab_callback != NULL) {
		[properties setObject:[NSString stringWithUTF8String:on_change_tab_callback] forKey:NATIVE_BAR_ON_CHANGE_TAB_CALLBACK];	
    }
	
    id runnable = [RhoNativeBarCreateTask class];
    id arg1 = [NSValue valueWithBytes:&bar_type objCType:@encode(int)];
    [Rhodes performOnUiThread:runnable arg:arg1 arg:main_properties wait:NO];
}

void remove_nativebar_innner() {
    if (!rho_rhodesapp_check_mode())
        return;
    int bar_type = NOBAR_TYPE;
    id runnable = [RhoNativeBarCreateTask class];
    id arg1 = [NSValue valueWithBytes:&bar_type objCType:@encode(int)];
    [Rhodes performOnUiThread:runnable arg:arg1 arg:nil wait:NO];
}

void nativebar_switch_tab_innner(int index) {
    if (!rho_rhodesapp_check_mode())
        return;
    id runnable = [RhoNativeBarSwitchTabTask class];
    id arg = [NSValue valueWithBytes:&index objCType:@encode(int)];
    [Rhodes performOnUiThread:runnable arg:arg wait:NO];
}

VALUE nativebar_started() {
    return rho_ruby_create_boolean(started);
}



void create_nativebar(int bar_type, rho_param *p)
{
	RAWLOG_INFO("NativeBar.create() is DEPRECATED. Use Rho::NativeToolbar.create() or Rho::NativeTabbar.create().");
	create_nativebar_innner(bar_type, p);
}

void remove_nativebar() {
	RAWLOG_INFO("NativeBar.remove() is DEPRECATED. Use Rho::NativeToolbar.remove() or Rho::NativeTabbar.remove().");
	remove_nativebar_innner();
}

void nativebar_switch_tab(int index) {
	RAWLOG_INFO("NativeBar.switch_tab() is DEPRECATED. Use Rho::NativeTabbar.switch_tab().");
	nativebar_switch_tab_innner(index);
}


void create_native_toolbar(int bar_type, rho_param *p) {
	create_nativebar_innner(bar_type, p);
}

void remove_native_toolbar() {
	remove_nativebar_innner();
}

void create_native_tabbar(int bar_type, rho_param *p) {
	create_nativebar_innner(bar_type, p);
}

void remove_native_tabbar() {
	remove_nativebar_innner();
}

void native_tabbar_switch_tab(int index) {
	nativebar_switch_tab_innner(index);
}



void native_tabbar_set_tab_badge(int index,char *val) {
    NSString* badge_text = [NSString stringWithUTF8String:val];
    
    NSMutableDictionary* params = [NSMutableDictionary dictionaryWithCapacity:2];
    NSValue* tab_index = [NSValue valueWithBytes:&index objCType:@encode(int)];
    [params setObject:tab_index forKey:TAB_INDEX];	
    [params setObject:badge_text forKey:BADGE_TEXT];	
    
    id runnable = [RhoNativeTabBarSetBadgeTask class];
    id arg = params;
    [Rhodes performOnUiThread:runnable arg:arg wait:NO];
    
}

int native_tabbar_get_current_tab() {
    Rhodes *r = [Rhodes sharedInstance];
    return [[r mainView] activeTab];
}


