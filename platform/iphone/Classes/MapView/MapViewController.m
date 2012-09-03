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

#ifdef __IPHONE_3_0

#import "MapAnnotation.h"
#import "MapViewController.h"
#import "Rhodes.h"
#import "RhoMainView.h"

#include "logging/RhoLog.h"
#include "ruby/ext/rho/rhoruby.h"

#undef DEFAULT_LOGCATEGORY
#define DEFAULT_LOGCATEGORY "MapView"

static MapViewController *mc = nil;





@interface RhoCreateMapTask : NSObject {}
+ (void)run:(NSValue*)value;
@end

@implementation RhoCreateMapTask
+ (void)run:(NSValue*)value {
    if (mc) {
        [mc close];
        mc = nil;
    }
    MapViewController* map = [[MapViewController alloc] init];
    [map setParams:[value pointerValue]];
    UIWindow *window = [[Rhodes sharedInstance] rootWindow];
	map.view.autoresizingMask = UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleWidth;
	map.view.autoresizesSubviews = YES;
	
	UIView* v = [[[Rhodes sharedInstance] mainView] view];
	map.savedMainView = v;
	[map.savedMainView retain];
    [map.savedMainView removeFromSuperview];
	[window addSubview:map.view];
    //window.autoresizesSubviews = YES;
	//[window layoutSubviews];
    
    mc = map;
}
@end

@interface RhoCloseMapTask : NSObject
+ (void)run;
@end

@implementation RhoCloseMapTask
+ (void)run {
    if (mc) {
        [mc close];
        mc = nil;
    }
}
@end


@implementation MapViewController

@synthesize region_center, gapikey, savedMainView;

+ (void)createMap:(rho_param *)params {
    id runnable = [RhoCreateMapTask class];
    id arg = [NSValue valueWithPointer:params];
    [Rhodes performOnUiThread:runnable arg:arg wait:NO];
}

+ (void)closeMap {
    id runnable = [RhoCloseMapTask class];
    [Rhodes performOnUiThread:runnable wait:NO];
}

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if (self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]) {
        mapType =  MKMapTypeStandard;
        zoomEnabled = TRUE;
        scrollEnabled = TRUE;	
        showsUserLocation = TRUE;
        region_set = FALSE;
        region_center = nil;
    }
    return self;
}

- (void)close {
    [self dismissModalViewControllerAnimated:YES]; 
    
	UIWindow *window = [[Rhodes sharedInstance] rootWindow];


	[window addSubview:self.savedMainView];
	[self.view removeFromSuperview];

	[self.savedMainView release];
	self.savedMainView = nil;
}

