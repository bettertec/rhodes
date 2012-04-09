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

package com.rhomobile.rhodes.mainview;

import java.lang.reflect.Constructor;
import java.util.Map;
import java.util.Vector;

import com.rhomobile.rhodes.AndroidR;
import com.rhomobile.rhodes.Capabilities;
import com.rhomobile.rhodes.Logger;
import com.rhomobile.rhodes.RhodesActivity;
import com.rhomobile.rhodes.RhodesAppOptions;
import com.rhomobile.rhodes.RhodesApplication;
import com.rhomobile.rhodes.RhodesService;
import com.rhomobile.rhodes.file.RhoFileApi;
import com.rhomobile.rhodes.mainview.MainView;
import com.rhomobile.rhodes.nativeview.RhoNativeViewManager;
import com.rhomobile.rhodes.util.ContextFactory;
import com.rhomobile.rhodes.util.PerformOnUiThread;
import com.rhomobile.rhodes.util.Utils;
import com.rhomobile.rhodes.webview.IRhoWebView;
import com.rhomobile.rhodes.webview.GoogleWebView;

import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.util.DisplayMetrics;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.AbsoluteLayout;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.TextView;

public class SimpleMainView implements MainView {

	private final static String TAG = "SimpleMainView";	
	
	private static final int WRAP_CONTENT = ViewGroup.LayoutParams.WRAP_CONTENT;
	private static final int FILL_PARENT = ViewGroup.LayoutParams.FILL_PARENT;
	
	private class ActionBack implements View.OnClickListener {
		public void onClick(View v) {
			goBack();//back(0);
		}
	};
	
	private void addWebViewToMainView(View webView, int index, LinearLayout.LayoutParams params) {
		Context ctx = RhodesActivity.getContext();
		AbsoluteLayout containerView = new AbsoluteLayout(ctx);
		
		containerView.addView(webView, new AbsoluteLayout.LayoutParams(AbsoluteLayout.LayoutParams.MATCH_PARENT, AbsoluteLayout.LayoutParams.MATCH_PARENT, 0, 0));
		
		view.addView(containerView, index, params);
	}
	
	private void removeWebViewFromMainView() {
		ViewGroup pv = (ViewGroup)webView.getView().getParent();
		pv.removeView(webView.getView());
		view.removeView(pv);
	}
	
	public class MyView extends LinearLayout {
		public MyView(Context ctx) {
			super(ctx);
		}
		
		protected void onSizeChanged (int w, int h, int oldw, int oldh) {
			super.onSizeChanged(w, h, oldw, oldh);
			StringBuilder msg = new StringBuilder();
			msg.append(" Main Window :: onSizeChanged() old [ ");
			msg.append(w);
			msg.append(" x ");
			msg.append(h);
			msg.append(" ]  new [ ");
			msg.append(oldw);
			msg.append(" x ");
			msg.append(oldh);
			msg.append(" ]");
			Utils.platformLog("SimpleMainView.View", msg.toString());
		}
	}
	
	private class ActionForward implements View.OnClickListener {
		public void onClick(View v) {
			forward(0);
		}
	};
	
	private class ActionHome implements View.OnClickListener {
		public void onClick(View v) {
			navigate(RhodesAppOptions.getStartUrl(), 0);
		}
	};
	
	private class ActionOptions implements View.OnClickListener {
		public void onClick(View v) {
			navigate(RhodesAppOptions.getOptionsUrl(), 0);
		}
	};
	
	private class ActionRefresh implements View.OnClickListener {
		public void onClick(View v) {
			reload(0);
		}
	};

	private class ActionExit implements View.OnClickListener {
		public void onClick(View v) {
			restoreWebView();
			RhodesService.exit();
		}
	};

	private class ActionCustom implements View.OnClickListener {
		private String url;
		
		public ActionCustom(String u) {
			url = u;
		}
		
		public void onClick(View v) {
			PerformOnUiThread.exec(new Runnable() {
				public void run() {
					RhodesService.loadUrl(ActionCustom.this.url);
				}
			});
		}
	};
	
	private LinearLayout view;
	private IRhoWebView webView;
	private RhoNativeViewManager.RhoNativeView mNativeView = null;
	private View mNativeViewView = null;
	private LinearLayout navBar = null;
	private LinearLayout toolBar = null;
	
