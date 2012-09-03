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

#import <AVFoundation/AVFoundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import "RhoAlert.h"
#import "Rhodes.h"

#include "common/RhodesApp.h"
#include "logging/RhoLog.h"

#undef DEFAULT_LOGCATEGORY
#define DEFAULT_LOGCATEGORY "Alert"

static UIAlertView *currentAlert = nil;
static BOOL is_current_alert_status = NO;

@interface RhoAlertShowPopupTask : NSObject<UIAlertViewDelegate> {
    NSString *callback;
    NSMutableArray *buttons;
}

@property (nonatomic,retain) NSString *callback;
@property (nonatomic,retain) NSMutableArray *buttons;

- (id)init;
- (void)dealloc;
- (void)run:(NSValue*)v;
- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex;
    
@end

@implementation RhoAlertShowPopupTask

@synthesize callback, buttons;

- (id)init {
    callback = nil;
    buttons = nil;
    return self;
}

- (void)dealloc {
    self.callback = nil;
    self.buttons = nil;
    [super dealloc];
}

- (void)run:(NSValue*)v {
    NSString *title = @"Alert";
    NSString *message = nil;
    
	is_current_alert_status = NO;
	
    self.buttons = [NSMutableArray arrayWithCapacity:1];
    rho_param *p = [v pointerValue];
    if (p->type == RHO_PARAM_STRING) {
        message = [NSString stringWithUTF8String:p->v.string];
        [buttons addObject:[NSMutableArray arrayWithObjects:@"OK", @"OK", nil]];
    }
    else if (p->type == RHO_PARAM_HASH) {
        for (int i = 0, lim = p->v.hash->size; i < lim; ++i) {
            char *name = p->v.hash->name[i];
            rho_param *value = p->v.hash->value[i];
            
            if (strcasecmp(name, "title") == 0) {
                if (value->type != RHO_PARAM_STRING) {
                    RAWLOG_ERROR("'title' should be string");
                    continue;
                }
                title = [NSString stringWithUTF8String:value->v.string];
            }
            else if (strcasecmp(name, "message") == 0) {
                if (value->type != RHO_PARAM_STRING) {
                    RAWLOG_ERROR("'message' should be string");
                    continue;
                }
                message = [NSString stringWithUTF8String:value->v.string];
            }
            else if (strcasecmp(name, "callback") == 0) {
                if (value->type != RHO_PARAM_STRING) {
                    RAWLOG_ERROR("'callback' should be string");
                    continue;
                }
                self.callback = [NSString stringWithUTF8String:value->v.string];
            }
            else if (strcasecmp(name, "buttons") == 0) {
                if (value->type != RHO_PARAM_ARRAY) {
                    RAWLOG_ERROR("'buttons' should be array");
                    continue;
                }
                for (int j = 0, limj = value->v.array->size; j < limj; ++j) {
                    rho_param *arrValue = value->v.array->value[j];
                    
                    NSString *itemId = nil;
                    NSString *itemTitle = nil;
                    switch (arrValue->type) {
                        case RHO_PARAM_STRING:
                            itemId = [NSString stringWithUTF8String:arrValue->v.string];
                            itemTitle = [NSString stringWithUTF8String:arrValue->v.string];
                            break;
                        case RHO_PARAM_HASH:
                            for (int k = 0, limk = arrValue->v.hash->size; k < limk; ++k) {
                                char *sName = arrValue->v.hash->name[k];
                                rho_param *sValue = arrValue->v.hash->value[k];
                                if (sValue->type != RHO_PARAM_STRING) {
                                    RAWLOG_ERROR("Illegal type of button item's value");
                                    continue;
                                }
                                if (strcasecmp(sName, "id") == 0)
                                    itemId = [NSString stringWithUTF8String:sValue->v.string];
                                else if (strcasecmp(sName, "title") == 0)
                                    itemTitle = [NSString stringWithUTF8String:sValue->v.string];
                            }
                            break;
                        default:
                            RAWLOG_ERROR("Illegal type of button item");
                            continue;
                    }
                    
                    if (!itemId || !itemTitle) {
                        RAWLOG_ERROR("Incomplete button item");
                        continue;
                    }
                    
                    NSMutableArray *btn = [NSMutableArray arrayWithCapacity:2];
                    [btn addObject:itemId];
                    [btn addObject:itemTitle];
                    [buttons addObject:btn];
                }
            }
			else if (strcasecmp(name, "status_type") == 0) {
				is_current_alert_status = YES;
            }
			

        }
    }
    rho_param_free(p);
    
	if ((currentAlert != nil) && (is_current_alert_status)) {
		currentAlert.message = message;
		return;
	}
	
    UIAlertView *alert = [[[UIAlertView alloc]
                  initWithTitle:title
                  message:message
                  delegate:self
                  cancelButtonTitle:nil
                  otherButtonTitles:nil] autorelease];
    
    for (int i = 0, lim = [buttons count]; i < lim; ++i) {
        NSArray *btn = [buttons objectAtIndex:i];
        NSString *title = [btn objectAtIndex:1];
        [alert addButtonWithTitle:title];
    }
    
    [alert show];
    currentAlert = alert;
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex {
    currentAlert = nil;
	is_current_alert_status = NO;

    if (!callback)
        return;
    
    if (buttonIndex < 0 || buttonIndex >= [buttons count])
        return;
    
    NSArray *btn = [buttons objectAtIndex:buttonIndex];
    NSString *itemId = [[btn objectAtIndex:0] stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
    NSString *itemTitle = [[btn objectAtIndex:1] stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
    
    rho_rhodesapp_callPopupCallback([callback UTF8String], [itemId UTF8String], [itemTitle UTF8String]);
    [self release];
}

@end

@interface RhoAlertHidePopupTask : NSObject {}
+ (void)run;
@end

@implementation RhoAlertHidePopupTask
+ (void)run {
    if (!currentAlert)
        return;
    [currentAlert dismissWithClickedButtonIndex:-1 animated:NO];
    currentAlert = nil;
	is_current_alert_status = NO;
}
@end


@interface RhoAlertVibrateTask : NSObject {}
+ (void)run;
@end

@implementation RhoAlertVibrateTask
+ (void)run {
    AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
}
@end

@interface RhoAlertPlayFileTask : NSObject {}
+ (void)run:(NSString*)file :(NSString*)type;
@end

@implementation RhoAlertPlayFileTask
+ (void)run:(NSString*)file :(NSString*)type {
    [[Rhodes sharedInstance] playStart:file mediaType:type];
}
@end

@implementation RhoAlert

+ (void)showPopup:(rho_param*)p {
    id runnable = [[RhoAlertShowPopupTask alloc] init];
    NSValue *value = [NSValue valueWithPointer:rho_param_dup(p)];
    [Rhodes performOnUiThread:runnable arg:value wait:NO];
}

+ (void)hidePopup {
    id runnable = [RhoAlertHidePopupTask class];
    [Rhodes performOnUiThread:runnable wait:NO];
}

+ (void)vibrate:(int)duration {
    id runnable = [RhoAlertVibrateTask class];
    [Rhodes performOnUiThread:runnable wait:NO];
}

+ (void)playFile:(NSString *)file mediaType:(NSString *)type {
    id runnable = [RhoAlertPlayFileTask class];
    [Rhodes performOnUiThread:runnable arg:file arg:type wait:NO];
}

@end

void alert_show_status(const char* szTitle, const char* szMessage, const char* szHide)
{
    //show new status dialog or update text
    if (!rho_rhodesapp_check_mode())
        return;
	
	rho_param* p = rho_param_hash(4);
	
	rho_param* p_title_key = rho_param_str("title");
	rho_param* p_title_value = rho_param_str(szTitle);

	rho_param* p_message_key = rho_param_str("message");
	rho_param* p_message_value = rho_param_str(szMessage);
	
	rho_param* p_buttons_key = rho_param_str("buttons");
	rho_param* p_buttons_value = rho_param_array(1);

	rho_param* p_status_key = rho_param_str("status_type");
	rho_param* p_status_value = rho_param_str("true");
	
	rho_param* p_button_value = rho_param_str(szHide);
	
	p_buttons_value->v.array->value[0] = p_button_value;
	
	p->v.hash->name[0] = p_title_key->v.string;
	p->v.hash->value[0] = p_title_value;
	
	p->v.hash->name[1] = p_message_key->v.string;
	p->v.hash->value[1] = p_message_value;
	
	p->v.hash->name[2] = p_buttons_key->v.string;
	p->v.hash->value[2] = p_buttons_value;

	p->v.hash->name[3] = p_status_key->v.string;
	p->v.hash->value[3] = p_status_value;
	
    [RhoAlert showPopup:p];
	
	rho_param_free(p);
	
	
}

void alert_show_popup(rho_param *p) {
    if (!rho_rhodesapp_check_mode())
        return;
    if (!p || (p->type != RHO_PARAM_STRING && p->type != RHO_PARAM_HASH)) {
        RAWLOG_ERROR("Alert.show_popup - wrong arguments");
        return;
	}
    
	
	//alert_show_status("Title", "Some message", "Close Status");
    [RhoAlert showPopup:p];
}

void alert_hide_popup() {
    if (!rho_rhodesapp_check_mode())
        return;
    [RhoAlert hidePopup];
}

void alert_vibrate(int duration_ms) {
    if (!rho_rhodesapp_check_mode())
        return;
    [RhoAlert vibrate:duration_ms];
}

void alert_play_file(char* file_name, char* media_type) {
    if (!rho_rhodesapp_check_mode())
        return;
    if (!file_name) {
        RAWLOG_ERROR("Alert.play_file - please specify file name to play");
        return;
    }
    
    [RhoAlert playFile:[NSString stringWithUTF8String:file_name]
            mediaType:media_type?[NSString stringWithUTF8String:media_type]:NULL];
}
