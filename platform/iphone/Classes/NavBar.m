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

#import "NavBar.h"
#import "Rhodes.h"

#include "logging/RhoLog.h"
#include "ruby/ext/rho/rhoruby.h"

#undef DEFAULT_LOGCATEGORY
#define DEFAULT_LOGCATEGORY "NavBar"

static int started = 0;

@interface RhoNavBarCreateTask : NSObject {}
+ (void)run:(NSArray*)args;
@end

@implementation RhoNavBarCreateTask
+ (void)run:(NSArray*)args {
    NSString *title = [args objectAtIndex:0];
    NSArray *left = [args objectAtIndex:1];
    NSArray *right = [args objectAtIndex:2];
    [[[Rhodes sharedInstance] mainView] addNavBar:title left:left right:right];
    started = 1;
}
@end

@interface RhoNavBarRemoveTask : NSObject {}
+ (void)run;
@end

@implementation RhoNavBarRemoveTask
+ (void)run {
    [[[Rhodes sharedInstance] mainView] removeNavBar];
    started = 0;
}
@end

@implementation NavBar

@end

void create_navbar(rho_param *p)
{
    if (!rho_rhodesapp_check_mode())
        return;
    if (p->type != RHO_PARAM_HASH) {
        RAWLOG_ERROR("Unexpected parameter type for create_navbar, should be Hash");
        return;
    }
    
    NSString *title = nil;
    NSMutableArray *left = [NSMutableArray arrayWithCapacity:3];
    NSMutableArray *right = [NSMutableArray arrayWithCapacity:3];
    
    int size = p->v.hash->size;
    for (int i = 0; i < size; ++i) {
        char *name = p->v.hash->name[i];
        rho_param *value = p->v.hash->value[i];
        
        if (strcasecmp(name, "title") == 0) {
            if (value->type != RHO_PARAM_STRING) {
                RAWLOG_ERROR("Unexpected type of 'title', should be String");
                return;
            }
            title = [NSString stringWithUTF8String:value->v.string];
        }
        else if (strcasecmp(name, "left") == 0 || strcasecmp(name, "right") == 0) {
            if (value->type != RHO_PARAM_HASH) {
                RAWLOG_ERROR1("Unexpected type of '%s', should be Hash", name);
                return;
            }
            char *label = NULL;
            char *icon = NULL;
            char *action = NULL;
            
            int hashSize = value->v.hash->size;
            for (int k = 0; k < hashSize; ++k) {
                char *n = value->v.hash->name[k];
                rho_param *v = value->v.hash->value[k];
                if (v->type != RHO_PARAM_STRING) {
                    RAWLOG_ERROR1("Unexpected type of '%s', should be String", n);
                    return;
                }
                
                if (strcasecmp(n, "label") == 0)
                    label = v->v.string;
                else if (strcasecmp(n, "icon") == 0)
                    icon = v->v.string;
                else if (strcasecmp(n, "action") == 0)
                    action = v->v.string;

            }
            
            NSMutableArray *button = strcasecmp(name, "left") == 0 ? left : right;
            [button addObject:[NSString stringWithUTF8String:(action ? action : "")]];
            [button addObject:[NSString stringWithUTF8String:(label ? label : "")]];
            [button addObject:[NSString stringWithUTF8String:(icon ? icon : "")]];
        }
    }
    
    id runnable = [RhoNavBarCreateTask class];
    NSMutableArray *args = [NSMutableArray arrayWithCapacity:3];
    [args addObject:title];
    [args addObject:left];
    [args addObject:right];
    [Rhodes performOnUiThread:runnable arg:args wait:YES];
}

void remove_navbar()
{
    if (!rho_rhodesapp_check_mode())
        return;
    id runnable = [RhoNavBarRemoveTask class];
    [Rhodes performOnUiThread:runnable wait:YES];
}

VALUE navbar_started() {
    return rho_ruby_create_boolean(started);
}
