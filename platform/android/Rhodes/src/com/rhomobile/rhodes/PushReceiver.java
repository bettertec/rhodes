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

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

public class PushReceiver extends BroadcastReceiver {
    private static final boolean DEBUG = true;

    private static final String TAG = PushReceiver.class.getSimpleName();

	private static final String REG_ID = "registration_id";
	
	public static final String INTENT_SOURCE = PushReceiver.class.getName();
	
	private static final String INTENT_PREFIX = PushReceiver.class.getPackage().getName();
	
	public static final String INTENT_TYPE = INTENT_PREFIX + ".type";
	public static final String INTENT_REGISTRATION_ID = INTENT_PREFIX + ".registration_id";
    public static final String INTENT_MESSAGE_TYPE = INTENT_PREFIX + ".message_type";
    public static final String INTENT_MESSAGE_EXTRAS = INTENT_PREFIX + ".extras";
    public static final String INTENT_MESSAGE_JSON = INTENT_PREFIX + ".json";
    public static final String INTENT_MESSAGE_DATA = INTENT_PREFIX + ".data";

	public static final int INTENT_TYPE_UNKNOWN = 0;
	public static final int INTENT_TYPE_REGISTRATION_ID = 1;
	public static final int INTENT_TYPE_MESSAGE = 2;

	private void handleRegistration(Context context, Intent intent) {
		String id = intent.getStringExtra(REG_ID);
		String error = intent.getStringExtra("error");
		String unregistered = intent.getStringExtra("unregistered");
		if (error != null) {
			Log.d(TAG, "Received error: " + error);
		}
		else if (unregistered != null) {
			Log.d(TAG, "Unregistered: " + unregistered);
		}
		else if (id != null) {
			Log.d(TAG, "Registered: " + id);
			Intent serviceIntent = new Intent(context, RhodesService.class);
			serviceIntent.putExtra(RhodesService.INTENT_SOURCE, INTENT_SOURCE);
			serviceIntent.putExtra(INTENT_TYPE, INTENT_TYPE_REGISTRATION_ID);
			serviceIntent.putExtra(INTENT_REGISTRATION_ID, id);
			context.startService(serviceIntent);
		}
		else
			Log.w(TAG, "Unknown registration event");
	}
	
	private void handleMessage(Context context, Intent intent) {
		Log.d(TAG, "PushReceiver: received");
		Bundle extras = intent.getExtras();

        if (DEBUG) {
            if (extras != null) {
                Log.d(TAG, "Message: " + extras.toString());
                for (String key: extras.keySet()) {
                    Log.d(TAG, key + ": " + extras.get(key).toString());
                }
            } else {
                Log.d(TAG, "Message: <empty>");
            }
        }

		Intent serviceIntent = new Intent(context, RhodesService.class);
		serviceIntent.putExtra(RhodesService.INTENT_SOURCE, INTENT_SOURCE);
		serviceIntent.putExtra(INTENT_TYPE, INTENT_TYPE_MESSAGE);
		serviceIntent.putExtra(INTENT_MESSAGE_EXTRAS, extras);
		context.startService(serviceIntent);
	}
	
	@Override
	public void onReceive(Context context, Intent intent) {
		String action = intent.getAction();
		if (action.equals(PushService.C2DM_INTENT_PREFIX + "REGISTRATION")) {
			try {
				handleRegistration(context, intent);
			}
			catch (Exception e) {
				Log.e(TAG, "Can't handle PUSH registration: " + e.getMessage());
			}
		}
		else if (action.equals(PushService.C2DM_INTENT_PREFIX + "RECEIVE")) {
			try {
				handleMessage(context, intent);
			}
			catch (Exception e) {
				Log.e(TAG, "Can't handle PUSH message: " + e.getMessage());
			}
		}
		else
			Log.w(TAG, "Unknown action received (PUSH): " + action);
		setResult(Activity.RESULT_OK, null /* data */, null /* extra */);
	}

}