	private int mCustomBackgroundColor = 0;
	private boolean mCustomBackgroundColorEnable = false;
	
	public View getView() {
		return view;
	}

	@Override
	public IRhoWebView getWebView(int tab_index) {
		return webView;
	}

	public void setNativeView(RhoNativeViewManager.RhoNativeView nview) {
		restoreWebView();
		mNativeView = nview;
		mNativeViewView = mNativeView.getView();
		if (mNativeViewView != null) {
			//view.removeView(webView.getView());
			removeWebViewFromMainView();
			//int view_index = 0;
			//if (navBar != null) {
				//view_index = 1;
			//}
			if (navBar != null) {
				view.removeView(navBar);
			}
			if (toolBar != null) {
				view.removeView(toolBar);
			}
			int index = 0;
			if (navBar != null) {
				view.addView(navBar, index);
				index++;
			}
			view.addView( mNativeViewView, index, new LinearLayout.LayoutParams(FILL_PARENT, 0, 1));
			index++;
			if (toolBar != null) {
				view.addView(toolBar, index);
			}
			
			//view.bringChildToFront(mNativeViewView);
			//view.requestLayout();
		}
		else {
			mNativeView = null;
			mNativeViewView = null;
		}
	}
	
	public void restoreWebView() {
		if (mNativeView != null) {
			view.removeView(mNativeViewView);
			mNativeViewView = null;
			//int view_index = 0;
			//if (navBar != null) {
				//view_index = 1;
			//}
			
			if (navBar != null) {
				view.removeView(navBar);
			}
			if (toolBar != null) {
				view.removeView(toolBar);
			}
			
			int index = 0;
			if (navBar != null) {
				view.addView(navBar, index);
				index++;
			}
			//view.addView(webView.getView(), index, new LinearLayout.LayoutParams(FILL_PARENT, 0, 1));
			addWebViewToMainView(webView.getView(), index, new LinearLayout.LayoutParams(FILL_PARENT, 0, 1));
			index++;
			if (toolBar != null) {
				view.addView(toolBar, index);
			}

			//view.bringChildToFront(webView);

			mNativeView.destroyView();
			mNativeView = null;
			//view.requestLayout();
		}
	}
	

    private String processForNativeView(String _url) {
    	StringBuilder s = new StringBuilder("processForNativeView : [");
    	s.append(_url);
    	s.append("]");
    	Utils.platformLog(TAG, s.toString());
    	
    	String url = _url;
    	String callback_prefix = "call_stay_native";

    	// find protocol:navto pairs

    	int last = -1;
    	int cur = url.indexOf(":", last+1);
    	while (cur > 0) {
    		String protocol = url.substring(last+1, cur);
    		String navto = url.substring(cur+1, url.length());
    		
    		if (callback_prefix.equals(protocol)) {
    			// navigate but still in native view
    			String cleared_url = url.substring(callback_prefix.length()+1, url.length());
    			return cleared_url;
    		}
    		// check protocol for nativeView
    		RhoNativeViewManager.RhoNativeView nvf = RhoNativeViewManager.getNativeViewByteType(protocol);
    		if (nvf != null) {
    			// we should switch to NativeView
    			//restoreWebView();
    			if (mNativeView != null) {
    				if ( !protocol.equals(mNativeView.getViewType()) ) {
        				setNativeView(nvf);
    				}
    			}
    			else {
    				setNativeView(nvf);
    			}
    			if (mNativeView != null) {
	    			mNativeView.navigate(navto);
	   				return "";
    			}
    		}
    		last = cur;
    		int c1 = url.indexOf(":", last+1);
    		int c2 = url.indexOf("/", last+1);
    		if ((c1 < c2)) {
    			if (c1 <= 0) {
    				cur = c2;
    			}
    			else {
    				cur = c1;
    			}
    		}
    		else {
    			if (c2 <= 0) {
    				cur = c1;
    			}
    			else {
    				cur = c2;
    			}
    		}
    	}
    	restoreWebView();
    	return url;
    }
	
	
	public IRhoWebView detachWebView() {
		restoreWebView();
		IRhoWebView v = null;
		if (webView != null) {
			//view.removeView(webView.getView());
			removeWebViewFromMainView();
			v = webView;
			webView = null;
		}
		return v;
	}
	