- (void)setSettings:(rho_param*)p {
    if (!p || p->type != RHO_PARAM_HASH)
        return;
    
    for (int i = 0, lim = p->v.hash->size; i < lim; ++i) {
        char *name = p->v.hash->name[i];
        rho_param *value = p->v.hash->value[i];
        if (!name || !value)
            continue;
        
        if (strcasecmp(name, "map_type") == 0) {
            if (value->type != RHO_PARAM_STRING)
                continue;
            char *map_type = value->v.string;
            if (strcasecmp(map_type, "standard") == 0 || strcasecmp(map_type, "roadmap") == 0)
                mapType = MKMapTypeStandard;
            else if (strcasecmp(map_type, "satellite") == 0)
                mapType = MKMapTypeSatellite;
            else if (strcasecmp(map_type, "hybrid") == 0)
                mapType = MKMapTypeHybrid;
            else {
                RAWLOG_ERROR1("Unknown map type: %s", map_type);
                continue;
            }
        }
        else if (strcasecmp(name, "region") == 0) {
            if (value->type == RHO_PARAM_ARRAY) {
                if (value->v.array->size != 4)
                    continue;
                
                rho_param *lat = value->v.array->value[0];
                rho_param *lon = value->v.array->value[1];
                rho_param *latSpan = value->v.array->value[2];
                rho_param *lonSpan = value->v.array->value[3];
                
                CLLocationCoordinate2D location;	
                location.latitude = lat->type == RHO_PARAM_STRING ? strtod(lat->v.string, NULL) : 0;
                location.longitude = lon->type == RHO_PARAM_STRING ? strtod(lon->v.string, NULL) : 0;
                MKCoordinateSpan span;
                span.latitudeDelta = latSpan->type == RHO_PARAM_STRING ? strtod(latSpan->v.string, NULL) : 0;
                span.longitudeDelta = lonSpan->type == RHO_PARAM_STRING ? strtod(lonSpan->v.string, NULL) : 0;
                region.span = span;
                region.center = location;
                region_set = TRUE;
            }
            else if (value->type == RHO_PARAM_HASH) {
                char *center = NULL;
                char *radius = NULL;
                
                for (int j = 0, limm = value->v.hash->size; j < limm; ++j) {
                    char *rname = value->v.hash->name[j];
                    rho_param *rvalue = value->v.hash->value[j];
                    if (strcasecmp(rname, "center") == 0) {
                        if (rvalue->type != RHO_PARAM_STRING) {
                            RAWLOG_ERROR("Wrong type of 'center', should be String");
                            continue;
                        }
                        center = rvalue->v.string;
                    }
                    else if (strcasecmp(rname, "radius") == 0) {
                        if (rvalue->type != RHO_PARAM_STRING) {
                            RAWLOG_ERROR("Wrong type of 'radius', should be String");
                            continue;
                        }
                        radius = rvalue->v.string;
                    }
                }
                
                if (!center || !radius)
                    continue;
                
                region_center = [NSString stringWithUTF8String:center];
                region_radius = strtod(radius, NULL);
            }
        }
        else if (strcasecmp(name, "zoom_enabled") == 0) {
            if (value->type != RHO_PARAM_STRING)
                continue;
            zoomEnabled = strcasecmp(value->v.string, "true") == 0;
        }
        else if (strcasecmp(name, "scroll_enabled") == 0) {
            if (value->type != RHO_PARAM_STRING)
                continue;
            scrollEnabled = strcasecmp(value->v.string, "true") == 0;
        }
        else if (strcasecmp(name, "shows_user_location") == 0) {
            if (value->type != RHO_PARAM_STRING)
                continue;
            showsUserLocation = strcasecmp(value->v.string, "true") == 0;
        }
        else if (strcasecmp(name, "api_key") == 0) {
            if (value->type != RHO_PARAM_STRING)
                continue;
            gapikey = [NSString stringWithUTF8String:value->v.string];
        }
    }
}

- (void)setAnnotations:(rho_param*)p {
    int size = 1;
    if (p && p->type == RHO_PARAM_ARRAY)
        size += p->v.array->size;
    NSMutableArray *annotations = [NSMutableArray arrayWithCapacity:size];
    if (region_center) {
        MapAnnotation *annObj = [[MapAnnotation alloc] init];
        annObj.type = @"center";
        annObj.address = region_center;
        CLLocationCoordinate2D c;
        c.latitude = c.longitude = 10000;
        annObj.coordinate = c;
        [annotations addObject:annObj];
        [annObj release];
    }
    if (p && p->type == RHO_PARAM_ARRAY) {
        for (int i = 0, lim = p->v.array->size; i < lim; ++i) {
            rho_param *ann = p->v.array->value[i];
            if (ann->type != RHO_PARAM_HASH)
                continue;
            
            CLLocationCoordinate2D coord;
            coord.latitude = 10000;
            coord.longitude = 10000;
            
            NSString *address = nil;
            NSString *title = nil;
            NSString *subtitle = nil;
            NSString *url = nil;
            
            NSString *image = nil;
            int image_x_offset = 0;
            int image_y_offset = 0;
            
            for (int j = 0, limm = ann->v.hash->size; j < limm; ++j) {
                char *name = ann->v.hash->name[j];
                rho_param *value = ann->v.hash->value[j];
                if (!name || !value)
                    continue;
                if (value->type != RHO_PARAM_STRING)
                    continue;
                char *v = value->v.string;
                
                if (strcasecmp(name, "latitude") == 0) {
                    coord.latitude = strtod(v, NULL);
                }
                else if (strcasecmp(name, "longitude") == 0) {
                    coord.longitude = strtod(v, NULL);
                }
                else if (strcasecmp(name, "street_address") == 0) {
                    address = [NSString stringWithUTF8String:v];
                }
                else if (strcasecmp(name, "title") == 0) {
                    title = [NSString stringWithUTF8String:v];
                }
                else if (strcasecmp(name, "subtitle") == 0) {
                    subtitle = [NSString stringWithUTF8String:v];
                }
                else if (strcasecmp(name, "url") == 0) {
                    url = [NSString stringWithUTF8String:v];
                }
                else if (strcasecmp(name, "image") == 0) {
                    image = [NSString stringWithUTF8String:v];
                }
                else if (strcasecmp(name, "image_x_offset") == 0) {
                    image_x_offset = (int)strtod(v, NULL);
                }
                else if (strcasecmp(name, "image_y_offset") == 0) {
                    image_y_offset = (int)strtod(v, NULL);
                }
            }
            
            MapAnnotation *annObj = [[MapAnnotation alloc] init];
            [annObj setCoordinate:coord];
            if (address) [annObj setAddress:address];
            if (title) [annObj setTitle:title];
            if (subtitle) [annObj setSubtitle:subtitle];
            if (url) [annObj setUrl:url];
            if (image) [annObj setImage:image];
            [annObj setImage_x_offset:image_x_offset];
            [annObj setImage_y_offset:image_y_offset];
            [annotations addObject:annObj];
            [annObj release];
        }
    }
    ggeoCoder = [[GoogleGeocoder alloc] initWithAnnotations:annotations apikey:gapikey];
    ggeoCoder.actionTarget = self;
    ggeoCoder.onDidFindAddress = @selector(didFindAddress:);
}

