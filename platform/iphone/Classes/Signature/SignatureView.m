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

#import "SignatureView.h"


@implementation SignatureViewProperties

@synthesize penColor, penWidth, bgColor, left, top, width, height;
@end


@implementation SignatureView

-(CGContextRef)CreateRGBABitmapContext:(int)width height:(int)height
{
	CGContextRef    context = NULL;
	CGColorSpaceRef colorSpace;
	void *          bitmapData;
	int             bitmapByteCount;
	int             bitmapBytesPerRow;
	
	// Get image width, height. We'll use the entire image.
	size_t pixelsWide = width;
	size_t pixelsHigh = height;
	
	// Declare the number of bytes per row. Each pixel in the bitmap in this
	// example is represented by 4 bytes; 8 bits each of red, green, blue, and
	// alpha.
	bitmapBytesPerRow   = (pixelsWide) << 2;
	bitmapByteCount     = (bitmapBytesPerRow * pixelsHigh);
	
	// Use the generic RGB color space.
	colorSpace = CGColorSpaceCreateDeviceRGB();//CGColorSpaceCreateDeviceGray();//(kCGColorSpaceGenericRGB);
	if (colorSpace == NULL)
	{
		fprintf(stderr, "Error allocating color space\n");
		return NULL;
	}
	
	// Allocate memory for image data. This is the destination in memory
	// where any drawing to the bitmap context will be rendered.
	bitmapData = malloc( bitmapByteCount );
	if (bitmapData == NULL) 
	{
		fprintf (stderr, "Memory not allocated!");
		CGColorSpaceRelease( colorSpace );
		return NULL;
	}
	
	// Create the bitmap context. We want pre-multiplied ARGB, 8-bits 
	// per component. Regardless of what the source image format is 
	// (CMYK, Grayscale, and so on) it will be converted over to the format
	// specified here by CGBitmapContextCreate.
	context = CGBitmapContextCreate (bitmapData,
									 pixelsWide,
									 pixelsHigh,
									 8,      // bits per component
									 bitmapBytesPerRow,
									 colorSpace,
									 kCGImageAlphaNoneSkipLast);
	if (context == NULL)
	{
		free (bitmapData);
		fprintf (stderr, "Context not created!");
	}
	
	// Make sure and release colorspace before returning
	CGColorSpaceRelease( colorSpace );
	
	return context;
}


-(void)setPenColor:(unsigned int)value
{
    penColor = value;
    [self setNeedsDisplay];
}

-(void)setPenWidth:(float)value
{
    penWidth = value;
    [self setNeedsDisplay];
}

-(void)setBgColor:(unsigned int)value
{
    bgColor = value;
    [self setNeedsDisplay];
}

- (id)initWithFrame:(CGRect)frame {
    if ((self = [super initWithFrame:frame])) {
        // Initialization code
		//self.backgroundColor = [UIColor groupTableViewBackgroundColor];
		self.backgroundColor = [UIColor whiteColor];
		self.opaque = YES;
		self.multipleTouchEnabled = NO;
        penColor = 0xFF66009A;
        penWidth = 3.0;
        bgColor = 0xFFFFFFFF;

    }
	mPath = nil;
	[self doClear];
    return self;
}


// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect {
    // Drawing code
	CGContextRef context = UIGraphicsGetCurrentContext();
	
	// draw backgound
    
	CGContextSetRGBFillColor(context,   ((float)((bgColor & 0xFF0000) >> 16))/255.0, 
                                        ((float)((bgColor & 0xFF00) >> 8))/255.0,
                                        ((float)((bgColor & 0xFF)))/255.0,
                                        ((float)((bgColor & 0xFF000000) >> 24))/255.0);
	
    
    CGContextSetRGBStrokeColor(context, ((float)((penColor & 0xFF0000) >> 16))/255.0,
                                        ((float)((penColor & 0xFF00) >> 8))/255.0,
                                        ((float)((penColor & 0xFF)))/255.0,
                                        ((float)((penColor & 0xFF000000) >> 24))/255.0);
	CGContextFillRect(context, rect);
	
	// draw signature
	CGContextSetLineWidth(context, penWidth);
	CGContextBeginPath(context);
	CGContextClipToRect(context, rect);
	//CGContextClip(context);
	CGContextAddPath(context,mPath);
	CGContextDrawPath(context, kCGPathStroke);
	
}

