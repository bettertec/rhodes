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

#import "Rhodes.h"
#import "SimpleMainView.h"
#import "RhoAlert.h"
#import "ParamsWrapper.h"

#include "sync/ClientRegister.h"
#include "sync/SyncThread.h"
#include "logging/RhoLogConf.h"
#include "logging/RhoLog.h"
#include "common/RhoConf.h"
#include "common/RhodesApp.h"
//#include "common/app_build_configs.h"
#import "SplitView/SplittedMainView.h"

#include "common/app_build_capabilities.h"

#include "InitMemoryInfoCollector.h"



/*
use this non-public code for see level of memory warning 
 
typedef enum {
	OSMemoryNotificationLevelAny      = -1,
	OSMemoryNotificationLevelNormal   =  0,
	OSMemoryNotificationLevelWarning  =  1,
	OSMemoryNotificationLevelUrgent   =  2,
	OSMemoryNotificationLevelCritical =  3
} OSMemoryNotificationLevel;

extern OSMemoryNotificationLevel OSMemoryNotificationCurrentLevel(void);
*/


extern int rho_sys_check_rollback_bundle(const char* szRhoPath);
void rho_sys_impl_exit_with_errormessage(const char* szTitle, const char* szMsg);




#undef DEFAULT_LOGCATEGORY
#define DEFAULT_LOGCATEGORY "Rhodes"

void rho_geoimpl_init();

static BOOL app_created = NO;


@interface RhoActivateTask : NSObject {}
+ (void)run;
@end

@implementation RhoActivateTask
+ (void)run {
    rho_rhodesapp_callAppActiveCallback(1);
}
@end


@interface RhoFullScreenEnableTask : NSObject {}
+ (void)run;
@end

@implementation RhoFullScreenEnableTask
+ (void)run {
#ifdef __IPHONE_3_2
    [[Rhodes application] setStatusBarHidden:YES withAnimation:UIStatusBarAnimationFade];
#else
    [[Rhodes application] setStatusBarHidden:YES animated:YES];
#endif
    [[[[Rhodes sharedInstance] mainView] view] setFrame:[Rhodes applicationFrame]];
}
@end

@interface RhoFullScreenDisableTask : NSObject {}
+ (void)run;
@end

@implementation RhoFullScreenDisableTask
+ (void)run {
#ifdef __IPHONE_3_2
    [[Rhodes application] setStatusBarHidden:NO withAnimation:UIStatusBarAnimationFade];
#else
    [[Rhodes application] setStatusBarHidden:NO animated:YES];
#endif
    [[[[Rhodes sharedInstance] mainView] view] setFrame:[Rhodes applicationFrame]];
}
@end





@implementation Rhodes

@synthesize window, player, cookies, signatureDelegate, nvDelegate, mBlockExit;
#ifdef __IPHONE_4_0
@synthesize eventStore;
#endif

static Rhodes *instance = NULL;

+ (Rhodes*)sharedInstance {
    return instance;
}

+ (UIApplication*)application {
    return [Rhodes sharedInstance]->application;
}

+ (CGRect)applicationFrame {
    CGRect frame = [[UIScreen mainScreen] applicationFrame];
    //CGRect sbFrame = [[UIApplication sharedApplication] statusBarFrame];
    //frame.origin.y += sbFrame.size.height;
    //frame.size.height -= sbFrame.size.height;
    return frame;
}





+ (void)setStatusBarHidden:(BOOL)v {
	if (v) {
		[Rhodes performOnUiThread:[RhoFullScreenEnableTask class] wait:NO];
	}
	else {
		[Rhodes performOnUiThread:[RhoFullScreenDisableTask class] wait:NO];
	}
}


#if 0
+ (UIView*)subviewWithTag:(NSInteger)tag ofView:(UIView*)view {
    if (!view)
        return nil;
    
    for (int i = 0, lim = [view.subviews count]; i < lim; ++i) {
        UIView *subview = [view.subviews objectAtIndex:i];
        if (subview.tag == tag)
            return subview;
        subview = [self subviewWithTag:tag ofView:subview];
        if (subview)
            return subview;
    }
    
    return nil;
}
#endif

