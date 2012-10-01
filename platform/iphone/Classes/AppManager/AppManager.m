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

#import <SystemConfiguration/SystemConfiguration.h>
#import <Foundation/Foundation.h>

#include "common/RhoPort.h"
#include "unzip/unzip.h"
#import "AppManager.h"
//#import "HttpContext.h"
//#import "HttpMessage.h"
//#import "Dispatcher.h"
#import "AppLoader.h"
#import "common/RhoConf.h"
#import "common/RhodesApp.h"
#import "logging/RhoLogConf.h"
#include "ruby/ext/rho/rhoruby.h"
#import "logging/RhoLog.h"
#import "../Event/Event.h"

#import "Rhodes.h"

#import "common/app_build_configs.h"

#include <sys/xattr.h>

#undef DEFAULT_LOGCATEGORY
#define DEFAULT_LOGCATEGORY "RhodesApp"

static bool UnzipApplication(const char* appRoot, const void* zipbuf, unsigned int ziplen);
//const char* RhoGetRootPath();



BOOL isPathIsSymLink(NSFileManager *fileManager, NSString* path) {
    NSError *error;
    
    NSDictionary *attributes = [fileManager attributesOfItemAtPath:path error:&error]; 
    
    if (attributes == nil) {
        //NSLog(@"     SymLink  NO : %@", path);
        return NO;
    }
    
    NSString* fileType = [attributes objectForKey:NSFileType];
    
    if (fileType == nil) {
        //NSLog(@"     SymLink  NO : %@", path);
        return NO;
    }

    BOOL res = [NSFileTypeSymbolicLink isEqualToString:fileType];
    if (res) {
        //NSLog(@"     SymLink YES : %@", path);
    }
    else {
        //NSLog(@"     SymLink  NO : %@", path);
    }
    
    return res;
}


@interface RhoFileManagerDelegate_RemoveOnly_SymLinks : NSObject {
@public
}

-(BOOL)fileManager:(NSFileManager *)fileManager shouldRemoveItemAtPath:(NSString *)path;
@end

@implementation RhoFileManagerDelegate_RemoveOnly_SymLinks


-(BOOL)fileManager:(NSFileManager *)fileManager shouldRemoveItemAtPath:(NSString *)path {
    return isPathIsSymLink(fileManager, path);
}

@end



@implementation AppManager

+ (AppManager *)instance
{
	static AppManager *gInstance = NULL;
	@synchronized(self)
    {
		if (gInstance == NULL)
			gInstance = [[self alloc] init];
    }
	return(gInstance);
}

/*
 * Gets root folder of the site
 * Application folders located undern the root
 */
+ (NSString *) getApplicationsRootPath {
	NSString *documentsDirectory = [NSString stringWithUTF8String:rho_native_rhopath()];
	return [documentsDirectory stringByAppendingPathComponent:@"apps"];
}

+ (NSString *) getDbPath {
	NSString *documentsDirectory = [NSString stringWithUTF8String:rho_native_rhodbpath()];
	return [documentsDirectory stringByAppendingPathComponent:@"db"];
}

+ (NSString *) getApplicationsUserPath {
	return [NSString stringWithUTF8String:rho_native_rhouserpath()];
}


+ (NSString *) getApplicationsRosterUrl {
	return @"http://dev.rhomobile.com/vlad/";
}

+ (bool) installApplication:(NSString*)appName data:(NSData*)appData {
	NSFileManager *fileManager = [NSFileManager defaultManager];
	NSString *appPath = [[AppManager getApplicationsRootPath] stringByAppendingPathComponent:appName]; 
	if ([fileManager fileExistsAtPath:appPath]) {
		NSError *error;
		[fileManager removeItemAtPath:appPath error:&error];
	}
    [fileManager createDirectoryAtPath:appPath withIntermediateDirectories: NO attributes:NULL error: NULL];

	static char appRoot[FILENAME_MAX];
	[appPath getFileSystemRepresentation:appRoot maxLength:sizeof(appRoot)];
	return UnzipApplication( appRoot, [appData bytes], [appData length]);
}