- (UIImage*)makeUIImage {
	
	CGRect bound_box = self.bounds;
	if (!CGPathIsEmpty(mPath)) {
		bound_box = CGPathGetBoundingBox(mPath);
	}
	CGRect rect = self.bounds;
	if (rect.size.width < (bound_box.origin.x+bound_box.size.width)) {
		rect.size.width = (bound_box.origin.x+bound_box.size.width)+2;
	}
	if (rect.size.height < (bound_box.origin.y+bound_box.size.height)) {
		rect.size.height = (bound_box.origin.y+bound_box.size.height)+2;
	}
	CGContextRef context = [self CreateRGBABitmapContext:rect.size.width height:rect.size.height];
	rect.origin.x = 0;
	rect.origin.y = 0;
	
	// draw backgound
	CGContextSetRGBFillColor(context,   ((float)((bgColor & 0xFF0000) >> 16))/255.0, 
                             ((float)((bgColor & 0xFF00) >> 8))/255.0,
                             ((float)((bgColor & 0xFF)))/255.0,
                             1);
	
    
    CGContextSetRGBStrokeColor(context, ((float)((penColor & 0xFF0000) >> 16))/255.0,
                               ((float)((penColor & 0xFF00) >> 8))/255.0,
                               ((float)((penColor & 0xFF)))/255.0,
                               ((float)((penColor & 0xFF000000) >> 24))/255.0);
	CGContextFillRect(context, rect);
	
	// draw signature
	if (!CGPathIsEmpty(mPath)) {
        CGContextSetLineWidth(context, penWidth);
		CGContextTranslateCTM (context, 0, rect.size.height);
		CGContextScaleCTM(context, 1, -1);
		CGContextBeginPath(context);
		CGContextAddPath(context,mPath);
		CGContextDrawPath(context, kCGPathStroke);
	}
	
	CGImageRef cg_image = CGBitmapContextCreateImage(context);
	UIImage* ui_image = [[UIImage alloc] initWithCGImage:cg_image];// scale:1 orientation:UIImageOrientationDownMirrored];
    
	CGImageRelease(cg_image);

	CGContextRelease(context);
	
	return ui_image;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
	NSArray *t = [[event allTouches] allObjects];
	if ([t count] > 0) {
		CGPoint cur_point;
		cur_point.x = [[t objectAtIndex:0] locationInView:self].x;
		cur_point.y = [[t objectAtIndex:0] locationInView:self].y;
		CGPathMoveToPoint(mPath, nil, cur_point.x, cur_point.y);
		mLastPoint = cur_point;
	}
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
	NSArray *t = [[event allTouches] allObjects];
	if ([t count] > 0) {
		CGPoint cur_point;
		cur_point.x = [[t objectAtIndex:0] locationInView:self].x;
		cur_point.y = [[t objectAtIndex:0] locationInView:self].y;
		
		CGPathAddLineToPoint(mPath, nil, cur_point.x, cur_point.y);

		CGRect rect;
		if (cur_point.x < mLastPoint.x) {
			rect.origin.x = cur_point.x - 2;
			rect.size.width = mLastPoint.x - cur_point.x + 4;
		}
		else {
			rect.origin.x = mLastPoint.x - 2;
			rect.size.width = cur_point.x - mLastPoint.x + 4;
		}
		if (cur_point.y < mLastPoint.y) {
			rect.origin.y = cur_point.y - 2;
			rect.size.height = mLastPoint.y - cur_point.y + 4;
		}
		else {
			rect.origin.y = mLastPoint.y - 2;
			rect.size.height = cur_point.y - mLastPoint.y + 4;
		}
		
		[self setNeedsDisplayInRect:rect];
		mLastPoint = cur_point;
	}
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
	[self touchesMoved:touches withEvent:event];
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
}

- (void)doClear {
	if (mPath != nil)
		[mPath release];
	mPath = CGPathCreateMutable();
	[self setNeedsDisplay];
}

- (void)dealloc {
	[mPath release];
    [super dealloc];
}


@end