	private View createButton(Map<Object,Object> hash) {
		Context ctx = RhodesActivity.getContext();
		
		Object actionObj = hash.get("action");
		if (actionObj == null || !(actionObj instanceof String))
			throw new IllegalArgumentException("'action' should be String");
		
		String action = (String)actionObj;
		if (action.length() == 0)
			throw new IllegalArgumentException("'action' should not be empty");
		
		Drawable icon = null;
		String label = null;
		View.OnClickListener onClick = null;
		
		if (action.equalsIgnoreCase("back")) {
			icon = ctx.getResources().getDrawable(AndroidR.drawable.back);
			onClick = new ActionBack();
		}
		else if (action.equalsIgnoreCase("forward")) {
			if (RhodesService.isJQTouch_mode()) {
				return null;
			}
			icon = ctx.getResources().getDrawable(AndroidR.drawable.next);
			onClick = new ActionForward();
		}
		else if (action.equalsIgnoreCase("home")) {
			icon = ctx.getResources().getDrawable(AndroidR.drawable.home);
			onClick = new ActionHome();
		}
		else if (action.equalsIgnoreCase("options")) {
			icon = ctx.getResources().getDrawable(AndroidR.drawable.options);
			onClick = new ActionOptions();
		}
		else if (action.equalsIgnoreCase("refresh")) {
			icon = ctx.getResources().getDrawable(AndroidR.drawable.refresh);
			onClick = new ActionRefresh();
		}
		else if (action.equalsIgnoreCase("close") || action.equalsIgnoreCase("exit")) {
			icon = ctx.getResources().getDrawable(AndroidR.drawable.exit);
			onClick = new ActionExit();
		}
		else if (action.equalsIgnoreCase("separator"))
			return null;
		
		DisplayMetrics metrics = new DisplayMetrics();
		WindowManager wm = (WindowManager)ctx.getSystemService(Context.WINDOW_SERVICE);
		wm.getDefaultDisplay().getMetrics(metrics);
		
		Object iconObj = hash.get("icon");
		if (iconObj != null) {
			if (!(iconObj instanceof String))
				throw new IllegalArgumentException("'icon' should be String");
			String iconPath = "apps/" + (String)iconObj;
			iconPath = RhoFileApi.normalizePath(iconPath);
			Bitmap bitmap = BitmapFactory.decodeStream(RhoFileApi.open(iconPath));
			if (bitmap == null)
				throw new IllegalArgumentException("Can't find icon: " + iconPath);
			bitmap.setDensity(DisplayMetrics.DENSITY_MEDIUM);
			icon = new BitmapDrawable(bitmap);
		}
		
		if (icon == null) {
			Object labelObj = hash.get("label");
			if (labelObj == null || !(labelObj instanceof String))
				throw new IllegalArgumentException("'label' should be String");
			label = (String)labelObj;
		}
		
		if (icon == null && label == null)
			throw new IllegalArgumentException("One of 'icon' or 'label' should be specified");
		
		if (onClick == null)
			onClick = new ActionCustom(action);
		
		View button;
		if (icon != null) {
			ImageButton btn = new ImageButton(ctx);
			btn.setImageDrawable(icon);
			button = btn;
			if (mCustomBackgroundColorEnable) {
				Drawable d = btn.getBackground();
				if (d != null) {
					d.setColorFilter(mCustomBackgroundColor, android.graphics.PorterDuff.Mode.SRC_OVER);
				}
				else {
					btn.setBackgroundColor(mCustomBackgroundColor);
				}
			}
		}
		else {
			Button btn = new Button(ctx);
			btn.setText(label);
			if (mCustomBackgroundColorEnable) {
				btn.setBackgroundColor(mCustomBackgroundColor);
				int gray = (((mCustomBackgroundColor & 0xFF0000) >> 16) + ((mCustomBackgroundColor & 0xFF00) >> 8) + ((mCustomBackgroundColor & 0xFF)))/3; 
				if (gray > 128) {
					btn.setTextColor(0xFF000000);
				}
				else {
					btn.setTextColor(0xFFFFFFFF);
				}
			}
			button = btn;
		}
		
		button.setOnClickListener(onClick);
		
		return button;
	}
	