- (void) copyFromMainBundle:(NSFileManager*)fileManager fromPath:(NSString *)source
					 toPath:(NSString*)target remove:(BOOL)remove {
	BOOL dir;
	if(![fileManager fileExistsAtPath:source isDirectory:&dir]) {
		NSAssert1(0, @"Source item '%@' does not exists in bundle", source);
		return;
	}
	
	if (!remove && dir) {
        NSError *error;
		if (![fileManager fileExistsAtPath:target])
			[fileManager createDirectoryAtPath:target withIntermediateDirectories:YES attributes:nil error:&error];
		
		NSDirectoryEnumerator *enumerator = [fileManager enumeratorAtPath:source];
		NSString *child;
		while (child = [enumerator nextObject]) {
			[self copyFromMainBundle:fileManager fromPath:[source stringByAppendingPathComponent:child]
							  toPath:[target stringByAppendingPathComponent:child] remove:NO];
		}
	}
	else {
		NSError *error;
		if ([fileManager fileExistsAtPath:target] && ![fileManager removeItemAtPath:target error:&error]) {
			NSAssert2(0, @"Failed to remove '%@': %@", target, [error localizedDescription]);
			return;
		}
		if (![fileManager copyItemAtPath:source toPath:target error:&error]) {
			NSAssert3(0, @"Failed to copy '%@' to '%@': %@", source, target, [error localizedDescription]);
			return;
		}
	}
}



- (BOOL)isContentsEqual:(NSFileManager*)fileManager first:(NSString*)filePath1 second:(NSString*)filePath2 {
    NSLog(@"filePath1: %@", filePath1);
    NSLog(@"filePath2: %@", filePath2);
    if (![fileManager fileExistsAtPath:filePath1] || ![fileManager fileExistsAtPath:filePath2])
        return NO;
    
    NSString *content1 = [[NSString alloc] initWithData:[fileManager contentsAtPath:filePath1]
                                               encoding:NSUTF8StringEncoding];
    NSString *content2 = [[NSString alloc] initWithData:[fileManager contentsAtPath:filePath2]
                                               encoding:NSUTF8StringEncoding];
    BOOL result = [content1 isEqualToString:content2];
    [content1 release];
    [content2 release];
    return result;
}


/*
 * Configures AppManager
 */
- (void) configure {
    [self configure:YES force_update_content:NO only_apps:NO];
}