- (void)setParams:(rho_param*)p {
    if (p && p->type == RHO_PARAM_HASH) {
        rho_param *st = NULL;
        rho_param *ann = NULL;
        for (int i = 0, lim = p->v.hash->size; i < lim; ++i) {
            char *name = p->v.hash->name[i];
            rho_param *value = p->v.hash->value[i];
            if (strcasecmp(name, "settings") == 0)
                st = value;
            else if (strcasecmp(name, "annotations") == 0)
                ann = value;
        }
        if (st)
            [self setSettings:st];
        [self setAnnotations:ann];
    }
    rho_param_free(p);
}


// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
    
    //Initialize the toolbar
    toolbar = [[UIToolbar alloc] init];
    toolbar.barStyle = UIBarStyleBlack;
    UIBarButtonItem *closeButton = [[UIBarButtonItem alloc]
                               initWithTitle:@"Close" style:UIBarButtonItemStyleBordered 
                               target:self action:@selector(close_clicked:)];
    [toolbar setItems:[NSArray arrayWithObjects:closeButton,nil]];

	
	toolbar.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleTopMargin | 
	UIViewAutoresizingFlexibleWidth;
	toolbar.autoresizesSubviews = YES;
    [toolbar sizeToFit];
	
    CGFloat toolbarHeight = [toolbar frame].size.height;
	// hack for do not reduce height of toolbar in Landscape mode
	if (toolbarHeight < 44) {
		toolbarHeight = 44;
	}

	//RhoMainView* rw = [[Rhodes sharedInstance] mainView];
    
	CGRect rootViewBounds = [[[Rhodes sharedInstance] mainView] view].frame;//bounds;
	
	self.view.frame = rootViewBounds;
	
    CGFloat rootViewHeight = rootViewBounds.size.height;
	//CGFloat rootViewHeight = CGRectGetHeight(rootViewBounds);
    CGFloat rootViewWidth = CGRectGetWidth(rootViewBounds);
    CGRect rectArea = CGRectMake(0, rootViewHeight - toolbarHeight, rootViewWidth, toolbarHeight);
    toolbar.frame = rectArea;
    
	
	[self.view addSubview:toolbar];
    [closeButton release];
    
    CGRect rectMapArea = CGRectMake(0, 0, rootViewWidth, rootViewHeight - toolbarHeight);
    mapView =[[MKMapView alloc] initWithFrame:rectMapArea];
	mapView.frame = rectMapArea;
    mapView.delegate=self;

    mapView.showsUserLocation=showsUserLocation;
    
    //[mapView setUserTrackingMode:MKUserTrackingModeNone animated:NO];
    
    mapView.scrollEnabled=scrollEnabled;
    mapView.zoomEnabled=zoomEnabled;
    mapView.mapType=mapType;
	
    mapView.autoresizesSubviews = YES;
    mapView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
	
	
	
    
    /*Geocoder Stuff*/
    [ggeoCoder start];
	
    //geoCoder=[[MKReverseGeocoder alloc] initWithCoordinate:location];
    //geoCoder.delegate=self;
    //[geoCoder start];
	
    /*Region and Zoom*/
    if (region_set) {
        [mapView setRegion:region animated:TRUE];
        [mapView regionThatFits:region];
    }
    
    [self.view insertSubview:mapView atIndex:0];
	//[[self.view superview] layoutSubviews];
	
}

