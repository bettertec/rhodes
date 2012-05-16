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

package com.rhomobile.rhodes.nativeview;

import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.FrameLayout;

import com.rhomobile.rhodes.RhodesActivity;
import com.rhomobile.rhodes.util.ContextFactory;

public class RhoNativeViewManager {

	private static class RhoNativeViewImpl implements IRhoCustomView {
        private String mViewType;
        private long mFactoryHandle;
        private long mViewHandle;
        private FrameLayout mContainerView;

        RhoNativeViewImpl(String viewType, long factory_h, long view_h) {
            mViewType = viewType;
            mFactoryHandle = factory_h;
            mViewHandle = view_h;
            mContainerView = new FrameLayout(ContextFactory.getUiContext());
            mContainerView.addView(getView(), new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT));
        }

		public View getView() {
			return getViewByHandle(mViewHandle);
		}
		
		public void navigate(String url) {
			navigateByHandle(mViewHandle, url);
		}
		public void destroyView() {
			destroyByHandle(mFactoryHandle, mViewHandle);
		}
		public String getViewType() {
			return mViewType;
		}

        @Override
        public ViewGroup getContainerView() {
            return mContainerView;
        }

        @Override
        public void stop() {
        }
	}
	
	public static Object getWebViewObject(int tab_index) {
		return RhodesActivity.safeGetInstance().getMainView().getWebView(tab_index).getView();
	}

	public static IRhoCustomView getNativeViewByType(String typename) {
		long factory_h = getFactoryHandleByViewType(typename);
		if (factory_h == 0) {
			return null;
		}
		long view_h = getViewHandleByFactoryHandle(factory_h);
		if (view_h == 0) {
			return null;
		}
		RhoNativeViewImpl nv = new RhoNativeViewImpl(typename, factory_h, view_h);
		return nv;
	}
	
	private native static View getViewByHandle(long handle);
	private native static void navigateByHandle(long handle, String url);
	private native static long getFactoryHandleByViewType(String viewtype);
	private native static long getViewHandleByFactoryHandle(long factory_h);
	private native static void destroyByHandle(long factory_h, long view_h);
	
	
}