- (void) configure:(BOOL)make_sym_links force_update_content:(BOOL)force_update_content only_apps:(BOOL)only_apps {
	
//#define RHO_DONT_COPY_ON_START
    
    NSFileManager *fileManager = [NSFileManager defaultManager];
    
	NSString *bundleRoot = [[NSBundle mainBundle] resourcePath];
	NSString *rhoRoot = [NSString stringWithUTF8String:rho_native_rhopath()];
	NSString *rhoUserRoot = [NSString stringWithUTF8String:rho_native_rhouserpath()];
    NSString *rhoDBRoot = [NSString stringWithUTF8String:rho_native_rhodbpath()]; 

	NSString *filePathNew = [bundleRoot stringByAppendingPathComponent:@"name"];
	NSString *filePathOld = [rhoRoot stringByAppendingPathComponent:@"name"];
//#ifndef RHO_DONT_COPY_ON_START
    BOOL hasOldName = [fileManager fileExistsAtPath:filePathOld];
//#endif
    BOOL nameChanged = ![self isContentsEqual:fileManager first:filePathNew second:filePathOld];

    BOOL restoreSymLinks_only = NO;

    BOOL contentChanged = force_update_content;
    if (nameChanged)
        contentChanged = YES;
	else {
		filePathNew = [bundleRoot stringByAppendingPathComponent:@"hash"];
		filePathOld = [rhoRoot stringByAppendingPathComponent:@"hash"];

        contentChanged = ![self isContentsEqual:fileManager first:filePathNew second:filePathOld];
        
        // check for lost sym-links (upgrade OS or reinstall application without change version)
        if (!contentChanged) {
            // check exist of sym-link
            NSString* testName = [rhoRoot stringByAppendingPathComponent:@"lib"];
            if (![fileManager fileExistsAtPath:testName]) {
                NSLog(@" Can not found main Sym-Link - we should restore all sym-links !");
                contentChanged = YES;
                restoreSymLinks_only = YES;
            }
            else {
                NSLog(@" Main Sym-Link founded - disable restoring !");
            }
        }
        
	}

    NSString* testName = [rhoRoot stringByAppendingPathComponent:@"lib"];
    BOOL libExist = [fileManager fileExistsAtPath:testName];
    if (libExist) {
        NSLog(@" Lib File is Exist: %@", testName);
    }
    else {
        NSLog(@" Lib File is NOT Exist: %@", testName);
    }
	
    
    if (contentChanged) {
        if (make_sym_links) {
//#ifdef RHO_DONT_COPY_ON_START
            // we have next situations when we should remove old content:
            // 1. we upgrade old version (where we copy all files)
            //    we should remove all files
            // 2. we upgrade version with symlinks
            //    we should remove only symlinks
            // 3. we should only restore sym-lins after that was cleared - OS upgrade/reinstall app with the same version/restore from bakup etc. 
            // we check old "lib" file - if it is SymLink then we have new version of Rhodes (with SymLinks instead of files)
            
            BOOL isNewVersion = isPathIsSymLink(fileManager, testName);
            
            RhoFileManagerDelegate_RemoveOnly_SymLinks* myDelegate = nil;
            if (isNewVersion) {
                myDelegate = [[RhoFileManagerDelegate_RemoveOnly_SymLinks alloc] init];
                [fileManager setDelegate:myDelegate];
            }
            
            NSError *error;
            
            NSString *appsDocDir = [rhoUserRoot stringByAppendingPathComponent:@"apps"];
            [fileManager createDirectoryAtPath:rhoRoot withIntermediateDirectories:YES attributes:nil error:&error];
            
            [fileManager createDirectoryAtPath:appsDocDir withIntermediateDirectories:YES attributes:nil error:&error];
            
            // Create symlink to "lib"
            NSString *src = [bundleRoot stringByAppendingPathComponent:@"lib"];
            NSLog(@"src: %@", src);
            NSString *dst = [rhoRoot stringByAppendingPathComponent:@"lib"];
            NSLog(@"dst: %@", dst);
            [fileManager removeItemAtPath:dst error:&error];
            
            [fileManager createSymbolicLinkAtPath:dst withDestinationPath:src error:&error];
            //[self copyFromMainBundle:fileManager fromPath:src toPath:dst remove:YES];
            
            NSString *dirs[] = {@"apps", @"db"};
            for (int i = 0, lim = sizeof(dirs)/sizeof(dirs[0]); i < lim; ++i) {
                // Create directory
                src = [bundleRoot stringByAppendingPathComponent:dirs[i]];
                NSLog(@"src: %@", src);
                dst = [rhoRoot stringByAppendingPathComponent:dirs[i]];
                NSLog(@"dst: %@", dst);
                if (![fileManager fileExistsAtPath:dst])
                    [fileManager createDirectoryAtPath:dst withIntermediateDirectories:YES attributes:nil error:&error];
                
                // And make symlinks from its content
                
                NSArray *subelements = [fileManager contentsOfDirectoryAtPath:src error:&error];
                for (int i = 0, lim = [subelements count]; i < lim; ++i) {
                    NSString *child = [subelements objectAtIndex:i];
                    NSString *fchild = [src stringByAppendingPathComponent:child];
                    NSLog(@" .. src: %@", fchild);
                    NSString *target = [dst stringByAppendingPathComponent:child];
                    NSLog(@" .. dst: %@", target);
                    [fileManager removeItemAtPath:target error:&error];
                    if ([child isEqualToString:@"rhoconfig.txt"]) {
                        [fileManager setDelegate:nil];
                        [fileManager removeItemAtPath:target error:&error];
                        [fileManager setDelegate:myDelegate];
                        [fileManager copyItemAtPath:fchild toPath:target error:&error];
                    }
                    else {
                        [fileManager createSymbolicLinkAtPath:target withDestinationPath:fchild error:&error];
                    }
                    //[self addSkipBackupAttributeToItemAtURL:target];
                }
            }
            
            // make symlinks for db files
            
            [fileManager setDelegate:nil];
            if (myDelegate != nil) {
                [myDelegate release];
            }
            // copy "db"
            
            
            NSString* exclude_db[] = {@"syncdb.schema", @"syncdb.triggers", @"syncdb_java.triggers"};
            
            if (!restoreSymLinks_only && !only_apps) { 
                NSString *copy_dirs[] = {@"db"};
                for (int i = 0, lim = sizeof(copy_dirs)/sizeof(copy_dirs[0]); i < lim; ++i) {
                    BOOL remove = nameChanged;
                    if ([copy_dirs[i] isEqualToString:@"db"] && !hasOldName)
                        remove = NO;
                    NSString *src = [bundleRoot stringByAppendingPathComponent:copy_dirs[i]];
                    NSLog(@"copy src: %@", src);
                    NSString *dst = [rhoDBRoot stringByAppendingPathComponent:copy_dirs[i]];
                    NSLog(@"copy dst: %@", dst);
                    
                    //[self copyFromMainBundle:fileManager fromPath:src toPath:dst remove:remove];
                    
                    NSArray *subelements = [fileManager contentsOfDirectoryAtPath:src error:&error];
                    for (int i = 0, lim = [subelements count]; i < lim; ++i) {
                        NSString *child = [subelements objectAtIndex:i];
                        NSString *fchild = [src stringByAppendingPathComponent:child];
                        NSLog(@" .. copy src: %@", fchild);
                        NSString *target = [dst stringByAppendingPathComponent:child];
                        NSLog(@" .. copy dst: %@", target);
                        
                        BOOL copyit = YES;
                        
                        int j, jlim;
                        for (j = 0, jlim = sizeof(exclude_db)/sizeof(exclude_db[0]); j < jlim; j++) {
                            if ([child isEqualToString:exclude_db[j]]) {
                                copyit = NO;
                            }
                        }
                        
                        if (copyit) {
                            [fileManager removeItemAtPath:target error:&error];
                            [fileManager copyItemAtPath:fchild toPath:target error:&error];
                        }
                    }
                    
                }
                // Finally, copy "hash" and "name" files
                NSString *items[] = {@"hash", @"name"};
                for (int i = 0, lim = sizeof(items)/sizeof(items[0]); i < lim; ++i) {
                    NSString *src = [bundleRoot stringByAppendingPathComponent:items[i]];
                    NSLog(@"copy src: %@", src);
                    NSString *dst = [rhoRoot stringByAppendingPathComponent:items[i]];
                    NSLog(@"copy dst: %@", dst);
                    [fileManager removeItemAtPath:dst error:&error];
                    [fileManager copyItemAtPath:src toPath:dst error:&error];
                    
                    //[self addSkipBackupAttributeToItemAtURL:dst];
                }
            }
            
        }
        else {
            NSString *dirs[] = {@"apps", @"lib", @"db", @"hash", @"name"};
            for (int i = 0, lim = sizeof(dirs)/sizeof(dirs[0]); i < lim; ++i) {
                if (!only_apps || [dirs[i] isEqualToString:@"apps"]) {
                    BOOL remove = nameChanged;
                    if ([dirs[i] isEqualToString:@"db"] && !hasOldName)
                        remove = NO;
                    NSString *src = [bundleRoot stringByAppendingPathComponent:dirs[i]];
                    NSLog(@"src: %@", src);
                    NSString *dst = [rhoRoot stringByAppendingPathComponent:dirs[i]];
                    NSLog(@"dst: %@", dst);
                    [self copyFromMainBundle:fileManager fromPath:src toPath:dst remove:remove];
                }
            }
        }
	}
    
	rho_logconf_Init_with_separate_user_path(rho_native_rhopath(), "", rho_native_rhouserpath());
	rho_rhodesapp_create_with_separate_user_path(rho_native_rhopath(), rho_native_rhouserpath());
	RAWLOG_INFO("Rhodes started");
}


