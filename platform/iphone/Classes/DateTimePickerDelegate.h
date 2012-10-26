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

#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>
#import "RhoDelegate.h"
#import "DateTime.h"
#import "DateTimePickerViewController.h"

@interface DateTimePickerDelegate : RhoDelegate <UIPickerViewDelegate>
{
@private
    UIDatePicker *pickerView;
    UIView *parentView;
    UIToolbar *toolbar;
    UILabel *barLabel;
    DateTime *dateTime;
    DateTimePickerViewController* dt_controller;
}

@property (nonatomic, retain) DateTime *dateTime;
@property (nonatomic, retain) UIDatePicker *pickerView;
@property (nonatomic, retain) UIView *parentView;
@property (nonatomic, retain) UIToolbar *toolbar;
@property (nonatomic, retain) UILabel *barLabel;

- (IBAction)dateAction:(id)sender;
- (IBAction)cancelAction:(id)sender;
- (void)createPicker:(UIView*)parent;

+(void)setChangeValueCallback:(NSString*)callback;

@end
