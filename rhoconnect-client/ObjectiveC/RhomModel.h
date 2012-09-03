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

#import <Foundation/Foundation.h>

#import "RhoConnectNotify.h"

@interface RhomModel : NSObject {
	NSString* name;
    NSString* partition;
	int       sync_type; 
    int       model_type;
    int       source_id; //generated when insert to database
    
    NSDictionary* associations;
    NSMutableString* blob_attribs;
}

@property(retain) NSString* name;
@property(retain) NSString* partition;
@property(assign) int       sync_type;
@property(assign) int       source_id;
@property(assign) int       model_type;
@property(retain) NSDictionary* associations;
@property(retain, readonly) NSString* blob_attribs;

- (id) init;
- (void) dealloc;

- (RhoConnectNotify*) sync;
- (void) sync: (SEL) callback target:(id)target;

- (void) setNotification: (SEL) callback target:(id)target;
- (void) clearNotification;

- (void) add_blob_attribute: (NSString *) attr_name;
- (void) add_blob_attribute: (NSString *) attr_name overwrite: (BOOL) attr_overwrite;

- (void) create: (NSMutableDictionary *) data;
- (NSMutableDictionary *) find: (NSString*)object_id;
- (NSMutableDictionary *) find_first: (NSDictionary *)cond;
- (NSMutableArray *) find_all: (NSDictionary *)cond;
- (NSMutableArray *) find_bysql: (NSString*)sql args: (NSArray*) sql_args;

- (void) save: (NSDictionary *)data;
- (void) destroy: (NSDictionary *)data;

- (void) startBulkUpdate;
- (void) stopBulkUpdate;

- (BOOL) is_changed;

- (void) setSyncType : (int)type;
- (void) pushChanges;

@end