	@SuppressWarnings("unchecked")
	private void setupToolbar(LinearLayout tool_bar, Object params) {
		Context ctx = RhodesActivity.getContext();

                mCustomBackgroundColorEnable = false;

		Vector<Object> buttons = null;
		if (params != null) {
			if (params instanceof Vector<?>) {
				buttons = (Vector<Object>)params;
			}
			else if (params instanceof Map<?,?>) {
				Map<Object,Object> settings = (Map<Object,Object>)params;
				
				Object colorObj = settings.get("color");
				if (colorObj != null && (colorObj instanceof Map<?,?>)) {
					Map<Object,Object> color = (Map<Object,Object>)colorObj;
					
					Object redObj = color.get("red");
					Object greenObj = color.get("green");
					Object blueObj = color.get("blue");
					
					if (redObj != null && greenObj != null && blueObj != null &&
							(redObj instanceof String) && (greenObj instanceof String) && (blueObj instanceof String)) {
						try {
							int red = Integer.parseInt((String)redObj);
							int green = Integer.parseInt((String)greenObj);
							int blue = Integer.parseInt((String)blueObj);
							
							mCustomBackgroundColor = ((red & 0xFF ) << 16) | ((green & 0xFF ) << 8) | ((blue & 0xFF )) | 0xFF000000;
							mCustomBackgroundColorEnable = true;
							
							tool_bar.setBackgroundColor(Color.rgb(red, green, blue));
						}
						catch (NumberFormatException e) {
							// Do nothing here
						}
					}
				}
				
				Object bkgObj = settings.get("background_color");
				if ((bkgObj != null) && (bkgObj instanceof String)) {
					int color = Integer.decode(((String)bkgObj)).intValue();
					int red = (color & 0xFF0000) >> 16;
					int green = (color & 0xFF00) >> 8;
					int blue = (color & 0xFF);
					tool_bar.setBackgroundColor(Color.rgb(red, green, blue));
					mCustomBackgroundColor = color | 0xFF000000;
					mCustomBackgroundColorEnable = true;
				}
				
				Object buttonsObj = settings.get("buttons");
				if (buttonsObj != null && (buttonsObj instanceof Vector<?>))
					buttons = (Vector<Object>)buttonsObj;
			}
		}
		
		if (params != null) {
			LinearLayout group = null;
			// First group should have gravity LEFT
			int gravity = Gravity.LEFT;
			for (int i = 0, lim = buttons.size(); i < lim; ++i) {
				Object param = buttons.elementAt(i);
				if (!(param instanceof Map<?,?>))
					throw new IllegalArgumentException("Hash expected");
				
				Map<Object, Object> hash = (Map<Object, Object>)param;
				
				View button = createButton(hash);
				if (button == null) {
					group = null;
					gravity = Gravity.CENTER;
					continue;
				}
				
				button.setLayoutParams(new LinearLayout.LayoutParams(WRAP_CONTENT, WRAP_CONTENT));
				if (group == null) {
					group = new LinearLayout(ctx);
					group.setGravity(gravity);
					group.setOrientation(LinearLayout.HORIZONTAL);
					group.setLayoutParams(new LinearLayout.LayoutParams(WRAP_CONTENT, FILL_PARENT, 1));
					tool_bar.addView(group);
				}
				group.addView(button);
			}
			
			// Last group should have gravity RIGHT
			if (group != null) {
				group.setGravity(Gravity.RIGHT);
				tool_bar.requestLayout();
			}
		}
	}
	
	private void init(IRhoWebView v, Object params) {
		RhodesActivity activity = RhodesActivity.safeGetInstance();
		
		view = new MyView(activity);
		view.setOrientation(LinearLayout.VERTICAL);
		view.setGravity(Gravity.BOTTOM);
		view.setLayoutParams(new LinearLayout.LayoutParams(FILL_PARENT, FILL_PARENT));
		
		webView = v;
		if (webView == null) {
		    webView = activity.createWebView();
		}
		addWebViewToMainView(webView.getView(), 0, new LinearLayout.LayoutParams(FILL_PARENT, 0, 1));
		
		LinearLayout bottom = new LinearLayout(activity);
		bottom.setOrientation(LinearLayout.HORIZONTAL);
		bottom.setBackgroundColor(Color.GRAY);
		bottom.setLayoutParams(new LinearLayout.LayoutParams(FILL_PARENT, WRAP_CONTENT, 0));
		view.addView(bottom);
		
		toolBar = bottom;
		
		setupToolbar(toolBar, params);
		
		webView.getView().requestFocus();
	}
	