- (UIViewController *) documentInteractionControllerViewControllerForPreview: (UIDocumentInteractionController *) controller {
    
    return [[Rhodes sharedInstance] mainView]; 
    
}

- (void)openDocInteractCommand:(NSString*)url {
    if (NSClassFromString(@"UIDocumentInteractionController")) {
        NSURL *fileURL = [NSURL fileURLWithPath:url];
        
        UIDocumentInteractionController* docController = [UIDocumentInteractionController interactionControllerWithURL:fileURL];
        
        docController.delegate = self;//[AppManager instance];
        
        BOOL result = [docController presentPreviewAnimated:YES];
        
        if (!result) {
            CGPoint centerPoint = [Rhodes sharedInstance].window.center;
            CGRect centerRec = CGRectMake(centerPoint.x, centerPoint.y, 0, 0);
            BOOL isValid = [docController presentOpenInMenuFromRect:centerRec inView:[Rhodes sharedInstance].window animated:YES];
        }    
    }
}

- (void)documentInteractionControllerDidEndPreview:(UIDocumentInteractionController *)docController
{
    [docController autorelease];
}

- (void)openDocInteract:(NSString*)url {
	[self performSelectorOnMainThread:@selector(openDocInteractCommand:) withObject:url waitUntilDone:NO];	
}



@end



const char* getUserPath() {
	static bool loaded = FALSE;
	static char root[FILENAME_MAX];
	if (!loaded){
		NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
		NSString *documentsDirectory = //[paths objectAtIndex:0];
		[ [paths objectAtIndex:0] stringByAppendingString:@"/"];
		[documentsDirectory getFileSystemRepresentation:root maxLength:sizeof(root)];
		
		loaded = TRUE;
	}
	
	return root;
}