- (void)runRunnable:(NSArray*)args {
    id runnable = [args objectAtIndex:0];
    if ([runnable respondsToSelector:@selector(run)])
        [runnable performSelector:@selector(run)];
    else if ([runnable respondsToSelector:@selector(run:)]) {
        id arg = nil;
        if ([args count] == 2)
            arg = [args objectAtIndex:1];
        [runnable performSelector:@selector(run:) withObject:arg];
    }
    else if ([runnable respondsToSelector:@selector(run::)]) {
        id arg1 = nil;
        if ([args count] > 1)
            arg1 = [args objectAtIndex:1];
        id arg2 = nil;
        if ([args count] > 2)
            arg2 = [args objectAtIndex:2];
        [runnable performSelector:@selector(run::) withObject:arg1 withObject:arg2];
    }
    else {
        RAWLOG_ERROR("runRunnable: unrecognized parameters");
    }
}

+ (void)performOnUiThread:(id)runnable wait:(BOOL)wait {
    NSMutableArray *args = [NSMutableArray arrayWithCapacity:1];
    [args addObject:runnable];
    [[Rhodes sharedInstance] performSelectorOnMainThread:@selector(runRunnable:) withObject:args waitUntilDone:wait];
}

+ (void)performOnUiThread:(id)runnable arg:(id)arg wait:(BOOL)wait {
    NSMutableArray *args = [NSMutableArray arrayWithCapacity:2];
    [args addObject:runnable];
    if (arg)
        [args addObject:arg];
    [[Rhodes sharedInstance] performSelectorOnMainThread:@selector(runRunnable:) withObject:args waitUntilDone:wait];
}

+ (void)performOnUiThread:(id)runnable arg:(id)arg1 arg:(id)arg2 wait:(BOOL)wait {
    NSMutableArray *args = [NSMutableArray arrayWithCapacity:3];
    [args addObject:runnable];
    if (arg1)
        [args addObject:arg1];
    if (arg2)
        [args addObject:arg2];
    [[Rhodes sharedInstance] performSelectorOnMainThread:@selector(runRunnable:) withObject:args waitUntilDone:wait];
}

- (BOOL)isRotationLocked {
    return rotationLocked;
}

- (void)openMapLocation:(NSString*)query {
	[self hideSplash];
    NSURL* url = [NSURL URLWithString:[@"http://maps.google.com/?" stringByAppendingString:query]];
    [[UIApplication sharedApplication] openURL:url];
}

- (void)mapLocation:(NSString *)query {
    [self performSelectorOnMainThread:@selector(openMapLocation:) withObject:query waitUntilDone:NO];
}

- (void)doSysCall:(ParamsWrapper*)params {
    PARAMS_WRAPPER pw;
    do_syscall([params unwrap:&pw]);
    [params release];
}

- (void)sysCall:(PARAMS_WRAPPER*)params {
    ParamsWrapper* pw = [ParamsWrapper wrap:params]; 	
    [self performSelectorOnMainThread:@selector(doSysCall:) withObject:pw waitUntilDone:NO];
}

- (void)doShowLogOptions {
    if (logOptionsController!=NULL) {
        [window addSubview:logOptionsController.view];
        logOptionsController.view.hidden = NO;
    }
}

- (void)showLogOptions {
    [self performSelectorOnMainThread:@selector(doShowLogOptions) withObject:nil waitUntilDone:NO];
}

- (void)doShowLog {
    if (logViewController!=NULL) {
        [window addSubview:logViewController.view];
        logViewController.view.hidden = NO;
    }
}

- (void)showLog {
    [self performSelectorOnMainThread:@selector(doShowLog) withObject:nil waitUntilDone:NO];
}

- (void)setCookie:(NSString*)cookie forUrl:(NSString*)url {
    NSURL *parsed = [NSURL URLWithString:url];
    NSString *basicUrl = [NSString stringWithFormat:@"%@://%@/%@", parsed.scheme, parsed.host, parsed.path];
    [cookies setObject:cookie forKey:basicUrl];
}

- (NSString*)cookie:(NSString*)url {
    NSURL *parsed = [NSURL URLWithString:url];
    NSString *basicUrl = [NSString stringWithFormat:@"%@://%@/%@", parsed.scheme, parsed.host, parsed.path];
    NSString *c = [cookies objectForKey:basicUrl];
    return c;
}

- (void)chooseDateTime:(DateTime*)dateTime {
	[self hideSplash];
    dateTimePickerDelegate.dateTime = dateTime;
    [dateTimePickerDelegate setPostUrl:dateTime.url];
    //[self normalizeUrl:dateTime.url]];
    @try {
        [dateTimePickerDelegate createPicker:[mainView view]];
    } @catch (NSException* theException) {
        RAWLOG_ERROR2("startDateTimePickerFromViewController failed(%s): %s", [[theException name] UTF8String], [[theException reason] UTF8String] );
    }
}

