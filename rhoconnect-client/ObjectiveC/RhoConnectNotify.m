//
//  RhoConnectNotify.m
//  SyncClientTest
//
//  Created by evgeny vovchenko on 8/23/10.
//  Copyright 2010 RhoMobile. All rights reserved.
//

#import "RhoConnectNotify.h"

@implementation RhoConnectNotify

@synthesize  total_count;
@synthesize  processed_count;
@synthesize  cumulative_count;
@synthesize  source_id;
@synthesize  error_code;
@synthesize  source_name;
@synthesize  status;
@synthesize  sync_type;
@synthesize  bulk_status;
@synthesize  partition;
@synthesize  error_message;
@synthesize  callback_params;
@synthesize  notify_data;

- (id) init: (RHO_CONNECT_NOTIFY*) data
{
	self = [super init];
	
    notify_data = *data;
	total_count = data->total_count;
	processed_count = data->processed_count;	
	cumulative_count = data->cumulative_count;	
	source_id = data->source_id;	
	error_code = data->error_code;	

	if ( data->source_name )
		source_name = [[NSString alloc] initWithUTF8String: data->source_name];
	if ( data->status )
		status = [[NSString alloc] initWithUTF8String: data->status];
	if ( data->sync_type )
		sync_type = [[NSString alloc] initWithUTF8String: data->sync_type];
	if ( data->bulk_status )
		bulk_status = [[NSString alloc] initWithUTF8String: data->bulk_status];
	if ( data->partition )
		partition = [[NSString alloc] initWithUTF8String: data->partition];
		
	if ( data->error_message )	
		error_message = [[NSString alloc] initWithUTF8String: data->error_message];
	if ( data->callback_params )	
		callback_params = [[NSString alloc] initWithUTF8String: data->callback_params];
	
	return self;
}

- (void)dealloc 
{
    rho_connectclient_free_syncnotify(&notify_data);
    
	if ( source_name )
		[source_name release];
	
	if ( status )
		[status release];
	
	if ( sync_type )
		[sync_type release];

	if ( bulk_status )
		[bulk_status release];

	if ( partition )
		[partition release];
	
	if (error_message)
		[error_message release];
	
	if (callback_params)
		[callback_params release];
	
    [super dealloc];
}

- (RHO_CONNECT_NOTIFY*)getNotifyPtr
{
    return &notify_data;
}

- (Boolean) hasCreateErrors
{
    return notify_data.create_errors != 0;
}

- (Boolean) hasUpdateErrors
{
    return notify_data.update_errors_obj != 0;
    
}

- (Boolean) hasDeleteErrors
{
    return notify_data.delete_errors_obj != 0;
}

- (Boolean) isUnknownClientError
{
    return [error_message caseInsensitiveCompare:@"unknown client"] == 0;
}

@end