const char* rho_native_rhopath() 
{
	static bool loaded = FALSE;
	static char root[FILENAME_MAX];
	if (!loaded){

        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
        NSString *rootDirectory = [ [paths objectAtIndex:0] stringByAppendingString:@"/Private Documents/"];
        
        const char* svalue = get_app_build_config_item("iphone_set_approot");
        if (svalue != NULL) {
            NSString* value = [NSString stringWithUTF8String:svalue];
            
            if ([value compare:@"Documents" options:NSCaseInsensitiveSearch] == NSOrderedSame) {
                paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
                rootDirectory = [ [paths objectAtIndex:0] stringByAppendingString:@"/"];
            }
            else if ([value compare:@"Library_Caches" options:NSCaseInsensitiveSearch] == NSOrderedSame) {
                paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
                rootDirectory = [ [paths objectAtIndex:0] stringByAppendingString:@"/Private Documents/"];
            }
            else if ([value compare:@"Library_Private_Documents" options:NSCaseInsensitiveSearch] == NSOrderedSame) {
                paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
                rootDirectory = [ [paths objectAtIndex:0] stringByAppendingString:@"/Private Documents/"];
            }
        } 
        
        [rootDirectory getFileSystemRepresentation:root maxLength:sizeof(root)];
		loaded = TRUE;
	}
	
	return root;
}

const char* rho_native_rhouserpath() 
{
    BOOL user_path_in_root = NO;
    const char* svalue = get_app_build_config_item("iphone_userpath_in_approot");
    if (svalue != NULL) {
        user_path_in_root = svalue[0] != '0';
    } 
    
    if (user_path_in_root) {
        return rho_native_rhopath();
    }

    return getUserPath();
}

const char* rho_native_rhodbpath()
{
    BOOL db_path_in_root = NO;
    const char* svalue = get_app_build_config_item("iphone_db_in_approot");
    if (svalue != NULL) {
        db_path_in_root = svalue[0] != '0';
    } 
    
    if (db_path_in_root) {
        return rho_native_rhopath();
    }
    return rho_native_rhouserpath();
}







VALUE rho_sys_get_locale() 
{
	NSString *preferredLang = [[NSLocale preferredLanguages] objectAtIndex:0];
	
	return rho_ruby_create_string( [preferredLang UTF8String] );
}

int rho_sys_get_screen_width()
{
    CGRect rect = [[UIScreen mainScreen] bounds];
    UIInterfaceOrientation current_orientation = [[UIApplication sharedApplication] statusBarOrientation];
	if ((current_orientation == UIInterfaceOrientationLandscapeLeft) || (current_orientation == UIInterfaceOrientationLandscapeRight)) {
        return rect.size.height;
	}
    return rect.size.width;
}

int rho_sys_get_screen_height()
{
    CGRect rect = [[UIScreen mainScreen] bounds];
    UIInterfaceOrientation current_orientation = [[UIApplication sharedApplication] statusBarOrientation];
	if ((current_orientation == UIInterfaceOrientationLandscapeLeft) || (current_orientation == UIInterfaceOrientationLandscapeRight)) {
        return rect.size.width;
	}
    return rect.size.height;
}

int rho_sys_set_sleeping(int sleeping)
{
	int ret = [[UIApplication sharedApplication] isIdleTimerDisabled] ? 0 : 1;
	[[UIApplication sharedApplication] setIdleTimerDisabled: (!sleeping ? YES : NO)];
    return ret;
}

void rho_sys_app_exit() {
    if (([Rhodes sharedInstance] != nil) && (![Rhodes sharedInstance].mBlockExit)) {
        exit(EXIT_SUCCESS);
    }
}

int rho_sys_is_app_installed(const char *appname) {
	NSString* app_name = [NSString stringWithUTF8String:appname];
	app_name = [app_name stringByAppendingString:@":check_for_exist"];
	NSURL* nsurl = [NSURL URLWithString:app_name];
	if ([[UIApplication sharedApplication] canOpenURL:nsurl]) {
		return 1;
	}
	return 0;
}

void rho_sys_app_uninstall(const char *appname) {
	NSLog(@"ALERT: Uninstall of applications is unsupported on iOS platfrom !!!");	
}


