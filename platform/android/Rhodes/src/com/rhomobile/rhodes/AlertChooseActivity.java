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

package com.rhomobile.rhodes;

import com.rhomobile.rhodes.file.RhoFileApi;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Bundle;

/**
 * This Aktivity will show a Dialog Box that  
 * 
 * @author salieri
 *
 */
public class AlertChooseActivity extends Activity {
	
	private static final String TAG = AlertChooseActivity.class.getSimpleName();
	public static final String INTENT_SOURCE = AlertChooseActivity.class.getName();
	private static final boolean DEBUG = false;
	private AlertDialog dialog;
	private Bundle parentExtras;
    @Override
	protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Logger.D(TAG, "onCreate");
        Bundle extras = getIntent().getExtras();
        
        parentExtras = extras.getBundle(RhodesService.INTENT_EXTRA_PREFIX + "PARENT_EXTRA");
        
        String iconName = "popup_logo.png";
        
        Resources res = getResources();
        Drawable icon = null;
		if (iconName != null) {
			if (iconName.equalsIgnoreCase("alert"))
				icon = res.getDrawable(AndroidR.drawable.alert_alert);
			else if (iconName.equalsIgnoreCase("question"))
				icon = res.getDrawable(AndroidR.drawable.alert_question);
			else if (iconName.equalsIgnoreCase("info"))
				icon = res.getDrawable(AndroidR.drawable.alert_info);
			else {
				String iconPath = RhoFileApi.normalizePath("apps/" + iconName);
				Bitmap bitmap = BitmapFactory.decodeStream(RhoFileApi.open(iconPath));
				if (bitmap != null)
					icon = new BitmapDrawable(bitmap);
			}
		}
        
        dialog = new AlertDialog.Builder(this)
        .setTitle(extras.getString(RhodesService.INTENT_EXTRA_PREFIX + "title"))
        .setMessage(extras.getString(RhodesService.INTENT_EXTRA_PREFIX + "message"))
        .setPositiveButton("Annehmen", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int whichButton) {
            	 Logger.D(TAG, "OK Button start");
            	 
            	 Context ctx = RhodesService.getContext();
            	Intent serviceIntent = new Intent(ctx, RhodesService.class);
            	serviceIntent.putExtra(RhodesService.INTENT_SOURCE, INTENT_SOURCE);
        		serviceIntent.putExtra("BUTTON_CLICKED", "OK");
        		serviceIntent.putExtra(RhodesService.INTENT_EXTRA_PREFIX + "PARENT_EXTRA", parentExtras);
        		ctx.startService(serviceIntent);
        		finish();
            }
        })
        .setNegativeButton("Ablehnen", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int whichButton) {
            	Logger.D(TAG, "NO Button start");
            	 Context ctx = RhodesService.getContext();
             	Intent serviceIntent = new Intent(ctx, RhodesService.class);
             	serviceIntent.putExtra(RhodesService.INTENT_SOURCE, INTENT_SOURCE);
         		serviceIntent.putExtra("BUTTON_CLICKED", "NO");
        		serviceIntent.putExtra(RhodesService.INTENT_EXTRA_PREFIX + "PARENT_EXTRA", parentExtras);
         		ctx.startService(serviceIntent);
         		finish();
            }
        }).setIcon(icon).create();
      //add this to your code
//          Window window = dialog.getWindow(); 
//          WindowManager.LayoutParams lp = window.getAttributes();
//          lp.token = mInputView.getWindowToken();
//          lp.type = WindowManager.LayoutParams.TYPE_APPLICATION_ATTACHED_DIALOG;
//          window.setAttributes(lp);
//          window.addFlags(WindowManager.LayoutParams.FLAG_ALT_FOCUSABLE_IM);
      //end addons

        dialog.show();
    }

	@Override
	public void onStart() {
		super.onStart();
        Logger.D(TAG, "onStart");
	}
	
	@Override
	public void onResume() {
        Logger.D(TAG, "onResume");
		super.onResume();
        
	}

    @Override
    public void onPause() 
    {
        super.onPause();
        Logger.D(TAG, "onPause");

    }

    @Override
	public void onStop() 
	{
		super.onStop();
        Logger.D(TAG, "onStop");
	}
	
	@Override
	public void onDestroy() {
        Logger.D(TAG, "onDestroy");
      
		super.onDestroy();
	}

	
	
}