-(BOOL)startCameraPicker:(PickImageDelegate*)delegateObject 
                                sourceType:(UIImagePickerControllerSourceType)type
{
	[self hideSplash];
#ifndef __IPHONE_3_0
    if ( (![UIImagePickerController isSourceTypeAvailable:UIImagePickerControllerSourceTypePhotoLibrary]) || 
        (![UIImagePickerController isSourceTypeAvailable:UIImagePickerControllerSourceTypeCamera]) ||
        (delegateObject == nil)) {
        return NO; 
    }
#endif
	
    @try {
        UIImagePickerController* picker = [[UIImagePickerController alloc] init]; 
        picker.sourceType = type;
        picker.delegate = delegateObject;
        
#ifndef __IPHONE_3_1
        if (delegateObject.settings.enable_editing != 0) {
            picker.allowsImageEditing = YES;
        }
        else {
            picker.allowsImageEditing = NO;
        }
#else
        if (delegateObject.settings.enable_editing != 0) {
            picker.allowsEditing = YES;
        }
        else {
            picker.allowsEditing = NO;
        }
#endif
        // Show picker
#ifdef __IPHONE_3_2
        if ((UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad) && (type != UIImagePickerControllerSourceTypeCamera)) {
            // WARNING!!! DO NOT USE UIPopoverController DIRECTLY HERE!!!
            // It will cause problem of starting application on iPhone OS < 3.2
            // All such stuff should be loaded in runtime using NSClassFromString
            id popover = [[NSClassFromString(@"UIPopoverController") alloc] initWithContentViewController:picker];
            [popover setDelegate:delegateObject];
            delegateObject->popover = popover;
            CGRect rect = [[[self mainView] view] frame];
            rect.origin.x += rect.size.width/4;
            rect.origin.y += rect.size.height/4;
            rect.size.width /= 2;
            rect.size.height /= 2;
            [popover setPopoverContentSize:CGSizeMake(CGRectGetWidth(rect), CGRectGetHeight(rect))];
            [popover presentPopoverFromRect:rect inView:[[self mainView] view] permittedArrowDirections:UIPopoverArrowDirectionAny animated:YES];
        }
        else 
#endif
        {
            if (delegateObject.settings.camera_type == CAMERA_SETTINGS_TYPE_FRONT) {
                picker.cameraDevice = UIImagePickerControllerCameraDeviceFront;
            }
            
            //[window addSubview:picker.view];
            [[mainView getMainViewController] presentModalViewController:picker animated:NO];
        }
    } @catch(NSException* theException) {
        RAWLOG_ERROR2("startCameraPickerFromViewController failed(%s): %s", [[theException name] UTF8String], [[theException reason] UTF8String] );
        return NO;
    }
	
	return YES;
}

- (void)takePicture:(RhoCameraSettings*) settings {
    if (!rho_rhodesapp_check_mode())
        return;
    [pickImageDelegate setPostUrl:settings.callback_url];
    pickImageDelegate.settings = settings;
    [self startCameraPicker:pickImageDelegate 
                 sourceType:UIImagePickerControllerSourceTypeCamera];
}

- (void)takeSignature:(NSString*)url {
    if (!rho_rhodesapp_check_mode())
        return;
	[self hideSplash];
	[signatureDelegate setPostUrl:url];
	//[signatureDelegate setImageFormat:format];
	[signatureDelegate setParentView:window];
	[signatureDelegate setPrevView:mainView.view];
	@try {
		CGRect rect = window.frame;
		rect.origin.x = 0;
		rect.origin.y = 0;
		SignatureViewController* svc = [[SignatureViewController alloc] initWithRect:rect delegate:signatureDelegate];
		[signatureDelegate setSignatureViewControllerValue:svc];
        
        [[mainView getMainViewController] presentModalViewController:svc animated:YES];
 
        
        
		//[mainView.view retain];
		//[mainView.view removeFromSuperview];
		//[window addSubview:svc.view];
    } @catch(NSException* theException) {
        RAWLOG_ERROR2("startSignatureViewController failed(%s): %s", [[theException name] UTF8String], [[theException reason] UTF8String] );
    }
    
}

- (void)openFullScreenNativeView:(UIView*)view {
    if (!rho_rhodesapp_check_mode())
        return;
	[self hideSplash];
	[nvDelegate setParentView:window];
	[nvDelegate setPrevView:mainView.view];
	@try {
		CGRect rect = window.frame;
		rect.origin.x = 0;
		rect.origin.y = 0;
		NVViewController* svc = [[NVViewController alloc] initWithRect:rect nvview:view delegate:nvDelegate];
		[nvDelegate setNVViewControllerValue:svc];

		[mainView.view retain];
		[mainView.view removeFromSuperview];
		[window addSubview:svc.view];
		
		[window layoutSubviews];
		[window setNeedsDisplay];
		
	} @catch(NSException* theException) {
        RAWLOG_ERROR2("startFullSreeenNativeViewViewController failed(%s): %s", [[theException name] UTF8String], [[theException reason] UTF8String] );
    }
}

