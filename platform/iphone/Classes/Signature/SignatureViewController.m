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

#import "SignatureViewController.h"

#import "SignatureView.h"
#import "AppManager.h"
#import "Rhodes.h"

#include "common/RhoConf.h"
#include "common/RhodesApp.h"
#include "logging/RhoLog.h"

#undef DEFAULT_LOGCATEGORY
#define DEFAULT_LOGCATEGORY "SignatureViewController"


@implementation SignatureViewController


- (id)initWithRect:(CGRect)rect  delegate:(SignatureDelegate*)delegate {
	self = [super init];
	
	signatureDelegate = delegate;
	self.view.frame = rect;

	//content.backgroundColor = [UIColor redColor];
	//1ontent.backgroundColor = [UIColor groupTableViewBackgroundColor];
	
	toolbar = [[UIToolbar alloc] init];
	toolbar.barStyle = UIBarStyleBlack;
	
	toolbar.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleWidth;
	toolbar.autoresizesSubviews = YES;
	
	{
		UIBarButtonItem *btn_fixed = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFixedSpace target:nil action:nil];
		UIBarButtonItem* btn_cancel = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemCancel target:self action:@selector(doCancel:)];
		UIBarButtonItem* btn_clear = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemTrash target:self action:@selector(doClear:)];
		UIBarButtonItem* btn_space = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace target:nil action:nil];
		UIBarButtonItem* btn_done = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone target:self action:@selector(doDone:)];
		
		NSMutableArray *btns = [NSMutableArray arrayWithCapacity:6];
		[btns addObject:btn_fixed];
		[btns addObject:btn_cancel];
		[btns addObject:btn_fixed];
		[btns addObject:btn_clear];
		[btns addObject:btn_space];
		[btns addObject:btn_done];
		
		[btn_fixed release];
		[btn_cancel release];
		[btn_clear release];
		[btn_space release];
		[btn_done release];
		
		[toolbar setItems:btns];
		
	}
	
    self.view.autoresizesSubviews = YES;
	
	[toolbar sizeToFit];
	CGRect srect = self.view.frame;
	CGRect trect = toolbar.frame;
	srect.size.height -= trect.size.height;
	srect.origin.y = 0;
	trect.origin.x = 0;
	trect.origin.y = srect.origin.y+srect.size.height;
	trect.size.width = srect.size.width;
	toolbar.frame = trect;
	toolbar.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleWidth;
	
	signatureView = [[SignatureView alloc] initWithFrame:srect];
	signatureView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
	signatureView.autoresizesSubviews = YES;
	signatureView.frame = srect;
	
	[self.view addSubview:signatureView];
	[self.view addSubview:toolbar];
	
	return self;

}

-(void)setPenColor:(unsigned int)value
{
    [signatureView setPenColor:value];
}

-(void)setPenWidth:(float)value
{
    [signatureView setPenWidth:value];
}

-(void)setBgColor:(unsigned int)value
{
    [signatureView setBgColor:value];
}


// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView {
	UIView* content = [[UIView alloc] initWithFrame:CGRectZero];

	self.view = content;
	[content release];
}

- (void)doDone:(id)sender {
	//NSData* data = UIImagePNGRepresentation([signatureView makeUIImage]);
	[signatureDelegate doDone:[signatureView makeUIImage]];
}

- (void)doClear:(id)sender {
	[signatureView doClear];
}

- (void)doCancel:(id)sender {
	[signatureDelegate doCancel];
}



///*
// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
	//[self.view 
}
//*/


- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation {
//	[signatureView setNeedsDisplay];
}

- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration {
	[signatureView setNeedsDisplay];
}



- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
}


- (void)dealloc {
	[signatureView removeFromSuperview];
	[toolbar removeFromSuperview];
	[signatureView release];
	[toolbar release];
    [super dealloc];
}



@end
