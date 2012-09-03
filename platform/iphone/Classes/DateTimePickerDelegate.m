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

#import "DateTimePickerDelegate.h"
#import "common/RhodesApp.h"
#import "ruby/ext/rho/rhoruby.h"


static NSString* ourChangeValueCallback = nil;

@implementation DateTimePickerDelegate

@synthesize dateTime, pickerView, parentView, toolbar, barLabel;

- (void)dealloc
{	
    [pickerView release];
    [toolbar release];
    [barLabel release];
    [dateTime release];
    [super dealloc];
}

- (void)createPickerBar:(CGRect)pickerFrame
{
    self.toolbar = [UIToolbar new];
    self.toolbar.barStyle = UIBarStyleBlackOpaque;
    
    // size up the toolbar and set its frame
    [self.toolbar sizeToFit];
	self.toolbar.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleWidth;
	self.toolbar.autoresizesSubviews = YES;
    CGFloat toolbarHeight = [self.toolbar frame].size.height;
    
    // TODO: This is an approximate y-origin, figure out why it is off by 3.7
    CGRect toolbarFrame = CGRectMake(pickerFrame.origin.x,
                                     pickerFrame.origin.y - toolbarHeight,
                                     pickerFrame.size.width,
                                     toolbarHeight);
    [self.toolbar setFrame:toolbarFrame];	
    
    UIBarButtonItem *cancelItem = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemCancel
                                                                                target:self 
                                                                                action:@selector(cancelAction:)];
    cancelItem.style = UIBarButtonItemStylePlain;
    
    // Setup label for toolbar
    self.barLabel = [[UILabel alloc] initWithFrame:toolbarFrame];
    self.barLabel.text = self.dateTime.title;
    self.barLabel.textColor = [UIColor whiteColor]; 
    self.barLabel.backgroundColor = [UIColor clearColor];
    self.barLabel.textAlignment = UITextAlignmentCenter;
	self.barLabel.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleWidth;
	self.barLabel.autoresizesSubviews = YES;

    UIBarButtonItem *flexItem = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
                                                                              target:self action:nil];
    
    UIBarButtonItem *doneItem = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone
                                                                              target:self 
                                                                              action:@selector(dateAction:)];
    doneItem.style = UIBarButtonItemStylePlain;

    [self.toolbar setItems:[NSArray arrayWithObjects: cancelItem, flexItem, doneItem, nil] animated:NO];
    [cancelItem release];
    [flexItem release];
    [doneItem release];
}


-(void) onChangeValue:(id)sender {
    if (ourChangeValueCallback == nil) {
        return;
    }
    
    long long ldate = [self.pickerView.date timeIntervalSince1970];

	NSString* strBody = @"&rho_callback=1";
	strBody = [strBody stringByAppendingString:@"&status=change&result="];
	strBody = [strBody stringByAppendingString:[[NSString alloc] initWithFormat:@"%qi" , ldate]];
	strBody = [strBody stringByAppendingString:@"&opaque="];
	strBody = [strBody stringByAppendingString:self.dateTime.data];

	const char* cb = [ourChangeValueCallback UTF8String];
	const char* b = [strBody UTF8String];
    char* norm_url = rho_http_normalizeurl(cb);
    rho_net_request_with_data(norm_url, b);
    rho_http_free(norm_url);
}