- (void)closeFullScreenNativeView {
	[nvDelegate doClose];
}


- (void)choosePicture:(RhoCameraSettings*) settings {
    if (!rho_rhodesapp_check_mode())
        return;
    [pickImageDelegate setPostUrl:settings.callback_url];
    pickImageDelegate.settings = settings;
    [self startCameraPicker:pickImageDelegate 
                 sourceType:UIImagePickerControllerSourceTypePhotoLibrary];
}

- (UIWindow*)rootWindow {
    return window;
}

- (void)hideSplash {
    if (splashViewController) {
        [splashViewController hideSplash];
        [splashViewController release];
        splashViewController = nil;
		
		[window addSubview:mainView.view];
		[window bringSubviewToFront:mainView.view];
    }
}

- (void)setMainView:(id<RhoMainView,NSObject>)view {
    if (mainView == view)
        return;

	UIViewController* sv = mainView;
	//[self hideSplash];
    
	// Special hack for solve problem with VerticalTabBar on iPad
	// When we switch from VerticalTabBar to any View - autorotation is disabled and application switch to Portrait orientation
	// But there are no problme on iPhone 4.1 and above
	//I think this is special iPad 3.2 problem
    BOOL isVerticalTab = NO;
	if ([mainView isKindOfClass:[SplittedMainView class]]) {
		BOOL is_iPad = NO;
		
		NSString *model = [[UIDevice currentDevice] model]; // "iPad ..."
		if ([model hasPrefix:@"iPad"]) {
			is_iPad = YES;
		}
		
		isVerticalTab = is_iPad;
	}
	
	CGRect main_frame = mainView.view.frame;
	
	UIWindow* www = window;
	if (isVerticalTab) {
		
		window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
		window.autoresizingMask = UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleWidth;
		window.autoresizesSubviews = YES;
		
		//view.view.frame = main_frame;
		mainView = nil;
	}
	else {
		[mainView.view removeFromSuperview];
		[mainView release];
	}
	
    mainView = [view retain];
    [window addSubview:mainView.view];
	//[window bringSubviewToFront:mainView.view];
	
	if (isVerticalTab) {
		[window makeKeyAndVisible];

		[sv.view removeFromSuperview];
		[sv release];
		[www release];
	}
	[window layoutSubviews];
	
}

- (id<RhoMainView,NSObject>)mainView {
    return mainView;
}



// make splash screen
// return YES if SplashScreen maked 
- (BOOL) showLoadingPagePre 
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    
    NSString *resourcePath = [[NSBundle mainBundle] resourcePath];
    NSString *htmPath = [NSString stringWithFormat:@"%@/apps/app/loading.html", resourcePath];
    
    if ([SplashViewController hasLoadingImage]) {
        splashViewController = [[SplashViewController alloc] initWithParentView:window];
    }
    else if ([fileManager fileExistsAtPath:htmPath]) {
        NSError *err;
        NSString *data = [NSString stringWithContentsOfFile:htmPath encoding:NSUTF8StringEncoding error:&err];
        [mainView loadHTMLString:data];
    }
	return [SplashViewController hasLoadingImage];
}

// execute rho_splash_screen_start(); - we can do it only after Rhodes initialization
- (void) showLoadingPagePost
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *resourcePath = [[NSBundle mainBundle] resourcePath];
    NSString *htmPath = [NSString stringWithFormat:@"%@/apps/app/loading.html", resourcePath];
    
    if (splashViewController != nil) {
		rho_splash_screen_start();
    }
    else if ([fileManager fileExistsAtPath:htmPath]) {
		rho_splash_screen_start();
		
    }
}


- (void)doRhoActivate {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    @try {
        NSLog(@"doRhoActivate thread started");
        while (!app_created)
            [NSThread sleepForTimeInterval:0.1];
        [Rhodes performOnUiThread:[RhoActivateTask class] wait:NO];
    }
    @finally {
        NSLog(@"doRhoActivate thread finished");
        [pool release];
    }
}

