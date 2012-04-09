
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "zbar.h"
#include "BarcodeViewController.h"

#include "ruby/ext/rho/rhoruby.h"

extern void rho_platform_image_load_grayscale(const char* url, void** image_pixels, int* pwidth, int* pheight);
extern void rho_platform_image_free(void* image_pixels);

extern void register_Barcode_View(void);

static char strbuf[1024];

void rho_barcode_take_barcode(const char* callback, VALUE options) {
    
    register_Barcode_View();
    
    [BarcodeViewController createBarcodeView:[NSString stringWithCString:callback]];

}


const char* rho_barcode_barcode_recognize(const char* filename) {

   //NSLog(@"$$$$$$$$$$$$$$$$$$$$  Barcode File : %@: ", [NSString stringWithUTF8String:filename]);

    void* img_buf = 0;
    int img_width;
    int img_height;	
    
  	
    rho_platform_image_load_grayscale(filename, &img_buf, &img_width, &img_height);
    
    if (img_buf != 0)
    {
        zbar_image_scanner_t* zbar_img_scanner = zbar_image_scanner_create();
        zbar_image_t* zbar_img = zbar_image_create();
        const zbar_symbol_t* zbar_symbol = 0;
        
        zbar_image_scanner_set_config(zbar_img_scanner, ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
        
        zbar_image_set_format(zbar_img, zbar_fourcc('Y','8','0','0'));
        zbar_image_set_size(zbar_img, img_width, img_height);
        //zbar_image_set_data(zbar_img, img_buf, img_width * img_height, zbar_image_free_data);
        zbar_image_set_data(zbar_img, img_buf, img_width * img_height, 0);
		
        zbar_scan_image(zbar_img_scanner, zbar_img);
        
        
        // get result
        //zbar_symbol_set_t* zbar_symbols = zbar_image_get_symbols(zbar_img);
        zbar_symbol = zbar_image_first_symbol(zbar_img);
        
        if (zbar_symbol != 0) {
            //printf(zbar_symbol_get_data(zbar_symbol));
            //sprintf(strbuf, "IMG [%d x %d ]\nCODE = %s",img_width, img_height, zbar_symbol_get_data(zbar_symbol)); 
            strcpy(strbuf, zbar_symbol_get_data(zbar_symbol));
        }
        else {
            //sprintf(strbuf, "IMG [%d x %d ]\nCODE IS UNRECOGNIZED ",img_width, img_height); 
            strcpy(strbuf, "");
        }
        
        
        zbar_image_destroy(zbar_img);
        zbar_image_scanner_destroy(zbar_img_scanner);
        
    }
    else {
		//sprintf(strbuf, "NO IMG TO RECOGNIZE",img_width, img_height); 
        strcpy(strbuf, "");
    }
    
    if (img_buf) {
        rho_platform_image_free(img_buf);
    }
    
    
    return strbuf;
}



@interface RhoUIThreadCallbackRunner : NSObject {
    
}

+ (RhoUIThreadCallbackRunner*) getSharedInstance;

+ (void) runEnumerateCallback:(NSString*) callback_url;

@end

static RhoUIThreadCallbackRunner* uiThreadCallbackRunner = nil;

@implementation RhoUIThreadCallbackRunner

+ (RhoUIThreadCallbackRunner*) getSharedInstance {
    if (uiThreadCallbackRunner == nil) {
        uiThreadCallbackRunner = [[RhoUIThreadCallbackRunner alloc] init];
    }
    return uiThreadCallbackRunner;
}

- (void) runEnumerateCallbackCommand:(NSString*) callback_url {
    Barcode_executeEnumerateCallback(callback_url);
}

- (void) runEnumerateCallbackObj:(NSString*) callback_url {
    [self performSelectorOnMainThread:@selector(runEnumerateCallbackCommand:) withObject:callback_url waitUntilDone:NO];
}

+ (void) runEnumerateCallback:(NSString*) callback_url {
    [[RhoUIThreadCallbackRunner getSharedInstance] runEnumerateCallbackObj:callback_url];
}


@end


void rho_motobarcode_enumerate(const char* callback) {
    [RhoUIThreadCallbackRunner runEnumerateCallback:[NSString stringWithUTF8String:callback]];
    //Barcode_executeEnumerateCallback([NSString stringWithUTF8String:callback]);
}

void  rho_motobarcode_enable(const char* callback, rho_param* p) {
    NSLog(@"Barcode::enable() DO NOT SUPPORT BY RHODES SW SCANNER !");    
}

void  rho_motobarcode_disable() {
    NSLog(@"Barcode::disable() DO NOT SUPPORT BY RHODES SW SCANNER !");    
}

void  rho_motobarcode_start() {
    NSLog(@"Barcode::start() DO NOT SUPPORT BY RHODES SW SCANNER !");    
}

void  rho_motobarcode_stop() {
    NSLog(@"Barcode::stop() DO NOT SUPPORT BY RHODES SW SCANNER !");    
}