void rho_sys_open_url(const char* url) 
{
    RAWLOG_INFO1("rho_sys_open_url: %s", url);	
	
	NSString* strUrl = [NSString stringWithUTF8String:url];
	BOOL res = FALSE;

    BOOL fileExists = [[NSFileManager defaultManager] fileExistsAtPath:strUrl];
    if (!fileExists) {
        NSString *fixed_path = [NSString stringWithUTF8String:rho_rhodesapp_getapprootpath()];
        fixed_path = [fixed_path stringByAppendingString:strUrl];
        fileExists = [[NSFileManager defaultManager] fileExistsAtPath:fixed_path];
        if (fileExists) {
            strUrl = fixed_path;
        }
    }
    if (fileExists) {
        res = TRUE;
        [[AppManager instance] openDocInteract:strUrl];
    }
    else {
        if ([[UIApplication sharedApplication] canOpenURL:[NSURL URLWithString:strUrl]]) {
            res = [[UIApplication sharedApplication] openURL:[NSURL URLWithString:strUrl]];
        }
    }
	if ( res)
		RAWLOG_INFO("rho_sys_open_url suceeded.");	
	else
		RAWLOG_INFO("rho_sys_open_url failed.");	
}

void rho_sys_app_install(const char *url) {
    rho_sys_open_url(url);
}

void rho_sys_run_app(const char* appname, VALUE params) 
{
	NSString* app_name = [NSString stringWithUTF8String:appname];
	app_name = [app_name stringByAppendingString:@":"];

	if (params != 0) {
		//if (TYPE(params) == T_STRING) {
			char* parameter = getStringFromValue(params);
			if (parameter != NULL) {
				NSString* param = [NSString stringWithUTF8String:(const char*)parameter];
				app_name = [app_name stringByAppendingString:param];
			}
		//}
	}
    const char* full_url = [app_name UTF8String];
    RAWLOG_INFO1("rho_sys_run_app: %s", full_url);	
	

	BOOL res = FALSE;

    if ([[UIApplication sharedApplication] canOpenURL:[NSURL URLWithString:app_name]]) {
        res = [[UIApplication sharedApplication] openURL:[NSURL URLWithString:app_name]];
    }
	
	if ( res)
		RAWLOG_INFO("rho_sys_run_app suceeded.");	
	else
		RAWLOG_INFO("rho_sys_run_app failed.");	
}

void rho_sys_bring_to_front()
{
    RAWLOG_INFO("rho_sys_bring_to_front has no implementation on iPhone.");	
}

void rho_sys_report_app_started()
{
    RAWLOG_INFO("rho_sys_report_app_started has no implementation on iPhone.");	
}


extern VALUE rho_sys_has_network();

// http://www.apple.com/iphone/specs.html
static const double RHO_IPHONE_PPI = 163.0;
static const double RHO_IPHONE4_PPI = 326.0;
// http://www.apple.com/ipad/specs/
static const double RHO_IPAD_PPI = 132.0;

static float get_scale() {
    float scales = 1;//[[UIScreen mainScreen] scale];
#ifdef __IPHONE_4_0
    if ( [[UIScreen mainScreen] respondsToSelector:@selector(scale)] ) {
        scales = [[UIScreen mainScreen] scale];
    }
#endif
    return scales;
}


