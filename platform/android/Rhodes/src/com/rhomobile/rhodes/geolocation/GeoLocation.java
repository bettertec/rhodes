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
package com.rhomobile.rhodes.geolocation;

import android.location.Location;
import android.location.LocationProvider;

import com.rhomobile.rhodes.Capabilities;
import com.rhomobile.rhodes.Logger;
import com.rhomobile.rhodes.RhoConf;
import com.rhomobile.rhodes.util.PerformOnUiThread;

import java.util.Iterator;
import java.util.List;

public class GeoLocation {

	private static final String TAG = "GeoLocation";
	private static volatile GeoLocationImpl locImpl = null;
	
	private static final String CALLBACK_UPDATE_INTERVAL = "gps_ping_timeout_sec";
	
	private static double ourLatitude = 0;
	private static double ourLongitude = 0;
	private static double ourAccuracy = 0;
	private static boolean ourIsKnownPosition = false;
	private static boolean ourIsEnable = true;
	private static boolean ourIsErrorState = false;
	
	public static native void geoCallback();
	public static native void geoCallbackError();
	
	
	public static void onUpdateLocation() {
		Location loc = getImpl().getLocation();
		synchronized (GeoLocation.class) {
			if (loc != null) {
				ourLatitude = loc.getLatitude();
				ourLongitude = loc.getLongitude();
				ourAccuracy = loc.getAccuracy();
				ourIsKnownPosition = true;
			}
			else {
				ourLatitude = 0;
				ourLongitude = 0;
				ourAccuracy = 0;
				ourIsKnownPosition = false;
			}
		}
	}
	
	public static void onGeoCallback() {
		Logger.T(TAG, "onGeoCallback()");
		if (ourIsEnable && !ourIsErrorState) {
			Logger.T(TAG, "onGeoCallback() run native Callback");
			geoCallback();
		}
		else {
			String myMsg = "onGeoCallback() SKIP - because ourIsEnable: " + ourIsEnable + " and ourIsErrorState " + ourIsErrorState;
			//Logger.T(TAG, "onGeoCallback() SKIP");
			Logger.T(TAG, myMsg);
		}
	}
	
	public static void onGeoCallbackError() {
		if (ourIsEnable && !ourIsErrorState) {
			Logger.T(TAG, "onGeoCallbackError() run native Callback");
			ourIsErrorState = true;
			geoCallbackError();
			ourIsErrorState = false;
		}
		else {
			Logger.T(TAG, "onGeoCallbackError() SKIP");
		}
	}
	
	private static Thread ourCallbackThread = null;
	
	private static void checkState() throws IllegalAccessException {
		if (!Capabilities.GPS_ENABLED)
			throw new IllegalAccessException("Capability GPS disabled");
	}
	
	private static GeoLocationImpl getImpl() {
		if (locImpl == null) {
			synchronized (GeoLocation.class) {
				if (locImpl == null) {
					Logger.T(TAG, "Creating GeoLocationImpl instance.");
					locImpl = new GeoLocationImpl();
					Logger.T(TAG, "GeoLocationImpl instance has created.");
					//ourIsFirstUpdate = true;
					locImpl.start();
					Logger.T(TAG, "GeoLocation has started.");
				}
			}
		}
		return locImpl;
	}
	
	public static void stop() {
		Logger.T(TAG, "stop");
		ourIsEnable = false;
		resetCallbackThread(0);
		try {
			if (locImpl == null)
				return;
			synchronized(GeoLocation.class) {
				if (locImpl == null)
					return;
				locImpl.stop();
				locImpl = null;
			}
		}
		catch (Exception e) {
			Logger.E(TAG, e);
		}
	}

	public static boolean isAvailable() {
		Logger.T(TAG, "isAvailable...");
		try {
			boolean result = false;
			if (locImpl != null) {
				checkState();
				result = getImpl().isAvailable() > 0;
			}
			Logger.T(TAG, "Geo location service is " + (result ? "" : "not ") + "available");
			Logger.T(TAG, "Geo location service number is " + getImpl().isAvailable());
			Logger.T(TAG, "full string: " + getLocationString());
			return result;
		}
		catch (Exception e) {
            Logger.E(TAG, e);
		}
		return false;
	}

	public static int numberAvailable() {
		Logger.T(TAG, "numberAvailable...");
		try {
			int result = 0;
			if (locImpl != null) {
				checkState();
				result = getImpl().isAvailable();
			}
			Logger.T(TAG, "Geo location service number is " + result);
			Logger.T(TAG, "full string: " + getLocationString());
			return result;
		}
		catch (Exception e) {
            Logger.E(TAG, e);
		}
		return 0;
	}
	
