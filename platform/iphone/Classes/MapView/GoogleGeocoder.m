﻿/*------------------------------------------------------------------------
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

#ifdef __IPHONE_3_0

#import "GoogleGeocoder.h"

#import "logging/RhoLog.h"
#undef DEFAULT_LOGCATEGORY
#define DEFAULT_LOGCATEGORY "GoogleGeocoder"

@implementation GoogleGeocoder

@synthesize actionTarget,onDidFindAddress,theElement,annotations,gapikey;

-(void)doGeocoding:(NSMutableArray*)items {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    NSEnumerator * enumerator = [items objectEnumerator];
    MapAnnotation* annotation, *element;	
    while(element = [enumerator nextObject]) {
        annotation = nil;
        if ( (element.coordinate.latitude==10000) ||
            (element.coordinate.longitude==10000) ) {
            NSString* address = [element address];
            if ( ([address length] > 0) && [self geocode:element]) {
                annotation = element;
            }
        } else {
            annotation = element;
        }
        
        if(annotation && actionTarget && [actionTarget respondsToSelector:onDidFindAddress]) {
            [actionTarget performSelectorOnMainThread:onDidFindAddress withObject:annotation waitUntilDone:NO];
        }
    }
    
    [pool release];	
}

-(void)start {
    if (annotations) {
        [NSThread detachNewThreadSelector:@selector(doGeocoding:) toTarget:self withObject:annotations];
    }
}

-(id)initWithAnnotations:(NSMutableArray*)_annotations apikey:(NSString*)key{
    self = [super init];
    annotations = _annotations;
    gapikey = [key copy];
    return self;
}

-(BOOL)geocode:(MapAnnotation *)annotation
{
    BOOL retvar = TRUE;
    currentAnnotation = annotation;
    NSMutableString *url = [[NSMutableString alloc] initWithString:kGeoCodeURL];
    [url appendString:[annotation address]];
    [url appendFormat:@"&output=xml"];
    [url replaceOccurrencesOfString:@"ä" withString:@"ae" options:1 range:(NSRange){0,[url length]}];
    [url replaceOccurrencesOfString:@"ö" withString:@"oe" options:1 range:(NSRange){0,[url length]}];
    [url replaceOccurrencesOfString:@"ü" withString:@"ue" options:1 range:(NSRange){0,[url length]}];
    [url replaceOccurrencesOfString:@"ß" withString:@"ss" options:1 range:(NSRange){0,[url length]}];
    [url replaceOccurrencesOfString:@" " withString:@"+" options:1 range:(NSRange){0,[url length]}];
    if (gapikey) {
        [url appendString:@"&key="];
        [url appendString:gapikey];
    }
    NSLog(@"Geocoding url = %@\n", url);
    NSURLRequest *request=[NSURLRequest requestWithURL:[NSURL URLWithString:url]							  
                                        cachePolicy:NSURLRequestUseProtocolCachePolicy
                                        timeoutInterval:60.0];	
    @try {
        NSHTTPURLResponse *response = NULL;
        NSData* data = [NSURLConnection sendSynchronousRequest:request returningResponse:&response error:nil];
        NSXMLParser *parser = [[NSXMLParser alloc] initWithData:data];
        [parser setDelegate:self];
        [parser parse];
        [self stringCooridinatesToCLLocation];
    }
    @catch (NSException *exception) {
        NSLog(@"Geocoding failed");
        retvar = FALSE;
    }
    @finally {
        //NSLog(@"finally");
    }
    return retvar;
}

-(CLLocation *)stringCooridinatesToCLLocation{
    NSArray *array = [[currentAnnotation coordinateString] componentsSeparatedByString:@","];
    double longitude = [[array objectAtIndex:0] doubleValue];
    double lattitude = [[array objectAtIndex:1] doubleValue];
    CLLocation *location = [[[CLLocation alloc] initWithLatitude:lattitude longitude:longitude] autorelease];
    [currentAnnotation setCoordinate:location.coordinate];
    return location;
}

#pragma mark -
#pragma mark XML Parsing Methods

-(void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName
 namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qualifiedName
   attributes:(NSDictionary *)attributeDict {
    if(theElement) [theElement release];
    theElement = [[NSString alloc] initWithString:elementName];
}

-(void)parser:(NSXMLParser *)parser foundCharacters:(NSString *)string {
    string = [string stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
    string = [string stringByReplacingOccurrencesOfString:@"\\n" withString:@""];
    if ([string length] == 0)
        return;
    
    if([theElement isEqualToString:@"address"])
        [currentAnnotation setResolvedAddress:string];
	
    if([theElement isEqualToString:@"coordinates"])
        [currentAnnotation setCoordinateString:string];
}

-(void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName
 namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qname {
}

	

@end

#endif