int rho_sysimpl_get_property(char* szPropName, VALUE* resValue)
{
    if (strcasecmp("platform", szPropName) == 0)
        {*resValue = rho_ruby_create_string("APPLE"); return 1;}
    else if (strcasecmp("locale", szPropName) == 0)
        {*resValue = rho_sys_get_locale(); return 1; }
    else if (strcasecmp("country", szPropName) == 0) {
        NSLocale *locale = [NSLocale currentLocale];
        NSString *cl = [locale objectForKey:NSLocaleCountryCode];
        *resValue = rho_ruby_create_string([cl UTF8String]);
        return 1;
    }
    else if (strcasecmp("screen_width", szPropName) == 0)
        {*resValue = rho_ruby_create_integer(rho_sys_get_screen_width()); return 1; }
    else if (strcasecmp("screen_height", szPropName) == 0)
        {*resValue = rho_ruby_create_integer(rho_sys_get_screen_height()); return 1; }
    else if (strcasecmp("real_screen_height", szPropName) == 0)
    {
        
        *resValue = rho_ruby_create_integer((int)(rho_sys_get_screen_height()*get_scale())); 
        return 1; 
    }
    else if (strcasecmp("real_screen_width", szPropName) == 0)
    {
        *resValue = rho_ruby_create_integer((int)(rho_sys_get_screen_width()*get_scale())); 
        return 1; 
    }
    else if (strcasecmp("screen_orientation", szPropName) == 0) {
        UIInterfaceOrientation current_orientation = [[UIApplication sharedApplication] statusBarOrientation];
        if ((current_orientation == UIInterfaceOrientationLandscapeLeft) || (current_orientation == UIInterfaceOrientationLandscapeRight)) {
            *resValue = rho_ruby_create_string("landscape");
        }
        else {
            *resValue = rho_ruby_create_string("portrait");
        }
        return 1;
    }
    else if (strcasecmp("has_network", szPropName) == 0)
        {*resValue = rho_sys_has_network(); return 1; }
    else if (strcasecmp("has_camera", szPropName) == 0) {
        int has_camera = [UIImagePickerController isSourceTypeAvailable:UIImagePickerControllerSourceTypeCamera];
        *resValue = rho_ruby_create_boolean(has_camera);
        return 1;
    }
    else if (strcasecmp("ppi_x", szPropName) == 0 ||
             strcasecmp("ppi_y", szPropName) == 0) {
#ifdef __IPHONE_3_2
        if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad) {
            *resValue = rho_ruby_create_double(RHO_IPAD_PPI);
            return 1;
        }
#endif
        if (get_scale() > 1.2) {
            *resValue = rho_ruby_create_double(RHO_IPHONE4_PPI);
        }
        else {
            *resValue = rho_ruby_create_double(RHO_IPHONE_PPI);
        }
        return 1;
    }
    else if (strcasecmp("device_name", szPropName) == 0) {
        NSString *model = [[UIDevice currentDevice] model];
        *resValue = rho_ruby_create_string([model UTF8String]);
        return 1;
    }
    else if (strcasecmp("os_version", szPropName) == 0) {
        NSString *version = [[UIDevice currentDevice] systemVersion];
        *resValue = rho_ruby_create_string([version UTF8String]);
        return 1;
    }
    else if (strcasecmp("webview_framework", szPropName) == 0) {
        NSString *version = [[UIDevice currentDevice] systemVersion];
        NSString *wvf = @"WEBKIT/";
        wvf = [wvf stringByAppendingString:version];
        *resValue = rho_ruby_create_string([wvf UTF8String]);
        return 1;
    }
    else if (strcasecmp("is_emulator", szPropName) == 0) {
        int bSim = 0; 
#if TARGET_IPHONE_SIMULATOR
		bSim = 1;
#endif		
		*resValue = rho_ruby_create_boolean(bSim);
        return 1;
    }else if (strcasecmp("has_calendar", szPropName) == 0) {
		int bCal = 0;
		if (is_rho_calendar_supported()) 
			bCal = 1;
		*resValue = rho_ruby_create_boolean(bCal);
		return 1;
	} 
	
	/* "device_id" property used only for PUSH technology !
	 else if (strcasecmp("device_id", szPropName) == 0) {
		NSString* uuid = [[UIDevice currentDevice] uniqueIdentifier];
        *resValue = rho_ruby_create_string([uuid UTF8String]);
        return 1;
	}
	*/

	
    /*
	 [[UIDevice currentDevice] uniqueIdentifier]
    // Removed because it's possibly dangerous: Apple could reject application
    // used such approach from its AppStore
    else if (strcasecmp("phone_number", szPropName) == 0) {
        NSString *num = [[NSUserDefaults standardUserDefaults] stringForKey:@"SBFormattedPhoneNumber"];
        if (!num)
            return 0;
        *resValue =  rho_ruby_create_string([num UTF8String]);
        return 1;
    }
    */

    return 0;
}

const char* GetApplicationsRootPath() {
	static bool loaded = FALSE;
	static char root[FILENAME_MAX];
	if (!loaded) {
		NSString *rootDirectory = [AppManager getApplicationsRootPath];
		[rootDirectory getFileSystemRepresentation:root maxLength:sizeof(root)];
		loaded = TRUE;
	}
	return root;
}

// TODO - do error checking
bool UnzipApplication(const char* appRoot, const void* zipbuf, unsigned int ziplen) {
	
	ZIPENTRY ze; 
    // Open zip file
	HZIP hz = OpenZip((void*)zipbuf, ziplen, 0);
	
	// Set base for unziping
	SetUnzipBaseDir(hz, appRoot);
	
	// Get info about the zip
	// -1 gives overall information about the zipfile
	GetZipItem(hz,-1,&ze);
	int numitems = ze.index;
	
	// Iterate through items and unzip them
	for (int zi = 0; zi<numitems; zi++)
	{ 
		//ZIPENTRY ze; 
		// fetch individual details, e.g. the item's name.
		GetZipItem(hz,zi,&ze); 
		// unzip item
		UnzipItem(hz, zi, ze.name);         
	}
	
	CloseZip(hz);
	
	return true;
}
/*
int _LoadApp(HttpContextRef context) {

	if ( (context->_request->_query!=NULL) && 
		(strlen(context->_request->_query)>0) ) {
		
		AppLoader* appLoader = [[AppLoader alloc] init];
		bool ret = [appLoader loadApplication:[NSString stringWithCString:context->_request->_query]];
		[appLoader release];
		if (!ret) {
			HttpSendErrorToTheServer(context, 500, "Error loading application");
			return -1;
		}
		
		char location[strlen(context->_request->_query)+2];
		HttpSnprintf(location, sizeof(location), "/%s", context->_request->_query);
		return HTTPRedirect(context, location);
	} 
	
	HttpSendErrorToTheServer(context, 400, "Application name to load and install is not specifyed");
	return -1;
}*/