- (void)createPicker:(UIView*)parent
{
    self.parentView = parent;
    CGRect parentFrame = parent.bounds;
    
    // Create the picker
    if (self.pickerView == nil) {
        CGRect frame = parentFrame;
        frame.size.height = 220;
        frame.origin.y = parentFrame.origin.y + parentFrame.size.height - frame.size.height;
		self.pickerView = [[UIDatePicker alloc] initWithFrame:frame];
    }
    
    if (self.pickerView.superview == nil) {
        self.pickerView.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleWidth;
        self.pickerView.autoresizesSubviews = YES;
        
        // Determine picker type
        int mode = self.dateTime.format;
        switch (mode) {
            case 1:
                self.pickerView.datePickerMode = UIDatePickerModeDate;
                break;
            case 2:
                self.pickerView.datePickerMode = UIDatePickerModeTime;
                break;
            case 0:
            default:
                self.pickerView.datePickerMode = UIDatePickerModeDateAndTime;
        }
        
        if (self.dateTime.initialTime) {
            self.pickerView.date = [NSDate dateWithTimeIntervalSince1970:self.dateTime.initialTime];
        }
		if (self.dateTime.minTime) {
            self.pickerView.minimumDate = [NSDate dateWithTimeIntervalSince1970:self.dateTime.minTime];
		}
		if (self.dateTime.maxTime) {
            self.pickerView.maximumDate = [NSDate dateWithTimeIntervalSince1970:self.dateTime.maxTime];
		}
		
        //CGSize pickerSize = CGSizeMake(parentFrame.size.width, parentFrame.size.height/2);
        CGSize pickerSize = [pickerView sizeThatFits:pickerView.frame.size];
		if (pickerSize.width < parentFrame.size.width) {
			pickerSize.width = parentFrame.size.width;
		}
        CGRect pickerFrame = CGRectMake(parentFrame.origin.x,
                                        parentFrame.origin.y + parentFrame.size.height - pickerSize.height,
                                        pickerSize.width,
                                        pickerSize.height);
        
        // Add toolbar to view
        [self createPickerBar:pickerFrame];
        
        // Add picker to view
        [parentView addSubview:self.pickerView];
        
        CGRect startFrame = pickerFrame;
        startFrame.origin.y = parentFrame.origin.y + parentFrame.size.height;
        self.pickerView.frame = startFrame;
        [UIView beginAnimations:nil context:NULL];
        [UIView setAnimationDuration:0.3];
        
        [UIView setAnimationDelegate:self];
        [UIView setAnimationDidStopSelector:@selector(slideUpDidStop)];
        
        self.pickerView.frame = pickerFrame;
        
        [UIView commitAnimations];
    }
}

- (void)slideUpDidStop
{
    // the date picker has finished sliding upwards, so add toolbar
    [self.parentView addSubview:self.toolbar];
    [self.parentView addSubview:self.barLabel];
    [self.pickerView addTarget:self action:@selector(onChangeValue:) forControlEvents:UIControlEventValueChanged]; 
}

- (void)slideDownDidStop
{
    // the date picker has finished sliding downwards, so remove it
    [self.pickerView removeFromSuperview];
	[self.pickerView release];
	self.pickerView = nil;
    [self.barLabel release];
    self.barLabel = nil;
    [self.toolbar release];
    self.toolbar = nil;
}

- (void)animateDown
{
    CGRect parentFrame = parentView.frame;
    CGRect endFrame = pickerView.frame;
    endFrame.origin.y = parentFrame.origin.y + parentFrame.size.height;
    // start the slide down animation
    [UIView beginAnimations:nil context:NULL];
    [UIView setAnimationDuration:0.3];
    
    // we need to perform some post operations after the animation is complete
    [UIView setAnimationDelegate:self];
    [UIView setAnimationDidStopSelector:@selector(slideDownDidStop)];
    
    // Remove toolbar immediately
    [self.barLabel removeFromSuperview];
    [self.toolbar removeFromSuperview];

    self.pickerView.frame = endFrame;
    [UIView commitAnimations];
}

- (IBAction)cancelAction:(id)sender
{
    ourChangeValueCallback = nil;
    [self.pickerView removeTarget:self action:@selector(onChangeValue:) forControlEvents:UIControlEventValueChanged];
    //NSString *message = @"status=cancel";
    //[self doCallback:message];
    rho_rhodesapp_callDateTimeCallback(
        [postUrl cStringUsingEncoding:[NSString defaultCStringEncoding]],
        0, [self.dateTime.data cStringUsingEncoding:[NSString defaultCStringEncoding]], 1 );	
    [self animateDown];
}

- (IBAction)dateAction:(id)sender
{
    ourChangeValueCallback = nil;
    [self.pickerView removeTarget:self action:@selector(onChangeValue:) forControlEvents:UIControlEventValueChanged];
    long ldate = [self.pickerView.date timeIntervalSince1970];
    //NSMutableString *message = [[NSMutableString alloc] initWithFormat:@"status=ok&result=%@", [NSNumber numberWithLong:ldate]];
    //if (self.dateTime.data) {
    //	[message appendFormat:@"&opaque=%@", self.dateTime.data];
    //}
    //[self doCallback:message];
    rho_rhodesapp_callDateTimeCallback(
        [postUrl cStringUsingEncoding:[NSString defaultCStringEncoding]],
        ldate, [self.dateTime.data cStringUsingEncoding:[NSString defaultCStringEncoding]], 0 );
    
    [self animateDown];
}

+(void)setChangeValueCallback:(NSString*)callback {
    ourChangeValueCallback = callback;
}


@end