- (void)doRhoInit {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    @try {
        NSLog(@"Init appManager");
        appManager = [AppManager instance]; 
        //Configure AppManager
        [appManager configure];
        
        const char *szRootPath = rho_native_rhopath();
        const char *szUserPath = rho_native_rhouserpath();
        NSLog(@"Init logconf");
        rho_logconf_Init_with_separate_user_path(szRootPath, szRootPath, "", szUserPath);
        InitMemoryInfoCollector();
        NSLog(@"Create rhodes app");
        rho_rhodesapp_create_with_separate_user_path(szRootPath, szUserPath);
        app_created = YES;
        
        rotationLocked = rho_conf_getBool("disable_screen_rotation");
        
        NSLog(@"Show loading page");
        [self performSelectorOnMainThread:@selector(showLoadingPagePost) withObject:nil waitUntilDone:NO];
        
        NSLog(@"Start rhodes app");
        rho_rhodesapp_start();
		rho_rhodesapp_callUiCreatedCallback();
    }
    @finally {
        [pool release];
    }
    
    if (rho_conf_getBool("full_screen") != 0) {
        [Rhodes setStatusBarHidden:YES];
    }
    
}

- (void)doStartUp {
    NSLog(@"Rhodes starting application...");
    
    
    [NSThread setThreadPriority:1.0];
    
    NSLog(@"Create new detached thread for initialization stuff");
    [NSThread detachNewThreadSelector:@selector(doRhoInit) toTarget:self withObject:nil];
    
    NSLog(@"Init all windows");
    
    
    [[UIApplication sharedApplication] setStatusBarStyle:UIStatusBarStyleDefault animated:NO];
    
    window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    window.autoresizingMask = UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleWidth;
    window.autoresizesSubviews = YES;
	
    mainView = nil;
    mainView = [[SimpleMainView alloc] initWithParentView:window frame:[Rhodes applicationFrame]];
    mainView.view.autoresizingMask = UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleWidth;
    mainView.view.autoresizesSubviews = YES;
    
	BOOL is_splash_screen_maked = [self showLoadingPagePre];
	
	if (!is_splash_screen_maked) {
		[window addSubview:mainView.view];
	}
	
    [window makeKeyAndVisible];

 
	CGRect rrr = [application statusBarFrame];
	
    NSLog(@"Init cookies");
    cookies = [[NSMutableDictionary alloc] initWithCapacity:0];
    
    // Init controllers
    NSLog(@"Init controllers");
    logOptionsController = [[LogOptionsController alloc] init];
    logViewController = [[LogViewController alloc] init];
    
    NSLog(@"Init delegates");
    dateTimePickerDelegate = [[DateTimePickerDelegate alloc] init];
    pickImageDelegate = [[PickImageDelegate alloc] init];
    signatureDelegate = [SignatureDelegate getSharedInstance];
    nvDelegate = [[NVDelegate alloc] init];
    
#ifdef APP_BUILD_CAPABILITY_PUSH    
#ifdef __IPHONE_3_0
    [[UIApplication sharedApplication] registerForRemoteNotificationTypes:
     (UIRemoteNotificationTypeBadge | UIRemoteNotificationTypeAlert | UIRemoteNotificationTypeSound)];
#endif //__IPHONE_3_0
#endif //APP_BUILD_CAPABILITY_PUSH    
    
#ifdef __IPHONE_4_0
    eventStore = [[EKEventStore alloc] init];
#endif

    NSLog(@"Initialization finished");
}