void rho_appmanager_load( void* httpContext, const char* szQuery)
{
	if ( !szQuery || !*szQuery )
	{
		rho_http_senderror(httpContext, 400, "Application name to load and install is not specifyed");
		return;
	}
	
	AppLoader* appLoader = [[AppLoader alloc] init];
	bool ret = [appLoader loadApplication:[NSString stringWithUTF8String:szQuery]];
	[appLoader release];
	if (!ret) {
		rho_http_senderror(httpContext, 500, "Error loading application");
		return;
	}
	
	char location[strlen(szQuery)+2];
	rho_http_snprintf(location, sizeof(location), "/%s", szQuery);
    rho_http_redirect(httpContext, location);
	
	return;
}


void rho_sys_set_application_icon_badge(int badge_number) {
    [[UIApplication sharedApplication] setApplicationIconBadgeNumber:badge_number];
}

void rho_platform_restart_application() {
//    [Rhodes restart_app];
}

int rho_sys_set_do_not_bakup_attribute(const char* path, int value) {
    const char* attrName = "com.apple.MobileBackup";
    u_int8_t attrValue = value;
    
    int result = setxattr(path, attrName, &attrValue, sizeof(attrValue), 0, 0);
    
    if (result != 0) {
        NSLog(@"Can not change [do_not_bakup] attribute for path: %@", [NSString stringWithUTF8String:path]);
    }
    
    return (int)(result == 0);
}


int rho_prepare_folder_for_upgrade(const char* szPath) {
    // replace all folders/files to real folder/files in this path
    NSFileManager *fileManager = [NSFileManager defaultManager];

    NSString* main_path = [NSString stringWithUTF8String:szPath];
    
    NSDirectoryEnumerator *enumerator = [fileManager enumeratorAtPath:main_path];
    NSString *child;
    while (child = [enumerator nextObject]) {
        // check for sym_link
        NSString* child_path = [main_path stringByAppendingPathComponent:child];
        if (isPathIsSymLink(fileManager, child_path) ) {
            
            NSError *error;
            
            NSString* tmp_path = [main_path stringByAppendingPathComponent:@"temporary_path_for_copying_sym_link"];
            
            [[AppManager instance] copyFromMainBundle:fileManager fromPath:child_path toPath:tmp_path remove:NO];
            if ([fileManager removeItemAtPath:child_path error:&error] != YES) {
                return 0;
            }
            if ([fileManager moveItemAtPath:tmp_path toPath:child_path error:&error] != YES) {
                return 0;
            }
        }
    }
    return 1;
}


/*
#define MAX_ACTIONS 4
const static struct {
	char*  _name;
	struct _action {
		char *_name;
		int (*_process)(HttpContextRef context);
	} _actions[MAX_ACTIONS];
} controllers[] = {
	{"loader" , { {"load", _LoadApp}, {NULL, NULL} } },
	{NULL} 
};
	
int ExecuteAppManager(HttpContextRef context, RouteRef route) {
	char err[512];
	
	if (!route->_model) {
		HttpSendErrorToTheServer(context, 404, "Controller is not specifyed for the App Manager");
		return -1;
	}
	
	for (int i = 0; controllers[i]._name != NULL; i++ ) {
		if (!strcmp(route->_model, controllers[i]._name)) {

			if (!route->_action) {
				sprintf(err,"No action specifyed for the controller [%s]", route->_model); 
				HttpSendErrorToTheServer(context, 404, err);
				return -1;	
			}
			
			for (int n = 0; controllers[i]._actions[n]._name; n++) {
				if (!strcmp(controllers[i]._actions[n]._name, route->_action)) {
					return (controllers[i]._actions[n]._process)(context);
				}
			}

			sprintf(err,"No action [%s] found for the App Manager controller [%s]", 
					route->_action, route->_model); 
			HttpSendErrorToTheServer(context, 404, err);
			return -1;	
			
		}
	}
	
	sprintf(err,"No [%s] controller found for App Manager", route->_model); 
	HttpSendErrorToTheServer(context, 404, err);
	return -1;
}*/
