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

/*
 *  RhoNativeViewManager.h
 *  rhorunner
 *
 *  Created by Dmitry Soldatenkov on 8/25/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _RHO_NATIVE_VIEW_MANAGER_
#define _RHO_NATIVE_VIEW_MANAGER_


#include "common/RhoStd.h"
#include "ruby/ext/rho/rhoruby.h"


class NativeView {
public:
	virtual ~NativeView(){}
	
        // that function must return native object provided view functionality :
	// UIView* for iPhone
	// jobject for Android - jobect must be android.view.View class type
	// HWND for Windows Mobile 
	virtual void* getView() = 0;
	
	virtual void navigate(const char* url) = 0;


        // that function must return native object provided view functionality :
	// UIView* for iPhone
	// jobject for Android - jobect must be android.view.View class type
	// HWND for Windows Mobile 
        // this function executed when we make native view by Ruby NativeViewManager (not by URL prefix) 
        virtual void* createView(VALUE params) {return getView();}
};

class NativeViewFactory {
public:
	virtual ~NativeViewFactory(){}
	virtual NativeView* getNativeView(const char* viewType) = 0;
	virtual void destroyNativeView(NativeView* nativeView) = 0;
};

#define   OPEN_IN_MODAL_FULL_SCREEN_WINDOW  11111


class RhoNativeViewManager {
public: 
	static void registerViewType(const char* viewType, NativeViewFactory* factory);
	static void unregisterViewType(const char* viewType);

	// that function return native object used for display Web content :
	// UIWebView* for iPhone
	// jobject for Android - jobect is android.webkit.WebView class type
	// HWND for Windows Mobile 
	static void* getWebViewObject(int tab_index);


        // destroy native view (opened with URL prefix or in separated full-screen window)
        // this function can executed from your native code (from NativeView code, for example)
        // instead of this function you can execute destroy() for Ruby NativeView object
        static void destroyNativeView(NativeView* nativeView);


        static int openNativeView(const char* viewType, int tab_index, VALUE params);

        static void closeNativeView(int v_id);

};


class RhoNativeViewRunnable {
public:
	virtual ~RhoNativeViewRunnable(){}
	virtual void run() = 0;
};


class RhoNativeViewUtil {
public: 
	static void executeInUIThread_WM(RhoNativeViewRunnable* command);
};

#endif