#ifdef __IPHONE_3_0
- (void)processDoSync:(NSDictionary *)userInfo
{
	NSArray *do_sync = [userInfo objectForKey:@"do_sync"];
	if (do_sync) {
		NSEnumerator *enumerator = [do_sync objectEnumerator];
		id url;
		
		NSLog(@"do_sync array: ");
		bool sync_all = false;
		while ( url = [enumerator nextObject] ) {
			NSLog( @"url = %@", url );
			if ([@"all" caseInsensitiveCompare:url] == NSOrderedSame) {
				sync_all = true;
			} else {
				//do sync of individual source
				NSString* srcUrl = [url stringByTrimmingCharactersInSet: [NSCharacterSet characterSetWithCharactersInString:@" \t\r\n"]];
				rho_sync_doSyncSourceByName([srcUrl cStringUsingEncoding:[NSString defaultCStringEncoding]]);
			}
		}
		
		if (sync_all) {
			rho_sync_doSyncAllSources(TRUE,"");
		}
	}
}
- (void)processPushMessage:(NSDictionary *)userInfo
{
    RAWLOG_INFO("Processing PUSH message...");
	
    {
		NSMutableString* strData = [[NSMutableString alloc] init];
		for (NSString* key in userInfo) 
		{
			if ( !key )
				continue;
			
		    NSLog(@"Push item: %@", key );
			if ( [key compare:@"aps"] == 0)
			{
				NSDictionary *aps = [userInfo objectForKey:key];
				for (NSString* key1 in aps) 
				{
					if ( !key1 )
						continue;
				    NSLog(@"Push aps item: %@", key1 );
					
					if ( [strData length] > 0 )
						[strData appendString:@"&"];					
					
					[strData appendString:key1];
					[strData appendString:@"="];
					if ( [aps objectForKey:key1] )
						[strData appendString:[NSString stringWithFormat:@"%@", [aps objectForKey:key1]]];
				}
				
				continue;
			}

			if ( [strData length] > 0 )
				[strData appendString:@"&"];					
			[strData appendString:key];
			[strData appendString:@"="];
			if ( [userInfo objectForKey:key] )
				[strData appendString:[NSString stringWithFormat:@"%@", [userInfo objectForKey:key]]];

		}	
		
//        NSString* strData = [userInfo description];
        NSLog(@"Push string: %@", strData );
        const char* szData = [strData cStringUsingEncoding:[NSString defaultCStringEncoding]];
        int nRes = rho_rhodesapp_callPushCallback(szData);
        [strData release];
        if ( nRes )
            return;
    }
    
    NSDictionary *aps = [userInfo objectForKey:@"aps"];
    if (aps) {
        NSString *alert = [aps objectForKey:@"alert"];
        if (alert && [alert length] > 0) {
            NSLog(@"Push Alert: %@", alert);
            rho_param *p = rho_param_str((char*)[alert UTF8String]);
            [RhoAlert showPopup:p];
            rho_param_free(p);
        }
        NSString *sound = [aps objectForKey:@"sound"];
        if (sound && [sound length] > 0) {
            NSLog(@"Sound file name: %@", sound);
            [RhoAlert playFile:[@"/public/alerts/" stringByAppendingPathComponent:sound] mediaType:NULL];
        }
        NSString *vibrate = [aps objectForKey:@"vibrate"];
        if (vibrate && [vibrate length] > 0) {
            NSLog(@"Do vibrate...");
            [RhoAlert vibrate:1];
        }
    }
    [self processDoSync:userInfo];
}
#endif

- (void) saveLastUsedTime {
}

- (void)playStart:(NSString *)fileName mediaType:(NSString *)type {
    NSString *soundFilePath;
	//hack to work around iphone limitation when it will play push alerts only from the main bundle root
	if ([fileName hasPrefix:@"/public/alerts/"] || [fileName hasPrefix:@"/apps/public/alerts/"]) {
		NSString *file = [fileName lastPathComponent];
		soundFilePath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:file];		
	} else {
		soundFilePath = [[AppManager getApplicationsRootPath] stringByAppendingPathComponent:fileName];
	}
	NSLog(@"Playing %@: ", soundFilePath);
	
	NSURL *fileURL = [[NSURL alloc] initFileURLWithPath: soundFilePath];
	NSError* err = nil;
	AVAudioPlayer *newPlayer = [[AVAudioPlayer alloc] initWithContentsOfURL:fileURL error:&err];
	NSLog(@"Init media player returns: %@", err);
	
	[fileURL release];
	self.player = newPlayer;
	[newPlayer release];
	
	[player prepareToPlay];	
	[player setDelegate: self];
	[self.player play];
}

- (void)playStop {
    if (!self.player)
        return;
    
    [self.player stop];
    self.player = nil;
}

// AVAudioPlayerDelegate implementation

- (void) audioPlayerDidFinishPlaying: (AVAudioPlayer *) player successfully: (BOOL) flag {
	if (flag == YES) {
		NSLog(@"Audio player finished playing...");
	}	
}

- (void)audioPlayerDecodeErrorDidOccur:(AVAudioPlayer *)player error:(NSError *)error {
	NSLog(@"Audio player decoding error %@", error);
}










- (void)doStartUpRe:(NSObject*)arg {
    NSLog(@"Rhodes starting application...");
    rotationLocked = NO;
    
    NSLog(@"Create new detached thread for initialization stuff");
    [NSThread detachNewThreadSelector:@selector(doRhoInit) toTarget:self withObject:nil];
    
    if (mainView != nil) {
        [mainView.view removeFromSuperview];
        [mainView release];
    }
    
    mainView = nil;
    [SimpleMainView enableHiddenOnStart];
    mainView = [[SimpleMainView alloc] initWithParentView:window frame:[Rhodes applicationFrame]];
    mainView.view.autoresizingMask = UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleWidth;
    mainView.view.autoresizesSubviews = YES;
    
	BOOL is_splash_screen_maked = [self showLoadingPagePre];
	
	if (!is_splash_screen_maked) {
		[window addSubview:mainView.view];
	}
	
    NSLog(@"Init cookies");
    cookies = [[NSMutableDictionary alloc] initWithCapacity:0];
    
    rho_rhodesapp_canstartapp("", ", ");
    
    NSLog(@"Initialization finished");
}