	public SimpleMainView() {
		init(null, null);
	}
	
	public SimpleMainView(IRhoWebView v) {
		init(v, null);
	}
	
	public SimpleMainView(IRhoWebView v, Object params) {
		init(v, params);
	}
	
	public void setWebBackgroundColor(int color) {
		view.setBackgroundColor(color);
		webView.getView().setBackgroundColor(color);
	}
	
	public void back(int index) {
		restoreWebView();
        
        boolean bStartPage = RhodesService.isOnStartPage();

        if ( !bStartPage && webView.canGoBack() ) {
            webView.goBack();
        }
        else
        {    
	        RhodesActivity ra = RhodesActivity.getInstance();
	        if ( ra != null )
	            ra.moveTaskToBack(true);
        }
	}
	
	public void goBack() 
	{
		RhodesService.navigateBack();
	}

	public void forward(int index) {
		restoreWebView();
		webView.goForward();
	}

	public void navigate(String url, int index) {
		String cleared_url = processForNativeView(url);
		Logger.I(TAG, "Cleared URL: " + url);
		if (cleared_url.length() > 0) {
			// check for handle because if we call loadUrl - WebView do not check this url for handle
			if (!RhodesService.getInstance().handleUrlLoading(cleared_url)) {
				webView.loadUrl(cleared_url);
			}
		}
	}

    @Override
    public void executeJS(String js, int index) {
        com.rhomobile.rhodes.WebView.executeJs(js, index);
    }
    
	public void reload(int index) {
		if (mNativeViewView != null) {
			mNativeViewView.invalidate();
		}
		else {
			webView.reload();
		}
	}
	
	public void stopNavigate(int index) {
	    if (mNativeViewView == null) {
	        webView.stopLoad();
	    }
	}
	
	public String currentLocation(int index) {
		return webView.getUrl();
	}

	public void switchTab(int index) {
		// Nothing
	}
	
	public int activeTab() {
		return 0;
	}
	
	public void loadData(String data, int index) {
		restoreWebView();
		webView.loadData(data, "text/html", "utf-8");
	}

	public void addNavBar(String title, Map<Object,Object> left, Map<Object,Object> right) {
		removeNavBar();
		
		Context ctx = RhodesActivity.getContext();
		
		LinearLayout top = new LinearLayout(ctx);
		top.setOrientation(LinearLayout.HORIZONTAL);
		top.setBackgroundColor(Color.GRAY);
		top.setGravity(Gravity.CENTER);
		top.setLayoutParams(new LinearLayout.LayoutParams(FILL_PARENT, WRAP_CONTENT, 0));
		
		View leftButton = createButton(left);
		leftButton.setLayoutParams(new LinearLayout.LayoutParams(WRAP_CONTENT, WRAP_CONTENT, 1));
		top.addView(leftButton);
		
		TextView label = new TextView(ctx);
		label.setText(title);
		label.setGravity(Gravity.CENTER);
		label.setTextSize((float)30.0);
		label.setLayoutParams(new LinearLayout.LayoutParams(WRAP_CONTENT, WRAP_CONTENT, 2));
		top.addView(label);
		
		if (right != null) {
			View rightButton = createButton(right);
			rightButton.setLayoutParams(new LinearLayout.LayoutParams(WRAP_CONTENT, WRAP_CONTENT, 1));
			top.addView(rightButton);
		}
		
		navBar = top;
		view.addView(navBar, 0);
	}
	
	public void removeNavBar() {
		if (navBar == null)
			return;
		view.removeViewAt(0);
		navBar = null;
	}
	
	public void setToolbar(Object params) {
		toolBar.setBackgroundColor(Color.GRAY);
		toolBar.removeAllViews();
		setupToolbar(toolBar, params);
		toolBar.requestLayout();
		view.requestLayout();
	}
	
	public void removeToolbar() {
		toolBar.removeAllViews();
		toolBar.requestLayout();
		view.requestLayout();
	}

	@Override
	public int getTabsCount() {
		return 0;
	}
}