- (void)mapView:(MKMapView *)mapView annotationView:(MKAnnotationView *)view 
    calloutAccessoryControlTapped:(UIControl *)control
{
    MapAnnotation *ann = (MapAnnotation*)[view annotation];
    NSString* url = [ann url];
    NSLog(@"Callout tapped... Url = %@\n", url);
    id<RhoMainView> mainView = [[Rhodes sharedInstance] mainView];
    [mainView navigateRedirect:url tab:[mainView activeTab]];
    [self dismissModalViewControllerAnimated:YES]; 
    [self close];
	//mc = nil;
    //self.view.hidden = YES;
}

- (void) close_clicked:(id)sender {
    [self close];
	//mc = nil;
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
    [super dealloc];
}

/*- (IBAction)changeType:(id)sender{
	if(mapType.selectedSegmentIndex==0){
		mapView.mapType=MKMapTypeStandard;
	}
	else if (mapType.selectedSegmentIndex==1){
		mapView.mapType=MKMapTypeSatellite;
	}
	else if (mapType.selectedSegmentIndex==2){
		mapView.mapType=MKMapTypeHybrid;
	}
}*/

- (void)reverseGeocoder:(MKReverseGeocoder *)geocoder didFailWithError:(NSError *)error{
    NSLog(@"Reverse Geocoder Errored");
}

- (void)didFindAddress:(MapAnnotation*)annotation {
    if ([[annotation type] isEqualToString:@"center"]) {
        MKCoordinateSpan span;
        span.latitudeDelta = region_radius;
        span.longitudeDelta = region_radius;
        region.center = annotation.coordinate;
        region.span = span;
        region_set = TRUE;
        
        [mapView setRegion:region animated:YES];
        [mapView regionThatFits:region];
    }
    else
        [mapView addAnnotation:annotation];
}
	
- (void)reverseGeocoder:(MKReverseGeocoder *)geocoder didFindPlacemark:(MKPlacemark *)placemark{
    NSLog(@"Reverse Geocoder completed");
    //mPlacemark=placemark;
    //[mapView addAnnotation:placemark];
}

- (MKAnnotationView *) mapView:(MKMapView *)mapView viewForAnnotation:(id <MKAnnotation>) annotation{
    
    MKAnnotationView *annView = nil;
    
    if ([annotation isKindOfClass:[MapAnnotation class]]) {
        MapAnnotation* ann = (MapAnnotation*)annotation;
        NSString* url = [ann url];
        if (ann.image != nil) {
            UIImage *img = nil;
            if ([ann.image length] > 0) {
                NSString *imagePath = [[AppManager getApplicationsRootPath] stringByAppendingPathComponent:ann.image];
                img = [UIImage imageWithContentsOfFile:imagePath];
            }
            if (img != nil) {
                annView = [[[MKAnnotationView alloc] initWithAnnotation:annotation reuseIdentifier:@"currentloc"] autorelease];
                annView.image = img;
                int w = (int)img.size.width;
                int h = (int)img.size.height;
                CGPoint offset;
                offset.x = w/2 - ann.image_x_offset;
                offset.y = h/2 - ann.image_y_offset;
                annView.centerOffset = offset;
            }
        }
        if (annView == nil) {
            annView = [[[MKPinAnnotationView alloc] initWithAnnotation:annotation reuseIdentifier:@"currentloc"] autorelease];
        }
        if ([url length] > 0) {
            [annView setRightCalloutAccessoryView:[UIButton buttonWithType:UIButtonTypeDetailDisclosure]];
        }

        if ([annView isKindOfClass:[MKPinAnnotationView class]]) {
            MKPinAnnotationView* annPinView = (MKPinAnnotationView*)annView;
            annPinView.animatesDrop = TRUE;
        }
    }
    else {
        annView = [[[MKPinAnnotationView alloc]
                                         initWithAnnotation:annotation reuseIdentifier:@"currentloc"] autorelease];
    }
    annView.canShowCallout = YES;
    return annView;
}

+ (BOOL)isStarted {
    return mc != nil;
}

+ (CLLocationCoordinate2D)center {
    CLLocationCoordinate2D center;
    if (mc) {
        center = mc->region.center;
    }
    else {
        center.latitude = 0;
        center.longitude = 0;
    }
        
    return center;
}

+ (double)centerLatitude {
	return [MapViewController center].latitude;
}

+ (double)centerLongitude {
	return [MapViewController center].longitude;
}


@end

#endif