-(void)restart_app_func
{
    [self performSelectorOnMainThread:@selector(doStartUpRe:) withObject:nil waitUntilDone:NO];
}

+(void)restart_app
{
    [[Rhodes sharedInstance] restart_app_func];

}


// UIApplicationDelegate implementation

#ifdef __IPHONE_3_0
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    
    
    self.mBlockExit = NO;
    
    
    instance = self;
    self->application = [UIApplication sharedApplication];
    rotationLocked = NO;
    
    if ( !rho_sys_check_rollback_bundle(rho_native_rhopath()) )
    {
        
        rho_sys_impl_exit_with_errormessage( "Bundle update.", "Application is currupted. Reinstall it , please.");
        return NO;
    }    
    
    

	NSURL* url = [launchOptions objectForKey:UIApplicationLaunchOptionsURLKey];
    NSLog(@"didFinishLaunchingWithOptions: %@", url);
	
    
	// store start parameter
	NSString* start_parameter = [NSString stringWithUTF8String:""];
	if (url != nil) {
		NSString* url_str = [url absoluteString];
		if (url_str != nil) {
			NSRange range = [url_str rangeOfString:@":"];
			if ((range.location >= 0) && (range.length > 0)) {
				start_parameter = [url_str substringFromIndex:range.location + 1];
			}
		}
	}	

    [self doStartUp];
    [self processDoSync:launchOptions];

    if ( !rho_rhodesapp_canstartapp([start_parameter UTF8String], ", ") )
    {
        //const char* szAppSecToken = get_app_build_config_item("security_token");
		NSLog(@"This is hidden app and can be started only with security key.");
        /*
        {
            
            NSString* message = @"Invalid security token !";
            
            rho_conf_Init(rho_native_rhopath());
            
            if (rho_conf_is_property_exists("invalid_security_token_message")) {
                const char* conf_message = rho_conf_getString("invalid_security_token_message");
                message = [NSString stringWithUTF8String:conf_message];
            }
            
            
            UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"" 
                                                            message:message
                                                           delegate:self 
                                                  cancelButtonTitle:@"OK" 
                                                  otherButtonTitles: nil];
            [alert show];
            [alert release];
        }
        */
        //exit(EXIT_SUCCESS);
    }
    if (!rho_can_app_started_with_current_licence()) {
		NSLog(@"############################");
		NSLog(@" ");
		NSLog(@"ERROR: motorola_licence is INVALID !");
		NSLog(@" ");
		NSLog(@"############################");
        //exit(EXIT_SUCCESS);
        [self exit_with_errormessage:@"Motorola Licence" message:@"Your licence key is invalid !"];
    }
	
	return NO;
}

- (void) exit_with_errormessage:(NSString*)title message:(NSString*)message 
{
    [Rhodes sharedInstance].mBlockExit = YES;
    
    UIAlertView *alert = [[UIAlertView alloc] initWithTitle:title 
                                                    message:message
                                                   delegate:self 
                                          cancelButtonTitle:@"OK" 
                                          otherButtonTitles: nil];
    [alert show];
    [alert release];
    
}


- (void)alertView:(UIAlertView *)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex {
    exit(EXIT_SUCCESS);
}



#endif
/*
- (void)applicationDidFinishLaunching:(UIApplication *)application 
{
	[self doStartUp];
} */


#ifdef APP_BUILD_CAPABILITY_PUSH

#ifdef __IPHONE_3_0

- (void)application:(UIApplication *)application didRegisterForRemoteNotificationsWithDeviceToken:(NSData *)deviceToken
{
	NSLog(@"Device token is %@", deviceToken);
	
	NSMutableString *stringBuffer = [NSMutableString stringWithCapacity:([deviceToken length] * 2)];
	const unsigned char *dataBuffer = [deviceToken bytes];
	for (int i = 0; i < [deviceToken length]; ++i)
		[stringBuffer appendFormat:@"%02x", (unsigned long)dataBuffer[ i ]];
	
	char* szpin = strdup([stringBuffer cStringUsingEncoding:[NSString defaultCStringEncoding]]);
	RAWLOG_INFO1("device pin: %s\n", szpin);

	rho_clientregister_create(szpin);
	free(szpin);
}