	public static String getLocationString() {
		List<String> currentProviders = getImpl().locationManager.getProviders(true);
		
		String strBody = "";

		Iterator<String> it = currentProviders.iterator();
		while (it.hasNext()) {
			String lpString = it.next();
			Logger.T(TAG, "looking at provider: " + lpString);
			Location l = getImpl().locationManager.getLastKnownLocation(lpString);
			strBody += "&[" + lpString + "][latitude]=" + l.getLatitude();
			strBody += "&[" + lpString + "][longitude]=" + l.getLongitude();
			strBody += "&[" + lpString + "][accuracy]=" + l.getAccuracy();
		}
		return strBody;
		/*Location loc = getImpl().getLocation(provider);
		synchronized (GeoLocation.class) {
			if (loc != null) {				
            	String strBody = "&geoProvider[" + loc.getProvider() + "][available]=1";
            	strBody += "&geoProvider[" + loc.getProvider() + "][known_position]=true";
            	strBody += "&geoProvider[" + loc.getProvider() + "][latitude]=" + loc.getLatitude();
            	strBody += "&geoProvider[" + loc.getProvider() + "][longitude]=" + loc.getLongitude();
            	strBody += "&geoProvider[" + loc.getProvider() + "][accuracy]=" + loc.getAccuracy();
    			return strBody;
			}
			else {
				String strBody = "&geoProvider[NA][available]=1";
            	strBody += "&geoProvider[NA][known_position]=true";
            	strBody += "&geoProvider[NA][latitude]=0";
            	strBody += "&geoProvider[NA][longitude]=0";
            	strBody += "&geoProvider[NA][accuracy]=0";
    			return strBody;
			}
		}*/
	}
	
	public static double getLatitude() {
		onUpdateLocation();
		try {
			checkState();
			Logger.T(TAG, "getLatitude");
			return ourLatitude;
		}
		catch (Exception e) {
            Logger.E(TAG, e);
		}
		return 0.0;
	}

	public static double getLongitude() {
		onUpdateLocation();
		try {
			checkState();
			Logger.T(TAG, "getLongitude");
			return ourLongitude;
		}
		catch (Exception e) {
            Logger.E(TAG, e);
		}
		return 0.0;
	}
	
	public static float getAccuracy() {
		onUpdateLocation();
		try {
			checkState();
			Logger.T(TAG, "getAccuracy");
			return (float)ourAccuracy;
		}
		catch (Exception e) {
            Logger.E(TAG, e);
		}
		return 0;
	}

	public static boolean isKnownPosition() {
		onUpdateLocation();
		try {
			checkState();
			Logger.T(TAG, "isKnownPosition");
			String myMsg = "position is known: " + ourIsKnownPosition;
			Logger.T(TAG, myMsg);
			return ourIsKnownPosition;
		}
		catch (Exception e) {
            Logger.E(TAG, e);
		}
		return false;
	}
	
	private static void resetCallbackThread(int period) {
		Logger.T(TAG, "resetCallbackThread: " + period + "s");
		if (ourCallbackThread != null) {
			ourCallbackThread.interrupt();
			ourCallbackThread = null;
		}
		if (period > 0) {
			final int sleep_period = period;
			ourCallbackThread = new Thread(new Runnable() {
				private boolean ourLastCommandProcessed = true;
				public void run() {
					Logger.I(TAG, "\"callback\" thread started");
					for (;;) {
						if (!ourIsEnable) {
							break;
						}
						try {
							if (ourLastCommandProcessed) {
								ourLastCommandProcessed = false;
								Logger.T(TAG, "callback thread: perform callback in UI thread");
								PerformOnUiThread.exec(new Runnable() {
									public void run() {
										Logger.T(TAG, "callback thread: callback in UI thread START");
										onGeoCallback();
										Logger.T(TAG, "callback thread: callback in UI thread FINISH");
										ourLastCommandProcessed = true;
									}
								});
							}
							else {
								Logger.T(TAG, "callback thread: previous command not processed - skip current callback");
							}
							Thread.sleep(sleep_period);
						}
						catch (InterruptedException e) {
							Logger.T(TAG, "\"callback\" thread interrupted");
							break;
						}
					}
				}
			});	
			ourCallbackThread.start();
		}
		else {
			Logger.T(TAG, "resetCallbackThread: zero period - not make any thread");
		}
		
	}
	
	public static void setTimeout(int nsec) {
		try {
			int p = nsec*1000;
			if (p < 0) {
				p = RhoConf.getInt(CALLBACK_UPDATE_INTERVAL) * 1000;
			}
			
			checkState();
			Logger.T(TAG, "setTimeout: " + nsec + "s");
		
			ourIsEnable = true;
			if (p <= 0) {
				p = 250;
			}
			resetCallbackThread(p);
		
		}
		catch (Exception e) {
            Logger.E(TAG, e);
		}
	}

}
