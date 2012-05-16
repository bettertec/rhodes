//
//  WaitLoginController.m
//  store
//
//  Created by Vlad on 8/30/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "RhoConnectEngine.h"
#import "WaitLoginController.h"

@implementation WaitLoginController

@synthesize lblMessage, indicator, homePage;


- (void)syncAllComplete:(RhoConnectNotify*) notify
{
    NSLog(@"sync_type: \"%@\"", notify.sync_type);
    NSLog(@"bulk_status: \"%@\"", notify.bulk_status);
    NSLog(@"partition: \"%@\"", notify.partition);
	if ( [notify.status compare:@"in_progress"] == 0)
	{
	}else if ([notify.status compare:@"complete"] == 0)
	{
		[[self navigationController] pushViewController:homePage animated:YES];
		[ [RhoConnectEngine sharedInstance].syncClient clearNotification];		
	}else if ([notify.status compare:@"error"] == 0)
	{
        if([notify.error_message caseInsensitiveCompare:@"unknown client"] == 0) 
        {
            [[RhoConnectEngine sharedInstance].syncClient database_client_reset]; 
            [[RhoConnectEngine sharedInstance].syncClient setNotification: @selector(syncAllComplete:) target:self];
            [RhoConnectEngine sharedInstance].syncClient.bulksync_state = 0;
            [[RhoConnectEngine sharedInstance].syncClient syncAll];
        }
	}
}

- (void)loginComplete:(NSString*) errorMessage 
{
	NSLog(@"Login error message: \"%@\"", errorMessage);
	[indicator stopAnimating];
	if ([RhoConnectEngine sharedInstance].loginState == logged_in) 
	{
		[ [RhoConnectEngine sharedInstance].syncClient setNotification: @selector(syncAllComplete:) target:self];
        [RhoConnectEngine sharedInstance].syncClient.bulksync_state = 0;
		[ [RhoConnectEngine sharedInstance].syncClient syncAll];
	} else {
		lblMessage.text = errorMessage;
		self.navigationItem.hidesBackButton = false;
	}
}

- (void)viewDidAppear:(BOOL)animated {
	self.navigationItem.title = @"Wait";
	if ([RhoConnectEngine sharedInstance].loginState == in_progress) {
		[indicator startAnimating];
		lblMessage.text = @"Working";
		self.navigationItem.hidesBackButton = true;
	} else {
		[indicator stopAnimating];
	}
}

/*
 // The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if ((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil])) {
        // Custom initialization
    }
    return self;
}
*/

/*
// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
}
*/

/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}


- (void)dealloc {
    [super dealloc];
}


@end