- (void)application:(UIApplication *)application didFailToRegisterForRemoteNotificationsWithError:(NSError *)error
{
	NSLog(@"Push Notification Error: %@", [error localizedDescription]);
}

- (void)application:(UIApplication *)application didReceiveRemoteNotification:(NSDictionary *)userInfo
{
	[self processPushMessage:userInfo];
}

#endif //__IPHONE_3_0

#endif //APP_BUILD_CAPABILITY_PUSH

- (void)applicationDidBecomeActive:(UIApplication *)application {
    RAWLOG_INFO("Application did become active");
    if (!app_created) {
        NSLog(@"Application is not created yet so postpone activation callback");
        [NSThread detachNewThreadSelector:@selector(doRhoActivate) toTarget:self withObject:nil];
    }
    else
        [Rhodes performOnUiThread:[RhoActivateTask class] wait:NO];
}

- (void)applicationWillResignActive:(UIApplication *)application {
    RAWLOG_INFO("Application will resign active");
    rho_rhodesapp_callAppActiveCallback(0);
    rho_rhodesapp_canstartapp("", ", ");
    [self saveLastUsedTime];
}

#ifdef __IPHONE_4_0
- (void)applicationDidEnterBackground:(UIApplication *)application {
    RAWLOG_INFO("Application go to background");
    rho_rhodesapp_callUiDestroyedCallback();
    rho_rhodesapp_canstartapp("", ", ");
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Nothing right now
}
#endif


- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application
{
    RAWLOG_INFO("Application receive Memory Warning !!!");
    /*
    int level = OSMemoryNotificationCurrentLevel();
    
    int o;
    o = 9;
    */
}

- (void)applicationWillTerminate:(UIApplication *)application {
    RAWLOG_INFO("Runner will terminate");
    rho_rhodesapp_callUiDestroyedCallback();
    [self saveLastUsedTime];
    rho_rhodesapp_destroy();
}

// UITabBarControllerDelegate implementation

- (void)tabBarController:(UITabBarController *)tabBarController
 didSelectViewController:(UIViewController *)viewController {
    SEL sel = @selector(onSwitchTab);
    if ([mainView respondsToSelector:sel])
        [mainView performSelector:sel];
}

- (BOOL)application:(UIApplication *)application handleOpenURL:(NSURL *)url
{
    if (!url) {  return NO; }

    NSBundle* mb = [NSBundle mainBundle];
    NSDictionary* md = [mb infoDictionary];
    NSArray* schemes = [md objectForKey:@"CFBundleURLTypes"];
    NSDictionary* scheme = [schemes objectAtIndex:0];
    NSArray* urls = [scheme objectForKey:@"CFBundleURLSchemes"];
    NSString* url0 = [urls objectAtIndex:0];
    
    if ([[url scheme] isEqualToString:url0]) {
        NSString *startparams = @"";
        NSString *fullurl = [[url absoluteString] stringByReplacingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
        
        NSRange range = [fullurl rangeOfString:@":"];
        if ((range.location > 0) && (range.length > 0)) {
            startparams = [fullurl substringFromIndex:range.location + 1];
        }

        // set start params
        rho_rhodesapp_canstartapp([startparams UTF8String], ", ");
        return YES;
    }
    return NO;	
    /*
    NSString *URLString = [url absoluteString];
    [[NSUserDefaults standardUserDefaults] setObject:URLString forKey:@"url"];
    [[NSUserDefaults standardUserDefaults] synchronize];

	//NSString* msg = @"handleOpenURL: ";
	//msg = [msg stringByAppendingString:URLString];
	
	UIAlertView *alert = [[[UIAlertView alloc]
						   initWithTitle:@"handleOpenURL:"
						   message:URLString
						   delegate:self
						   cancelButtonTitle:@"Close"
						   otherButtonTitles:nil] autorelease];
    
    [alert show];
	*/
    return YES;
}



@end

// Native functions


void rho_sys_impl_exit_with_errormessage(const char* szTitle, const char* szMsg) {

    [[Rhodes sharedInstance] exit_with_errormessage:[NSString stringWithUTF8String:szTitle] message:[NSString stringWithUTF8String:szMsg]];
}


void rho_map_location(char* query) {
    [[Rhodes sharedInstance] mapLocation:[NSString stringWithUTF8String:query]];
}

void _rho_ext_syscall(PARAMS_WRAPPER* params) {
	[[Rhodes sharedInstance] sysCall:params];
}

void rho_conf_show_log() {
    [[Rhodes sharedInstance] showLog];
}
